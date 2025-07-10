#include "cycle_manager.h"
#include "../system/charging_manager.h"
#include "../system/battery_code.h"
#include "../system/device_status.h"
#include "../led/led_manager.h"
#include "../utils/power_management.h"
#include "../utils/memory_utils.h"
#include "../camera/camera.h"
#include "../bluetooth/ble_manager.h"
#include "../bluetooth/ble_characteristics.h"
#include "../platform/constants.h"
#include "esp_camera.h"
#include <BLECharacteristic.h>

// External declarations for variables and functions defined in main firmware
extern size_t read_microphone();
extern uint8_t *s_recording_buffer;
extern uint8_t *s_compressed_frame_2;

// Device state variables
extern bool deviceReady;
extern uint8_t deviceStatus;

// Battery and power variables
extern uint8_t batteryLevel;
extern bool batteryDetected;
extern bool connectionStable;
extern bool isCharging;

// Camera state variables
extern bool isCapturingPhotos;
extern int captureInterval;
extern unsigned long lastCaptureTime;
extern size_t sent_photo_bytes;
extern size_t sent_photo_frames;
extern bool photoDataUploading;
extern bool isStreamingFrame;

// Video streaming variables
extern bool isStreamingVideo;
extern int streamingFPS;
extern unsigned long lastStreamFrame;
extern size_t totalStreamingFrames;

// Camera frame buffer
extern camera_fb_t *fb;

// Function declarations
extern bool isConnected();
extern bool shouldDropFrame();
extern bool take_photo();

// BLE characteristics
extern BLECharacteristic *batteryLevelCharacteristic;

// Constants that might not be available
#ifndef POWER_IDLE_TIMEOUT_MS
#define POWER_IDLE_TIMEOUT_MS 300000  // 5 minutes
#endif

// ===================================================================
// CHARGING CYCLE MANAGER
// ===================================================================

