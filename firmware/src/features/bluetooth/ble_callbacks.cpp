#include "ble_callbacks.h"
#include "../led/led_manager.h"
#include "../../system/device_status.h"
// #include "../utils/hotspot_manager.h"  // DISABLED: Causes BLE interference

// Connection state
bool bleConnected = false;

// BLE Server Connection Handler Implementation
void BLEServerHandler::onConnect(BLEServer *server) {
    bleConnected = true;
    Serial.println("BLE Client connected");
    setLedPattern(LED_CONNECTED);
    updateDeviceStatus(deviceReady ? DEVICE_STATUS_READY : deviceStatus);
    
    // Update hotspot statistics with BLE connection
    // String client_info = "BLE Client " + String(server->getConnId());
    // updateBLEConnectionStatus(true, client_info);  // DISABLED: Causes BLE interference
}

void BLEServerHandler::onDisconnect(BLEServer *server) {
    bleConnected = false;
    Serial.println("BLE Client disconnected, restarting advertising");
    setLedPattern(LED_DISCONNECTED);
    BLEDevice::startAdvertising();
    
    // Update hotspot statistics with BLE disconnection
    // updateBLEConnectionStatus(false);  // DISABLED: Causes BLE interference
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

// Hotspot Control Callback Implementation
void HotspotControlCallback::onWrite(BLECharacteristic *characteristic) {
    Serial.printf("Hotspot control write received, length: %d\n", characteristic->getLength());
    
    // Record BLE command received
    // recordBLECommandReceived();  // DISABLED: Function not defined and causes BLE interference
    
    if (characteristic->getLength() == 1) {
        uint8_t value = characteristic->getData()[0];
        Serial.printf("Hotspot control value: %d (0x%02X)\n", value, value);
        // handleHotspotControl(value);  // DISABLED: Causes BLE interference
    } else {
        Serial.println("Invalid hotspot control message length");
    }
}

void initializeBLECallbacks() {
    // Callbacks are initialized when creating characteristics
    // This function is kept for future initialization needs
    Serial.println("BLE callbacks initialized");
} 