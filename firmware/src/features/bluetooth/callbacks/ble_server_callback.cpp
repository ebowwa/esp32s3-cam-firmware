#include "ble_server_callback.h"
#include "../../../hal/led/led_manager.h"
#include "../../../status/device_status.h"

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