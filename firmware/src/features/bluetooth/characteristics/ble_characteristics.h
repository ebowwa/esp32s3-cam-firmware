#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEService.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>
#include "../services/ble_services.h"
#include "../callbacks/callbacks.h"
#include "../../../hal/constants.h"
#include "../../camera/camera.h"

// BLE Characteristics - Audio
extern BLECharacteristic *audioDataCharacteristic;
extern BLECharacteristic *audioCodecCharacteristic;

// BLE Characteristics - Photo
extern BLECharacteristic *photoDataCharacteristic;
extern BLECharacteristic *photoControlCharacteristic;

// BLE Characteristics - Video
extern BLECharacteristic *videoDataCharacteristic;
extern BLECharacteristic *videoControlCharacteristic;
extern BLECharacteristic *videoStatusCharacteristic;

// BLE Characteristics - Device Info
extern BLECharacteristic *manufacturerNameCharacteristic;
extern BLECharacteristic *modelNumberCharacteristic;
extern BLECharacteristic *firmwareRevisionCharacteristic;
extern BLECharacteristic *hardwareRevisionCharacteristic;

// BLE Characteristics - Hotspot Control
extern BLECharacteristic *hotspotControlCharacteristic;
extern BLECharacteristic *hotspotStatusCharacteristic;

// Characteristic creation functions
void createAudioCharacteristics(BLEService *service);
void createPhotoCharacteristics(BLEService *service);
void createVideoCharacteristics(BLEService *videoService);
void createDeviceInfoCharacteristics(BLEService *deviceInfoService);
void createHotspotCharacteristics(BLEService *service);

// Characteristic utility functions
void updateVideoStatus();
void notifyAudioData(uint8_t *data, size_t length);
void notifyPhotoData(uint8_t *data, size_t length);
void notifyVideoData(uint8_t *data, size_t length);

// Initialize all BLE characteristics
void initializeBLECharacteristics(); 