namespace ChargingCycles {
    int charging_status_cycle_id = -1;
    int charging_history_cycle_id = -1;
    int charging_safety_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Charging Cycles...");
        registerChargingStatusCycle();
        registerChargingHistoryCycle();
        registerChargingSafetyCycle();
    }
    
    void registerChargingStatusCycle() {
        charging_status_cycle_id = registerIntervalCycle(
            "ChargingStatus",
            CHARGING_UPDATE_INTERVAL,
            []() {
                updateChargingStatus();
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerChargingHistoryCycle() {
        charging_history_cycle_id = registerConditionCycle(
            "ChargingHistory",
            []() {
                // Record history when charging state changes from charging to not charging
                static charging_state_t last_state = CHARGING_STATE_UNKNOWN;
                bool should_record = (last_state != CHARGING_STATE_NOT_CHARGING && 
                                    chargingStats.state == CHARGING_STATE_NOT_CHARGING);
                last_state = chargingStats.state;
                return should_record;
            },
            []() {
                recordChargingHistory();
            },
            CYCLE_PRIORITY_BACKGROUND
        );
    }
    
    void registerChargingSafetyCycle() {
        charging_safety_cycle_id = registerIntervalCycle(
            "ChargingSafety",
            CHARGING_SAFETY_CHECK_INTERVAL,
            []() {
                charging_safety_t safety = checkChargingSafety();
                if (safety != CHARGING_SAFETY_OK) {
                    handleChargingError(safety);
                }
            },
            CYCLE_PRIORITY_CRITICAL
        );
    }
}

// ===================================================================
// LED PATTERN CYCLE MANAGER
// ===================================================================

namespace LEDCycles {
    int led_update_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing LED Cycles...");
        registerLEDUpdateCycle();
        registerPatternCycles();
    }
    
    void registerLEDUpdateCycle() {
        led_update_cycle_id = registerIntervalCycle(
            "LEDUpdate",
            20, // 20ms for smooth LED updates
            []() {
                updateLed();
            },
            CYCLE_PRIORITY_CRITICAL
        );
    }
    
    void registerPatternCycles() {
        // LED patterns are handled by the main LED update cycle
        // Individual patterns can be registered as needed
    }
}

// ===================================================================
// POWER MANAGEMENT CYCLE MANAGER
// ===================================================================

namespace PowerCycles {
    int battery_update_cycle_id = -1;
    int power_stats_cycle_id = -1;
    int sleep_management_cycle_id = -1;
    int memory_monitor_cycle_id = -1;
    int memory_leak_check_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Power Cycles...");
        registerBatteryUpdateCycle();
        registerPowerStatsCycle();
        registerSleepManagementCycle();
        registerMemoryMonitorCycle();
        registerMemoryLeakCheckCycle();
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
                
                // Optimize power based on battery level and charging state
                optimizePowerForBattery(batteryLevel, chargingStats.state != CHARGING_STATE_NOT_CHARGING);
                
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
}

// ===================================================================
// DATA ACQUISITION CYCLE MANAGER
// ===================================================================

namespace DataCycles {
    int audio_capture_cycle_id = -1;
    int photo_cycle_id = -1;
    int video_stream_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Data Cycles...");
        registerAudioCaptureCycle();
        registerPhotoCycle();
        registerVideoStreamCycle();
    }
    
    void registerAudioCaptureCycle() {
        audio_capture_cycle_id = registerConditionCycle(
            "AudioCapture",
            []() {
                return deviceReady && isConnected();
            },
            []() {
                size_t bytes_recorded = read_microphone();
                if (bytes_recorded > 0) {
                    BLEManager::transmitAudio(s_recording_buffer, RECORDING_BUFFER_SIZE, bytes_recorded);
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerPhotoCycle() {
        photo_cycle_id = registerConditionCycle(
            "PhotoCapture",
            []() {
                if (!deviceReady || !isConnected() || photoDataUploading) {
                    return false;
                }
                
                if (!isCapturingPhotos) {
                    return false;
                }
                
                if (captureInterval == 0) {
                    // Single photo requested - capture immediately
                    return true;
                } else {
                    // Interval capture - check if enough time has passed
                    return (millis() - lastCaptureTime) >= captureInterval;
                }
            },
            []() {
                if (captureInterval == 0) {
                    // Single photo requested
                    isCapturingPhotos = false;
                }

                // Take the photo
                if (take_photo()) {
                    photoDataUploading = true;
                    sent_photo_bytes = 0;
                    sent_photo_frames = 0;
                    lastCaptureTime = millis();
                    isStreamingFrame = false;
                }
            },
            CYCLE_PRIORITY_NORMAL
        );
    }
    
    void registerVideoStreamCycle() {
        video_stream_cycle_id = registerConditionCycle(
            "VideoStream",
            []() {
                if (!deviceReady || !isConnected() || photoDataUploading || !isStreamingVideo) {
                    return false;
                }
                
                if (shouldDropFrame()) {
                    return false;
                }
                
                unsigned long frameInterval = VIDEO_STREAM_FRAME_INTERVAL(streamingFPS);
                return (millis() - lastStreamFrame) >= frameInterval;
            },
            []() {
                if (take_photo()) {
                    photoDataUploading = true;
                    sent_photo_bytes = 0;
                    sent_photo_frames = 0;
                    lastStreamFrame = millis();
                    isStreamingFrame = true;
                    totalStreamingFrames++;
                }
            },
            CYCLE_PRIORITY_NORMAL
        );
    }
}

// ===================================================================
// COMMUNICATION CYCLE MANAGER
// ===================================================================

namespace CommCycles {
    int ble_update_cycle_id = -1;
    int data_transmission_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Communication Cycles...");
        registerBLEUpdateCycle();
        registerDataTransmissionCycle();
    }
    
    void registerBLEUpdateCycle() {
        ble_update_cycle_id = registerIntervalCycle(
            "BLEUpdate",
            1000, // 1 second
            []() {
                // Update BLE characteristics
                if (isConnected()) {
                    // Update device status
                    updateDeviceStatus(deviceStatus);
                    
                    // Update battery level
                    if (batteryLevelCharacteristic) {
                        batteryLevelCharacteristic->setValue(&batteryLevel, 1);
                        batteryLevelCharacteristic->notify();
                    }
                    
                    // Update charging stats
                    notifyChargingClients();
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerDataTransmissionCycle() {
        data_transmission_cycle_id = registerConditionCycle(
            "DataTransmission",
            []() {
                return photoDataUploading && fb && isConnected();
            },
            []() {
                size_t remaining = fb->len - sent_photo_bytes;
                if (remaining > 0) {
                    // Populate buffer with frame type header
                    s_compressed_frame_2[0] = sent_photo_frames & 0xFF;
                    s_compressed_frame_2[1] = (sent_photo_frames >> 8) & 0xFF;
                    s_compressed_frame_2[2] = isStreamingFrame ? 0x02 : 0x01; // Frame type
                    
                    size_t bytes_to_copy = remaining;
                    if (bytes_to_copy > PHOTO_CHUNK_SIZE - 1) {
                        bytes_to_copy = PHOTO_CHUNK_SIZE - 1;
                    }
                    
                    // Bounds check
                    if (sent_photo_bytes + bytes_to_copy <= fb->len) {
                        memcpy(&s_compressed_frame_2[3], &fb->buf[sent_photo_bytes], bytes_to_copy);

                        // Push to appropriate BLE characteristic
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
                    // End flag
                    if (isStreamingFrame) {
                        BLEManager::transmitVideoEnd();
                        Serial.printf("Video frame completed: %d bytes in %d frames\n", sent_photo_bytes, sent_photo_frames);
                    } else {
                        BLEManager::transmitPhotoEnd();
                        Serial.printf("Photo upload completed: %d bytes in %d frames\n", sent_photo_bytes, sent_photo_frames);
                    }
                    photoDataUploading = false;
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
} 