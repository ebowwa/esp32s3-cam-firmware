#include "microphone_manager.h"
#include "audio_filters.h"
#ifdef CODEC_OPUS
#include "opus_codec.h"
#endif
#include "driver/i2s.h"  // Use ESP-IDF driver for proper PDM support
#include <Arduino.h>
#include "../../hal/constants.h"
#include "../../system/memory/memory_utils.h"
#include "../../status/device_status.h"

// Static member definitions
uint8_t* MicrophoneManager::s_recording_buffer = nullptr;
uint8_t* MicrophoneManager::s_raw_pdm_buffer = nullptr;
uint8_t* MicrophoneManager::s_compressed_frame = nullptr;
uint8_t* MicrophoneManager::s_compressed_frame_2 = nullptr;
bool MicrophoneManager::s_initialized = false;
bool MicrophoneManager::s_configured = false;

// Audio processing state moved to AudioFilters class
// Opus codec moved to OpusCodec class

bool MicrophoneManager::initialize() {
    if (s_initialized) {
        return true;
    }
    
    Serial.println("🎤 Initializing microphone manager for XIAO ESP32S3 Sense...");
    
    // Initialize audio filters
    AudioFilters::initialize();
    
    // Initialize Opus codec if enabled
    #ifdef CODEC_OPUS
    if (!OpusCodec::initialize()) {
        Serial.println("Failed to initialize Opus codec!");
        return false;
    }
    #endif
    
    s_initialized = true;
    return true;
}

bool MicrophoneManager::configure() {
    if (!s_initialized) {
        Serial.println("❌ Microphone manager not initialized!");
        return false;
    }
    
    if (s_configured) {
        Serial.println("✅ Microphone already configured");
        return true;
    }
    
    Serial.println("🎤 Configuring XIAO ESP32S3 Sense PDM microphone...");
    
    // ESP-IDF I2S configuration for XIAO ESP32S3 Sense PDM microphone
    // Based on working examples from Seeed Studio and Edge Impulse
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // XIAO ESP32S3 uses left channel
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    // Pin configuration for XIAO ESP32S3 Sense
    // PDM_CLK → GPIO42, PDM_DATA → GPIO41
    i2s_pin_config_t pin_config = {
        .bck_io_num = -1,           // Not used for PDM
        .ws_io_num = 42,            // PDM CLK pin
        .data_out_num = -1,         // Not used for RX
        .data_in_num = 41,          // PDM DATA pin
    };
    
    Serial.printf("🎤 I2S Config: sample_rate=%d, dma_buf_count=%d, dma_buf_len=%d\n", 
                  i2s_config.sample_rate, i2s_config.dma_buf_count, i2s_config.dma_buf_len);
    Serial.printf("🎤 Pin Config: ws_io_num=%d (CLK), data_in_num=%d (DATA)\n", 
                  pin_config.ws_io_num, pin_config.data_in_num);
    
    // Install I2S driver
    esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        Serial.printf("❌ Failed to install I2S driver: %s\n", esp_err_to_name(ret));
        return false;
    }
    Serial.println("✅ I2S driver installed successfully");
    
    // Set I2S pin configuration
    ret = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (ret != ESP_OK) {
        Serial.printf("❌ Failed to set I2S pins: %s\n", esp_err_to_name(ret));
        i2s_driver_uninstall(I2S_NUM_0);
        return false;
    }
    Serial.println("✅ I2S pins configured successfully");
    
    // Zero DMA buffer
    ret = i2s_zero_dma_buffer(I2S_NUM_0);
    if (ret != ESP_OK) {
        Serial.printf("❌ Failed to zero DMA buffer: %s\n", esp_err_to_name(ret));
        i2s_driver_uninstall(I2S_NUM_0);
        return false;
    }
    Serial.println("✅ DMA buffer zeroed successfully");
    
    // Test I2S read to verify it's working
    uint8_t test_buffer[128];
    size_t bytes_read = 0;
    ret = i2s_read(I2S_NUM_0, test_buffer, sizeof(test_buffer), &bytes_read, pdMS_TO_TICKS(100));
    if (ret == ESP_OK && bytes_read > 0) {
        Serial.printf("✅ I2S test read successful: %d bytes\n", bytes_read);
    } else {
        Serial.printf("⚠️  I2S test read failed or no data: %s, bytes=%d\n", esp_err_to_name(ret), bytes_read);
    }
    
    // Allocate audio buffers
    if (!allocateBuffers()) {
        Serial.println("❌ Failed to allocate audio buffers!");
        i2s_driver_uninstall(I2S_NUM_0);
        return false;
    }
    Serial.println("✅ Audio buffers allocated successfully");
    
    Serial.printf("🎤 DMA buffers: %d x %d bytes = %d total bytes\n", 
                  i2s_config.dma_buf_count, i2s_config.dma_buf_len * 2, 
                  i2s_config.dma_buf_count * i2s_config.dma_buf_len * 2);
    Serial.printf("🎤 Recording buffer: %d bytes (%.1f ms audio)\n", 
                  RECORDING_BUFFER_SIZE, 
                  (float)RECORDING_BUFFER_SIZE / 2.0 / SAMPLE_RATE * 1000.0);
    
    s_configured = true;
    Serial.println("🎤 Microphone configuration completed successfully");
    
    return true;
}

