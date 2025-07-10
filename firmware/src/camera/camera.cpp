#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera.h"
#include "../utils/timing.h"
#include "../led/led_manager.h"
#include "../system/device_status.h"

// External reference to connection status
// Note: BLE connection state is now managed by BLE manager
#include "../bluetooth/ble_callbacks.h"

// Camera state variables (defined here, declared in header)
camera_fb_t *fb = nullptr;
bool isCapturingPhotos = false;
int captureInterval = 0;
unsigned long lastCaptureTime = 0;
size_t sent_photo_bytes = 0;
size_t sent_photo_frames = 0;
bool photoDataUploading = false;

// Video streaming state variables
bool isStreamingVideo = false;
int streamingFPS = VIDEO_STREAM_DEFAULT_FPS;
unsigned long lastStreamFrame = 0;
bool isStreamingFrame = false;
unsigned long streamingStartTime = 0;
size_t totalStreamingFrames = 0;
size_t droppedFrames = 0;
camera_mode_t currentCameraMode = CAMERA_MODE_IDLE;

bool take_photo() {
  // Release previous buffer if exists
  if (fb) {
    esp_camera_fb_return(fb);
    fb = nullptr;
  }

  // Flash LED to indicate photo capture
  setLedPattern(LED_PHOTO_CAPTURE);

  // Take a photo with retry logic using timing utilities
  int retries = 3;
  unsigned long captureStartTime = measureStart();
  
  while (retries > 0) {
    unsigned long attemptStart = measureStart();
    fb = esp_camera_fb_get();
    unsigned long attemptDuration = measureEnd(attemptStart);
    
    if (fb && fb->len > 0) {
      unsigned long totalDuration = measureEnd(captureStartTime);
      Serial.printf("Photo captured successfully, size: %d bytes (took %lu ms)\n", fb->len, totalDuration);
      return true;
    }
    
    if (fb) {
      esp_camera_fb_return(fb);
      fb = nullptr;
    }
    
    retries--;
    Serial.printf("Photo capture failed (attempt took %lu ms), retries left: %d\n", attemptDuration, retries);
    
    // Use timing utility for retry delay
    static unsigned long retryDelayTime = 0;
    if (nonBlockingDelayStateful(&retryDelayTime, TIMING_SHORT)) {
      // Delay completed, continue to next retry
    } else {
      // Still waiting, but for this synchronous function we'll use blocking delay
      delay(TIMING_SHORT);
    }
  }
  
  unsigned long totalDuration = measureEnd(captureStartTime);
  Serial.printf("Failed to get camera frame buffer after retries (total time: %lu ms)\n", totalDuration);
  return false;
}

void handlePhotoControl(int8_t controlValue)
{
  Serial.printf("Photo control command: %d\n", controlValue);
  
  // Check if device is ready before processing photo commands
  if (!deviceReady) {
    Serial.println("Device not ready, ignoring photo control command");
    return;
  }
  
  if (controlValue == PHOTO_SINGLE_SHOT)
  {
    // Take a single photo
    if (!photoDataUploading) {
      isCapturingPhotos = true;
      captureInterval = 0;
      Serial.println("Single photo capture requested");
    } else {
      Serial.println("Photo upload in progress, ignoring single photo request");
    }
  }
  else if (controlValue == PHOTO_STOP)
  {
    // Stop taking photos
    isCapturingPhotos = false;
    captureInterval = 0;
    Serial.println("Photo capture stopped");
  }
  else if (controlValue >= PHOTO_MIN_INTERVAL && controlValue <= PHOTO_MAX_INTERVAL)
  {
    // Start taking photos at specified interval
    if (!photoDataUploading) {
      captureInterval = (controlValue / PHOTO_MIN_INTERVAL) * (PHOTO_MIN_INTERVAL * 1000); // Round to nearest 5 seconds and convert to milliseconds
      isCapturingPhotos = true;
      lastCaptureTime = millis() - captureInterval;
      Serial.printf("Interval photo capture started: %d seconds\n", captureInterval / 1000);
    } else {
      Serial.println("Photo upload in progress, ignoring interval photo request");
    }
  }
  else
  {
    Serial.printf("Invalid photo control value: %d\n", controlValue);
  }
}

