# Camera Module

This directory contains the camera functionality for the OpenGlass firmware.

## Files

- `camera.h` - Camera module header with function declarations and extern variables
- `camera.cpp` - Camera module implementation with all camera functions
- `README.md` - This documentation file

## Functions

### Core Camera Functions
- `configure_camera()` - Initialize and configure the ESP32 camera
- `take_photo()` - Capture a single photo with retry logic
- `handlePhotoControl(int8_t controlValue)` - Handle BLE photo control commands

### Camera State Variables
- `camera_fb_t *fb` - Current camera frame buffer
- `bool isCapturingPhotos` - Photo capture state flag
- `int captureInterval` - Interval between photos (milliseconds)
- `unsigned long lastCaptureTime` - Timestamp of last photo capture
- `size_t sent_photo_bytes` - Bytes sent during photo transmission
- `size_t sent_photo_frames` - Number of frames sent during transmission
- `bool photoDataUploading` - Photo upload state flag

## Usage

```cpp
#include "src/camera/camera.h"

// Initialize camera
configure_camera();

// Take a photo
if (take_photo()) {
    // Photo captured successfully in global 'fb' variable
    Serial.printf("Photo size: %d bytes\n", fb->len);
}

// Handle photo control command
handlePhotoControl(PHOTO_SINGLE_SHOT);
```

## Photo Control Commands

- `PHOTO_SINGLE_SHOT` (-1) - Take a single photo
- `PHOTO_STOP` (0) - Stop photo capture
- `PHOTO_MIN_INTERVAL` to `PHOTO_MAX_INTERVAL` (5-300) - Start interval capture

## Camera Configuration

The camera is configured for:
- **Frame Size**: Low quality for efficient transmission
- **Pixel Format**: JPEG compression
- **Frame Buffer**: Located in PSRAM
- **Quality**: Optimized for BLE transmission
- **Retry Logic**: 3 attempts with timing delays

## Dependencies

- `../hardware/camera_pins.h` - Camera pin definitions
- `../platform/constants.h` - Camera configuration constants
- `../utils/timing.h` - Timing utilities for delays and measurements
- `../core/device_status.h` - Device status management

## Integration

This module is designed to work with the existing OpenGlass BLE photo transmission system. The camera functions maintain the same interface as the original firmware implementation for seamless integration. 