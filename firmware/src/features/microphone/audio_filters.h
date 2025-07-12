#ifndef AUDIO_FILTERS_H
#define AUDIO_FILTERS_H

#include <stdint.h>
#include <stddef.h>

class AudioFilters {
public:
    // Initialize audio filters
    static void initialize();
    
    // Apply all audio filters to PCM data
    static void applyFilters(int16_t* audio_data, size_t sample_count);
    
    // Individual filter functions
    static void applyDCBlockingFilter(int16_t* audio_data, size_t sample_count);
    static void applyHighPassFilter(int16_t* audio_data, size_t sample_count);
    static void applyGainControl(int16_t* audio_data, size_t sample_count);
    
    // Low-level filter implementations
    static float dcBlockingFilter(float input, float* state);
    static float highPassFilter(float input, float* state);
    
    // Reset filter states
    static void resetFilters();
    
private:
    // Filter state variables
    static float s_dc_filter_state;
    static float s_highpass_filter_state;
    
    // Filter constants
    static const float DC_FILTER_ALPHA;
    static const float HIGHPASS_FILTER_ALPHA;
    static const float GAIN_FACTOR;
};

#endif // AUDIO_FILTERS_H 