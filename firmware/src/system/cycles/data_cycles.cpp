#include "data_cycles.h"
#include "cycle_manager.h"
#include "../../features/camera/camera.h"
#include "../../features/bluetooth/ble_data_handler.h"
#include "../../features/microphone/microphone_manager.h"
#include "../../hal/constants.h"
#include "../clock/timing.h"
#include "esp_camera.h"
#include <Arduino.h>

// External variables
extern bool deviceReady;
extern bool isCapturingPhotos;
extern int captureInterval;
extern unsigned long lastCaptureTime;
extern size_t sent_photo_bytes;
extern size_t sent_photo_frames;
extern bool photoDataUploading;
extern bool isStreamingFrame;
extern bool isStreamingVideo;
extern int streamingFPS;
extern unsigned long lastStreamFrame;
extern size_t totalStreamingFrames;
extern camera_fb_t *fb;

// Function declarations
extern bool isConnected();
extern bool shouldDropFrame();
extern bool take_photo();

// ===================================================================
// DATA ACQUISITION CYCLE MANAGER
// ===================================================================

namespace DataCycles {
    int audio_capture_cycle_id = -1;
    int photo_cycle_id = -1;
    int video_stream_cycle_id = -1;
    
    void initialize() {
        Serial.println("Initializing Data Cycles...");
        registerAudioCaptureCycle();
        registerPhotoCycle();
        // registerVideoStreamCycle();   // DISABLED: Not needed for photo capture
        Serial.println("Data cycles: Audio capture and photo capture enabled");
    }
    
    void registerAudioCaptureCycle() {
        audio_capture_cycle_id = registerConditionCycle(
            "AudioCapture",
            []() {
                // Always capture audio when microphone is ready
                bool micReady = MicrophoneManager::isReady();
                
                // Debug logging every 5 seconds
                static unsigned long lastConditionLog = 0;
                if (shouldExecute(&lastConditionLog, 5000)) {
                    Serial.printf("🎤 Audio Capture Condition: micReady=%s\n", micReady ? "YES" : "NO");
                    Serial.printf("🎤 Connected=%s, deviceReady=%s\n", 
                                  isConnected() ? "YES" : "NO", 
                                  deviceReady ? "YES" : "NO");
                }
                
                // Always capture when microphone is ready
                return micReady;
            },
            []() {
                // Add debug logging for audio capture attempts
                static unsigned long lastCaptureLog = 0;
                if (shouldExecute(&lastCaptureLog, 2000)) {
                    Serial.println("🎤 Audio capture cycle executing...");
                }
                
                // Read microphone data with larger buffer for continuous recording
                size_t bytes_recorded = MicrophoneManager::readAudio();
                
                if (bytes_recorded > 0) {
                    Serial.printf("🎤 Got %d bytes of audio data!\n", bytes_recorded);
                    
                    uint8_t* recording_buffer = MicrophoneManager::getRecordingBuffer();
                    if (recording_buffer) {
                        // Always transmit audio data if we have it
                        if (isConnected()) {
                            transmitAudioData(recording_buffer, MicrophoneManager::getRecordingBufferSize(), bytes_recorded);
                        } else {
                            Serial.println("🎤 Audio captured but not connected - data ready for transmission");
                        }
                        
                        // Enhanced logging for audio capture
                        static unsigned long lastAudioLog = 0;
                        static size_t totalBytesRecorded = 0;
                        static unsigned long audioStartTime = 0;
                        static size_t frameCount = 0;
                        
                        if (audioStartTime == 0) audioStartTime = millis();
                        totalBytesRecorded += bytes_recorded;
                        frameCount++;
                        
                        if (shouldExecute(&lastAudioLog, 3000)) { // Log every 3 seconds
                            float duration_s = (millis() - audioStartTime) / 1000.0;
                            float expected_bytes = duration_s * SAMPLE_RATE * 2; // 16-bit samples
                            float capture_rate = (totalBytesRecorded / expected_bytes) * 100.0;
                            
                            Serial.printf("🎤 Audio: %d bytes this frame, %d total frames, %.1f%% capture rate over %.1fs\n", 
                                          bytes_recorded, frameCount, capture_rate, duration_s);
                        }
                    }
                } else {
                    // Log when no audio data is captured
                    static unsigned long lastNoAudioLog = 0;
                    if (shouldExecute(&lastNoAudioLog, 5000)) {
                        Serial.println("⚠️  No audio data captured in this cycle");
                    }
                }
            },
            CYCLE_PRIORITY_HIGH  // High priority for continuous audio capture
        );
    }
    
    void registerPhotoCycle() {
        photo_cycle_id = registerConditionCycle(
            "PhotoCapture",
            []() {
                if (!deviceReady || !isConnected() || photoDataUploading) {
                    return false;
                }
                
                // Check if we should capture based on control flags
                bool should_capture = isCapturingPhotos && (captureInterval == 0 || 
                    getElapsedTime(lastCaptureTime) >= (captureInterval * 1000));
                
                return should_capture;
            },
            []() {
                Serial.println("Taking photo...");
                
                // Take photo
                if (take_photo()) {
                    Serial.printf("Photo captured: %d bytes\n", fb->len);
                    
                    // Set flags for data transmission
                    photoDataUploading = true;
                    lastCaptureTime = measureStart();
                    
                    // Update capture count
                    if (captureInterval == 0) {
                        // Single capture mode - stop after one photo
                        isCapturingPhotos = false;
                        Serial.println("Single photo capture completed");
                    }
                } else {
                    Serial.println("Photo capture failed");
                }
            },
            CYCLE_PRIORITY_HIGH
        );
    }
    
    void registerVideoStreamCycle() {
        video_stream_cycle_id = registerConditionCycle(
            "VideoStream",
            []() {
                // Video streaming when connected and streaming is active
                return isConnected() && deviceReady && isStreamingVideo && !photoDataUploading;
            },
            []() {
                // Video streaming logic would go here
                Serial.println("Video stream cycle (placeholder)");
            },
            CYCLE_PRIORITY_HIGH
        );
    }
} 