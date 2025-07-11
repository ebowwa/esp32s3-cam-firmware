#include "device_status.h"
#include "../features/bluetooth/services/ble_services.h"
#include "../hal/xiao_esp32s3_constants.h"
#include "../features/led/led_manager.h"

BLECharacteristic *deviceStatusCharacteristic = nullptr;
uint8_t deviceStatus = DEVICE_STATUS_INITIALIZING;
bool deviceReady = false;

void updateDeviceStatus(uint8_t status) {
  deviceStatus = status;
  Serial.printf("Device status updated to: %d\n", status);
  
  // Update LED pattern based on status
  setLedForDeviceStatus(status);
  
  if (deviceStatusCharacteristic) {
    deviceStatusCharacteristic->setValue(&deviceStatus, 1);
    deviceStatusCharacteristic->notify();
  }
}

void setupDeviceStatusService(BLEService *service) {
  deviceStatusCharacteristic = service->createCharacteristic(
      DEVICE_STATUS_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  
  BLE2902 *ccc = new BLE2902();
  ccc->setNotifications(true);
  deviceStatusCharacteristic->addDescriptor(ccc);
  deviceStatusCharacteristic->setValue(&deviceStatus, 1);
} 