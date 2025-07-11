// Test sketch for Dual LED manager functionality
// This demonstrates all LED patterns with dual LED support

#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE

// Uncomment the next line to enable external RGB LED support
// #define RGB_LED_ENABLED

#include "../src/features/led/led_manager.h"
#include "../src/hal/xiao_esp32s3_constants.h"

void setup() {
  Serial.begin(XIAO_ESP32S3_SERIAL_BAUD_RATE);
  Serial.println("Testing Dual LED Manager...");
  Serial.println("===========================");
  
  Serial.println("XIAO ESP32-S3 Dual LED Configuration:");
  Serial.println("- User LED: GPIO21 (programmable)");
  Serial.println("- Charge LED: Hardware controlled (charging status)");
  
#ifdef RGB_LED_ENABLED
  Serial.println("- External RGB LED: GPIO" + String(RGB_LED_PIN) + " (enhanced mode)");
  Serial.println("Mode: DUAL_LED_MODE_RGB_ENHANCED");
#else
  Serial.println("Mode: DUAL_LED_MODE_DUAL_BASIC");
#endif
  
  // Initialize dual LED manager
  initLedManager();
  
  // Test all patterns with dual colors
  testAllDualPatterns();
  
  // Test device status patterns
  testDeviceStatusPatterns();
  
  // Test convenience functions
  testConvenienceFunctions();
}

void loop() {
  // Update LED in main loop
  updateLed();
  
  // Test pattern cycling every 4 seconds with dual colors
  static unsigned long lastPatternChange = 0;
  static int currentPattern = 0;
  static int currentColorSet = 0;
  
  if (millis() - lastPatternChange > 4000) {
    cycleDualPatternsWithColors(currentPattern, currentColorSet);
    currentPattern = (currentPattern + 1) % 18; // 18 total patterns
    if (currentPattern == 0) {
      currentColorSet = (currentColorSet + 1) % 6; // 6 color combinations
    }
    lastPatternChange = millis();
  }
}

void testAllDualPatterns() {
  Serial.println("\n=== Testing All Dual LED Patterns ===");
  
  rgb_color_t primaryColors[] = {
    (rgb_color_t)LED_COLOR_RED,
    (rgb_color_t)LED_COLOR_GREEN,
    (rgb_color_t)LED_COLOR_BLUE,
    (rgb_color_t)LED_COLOR_YELLOW,
    (rgb_color_t)LED_COLOR_PURPLE,
    (rgb_color_t)LED_COLOR_CYAN
  };
  
  rgb_color_t secondaryColors[] = {
    (rgb_color_t)LED_COLOR_MAROON,
    (rgb_color_t)LED_COLOR_LIME,
    (rgb_color_t)LED_COLOR_NAVY,
    (rgb_color_t)LED_COLOR_GOLD,
    (rgb_color_t)LED_COLOR_VIOLET,
    (rgb_color_t)LED_COLOR_TEAL
  };
  
  led_pattern_t patterns[] = {
    LED_OFF, LED_ON, LED_BLINK_SLOW, LED_BLINK_FAST, LED_BLINK_VERY_FAST,
    LED_PULSE, LED_HEARTBEAT, LED_SOS, LED_STARTUP, LED_DUAL_INDICATION
  };
  
  String patternNames[] = {
    "OFF", "ON", "BLINK_SLOW", "BLINK_FAST", "BLINK_VERY_FAST",
    "PULSE", "HEARTBEAT", "SOS", "STARTUP", "DUAL_INDICATION"
  };
  
  for (int i = 0; i < 10; i++) {
    Serial.print("Testing pattern: ");
    Serial.println(patternNames[i]);
    
    setLedPattern(patterns[i], primaryColors[i % 6], secondaryColors[i % 6]);
    
    // Let pattern run for 2 seconds
    unsigned long start = millis();
    while (millis() - start < 2000) {
      updateLed();
      delay(10);
    }
    
    // Brief pause between patterns
    setLedPattern(LED_OFF);
    delay(500);
  }
}

