#include "opus_codec.h"

#ifdef CODEC_OPUS

#include <opus.h>
#include <Arduino.h>
#include "../../hal/constants.h"

// Static member definitions
OpusEncoder* OpusCodec::s_encoder = nullptr;
OpusDecoder* OpusCodec::s_decoder = nullptr;
bool OpusCodec::s_initialized = false;
bool OpusCodec::s_encoder_ready = false;
bool OpusCodec::s_decoder_ready = false;

bool OpusCodec::initialize() {
    if (s_initialized) {
        return true;
    }
    
    Serial.println("🎵 Initializing Opus codec...");
    
    // Initialize encoder
    if (!initializeEncoder()) {
        Serial.println("❌ Failed to initialize Opus encoder");
        cleanup();
        return false;
    }
    
    // Initialize decoder (optional for now)
    if (!initializeDecoder()) {
        Serial.println("⚠️  Failed to initialize Opus decoder (encoder-only mode)");
        // Don't fail completely - encoder-only mode is acceptable
    }
    
    s_initialized = true;
    Serial.println("✅ Opus codec initialized successfully");
    return true;
}

void OpusCodec::cleanup() {
    Serial.println("🧹 Cleaning up Opus codec...");
    
    cleanupEncoder();
    cleanupDecoder();
    
    s_initialized = false;
    Serial.println("✅ Opus codec cleanup completed");
}

int OpusCodec::encode(const int16_t* input_samples, size_t sample_count, uint8_t* output_buffer, size_t output_buffer_size) {
    if (!s_encoder_ready || !s_encoder) {
        Serial.println("❌ Opus encoder not ready");
        return -1;
    }
    
    // Validate input parameters
    if (!input_samples || !output_buffer || sample_count == 0 || output_buffer_size == 0) {
        Serial.println("❌ Invalid parameters for Opus encoding");
        return -1;
    }
    
    // Ensure we have the right number of samples for Opus frame
    if (sample_count != FRAME_SIZE) {
        Serial.printf("⚠️  Opus encoding: expected %d samples, got %zu\n", FRAME_SIZE, sample_count);
        // Pad or truncate as needed
        // For now, just process what we have up to FRAME_SIZE
        sample_count = min(sample_count, (size_t)FRAME_SIZE);
    }
    
    // Encode the audio frame
    int encoded_bytes = opus_encode(s_encoder, input_samples, sample_count, output_buffer, output_buffer_size);
    
    if (encoded_bytes < 0) {
        Serial.printf("❌ Opus encoding failed: %d (%s)\n", encoded_bytes, opus_strerror(encoded_bytes));
        return encoded_bytes;
    }
    
    // Debug logging (reduce frequency to avoid spam)
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 5000) {
        Serial.printf("🎵 Opus encoded %zu samples to %d bytes (%.1f%% compression)\n", 
                      sample_count, encoded_bytes, 
                      (float)encoded_bytes / (sample_count * 2) * 100.0);
        last_debug = millis();
    }
    
    return encoded_bytes;
}

int OpusCodec::decode(const uint8_t* input_data, size_t input_size, int16_t* output_samples, size_t max_samples) {
    if (!s_decoder_ready || !s_decoder) {
        Serial.println("❌ Opus decoder not ready");
        return -1;
    }
    
    // Validate input parameters
    if (!input_data || !output_samples || input_size == 0 || max_samples == 0) {
        Serial.println("❌ Invalid parameters for Opus decoding");
        return -1;
    }
    
    // Decode the audio frame
    int decoded_samples = opus_decode(s_decoder, input_data, input_size, output_samples, max_samples, 0);
    
    if (decoded_samples < 0) {
        Serial.printf("❌ Opus decoding failed: %d (%s)\n", decoded_samples, opus_strerror(decoded_samples));
        return decoded_samples;
    }
    
    // Debug logging (reduce frequency to avoid spam)
    static unsigned long last_debug = 0;
    if (millis() - last_debug > 5000) {
        Serial.printf("🎵 Opus decoded %zu bytes to %d samples\n", input_size, decoded_samples);
        last_debug = millis();
    }
    
    return decoded_samples;
}

bool OpusCodec::isReady() {
    return s_initialized && s_encoder_ready;
}

const char* OpusCodec::getEncoderInfo() {
    if (!s_encoder_ready) {
        return "Opus encoder not initialized";
    }
    
    static char info[128];
    snprintf(info, sizeof(info), "Opus encoder: %dHz, %d channels, %d kbps", 
             SAMPLE_RATE, CHANNELS, OPUS_BITRATE / 1000);
    return info;
}

const char* OpusCodec::getDecoderInfo() {
    if (!s_decoder_ready) {
        return "Opus decoder not initialized";
    }
    
    static char info[128];
    snprintf(info, sizeof(info), "Opus decoder: %dHz, %d channels", 
             SAMPLE_RATE, CHANNELS);
    return info;
}

bool OpusCodec::initializeEncoder() {
    if (s_encoder_ready) {
        return true;
    }
    
    Serial.println("🎵 Initializing Opus encoder...");
    
    int opus_err;
    s_encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION, &opus_err);
    
    if (opus_err != OPUS_OK || !s_encoder) {
        Serial.printf("❌ Failed to create Opus encoder: %d (%s)\n", opus_err, opus_strerror(opus_err));
        return false;
    }
    
    // Configure encoder settings
    opus_encoder_ctl(s_encoder, OPUS_SET_BITRATE(OPUS_BITRATE));
    
    // Optional: Configure additional settings
    opus_encoder_ctl(s_encoder, OPUS_SET_COMPLEXITY(5)); // Balance between quality and CPU usage
    opus_encoder_ctl(s_encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE)); // Optimize for voice
    
    s_encoder_ready = true;
    Serial.printf("✅ Opus encoder created: %dHz, %d channels, %d kbps\n", 
                  SAMPLE_RATE, CHANNELS, OPUS_BITRATE / 1000);
    
    return true;
}

bool OpusCodec::initializeDecoder() {
    if (s_decoder_ready) {
        return true;
    }
    
    Serial.println("🎵 Initializing Opus decoder...");
    
    int opus_err;
    s_decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opus_err);
    
    if (opus_err != OPUS_OK || !s_decoder) {
        Serial.printf("❌ Failed to create Opus decoder: %d (%s)\n", opus_err, opus_strerror(opus_err));
        return false;
    }
    
    s_decoder_ready = true;
    Serial.printf("✅ Opus decoder created: %dHz, %d channels\n", SAMPLE_RATE, CHANNELS);
    
    return true;
}

void OpusCodec::cleanupEncoder() {
    if (s_encoder) {
        opus_encoder_destroy(s_encoder);
        s_encoder = nullptr;
    }
    s_encoder_ready = false;
}

void OpusCodec::cleanupDecoder() {
    if (s_decoder) {
        opus_decoder_destroy(s_decoder);
        s_decoder = nullptr;
    }
    s_decoder_ready = false;
}

#endif // CODEC_OPUS 