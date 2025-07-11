// Simple compilation test for the organized firmware structure
// This will test that all functions and variables are properly linked

#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE

#include "../src/hal/camera_pins.h"
#include "../src/hal/constants.h"
#include "../src/system/battery/battery_code.h"
#include "../src/status/device_status.h"
#include "../src/utils/mulaw.h"

void setup() {
  Serial.begin(XIAO_ESP32S3_SERIAL_BAUD_RATE);
  Serial.println("Testing organized firmware structure...");
  
  // Test device status functions
  updateDeviceStatus(DEVICE_STATUS_INITIALIZING);
  Serial.print("Device Status: "); Serial.println(deviceStatus);
  Serial.print("Device Ready: "); Serial.println(deviceReady ? "Yes" : "No");
  
  // Test battery functions
  float voltage = readBatteryVoltage();
  Serial.print("Battery Voltage: "); Serial.print(voltage); Serial.println("V");
  
  bool batteryPresent = checkBatteryPresence();
  Serial.print("Battery Present: "); Serial.println(batteryPresent ? "Yes" : "No");
  Serial.print("Battery Detected: "); Serial.println(batteryDetected ? "Yes" : "No");
  Serial.print("Connection Stable: "); Serial.println(connectionStable ? "Yes" : "No");
  Serial.print("Is Charging: "); Serial.println(isCharging ? "Yes" : "No");
  
  // Test constants
  Serial.print("Battery ADC Pin: "); Serial.println(BATTERY_ADC_PIN);
  Serial.print("USB Power ADC Pin: "); Serial.println(USB_POWER_ADC_PIN);
  Serial.print("Camera XCLK Pin: "); Serial.println(XCLK_GPIO_NUM);
  
  Serial.println("✅ All functions and variables are accessible!");
  Serial.println("The organized structure is working correctly.");
}

void loop() {
  // Test periodic battery update
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  if (now - lastUpdate > 5000) {  // Every 5 seconds
    updateBatteryLevel();
    Serial.print("Battery Level: "); Serial.print(batteryLevel); Serial.println("%");
    Serial.print("Last Battery Update: "); Serial.println(lastBatteryUpdate);
    lastUpdate = now;
  }
  
  delay(100);
} 