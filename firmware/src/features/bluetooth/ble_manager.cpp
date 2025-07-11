#include "ble_manager.h"

// Static member initialization
bool BLEManager::initialized = false;
bool BLEManager::configured = false;
bool BLEManager::started = false;

void BLEManager::initialize() {
    if (initialized) return;
    
    Serial.println("BLE Manager: Initializing...");
    
    // Initialize BLE components
    initializeBLEUUIDs();
    initializeBLEServer();
    initializeBLECallbacks();
    initializeBLECharacteristics();
    initializeBLEDataHandler();
    
    initialized = true;
    Serial.println("BLE Manager: Initialized");
}

void BLEManager::configure() {
    if (!initialized) {
        Serial.println("BLE Manager: Must initialize before configure");
        return;
    }
    
    if (configured) return;
    
    Serial.println("BLE Manager: Configuring...");
    
    // Configure BLE server and services
    configureBLEServer();
    
    configured = true;
    Serial.println("BLE Manager: Configured");
}

void BLEManager::start() {
    if (!initialized || !configured) {
        Serial.println("BLE Manager: Must initialize and configure before start");
        return;
    }
    
    if (started) return;
    
    Serial.println("BLE Manager: Starting...");
    
    // Start BLE services and advertising
    startBLEServices();
    startBLEAdvertising();
    
    started = true;
    Serial.println("BLE Manager: Started");
}

void BLEManager::stop() {
    if (!started) return;
    
    Serial.println("BLE Manager: Stopping...");
    
    stopBLEAdvertising();
    
    started = false;
    Serial.println("BLE Manager: Stopped");
}

bool BLEManager::isConnected() {
    return bleConnected;
}

bool BLEManager::isAdvertising() {
    return isBLEAdvertising();
}

void BLEManager::startAdvertising() {
    startBLEAdvertising();
}

void BLEManager::stopAdvertising() {
    stopBLEAdvertising();
}

void BLEManager::transmitAudio(uint8_t *audioBuffer, size_t bufferSize, size_t bytesRecorded) {
    if (!started || !isConnected()) return;
    transmitAudioData(audioBuffer, bufferSize, bytesRecorded);
}

void BLEManager::transmitPhoto(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber) {
    if (!started || !isConnected()) return;
    transmitPhotoData(frameBuffer, frameSize, frameNumber, false);
}

void BLEManager::transmitVideo(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber) {
    if (!started || !isConnected()) return;
    transmitVideoData(frameBuffer, frameSize, frameNumber);
}

void BLEManager::transmitPhotoEnd() {
    if (!started || !isConnected()) return;
    transmitEndMarker(false);
}

void BLEManager::transmitVideoEnd() {
    if (!started || !isConnected()) return;
    transmitEndMarker(true);
}

void BLEManager::updateVideoStatus() {
    if (!started) return;
    ::updateVideoStatus();
}

void BLEManager::reset() {
    Serial.println("BLE Manager: Resetting...");
    
    resetTransmissionState();
    
    // Reset state flags
    if (started) {
        stop();
    }
    
    initialized = false;
    configured = false;
    started = false;
    
    Serial.println("BLE Manager: Reset complete");
}

// Convenience functions for backward compatibility
void configureBLE() {
    BLEManager::initialize();
    BLEManager::configure();
    BLEManager::start();
}

bool isConnected() {
    return BLEManager::isConnected();
}

// Note: updateVideoStatus() is defined in ble_characteristics.cpp 