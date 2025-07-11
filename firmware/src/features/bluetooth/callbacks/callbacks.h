#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include "../services/ble_services.h"

// Include all individual callback headers
#include "ble_server_callback.h"
#include "photo_control_callback.h"
#include "video_control_callback.h"
#include "hotspot_control_callback.h"

// Initialize BLE callbacks
void initializeBLECallbacks(); 