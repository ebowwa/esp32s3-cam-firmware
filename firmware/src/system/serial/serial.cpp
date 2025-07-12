#include "serial.h"
#include <stdarg.h>

// ===================================================================
// UNIFIED SERIAL INTERFACE IMPLEMENTATION
// ===================================================================

bool SerialSystem::initialize(unsigned long baud_rate, LogLevel log_level, uint8_t debug_categories) {
    // Initialize SerialManager first
    if (!SerialManager::initialize(baud_rate)) {
        return false;
    }
    
    // Set log level
    SerialManager::setLogLevel(log_level);
    
    // Initialize DebugLogger
    DebugLogger::initialize(debug_categories);
    
    // Configure debug features based on build type
    #ifdef DEBUG
        DebugLogger::enableTiming(true);
        DebugLogger::enableMemoryTracking(true);
        DebugLogger::enablePerformanceMonitoring(true);
    #else
        DebugLogger::enableTiming(false);
        DebugLogger::enableMemoryTracking(false);
        DebugLogger::enablePerformanceMonitoring(false);
    #endif
    
    // Log successful initialization
    SerialManager::printHeader("Serial System Initialized");
    SerialManager::infof(MODULE_SYSTEM, "Baud rate: %lu", baud_rate);
    SerialManager::infof(MODULE_SYSTEM, "Log level: %d", log_level);
    SerialManager::infof(MODULE_SYSTEM, "Debug categories: 0x%02X", debug_categories);
    
    #ifdef DEBUG
        SerialManager::info("Debug build - full logging enabled", MODULE_SYSTEM);
    #else
        SerialManager::info("Release build - optimized logging", MODULE_SYSTEM);
    #endif
    
    SerialManager::printSeparator();
    
    return true;
}

void SerialSystem::cleanup() {
    SerialManager::info("Serial System shutting down", MODULE_SYSTEM);
    DebugLogger::cleanup();
    SerialManager::cleanup();
}

bool SerialSystem::isReady() {
    return SerialManager::isInitialized();
} 