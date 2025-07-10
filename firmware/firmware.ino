#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE  // Enable Sense version constants
#include <I2S.h>
#include "esp_camera.h"
#include "src/hardware/camera_pins.h"
#include "src/utils/mulaw.h"
#include "src/utils/timing.h"
#include "src/utils/power_management.h"
#include "src/utils/led_manager.h"
#include "src/core/battery_code.h"
#include "src/core/charging_manager.h"
#include "src/platform/constants.h"
#include "src/core/device_status.h"
#include "src/camera/camera.h"
#include "src/bluetooth/ble_manager.h"

// Audio

#ifdef CODEC_OPUS
#include <opus.h>
OpusEncoder *opus_encoder = nullptr;
#endif

// State variables
// Note: BLE connection state is now managed by BLE manager

// Camera state variables are now defined in camera_simple.cpp




// Forward declarations for control handlers
void handlePhotoControl(int8_t controlValue);
void handleVideoControl(uint8_t controlValue);

// Note: BLE callback classes are now in src/bluetooth/ble_callbacks.h

// Note: BLE configuration is now handled by the BLE manager
// See src/bluetooth/ble_manager.h for the new interface

// Camera functions are now defined in camera_simple.cpp

// handlePhotoControl function is now defined in camera_simple.cpp
// updateVideoStatus function is now in src/bluetooth/ble_characteristics.cpp

//
// Microphone
//

static uint8_t *s_recording_buffer = nullptr;
static uint8_t *s_compressed_frame = nullptr;
static uint8_t *s_compressed_frame_2 = nullptr;

void configure_microphone() {
  // start I2S at 16 kHz with 16-bits per sample
  I2S.setAllPins(-1, I2S_WS_PIN, I2S_SCK_PIN, -1, -1);
  if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1); // do nothing
  }

  // Allocate buffers
  s_recording_buffer = (uint8_t *) ps_calloc(RECORDING_BUFFER_SIZE, sizeof(uint8_t));
  s_compressed_frame = (uint8_t *) ps_calloc(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t));
  s_compressed_frame_2 = (uint8_t *) ps_calloc(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t));
  
  if (!s_recording_buffer || !s_compressed_frame || !s_compressed_frame_2) {
    Serial.println("Failed to allocate audio buffers!");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1); // do nothing
  }
}

size_t read_microphone() {
  size_t bytes_recorded = 0;
  esp_i2s::i2s_read(esp_i2s::I2S_NUM_0, s_recording_buffer, RECORDING_BUFFER_SIZE, &bytes_recorded, portMAX_DELAY);
  return bytes_recorded;
}

//
// Camera functions are now defined in camera_simple.cpp
//



//
// Main
//

// static uint8_t *s_compressed_frame_2 = nullptr;
// static size_t compressed_buffer_size = 400 + 3;
void setup() {
  Serial.begin(XIAO_ESP32S3_SERIAL_BAUD_RATE);
  Serial.println("OpenGlass starting up...");
  
  // Initialize LED manager first for early status indication
  initLedManager();
  
  // Print wake-up reason
  Serial.printf("Wake-up reason: %s\n", getWakeupReason());
  
  // Initialize power management
  initializePowerManagement();
  
  // Initialize charging manager
  initializeChargingManager();
  
  updateDeviceStatus(DEVICE_STATUS_BLE_INIT);
  configureBLE();
  Serial.println("BLE configured");
  
#ifdef CODEC_OPUS
  int opus_err;
  opus_encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION, &opus_err);
  if (opus_err != OPUS_OK || !opus_encoder)
  {
    Serial.println("Failed to create Opus encoder!");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1)
      ; // do nothing
  }
  opus_encoder_ctl(opus_encoder, OPUS_SET_BITRATE(OPUS_BITRATE));
  Serial.println("Opus encoder configured");
