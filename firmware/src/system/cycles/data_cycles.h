#ifndef DATA_CYCLES_H
#define DATA_CYCLES_H

namespace DataCycles {
    void initialize();
    void registerAudioCaptureCycle();
    void registerPhotoCycle();
    void registerVideoStreamCycle();
    
    extern int audio_capture_cycle_id;
    extern int photo_cycle_id;
    extern int video_stream_cycle_id;
}

#endif // DATA_CYCLES_H 