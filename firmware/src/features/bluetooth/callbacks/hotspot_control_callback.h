#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLECharacteristic.h>

// Forward declarations
void handleHotspotControl(uint8_t controlValue);
void updateHotspotStatus();

// Hotspot Control Callback Handler
class HotspotControlCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *characteristic) override;
}; 