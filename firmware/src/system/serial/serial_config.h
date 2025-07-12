#ifndef SERIAL_CONFIG_H
#define SERIAL_CONFIG_H

#include "../../hal/xiao_esp32s3_constants.h"

// ===================================================================
// SERIAL COMMUNICATION CONFIGURATION
// ===================================================================

// Default serial configuration
#define SERIAL_DEFAULT_BAUD_RATE XIAO_ESP32S3_SERIAL_BAUD_RATE
#define SERIAL_DEFAULT_BUFFER_SIZE XIAO_ESP32S3_SERIAL_BUFFER_SIZE
#define SERIAL_DEFAULT_TIMEOUT_MS 1000

// Serial port configuration
#define SERIAL_DATA_BITS 8
#define SERIAL_STOP_BITS 1
#define SERIAL_PARITY SERIAL_8N1

// Hardware serial pins
#define SERIAL_RX_PIN XIAO_ESP32S3_PIN_RX
#define SERIAL_TX_PIN XIAO_ESP32S3_PIN_TX

// ===================================================================
// LOGGING CONFIGURATION
// ===================================================================

// Default log level for different builds
#ifdef DEBUG
    #define DEFAULT_LOG_LEVEL LOG_LEVEL_DEBUG
    #define DEFAULT_DEBUG_CATEGORIES DEBUG_ALL
#else
    #define DEFAULT_LOG_LEVEL LOG_LEVEL_INFO
    #define DEFAULT_DEBUG_CATEGORIES (DEBUG_SYSTEM | DEBUG_BLE | DEBUG_CAMERA)
#endif

// Performance monitoring settings
#define PERFORMANCE_REPORT_INTERVAL_MS 30000  // 30 seconds
#define ENABLE_PERFORMANCE_MONITORING true
#define ENABLE_MEMORY_TRACKING true
#define ENABLE_TIMING_DEBUG true

// ===================================================================
// MODULE-SPECIFIC LOGGING SETTINGS
// ===================================================================

// BLE logging configuration
#define BLE_LOG_CONNECTIONS true
#define BLE_LOG_DATA_TRANSFER true
#define BLE_LOG_CHARACTERISTICS true
#define BLE_LOG_SERVICES true

// Camera logging configuration
#define CAMERA_LOG_CAPTURE true
#define CAMERA_LOG_STREAMING true
#define CAMERA_LOG_CONFIGURATION true
#define CAMERA_LOG_ERRORS true

// Microphone logging configuration
#define MICROPHONE_LOG_INITIALIZATION true
#define MICROPHONE_LOG_AUDIO_DATA false  // Can be verbose
#define MICROPHONE_LOG_CODEC_OPERATIONS true
#define MICROPHONE_LOG_BUFFER_STATUS true

// Battery logging configuration
#define BATTERY_LOG_VOLTAGE_READINGS true
#define BATTERY_LOG_CHARGING_STATUS true
#define BATTERY_LOG_LEVEL_CHANGES true
#define BATTERY_LOG_CONNECTION_ISSUES true

// Power management logging configuration
#define POWER_LOG_MODE_CHANGES true
#define POWER_LOG_SLEEP_OPERATIONS true
#define POWER_LOG_CPU_FREQUENCY true
#define POWER_LOG_CONSUMPTION true

// Memory logging configuration
#define MEMORY_LOG_ALLOCATIONS false  // Can be very verbose
#define MEMORY_LOG_DEALLOCATIONS false  // Can be very verbose
#define MEMORY_LOG_LEAKS true
#define MEMORY_LOG_FRAGMENTATION true

// Cycle manager logging configuration
#define CYCLES_LOG_REGISTRATION true
#define CYCLES_LOG_EXECUTION false  // Can be very verbose
#define CYCLES_LOG_TIMING true
#define CYCLES_LOG_ERRORS true

// ===================================================================
// SERIAL OUTPUT FORMATTING
// ===================================================================

// Timestamp format
#define TIMESTAMP_FORMAT_MILLIS true
#define TIMESTAMP_FORMAT_SECONDS false

