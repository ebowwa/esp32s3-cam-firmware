#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE  // Enable Sense version constants
#include "esp_camera.h"
#include "src/hal/camera_pins.h"
#include "src/features/microphone/mulaw.h"
#include "src/system/clock/timing.h"
#include "src/system/power_management/power_management.h"
#include "src/system/memory/memory_utils.h"
// #include "src/utils/hotspot_manager.h"  // DISABLED: Causes BLE interference
#include "src/system/cycles/cycle_manager.h"
#include "src/system/cycles/specialized_cycles.h"
#include "src/hal/led/led_manager.h"
#include "src/system/battery/battery_code.h"
// #include "src/system/charging/charging_manager.h"  // DISABLED: Compilation issues
#include "src/hal/constants.h"
#include "src/status/device_status.h"
#include "src/features/camera/camera.h"
#include "src/features/bluetooth/ble_manager.h"
#include "src/features/microphone/microphone_manager.h"
#include "src/system/serial/serial.h"

// State variables
// Note: BLE connection state is now managed by BLE manager

// Camera state variables are now defined in camera_simple.cpp




// Forward declarations for control handlers
void handlePhotoControl(int8_t controlValue);
void handleVideoControl(uint8_t controlValue);

// Note: BLE callback classes are now in src/bluetooth/callbacks/

// Note: BLE configuration is now handled by the BLE manager
// See src/bluetooth/ble_manager.h for the new interface

// Camera functions are now defined in camera_simple.cpp

// handlePhotoControl function is now defined in camera_simple.cpp
// updateVideoStatus function is now in src/features/bluetooth/characteristics/ble_characteristics.cpp

// Microphone functionality moved to MicrophoneManager class

//
// Camera functions are now defined in camera_simple.cpp
//



//
// Main
//

// static uint8_t *s_compressed_frame_2 = nullptr;
// static size_t compressed_buffer_size = 400 + 3;
void setup() {
  // Initialize the unified serial system
  SerialSystem::initialize();
  SerialSystem::info("OpenGlass starting up...", MODULE_MAIN);
  
  // Initialize LED manager first for early status indication
  initLedManager();
  
  // Print wake-up reason
  SerialSystem::infof(MODULE_MAIN, "Wake-up reason: %s", getWakeupReason());
  
  // Initialize power management
  initializePowerManagement();
  
  // Initialize charging manager
  // initializeChargingManager();  // DISABLED: Compilation issues
  
  // Initialize memory manager
  initializeMemoryManager();
  
  // Initialize hotspot manager
  // initializeHotspotManager();  // DISABLED: Causes BLE interference
  
  // Initialize centralized cycle manager
  initializeCycleManager();
  
  updateDeviceStatus(DEVICE_STATUS_BLE_INIT);
  configureBLE();
  SerialSystem::logInitialization("BLE", true, MODULE_BLE);
  
  updateDeviceStatus(DEVICE_STATUS_MICROPHONE_INIT);
  if (!MicrophoneManager::initialize()) {
    SerialSystem::logInitialization("Microphone Manager", false, MODULE_MICROPHONE);
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1); // do nothing
  }
  
  if (!MicrophoneManager::configure()) {
    SerialSystem::logError("Microphone", "Configuration failed", MODULE_MICROPHONE);
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1); // do nothing
  }
  SerialSystem::logInitialization("Microphone", true, MODULE_MICROPHONE);
  
  updateDeviceStatus(DEVICE_STATUS_CAMERA_INIT);
  configure_camera();
  SerialSystem::logInitialization("Camera", true, MODULE_CAMERA);
  
  // Test camera functionality
  SerialSystem::info("Testing camera functionality...", MODULE_CAMERA);
  if (take_photo()) {
    SerialSystem::logPhotoCapture(fb->len, "test");
    if (fb) {
      esp_camera_fb_return(fb);
      fb = nullptr;
    }
  } else {
    SerialSystem::logError("Camera", "Test photo failed - device will be in ERROR state", MODULE_CAMERA);
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    // Continue with setup but device will remain in error state
  }
  
  // Check battery presence
  SerialSystem::info("Checking battery connection...", MODULE_BATTERY);
  if (!checkBatteryPresence()) {
    SerialSystem::warning("No lithium battery detected!", MODULE_BATTERY);
    updateDeviceStatus(DEVICE_STATUS_BATTERY_NOT_DETECTED);
    delay(TIMING_LONG); // Give time to see the warning
  } else {
    SerialSystem::info("Battery detected and connected", MODULE_BATTERY);
  }
  
  updateDeviceStatus(DEVICE_STATUS_WARMING_UP);
  SerialSystem::info("Device warming up...", MODULE_MAIN);
  delay(TIMING_LONG); // Give camera and microphone time to stabilize
  
  deviceReady = true;
  updateDeviceStatus(DEVICE_STATUS_READY);
  SerialSystem::info("OpenGlass ready!", MODULE_MAIN);
  
  // Initialize all specialized cycle managers
  initializeSpecializedCycles();
  
  SerialSystem::logInitialization("Specialized Cycle Managers", true, MODULE_CYCLES);
}

void loop() {
  // Performance monitoring
  static unsigned long loopCount = 0;
  static unsigned long lastPerformanceReport = 0;
  static unsigned long totalLoopTime = 0;
  static unsigned long maxLoopTime = 0;
  
  unsigned long loopStart = measureStart();
  
  // Update all cycles using centralized cycle manager
  updateCycles();
  
  // Handle connection loss during photo/video upload
  if (photoDataUploading && !isConnected()) {
    SerialSystem::warning("Connection lost during photo/video upload, stopping", MODULE_BLE);
    photoDataUploading = false;
  }
  
  // Calculate loop performance
  unsigned long loopDuration = measureEnd(loopStart);
  totalLoopTime += loopDuration;
  if (loopDuration > maxLoopTime) {
    maxLoopTime = loopDuration;
  }
  loopCount++;
  
  // Report performance every 30 seconds
  if (shouldExecute(&lastPerformanceReport, 30000)) {
    float avgLoopTime = (float)totalLoopTime / loopCount;
    SerialSystem::infof(MODULE_SYSTEM, "🔧 Loop Performance: Avg=%.2fms, Max=%lums, Count=%lu", 
                       avgLoopTime, maxLoopTime, loopCount);
    
    // Reset counters
    totalLoopTime = 0;
    maxLoopTime = 0;
    loopCount = 0;
  }
  
  // Update the serial system for periodic monitoring
  SerialSystem::update();
  
  // Small delay to prevent excessive CPU usage
  delay(MAIN_LOOP_DELAY);
}
