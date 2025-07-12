#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>

// Serial configuration constants
#define SERIAL_BUFFER_SIZE 256
#define SERIAL_TIMEOUT_MS 1000

// Log levels
enum LogLevel {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_VERBOSE = 5
};

// Module name constants
#define MODULE_MAIN "MAIN"
#define MODULE_BLE "BLE"
#define MODULE_CAMERA "CAMERA"
#define MODULE_MICROPHONE "MICROPHONE"
#define MODULE_BATTERY "BATTERY"
#define MODULE_POWER "POWER"
#define MODULE_MEMORY "MEMORY"
#define MODULE_LED "LED"
#define MODULE_CYCLES "CYCLES"
#define MODULE_HOTSPOT "HOTSPOT"
#define MODULE_AUDIO "AUDIO"
#define MODULE_SYSTEM "SYSTEM"

// Serial manager class
class SerialManager {
private:
    static bool initialized;
    static LogLevel current_log_level;
    static unsigned long last_performance_report;
    static unsigned long total_bytes_sent;
    static unsigned long message_count;
    
    static void printLogPrefix(LogLevel level, const char* module = nullptr);
    static const char* getLogLevelString(LogLevel level);
    
public:
    // ===================================================================
    // INITIALIZATION AND CONFIGURATION
    // ===================================================================
    
    static bool initialize(unsigned long baud_rate = 921600);
    static void cleanup();
    
    static void setLogLevel(LogLevel level);
    static LogLevel getLogLevel();
    static bool isInitialized();
    
    // ===================================================================
    // BASIC LOGGING METHODS
    // ===================================================================
    
    static void log(LogLevel level, const char* message, const char* module = nullptr);
    static void log(LogLevel level, const String& message, const char* module = nullptr);
    static void logf(LogLevel level, const char* module, const char* format, ...);
    
    // ===================================================================
    // CONVENIENCE METHODS FOR DIFFERENT LOG LEVELS
    // ===================================================================
    
    static void error(const char* message, const char* module = nullptr);
    static void warning(const char* message, const char* module = nullptr);
    static void info(const char* message, const char* module = nullptr);
    static void debug(const char* message, const char* module = nullptr);
    static void verbose(const char* message, const char* module = nullptr);
    
    // ===================================================================
    // FORMATTED LOGGING METHODS
    // ===================================================================
    
    static void errorf(const char* module, const char* format, ...);
    static void warningf(const char* module, const char* format, ...);
    static void infof(const char* module, const char* format, ...);
    static void debugf(const char* module, const char* format, ...);
    static void verbosef(const char* module, const char* format, ...);
    
    // ===================================================================
    // PERFORMANCE MONITORING
    // ===================================================================
    
    static void reportPerformance();
    static void logPerformanceMetrics(const char* operation, unsigned long duration_ms);
    
    // ===================================================================
    // DATA LOGGING
    // ===================================================================
    
    static void logHexData(const char* label, const uint8_t* data, size_t length, const char* module = nullptr);
    static void logAudioData(size_t bytes_read, const char* module = "AUDIO");
    static void logBatteryData(float voltage, uint8_t level, bool charging, const char* module = "BATTERY");
    static void logMemoryData(size_t free_heap, size_t free_psram, const char* module = "MEMORY");
    
    // ===================================================================
    // SYSTEM STATUS LOGGING
    // ===================================================================
    
    static void logSystemStatus(const char* component, const char* status, const char* module = nullptr);
    static void logInitialization(const char* component, bool success, const char* module = nullptr);
    static void logError(const char* component, const char* error_msg, const char* module = nullptr);
    
    // ===================================================================
    // BLE SPECIFIC LOGGING
    // ===================================================================
    
    static void logBLEEvent(const char* event, const char* details = nullptr);
    static void logBLEData(const char* characteristic, size_t bytes, const char* direction = "TX");
    static void logBLEConnection(bool connected, const char* device_name = nullptr);
    
    // ===================================================================
    // CAMERA SPECIFIC LOGGING
    // ===================================================================
    
    static void logCameraEvent(const char* event, const char* details = nullptr);
    static void logPhotoCapture(size_t photo_size, const char* quality = nullptr);
    static void logVideoStream(uint8_t fps, uint32_t frame_count);
    
    // ===================================================================
    // MICROPHONE SPECIFIC LOGGING
    // ===================================================================
    
    static void logMicrophoneEvent(const char* event, const char* details = nullptr);
    static void logAudioProcessing(size_t input_bytes, size_t output_bytes, const char* codec = nullptr);
    
    // ===================================================================
    // POWER MANAGEMENT LOGGING
    // ===================================================================
    
    static void logPowerEvent(const char* event, const char* details = nullptr);
    static void logPowerMode(const char* mode, uint32_t cpu_freq_mhz);
    
    // ===================================================================
    // FORMATTING AND DISPLAY
    // ===================================================================
    
    static void printSeparator(char character = '=', int length = 60);
    static void printHeader(const char* title);
    static void printSubheader(const char* subtitle);
    
    // ===================================================================
    // RAW SERIAL ACCESS (FOR COMPATIBILITY)
    // ===================================================================
    
    static void print(const char* message);
    static void println(const char* message);
    static void printf(const char* format, ...);
};

// ===================================================================
// CONVENIENCE MACROS FOR MODULE-SPECIFIC LOGGING
// ===================================================================

#define SERIAL_LOG_ERROR(module, msg) SerialManager::error(msg, module)
#define SERIAL_LOG_WARNING(module, msg) SerialManager::warning(msg, module)
#define SERIAL_LOG_INFO(module, msg) SerialManager::info(msg, module)
#define SERIAL_LOG_DEBUG(module, msg) SerialManager::debug(msg, module)
#define SERIAL_LOG_VERBOSE(module, msg) SerialManager::verbose(msg, module)

#define SERIAL_LOGF_ERROR(module, fmt, ...) SerialManager::errorf(module, fmt, __VA_ARGS__)
#define SERIAL_LOGF_WARNING(module, fmt, ...) SerialManager::warningf(module, fmt, __VA_ARGS__)
#define SERIAL_LOGF_INFO(module, fmt, ...) SerialManager::infof(module, fmt, __VA_ARGS__)
#define SERIAL_LOGF_DEBUG(module, fmt, ...) SerialManager::debugf(module, fmt, __VA_ARGS__)
#define SERIAL_LOGF_VERBOSE(module, fmt, ...) SerialManager::verbosef(module, fmt, __VA_ARGS__)

#endif // SERIAL_MANAGER_H 