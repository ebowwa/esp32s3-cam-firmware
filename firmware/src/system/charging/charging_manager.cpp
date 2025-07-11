#include "charging_manager.h"
#include "../../features/led/led_manager.h"
#include "../device_status.h"
#include "../battery/battery_code.h"

// BLE Characteristics
BLECharacteristic *chargingStateCharacteristic = nullptr;
BLECharacteristic *chargingStatsCharacteristic = nullptr;
BLECharacteristic *chargingProfileCharacteristic = nullptr;
BLECharacteristic *chargingControlCharacteristic = nullptr;

// Charging state variables
charging_stats_t chargingStats = {};
charging_profile_t chargingProfile = {};
charging_history_entry_t chargingHistory[CHARGING_HISTORY_SIZE] = {};
int chargingHistoryIndex = 0;
unsigned long lastChargingUpdate = 0;
unsigned long chargingStartTime = 0;
bool chargingEnabled = true;

// Internal variables
static float temperatureHistory[CHARGING_TEMPERATURE_SAMPLES] = {0};
static int temperatureHistoryIndex = 0;
static unsigned long lastSafetyCheck = 0;
static charging_state_t previousState = CHARGING_STATE_UNKNOWN;
static float currentHistory[10] = {0};
static int currentHistoryIndex = 0;
static bool previousUsbConnected = false;

void initializeChargingManager() {
    Serial.println("Initializing Advanced Charging Manager...");
    
    // Initialize charging profile with defaults
    chargingProfile.max_voltage = CHARGING_MAX_VOLTAGE;
    chargingProfile.max_current = CHARGING_MAX_CURRENT;
    chargingProfile.trickle_voltage = CHARGING_TRICKLE_VOLTAGE;
    chargingProfile.trickle_current = CHARGING_TRICKLE_CURRENT;
    chargingProfile.cv_threshold = CHARGING_CV_THRESHOLD;
    chargingProfile.full_threshold = CHARGING_FULL_THRESHOLD;
    chargingProfile.timeout_ms = CHARGING_TIMEOUT_DEFAULT;
    chargingProfile.temp_max = CHARGING_TEMP_MAX;
    chargingProfile.temp_min = CHARGING_TEMP_MIN;
    
    // Initialize charging stats
    chargingStats.state = CHARGING_STATE_NOT_CHARGING;
    chargingStats.safety_status = CHARGING_SAFETY_OK;
    chargingStats.health_percentage = 100.0f;
    chargingStats.charge_cycles = 0;
    
    // Initialize temperature history
    for (int i = 0; i < CHARGING_TEMPERATURE_SAMPLES; i++) {
        temperatureHistory[i] = 25.0f; // Default to 25°C
    }
    
    Serial.println("Advanced Charging Manager initialized");
}

