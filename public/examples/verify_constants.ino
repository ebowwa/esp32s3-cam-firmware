// Simple verification sketch for XIAO ESP32-S3 constants
// This sketch will compile successfully if all constants are properly defined

#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE

#include "../src/hardware/xiao_esp32s3_constants.h"

void setup() {
  Serial.begin(XIAO_ESP32S3_SERIAL_BAUD_RATE);
  Serial.println("XIAO ESP32-S3 Constants Verification");
  Serial.println("====================================");
  
  // Test chip information
  Serial.println("Chip Information:");
  Serial.print("  Model: "); Serial.println(XIAO_ESP32S3_CHIP_MODEL);
  Serial.print("  Architecture: "); Serial.println(XIAO_ESP32S3_ARCHITECTURE);
  Serial.print("  Cores: "); Serial.println(XIAO_ESP32S3_CORES);
  Serial.print("  Max Frequency: "); Serial.print(XIAO_ESP32S3_MAX_FREQ_MHZ); Serial.println(" MHz");
  
  // Test memory configuration
  Serial.println("\nMemory Configuration:");
  Serial.print("  Flash Size: "); Serial.print(XIAO_ESP32S3_FLASH_SIZE_MB); Serial.println(" MB");
  Serial.print("  PSRAM Size: "); Serial.print(XIAO_ESP32S3_PSRAM_SIZE_MB); Serial.println(" MB");
  Serial.print("  SRAM Size: "); Serial.print(XIAO_ESP32S3_SRAM_SIZE_KB); Serial.println(" KB");
  
  // Test GPIO pins
  Serial.println("\nGPIO Pin Definitions:");
  Serial.print("  A0 (Battery): GPIO"); Serial.println(XIAO_ESP32S3_PIN_A0);
  Serial.print("  A1 (USB Power): GPIO"); Serial.println(XIAO_ESP32S3_PIN_A1);
  Serial.print("  User LED: GPIO"); Serial.println(XIAO_ESP32S3_USER_LED_PIN);
  Serial.print("  I2S WS: GPIO"); Serial.println(XIAO_ESP32S3_SENSE_PIN_D11);
  Serial.print("  I2S SCK: GPIO"); Serial.println(XIAO_ESP32S3_SENSE_PIN_D12);
  
  // Test camera pins
  Serial.println("\nCamera Pin Definitions:");
  Serial.print("  XCLK: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_XCLK_PIN);
  Serial.print("  SIOD: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_SIOD_PIN);
  Serial.print("  SIOC: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_SIOC_PIN);
  Serial.print("  Y9: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y9_PIN);
  Serial.print("  Y8: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y8_PIN);
  Serial.print("  Y7: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y7_PIN);
  Serial.print("  Y6: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y6_PIN);
  Serial.print("  Y5: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y5_PIN);
  Serial.print("  Y4: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y4_PIN);
  Serial.print("  Y3: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y3_PIN);
  Serial.print("  Y2: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_Y2_PIN);
  Serial.print("  VSYNC: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_VSYNC_PIN);
  Serial.print("  HREF: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_HREF_PIN);
  Serial.print("  PCLK: GPIO"); Serial.println(XIAO_ESP32S3_CAMERA_PCLK_PIN);
  
  // Test ADC configuration
  Serial.println("\nADC Configuration:");
  Serial.print("  Resolution: "); Serial.print(XIAO_ESP32S3_ADC_RESOLUTION_BITS); Serial.println(" bits");
  Serial.print("  Max Value: "); Serial.println(XIAO_ESP32S3_ADC_MAX_VALUE);
  Serial.print("  Reference Voltage: "); Serial.print(XIAO_ESP32S3_ADC_VREF_MV); Serial.println(" mV");
  
  // Test power specifications
  Serial.println("\nPower Specifications:");
  Serial.print("  Operating Voltage: "); Serial.print(XIAO_ESP32S3_OPERATING_VOLTAGE_V); Serial.println(" V");
  Serial.print("  USB Input Voltage: "); Serial.print(XIAO_ESP32S3_USB_INPUT_VOLTAGE_V); Serial.println(" V");
  Serial.print("  Battery Voltage: "); Serial.print(XIAO_ESP32S3_BATTERY_VOLTAGE_V); Serial.println(" V");
  
  // Test Sense version features
  Serial.println("\nSense Version Features:");
  Serial.print("  Has Camera: "); Serial.println(XIAO_ESP32S3_HAS_CAMERA ? "Yes" : "No");
  Serial.print("  Has Microphone: "); Serial.println(XIAO_ESP32S3_HAS_MICROPHONE ? "Yes" : "No");
  Serial.print("  Has SD Card: "); Serial.println(XIAO_ESP32S3_HAS_SD_CARD ? "Yes" : "No");
  
  Serial.println("\n✅ All constants are properly defined!");
  Serial.println("The XIAO ESP32-S3 constants are ready for use.");
}

void loop() {
  // Blink the user LED to show the sketch is running
  digitalWrite(XIAO_ESP32S3_USER_LED_PIN, HIGH);
  delay(500);
  digitalWrite(XIAO_ESP32S3_USER_LED_PIN, LOW);
  delay(500);
} 