void configure_camera() {
  Serial.println("=== Camera Configuration Debug ===");
  
  // Print pin configuration
  Serial.printf("Camera Pin Configuration:\n");
  Serial.printf("  PWDN: %d, RESET: %d, XCLK: %d\n", PWDN_GPIO_NUM, RESET_GPIO_NUM, XCLK_GPIO_NUM);
  Serial.printf("  SIOD: %d, SIOC: %d\n", SIOD_GPIO_NUM, SIOC_GPIO_NUM);
  Serial.printf("  Y9-Y2: %d,%d,%d,%d,%d,%d,%d,%d\n", Y9_GPIO_NUM, Y8_GPIO_NUM, Y7_GPIO_NUM, Y6_GPIO_NUM, Y5_GPIO_NUM, Y4_GPIO_NUM, Y3_GPIO_NUM, Y2_GPIO_NUM);
  Serial.printf("  VSYNC: %d, HREF: %d, PCLK: %d\n", VSYNC_GPIO_NUM, HREF_GPIO_NUM, PCLK_GPIO_NUM);
  
  // Check PSRAM availability
  bool psram_available = psramFound();
  if (psram_available) {
    Serial.printf("  PSRAM: Available (%d bytes)\n", ESP.getPsramSize());
  } else {
    Serial.println("  PSRAM: Not found - using DRAM only");
  }
  
  // Try multiple configurations in order of preference
  CameraConfig configs[] = {
    {FRAMESIZE_QVGA, 15, CAMERA_FB_IN_PSRAM, 20000000, "QVGA + PSRAM"},
    {FRAMESIZE_QQVGA, 20, CAMERA_FB_IN_PSRAM, 20000000, "QQVGA + PSRAM"},
    {FRAMESIZE_QQVGA, 25, CAMERA_FB_IN_DRAM, 20000000, "QQVGA + DRAM"},
    {FRAMESIZE_96X96, 30, CAMERA_FB_IN_DRAM, 10000000, "96x96 + DRAM (minimal)"},
  };
  
  int num_configs = sizeof(configs) / sizeof(configs[0]);
  bool camera_initialized = false;
  
  for (int i = 0; i < num_configs; i++) {
    CameraConfig& config = configs[i];
    
    // Skip PSRAM configs if PSRAM not available
    if (!psram_available && config.fb_location == CAMERA_FB_IN_PSRAM) {
      Serial.printf("Skipping %s (PSRAM not available)\n", config.description);
      continue;
    }
    
    Serial.printf("Trying configuration: %s\n", config.description);
    
    if (initCameraWithConfig(config)) {
      Serial.printf("✅ Camera initialized successfully with: %s\n", config.description);
      camera_initialized = true;
      break;
    } else {
      Serial.printf("❌ Failed with: %s\n", config.description);
    }
  }
  
  if (!camera_initialized) {
    Serial.println("❌ All camera configurations failed!");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    return;
  }
  
  // Test camera sensor
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    Serial.printf("Camera sensor detected: PID=0x%02X\n", s->id.PID);
    Serial.println("Camera configuration completed successfully");
  } else {
    Serial.println("⚠️  Camera sensor not accessible after init");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    return;
  }
  
  Serial.println("=== Camera Configuration Complete ===");
}

bool initCameraWithConfig(const CameraConfig& config) {
  camera_config_t cam_config;
  cam_config.ledc_channel = LEDC_CHANNEL_0;
  cam_config.ledc_timer = LEDC_TIMER_0;
  cam_config.pin_d0 = Y2_GPIO_NUM;
  cam_config.pin_d1 = Y3_GPIO_NUM;
  cam_config.pin_d2 = Y4_GPIO_NUM;
  cam_config.pin_d3 = Y5_GPIO_NUM;
  cam_config.pin_d4 = Y6_GPIO_NUM;
  cam_config.pin_d5 = Y7_GPIO_NUM;
  cam_config.pin_d6 = Y8_GPIO_NUM;
  cam_config.pin_d7 = Y9_GPIO_NUM;
  cam_config.pin_xclk = XCLK_GPIO_NUM;
  cam_config.pin_pclk = PCLK_GPIO_NUM;
  cam_config.pin_vsync = VSYNC_GPIO_NUM;
  cam_config.pin_href = HREF_GPIO_NUM;
  cam_config.pin_sscb_sda = SIOD_GPIO_NUM;
  cam_config.pin_sscb_scl = SIOC_GPIO_NUM;
  cam_config.pin_pwdn = PWDN_GPIO_NUM;
  cam_config.pin_reset = RESET_GPIO_NUM;
  cam_config.xclk_freq_hz = config.xclk_freq_hz;
  cam_config.frame_size = config.frame_size;
  cam_config.pixel_format = PIXFORMAT_JPEG;
  cam_config.fb_count = 1;
  cam_config.jpeg_quality = config.jpeg_quality;
  cam_config.fb_location = config.fb_location;
  cam_config.grab_mode = CAMERA_GRAB_LATEST;
  
  Serial.printf("  Frame size: %d, Quality: %d, FB location: %s, XCLK: %d Hz\n", 
                config.frame_size, config.jpeg_quality, 
                config.fb_location == CAMERA_FB_IN_PSRAM ? "PSRAM" : "DRAM",
                config.xclk_freq_hz);
  
  esp_err_t err = esp_camera_init(&cam_config);
  if (err != ESP_OK) {
    Serial.printf("  Camera init failed: 0x%x (%s)\n", err, esp_err_to_name(err));
    return false;
  }
  
  return true;
}

