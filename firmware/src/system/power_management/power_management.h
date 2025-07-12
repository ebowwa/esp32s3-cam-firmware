#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include "../../hal/xiao_esp32s3_constants.h"

// ===================================================================
// POWER MANAGEMENT UTILITIES
// ===================================================================
//
// Note: RTC GPIO functionality has been limited due to compilation conflicts
// with ESP32 Arduino Core 2.0.17. For advanced RTC GPIO configuration,
// use the ESP-IDF directly or configure GPIO pullups manually before
// calling sleep functions.
//

/**
 * Power management modes
 */
typedef enum {
    POWER_MODE_PERFORMANCE,    // Maximum performance, highest power consumption
    POWER_MODE_BALANCED,       // Balanced performance and power
    POWER_MODE_POWER_SAVE,     // Power saving mode with reduced performance
    POWER_MODE_ULTRA_LOW       // Ultra low power mode
} power_mode_t;

/**
 * Sleep wake-up sources
 */
typedef enum {
    WAKEUP_TIMER,             // Wake up after specified time
    WAKEUP_EXT0,              // Wake up from external pin (RTC_IO)
    WAKEUP_EXT1,              // Wake up from multiple pins
    WAKEUP_TOUCHPAD,          // Wake up from touch sensor
    WAKEUP_ULP                // Wake up from ULP program
} wakeup_source_t;

/**
 * Power consumption tracking
 */
typedef struct {
    float voltage;             // Battery voltage
    float current_ma;          // Estimated current consumption in mA
    float power_mw;            // Estimated power consumption in mW
    unsigned long timestamp;   // Timestamp of measurement
    power_mode_t mode;         // Current power mode
} power_stats_t;

// Global power statistics - defined only once
#ifndef POWER_MANAGEMENT_GLOBALS_DEFINED
#define POWER_MANAGEMENT_GLOBALS_DEFINED
static power_stats_t currentPowerStats = {0};
static power_mode_t currentPowerMode = POWER_MODE_BALANCED;
#endif

/**
 * Initialize power management system
 */
static inline void initializePowerManagement() {
    // Configure power management
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = XIAO_ESP32S3_MAX_FREQ_MHZ,
        .min_freq_mhz = 80,  // Minimum frequency for stable operation
        .light_sleep_enable = true
    };
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret != ESP_OK) {
        Serial.printf("Power management config failed: %d\n", ret);
    } else {
        Serial.println("Power management configured successfully");
    }
    
    currentPowerMode = POWER_MODE_BALANCED;
    Serial.println("Power management initialized");
}

/**
 * Enter light sleep mode
 * @param duration_ms Duration to sleep in milliseconds (0 = indefinite)
 * @param wakeup_source Wake-up source to use
 * @param wakeup_pin GPIO pin for external wake-up (if using EXT0)
 */
static inline void enterLightSleep(uint32_t duration_ms, wakeup_source_t wakeup_source = WAKEUP_TIMER, int wakeup_pin = -1) {
    Serial.printf("Entering light sleep for %u ms\n", duration_ms);
    
    // Configure wake-up source
    switch (wakeup_source) {
        case WAKEUP_TIMER:
            if (duration_ms > 0) {
                esp_sleep_enable_timer_wakeup(duration_ms * 1000ULL); // Convert to microseconds
            }
            break;
            
        case WAKEUP_EXT0:
            if (wakeup_pin >= 0) {
                esp_sleep_enable_ext0_wakeup((gpio_num_t)wakeup_pin, 0); // Wake on LOW
                // Note: Configure GPIO pullup manually if needed before calling this function
            }
            break;
            
        case WAKEUP_EXT1:
            // Configure multiple pins for wake-up
            esp_sleep_enable_ext1_wakeup(0, ESP_EXT1_WAKEUP_ANY_HIGH);
            break;
            
        default:
            break;
    }
    
    // Enter light sleep
    esp_light_sleep_start();
    
    Serial.println("Woke up from light sleep");
}

/**
 * Enter deep sleep mode
 * @param duration_ms Duration to sleep in milliseconds (0 = indefinite)
 * @param wakeup_source Wake-up source to use
 * @param wakeup_pin GPIO pin for external wake-up (if using EXT0)
 */
