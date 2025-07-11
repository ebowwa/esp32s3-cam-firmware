#pragma once

#include <Arduino.h>
#include "characteristics/ble_characteristics.h"
#include "ble_callbacks.h"
#include "../../hal/constants.h"

// Audio frame management
extern uint16_t audioFrameCount;

// Audio data transmission
void transmitAudioData(uint8_t *audioBuffer, size_t bufferSize, size_t bytesRecorded);
void prepareAudioFrame(uint8_t *compressedFrame, uint8_t *audioBuffer, size_t bytesRecorded, int &encodedBytes);

// Photo/Video data transmission
void transmitPhotoData(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber, bool isStreamingFrame);
void transmitVideoData(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber);
void transmitEndMarker(bool isStreamingFrame);

// Data transmission utilities
bool isReadyForTransmission();
void resetTransmissionState();

// Initialize BLE data handler
void initializeBLEDataHandler();

// Hotspot status update
void updateHotspotStatus(); 