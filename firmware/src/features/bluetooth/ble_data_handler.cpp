#include "ble_data_handler.h"
#include "../microphone/mulaw.h"
#include "../microphone/audio_filters.h"
#ifdef CODEC_OPUS
#include "../microphone/opus_codec.h"
#endif
#include "../../system/memory/memory_utils.h"
// #include "../../utils/hotspot_manager.h"  // DISABLED: Causes BLE interference
#include "../camera/camera.h"

// Audio frame management
uint16_t audioFrameCount = 0;

void transmitAudioData(uint8_t *audioBuffer, size_t bufferSize, size_t bytesRecorded) {
    if (!bleConnected || bytesRecorded == 0) return;
    
    // Allocate compressed frame buffer
    static uint8_t *compressedFrame = nullptr;
    if (!compressedFrame) {
        compressedFrame = (uint8_t *)PS_CALLOC_TRACKED(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t), "BLECompressedFrame");
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
        
        // For large frames, we need to split them into smaller chunks for BLE transmission
        const size_t MAX_BLE_CHUNK = 400; // Stay well under MTU limit
        
        if (totalSize <= MAX_BLE_CHUNK) {
            // Small frame - send directly
            notifyAudioData(compressedFrame, totalSize);
        } else {
            // Large frame - split into chunks
            size_t offset = 0;
            uint8_t chunkIndex = 0;
            
            while (offset < totalSize) {
                size_t chunkSize = min(MAX_BLE_CHUNK - 4, totalSize - offset); // Leave room for chunk header
                
                // Create chunk with header: [frameCount_low, frameCount_high, chunkIndex, chunkType, ...data]
                uint8_t chunkBuffer[MAX_BLE_CHUNK];
                chunkBuffer[0] = audioFrameCount & 0xFF;
                chunkBuffer[1] = (audioFrameCount >> 8) & 0xFF;
                chunkBuffer[2] = chunkIndex;
                chunkBuffer[3] = (offset + chunkSize >= totalSize) ? 0x80 : 0x00; // 0x80 = last chunk
                
                memcpy(&chunkBuffer[4], &compressedFrame[offset], chunkSize);
                notifyAudioData(chunkBuffer, chunkSize + 4);
                
                offset += chunkSize;
                chunkIndex++;
            }
        }
        
        audioFrameCount++;
    }
}

void prepareAudioFrame(uint8_t *compressedFrame, uint8_t *audioBuffer, size_t bytesRecorded, int &encodedBytes) {
    encodedBytes = 0;
    
    // Apply audio filters to the raw audio data before encoding
    int16_t* audio_samples = (int16_t*)audioBuffer;
    size_t sample_count = bytesRecorded / 2;
    AudioFilters::applyFilters(audio_samples, sample_count);
    
#ifdef CODEC_OPUS
    // Opus encoding - process in FRAME_SIZE chunks
    int16_t samples[FRAME_SIZE];
    size_t samples_to_process = min(sample_count, (size_t)FRAME_SIZE);
    
    // Prepare samples with volume gain
    for (size_t i = 0; i < samples_to_process; i++) {
        samples[i] = audio_samples[i] << VOLUME_GAIN;
    }
    
    // Pad with zeros if we don't have enough samples
    for (size_t i = samples_to_process; i < FRAME_SIZE; i++) {
        samples[i] = 0;
    }
    
    encodedBytes = OpusCodec::encode(samples, FRAME_SIZE, &compressedFrame[3], COMPRESSED_BUFFER_SIZE - 3);
    
    if (encodedBytes <= 0) {
        Serial.printf("Opus encoding failed: %d\n", encodedBytes);
        encodedBytes = 0;
    }
    
#else
#ifdef CODEC_MULAW
    // μ-law encoding - process all samples
    for (size_t i = 0; i < sample_count; i++) {
        int16_t sample = audio_samples[i] << VOLUME_GAIN;
        compressedFrame[i + 3] = linear2ulaw(sample);
    }
    encodedBytes = sample_count;
    
#else
    // PCM encoding (16-bit to 16-bit with volume gain) - process all samples
    for (size_t i = 0; i < sample_count; i++) {
        int16_t sample = audio_samples[i] << VOLUME_GAIN;
        compressedFrame[i * 2 + 3] = sample & 0xFF;
        compressedFrame[i * 2 + 4] = (sample >> 8) & 0xFF;
    }
    encodedBytes = sample_count * 2;
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



 