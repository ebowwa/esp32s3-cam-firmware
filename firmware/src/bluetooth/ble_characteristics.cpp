#include "ble_characteristics.h"
#include "../system/device_status.h"
#include "../system/battery_code.h"
#include "../camera/camera.h"

// BLE Characteristics - Audio
BLECharacteristic *audioDataCharacteristic = nullptr;
BLECharacteristic *audioCodecCharacteristic = nullptr;

// BLE Characteristics - Photo
BLECharacteristic *photoDataCharacteristic = nullptr;
BLECharacteristic *photoControlCharacteristic = nullptr;

// BLE Characteristics - Video
BLECharacteristic *videoDataCharacteristic = nullptr;
BLECharacteristic *videoControlCharacteristic = nullptr;
BLECharacteristic *videoStatusCharacteristic = nullptr;

// BLE Characteristics - Device Info
BLECharacteristic *manufacturerNameCharacteristic = nullptr;
BLECharacteristic *modelNumberCharacteristic = nullptr;
BLECharacteristic *firmwareRevisionCharacteristic = nullptr;
BLECharacteristic *hardwareRevisionCharacteristic = nullptr;

void createAudioCharacteristics(BLEService *service) {
    // Audio data characteristic
    audioDataCharacteristic = service->createCharacteristic(
        audioDataUUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    BLE2902 *ccc = new BLE2902();
    ccc->setNotifications(true);
    audioDataCharacteristic->addDescriptor(ccc);

    // Audio codec characteristic
    audioCodecCharacteristic = service->createCharacteristic(
        audioCodecUUID,
        BLECharacteristic::PROPERTY_READ
    );
    uint8_t codecId = CODEC_ID;
    audioCodecCharacteristic->setValue(&codecId, 1);
    
    Serial.println("Audio characteristics created");
}

void createPhotoCharacteristics(BLEService *service) {
    // Photo data characteristic
    photoDataCharacteristic = service->createCharacteristic(
        photoDataUUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    BLE2902 *ccc = new BLE2902();
    ccc->setNotifications(true);
    photoDataCharacteristic->addDescriptor(ccc);

    // Photo control characteristic
    photoControlCharacteristic = service->createCharacteristic(
        photoControlUUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    photoControlCharacteristic->setCallbacks(new PhotoControlCallback());
    uint8_t controlValue = 0;
    photoControlCharacteristic->setValue(&controlValue, 1);
    
    Serial.println("Photo characteristics created");
}

void createVideoCharacteristics(BLEService *videoService) {
    // Video data characteristic
    videoDataCharacteristic = videoService->createCharacteristic(
        videoDataUUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    BLE2902 *ccc = new BLE2902();
    ccc->setNotifications(true);
    videoDataCharacteristic->addDescriptor(ccc);

    // Video control characteristic
    videoControlCharacteristic = videoService->createCharacteristic(
        videoControlUUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    videoControlCharacteristic->setCallbacks(new VideoControlCallback());
    uint8_t videoControlValue = 0;
    videoControlCharacteristic->setValue(&videoControlValue, 1);

    // Video status characteristic
    videoStatusCharacteristic = videoService->createCharacteristic(
        videoStatusUUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    ccc = new BLE2902();
    ccc->setNotifications(true);
    videoStatusCharacteristic->addDescriptor(ccc);
    
    Serial.println("Video characteristics created");
}

void createDeviceInfoCharacteristics(BLEService *deviceInfoService) {
    // Manufacturer name characteristic
    manufacturerNameCharacteristic = deviceInfoService->createCharacteristic(
        MANUFACTURER_NAME_STRING_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    
    // Model number characteristic
    modelNumberCharacteristic = deviceInfoService->createCharacteristic(
        MODEL_NUMBER_STRING_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    
    // Firmware revision characteristic
    firmwareRevisionCharacteristic = deviceInfoService->createCharacteristic(
        FIRMWARE_REVISION_STRING_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ
    );
    
    // Hardware revision characteristic
    hardwareRevisionCharacteristic = deviceInfoService->createCharacteristic(
        HARDWARE_REVISION_STRING_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ
    );

    // Set values
    manufacturerNameCharacteristic->setValue(MANUFACTURER_NAME);
    modelNumberCharacteristic->setValue(MODEL_NUMBER);
    firmwareRevisionCharacteristic->setValue(FIRMWARE_VERSION);
    hardwareRevisionCharacteristic->setValue(HARDWARE_VERSION);
    
    Serial.println("Device info characteristics created");
}

void updateVideoStatus() {
    if (!videoStatusCharacteristic) return;
    
    video_status_t status = {
        .streaming = isStreamingVideo ? 1 : 0,
        .fps = streamingFPS,
        .frameCount = totalStreamingFrames,
        .droppedFrames = droppedFrames
    };
    
    videoStatusCharacteristic->setValue((uint8_t*)&status, sizeof(status));
    videoStatusCharacteristic->notify();
}

void notifyAudioData(uint8_t *data, size_t length) {
    if (audioDataCharacteristic && bleConnected) {
        audioDataCharacteristic->setValue(data, length);
        audioDataCharacteristic->notify();
    }
}

void notifyPhotoData(uint8_t *data, size_t length) {
    if (photoDataCharacteristic && bleConnected) {
        photoDataCharacteristic->setValue(data, length);
        photoDataCharacteristic->notify();
    }
}

void notifyVideoData(uint8_t *data, size_t length) {
    if (videoDataCharacteristic && bleConnected) {
        videoDataCharacteristic->setValue(data, length);
        videoDataCharacteristic->notify();
    }
}

void initializeBLECharacteristics() {
    // Characteristics are initialized when services are created
    // This function is kept for future initialization needs
    Serial.println("BLE characteristics initialized");
} 