void testDeviceStatusPatterns() {
  Serial.println("\n=== Testing Device Status Patterns ===");
  
  uint8_t statusCodes[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
  String statusNames[] = {
    "INITIALIZING", "WARMING_UP", "READY", "ERROR", "CAMERA_INIT",
    "MICROPHONE_INIT", "BLE_INIT", "BATTERY_NOT_DETECTED", "CHARGING", "BATTERY_UNSTABLE"
  };
  
  for (int i = 0; i < 10; i++) {
    Serial.print("Testing device status: ");
    Serial.println(statusNames[i]);
    
    setLedForDeviceStatus(statusCodes[i]);
    
    // Let pattern run for 3 seconds
    unsigned long start = millis();
    while (millis() - start < 3000) {
      updateLed();
      delay(10);
    }
    
    // Brief pause between status patterns
    setLedPattern(LED_OFF);
    delay(500);
  }
}

void testConvenienceFunctions() {
  Serial.println("\n=== Testing Convenience Functions ===");
  
  Serial.println("Testing dual red pattern...");
  setLedPatternDualRed();
  delay(2000);
  
  Serial.println("Testing dual green pattern...");
  setLedPatternDualGreen();
  delay(2000);
  
  Serial.println("Testing dual blue pattern...");
  setLedPatternDualBlue();
  delay(2000);
  
  Serial.println("Testing streaming pattern...");
  setLedPatternStreaming();
  delay(2000);
  
  Serial.println("Testing photo capture flash...");
  for (int i = 0; i < 3; i++) {
    setLedPatternPhotoCapture();
    delay(1000);
  }
  
  Serial.println("Testing battery low pattern...");
  setLedPatternBatteryLow();
  delay(2000);
  
  Serial.println("Testing connected pattern...");
  setLedPatternConnected();
  delay(2000);
  
  Serial.println("Testing disconnected pattern...");
  setLedPatternDisconnected();
  delay(2000);
  
  setLedPattern(LED_OFF);
}

void cycleDualPatternsWithColors(int patternIndex, int colorSet) {
  rgb_color_t primaryColorSets[][2] = {
    {(rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_GREEN},
    {(rgb_color_t)LED_COLOR_BLUE, (rgb_color_t)LED_COLOR_YELLOW},
    {(rgb_color_t)LED_COLOR_PURPLE, (rgb_color_t)LED_COLOR_CYAN},
    {(rgb_color_t)LED_COLOR_ORANGE, (rgb_color_t)LED_COLOR_PINK},
    {(rgb_color_t)LED_COLOR_LIME, (rgb_color_t)LED_COLOR_INDIGO},
    {(rgb_color_t)LED_COLOR_GOLD, (rgb_color_t)LED_COLOR_SILVER}
  };
  
  rgb_color_t secondaryColorSets[][2] = {
    {(rgb_color_t)LED_COLOR_MAROON, (rgb_color_t)LED_COLOR_LIME},
    {(rgb_color_t)LED_COLOR_NAVY, (rgb_color_t)LED_COLOR_GOLD},
    {(rgb_color_t)LED_COLOR_VIOLET, (rgb_color_t)LED_COLOR_TEAL},
    {(rgb_color_t)LED_COLOR_OLIVE, (rgb_color_t)LED_COLOR_PINK},
    {(rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_PURPLE},
    {(rgb_color_t)LED_COLOR_YELLOW, (rgb_color_t)LED_COLOR_BLUE}
  };
  
  led_pattern_t patterns[] = {
    LED_OFF, LED_ON, LED_BLINK_SLOW, LED_BLINK_FAST, LED_BLINK_VERY_FAST,
    LED_PULSE, LED_HEARTBEAT, LED_SOS, LED_STARTUP, LED_DUAL_INDICATION,
    LED_BLINK_SLOW, LED_BLINK_FAST, LED_PULSE, LED_HEARTBEAT, 
    LED_DUAL_INDICATION, LED_PULSE, LED_BLINK_SLOW, LED_ON
  };
  
  String patternNames[] = {
    "OFF", "ON", "SLOW_BLINK", "FAST_BLINK", "VERY_FAST_BLINK",
    "PULSE", "HEARTBEAT", "SOS", "STARTUP", "DUAL_INDICATION",
    "SLOW_BLINK_2", "FAST_BLINK_2", "PULSE_2", "HEARTBEAT_2",
    "DUAL_INDICATION_2", "PULSE_3", "SLOW_BLINK_3", "ON_2"
  };
  
  rgb_color_t primary = primaryColorSets[colorSet][patternIndex % 2];
  rgb_color_t secondary = secondaryColorSets[colorSet][patternIndex % 2];
  
  Serial.print("Pattern: ");
  Serial.print(patternNames[patternIndex]);
  Serial.print(" | Color Set: ");
  Serial.print(colorSet);
  Serial.print(" | Mode: ");
  Serial.println(getCurrentLedMode() == DUAL_LED_MODE_RGB_ENHANCED ? "RGB_ENHANCED" : "DUAL_BASIC");
  
  setLedPattern(patterns[patternIndex], primary, secondary);
}

void testDualLedModes() {
  Serial.println("\n=== Testing Dual LED Modes ===");
  
  Serial.print("Current LED Mode: ");
  switch (getCurrentLedMode()) {
    case DUAL_LED_MODE_SINGLE:
      Serial.println("SINGLE");
      break;
    case DUAL_LED_MODE_DUAL_BASIC:
      Serial.println("DUAL_BASIC");
      break;
    case DUAL_LED_MODE_RGB_ENHANCED:
      Serial.println("RGB_ENHANCED");
      break;
    default:
      Serial.println("UNKNOWN");
      break;
  }
  
  Serial.println("Testing dual LED flash...");
  for (int i = 0; i < 5; i++) {
    flashDualLed((rgb_color_t)LED_COLOR_WHITE, (rgb_color_t)LED_COLOR_BLUE, 200);
    delay(300);
  }
  
  Serial.println("Testing LED enable/disable...");
  setLedEnabled(false);
  delay(1000);
  setLedEnabled(true);
  delay(500);
}

void printLedStatus() {
  Serial.println("\n=== LED Status ===");
  Serial.print("Current Pattern: ");
  Serial.println(getCurrentLedPattern());
  Serial.print("LED Mode: ");
  Serial.println(getCurrentLedMode());
  Serial.print("User LED State: ");
  Serial.println(dualLedState.user_led_state ? "ON" : "OFF");
  Serial.print("Charge LED Detected: ");
  Serial.println(dualLedState.charge_led_detected ? "YES" : "NO");
  Serial.print("LED Manager Enabled: ");
  Serial.println(dualLedState.enabled ? "YES" : "NO");
} 