#endif
  
  updateDeviceStatus(DEVICE_STATUS_MICROPHONE_INIT);
  configure_microphone();
  Serial.println("Microphone configured");
  
  updateDeviceStatus(DEVICE_STATUS_CAMERA_INIT);
  configure_camera();
  Serial.println("Camera configured");
  
  // Test camera functionality
  Serial.println("Testing camera functionality...");
  if (take_photo()) {
    Serial.printf("✅ Camera test successful! Photo size: %d bytes\n", fb->len);
    if (fb) {
      esp_camera_fb_return(fb);
      fb = nullptr;
    }
  } else {
    Serial.println("❌ Camera test failed - device will be in ERROR state");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    // Continue with setup but device will remain in error state
  }
  
  // Check battery presence
  Serial.println("Checking battery connection...");
  if (!checkBatteryPresence()) {
    Serial.println("WARNING: No lithium battery detected!");
    updateDeviceStatus(DEVICE_STATUS_BATTERY_NOT_DETECTED);
    delay(TIMING_LONG); // Give time to see the warning
  } else {
    Serial.println("Battery detected and connected");
  }
  
  updateDeviceStatus(DEVICE_STATUS_WARMING_UP);
  Serial.println("Device warming up...");
  delay(TIMING_LONG); // Give camera and microphone time to stabilize
  
  deviceReady = true;
  updateDeviceStatus(DEVICE_STATUS_READY);
  Serial.println("OpenGlass ready!");
}

