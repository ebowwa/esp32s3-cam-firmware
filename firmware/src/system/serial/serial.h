#ifndef SERIAL_H
#define SERIAL_H

// Include all serial management components
#include "serial_config.h"
#include "serial_manager.h"
#include "debug_logger.h"

// ===================================================================
// UNIFIED SERIAL INTERFACE
// ===================================================================

class SerialSystem {
public:
    // ===================================================================
    // INITIALIZATION AND CLEANUP
    // ===================================================================
    
    static bool initialize(unsigned long baud_rate = SERIAL_DEFAULT_BAUD_RATE, 
                          LogLevel log_level = DEFAULT_LOG_LEVEL,
                          uint8_t debug_categories = DEFAULT_DEBUG_CATEGORIES);
    
    static void cleanup();
    static bool isReady();
    
    // ===================================================================
    // CONFIGURATION MANAGEMENT
    // ===================================================================
    
    // Serial Manager configuration
    static void setLogLevel(LogLevel level) { SerialManager::setLogLevel(level); }
    static LogLevel getLogLevel() { return SerialManager::getLogLevel(); }
    
    // Debug Logger configuration
    static void setDebugCategories(uint8_t categories) { DebugLogger::setCategories(categories); }
    static uint8_t getDebugCategories() { return DebugLogger::getCategories(); }
    
    static void enableTiming(bool enabled) { DebugLogger::enableTiming(enabled); }
    static void enableMemoryTracking(bool enabled) { DebugLogger::enableMemoryTracking(enabled); }
    static void enablePerformanceMonitoring(bool enabled) { DebugLogger::enablePerformanceMonitoring(enabled); }
    
    // ===================================================================
    // QUICK ACCESS LOGGING METHODS
    // ===================================================================
    
    // Basic logging
    static void error(const char* message, const char* module = nullptr) { 
        SerialManager::error(message, module); 
    }
    static void warning(const char* message, const char* module = nullptr) { 
        SerialManager::warning(message, module); 
    }
    static void info(const char* message, const char* module = nullptr) { 
        SerialManager::info(message, module); 
    }
    static void debug(const char* message, const char* module = nullptr) { 
        SerialManager::debug(message, module); 
    }
    
    // Formatted logging
    static void errorf(const char* module, const char* format, ...) {
        va_list args;
        va_start(args, format);
        SerialManager::errorf(module, format, args);
        va_end(args);
    }
    
    static void warningf(const char* module, const char* format, ...) {
        va_list args;
        va_start(args, format);
        SerialManager::warningf(module, format, args);
        va_end(args);
    }
    
    static void infof(const char* module, const char* format, ...) {
        va_list args;
        va_start(args, format);
        SerialManager::infof(module, format, args);
        va_end(args);
    }
    
    static void debugf(const char* module, const char* format, ...) {
        va_list args;
        va_start(args, format);
        SerialManager::debugf(module, format, args);
        va_end(args);
    }
    
    // ===================================================================
    // SPECIALIZED LOGGING METHODS
    // ===================================================================
    
    // System status logging
    static void logInitialization(const char* component, bool success, const char* module = nullptr) {
        SerialManager::logInitialization(component, success, module);
    }
    
    static void logError(const char* component, const char* error_msg, const char* module = nullptr) {
        SerialManager::logError(component, error_msg, module);
    }
    
    // BLE logging
    static void logBLEEvent(const char* event, const char* details = nullptr) {
        SerialManager::logBLEEvent(event, details);
    }
    
    static void logBLEConnection(bool connected, const char* device_name = nullptr) {
        SerialManager::logBLEConnection(connected, device_name);
    }
    
    // Camera logging
    static void logCameraEvent(const char* event, const char* details = nullptr) {
        SerialManager::logCameraEvent(event, details);
    }
    
    static void logPhotoCapture(size_t photo_size, const char* quality = nullptr) {
        SerialManager::logPhotoCapture(photo_size, quality);
    }
    
    // Microphone logging
    static void logMicrophoneEvent(const char* event, const char* details = nullptr) {
        SerialManager::logMicrophoneEvent(event, details);
    }
    
    static void logAudioProcessing(size_t input_bytes, size_t output_bytes, const char* codec = nullptr) {
        SerialManager::logAudioProcessing(input_bytes, output_bytes, codec);
    }
    
    // Battery logging
    static void logBatteryData(float voltage, uint8_t level, bool charging, const char* module = "BATTERY") {
        SerialManager::logBatteryData(voltage, level, charging, module);
    }
    
    // Memory logging
    static void logMemoryData(size_t free_heap, size_t free_psram, const char* module = "MEMORY") {
        SerialManager::logMemoryData(free_heap, free_psram, module);
    }
    
    // Power management logging
    static void logPowerEvent(const char* event, const char* details = nullptr) {
        SerialManager::logPowerEvent(event, details);
    }
    
    static void logPowerMode(const char* mode, uint32_t cpu_freq_mhz) {
        SerialManager::logPowerMode(mode, cpu_freq_mhz);
    }
    
    // ===================================================================
    // PERFORMANCE AND MONITORING
    // ===================================================================
    
    static void reportPerformance() { SerialManager::reportPerformance(); }
    static void logPerformanceMetrics(const char* operation, unsigned long duration_ms) {
        SerialManager::logPerformanceMetrics(operation, duration_ms);
    }
    
    // Timing measurements
    static unsigned long startTiming(const char* operation) {
        return DebugLogger::startTiming(operation);
    }
    
    static void endTiming(unsigned long start_time, const char* operation) {
        DebugLogger::endTiming(start_time, operation);
    }
    
    // Memory tracking
    static void logMemoryUsage(const char* context = nullptr) {
        DebugLogger::logMemoryUsage(context);
    }
    
