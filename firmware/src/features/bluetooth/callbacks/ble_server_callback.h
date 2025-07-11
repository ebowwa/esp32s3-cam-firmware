#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include "../services/ble_services.h"

// BLE Server Connection Handler
class BLEServerHandler : public BLEServerCallbacks {
public:
    void onConnect(BLEServer *server) override;
    void onDisconnect(BLEServer *server) override;
};

// Connection state management
extern bool bleConnected; 