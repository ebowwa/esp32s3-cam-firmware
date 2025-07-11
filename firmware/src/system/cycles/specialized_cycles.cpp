#include "cycle_manager.h"
#include "../charging/charging_manager.h"
#include "../battery/battery_code.h"
#include "../device_status.h"
#include "../../hal/led/led_manager.h"
#include "../power_management/power_management.h"
#include "../memory/memory_utils.h"
// #include "../../utils/hotspot_manager.h"  // DISABLED: Causes BLE interference
#include "../../features/camera/camera.h"
#include "../../features/bluetooth/ble_manager.h"
#include "../../features/bluetooth/characteristics/ble_characteristics.h"
#include "../../hal/constants.h"
#include "../clock/timing.h"
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
        // TEMPORARILY DISABLED: Charging cycles causing stack overflow during photo transmission
        // registerChargingStatusCycle();
        // registerChargingHistoryCycle();
        // registerChargingSafetyCycle();
        Serial.println("Charging cycles temporarily disabled to prevent stack overflow");
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

// ===================================================================
// DATA ACQUISITION CYCLE MANAGER
// ===================================================================

namespace DataCycles {
    int audio_capture_cycle_id = -1;
    int photo_cycle_id = -1;
    int video_stream_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Data Cycles...");
        // TEMPORARILY DISABLED: Audio capture cycle causing log spam
        // registerAudioCaptureCycle();
        registerPhotoCycle();
        // registerVideoStreamCycle();   // DISABLED: Not needed for photo capture
        Serial.println("Data cycles: Only photo capture enabled");
    }
    
    void registerAudioCaptureCycle() {
        audio_capture_cycle_id = registerConditionCycle(
            "AudioCapture",
            []() {
                // Audio capture when connected and recording
                return isConnected() && deviceReady && !isCapturingPhotos && !photoDataUploading;
            },
            []() {
                // Audio capture logic would go here
                // For now, just a placeholder
                Serial.println("Audio capture cycle (placeholder)");
            },
            CYCLE_PRIORITY_NORMAL
        );
    }
    
    void registerPhotoCycle() {
        photo_cycle_id = registerConditionCycle(
            "PhotoCapture",
            []() {
                if (!deviceReady || !isConnected() || photoDataUploading) {
                    return false;
                }
                
                // Check if we should capture based on control flags
                bool should_capture = isCapturingPhotos && (captureInterval == 0 || 
                    getElapsedTime(lastCaptureTime) >= (captureInterval * 1000));
                
                return should_capture;
            },
            []() {
                Serial.println("Taking photo...");
                
                // Take photo
                if (take_photo()) {
                    Serial.printf("Photo captured: %d bytes\n", fb->len);
                    
                    // Set flags for data transmission
                    photoDataUploading = true;
                    lastCaptureTime = measureStart();
                    
                    // Update capture count
                    if (captureInterval == 0) {
                        // Single capture mode - stop after one photo
                        isCapturingPhotos = false;
                        Serial.println("Single photo capture completed");
                    }
                } else {
                    Serial.println("Photo capture failed");
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerVideoStreamCycle() {
        video_stream_cycle_id = registerConditionCycle(
            "VideoStream",
            []() {
                // Video streaming when connected and streaming is active
                return isConnected() && deviceReady && isStreamingVideo && !photoDataUploading;
            },
            []() {
                // Video streaming logic would go here
                Serial.println("Video stream cycle (placeholder)");
            },
            CYCLE_PRIORITY_HIGH
        );
    }
}

// ===================================================================
// COMMUNICATION CYCLE MANAGER
// ===================================================================

namespace CommCycles {
    int data_transmission_cycle_id = -1;
    int connection_monitor_cycle_id = -1;
    
    void registerDataTransmissionCycle();
    void registerConnectionMonitorCycle();
    
    void initialize() {
        Serial.println("Initializing Communication Cycles...");
        registerDataTransmissionCycle();
        registerConnectionMonitorCycle();
    }
    
    void registerDataTransmissionCycle() {
        data_transmission_cycle_id = registerConditionCycle(
            "DataTransmission",
            []() {
                return photoDataUploading && fb && isConnected();
            },
            []() {
                if (!fb || !isConnected()) {
                    photoDataUploading = false;
                    sent_photo_bytes = 0;
                    sent_photo_frames = 0;
                    return;
                }
                
                // Calculate remaining data to send
                size_t remaining = fb->len - sent_photo_bytes;
                
                if (remaining > 0) {
                    // Prepare frame with header
                    uint8_t frame_buffer[PHOTO_CHUNK_SIZE + 3]; // +3 for header
                    
                    // Frame header: [frame_number_low, frame_number_high, frame_type]
                    frame_buffer[0] = sent_photo_frames & 0xFF;
                    frame_buffer[1] = (sent_photo_frames >> 8) & 0xFF;
                    frame_buffer[2] = 0x01; // Photo frame type
                    
                    // Calculate chunk size (leave room for header)
                    size_t chunk_size = min(remaining, (size_t)PHOTO_CHUNK_SIZE);
                    
                    // Copy photo data after header
                    memcpy(&frame_buffer[3], fb->buf + sent_photo_bytes, chunk_size);
                    
                    // Send frame with header + data
                    notifyPhotoData(frame_buffer, chunk_size + 3);
                    
                    sent_photo_bytes += chunk_size;
                    sent_photo_frames++;
                    
                    Serial.printf("Sent photo frame %d: %d bytes (total: %d/%d)\n", 
                                 sent_photo_frames, chunk_size, sent_photo_bytes, fb->len);
                    
                    // Note: BLE transmission throttling is handled by the cycle manager timing
                } else {
                    // Transmission complete - send end marker
                    Serial.printf("Photo transmission complete: %d bytes in %d frames\n", 
                                 sent_photo_bytes, sent_photo_frames);
                    
                    // Send end marker: [0xFF, 0xFF, 0x01]
                    uint8_t endMarker[3] = {0xFF, 0xFF, 0x01};
                    notifyPhotoData(endMarker, 3);
                    
                    // Clean up
                    if (fb) {
                        esp_camera_fb_return(fb);
                        fb = nullptr;
                    }
                    
                    photoDataUploading = false;
                    sent_photo_bytes = 0;
                    sent_photo_frames = 0;
                    
                    Serial.println("Photo transmission cycle completed");
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerConnectionMonitorCycle() {
        connection_monitor_cycle_id = registerIntervalCycle(
            "ConnectionMonitor",
            5000, // Check every 5 seconds
            []() {
                static bool lastConnected = false;
                bool currentConnected = isConnected();
                
                if (currentConnected != lastConnected) {
                    if (currentConnected) {
                        Serial.println("BLE connection established");
                        setLedPattern(LED_CONNECTED);
                    } else {
                        Serial.println("BLE connection lost");
                        setLedPattern(LED_DISCONNECTED);
                        
                        // Clean up any ongoing operations
                        if (photoDataUploading) {
                            Serial.println("Cleaning up photo upload due to disconnection");
                            if (fb) {
                                esp_camera_fb_return(fb);
                                fb = nullptr;
                            }
                            photoDataUploading = false;
                            sent_photo_bytes = 0;
                            sent_photo_frames = 0;
                        }
                    }
                    lastConnected = currentConnected;
                                 }
             },
             CYCLE_PRIORITY_NORMAL
        );
    }
} 