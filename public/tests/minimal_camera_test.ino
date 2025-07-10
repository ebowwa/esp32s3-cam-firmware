#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE

#include "esp_camera.h"
#include "camera_pins.h"

// Camera pin definitions for XIAO ESP32S3 Sense
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13

void setup() {
  Serial.begin(115200);
  Serial.println("=== XIAO ESP32S3 Camera Test ===");
  
  // Wait for serial connection
  delay(2000);
  
  // Print system information
  Serial.printf("ESP32 Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("ESP32 Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("ESP32 CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("ESP32 Flash Size: %d bytes\n", ESP.getFlashChipSize());
  
  // Check PSRAM
  if (psramFound()) {
    Serial.printf("PSRAM: Available (%d bytes)\n", ESP.getPsramSize());
  } else {
    Serial.println("PSRAM: Not found");
  }
  
  // Print pin configuration
  Serial.println("\nCamera Pin Configuration:");
  Serial.printf("  PWDN: %d, RESET: %d, XCLK: %d\n", PWDN_GPIO_NUM, RESET_GPIO_NUM, XCLK_GPIO_NUM);
  Serial.printf("  SIOD: %d, SIOC: %d\n", SIOD_GPIO_NUM, SIOC_GPIO_NUM);
  Serial.printf("  Y9-Y2: %d,%d,%d,%d,%d,%d,%d,%d\n", Y9_GPIO_NUM, Y8_GPIO_NUM, Y7_GPIO_NUM, Y6_GPIO_NUM, Y5_GPIO_NUM, Y4_GPIO_NUM, Y3_GPIO_NUM, Y2_GPIO_NUM);
  Serial.printf("  VSYNC: %d, HREF: %d, PCLK: %d\n", VSYNC_GPIO_NUM, HREF_GPIO_NUM, PCLK_GPIO_NUM);
  
  // Test different camera configurations
  testCameraConfiguration();
}

void testCameraConfiguration() {
  Serial.println("\n=== Testing Camera Configurations ===");
  
  // Configuration 1: Basic QVGA with PSRAM
  Serial.println("\n1. Testing QVGA + PSRAM configuration...");
  if (initCamera(FRAMESIZE_QVGA, 15, CAMERA_FB_IN_PSRAM)) {
    Serial.println("✅ QVGA + PSRAM: SUCCESS");
    testPhotoCapture();
    esp_camera_deinit();
  } else {
    Serial.println("❌ QVGA + PSRAM: FAILED");
  }
  
  // Configuration 2: Smaller QQVGA with PSRAM
  Serial.println("\n2. Testing QQVGA + PSRAM configuration...");
  if (initCamera(FRAMESIZE_QQVGA, 20, CAMERA_FB_IN_PSRAM)) {
    Serial.println("✅ QQVGA + PSRAM: SUCCESS");
    testPhotoCapture();
    esp_camera_deinit();
  } else {
    Serial.println("❌ QQVGA + PSRAM: FAILED");
  }
  
  // Configuration 3: QQVGA with DRAM
  Serial.println("\n3. Testing QQVGA + DRAM configuration...");
  if (initCamera(FRAMESIZE_QQVGA, 25, CAMERA_FB_IN_DRAM)) {
    Serial.println("✅ QQVGA + DRAM: SUCCESS");
    testPhotoCapture();
    esp_camera_deinit();
  } else {
    Serial.println("❌ QQVGA + DRAM: FAILED");
  }
  
  // Configuration 4: Minimal configuration
  Serial.println("\n4. Testing minimal configuration...");
  if (initCameraMinimal()) {
    Serial.println("✅ Minimal config: SUCCESS");
    testPhotoCapture();
    esp_camera_deinit();
  } else {
    Serial.println("❌ Minimal config: FAILED");
  }
  
  Serial.println("\n=== Camera Test Complete ===");
}

bool initCamera(framesize_t framesize, int quality, camera_fb_location_t fb_location) {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = framesize;
  config.jpeg_quality = quality;
  config.fb_count = 1;
  config.fb_location = fb_location;
  config.grab_mode = CAMERA_GRAB_LATEST;
  
  Serial.printf("  Frame size: %d, Quality: %d, FB location: %s\n", 
                framesize, quality, fb_location == CAMERA_FB_IN_PSRAM ? "PSRAM" : "DRAM");
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("  Camera init failed: 0x%x (%s)\n", err, esp_err_to_name(err));
    return false;
  }
  
  // Test sensor access
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    Serial.printf("  Sensor detected: PID=0x%02X\n", s->id.PID);
    return true;
  } else {
    Serial.println("  Sensor not accessible");
    esp_camera_deinit();
    return false;
  }
}

bool initCameraMinimal() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;  // Lower frequency
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_96X96;  // Smallest possible
  config.jpeg_quality = 30;  // Lower quality
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_DRAM;  // Force DRAM
  config.grab_mode = CAMERA_GRAB_LATEST;
  
  Serial.println("  Minimal: 96x96, Quality 30, DRAM, 10MHz XCLK");
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("  Minimal init failed: 0x%x (%s)\n", err, esp_err_to_name(err));
    return false;
  }
  
  return true;
}

void testPhotoCapture() {
  Serial.println("  Testing photo capture...");
  
  camera_fb_t *fb = esp_camera_fb_get();
  if (fb) {
    Serial.printf("  ✅ Photo captured: %d bytes\n", fb->len);
    esp_camera_fb_return(fb);
  } else {
    Serial.println("  ❌ Photo capture failed");
  }
}

void loop() {
  delay(10000);
  Serial.println("Test complete - check results above");
} 