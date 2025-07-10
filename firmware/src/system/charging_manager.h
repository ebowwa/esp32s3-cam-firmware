#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "../platform/constants.h"
#include "../platform/xiao_esp32s3_constants.h"

// ===================================================================
// ADVANCED CHARGING MANAGEMENT SYSTEM
// ===================================================================

/**
 * Detailed charging states
 */
typedef enum {
    CHARGING_STATE_NOT_CHARGING = 0,    // No USB power or battery full
    CHARGING_STATE_TRICKLE = 1,         // Trickle charge (very low battery)
    CHARGING_STATE_CC = 2,              // Constant Current phase
    CHARGING_STATE_CV = 3,              // Constant Voltage phase
    CHARGING_STATE_FULL = 4,            // Battery full, maintenance mode
    CHARGING_STATE_ERROR = 5,           // Charging error detected
    CHARGING_STATE_TIMEOUT = 6,         // Charging timeout
    CHARGING_STATE_THERMAL_LIMIT = 7,   // Temperature too high
    CHARGING_STATE_UNKNOWN = 8          // Unknown state
} charging_state_t;

/**
 * Charging safety status
 */
typedef enum {
    CHARGING_SAFETY_OK = 0,
    CHARGING_SAFETY_OVERVOLTAGE = 1,
    CHARGING_SAFETY_OVERCURRENT = 2,
    CHARGING_SAFETY_OVERTEMPERATURE = 3,
    CHARGING_SAFETY_TIMEOUT = 4,
    CHARGING_SAFETY_BATTERY_FAULT = 5
} charging_safety_t;

/**
 * Charging profile settings
 */
typedef struct {
    float max_voltage;          // Maximum charging voltage (V)
    float max_current;          // Maximum charging current (mA)
    float trickle_voltage;      // Voltage threshold for trickle charge (V)
    float trickle_current;      // Trickle charge current (mA)
    float cv_threshold;         // Voltage threshold for CV phase (V)
    float full_threshold;       // Current threshold for full detection (mA)
    uint32_t timeout_ms;        // Charging timeout (ms)
    float temp_max;             // Maximum charging temperature (°C)
    float temp_min;             // Minimum charging temperature (°C)
} charging_profile_t;

/**
 * Charging statistics
 */
typedef struct {
    charging_state_t state;
    charging_safety_t safety_status;
    float voltage;              // Current battery voltage (V)
    float current;              // Current charging current (mA)
    float power;                // Charging power (mW)
    float temperature;          // Battery temperature (°C)
    uint32_t time_elapsed;      // Time since charging started (ms)
    uint32_t time_remaining;    // Estimated time to full (ms)
    uint8_t charge_level;       // Battery charge level (0-100%)
    uint32_t charge_cycles;     // Total charge cycles
    float health_percentage;    // Battery health (0-100%)
    bool usb_connected;         // USB power status
    float usb_voltage;          // USB voltage (V)
} charging_stats_t;

/**
 * Charging history entry
 */
typedef struct {
    uint32_t timestamp;         // Charge start timestamp
    uint32_t duration;          // Charging duration (ms)
    uint8_t start_level;        // Starting battery level
    uint8_t end_level;          // Ending battery level
    float avg_current;          // Average charging current
    float max_temperature;      // Maximum temperature during charge
    charging_state_t final_state; // Final charging state
} charging_history_entry_t;

// Charging Service UUIDs
#define CHARGING_SERVICE_UUID           "19b10010-e8f2-537e-4f6c-d104768a1214"
#define CHARGING_STATE_CHAR_UUID        "19b10011-e8f2-537e-4f6c-d104768a1214"
#define CHARGING_STATS_CHAR_UUID        "19b10012-e8f2-537e-4f6c-d104768a1214"
#define CHARGING_PROFILE_CHAR_UUID      "19b10013-e8f2-537e-4f6c-d104768a1214"
#define CHARGING_CONTROL_CHAR_UUID      "19b10014-e8f2-537e-4f6c-d104768a1214"

// Charging constants
#define CHARGING_UPDATE_INTERVAL        5000    // 5 seconds
#define CHARGING_HISTORY_SIZE           10      // Keep last 10 charge cycles
#define CHARGING_SAFETY_CHECK_INTERVAL  1000    // 1 second
#define CHARGING_TIMEOUT_DEFAULT        (4 * 60 * 60 * 1000) // 4 hours
#define CHARGING_TEMPERATURE_SAMPLES    5       // Temperature averaging samples

// Default charging profile (Li-ion 3.7V)
#define CHARGING_MAX_VOLTAGE            4.2f    // 4.2V max
#define CHARGING_MAX_CURRENT            500.0f  // 500mA max
#define CHARGING_TRICKLE_VOLTAGE        3.0f    // Start trickle at 3.0V
#define CHARGING_TRICKLE_CURRENT        50.0f   // 50mA trickle
#define CHARGING_CV_THRESHOLD           4.1f    // CV phase at 4.1V
#define CHARGING_FULL_THRESHOLD         50.0f   // Full when current < 50mA
#define CHARGING_TEMP_MAX               45.0f   // 45°C max
#define CHARGING_TEMP_MIN               0.0f    // 0°C min

// Current sensing configuration
#define CHARGING_CURRENT_SENSE_PIN      XIAO_ESP32S3_PIN_A2  // GPIO4 for current sensing
#define CHARGING_CURRENT_SENSE_RESISTOR 0.1f    // 0.1 ohm sense resistor
#define CHARGING_CURRENT_AMPLIFIER_GAIN 50.0f   // Op-amp gain

// External variables
extern BLECharacteristic *chargingStateCharacteristic;
extern BLECharacteristic *chargingStatsCharacteristic;
extern BLECharacteristic *chargingProfileCharacteristic;
extern BLECharacteristic *chargingControlCharacteristic;

extern charging_stats_t chargingStats;
extern charging_profile_t chargingProfile;
extern charging_history_entry_t chargingHistory[CHARGING_HISTORY_SIZE];
extern int chargingHistoryIndex;
extern unsigned long lastChargingUpdate;
extern unsigned long chargingStartTime;
extern bool chargingEnabled;

// Function declarations
void initializeChargingManager();
void setupChargingService(BLEServer *server);
void updateChargingStatus();
void checkUSBConnectionEvent();
charging_state_t determineChargingState();
charging_safety_t checkChargingSafety();
float readChargingCurrent();
float readBatteryTemperature();
void optimizeChargingProfile();
void recordChargingHistory();
void handleChargingControl(uint8_t command);
const char* getChargingStateString(charging_state_t state);
const char* getChargingSafetyString(charging_safety_t safety);
uint32_t estimateTimeToFull();
void enableCharging(bool enable);
void setChargingProfile(const charging_profile_t* profile);
void resetChargingStats();
void printChargingStats();
bool isChargingSafe();
void handleChargingError(charging_safety_t error);
void updateChargingLED();
void notifyChargingClients(); 