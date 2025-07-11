#include "ble_data_handler.h"
#include "../../utils/mulaw.h"
#include "../../system/memory/memory_utils.h"
// #include "../../utils/hotspot_manager.h"  // DISABLED: Causes BLE interference
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

// Hotspot Control Handler - DISABLED: Causes BLE interference
void handleHotspotControl(uint8_t controlValue) {
    Serial.printf("Hotspot control received: %d (DISABLED)\n", controlValue);
    // All hotspot functionality disabled to prevent WiFi/BLE interference
    /*
    switch (controlValue) {
        case 0x00: // Stop hotspot
            Serial.println("Stopping hotspot via BLE command");
            stopHotspot();
            break;
            
        case 0x01: // Start hotspot
            Serial.println("Starting hotspot via BLE command");
            startHotspot();
            break;
            
        case 0x02: // Toggle hotspot
            Serial.println("Toggling hotspot via BLE command");
            toggleHotspot();
            break;
            
        case 0x03: // Get hotspot status
            Serial.println("Hotspot status requested via BLE");
            updateHotspotStatus();
            break;
            
        default:
            Serial.printf("Unknown hotspot control value: 0x%02X\n", controlValue);
            break;
    }
    */
}

// Update hotspot status characteristic - DISABLED: Causes BLE interference
void updateHotspotStatus() {
    if (!hotspotStatusCharacteristic || !bleConnected) return;
    
    // Send minimal status to indicate hotspot is disabled
    uint8_t statusData[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // All zeros = disabled
    hotspotStatusCharacteristic->setValue(statusData, 8);
    hotspotStatusCharacteristic->notify();
    
    Serial.println("Hotspot status updated: DISABLED (prevents BLE interference)");
    
    /*
    // Original hotspot status code disabled to prevent WiFi/BLE interference
    hotspot_stats_t stats = getHotspotStats();
    
    // Create extended status packet with BLE data
    uint8_t statusData[64];
    int pos = 0;
    
    // Basic hotspot status
    statusData[pos++] = (uint8_t)stats.status;
    statusData[pos++] = stats.connected_clients;
    statusData[pos++] = stats.max_clients;
    
    // BLE integration data
    statusData[pos++] = stats.ble_connected ? 1 : 0;
    statusData[pos++] = stats.active_interfaces;
    
    // Data statistics (4 bytes each, little endian)
    uint32_t total_data = stats.total_data_all_interfaces;
    statusData[pos++] = total_data & 0xFF;
    statusData[pos++] = (total_data >> 8) & 0xFF;
    statusData[pos++] = (total_data >> 16) & 0xFF;
    statusData[pos++] = (total_data >> 24) & 0xFF;
    
    uint32_t ble_data = stats.ble_data_transmitted;
    statusData[pos++] = ble_data & 0xFF;
    statusData[pos++] = (ble_data >> 8) & 0xFF;
    statusData[pos++] = (ble_data >> 16) & 0xFF;
    statusData[pos++] = (ble_data >> 24) & 0xFF;
    
    // Add SSID length and SSID (up to 20 chars)
    uint8_t ssidLen = min(stats.ssid.length(), (unsigned int)20);
    statusData[pos++] = ssidLen;
    if (ssidLen > 0) {
        memcpy(&statusData[pos], stats.ssid.c_str(), ssidLen);
        pos += ssidLen;
    }
    
    // Add IP address length and IP (up to 15 chars)
    uint8_t ipLen = min(stats.ip_address.length(), (unsigned int)15);
    statusData[pos++] = ipLen;
    if (ipLen > 0) {
        memcpy(&statusData[pos], stats.ip_address.c_str(), ipLen);
        pos += ipLen;
    }
    
    hotspotStatusCharacteristic->setValue(statusData, pos);
    hotspotStatusCharacteristic->notify();
    
    // Record BLE data transmission for this status update
    recordBLEDataTransmission(pos);
    
    Serial.printf("Hotspot status updated: Status=%d, Clients=%d/%d, SSID=%s, IP=%s, BLE=%s, Total=%u bytes\n",
                  stats.status, stats.connected_clients, stats.max_clients,
                  stats.ssid.c_str(), stats.ip_address.c_str(),
                  stats.ble_connected ? "Connected" : "Disconnected",
                  stats.total_data_all_interfaces);
    */
} 