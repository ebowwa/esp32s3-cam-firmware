#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include "ble_services.h"
#include "ble_characteristics.h"
#include "ble_callbacks.h"

// BLE Server instance
extern BLEServer *bleServer;
// BLE advertising state
extern bool bleAdvertisingActive;

// BLE Services
extern BLEService *mainService;
extern BLEService *videoService;
extern BLEService *deviceInfoService;

// BLE Server management functions
void initializeBLEServer();
void configureBLEServer();
void startBLEServices();
void startBLEAdvertising();
void stopBLEAdvertising();

// BLE Server status
bool isBLEServerRunning();
bool isBLEAdvertising(); 