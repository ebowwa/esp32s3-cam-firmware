#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "../platform/constants.h"
#include "../hardware/xiao_esp32s3_constants.h"

extern BLECharacteristic *deviceStatusCharacteristic;
extern uint8_t deviceStatus;
extern bool deviceReady;

// Updates and notifies the current device status. Call when status changes.
void updateDeviceStatus(uint8_t status);

// Initialize device status service. Call from BLE configuration.
void setupDeviceStatusService(BLEService *service); 