size_t MicrophoneManager::readAudio() {
    if (!s_configured || !s_recording_buffer) {
        Serial.println("Microphone not configured or buffer not allocated!");
        return 0;
    }
    
    size_t bytes_read = 0;
    
    // Read audio data using ESP-IDF I2S driver
    esp_err_t ret = i2s_read(I2S_NUM_0, s_recording_buffer, RECORDING_BUFFER_SIZE, &bytes_read, pdMS_TO_TICKS(100));
    
    if (ret != ESP_OK) {
        Serial.printf("I2S read failed: %s\n", esp_err_to_name(ret));
        return 0;
    }
    
    // Log audio data for debugging
    logAudioData(bytes_read);
    
    return bytes_read;
}

size_t MicrophoneManager::processPDMAudio(uint8_t* raw_data, size_t raw_bytes, int16_t* output_pcm) {
    // The ESP32-S3 I2S PDM already converts PDM to PCM, just copy the data
    size_t sample_count = raw_bytes / 2;  // 16-bit samples
    int16_t* input_samples = (int16_t*)raw_data;
    
    for (size_t i = 0; i < sample_count; i++) {
        // Direct copy - ESP32-S3 outputs clean PCM from PDM
        output_pcm[i] = input_samples[i];
    }
    
    return raw_bytes;  // Same size after processing
}

// Audio filtering methods moved to AudioFilters class

uint8_t* MicrophoneManager::getRecordingBuffer() {
    return s_recording_buffer;
}

uint8_t* MicrophoneManager::getCompressedBuffer() {
    return s_compressed_frame;
}

uint8_t* MicrophoneManager::getCompressedBuffer2() {
    return s_compressed_frame_2;
}

size_t MicrophoneManager::getRecordingBufferSize() {
    return RECORDING_BUFFER_SIZE;
}

size_t MicrophoneManager::getCompressedBufferSize() {
    return COMPRESSED_BUFFER_SIZE;
}

void MicrophoneManager::cleanup() {
    Serial.println("Cleaning up microphone manager...");
    
    #ifdef CODEC_OPUS
    OpusCodec::cleanup();
    #endif
    
    // Uninstall I2S driver
    if (s_configured) {
        i2s_driver_uninstall(I2S_NUM_0);
        Serial.println("✅ I2S stopped");
    }
    
    deallocateBuffers();
    
    s_configured = false;
    s_initialized = false;
}

bool MicrophoneManager::isReady() {
    return s_initialized && s_configured && s_recording_buffer != nullptr;
}

bool MicrophoneManager::allocateBuffers() {
    // Allocate buffers with tracking - using larger buffers for continuous recording
    Serial.printf("Allocating audio buffers: Recording=%d bytes, Compressed=%d bytes\n", 
                  RECORDING_BUFFER_SIZE, COMPRESSED_BUFFER_SIZE);
    
    s_recording_buffer = (uint8_t*)PS_CALLOC_TRACKED(RECORDING_BUFFER_SIZE, sizeof(uint8_t), "AudioRecording");
    s_compressed_frame = (uint8_t*)PS_CALLOC_TRACKED(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t), "AudioCompressed");
    s_compressed_frame_2 = (uint8_t*)PS_CALLOC_TRACKED(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t), "AudioCompressed2");
    
    if (!s_recording_buffer || !s_compressed_frame || !s_compressed_frame_2) {
        deallocateBuffers();
        return false;
    }
    
    return true;
}

void MicrophoneManager::deallocateBuffers() {
    if (s_recording_buffer) {
        SAFE_FREE(s_recording_buffer);
        s_recording_buffer = nullptr;
    }
    
    if (s_compressed_frame) {
        SAFE_FREE(s_compressed_frame);
        s_compressed_frame = nullptr;
    }
    
    if (s_compressed_frame_2) {
        SAFE_FREE(s_compressed_frame_2);
        s_compressed_frame_2 = nullptr;
    }
}

void MicrophoneManager::logAudioData(size_t bytes_recorded) {
    // Debug logging every 5 seconds
    static unsigned long lastDebugLog = 0;
    if (millis() - lastDebugLog > 5000) {
        Serial.printf("ESP-IDF I2S read: %d bytes requested, %d bytes received (%.1f%% filled)\n", 
                      RECORDING_BUFFER_SIZE, bytes_recorded, 
                      (float)bytes_recorded / RECORDING_BUFFER_SIZE * 100.0);
        
        if (bytes_recorded > 0) {
            // Show first few bytes as hex to see the data pattern
            Serial.printf("First 8 bytes: ");
            for (int i = 0; i < min(8, (int)bytes_recorded); i++) {
                Serial.printf("%02X ", s_recording_buffer[i]);
            }
            Serial.println();
            
            // Calculate audio duration for this buffer
            float duration_ms = (float)bytes_recorded / 2.0 / SAMPLE_RATE * 1000.0;
            Serial.printf("Audio duration: %.1f ms (direct I2S read)\n", duration_ms);
        }
        lastDebugLog = millis();
    }
} 