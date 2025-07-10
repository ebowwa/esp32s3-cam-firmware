#pragma once

#include "xiao_esp32s3_constants.h"

// Audio Configuration
#define CODEC_PCM
// #define CODEC_MULAW
// #define CODEC_OPUS

#ifdef CODEC_OPUS
#define OPUS_APPLICATION OPUS_APPLICATION_VOIP
#define OPUS_BITRATE 16000
#define CHANNELS 1
#define MAX_PACKET_SIZE 1000
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define FRAME_SIZE 160
#else
#ifdef CODEC_MULAW
#define SAMPLE_RATE 8000
#define SAMPLE_BITS 16
#else
#define FRAME_SIZE 160
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#endif
#endif

// Audio Buffer Configuration
#ifdef CODEC_OPUS
static const size_t RECORDING_BUFFER_SIZE = FRAME_SIZE * 2; // 16-bit samples
static const size_t COMPRESSED_BUFFER_SIZE = MAX_PACKET_SIZE;
#else
#ifdef CODEC_MULAW
static const size_t RECORDING_BUFFER_SIZE = 400;
static const size_t COMPRESSED_BUFFER_SIZE = 400 + 3; /* header */
#else
static const size_t RECORDING_BUFFER_SIZE = FRAME_SIZE * 2; // 16-bit samples
static const size_t COMPRESSED_BUFFER_SIZE = RECORDING_BUFFER_SIZE + 3; /* header */
#endif
#endif

#define VOLUME_GAIN 2

// Device Information - Using XIAO ESP32-S3 constants
// Note: BLE Service UUIDs are now defined in src/bluetooth/ble_services.h
static const char* DEVICE_NAME = "OpenGlass";

// Photo Control Commands
#define PHOTO_SINGLE_SHOT -1
#define PHOTO_STOP 0
#define PHOTO_MIN_INTERVAL 5
#define PHOTO_MAX_INTERVAL 300

// Video Control Commands
#define VIDEO_STREAM_START 1
#define VIDEO_STREAM_STOP 0
#define VIDEO_SET_FPS_1 1
#define VIDEO_SET_FPS_2 2
#define VIDEO_SET_FPS_5 5
#define VIDEO_SET_FPS_10 10

// Video Stream Configuration
#define VIDEO_STREAM_FPS_MIN 1
#define VIDEO_STREAM_FPS_MAX 10
#define VIDEO_STREAM_DEFAULT_FPS 5
#define VIDEO_STREAM_FRAME_INTERVAL(fps) (1000 / fps)

// Camera Configuration
#define CAMERA_JPEG_QUALITY 10
#define CAMERA_FRAME_SIZE_HIGH FRAMESIZE_UXGA
#define CAMERA_FRAME_SIZE_LOW FRAMESIZE_SVGA
#define CAMERA_XCLK_FREQ 20000000
#define CAMERA_FB_COUNT 1

// Streaming-specific Camera Configuration
#define CAMERA_STREAMING_QUALITY 25        // Higher number = smaller file size
#define CAMERA_STREAMING_FRAME_SIZE FRAMESIZE_QQVGA  // Even smaller for streaming (160x120)

// Photo Transfer Configuration
#define PHOTO_CHUNK_SIZE 400  // Increased from 200 for better throughput
#define PHOTO_END_MARKER_LOW 0xFF
#define PHOTO_END_MARKER_HIGH 0xFF

// Timing Configuration
#define BATTERY_UPDATE_INTERVAL 60000  // 60 seconds
#define MAIN_LOOP_DELAY 10             // Reduced from 20ms to 10ms for faster streaming

// I2S Pin Configuration - Using XIAO ESP32-S3 constants
#define I2S_WS_PIN XIAO_ESP32S3_SENSE_PIN_D11   // GPIO42
#define I2S_SCK_PIN XIAO_ESP32S3_SENSE_PIN_D12  // GPIO41

// Codec IDs for BLE
#ifdef CODEC_OPUS
#define CODEC_ID 20  // Opus 16khz
#else
#ifdef CODEC_MULAW
#define CODEC_ID 11  // MuLaw 8khz
#else
#define CODEC_ID 1   // PCM 8khz
#endif
#endif 

