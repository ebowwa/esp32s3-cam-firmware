#ifndef OPUS_CODEC_H
#define OPUS_CODEC_H

#include <stdint.h>
#include <stddef.h>

#ifdef CODEC_OPUS

// Forward declaration
class OpusEncoder;
class OpusDecoder;

class OpusCodec {
public:
    // Initialize Opus codec
    static bool initialize();
    
    // Cleanup Opus codec
    static void cleanup();
    
    // Encode audio samples to Opus format
    static int encode(const int16_t* input_samples, size_t sample_count, uint8_t* output_buffer, size_t output_buffer_size);
    
    // Decode Opus data to audio samples
    static int decode(const uint8_t* input_data, size_t input_size, int16_t* output_samples, size_t max_samples);
    
    // Check if codec is ready
    static bool isReady();
    
    // Get encoder/decoder info
    static const char* getEncoderInfo();
    static const char* getDecoderInfo();
    
private:
    // Opus encoder/decoder instances
    static OpusEncoder* s_encoder;
    static OpusDecoder* s_decoder;
    
    // State tracking
    static bool s_initialized;
    static bool s_encoder_ready;
    static bool s_decoder_ready;
    
    // Internal initialization methods
    static bool initializeEncoder();
    static bool initializeDecoder();
    static void cleanupEncoder();
    static void cleanupDecoder();
};

#endif // CODEC_OPUS

#endif // OPUS_CODEC_H 