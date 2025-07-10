// Test sketch for timing utilities
// This demonstrates all the timing utility functions

#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE

#include "../src/utils/timing.h"
#include "../src/hardware/xiao_esp32s3_constants.h"

void setup() {
  Serial.begin(XIAO_ESP32S3_SERIAL_BAUD_RATE);
  Serial.println("Testing Timing Utilities...");
  Serial.println("===========================");
  
  // Test 1: Basic timeout functionality
  Serial.println("\n1. Testing timeout functionality:");
  unsigned long startTime = millis();
  Serial.printf("Start time: %lu ms\n", startTime);
  
  delay(500);
  
  if (hasTimedOut(startTime, 1000)) {
    Serial.println("❌ Timeout test failed - should not timeout yet");
  } else {
    Serial.println("✅ Timeout test passed - no timeout yet");
  }
  
  delay(600);
  
  if (hasTimedOut(startTime, 1000)) {
    Serial.println("✅ Timeout test passed - timeout occurred");
  } else {
    Serial.println("❌ Timeout test failed - should have timed out");
  }
  
  // Test 2: Elapsed time
  Serial.println("\n2. Testing elapsed time:");
  unsigned long elapsed = getElapsedTime(startTime);
  Serial.printf("Elapsed time: %lu ms\n", elapsed);
  
  // Test 3: Performance measurement
  Serial.println("\n3. Testing performance measurement:");
  unsigned long perfStart = measureStart();
  
  // Simulate some work
  for (int i = 0; i < 1000; i++) {
    volatile int dummy = i * 2;
  }
  
  unsigned long perfDuration = measureEnd(perfStart);
  Serial.printf("Loop execution took: %lu ms\n", perfDuration);
  
  // Test 4: Non-blocking delay
  Serial.println("\n4. Testing non-blocking delay:");
  Serial.println("Waiting 2 seconds non-blocking...");
  
  unsigned long nbDelayStart = millis();
  static unsigned long nbDelayTime = 0;
  bool delayCompleted = false;
  
  while (!delayCompleted) {
    if (nonBlockingDelayStateful(&nbDelayTime, 2000)) {
      delayCompleted = true;
      Serial.println("✅ Non-blocking delay completed");
    } else {
      // Do other work while waiting
      Serial.print(".");
      delay(100);
    }
  }
  
  // Test 5: Rate limiting
  Serial.println("\n5. Testing rate limiting:");
  static unsigned long rateLimitTime = 0;
  int executionCount = 0;
  
  for (int i = 0; i < 50; i++) {
    if (shouldExecute(&rateLimitTime, 200)) {
      executionCount++;
      Serial.printf("Rate limited execution #%d\n", executionCount);
    }
    delay(50);
  }
  
  Serial.printf("Total executions: %d (should be around 12-13)\n", executionCount);
  
  // Test 6: Throttling
  Serial.println("\n6. Testing throttling:");
  static unsigned long throttleTime = 0;
  int throttleCount = 0;
  
  for (int i = 0; i < 20; i++) {
    if (throttle(&throttleTime, 150)) {
      throttleCount++;
      Serial.printf("Throttled call #%d\n", throttleCount);
    }
    delay(50);
  }
  
  Serial.printf("Total throttled calls: %d\n", throttleCount);
  
  // Test 7: Time remaining
  Serial.println("\n7. Testing time remaining:");
  unsigned long timeoutTest = millis();
  unsigned long timeoutDuration = 3000;
  
  while (true) {
    unsigned long remaining = getTimeRemaining(timeoutTest, timeoutDuration);
    if (remaining == 0) {
      Serial.println("✅ Timeout reached");
      break;
    }
    Serial.printf("Time remaining: %lu ms\n", remaining);
    delay(500);
  }
  
  Serial.println("\n✅ All timing utility tests completed!");
}

void loop() {
  // Test 8: Continuous throttling in loop
  static unsigned long loopThrottleTime = 0;
  static int loopCount = 0;
  
  if (throttle(&loopThrottleTime, 1000)) {
    loopCount++;
    Serial.printf("Loop iteration #%d (throttled to 1Hz)\n", loopCount);
  }
  
  // Test 9: Time window check
  static unsigned long windowStart = 0;
  static bool windowStarted = false;
  
  if (!windowStarted) {
    windowStart = millis();
    windowStarted = true;
    Serial.println("Starting 5-second time window...");
  }
  
  if (isWithinTimeWindow(windowStart, 5000)) {
    static unsigned long windowPrintTime = 0;
    if (throttle(&windowPrintTime, 1000)) {
      Serial.println("Within time window");
    }
  } else {
    static bool windowEnded = false;
    if (!windowEnded) {
      Serial.println("Time window ended, resetting...");
      windowStarted = false;
      windowEnded = true;
    }
  }
  
  delay(100);
} 