// Module name formatting
#define MODULE_NAME_WIDTH 12
#define MODULE_NAME_PADDING true

// Log level formatting
#define LOG_LEVEL_WIDTH 7
#define LOG_LEVEL_COLORS false  // ESP32 serial doesn't support colors

// Message formatting
#define MAX_MESSAGE_LENGTH 512
#define LINE_SEPARATOR_LENGTH 60
#define HEADER_SEPARATOR_CHAR '='
#define SUBHEADER_SEPARATOR_CHAR '-'

// ===================================================================
// BUFFER MANAGEMENT
// ===================================================================

// Serial buffer sizes
#define SERIAL_TX_BUFFER_SIZE 1024
#define SERIAL_RX_BUFFER_SIZE 256

// Message queue settings
#define SERIAL_MESSAGE_QUEUE_SIZE 50
#define SERIAL_MESSAGE_TIMEOUT_MS 100

// ===================================================================
// PERFORMANCE SETTINGS
// ===================================================================

// Throttling settings
#define SERIAL_MAX_MESSAGES_PER_SECOND 100
#define SERIAL_THROTTLE_THRESHOLD_MS 10

// Batch processing
#define SERIAL_BATCH_SIZE 10
#define SERIAL_BATCH_TIMEOUT_MS 50

// ===================================================================
// ERROR HANDLING
// ===================================================================

// Error recovery settings
#define SERIAL_MAX_RETRY_ATTEMPTS 3
#define SERIAL_RETRY_DELAY_MS 100

// Error reporting
#define REPORT_SERIAL_ERRORS true
#define REPORT_BUFFER_OVERFLOWS true
#define REPORT_TIMEOUT_ERRORS true

// ===================================================================
// CONDITIONAL COMPILATION
// ===================================================================

// Feature flags
#define ENABLE_SERIAL_MANAGER true
#define ENABLE_DEBUG_LOGGER true
#define ENABLE_PERFORMANCE_MONITORING true
#define ENABLE_MEMORY_TRACKING true
#define ENABLE_TIMING_DEBUG true

// Platform-specific features
#define ENABLE_PSRAM_LOGGING true
#define ENABLE_HEAP_MONITORING true
#define ENABLE_TASK_MONITORING false  // Not implemented yet

// ===================================================================
// COMPATIBILITY SETTINGS
// ===================================================================

// Backward compatibility
#define MAINTAIN_SERIAL_COMPATIBILITY true
#define PROVIDE_LEGACY_MACROS true

// Arduino IDE compatibility
#define ARDUINO_IDE_COMPATIBLE true
#define PLATFORMIO_COMPATIBLE true

// ===================================================================
// API COMMUNICATION SETTINGS
// ===================================================================

// API response formatting
#define API_RESPONSE_JSON_FORMAT true
#define API_RESPONSE_PRETTY_PRINT false
#define API_MAX_RESPONSE_SIZE 2048

// Command processing
#define API_COMMAND_TIMEOUT_MS 5000
#define API_MAX_COMMAND_LENGTH 256
#define API_COMMAND_BUFFER_SIZE 512

// Status reporting
#define API_STATUS_REPORT_INTERVAL_MS 10000
#define API_HEARTBEAT_INTERVAL_MS 30000
#define API_ERROR_RETRY_COUNT 3

// ===================================================================
// DEVICE LOGGING SETTINGS
// ===================================================================

// Device state logging
#define DEVICE_LOG_STATE_CHANGES true
#define DEVICE_LOG_INITIALIZATION true
#define DEVICE_LOG_ERRORS true
#define DEVICE_LOG_WARNINGS true

// Sensor data logging
#define SENSOR_LOG_READINGS false  // Can be very verbose
#define SENSOR_LOG_CALIBRATION true
#define SENSOR_LOG_ERRORS true

// Communication logging
#define COMM_LOG_CONNECTIONS true
#define COMM_LOG_DATA_TRANSFER false  // Can be very verbose
#define COMM_LOG_PROTOCOL_ERRORS true

#endif // SERIAL_CONFIG_H 