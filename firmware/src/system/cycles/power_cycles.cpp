#include "power_cycles.h"
#include "cycle_manager.h"
#include "../battery/battery_code.h"
#include "../../status/device_status.h"
#include "../power_management/power_management.h"
#include "../memory/memory_utils.h"
#include "../clock/timing.h"
#include "../../hal/constants.h"
#include <Arduino.h>

// External variables
extern bool deviceReady;
extern uint8_t deviceStatus;
extern uint8_t batteryLevel;
extern bool batteryDetected;
extern bool connectionStable;
extern bool isCharging;
extern bool isCapturingPhotos;
extern bool photoDataUploading;
extern bool isStreamingVideo;

// Function declarations
extern bool isConnected();

// Constants that might not be available
#ifndef POWER_IDLE_TIMEOUT_MS
#define POWER_IDLE_TIMEOUT_MS 300000  // 5 minutes
#endif

// ===================================================================
// POWER MANAGEMENT CYCLE MANAGER
// ===================================================================

namespace PowerCycles {
    int battery_update_cycle_id = -1;
    int power_stats_cycle_id = -1;
    int sleep_management_cycle_id = -1;
    int memory_monitor_cycle_id = -1;
    int memory_leak_check_cycle_id = -1;
    int hotspot_monitor_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Power Cycles...");
        // TEMPORARILY DISABLED: Power cycles causing stack overflow during photo transmission
        // registerBatteryUpdateCycle();
        // registerPowerStatsCycle();
        // registerSleepManagementCycle();
        // registerMemoryMonitorCycle();
        // registerMemoryLeakCheckCycle();
        // registerHotspotMonitorCycle();  // DISABLED: Causes BLE interference
        Serial.println("Power cycles temporarily disabled to prevent stack overflow");
    }
    
    void registerBatteryUpdateCycle() {
        battery_update_cycle_id = registerIntervalCycle(
            "BatteryUpdate",
            BATTERY_UPDATE_INTERVAL,
            []() {
                updateBatteryLevel();
                
                // Update power statistics
                float batteryVoltage = readBatteryVoltage();
                bool cameraActive = isCapturingPhotos || photoDataUploading;
                updatePowerStats(batteryVoltage, false, isConnected(), cameraActive);
                
                // Optimize power based on battery level (charging state disabled)
                optimizePowerForBattery(batteryLevel, false);
                
                // Check battery status changes
                static bool lastBatteryDetected = true;
                static bool lastConnectionStable = true;
                static bool lastIsCharging = false;
                
                if (!batteryDetected && lastBatteryDetected && deviceReady) {
                    Serial.println("Battery disconnected during operation!");
                    updateDeviceStatus(DEVICE_STATUS_BATTERY_NOT_DETECTED);
                } else if (batteryDetected && !lastBatteryDetected && deviceStatus == DEVICE_STATUS_BATTERY_NOT_DETECTED) {
                    Serial.println("Battery reconnected!");
                    updateDeviceStatus(DEVICE_STATUS_READY);
                }
                
                if (batteryDetected && !connectionStable && lastConnectionStable && deviceReady) {
                    Serial.println("⚠️  Unstable battery connection detected!");
                    updateDeviceStatus(DEVICE_STATUS_BATTERY_UNSTABLE);
                } else if (batteryDetected && connectionStable && !lastConnectionStable && deviceStatus == DEVICE_STATUS_BATTERY_UNSTABLE) {
                    Serial.println("✅ Battery connection stabilized");
                    updateDeviceStatus(DEVICE_STATUS_READY);
                }
                
                if (isCharging && !lastIsCharging && deviceReady && deviceStatus != DEVICE_STATUS_CHARGING) {
                    Serial.println("Device is now charging!");
                    updateDeviceStatus(DEVICE_STATUS_CHARGING);
                } else if (!isCharging && lastIsCharging && deviceStatus == DEVICE_STATUS_CHARGING) {
                    Serial.println("Device is no longer charging");
                    updateDeviceStatus(DEVICE_STATUS_READY);
                }
                
                lastBatteryDetected = batteryDetected;
                lastConnectionStable = connectionStable;
                lastIsCharging = isCharging;
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerPowerStatsCycle() {
        power_stats_cycle_id = registerIntervalCycle(
            "PowerStats",
            600000, // 10 minutes
            []() {
                printPowerStats();
            },
            CYCLE_PRIORITY_BACKGROUND
        );
    }
    
    void registerSleepManagementCycle() {
        sleep_management_cycle_id = registerIntervalCycle(
            "SleepManagement",
            10000, // Check every 10 seconds
            []() {
                static unsigned long lastActivityTime = measureStart();
                
                // Update activity time if there's active operation
                if (isConnected() || isCapturingPhotos || photoDataUploading || isStreamingVideo) {
                    lastActivityTime = measureStart();
                }
                
                // Check if we should enter power saving mode
                unsigned long idleTime = getElapsedTime(lastActivityTime);
                if (shouldEnterPowerSaving(batteryLevel, idleTime)) {
                    // Enter light sleep for a short period if idle
                    if (idleTime > POWER_IDLE_TIMEOUT_MS && !isConnected()) {
                        Serial.println("Device idle, entering light sleep...");
                        prepareForSleep();
                        enterLightSleep(1000); // Sleep for 1 second
                        wakeFromSleep();
                    }
                }
            },
            CYCLE_PRIORITY_LOW
        );
    }
    
    void registerMemoryMonitorCycle() {
        memory_monitor_cycle_id = registerIntervalCycle(
            "MemoryMonitor",
            MEMORY_UPDATE_INTERVAL,
            []() {
                updateMemoryStats();
                
                // Check memory health
                if (!memoryHealthCheck()) {
                    Serial.println("⚠️  Memory health check failed");
                    updateDeviceStatus(DEVICE_STATUS_ERROR);
                }
                
                // Emergency cleanup if memory pressure is high
                if (memoryStats.memory_pressure) {
                    Serial.println("🚨 Memory pressure detected, initiating cleanup");
                    emergencyMemoryCleanup();
                    
                    // Force garbage collection
                    esp_get_free_heap_size();
                    
                    // Update stats after cleanup
                    updateMemoryStats();
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerMemoryLeakCheckCycle() {
        memory_leak_check_cycle_id = registerIntervalCycle(
            "MemoryLeakCheck",
            MEMORY_LEAK_CHECK_INTERVAL,
            []() {
                int leak_count = checkMemoryLeaks();
                if (leak_count > 0) {
                    Serial.printf("⚠️  Memory leak check found %d potential leaks\n", leak_count);
                    printTrackedAllocations();
                }
            },
            CYCLE_PRIORITY_BACKGROUND
        );
    }
    
    void registerHotspotMonitorCycle() {
        // DISABLED: Causes BLE interference
        Serial.println("Hotspot monitor cycle disabled (prevents BLE interference)");
    }
} 