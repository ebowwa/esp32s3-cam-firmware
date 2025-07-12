#include "audio_filters.h"
#include <Arduino.h>

// Static member definitions
float AudioFilters::s_dc_filter_state = 0.0f;
float AudioFilters::s_highpass_filter_state = 0.0f;

// Filter constants - MUCH gentler filtering to preserve speech
const float AudioFilters::DC_FILTER_ALPHA = 0.999f;        // Very gentle DC blocking
const float AudioFilters::HIGHPASS_FILTER_ALPHA = 0.99f;   // Very gentle high-pass
const float AudioFilters::GAIN_FACTOR = 1.5f;              // 3dB gain instead of 6dB

void AudioFilters::initialize() {
    Serial.println("🎛️ Initializing audio filters...");
    resetFilters();
    Serial.println("✅ Audio filters initialized");
}

void AudioFilters::resetFilters() {
    s_dc_filter_state = 0.0f;
    s_highpass_filter_state = 0.0f;
}

void AudioFilters::applyFilters(int16_t* audio_data, size_t sample_count) {
    // Apply all filters in sequence
    applyDCBlockingFilter(audio_data, sample_count);
    // Skip high-pass filter for now - it was removing speech
    // applyHighPassFilter(audio_data, sample_count);
    applyGainControl(audio_data, sample_count);
}

void AudioFilters::applyDCBlockingFilter(int16_t* audio_data, size_t sample_count) {
    // MUCH gentler filtering to preserve speech
    for (size_t i = 0; i < sample_count; i++) {
        float sample = (float)audio_data[i];
        
        // Very gentle DC blocking filter - only removes true DC offset
        sample = dcBlockingFilter(sample, &s_dc_filter_state);
        
        // Clamp to 16-bit range
        if (sample > 32767.0f) sample = 32767.0f;
        if (sample < -32768.0f) sample = -32768.0f;
        
        audio_data[i] = (int16_t)sample;
    }
}

void AudioFilters::applyHighPassFilter(int16_t* audio_data, size_t sample_count) {
    // Currently disabled - was too aggressive and removed speech
    // for (size_t i = 0; i < sample_count; i++) {
    //     float sample = (float)audio_data[i];
    //     sample = highPassFilter(sample, &s_highpass_filter_state);
    //     audio_data[i] = (int16_t)sample;
    // }
}

void AudioFilters::applyGainControl(int16_t* audio_data, size_t sample_count) {
    // Much gentler gain - preserve dynamics
    for (size_t i = 0; i < sample_count; i++) {
        float sample = (float)audio_data[i] * GAIN_FACTOR;
        
        // Soft clipping to prevent harsh distortion
        if (sample > 32767.0f) sample = 32767.0f;
        if (sample < -32768.0f) sample = -32768.0f;
        
        audio_data[i] = (int16_t)sample;
    }
}

float AudioFilters::dcBlockingFilter(float input, float* state) {
    // Very gentle DC blocking - only removes true DC component
    // y[n] = x[n] - x[n-1] + α * y[n-1]
    float output = input - *state + DC_FILTER_ALPHA * (*state);
    *state = input;
    return output;
}

float AudioFilters::highPassFilter(float input, float* state) {
    // Disabled for now - was too aggressive
    return input;
} 