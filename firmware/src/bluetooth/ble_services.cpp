#include "ble_services.h"

// BLE UUIDs as objects
BLEUUID serviceUUID(SERVICE_UUID);
BLEUUID videoServiceUUID(VIDEO_SERVICE_UUID);
BLEUUID audioDataUUID(AUDIO_DATA_UUID);
BLEUUID audioCodecUUID(AUDIO_CODEC_UUID);
BLEUUID photoDataUUID(PHOTO_DATA_UUID);
BLEUUID photoControlUUID(PHOTO_CONTROL_UUID);
BLEUUID videoDataUUID(VIDEO_DATA_UUID);
BLEUUID videoControlUUID(VIDEO_CONTROL_UUID);
BLEUUID videoStatusUUID(VIDEO_STATUS_UUID);

void initializeBLEUUIDs() {
    // UUIDs are already initialized as global objects
    // This function is kept for future initialization needs
    Serial.println("BLE UUIDs initialized");
} 