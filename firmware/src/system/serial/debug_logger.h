#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#include <Arduino.h>
#include "serial_manager.h"

// Debug categories for filtering
#define DEBUG_NONE       0x00
#define DEBUG_SYSTEM     0x01
#define DEBUG_BLE        0x02
#define DEBUG_CAMERA     0x04
#define DEBUG_MICROPHONE 0x08
#define DEBUG_BATTERY    0x10
#define DEBUG_POWER      0x20
#define DEBUG_MEMORY     0x40
#define DEBUG_TIMING     0x80
#define DEBUG_ALL        0xFF

// Performance monitoring constants
#define MAX_TIMING_ENTRIES 20
#define MEMORY_SAMPLE_INTERVAL_MS 5000
#define PERFORMANCE_REPORT_INTERVAL_MS 30000

// Timing entry structure
struct TimingEntry {
    const char* operation;
    unsigned long start_time;
    unsigned long duration;
    bool active;
};

// Memory sample structure
struct MemorySample {
    unsigned long timestamp;
    size_t free_heap;
    size_t free_psram;
    size_t largest_free_block;
};

class DebugLogger {
private:
    static uint8_t debug_categories;
    static bool timing_enabled;
    static bool memory_tracking_enabled;
    static bool performance_monitoring_enabled;
    
    // Timing tracking
    static TimingEntry timing_entries[MAX_TIMING_ENTRIES];
    static uint8_t timing_entry_count;
    static unsigned long total_timing_operations;
    
    // Memory tracking
    static MemorySample memory_samples[10];
    static uint8_t memory_sample_count;
    static unsigned long last_memory_sample;
    
    // Performance monitoring
    static unsigned long last_performance_report;
    static unsigned long total_debug_messages;
    static unsigned long total_debug_bytes;
    
    // Helper methods
    static int findTimingEntry(const char* operation);
    static void addMemorySample();
    static void reportPerformanceMetrics();
    
public:
    // ===================================================================
    // INITIALIZATION AND CONFIGURATION
    // ===================================================================
    
    static void initialize(uint8_t categories = DEBUG_ALL);
    static void cleanup();
    
    static void setCategories(uint8_t categories);
    static uint8_t getCategories();
    
    static void enableTiming(bool enabled);
    static void enableMemoryTracking(bool enabled);
    static void enablePerformanceMonitoring(bool enabled);
    
    // ===================================================================
    // CATEGORY-BASED LOGGING
    // ===================================================================
    
    static void logSystem(const char* message);
    static void logBLE(const char* message);
    static void logCamera(const char* message);
    static void logMicrophone(const char* message);
    static void logBattery(const char* message);
    static void logPower(const char* message);
    static void logMemory(const char* message);
    static void logTiming(const char* message);
    
    // ===================================================================
    // FORMATTED CATEGORY LOGGING
    // ===================================================================
    
    static void logSystemf(const char* format, ...);
    static void logBLEf(const char* format, ...);
    static void logCameraf(const char* format, ...);
    static void logMicrophonef(const char* format, ...);
    static void logBatteryf(const char* format, ...);
    static void logPowerf(const char* format, ...);
    static void logMemoryf(const char* format, ...);
    static void logTimingf(const char* format, ...);
    
    // ===================================================================
    // TIMING AND PERFORMANCE MONITORING
    // ===================================================================
    
    static unsigned long startTiming(const char* operation);
    static void endTiming(unsigned long start_time, const char* operation);
    static void logTimingResult(const char* operation, unsigned long duration);
    
    static void measureOperation(const char* operation, void (*func)());
    static void measureOperationWithResult(const char* operation, bool (*func)(), bool& result);
    
    // ===================================================================
    // MEMORY TRACKING
    // ===================================================================
    
    static void logMemoryUsage(const char* context = nullptr);
    static void logMemoryLeak(const char* operation, size_t bytes_before, size_t bytes_after);
    static void logMemoryFragmentation();
    
    static void trackMemoryAllocation(const char* operation, size_t bytes);
    static void trackMemoryDeallocation(const char* operation, size_t bytes);
    
    // ===================================================================
    // SYSTEM INFORMATION DUMPS
    // ===================================================================
    
