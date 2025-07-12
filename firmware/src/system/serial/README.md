# Serial System Documentation

The ESP32S3 Camera Firmware uses a comprehensive serial communication system that handles general serial communication, structured device logging, and advanced debugging features.

## Overview

The serial system is organized into several components:

- **SerialManager**: Core serial communication and logging functionality
- **DebugLogger**: Advanced debugging features with timing, memory tracking, and performance monitoring
- **SerialSystem**: Unified interface that combines all serial features
- **Configuration**: Centralized configuration for all serial features

## Quick Start

### Basic Usage

```cpp
#include "src/system/serial/serial.h"

void setup() {
    // Initialize the serial system
    SerialSystem::initialize();
    
    // Basic logging
    SerialSystem::info("System starting up", MODULE_MAIN);
    SerialSystem::error("Something went wrong", MODULE_CAMERA);
    
    // Formatted logging
    SerialSystem::infof(MODULE_BATTERY, "Battery level: %d%%", 85);
}

void loop() {
    // Update for periodic monitoring
    SerialSystem::update();
}
```

### Module-Specific Logging

```cpp
// Use predefined module constants
SerialSystem::info("BLE connection established", MODULE_BLE);
SerialSystem::logCameraEvent("Photo captured", "1920x1080");
SerialSystem::logBatteryData(3.7f, 85, true);
SerialSystem::logMemoryUsage("after_initialization");
```

## Core Features

### 1. Structured Logging

The system provides multiple log levels:
- **ERROR**: Critical errors that affect functionality
- **WARNING**: Non-critical issues that should be noted
- **INFO**: General information messages
- **DEBUG**: Detailed debugging information
- **VERBOSE**: Extremely detailed trace information

```cpp
// Set log level (only messages at or below this level will be shown)
SerialSystem::setLogLevel(LOG_LEVEL_INFO);

// Log at different levels
SerialSystem::error("Critical error occurred", MODULE_SYSTEM);
SerialSystem::warning("Non-critical issue", MODULE_BATTERY);
SerialSystem::info("General information", MODULE_MAIN);
SerialSystem::debug("Debugging info", MODULE_BLE);
SerialSystem::verbose("Trace information", MODULE_CAMERA);
```

### 2. Module-Based Organization

All log messages are tagged with modules for easy filtering:

```cpp
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
```

### 3. Specialized Logging Methods

#### System Status Logging
```cpp
SerialSystem::logInitialization("Camera", true, MODULE_CAMERA);
SerialSystem::logError("BLE", "Connection failed", MODULE_BLE);
```

#### Hardware-Specific Logging
```cpp
// BLE logging
SerialSystem::logBLEEvent("Connection established");
SerialSystem::logBLEConnection(true, "iPhone");

// Camera logging
SerialSystem::logCameraEvent("Photo captured");
SerialSystem::logPhotoCapture(1024000, "high");

// Battery logging
SerialSystem::logBatteryData(3.7f, 85, true);

// Memory logging
SerialSystem::logMemoryData(free_heap, free_psram);
```

## Advanced Features

### 1. Debug Categories

Enable/disable debug output for specific categories:

```cpp
// Enable debug output for specific categories
SerialSystem::setDebugCategories(DEBUG_BLE | DEBUG_CAMERA | DEBUG_MEMORY);

// Use conditional debug logging
DEBUG_BLE_LOGF("Connection state: %s", connected ? "ON" : "OFF");
DEBUG_CAMERA_LOG("Frame captured");
DEBUG_MEMORY_CHECKPOINT("after_allocation");
```

### 2. Performance Monitoring

#### Timing Measurements
```cpp
// Manual timing
unsigned long start = SerialSystem::startTiming("photo_capture");
// ... do work ...
SerialSystem::endTiming(start, "photo_capture");

// Macro-based timing
DEBUG_TIME_START(operation);
// ... do work ...
DEBUG_TIME_END(operation);

// Block timing
DEBUG_TIME_BLOCK("complex_operation", {
    // ... complex code ...
});
```

#### Memory Tracking
```cpp
// Enable memory tracking
SerialSystem::enableMemoryTracking(true);

// Log memory usage at checkpoints
SerialSystem::logMemoryUsage("initialization");
SerialSystem::logMemoryUsage("after_camera_init");

// Track allocations
DEBUG_MEMORY_ALLOC("camera_buffer", 1024000);
DEBUG_MEMORY_FREE("camera_buffer", 1024000);
```

### 3. System Diagnostics

#### Comprehensive System Information
```cpp
SerialSystem::dumpSystemInfo();    // CPU, memory, chip info
SerialSystem::dumpMemoryInfo();    // Detailed memory statistics
SerialSystem::dumpTimingInfo();    // Performance timing data
SerialSystem::dumpAllInfo();       // Everything
```

#### Data Dumps
```cpp
uint8_t data[256];
SerialSystem::hexDump("sensor_data", data, 256);
SerialSystem::binaryDump("flags", data, 32);
SerialSystem::asciiDump("text_data", data, 128);
```

## Configuration

### Build-Time Configuration

