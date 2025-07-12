#include "ble_server.h"
#include "services/ble_services.h"
#include "../../status/device_status.h"
#include "../../system/battery/battery_code.h"
// #include "../../system/charging/charging_manager.h"  // DISABLED: Charging system removed

// BLE Server instance
BLEServer *bleServer = nullptr;
// BLE advertising state
bool bleAdvertisingActive = false;

// BLE Services
BLEService *mainService = nullptr;
BLEService *videoService = nullptr;
BLEService *deviceInfoService = nullptr;

void initializeBLEServer() {
    Serial.println("Initializing BLE server...");
    
    // Initialize BLE device
    BLEDevice::init(BLE_DEVICE_NAME);
    
    // Create BLE server
    bleServer = BLEDevice::createServer();
    
    // Set larger MTU for better throughput
    BLEDevice::setMTU(BLE_MTU_SIZE);
    
    // Set server callbacks
    bleServer->setCallbacks(new BLEServerHandler());
    
    Serial.println("BLE server initialized");
}

void configureBLEServer() {
    Serial.println("Configuring BLE services...");
    
    // Create main service
    mainService = bleServer->createService(serviceUUID);
    
    // Create video service
    videoService = bleServer->createService(videoServiceUUID);
    
    // Create device information service
    deviceInfoService = bleServer->createService(DEVICE_INFORMATION_SERVICE_UUID);
    
    // Create characteristics for each service
    createAudioCharacteristics(mainService);
    createPhotoCharacteristics(mainService);
    createVideoCharacteristics(videoService);
    createDeviceInfoCharacteristics(deviceInfoService);
    createHotspotCharacteristics(mainService);
    
    // Setup device status service
    setupDeviceStatusService(mainService);
    
    // Setup battery service
    setupBatteryService(bleServer);
    
    // Setup charging service
    // setupChargingService(bleServer);  // DISABLED: Charging system removed
    
    Serial.println("BLE services configured");
}

void startBLEServices() {
    Serial.println("Starting BLE services...");
    
    // Start all services
    if (mainService) {
        mainService->start();
        Serial.println("Main service started");
    }
    
    if (videoService) {
        videoService->start();
        Serial.println("Video service started");
    }
    
    if (deviceInfoService) {
        deviceInfoService->start();
        Serial.println("Device info service started");
    }
    
    Serial.println("All BLE services started");
}

void startBLEAdvertising() {
    Serial.println("Starting BLE advertising...");
    
    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    
    // Add service UUIDs to advertising
    advertising->addServiceUUID(BATTERY_SERVICE_UUID);
    advertising->addServiceUUID(DEVICE_INFORMATION_SERVICE_UUID);
    // advertising->addServiceUUID(CHARGING_SERVICE_UUID);  // DISABLED: Charging system removed
    
    if (mainService) {
        advertising->addServiceUUID(mainService->getUUID());
    }
    
    if (videoService) {
        advertising->addServiceUUID(videoService->getUUID());
    }
    
    // Configure advertising parameters
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMaxPreferred(0x12);
    
    // Start advertising
    BLEDevice::startAdvertising();
    bleAdvertisingActive = true;
    
    Serial.println("BLE advertising started");
}

void stopBLEAdvertising() {
    BLEDevice::stopAdvertising();
    bleAdvertisingActive = false;
    Serial.println("BLE advertising stopped");
}

bool isBLEServerRunning() {
    return bleServer != nullptr;
}

bool isBLEAdvertising() {
    return bleAdvertisingActive;
} 