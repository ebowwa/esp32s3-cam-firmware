#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLECharacteristic.h>

// Forward declaration
void handleVideoControl(uint8_t controlValue);

// Video Control Callback Handler
class VideoControlCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic *characteristic) override;
}; 