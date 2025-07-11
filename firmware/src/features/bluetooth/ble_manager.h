#pragma once

#include <Arduino.h>
#include "services/ble_services.h"
#include "ble_server.h"
#include "characteristics/ble_characteristics.h"
#include "ble_callbacks.h"
#include "ble_data_handler.h"

// Main BLE Manager Interface
class BLEManager {
public:
    // Initialization
    static void initialize();
    static void configure();
    static void start();
    static void stop();
    
    // Connection management
    static bool isConnected();
    static bool isAdvertising();
    static void startAdvertising();
    static void stopAdvertising();
    
    // Data transmission
    static void transmitAudio(uint8_t *audioBuffer, size_t bufferSize, size_t bytesRecorded);
    static void transmitPhoto(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber);
    static void transmitVideo(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber);
    static void transmitPhotoEnd();
    static void transmitVideoEnd();
    
    // Status and utility
    static void updateVideoStatus();
    static void reset();
    
private:
    static bool initialized;
    static bool configured;
    static bool started;
};

// Convenience functions for backward compatibility
void configureBLE();
bool isConnected();
void updateVideoStatus(); 