The system automatically configures itself based on build type:

```cpp
#ifdef DEBUG
    // Debug builds enable all features
    - Full debug logging
    - Timing measurements
    - Memory tracking
    - Performance monitoring
#else
    // Release builds are optimized
    - Essential logging only
    - Minimal overhead
    - Performance optimized
#endif
```

### Runtime Configuration

```cpp
// Configure log levels
SerialSystem::setLogLevel(LOG_LEVEL_DEBUG);

// Configure debug categories
SerialSystem::setDebugCategories(DEBUG_BLE | DEBUG_CAMERA);

// Enable/disable features
SerialSystem::enableTiming(true);
SerialSystem::enableMemoryTracking(true);
SerialSystem::enablePerformanceMonitoring(true);
```

## Message Format

All log messages follow a consistent format:

```
[timestamp] [level] [module] message
```

Example:
```
[    12345] [INFO] [CAMERA] Photo captured: 1024000 bytes
[    12350] [ERROR] [BLE] Connection failed: timeout
[    12355] [DEBUG] [MEMORY] Memory: Heap=512 KB, PSRAM=1024 KB
```

## Performance Considerations

### Logging Overhead
- **INFO/ERROR**: Minimal overhead, always enabled
- **DEBUG**: Low overhead, disabled in release builds
- **VERBOSE**: Higher overhead, use sparingly

### Memory Usage
- Serial buffers: ~2KB
- Timing tracking: ~1KB
- Memory samples: ~400 bytes
- Total overhead: ~4KB

### Best Practices

1. **Use appropriate log levels**
   ```cpp
   SerialSystem::error("Critical errors only");
   SerialSystem::info("Important status updates");
   SerialSystem::debug("Development debugging");
   ```

2. **Use conditional debugging for verbose output**
   ```cpp
   DEBUG_BLE_LOGF("Detailed BLE state: %d", state);
   ```

3. **Enable timing only when needed**
   ```cpp
   #ifdef MEASURE_PERFORMANCE
   SerialSystem::enableTiming(true);
   #endif
   ```

4. **Use memory tracking for leak detection**
   ```cpp
   #ifdef DEBUG_MEMORY_LEAKS
   SerialSystem::enableMemoryTracking(true);
   #endif
   ```

## Integration Examples

### Main Firmware Integration
```cpp
#include "src/system/serial/serial.h"

void setup() {
    SerialSystem::initialize();
    SerialSystem::info("OpenGlass starting up", MODULE_MAIN);
    
    // Initialize other systems with logging
    if (initializeCamera()) {
        SerialSystem::logInitialization("Camera", true, MODULE_CAMERA);
    } else {
        SerialSystem::logInitialization("Camera", false, MODULE_CAMERA);
    }
}

void loop() {
    SerialSystem::update(); // Periodic monitoring
    
    // Performance monitoring
    unsigned long start = SerialSystem::startTiming("main_loop");
    
    // ... main loop work ...
    
    SerialSystem::endTiming(start, "main_loop");
}
```

### Feature Module Integration
```cpp
#include "../../system/serial/serial.h"

class CameraManager {
public:
    bool initialize() {
        SerialSystem::info("Initializing camera...", MODULE_CAMERA);
        
        DEBUG_TIME_START(camera_init);
        bool success = setupCamera();
        DEBUG_TIME_END(camera_init);
        
        if (success) {
            SerialSystem::logCameraEvent("Initialization successful");
        } else {
            SerialSystem::logError("Camera", "Initialization failed", MODULE_CAMERA);
        }
        
        return success;
    }
    
    bool capturePhoto() {
        DEBUG_MEMORY_CHECKPOINT("before_capture");
        
        size_t photo_size = performCapture();
        if (photo_size > 0) {
            SerialSystem::logPhotoCapture(photo_size, "high");
            return true;
        }
        
        SerialSystem::logError("Camera", "Photo capture failed", MODULE_CAMERA);
        return false;
    }
};
```

## Troubleshooting

### Common Issues

1. **No serial output**
   - Check baud rate (default: 921600)
   - Ensure SerialSystem::initialize() is called
   - Verify log level settings

2. **Missing debug output**
   - Check if debug categories are enabled
   - Verify build configuration (DEBUG vs RELEASE)
   - Ensure debug level is appropriate

3. **Performance issues**
   - Reduce log level in production
   - Disable verbose logging
   - Use conditional debug macros

### Debug Commands

```cpp
// Check system status
SerialSystem::dumpSystemInfo();

// Check memory status
SerialSystem::dumpMemoryInfo();

// Check performance
SerialSystem::dumpPerformanceInfo();

// Check configuration
SerialSystem::infof(MODULE_SYSTEM, "Log level: %d", SerialSystem::getLogLevel());
SerialSystem::infof(MODULE_SYSTEM, "Debug categories: 0x%02X", SerialSystem::getDebugCategories());
```

## API Reference

See the header files for complete API documentation:
- `serial.h` - Main interface
- `serial_manager.h` - Core logging functionality
- `debug_logger.h` - Advanced debugging features
- `serial_config.h` - Configuration options 