void setupChargingService(BLEServer *server) {
    Serial.println("Setting up Charging BLE Service...");
    
    BLEService *chargingService = server->createService(CHARGING_SERVICE_UUID);
    
    // Charging State Characteristic
    chargingStateCharacteristic = chargingService->createCharacteristic(
        CHARGING_STATE_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    
    BLE2902 *stateCcc = new BLE2902();
    stateCcc->setNotifications(true);
    chargingStateCharacteristic->addDescriptor(stateCcc);
    
    // Charging Stats Characteristic
    chargingStatsCharacteristic = chargingService->createCharacteristic(
        CHARGING_STATS_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    
    BLE2902 *statsCcc = new BLE2902();
    statsCcc->setNotifications(true);
    chargingStatsCharacteristic->addDescriptor(statsCcc);
    
    // Charging Profile Characteristic
    chargingProfileCharacteristic = chargingService->createCharacteristic(
        CHARGING_PROFILE_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    
    // Charging Control Characteristic
    chargingControlCharacteristic = chargingService->createCharacteristic(
        CHARGING_CONTROL_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    
    chargingService->start();
    
    Serial.println("Charging BLE Service started");
}

void updateChargingStatus() {
    // Always check USB connection status immediately (not time-based)
    checkUSBConnectionEvent();
    
    // Only do full charging update on timer
    if (millis() - lastChargingUpdate < CHARGING_UPDATE_INTERVAL) {
        return;
    }
    
    Serial.println("🔍 Full charging status update...");
    
    // Update basic measurements
    chargingStats.voltage = readBatteryVoltage();
    chargingStats.current = readChargingCurrent();
    chargingStats.power = chargingStats.voltage * chargingStats.current;
    chargingStats.temperature = readBatteryTemperature();
    chargingStats.charge_level = batteryLevel;
    
    // Determine charging state
    charging_state_t newState = determineChargingState();
    
    // Check for state changes
    if (newState != chargingStats.state) {
        Serial.printf("Charging state changed: %s -> %s\n", 
                     getChargingStateString(chargingStats.state),
                     getChargingStateString(newState));
        
        // Handle state transitions
        if (chargingStats.state == CHARGING_STATE_NOT_CHARGING && 
            newState != CHARGING_STATE_NOT_CHARGING) {
            // Charging started
            chargingStartTime = millis();
            Serial.println("🔋 Charging started");
        } else if (chargingStats.state != CHARGING_STATE_NOT_CHARGING && 
                   newState == CHARGING_STATE_NOT_CHARGING) {
            // Charging stopped
            recordChargingHistory();
            Serial.println("🔋 Charging stopped");
        }
        
        chargingStats.state = newState;
        updateChargingLED();
    }
    
    // Update timing
    if (chargingStats.state != CHARGING_STATE_NOT_CHARGING) {
        chargingStats.time_elapsed = millis() - chargingStartTime;
        chargingStats.time_remaining = estimateTimeToFull();
    } else {
        chargingStats.time_elapsed = 0;
        chargingStats.time_remaining = 0;
    }
    
    // Safety checks
    if (millis() - lastSafetyCheck >= CHARGING_SAFETY_CHECK_INTERVAL) {
        chargingStats.safety_status = checkChargingSafety();
        lastSafetyCheck = millis();
        
        if (chargingStats.safety_status != CHARGING_SAFETY_OK) {
            handleChargingError(chargingStats.safety_status);
        }
    }
    
    // Notify BLE clients
    notifyChargingClients();
    
    lastChargingUpdate = millis();
}

charging_state_t determineChargingState() {
    if (!chargingEnabled) {
        return CHARGING_STATE_NOT_CHARGING;
    }
    
    // Check if USB power is connected
    if (!chargingStats.usb_connected) {
        return CHARGING_STATE_NOT_CHARGING;
    }
    
    // Check safety first
    if (chargingStats.safety_status != CHARGING_SAFETY_OK) {
        return CHARGING_STATE_ERROR;
    }
    
    // Check for timeout
    if (chargingStats.time_elapsed > chargingProfile.timeout_ms) {
        return CHARGING_STATE_TIMEOUT;
    }
    
    // Check for thermal limiting
    if (chargingStats.temperature > chargingProfile.temp_max ||
        chargingStats.temperature < chargingProfile.temp_min) {
        return CHARGING_STATE_THERMAL_LIMIT;
    }
    
    // Determine charging phase based on voltage and current
    if (chargingStats.voltage < chargingProfile.trickle_voltage) {
        return CHARGING_STATE_TRICKLE;
    } else if (chargingStats.voltage < chargingProfile.cv_threshold) {
        return CHARGING_STATE_CC; // Constant Current
    } else if (chargingStats.current > chargingProfile.full_threshold) {
        return CHARGING_STATE_CV; // Constant Voltage
    } else {
        return CHARGING_STATE_FULL; // Battery full
    }
}

charging_safety_t checkChargingSafety() {
    // Check overvoltage
    if (chargingStats.voltage > chargingProfile.max_voltage + 0.1f) {
        return CHARGING_SAFETY_OVERVOLTAGE;
    }
    
    // Check overcurrent
    if (chargingStats.current > chargingProfile.max_current + 100.0f) {
        return CHARGING_SAFETY_OVERCURRENT;
    }
    
    // Check temperature
    if (chargingStats.temperature > chargingProfile.temp_max + 5.0f) {
        return CHARGING_SAFETY_OVERTEMPERATURE;
    }
    
    // Check timeout
    if (chargingStats.time_elapsed > chargingProfile.timeout_ms) {
        return CHARGING_SAFETY_TIMEOUT;
    }
    
    // Check for battery fault (rapid voltage drops)
    static float lastVoltage = 0;
    if (lastVoltage > 0 && (lastVoltage - chargingStats.voltage) > 0.5f) {
        return CHARGING_SAFETY_BATTERY_FAULT;
    }
    lastVoltage = chargingStats.voltage;
    
    return CHARGING_SAFETY_OK;
}

float readChargingCurrent() {
    // Read current from current sense resistor
    // This assumes a current sense resistor and amplifier setup
    uint32_t adcSum = 0;
    for (int i = 0; i < 5; i++) {
        adcSum += analogRead(CHARGING_CURRENT_SENSE_PIN);
        delay(2);
    }
    
    float averageAdc = (float)adcSum / 5.0f;
    float adcVoltage = (averageAdc / XIAO_ESP32S3_ADC_MAX_VALUE) * XIAO_ESP32S3_ADC_VREF_MV / 1000.0f;
    
    // Convert to current: V = I * R * Gain
    float current = (adcVoltage / CHARGING_CURRENT_AMPLIFIER_GAIN) / CHARGING_CURRENT_SENSE_RESISTOR;
    current *= 1000.0f; // Convert to mA
    
    // Store in history for smoothing
    currentHistory[currentHistoryIndex] = current;
    currentHistoryIndex = (currentHistoryIndex + 1) % 10;
    
    // Return smoothed current
    float smoothedCurrent = 0;
    for (int i = 0; i < 10; i++) {
        smoothedCurrent += currentHistory[i];
    }
    return smoothedCurrent / 10.0f;
}

float readBatteryTemperature() {
    // For now, estimate temperature based on charging current and ambient
    // In a real implementation, you'd use a temperature sensor
    float baseTemp = 25.0f; // Ambient temperature
    float thermalRise = chargingStats.current * 0.01f; // Rough estimate
    
    float currentTemp = baseTemp + thermalRise;
    
    // Store in history for smoothing
    temperatureHistory[temperatureHistoryIndex] = currentTemp;
    temperatureHistoryIndex = (temperatureHistoryIndex + 1) % CHARGING_TEMPERATURE_SAMPLES;
    
    // Return smoothed temperature
    float smoothedTemp = 0;
    for (int i = 0; i < CHARGING_TEMPERATURE_SAMPLES; i++) {
        smoothedTemp += temperatureHistory[i];
    }
    return smoothedTemp / CHARGING_TEMPERATURE_SAMPLES;
}

uint32_t estimateTimeToFull() {
    if (chargingStats.state == CHARGING_STATE_NOT_CHARGING || 
        chargingStats.state == CHARGING_STATE_FULL) {
        return 0;
    }
    
    if (chargingStats.current <= 0) {
        return 0; // No charging current
    }
    
    // Estimate based on current charge level and charging current
    float remainingCapacity = (100 - chargingStats.charge_level) / 100.0f;
    float estimatedCapacity = 1000.0f; // Assume 1000mAh battery
    float remainingCharge = remainingCapacity * estimatedCapacity;
    
    // Time = Capacity / Current (in hours), then convert to ms
    float timeHours = remainingCharge / chargingStats.current;
    return (uint32_t)(timeHours * 3600000); // Convert to milliseconds
}

void recordChargingHistory() {
    charging_history_entry_t *entry = &chargingHistory[chargingHistoryIndex];
    
    entry->timestamp = millis();
    entry->duration = chargingStats.time_elapsed;
    entry->start_level = 0; // Would need to track this
    entry->end_level = chargingStats.charge_level;
    entry->avg_current = chargingStats.current; // Simplified
    entry->max_temperature = chargingStats.temperature; // Simplified
    entry->final_state = chargingStats.state;
    
    chargingHistoryIndex = (chargingHistoryIndex + 1) % CHARGING_HISTORY_SIZE;
    chargingStats.charge_cycles++;
    
    Serial.printf("Charging cycle recorded: %d%% in %lums\n", 
                  entry->end_level, entry->duration);
}

void handleChargingError(charging_safety_t error) {
    Serial.printf("⚠️ Charging safety error: %s\n", getChargingSafetyString(error));
    
    switch (error) {
        case CHARGING_SAFETY_OVERVOLTAGE:
        case CHARGING_SAFETY_OVERCURRENT:
        case CHARGING_SAFETY_OVERTEMPERATURE:
            // Disable charging for safety
            chargingEnabled = false;
            Serial.println("🚨 Charging disabled for safety");
            break;
        case CHARGING_SAFETY_TIMEOUT:
            // Just log, don't disable
            Serial.println("⏰ Charging timeout reached");
            break;
        case CHARGING_SAFETY_BATTERY_FAULT:
            // Disable charging
            chargingEnabled = false;
            Serial.println("🚨 Battery fault detected, charging disabled");
            break;
        default:
            break;
    }
    
    // Update device status
    updateDeviceStatus(DEVICE_STATUS_ERROR);
}

void updateChargingLED() {
    switch (chargingStats.state) {
        case CHARGING_STATE_TRICKLE:
            setLedPattern(LED_PULSE, (rgb_color_t)LED_COLOR_ORANGE, (rgb_color_t)LED_COLOR_YELLOW);
            break;
        case CHARGING_STATE_CC:
            setLedPattern(LED_PULSE, (rgb_color_t)LED_COLOR_YELLOW, (rgb_color_t)LED_COLOR_GREEN);
            break;
        case CHARGING_STATE_CV:
            setLedPattern(LED_BLINK_SLOW, (rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_BLUE);
            break;
        case CHARGING_STATE_FULL:
            setLedPattern(LED_ON, (rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_GREEN);
            break;
        case CHARGING_STATE_ERROR:
        case CHARGING_STATE_THERMAL_LIMIT:
            setLedPattern(LED_ERROR, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_ORANGE);
            break;
        case CHARGING_STATE_TIMEOUT:
            setLedPattern(LED_SOS, (rgb_color_t)LED_COLOR_YELLOW, (rgb_color_t)LED_COLOR_RED);
            break;
        default:
            // Use default charging pattern
            setLedPattern(LED_PULSE, (rgb_color_t)LED_COLOR_YELLOW, (rgb_color_t)LED_COLOR_GREEN);
            break;
    }
}

void notifyChargingClients() {
    if (chargingStateCharacteristic) {
        uint8_t state = (uint8_t)chargingStats.state;
        chargingStateCharacteristic->setValue(&state, 1);
        chargingStateCharacteristic->notify();
    }
    
    if (chargingStatsCharacteristic) {
        // Pack charging stats into a byte array for BLE transmission
        uint8_t statsData[32];
        memcpy(statsData, &chargingStats, sizeof(chargingStats));
        chargingStatsCharacteristic->setValue(statsData, sizeof(statsData));
        chargingStatsCharacteristic->notify();
    }
}

void printChargingStats() {
    Serial.println("=== Charging Statistics ===");
    Serial.printf("State: %s\n", getChargingStateString(chargingStats.state));
    Serial.printf("Safety: %s\n", getChargingSafetyString(chargingStats.safety_status));
    Serial.printf("Voltage: %.2f V\n", chargingStats.voltage);
    Serial.printf("Current: %.1f mA\n", chargingStats.current);
    Serial.printf("Power: %.1f mW\n", chargingStats.power);
    Serial.printf("Temperature: %.1f °C\n", chargingStats.temperature);
    Serial.printf("USB Voltage: %.2f V\n", chargingStats.usb_voltage);
    Serial.printf("Charge Level: %d%%\n", chargingStats.charge_level);
    Serial.printf("Time Elapsed: %lu ms\n", chargingStats.time_elapsed);
    Serial.printf("Time Remaining: %lu ms\n", chargingStats.time_remaining);
    Serial.printf("Charge Cycles: %lu\n", chargingStats.charge_cycles);
    Serial.printf("Battery Health: %.1f%%\n", chargingStats.health_percentage);
    Serial.println("===========================");
}

const char* getChargingStateString(charging_state_t state) {
    switch (state) {
        case CHARGING_STATE_NOT_CHARGING: return "NOT_CHARGING";
        case CHARGING_STATE_TRICKLE: return "TRICKLE";
        case CHARGING_STATE_CC: return "CONSTANT_CURRENT";
        case CHARGING_STATE_CV: return "CONSTANT_VOLTAGE";
        case CHARGING_STATE_FULL: return "FULL";
        case CHARGING_STATE_ERROR: return "ERROR";
        case CHARGING_STATE_TIMEOUT: return "TIMEOUT";
        case CHARGING_STATE_THERMAL_LIMIT: return "THERMAL_LIMIT";
        default: return "UNKNOWN";
    }
}

const char* getChargingSafetyString(charging_safety_t safety) {
    switch (safety) {
        case CHARGING_SAFETY_OK: return "OK";
        case CHARGING_SAFETY_OVERVOLTAGE: return "OVERVOLTAGE";
        case CHARGING_SAFETY_OVERCURRENT: return "OVERCURRENT";
        case CHARGING_SAFETY_OVERTEMPERATURE: return "OVERTEMPERATURE";
        case CHARGING_SAFETY_TIMEOUT: return "TIMEOUT";
        case CHARGING_SAFETY_BATTERY_FAULT: return "BATTERY_FAULT";
        default: return "UNKNOWN";
    }
}

void enableCharging(bool enable) {
    chargingEnabled = enable;
    Serial.printf("Charging %s\n", enable ? "enabled" : "disabled");
}

bool isChargingSafe() {
    return chargingStats.safety_status == CHARGING_SAFETY_OK;
}

void handleChargingControl(uint8_t command) {
    switch (command) {
        case 0x01: // Enable charging
            enableCharging(true);
            break;
        case 0x02: // Disable charging
            enableCharging(false);
            break;
        case 0x03: // Reset charging stats
            resetChargingStats();
            break;
        case 0x04: // Print charging stats
            printChargingStats();
            break;
        default:
            Serial.printf("Unknown charging control command: 0x%02X\n", command);
            break;
    }
}

void resetChargingStats() {
    chargingStats.charge_cycles = 0;
    chargingStats.health_percentage = 100.0f;
    chargingHistoryIndex = 0;
    memset(chargingHistory, 0, sizeof(chargingHistory));
    Serial.println("Charging stats reset");
}

void checkUSBConnectionEvent() {
    // Read USB voltage immediately (not time-based)
    chargingStats.usb_voltage = readUSBPowerVoltage();
    chargingStats.usb_connected = (chargingStats.usb_voltage >= USB_POWER_THRESHOLD);
    
    // Check for USB connection event (plugged in)
    if (chargingStats.usb_connected && !previousUsbConnected) {
        Serial.println("🔌 USB POWER CONNECTED - FLASHING GREEN!");
        Serial.printf("USB voltage detected: %.2fV\n", chargingStats.usb_voltage);
        
        // Flash green LEDs immediately to indicate USB connection
        Serial.println("Flash 1...");
        setDualLedColors((rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_LIME);
        delay(200);
        setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
        delay(100);
        
        Serial.println("Flash 2...");
        setDualLedColors((rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_LIME);
        delay(200);
        setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
        delay(100);
        
        Serial.println("Flash 3...");
        setDualLedColors((rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_LIME);
        delay(200);
        setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
        
        Serial.println("✅ Green flash sequence completed!");
        
        // Trigger immediate full charging status update
        lastChargingUpdate = 0; // Force immediate update
    }
    
    // Check for USB disconnection event
    if (!chargingStats.usb_connected && previousUsbConnected) {
        Serial.println("🔌 USB power disconnected");
    }
    
    // Update USB connection state for next check
    previousUsbConnected = chargingStats.usb_connected;
} 