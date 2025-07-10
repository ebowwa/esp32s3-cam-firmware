#include "ble_callbacks.h"
#include "../utils/led_manager.h"
#include "../core/device_status.h"

// Connection state
bool bleConnected = false;

// BLE Server Connection Handler Implementation
void BLEServerHandler::onConnect(BLEServer *server) {
    bleConnected = true;
    Serial.println("BLE Client connected");
    setLedPattern(LED_CONNECTED);
    updateDeviceStatus(deviceReady ? DEVICE_STATUS_READY : deviceStatus);
}

void BLEServerHandler::onDisconnect(BLEServer *server) {
    bleConnected = false;
    Serial.println("BLE Client disconnected, restarting advertising");
    setLedPattern(LED_DISCONNECTED);
    BLEDevice::startAdvertising();
}

// Photo Control Callback Implementation
void PhotoControlCallback::onWrite(BLECharacteristic *characteristic) {
    Serial.printf("Photo control write received, length: %d\n", characteristic->getLength());
    if (characteristic->getLength() == 1) {
        uint8_t value = characteristic->getData()[0];
        Serial.printf("Photo control value: %d (0x%02X)\n", (int8_t)value, value);
        handlePhotoControl((int8_t)value);
    } else {
        Serial.println("Invalid photo control message length");
    }
}

// Video Control Callback Implementation
void VideoControlCallback::onWrite(BLECharacteristic *characteristic) {
    Serial.printf("Video control write received, length: %d\n", characteristic->getLength());
    if (characteristic->getLength() == 1) {
        uint8_t value = characteristic->getData()[0];
        Serial.printf("Video control value: %d (0x%02X)\n", value, value);
        handleVideoControl(value);
    } else {
        Serial.println("Invalid video control message length");
    }
}

void initializeBLECallbacks() {
    // Callbacks are initialized when creating characteristics
    // This function is kept for future initialization needs
    Serial.println("BLE callbacks initialized");
} 