#include "system/battery_code.h"
#include "platform/xiao_esp32s3_constants.h"

BLECharacteristic *batteryLevelCharacteristic = nullptr;
uint8_t batteryLevel = 100;
unsigned long lastBatteryUpdate = 0;
bool batteryDetected = false;
bool isCharging = false;

// Battery connection monitoring variables
float batteryVoltageHistory[BATTERY_STABILITY_SAMPLES] = {0};
int voltageHistoryIndex = 0;
int unstableReadingCount = 0;
float lastStableVoltage = 0.0;
bool connectionStable = true;
unsigned long lastVoltageChangeTime = 0;

void setupBatteryService(BLEServer *server) {
    BLEService *batteryService = server->createService(BATTERY_SERVICE_UUID);
    batteryLevelCharacteristic = batteryService->createCharacteristic(
        BATTERY_LEVEL_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

    BLE2902 *ccc = new BLE2902();
    ccc->setNotifications(true);
    batteryLevelCharacteristic->addDescriptor(ccc);
    batteryLevelCharacteristic->setValue(&batteryLevel, 1);

    batteryService->start();
    
    // Initialize voltage history
    for (int i = 0; i < BATTERY_STABILITY_SAMPLES; i++) {
        batteryVoltageHistory[i] = 0.0;
    }
}

bool detectRapidVoltageChange(float currentVoltage) {
    if (lastStableVoltage == 0.0) {
        lastStableVoltage = currentVoltage;
        return false;
    }
    
    float voltageChange = abs(currentVoltage - lastStableVoltage);
    unsigned long currentTime = millis();
    
    // Check for rapid voltage changes (possible connection issue)
    if (voltageChange > BATTERY_RAPID_CHANGE_THRESHOLD) {
        if (currentTime - lastVoltageChangeTime < 5000) { // Within 5 seconds
            Serial.printf("⚠️  Rapid voltage change detected: %.2fV → %.2fV (Δ%.2fV)\n", 
                         lastStableVoltage, currentVoltage, voltageChange);
            return true;
        }
        lastVoltageChangeTime = currentTime;
    }
    
    return false;
}

bool analyzeBatteryConnectionStability(float currentVoltage) {
    // Store voltage in circular buffer
    batteryVoltageHistory[voltageHistoryIndex] = currentVoltage;
    voltageHistoryIndex = (voltageHistoryIndex + 1) % BATTERY_STABILITY_SAMPLES;
    
    // Check for rapid voltage changes
    bool rapidChange = detectRapidVoltageChange(currentVoltage);
    
    // Calculate voltage stability over recent samples
    float minVoltage = 4.3, maxVoltage = 0.0;
    int validSamples = 0;
    
    for (int i = 0; i < BATTERY_STABILITY_SAMPLES; i++) {
        if (batteryVoltageHistory[i] > 0.0) {
            validSamples++;
            if (batteryVoltageHistory[i] < minVoltage) minVoltage = batteryVoltageHistory[i];
            if (batteryVoltageHistory[i] > maxVoltage) maxVoltage = batteryVoltageHistory[i];
        }
    }
    
    if (validSamples < 3) return true; // Not enough data yet, assume stable
    
    float voltageSpread = maxVoltage - minVoltage;
    bool isStable = (voltageSpread <= BATTERY_VOLTAGE_TOLERANCE) && !rapidChange;
    
    if (!isStable) {
        unstableReadingCount++;
        Serial.printf("🔍 Unstable reading #%d: spread=%.2fV (%.2f-%.2fV), rapid=%s\n", 
                     unstableReadingCount, voltageSpread, minVoltage, maxVoltage, 
                     rapidChange ? "YES" : "NO");
    } else {
        unstableReadingCount = 0; // Reset counter on stable reading
        lastStableVoltage = currentVoltage;
    }
    
    connectionStable = (unstableReadingCount < BATTERY_INSTABILITY_COUNT);
    return connectionStable;
}

const char* getBatteryConnectionStatus() {
    if (!batteryDetected) {
        return "DISCONNECTED";
    } else if (!connectionStable) {
        return "UNSTABLE_CONNECTION";
    } else if (isCharging) {
        return "CHARGING_STABLE";
    } else {
        return "CONNECTED_STABLE";
    }
}

float readUSBPowerVoltage() {
    // Take multiple samples for more accurate reading
    uint32_t adcSum = 0;
    for (int i = 0; i < BATTERY_CHECK_SAMPLES; i++) {
        adcSum += analogRead(USB_POWER_ADC_PIN);
        delay(5); // Small delay between samples
    }
    
    // Calculate average ADC reading
    float averageAdc = (float)adcSum / BATTERY_CHECK_SAMPLES;
    
    // Convert ADC reading to voltage (before voltage divider)
    float adcVoltage = (averageAdc / BATTERY_ADC_RESOLUTION) * BATTERY_REFERENCE_VOLTAGE;
    
    // Account for voltage divider to get actual USB voltage
    float usbVoltage = adcVoltage * USB_POWER_VOLTAGE_DIVIDER;
    
    // Debug information
    Serial.printf("USB ADC raw: %.0f, ADC voltage: %.2fV, USB voltage: %.2fV\n", 
                  averageAdc, adcVoltage, usbVoltage);
    
    return usbVoltage;
}

bool checkChargingStatus() {
    // First check if USB power is present
    float usbVoltage = readUSBPowerVoltage();
    bool usbPowerPresent = (usbVoltage >= USB_POWER_THRESHOLD);
    
    if (!usbPowerPresent) {
        Serial.println("No USB power detected - not charging");
        return false;
    }
    
    // If USB power is present, check battery voltage to determine charging state
    float batteryVoltage = readBatteryVoltage();
    
    // If battery voltage is high and USB is connected, likely charging or charged
    bool chargingDetected = (batteryVoltage >= CHARGING_VOLTAGE_THRESHOLD);
    
    Serial.printf("USB power: %.2fV, Battery: %.2fV, Charging: %s\n", 
                  usbVoltage, batteryVoltage, chargingDetected ? "YES" : "NO");
    
    return chargingDetected;
}

float readBatteryVoltage() {
    // Take multiple samples for more accurate reading
    uint32_t adcSum = 0;
    for (int i = 0; i < BATTERY_CHECK_SAMPLES; i++) {
        adcSum += analogRead(BATTERY_ADC_PIN);
        delay(10); // Small delay between samples
    }
    
    // Calculate average ADC reading
    float averageAdc = (float)adcSum / BATTERY_CHECK_SAMPLES;
    
    // Convert ADC reading to voltage (before voltage divider)
    float adcVoltage = (averageAdc / BATTERY_ADC_RESOLUTION) * BATTERY_REFERENCE_VOLTAGE;
    
    // Account for voltage divider if present
    float batteryVoltage = adcVoltage * BATTERY_VOLTAGE_DIVIDER;
    
    // Debug information
    Serial.printf("ADC raw: %.0f, ADC voltage: %.2fV, Battery voltage: %.2fV\n", 
                  averageAdc, adcVoltage, batteryVoltage);
    
    return batteryVoltage;
}

bool checkBatteryPresence() {
    float batteryVoltage = readBatteryVoltage();
    
    Serial.printf("Battery voltage reading: %.2fV\n", batteryVoltage);
    
    // Analyze connection stability
    bool isStable = analyzeBatteryConnectionStability(batteryVoltage);
    
    // Check if voltage is within expected battery range
    bool isPresent = (batteryVoltage >= BATTERY_MIN_VOLTAGE && batteryVoltage <= BATTERY_MAX_VOLTAGE);
    
    if (isPresent) {
        Serial.printf("Battery detected: %.2fV (%s)\n", batteryVoltage, 
                     isStable ? "STABLE" : "UNSTABLE");
        
        // More accurate Li-ion battery level calculation
        // Based on typical Li-ion discharge curve:
        // 4.2V = 100%, 4.0V = ~75%, 3.8V = ~50%, 3.6V = ~25%, 3.0V = 0%
        
        if (batteryVoltage >= 4.15) {
            // 4.15V - 4.2V+ = 95-100%
            batteryLevel = (uint8_t)(95 + ((batteryVoltage - 4.15) / 0.05) * 5);
        } else if (batteryVoltage >= 4.0) {
            // 4.0V - 4.15V = 75-95%
            batteryLevel = (uint8_t)(75 + ((batteryVoltage - 4.0) / 0.15) * 20);
        } else if (batteryVoltage >= 3.8) {
            // 3.8V - 4.0V = 50-75%
            batteryLevel = (uint8_t)(50 + ((batteryVoltage - 3.8) / 0.2) * 25);
        } else if (batteryVoltage >= 3.6) {
            // 3.6V - 3.8V = 25-50%
            batteryLevel = (uint8_t)(25 + ((batteryVoltage - 3.6) / 0.2) * 25);
        } else if (batteryVoltage >= 3.4) {
            // 3.4V - 3.6V = 10-25%
            batteryLevel = (uint8_t)(10 + ((batteryVoltage - 3.4) / 0.2) * 15);
        } else if (batteryVoltage >= 3.2) {
            // 3.2V - 3.4V = 5-10%
            batteryLevel = (uint8_t)(5 + ((batteryVoltage - 3.2) / 0.2) * 5);
        } else if (batteryVoltage >= 3.0) {
            // 3.0V - 3.2V = 0-5%
            batteryLevel = (uint8_t)((batteryVoltage - 3.0) / 0.2 * 5);
        } else {
            batteryLevel = 0;
        }
        
        // Ensure battery level is within bounds
        if (batteryLevel > 100) batteryLevel = 100;
        if (batteryLevel < 0) batteryLevel = 0;
        
        Serial.printf("Calculated battery level: %d%% (%.2fV) - Connection: %s\n", 
                     batteryLevel, batteryVoltage, getBatteryConnectionStatus());
    } else {
        Serial.printf("No battery detected or battery voltage out of range: %.2fV\n", batteryVoltage);
        batteryLevel = 0;
        
        // Reset stability tracking when battery is disconnected
        unstableReadingCount = 0;
        connectionStable = true;
        lastStableVoltage = 0.0;
    }
    
    return isPresent;
}

void updateBatteryLevel() {
    if (!batteryLevelCharacteristic) return;
    
    // Check battery presence and update level
    batteryDetected = checkBatteryPresence();
    
    // Check charging status
    isCharging = checkChargingStatus();
    
    batteryLevelCharacteristic->setValue(&batteryLevel, 1);
    batteryLevelCharacteristic->notify();
    lastBatteryUpdate = millis();
    
    Serial.printf("Battery status: %s | Level: %d%% | Charging: %s\n", 
                  getBatteryConnectionStatus(), batteryLevel, isCharging ? "YES" : "NO");
    Serial.println("----------------------------------------");
}