static inline void enterDeepSleep(uint32_t duration_ms, wakeup_source_t wakeup_source = WAKEUP_TIMER, int wakeup_pin = -1) {
    Serial.printf("Entering deep sleep for %u ms\n", duration_ms);
    Serial.flush(); // Ensure message is sent before sleep
    
    // Configure wake-up source
    switch (wakeup_source) {
        case WAKEUP_TIMER:
            if (duration_ms > 0) {
                esp_sleep_enable_timer_wakeup(duration_ms * 1000ULL); // Convert to microseconds
            }
            break;
            
        case WAKEUP_EXT0:
            if (wakeup_pin >= 0) {
                esp_sleep_enable_ext0_wakeup((gpio_num_t)wakeup_pin, 0); // Wake on LOW
                // Note: RTC GPIO pullup configuration removed due to compilation conflicts
                // Configure pullup manually if needed before calling this function
            }
            break;
            
        case WAKEUP_EXT1:
            // Configure multiple pins for wake-up
            esp_sleep_enable_ext1_wakeup(0, ESP_EXT1_WAKEUP_ANY_HIGH);
            break;
            
        default:
            break;
    }
    
    // Enter deep sleep (will reset on wake-up)
    esp_deep_sleep_start();
}

/**
 * Set power mode for optimization
 * @param mode Power mode to set
 */
static inline void setPowerMode(power_mode_t mode) {
    currentPowerMode = mode;
    
    switch (mode) {
        case POWER_MODE_PERFORMANCE:
            // Maximum performance settings
            setCpuFrequencyMhz(XIAO_ESP32S3_MAX_FREQ_MHZ);
            Serial.println("Power mode: PERFORMANCE");
            break;
            
        case POWER_MODE_BALANCED:
            // Balanced settings
            setCpuFrequencyMhz(160);
            Serial.println("Power mode: BALANCED");
            break;
            
        case POWER_MODE_POWER_SAVE:
            // Power saving settings
            setCpuFrequencyMhz(80);
            Serial.println("Power mode: POWER_SAVE");
            break;
            
        case POWER_MODE_ULTRA_LOW:
            // Ultra low power settings
            setCpuFrequencyMhz(40);
            Serial.println("Power mode: ULTRA_LOW");
            break;
    }
}

/**
 * Get current CPU frequency
 * @return Current CPU frequency in MHz
 */
static inline uint32_t getCurrentCpuFrequency() {
    return getCpuFrequencyMhz();
}

/**
 * Estimate current power consumption based on mode and activity
 * @param wifi_active Whether WiFi is active
 * @param ble_active Whether BLE is active
 * @param camera_active Whether camera is active
 * @return Estimated current consumption in mA
 */
static inline float estimateCurrentConsumption(bool wifi_active = false, bool ble_active = true, bool camera_active = false) {
    float base_current = 0;
    
    // Base current based on power mode
    switch (currentPowerMode) {
        case POWER_MODE_PERFORMANCE:
            base_current = 50.0; // Higher base consumption
            break;
        case POWER_MODE_BALANCED:
            base_current = 30.0;
            break;
        case POWER_MODE_POWER_SAVE:
            base_current = 20.0;
            break;
        case POWER_MODE_ULTRA_LOW:
            base_current = 15.0;
            break;
    }
    
    // Add consumption for active components
    if (wifi_active) {
        base_current += XIAO_ESP32S3_WIFI_ACTIVE_MA;
    }
    
    if (ble_active) {
        base_current += XIAO_ESP32S3_BLE_ACTIVE_MA;
    }
    
    if (camera_active) {
        base_current += 50.0; // Estimated camera consumption
    }
    
    return base_current;
}

/**
 * Update power statistics
 * @param battery_voltage Current battery voltage
 * @param wifi_active Whether WiFi is active
 * @param ble_active Whether BLE is active
 * @param camera_active Whether camera is active
 */
static inline void updatePowerStats(float battery_voltage, bool wifi_active = false, bool ble_active = true, bool camera_active = false) {
    currentPowerStats.voltage = battery_voltage;
    currentPowerStats.current_ma = estimateCurrentConsumption(wifi_active, ble_active, camera_active);
    currentPowerStats.power_mw = currentPowerStats.voltage * currentPowerStats.current_ma;
    currentPowerStats.timestamp = millis();
    currentPowerStats.mode = currentPowerMode;
}

/**
 * Get current power statistics
 * @return Current power statistics structure
 */
static inline power_stats_t getPowerStats() {
    return currentPowerStats;
}

/**
 * Print power statistics to serial
 */
static inline void printPowerStats() {
    Serial.println("=== Power Statistics ===");
    Serial.printf("Battery Voltage: %.2f V\n", currentPowerStats.voltage);
    Serial.printf("Current Consumption: %.1f mA\n", currentPowerStats.current_ma);
    Serial.printf("Power Consumption: %.1f mW\n", currentPowerStats.power_mw);
    Serial.printf("CPU Frequency: %u MHz\n", getCurrentCpuFrequency());
    Serial.printf("Power Mode: %s\n", 
        currentPowerMode == POWER_MODE_PERFORMANCE ? "PERFORMANCE" :
        currentPowerMode == POWER_MODE_BALANCED ? "BALANCED" :
        currentPowerMode == POWER_MODE_POWER_SAVE ? "POWER_SAVE" : "ULTRA_LOW");
    Serial.printf("Timestamp: %lu ms\n", currentPowerStats.timestamp);
    Serial.println("========================");
}

