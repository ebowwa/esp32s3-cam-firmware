#pragma once

#define CAMERA_MODEL_XIAO_ESP32S3
#include <Arduino.h>
#include "esp_camera.h"
#include "../platform/camera_pins.h"
#include "../platform/constants.h"

// Camera state variables (extern declarations)
extern camera_fb_t *fb;
extern bool isCapturingPhotos;
extern int captureInterval;
extern unsigned long lastCaptureTime;
extern size_t sent_photo_bytes;
extern size_t sent_photo_frames;
extern bool photoDataUploading;

// Video streaming state variables
extern bool isStreamingVideo;
extern int streamingFPS;
extern unsigned long lastStreamFrame;
extern bool isStreamingFrame;
extern unsigned long streamingStartTime;
extern size_t totalStreamingFrames;
extern size_t droppedFrames;

// Camera mode enumeration
typedef enum {
    CAMERA_MODE_IDLE,
    CAMERA_MODE_SINGLE_PHOTO,
    CAMERA_MODE_INTERVAL_PHOTO,
    CAMERA_MODE_VIDEO_STREAMING
} camera_mode_t;

// Camera configuration structure for fallback initialization
typedef struct {
    framesize_t frame_size;
    int jpeg_quality;
    camera_fb_location_t fb_location;
    int xclk_freq_hz;
    const char* description;
} CameraConfig;

extern camera_mode_t currentCameraMode;

// Video status structure
typedef struct {
  uint8_t streaming;      // 0 = stopped, 1 = streaming
  uint8_t fps;           // Current FPS
  uint16_t frameCount;   // Total frames sent
  uint16_t droppedFrames; // Dropped frames
} video_status_t;

// Camera functions - exact same interface as firmware.ino
void configure_camera();
bool take_photo();
void handlePhotoControl(int8_t controlValue);
bool initCameraWithConfig(const CameraConfig& config);

// Video streaming functions
void handleVideoControl(uint8_t controlValue);
void startVideoStreaming();
void stopVideoStreaming();
void setVideoFPS(uint8_t fps);
void configure_camera_for_streaming();
void configure_camera_for_photo();
bool shouldDropFrame();
void updateVideoStatus(); 