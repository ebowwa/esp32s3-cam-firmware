#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE Service UUIDs
#define DEVICE_INFORMATION_SERVICE_UUID (uint16_t)0x180A
#define BATTERY_SERVICE_UUID (uint16_t)0x180F

// Device Information Characteristic UUIDs
#define MANUFACTURER_NAME_STRING_CHAR_UUID (uint16_t)0x2A29
#define MODEL_NUMBER_STRING_CHAR_UUID (uint16_t)0x2A24
#define FIRMWARE_REVISION_STRING_CHAR_UUID (uint16_t)0x2A26
#define HARDWARE_REVISION_STRING_CHAR_UUID (uint16_t)0x2A27

// Battery Service Characteristic UUIDs
#define BATTERY_LEVEL_CHAR_UUID (uint16_t)0x2A19

// Main OpenGlass Service UUIDs
static const char* SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
static const char* VIDEO_SERVICE_UUID = "19B10010-E8F2-537E-4F6C-D104768A1214";

// Audio Characteristic UUIDs
static const char* AUDIO_DATA_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214";
static const char* AUDIO_CODEC_UUID = "19B10002-E8F2-537E-4F6C-D104768A1214";

// Photo Characteristic UUIDs
static const char* PHOTO_DATA_UUID = "19B10005-E8F2-537E-4F6C-D104768A1214";
static const char* PHOTO_CONTROL_UUID = "19B10006-E8F2-537E-4F6C-D104768A1214";

// Device Status Characteristic UUID
static const char* DEVICE_STATUS_UUID = "19B10007-E8F2-537E-4F6C-D104768A1214";

// Video Characteristic UUIDs
static const char* VIDEO_DATA_UUID = "19B10008-E8F2-537E-4F6C-D104768A1214";
static const char* VIDEO_CONTROL_UUID = "19B10009-E8F2-537E-4F6C-D104768A1214";
static const char* VIDEO_STATUS_UUID = "19B1000A-E8F2-537E-4F6C-D104768A1214";

// BLE Configuration Constants
#define BLE_MTU_SIZE 512
#define BLE_DEVICE_NAME "OpenGlass"

// Device Information Constants
#define MANUFACTURER_NAME "Based Hardware"
#define MODEL_NUMBER "OpenGlass"
#define FIRMWARE_VERSION "1.0.1"
#define HARDWARE_VERSION "Seeed Studio XIAO ESP32S3 Sense"

// BLE UUIDs as objects for easier use
extern BLEUUID serviceUUID;
extern BLEUUID videoServiceUUID;
extern BLEUUID audioDataUUID;
extern BLEUUID audioCodecUUID;
extern BLEUUID photoDataUUID;
extern BLEUUID photoControlUUID;
extern BLEUUID videoDataUUID;
extern BLEUUID videoControlUUID;
extern BLEUUID videoStatusUUID;

// Initialize BLE UUIDs
void initializeBLEUUIDs(); 