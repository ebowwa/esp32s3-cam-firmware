#ifndef MICROPHONE_MANAGER_H
#define MICROPHONE_MANAGER_H

#include <stdint.h>
#include <stddef.h>

// Microphone manager class
class MicrophoneManager {
public:
    // Initialize microphone system
    static bool initialize();
    
    // Configure I2S microphone
    static bool configure();
    
    // Read audio data from microphone
    static size_t readAudio();
    
    // Get audio buffer pointers
    static uint8_t* getRecordingBuffer();
    static uint8_t* getCompressedBuffer();
    static uint8_t* getCompressedBuffer2();
    
    // Get buffer sizes
    static size_t getRecordingBufferSize();
    static size_t getCompressedBufferSize();
    
    // Cleanup resources
    static void cleanup();
    
    // Check if microphone is ready
    static bool isReady();

private:
    // Buffer management
    static uint8_t* s_recording_buffer;
    static uint8_t* s_raw_pdm_buffer;  // Raw PDM data before filtering
    static uint8_t* s_compressed_frame;
    static uint8_t* s_compressed_frame_2;
    
    // State tracking
    static bool s_initialized;
    static bool s_configured;
    
    // Audio processing moved to AudioFilters class
    
    // Internal methods
    static bool allocateBuffers();
    static void deallocateBuffers();
    static void logAudioData(size_t bytes_recorded);
    static size_t processPDMAudio(uint8_t* raw_data, size_t raw_bytes, int16_t* output_pcm);
};

#endif // MICROPHONE_MANAGER_H 