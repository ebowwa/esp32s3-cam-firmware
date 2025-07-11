#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include "services/ble_services.h"

// Forward declarations
void handlePhotoControl(int8_t controlValue);
void handleVideoControl(uint8_t controlValue);
void handleHotspotControl(uint8_t controlValue);

// BLE Server Connection Handler
class BLEServerHandler : public BLEServerCallbacks {
public:
    void onConnect(BLEServer *server) override;
    void onDisconnect(BLEServer *server) override;
};

// Photo Control Callback Handler
class PhotoControlCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *characteristic) override;
};

// Video Control Callback Handler
class VideoControlCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *characteristic) override;
};

class HotspotControlCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *characteristic) override;
};

// Connection state management
extern bool bleConnected;

// Initialize BLE callbacks
void initializeBLECallbacks(); 