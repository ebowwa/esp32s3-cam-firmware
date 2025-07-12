#include "comm_cycles.h"
#include "cycle_manager.h"
#include "../../features/bluetooth/ble_data_handler.h"
#include "../../hal/led/led_manager.h"
#include "../../hal/constants.h"
#include "esp_camera.h"
#include <Arduino.h>

// External variables
extern bool photoDataUploading;
extern camera_fb_t *fb;
extern size_t sent_photo_bytes;
extern size_t sent_photo_frames;

// Function declarations
extern bool isConnected();
extern void notifyPhotoData(uint8_t* data, size_t length);

// ===================================================================
// COMMUNICATION CYCLE MANAGER
// ===================================================================

namespace CommCycles {
    int data_transmission_cycle_id = -1;
    int connection_monitor_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Communication Cycles...");
        registerDataTransmissionCycle();
        registerConnectionMonitorCycle();
    }
    
    void registerDataTransmissionCycle() {
        data_transmission_cycle_id = registerConditionCycle(
            "DataTransmission",
            []() {
                return photoDataUploading && fb && isConnected();
            },
            []() {
                if (!fb || !isConnected()) {
                    photoDataUploading = false;
                    sent_photo_bytes = 0;
                    sent_photo_frames = 0;
                    return;
                }
                
                // Calculate remaining data to send
                size_t remaining = fb->len - sent_photo_bytes;
                
                if (remaining > 0) {
                    // Prepare frame with header
                    uint8_t frame_buffer[PHOTO_CHUNK_SIZE + 3]; // +3 for header
                    
                    // Frame header: [frame_number_low, frame_number_high, frame_type]
                    frame_buffer[0] = sent_photo_frames & 0xFF;
                    frame_buffer[1] = (sent_photo_frames >> 8) & 0xFF;
                    frame_buffer[2] = 0x01; // Photo frame type
                    
                    // Calculate chunk size (leave room for header)
                    size_t chunk_size = min(remaining, (size_t)PHOTO_CHUNK_SIZE);
                    
                    // Copy photo data after header
                    memcpy(&frame_buffer[3], fb->buf + sent_photo_bytes, chunk_size);
                    
                    // Send frame with header + data
                    notifyPhotoData(frame_buffer, chunk_size + 3);
                    
                    sent_photo_bytes += chunk_size;
                    sent_photo_frames++;
                    
                    Serial.printf("Sent photo frame %d: %d bytes (total: %d/%d)\n", 
                                 sent_photo_frames, chunk_size, sent_photo_bytes, fb->len);
                    
                    // Note: BLE transmission throttling is handled by the cycle manager timing
                } else {
                    // Transmission complete - send end marker
                    Serial.printf("Photo transmission complete: %d bytes in %d frames\n", 
                                 sent_photo_bytes, sent_photo_frames);
                    
                    // Send end marker: [0xFF, 0xFF, 0x01]
                    uint8_t endMarker[3] = {0xFF, 0xFF, 0x01};
                    notifyPhotoData(endMarker, 3);
                    
                    // Clean up
                    if (fb) {
                        esp_camera_fb_return(fb);
                        fb = nullptr;
                    }
                    
                    photoDataUploading = false;
                    sent_photo_bytes = 0;
                    sent_photo_frames = 0;
                    
                    Serial.println("Photo transmission cycle completed");
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerConnectionMonitorCycle() {
        connection_monitor_cycle_id = registerIntervalCycle(
            "ConnectionMonitor",
            5000, // Check every 5 seconds
            []() {
                static bool lastConnected = false;
                bool currentConnected = isConnected();
                
                if (currentConnected != lastConnected) {
                    if (currentConnected) {
                        Serial.println("BLE connection established");
                        setLedPattern(LED_CONNECTED);
                    } else {
                        Serial.println("BLE connection lost");
                        setLedPattern(LED_DISCONNECTED);
                        
                        // Clean up any ongoing operations
                        if (photoDataUploading) {
                            Serial.println("Cleaning up photo upload due to disconnection");
                            if (fb) {
                                esp_camera_fb_return(fb);
                                fb = nullptr;
                            }
                            photoDataUploading = false;
                            sent_photo_bytes = 0;
                            sent_photo_frames = 0;
                        }
                    }
                    lastConnected = currentConnected;
                }
            },
            CYCLE_PRIORITY_NORMAL
        );
    }
} 