    static void dumpSystemInfo();
    static void dumpMemoryInfo();
    static void dumpTimingInfo();
    static void dumpPerformanceInfo();
    static void dumpAllInfo();
    
    // ===================================================================
    // CONDITIONAL DEBUG MACROS
    // ===================================================================
    
    static bool isEnabled(uint8_t category);
    static void conditionalLog(uint8_t category, const char* message);
    static void conditionalLogf(uint8_t category, const char* format, ...);
    
    // ===================================================================
    // ADVANCED DEBUGGING FEATURES
    // ===================================================================
    
    static void logStackUsage();
    static void logTaskInfo();
    static void logInterruptInfo();
    static void logHeapFragmentation();
    
    // ===================================================================
    // PERIODIC MONITORING
    // ===================================================================
    
    static void update(); // Call from main loop
    static void periodicMemoryCheck();
    static void periodicPerformanceReport();
    
    // ===================================================================
    // UTILITY METHODS
    // ===================================================================
    
    static void hexDump(const char* label, const uint8_t* data, size_t length);
    static void binaryDump(const char* label, const uint8_t* data, size_t length);
    static void asciiDump(const char* label, const uint8_t* data, size_t length);
};

// ===================================================================
// CONVENIENCE MACROS FOR CONDITIONAL DEBUGGING
// ===================================================================

#define DEBUG_SYSTEM_LOG(msg) DebugLogger::conditionalLog(DEBUG_SYSTEM, msg)
#define DEBUG_BLE_LOG(msg) DebugLogger::conditionalLog(DEBUG_BLE, msg)
#define DEBUG_CAMERA_LOG(msg) DebugLogger::conditionalLog(DEBUG_CAMERA, msg)
#define DEBUG_MICROPHONE_LOG(msg) DebugLogger::conditionalLog(DEBUG_MICROPHONE, msg)
#define DEBUG_BATTERY_LOG(msg) DebugLogger::conditionalLog(DEBUG_BATTERY, msg)
#define DEBUG_POWER_LOG(msg) DebugLogger::conditionalLog(DEBUG_POWER, msg)
#define DEBUG_MEMORY_LOG(msg) DebugLogger::conditionalLog(DEBUG_MEMORY, msg)
#define DEBUG_TIMING_LOG(msg) DebugLogger::conditionalLog(DEBUG_TIMING, msg)

#define DEBUG_SYSTEM_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_SYSTEM, fmt, __VA_ARGS__)
#define DEBUG_BLE_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_BLE, fmt, __VA_ARGS__)
#define DEBUG_CAMERA_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_CAMERA, fmt, __VA_ARGS__)
#define DEBUG_MICROPHONE_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_MICROPHONE, fmt, __VA_ARGS__)
#define DEBUG_BATTERY_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_BATTERY, fmt, __VA_ARGS__)
#define DEBUG_POWER_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_POWER, fmt, __VA_ARGS__)
#define DEBUG_MEMORY_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_MEMORY, fmt, __VA_ARGS__)
#define DEBUG_TIMING_LOGF(fmt, ...) DebugLogger::conditionalLogf(DEBUG_TIMING, fmt, __VA_ARGS__)

// ===================================================================
// TIMING MEASUREMENT MACROS
// ===================================================================

#define DEBUG_TIME_START(operation) unsigned long __debug_start_##operation = DebugLogger::startTiming(#operation)
#define DEBUG_TIME_END(operation) DebugLogger::endTiming(__debug_start_##operation, #operation)

#define DEBUG_TIME_BLOCK(operation, block) \
    do { \
        unsigned long __start = DebugLogger::startTiming(operation); \
        block; \
        DebugLogger::endTiming(__start, operation); \
    } while(0)

// ===================================================================
// MEMORY TRACKING MACROS
// ===================================================================

#define DEBUG_MEMORY_CHECKPOINT(label) DebugLogger::logMemoryUsage(label)
#define DEBUG_MEMORY_ALLOC(operation, bytes) DebugLogger::trackMemoryAllocation(operation, bytes)
#define DEBUG_MEMORY_FREE(operation, bytes) DebugLogger::trackMemoryDeallocation(operation, bytes)

#endif // DEBUG_LOGGER_H 