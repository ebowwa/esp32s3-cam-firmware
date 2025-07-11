#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLECharacteristic.h>

// Forward declaration
void handlePhotoControl(int8_t controlValue);

// Photo Control Callback Handler
class PhotoControlCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *characteristic) override;
}; 