/**
 * Optimize power settings based on battery level
 * @param battery_level Battery level percentage (0-100)
 * @param is_charging Whether device is charging
 */
static inline void optimizePowerForBattery(uint8_t battery_level, bool is_charging) {
    if (is_charging) {
        // When charging, use performance mode
        setPowerMode(POWER_MODE_PERFORMANCE);
    } else if (battery_level > 50) {
        // Good battery level, use balanced mode
        setPowerMode(POWER_MODE_BALANCED);
    } else if (battery_level > 20) {
        // Medium battery level, use power save mode
        setPowerMode(POWER_MODE_POWER_SAVE);
    } else {
        // Low battery level, use ultra low power mode
        setPowerMode(POWER_MODE_ULTRA_LOW);
    }
}

/**
 * Disable unused peripherals to save power
 */
static inline void disableUnusedPeripherals() {
    // Disable WiFi if not needed
    // esp_wifi_stop();
    // esp_wifi_deinit();
    
    // Note: BLE is typically needed for OpenGlass, so we don't disable it
    // If you want to disable BLE: esp_bt_controller_disable();
    
    Serial.println("Unused peripherals disabled for power saving");
}

/**
 * Enable peripherals when needed
 */
static inline void enablePeripherals() {
    // Re-enable WiFi if needed
    // esp_wifi_init();
    // esp_wifi_start();
    
    Serial.println("Peripherals enabled");
}

/**
 * Calculate estimated battery life
 * @param battery_capacity_mah Battery capacity in mAh
 * @param current_level Current battery level (0-100)
 * @return Estimated battery life in hours
 */
static inline float estimateBatteryLife(uint16_t battery_capacity_mah, uint8_t current_level) {
    if (currentPowerStats.current_ma <= 0) {
        return 0;
    }
    
    float remaining_capacity = (battery_capacity_mah * current_level) / 100.0;
    return remaining_capacity / currentPowerStats.current_ma;
}

/**
 * Check if device should enter power saving mode
 * @param battery_level Current battery level
 * @param idle_time_ms Time since last activity
 * @return true if should enter power saving mode
 */
static inline bool shouldEnterPowerSaving(uint8_t battery_level, unsigned long idle_time_ms) {
    // Enter power saving if battery is low OR device has been idle
    return (battery_level < 30) || (idle_time_ms > 300000); // 5 minutes idle
}

/**
 * Prepare for sleep mode
 * Saves current state and configures wake-up sources
 */
static inline void prepareForSleep() {
    // Flush serial output
    Serial.flush();
    
    // Save current power mode
    // (This would be expanded to save more state as needed)
    
    Serial.println("Device prepared for sleep");
}

/**
 * Wake up from sleep mode
 * Restores state and re-initializes peripherals
 */
static inline void wakeFromSleep() {
    // Restore power mode
    // (This would be expanded to restore more state as needed)
    
    Serial.println("Device woke from sleep");
    
    // Re-initialize peripherals if needed
    // enablePeripherals();
}

/**
 * Get wake-up reason after sleep
 * @return String describing wake-up reason
 */
static inline const char* getWakeupReason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            return "External signal using RTC_IO";
        case ESP_SLEEP_WAKEUP_EXT1:
            return "External signal using RTC_CNTL";
        case ESP_SLEEP_WAKEUP_TIMER:
            return "Timer";
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            return "Touchpad";
        case ESP_SLEEP_WAKEUP_ULP:
            return "ULP program";
        default:
            return "Reset or power-on";
    }
}

// Power management constants
#define POWER_IDLE_TIMEOUT_MS 300000        // 5 minutes
#define POWER_SLEEP_TIMEOUT_MS 600000       // 10 minutes
#define POWER_DEEP_SLEEP_TIMEOUT_MS 3600000 // 1 hour

// Battery level thresholds
#define BATTERY_LEVEL_CRITICAL 10
#define BATTERY_LEVEL_LOW 20
#define BATTERY_LEVEL_MEDIUM 50
#define BATTERY_LEVEL_HIGH 80

// Power consumption estimates (mA)
#define POWER_CONSUMPTION_IDLE 19
#define POWER_CONSUMPTION_LIGHT_SLEEP 2
#define POWER_CONSUMPTION_DEEP_SLEEP 0.014

#endif // POWER_MANAGEMENT_H 