    // ===================================================================
    // SYSTEM INFORMATION AND DIAGNOSTICS
    // ===================================================================
    
    static void dumpSystemInfo() { DebugLogger::dumpSystemInfo(); }
    static void dumpMemoryInfo() { DebugLogger::dumpMemoryInfo(); }
    static void dumpTimingInfo() { DebugLogger::dumpTimingInfo(); }
    static void dumpPerformanceInfo() { DebugLogger::dumpPerformanceInfo(); }
    static void dumpAllInfo() { DebugLogger::dumpAllInfo(); }
    
    // ===================================================================
    // FORMATTING AND DISPLAY
    // ===================================================================
    
    static void printSeparator(char character = '=', int length = 60) {
        SerialManager::printSeparator(character, length);
    }
    
    static void printHeader(const char* title) {
        SerialManager::printHeader(title);
    }
    
    static void printSubheader(const char* subtitle) {
        SerialManager::printSubheader(subtitle);
    }
    
    // ===================================================================
    // DATA DUMPS AND UTILITIES
    // ===================================================================
    
    static void hexDump(const char* label, const uint8_t* data, size_t length) {
        DebugLogger::hexDump(label, data, length);
    }
    
    static void binaryDump(const char* label, const uint8_t* data, size_t length) {
        DebugLogger::binaryDump(label, data, length);
    }
    
    static void asciiDump(const char* label, const uint8_t* data, size_t length) {
        DebugLogger::asciiDump(label, data, length);
    }
    
    // ===================================================================
    // PERIODIC UPDATES
    // ===================================================================
    
    static void update() { DebugLogger::update(); }
    
    // ===================================================================
    // RAW SERIAL ACCESS (FOR COMPATIBILITY)
    // ===================================================================
    
    static void print(const char* message) { SerialManager::print(message); }
    static void println(const char* message) { SerialManager::println(message); }
    static void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        SerialManager::printf(format, args);
        va_end(args);
    }
};

// ===================================================================
// CONVENIENCE MACROS FOR BACKWARD COMPATIBILITY
// ===================================================================

#if PROVIDE_LEGACY_MACROS

// Legacy Serial.print compatibility
#define SERIAL_PRINT(msg) SerialSystem::print(msg)
#define SERIAL_PRINTLN(msg) SerialSystem::println(msg)
#define SERIAL_PRINTF(fmt, ...) SerialSystem::printf(fmt, __VA_ARGS__)

// Legacy logging macros
#define LOG_ERROR(msg) SerialSystem::error(msg)
#define LOG_WARNING(msg) SerialSystem::warning(msg)
#define LOG_INFO(msg) SerialSystem::info(msg)
#define LOG_DEBUG(msg) SerialSystem::debug(msg)

#define LOG_ERRORF(fmt, ...) SerialSystem::errorf(MODULE_SYSTEM, fmt, __VA_ARGS__)
#define LOG_WARNINGF(fmt, ...) SerialSystem::warningf(MODULE_SYSTEM, fmt, __VA_ARGS__)
#define LOG_INFOF(fmt, ...) SerialSystem::infof(MODULE_SYSTEM, fmt, __VA_ARGS__)
#define LOG_DEBUGF(fmt, ...) SerialSystem::debugf(MODULE_SYSTEM, fmt, __VA_ARGS__)

// Module-specific logging shortcuts
#define BLE_LOG_INFO(msg) SerialSystem::info(msg, MODULE_BLE)
#define BLE_LOG_ERROR(msg) SerialSystem::error(msg, MODULE_BLE)
#define CAMERA_LOG_INFO(msg) SerialSystem::info(msg, MODULE_CAMERA)
#define CAMERA_LOG_ERROR(msg) SerialSystem::error(msg, MODULE_CAMERA)
#define MICROPHONE_LOG_INFO(msg) SerialSystem::info(msg, MODULE_MICROPHONE)
#define MICROPHONE_LOG_ERROR(msg) SerialSystem::error(msg, MODULE_MICROPHONE)
#define BATTERY_LOG_INFO(msg) SerialSystem::info(msg, MODULE_BATTERY)
#define BATTERY_LOG_ERROR(msg) SerialSystem::error(msg, MODULE_BATTERY)

#endif // PROVIDE_LEGACY_MACROS

// ===================================================================
// RECOMMENDED USAGE PATTERNS
// ===================================================================

/*
RECOMMENDED USAGE:

1. In main firmware.ino:
   ```cpp
   #include "src/system/serial/serial.h"
   
   void setup() {
       SerialSystem::initialize();
       SerialSystem::info("System starting up", MODULE_MAIN);
   }
   
   void loop() {
       SerialSystem::update(); // Call periodically for monitoring
   }
   ```

2. In feature modules:
   ```cpp
   #include "../../system/serial/serial.h"
   
   void someFunction() {
       SerialSystem::logBLEEvent("Connection established");
       SerialSystem::logMemoryUsage("after_initialization");
   }
   ```

3. For performance monitoring:
   ```cpp
   unsigned long start = SerialSystem::startTiming("operation");
   // ... do work ...
   SerialSystem::endTiming(start, "operation");
   ```

4. For conditional debug logging:
   ```cpp
   DEBUG_BLE_LOGF("Connection state: %s", connected ? "ON" : "OFF");
   ```

5. For structured logging:
   ```cpp
   SerialSystem::logBatteryData(voltage, level, charging);
   SerialSystem::logMemoryData(free_heap, free_psram);
   ```

6. For system diagnostics:
   ```cpp
   SerialSystem::dumpAllInfo(); // Comprehensive system dump
   SerialSystem::hexDump("sensor_data", data, length);
   ```
*/

#endif // SERIAL_H 