#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE  // Enable Sense version constants
#include <I2S.h>
#include "esp_camera.h"
#include "src/platform/camera_pins.h"
#include "src/utils/mulaw.h"
#include "src/utils/timing.h"
#include "src/utils/power_management.h"
#include "src/utils/memory_utils.h"
#include "src/utils/cycle_manager.h"
#include "src/led/led_manager.h"
#include "src/system/battery_code.h"
#include "src/system/charging_manager.h"
#include "src/platform/constants.h"
#include "src/system/device_status.h"
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

uint8_t *s_recording_buffer = nullptr;
static uint8_t *s_compressed_frame = nullptr;
uint8_t *s_compressed_frame_2 = nullptr;

void configure_microphone() {
  // start I2S at 16 kHz with 16-bits per sample
  I2S.setAllPins(-1, I2S_WS_PIN, I2S_SCK_PIN, -1, -1);
  if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1); // do nothing
  }

  // Allocate buffers with tracking
  s_recording_buffer = (uint8_t *) PS_CALLOC_TRACKED(RECORDING_BUFFER_SIZE, sizeof(uint8_t), "AudioRecording");
  s_compressed_frame = (uint8_t *) PS_CALLOC_TRACKED(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t), "AudioCompressed");
  s_compressed_frame_2 = (uint8_t *) PS_CALLOC_TRACKED(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t), "AudioCompressed2");
  
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
  
  // Initialize memory manager
  initializeMemoryManager();
  
  // Initialize centralized cycle manager
  initializeCycleManager();
  
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
  
  // Initialize all specialized cycle managers
  ChargingCycles::initialize();
  LEDCycles::initialize();
  PowerCycles::initialize();
  DataCycles::initialize();
  CommCycles::initialize();
  
  Serial.println("All cycle managers initialized");
}

void loop() {
  // Update all cycles using centralized cycle manager
  updateCycles();
  
  // Handle connection loss during photo/video upload
  if (photoDataUploading && !isConnected()) {
    Serial.println("Connection lost during photo/video upload, stopping");
    photoDataUploading = false;
  }
  
  // Small delay to prevent excessive CPU usage
  delay(MAIN_LOOP_DELAY);
}
