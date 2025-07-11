#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "../../hal/constants.h"
#include "../../hal/xiao_esp32s3_constants.h"

// Battery Level Service UUIDs
#define BATTERY_SERVICE_UUID (uint16_t)0x180F
#define BATTERY_LEVEL_CHAR_UUID (uint16_t)0x2A19

extern BLECharacteristic *batteryLevelCharacteristic;
extern uint8_t batteryLevel;
extern unsigned long lastBatteryUpdate;
extern bool batteryDetected;
extern bool isCharging;

// Battery connection monitoring variables
extern float batteryVoltageHistory[BATTERY_STABILITY_SAMPLES];
extern int voltageHistoryIndex;
extern int unstableReadingCount;
extern float lastStableVoltage;
extern bool connectionStable;
extern unsigned long lastVoltageChangeTime;

// Initializes the battery service. Call from BLE configuration.
void setupBatteryService(BLEServer *server);

// Updates and notifies the current battery level. Call periodically.
void updateBatteryLevel();

// Checks if a battery is connected to the device
bool checkBatteryPresence();

// Reads the battery voltage
float readBatteryVoltage();

// Checks if the device is charging via USB power
bool checkChargingStatus();

// Reads the USB power voltage
float readUSBPowerVoltage();

// Analyzes battery connection stability
bool analyzeBatteryConnectionStability(float currentVoltage);

// Detects rapid voltage changes that indicate connection issues
bool detectRapidVoltageChange(float currentVoltage);

// Gets battery connection quality assessment
const char* getBatteryConnectionStatus();
