#include "ble_data_handler.h"
#include "../utils/mulaw.h"
#include "../camera/camera.h"

#ifdef CODEC_OPUS
#include <opus.h>
extern OpusEncoder *opus_encoder;
#endif

// Audio frame management
uint16_t audioFrameCount = 0;

void transmitAudioData(uint8_t *audioBuffer, size_t bufferSize, size_t bytesRecorded) {
    if (!bleConnected || bytesRecorded == 0) return;
    
    // Allocate compressed frame buffer
    static uint8_t *compressedFrame = nullptr;
    if (!compressedFrame) {
        compressedFrame = (uint8_t *)ps_calloc(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t));
        if (!compressedFrame) {
            Serial.println("Failed to allocate compressed frame buffer");
            return;
        }
    }
    
    int encodedBytes = 0;
    prepareAudioFrame(compressedFrame, audioBuffer, bytesRecorded, encodedBytes);
    
    if (encodedBytes > 0) {
        // Add frame header
        compressedFrame[0] = audioFrameCount & 0xFF;
        compressedFrame[1] = (audioFrameCount >> 8) & 0xFF;
        compressedFrame[2] = 0; // Frame type
        
        size_t totalSize = encodedBytes + 3;
        notifyAudioData(compressedFrame, totalSize);
        audioFrameCount++;
    }
}

void prepareAudioFrame(uint8_t *compressedFrame, uint8_t *audioBuffer, size_t bytesRecorded, int &encodedBytes) {
    encodedBytes = 0;
    
#ifdef CODEC_OPUS
    // Opus encoding
    int16_t samples[FRAME_SIZE];
    for (size_t i = 0; i < bytesRecorded; i += 2) {
        samples[i / 2] = ((audioBuffer[i + 1] << 8) | audioBuffer[i]) << VOLUME_GAIN;
    }
    
    encodedBytes = opus_encode(opus_encoder, samples, FRAME_SIZE, &compressedFrame[3], COMPRESSED_BUFFER_SIZE - 3);
    
    if (encodedBytes <= 0) {
        Serial.printf("Opus encoding failed: %d\n", encodedBytes);
        encodedBytes = 0;
    }
    
#else
#ifdef CODEC_MULAW
    // μ-law encoding
    for (size_t i = 0; i < bytesRecorded; i += 2) {
        int16_t sample = ((audioBuffer[i + 1] << 8) | audioBuffer[i]) << VOLUME_GAIN;
        compressedFrame[i / 2 + 3] = linear2ulaw(sample);
    }
    encodedBytes = bytesRecorded / 2;
    
#else
    // PCM encoding (16-bit to 16-bit with volume gain)
    for (size_t i = 0; i < bytesRecorded / 4; i++) {
        int16_t sample = ((int16_t *)audioBuffer)[i * 2] << VOLUME_GAIN;
        compressedFrame[i * 2 + 3] = sample & 0xFF;
        compressedFrame[i * 2 + 4] = (sample >> 8) & 0xFF;
    }
    encodedBytes = bytesRecorded / 2;
#endif
#endif
}

void transmitPhotoData(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber, bool isStreamingFrame) {
    if (!bleConnected || !frameBuffer || frameSize == 0) return;
    
    // Frame buffer already contains headers and data from main firmware
    // Just transmit directly without adding additional headers
    if (isStreamingFrame) {
        notifyVideoData(frameBuffer, frameSize);
    } else {
        notifyPhotoData(frameBuffer, frameSize);
    }
}

void transmitVideoData(uint8_t *frameBuffer, size_t frameSize, uint16_t frameNumber) {
    transmitPhotoData(frameBuffer, frameSize, frameNumber, true);
}

void transmitEndMarker(bool isStreamingFrame) {
    if (!bleConnected) return;
    
    uint8_t endMarker[3] = {
        PHOTO_END_MARKER_LOW,
        PHOTO_END_MARKER_HIGH,
        isStreamingFrame ? 0x02 : 0x01
    };
    
    if (isStreamingFrame) {
        notifyVideoData(endMarker, 3);
    } else {
        notifyPhotoData(endMarker, 3);
    }
}

bool isReadyForTransmission() {
    return bleConnected;
}

void resetTransmissionState() {
    audioFrameCount = 0;
    Serial.println("BLE transmission state reset");
}

void initializeBLEDataHandler() {
    audioFrameCount = 0;
    Serial.println("BLE data handler initialized");
} 