// Video streaming functions
void handleVideoControl(uint8_t controlValue) {
  Serial.printf("Video control command: %d\n", controlValue);
  
  if (!deviceReady) {
    Serial.println("Device not ready, ignoring video control command");
    return;
  }
  
  if (controlValue == VIDEO_STREAM_START) {
    startVideoStreaming();
  } else if (controlValue == VIDEO_STREAM_STOP) {
    stopVideoStreaming();
  } else if (controlValue >= VIDEO_STREAM_FPS_MIN && controlValue <= VIDEO_STREAM_FPS_MAX) {
    setVideoFPS(controlValue);
  } else {
    Serial.printf("Invalid video control value: %d\n", controlValue);
  }
}

void startVideoStreaming() {
  if (!photoDataUploading && !isStreamingVideo) {
    isStreamingVideo = true;
    currentCameraMode = CAMERA_MODE_VIDEO_STREAMING;
    streamingFPS = VIDEO_STREAM_DEFAULT_FPS;
    lastStreamFrame = millis();
    streamingStartTime = millis();
    totalStreamingFrames = 0;
    droppedFrames = 0;
    configure_camera_for_streaming();
    setLedPattern(LED_STREAMING);
    Serial.println("Video streaming started");
    updateVideoStatus();
  } else {
    Serial.println("Cannot start video streaming - photo upload in progress or already streaming");
  }
}

void stopVideoStreaming() {
  isStreamingVideo = false;
  currentCameraMode = CAMERA_MODE_IDLE;
  configure_camera_for_photo();
  // Return to connection status LED
  if (bleConnected) {
    setLedPattern(LED_CONNECTED);
  } else {
    setLedPattern(LED_DISCONNECTED);
  }
  Serial.println("Video streaming stopped");
  updateVideoStatus();
}

void setVideoFPS(uint8_t fps) {
  if (fps >= VIDEO_STREAM_FPS_MIN && fps <= VIDEO_STREAM_FPS_MAX) {
    streamingFPS = fps;
    Serial.printf("Video streaming FPS set to: %d\n", streamingFPS);
    updateVideoStatus();
  } else {
    Serial.printf("Invalid FPS value: %d\n", fps);
  }
}

void configure_camera_for_streaming() {
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_quality(s, CAMERA_STREAMING_QUALITY); // Use streaming-specific quality
    s->set_framesize(s, CAMERA_STREAMING_FRAME_SIZE); // Use streaming-specific frame size
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    Serial.printf("Camera configured for streaming: %dx%d, quality %d\n", 
                  CAMERA_STREAMING_FRAME_SIZE == FRAMESIZE_QQVGA ? 160 : 320,
                  CAMERA_STREAMING_FRAME_SIZE == FRAMESIZE_QQVGA ? 120 : 240,
                  CAMERA_STREAMING_QUALITY);
  }
}

void configure_camera_for_photo() {
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_quality(s, CAMERA_JPEG_QUALITY);
    s->set_framesize(s, CAMERA_FRAME_SIZE_LOW);
    Serial.println("Camera configured for photo");
  }
}

bool shouldDropFrame() {
  // Drop frames more aggressively during streaming to maintain performance
  if (photoDataUploading && isStreamingVideo) {
    droppedFrames++;
    Serial.printf("Dropping frame #%d (upload in progress)\n", droppedFrames);
    return true;
  }
  
  // Also drop frames if we're behind on the streaming schedule
  if (isStreamingVideo && totalStreamingFrames > 0) {
    unsigned long expectedFrames = (millis() - streamingStartTime) / VIDEO_STREAM_FRAME_INTERVAL(streamingFPS);
    if (totalStreamingFrames < expectedFrames * 0.5) { // If we're less than 50% of expected rate
      droppedFrames++;
      Serial.printf("Dropping frame #%d (behind schedule: %d/%d)\n", droppedFrames, totalStreamingFrames, expectedFrames);
      return true;
    }
  }
  
  return false;
}

 
 