// Device Status Values
// These values are sent via BLE to inform the client about device state
// 0x01: Device is initializing (startup phase)
// 0x02: Device is warming up (sensors stabilizing)
// 0x03: Device is ready for normal operation
// 0x04: Device encountered an error
// 0x05: Camera initialization in progress
// 0x06: Microphone initialization in progress
// 0x07: BLE initialization in progress
// 0x08: Battery not detected
// 0x09: Device is charging
// 0x0A: Battery connection unstable
#define DEVICE_STATUS_INITIALIZING 0x01
#define DEVICE_STATUS_WARMING_UP 0x02
#define DEVICE_STATUS_READY 0x03
#define DEVICE_STATUS_ERROR 0x04
#define DEVICE_STATUS_CAMERA_INIT 0x05
#define DEVICE_STATUS_MICROPHONE_INIT 0x06
#define DEVICE_STATUS_BLE_INIT 0x07
#define DEVICE_STATUS_BATTERY_NOT_DETECTED 0x08
#define DEVICE_STATUS_CHARGING 0x09
#define DEVICE_STATUS_BATTERY_UNSTABLE 0x0A

// Battery Detection Configuration - Using XIAO ESP32-S3 constants
#define BATTERY_ADC_PIN XIAO_ESP32S3_PIN_A0       // ADC pin for battery voltage reading
#define BATTERY_MIN_VOLTAGE 3.0                   // Minimum voltage to consider battery present (V)
#define BATTERY_MAX_VOLTAGE 4.3                   // Maximum expected battery voltage (V)
#define BATTERY_VOLTAGE_DIVIDER 1.0               // Voltage divider ratio - may need calibration
#define BATTERY_ADC_RESOLUTION XIAO_ESP32S3_ADC_MAX_VALUE  // 12-bit ADC resolution
#define BATTERY_REFERENCE_VOLTAGE (XIAO_ESP32S3_ADC_VREF_MV / 1000.0)  // Reference voltage for ADC (V)
#define BATTERY_CHECK_SAMPLES 10                  // Number of samples for battery voltage averaging

// Battery Connection Monitoring
#define BATTERY_STABILITY_SAMPLES 5               // Number of recent readings to track for stability
#define BATTERY_VOLTAGE_TOLERANCE 0.3             // Maximum voltage variation to consider stable (V)
#define BATTERY_DISCONNECT_THRESHOLD 2.5          // Voltage below which we consider battery disconnected (V)
#define BATTERY_RECONNECT_THRESHOLD 3.2           // Voltage above which we consider battery reconnected (V)
#define BATTERY_INSTABILITY_COUNT 3               // Number of unstable readings before flagging as unstable
#define BATTERY_RAPID_CHANGE_THRESHOLD 0.5        // Voltage change that indicates connection issue (V)

// USB Power/Charging Detection Configuration - Using XIAO ESP32-S3 constants
#define USB_POWER_ADC_PIN XIAO_ESP32S3_PIN_A1     // ADC pin for USB power detection
#define USB_POWER_VOLTAGE_DIVIDER 2.27            // Voltage divider ratio (100K + 47K) / 47K = 3.13, but use 2.27 for safety
#define USB_POWER_THRESHOLD 2.5                   // Minimum voltage to consider USB power present (V)
#define USB_POWER_NOMINAL XIAO_ESP32S3_USB_INPUT_VOLTAGE_V  // Expected USB power voltage (V)
#define CHARGING_VOLTAGE_THRESHOLD 4.1            // Battery voltage above which we consider it charging (V)

// Note: BATTERY_VOLTAGE_DIVIDER may need adjustment based on actual hardware
// If readings seem too high/low, try adjusting this value:
// - If battery voltage reads too high: Decrease the value
// - If battery voltage reads too low: Increase the value
//
// USB Power Detection Circuit:
// Connect 5V pin through 100K resistor to USB_POWER_ADC_PIN
// Connect USB_POWER_ADC_PIN through 47K resistor to GND
// This creates a voltage divider: 5V * (47K / (100K + 47K)) = ~1.6V
// Adjust USB_POWER_VOLTAGE_DIVIDER if using different resistor values 