void loop() {
  // Update LED first (always, even if device not ready)
  updateLed();
  
  // Only proceed with normal operations if device is ready
  if (!deviceReady) {
    delay(MAIN_LOOP_DELAY);
    return;
  }

  // Read from mic
  size_t bytes_recorded = read_microphone();

  // Push audio to BLE using BLE manager
  if (bytes_recorded > 0 && isConnected()) {
    BLEManager::transmitAudio(s_recording_buffer, RECORDING_BUFFER_SIZE, bytes_recorded);
  }

  // Take a photo using timing utilities
  // Don't take a photo if we are already sending data for previous photo
  if (isCapturingPhotos && !photoDataUploading && isConnected())
  {
    bool shouldCapture = false;
    
    if (captureInterval == 0) {
      // Single photo requested - capture immediately
      shouldCapture = true;
    } else {
      // Interval capture - check if enough time has passed
      shouldCapture = hasTimedOut(lastCaptureTime, captureInterval);
    }
    
    if (shouldCapture) {
      if (captureInterval == 0) {
        // Single photo requested
        isCapturingPhotos = false;
      }

      // Take the photo
      if (take_photo())
      {
        photoDataUploading = true;
        sent_photo_bytes = 0;
        sent_photo_frames = 0;
        lastCaptureTime = millis();
        isStreamingFrame = false;
      }
    }
  }

  // Video streaming logic
  if (isStreamingVideo && !photoDataUploading && isConnected()) {
    if (!shouldDropFrame()) {
      unsigned long frameInterval = VIDEO_STREAM_FRAME_INTERVAL(streamingFPS);
      
      if (hasTimedOut(lastStreamFrame, frameInterval)) {
        if (take_photo()) {
          photoDataUploading = true;
          sent_photo_bytes = 0;
          sent_photo_frames = 0;
          lastStreamFrame = millis();
          isStreamingFrame = true;
          totalStreamingFrames++;
        }
      }
    }
  }

  // Push photo/video data to BLE
  if (photoDataUploading && fb && isConnected()) {
    size_t remaining = fb->len - sent_photo_bytes;
    if (remaining > 0) {
      // Populate buffer with frame type header
      s_compressed_frame_2[0] = sent_photo_frames & 0xFF;
      s_compressed_frame_2[1] = (sent_photo_frames >> 8) & 0xFF;
      s_compressed_frame_2[2] = isStreamingFrame ? 0x02 : 0x01; // Frame type: 0x02 = streaming, 0x01 = single photo
      size_t bytes_to_copy = remaining;
      if (bytes_to_copy > PHOTO_CHUNK_SIZE - 1) { // Account for frame type byte
        bytes_to_copy = PHOTO_CHUNK_SIZE - 1;
      }
      
      // Bounds check
      if (sent_photo_bytes + bytes_to_copy <= fb->len) {
        memcpy(&s_compressed_frame_2[3], &fb->buf[sent_photo_bytes], bytes_to_copy);

        // Push to appropriate BLE characteristic using BLE manager
        if (isStreamingFrame) {
          BLEManager::transmitVideo(s_compressed_frame_2, bytes_to_copy + 3, sent_photo_frames);
        } else {
          BLEManager::transmitPhoto(s_compressed_frame_2, bytes_to_copy + 3, sent_photo_frames);
        }
        sent_photo_bytes += bytes_to_copy;
        sent_photo_frames++;
      } else {
        Serial.println("Photo/video upload bounds error, stopping");
        photoDataUploading = false;
      }
    } else {
      // End flag using BLE manager
      if (isStreamingFrame) {
        BLEManager::transmitVideoEnd();
        Serial.printf("Video frame completed: %d bytes in %d frames\n", sent_photo_bytes, sent_photo_frames);
      } else {
        BLEManager::transmitPhotoEnd();
        Serial.printf("Photo upload completed: %d bytes in %d frames\n", sent_photo_bytes, sent_photo_frames);
      }
      photoDataUploading = false;
    }
  } else if (photoDataUploading && !isConnected()) {
    // Connection lost during upload
    Serial.println("Connection lost during photo/video upload, stopping");
    photoDataUploading = false;
  }

  // Update battery and charging status using timing utilities
  if (shouldExecute(&lastBatteryUpdate, BATTERY_UPDATE_INTERVAL))
  {
    updateBatteryLevel();
    
    // Update advanced charging status
    updateChargingStatus();
    
    // Update power statistics
    float batteryVoltage = readBatteryVoltage();
    bool cameraActive = isCapturingPhotos || photoDataUploading;
    updatePowerStats(batteryVoltage, false, isConnected(), cameraActive);
    
    // Optimize power based on battery level and charging state
    optimizePowerForBattery(batteryLevel, chargingStats.state != CHARGING_STATE_NOT_CHARGING);
    
    // Check if battery status changed
    if (!batteryDetected && deviceReady) {
      Serial.println("Battery disconnected during operation!");
      updateDeviceStatus(DEVICE_STATUS_BATTERY_NOT_DETECTED);
    } else if (batteryDetected && deviceStatus == DEVICE_STATUS_BATTERY_NOT_DETECTED) {
      Serial.println("Battery reconnected!");
      updateDeviceStatus(DEVICE_STATUS_READY);
    }
    
    // Check for unstable battery connection
    if (batteryDetected && !connectionStable && deviceReady) {
      Serial.println("⚠️  Unstable battery connection detected!");
      updateDeviceStatus(DEVICE_STATUS_BATTERY_UNSTABLE);
    } else if (batteryDetected && connectionStable && deviceStatus == DEVICE_STATUS_BATTERY_UNSTABLE) {
      Serial.println("✅ Battery connection stabilized");
      updateDeviceStatus(DEVICE_STATUS_READY);
    }
    
    // Check charging status and update device status accordingly
    if (isCharging && deviceReady && deviceStatus != DEVICE_STATUS_CHARGING) {
      Serial.println("Device is now charging!");
      updateDeviceStatus(DEVICE_STATUS_CHARGING);
    } else if (!isCharging && deviceStatus == DEVICE_STATUS_CHARGING) {
      Serial.println("Device is no longer charging");
      updateDeviceStatus(DEVICE_STATUS_READY);
    }
    
    // Print power statistics periodically (every 10 battery updates)
    static int powerStatsPrintCounter = 0;
    if (++powerStatsPrintCounter >= 10) {
      printPowerStats();
      powerStatsPrintCounter = 0;
    }
  }

  // Power management - check for idle state
  static unsigned long lastActivityTime = millis();
  
  // Update activity time if there's active operation
  if (isConnected() || isCapturingPhotos || photoDataUploading || isStreamingVideo) {
    lastActivityTime = millis();
  }
  
  // Check if we should enter power saving mode
  unsigned long idleTime = millis() - lastActivityTime;
  if (shouldEnterPowerSaving(batteryLevel, idleTime)) {
    // Enter light sleep for a short period if idle
    if (idleTime > POWER_IDLE_TIMEOUT_MS && !isConnected()) {
      Serial.println("Device idle, entering light sleep...");
      prepareForSleep();
      enterLightSleep(1000); // Sleep for 1 second
      wakeFromSleep();
    }
  }
  
  // Main loop delay using timing utilities
  static unsigned long mainLoopDelayTime = 0;
  if (nonBlockingDelayStateful(&mainLoopDelayTime, MAIN_LOOP_DELAY)) {
    // Delay completed, continue loop
  } else {
    // For main loop, we still use blocking delay to maintain timing
    delay(MAIN_LOOP_DELAY);
  }
}
