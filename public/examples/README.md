# OpenGlass Firmware Examples

This directory contains example sketches and verification tools for the OpenGlass firmware.

## Available Examples

### `verify_constants.ino`
**Purpose**: Verify that all XIAO ESP32-S3 constants are properly defined and accessible.

**Features**:
- Tests chip information (model, architecture, cores, frequency)
- Verifies memory configuration (Flash, PSRAM, SRAM)
- Checks GPIO pin definitions
- Validates camera pin mappings
- Tests ADC configuration
- Displays power specifications
- Shows Sense version features

**Usage**:
1. Open `verify_constants.ino` in Arduino IDE
2. Select the XIAO ESP32-S3 board
3. Compile and upload
4. Open Serial Monitor (921600 baud)
5. View the verification results

### `test_compilation.ino`
**Purpose**: Test that all functions and variables are properly linked in the organized firmware structure.

**Features**:
- Tests device status functions (`updateDeviceStatus`, `deviceStatus`, `deviceReady`)
- Verifies battery management functions (`readBatteryVoltage`, `checkBatteryPresence`)
- Checks battery status variables (`batteryDetected`, `connectionStable`, `isCharging`)
- Tests periodic battery level updates
- Validates constants accessibility

**Usage**:
1. Open `test_compilation.ino` in Arduino IDE
2. Select the XIAO ESP32-S3 board
3. Compile and upload
4. Open Serial Monitor (921600 baud)
5. Monitor the real-time battery and device status

### `test_timing_utils.ino`
**Purpose**: Comprehensive test of all timing utility functions and demonstrate their usage.

**Features**:
- Tests timeout functionality (`hasTimedOut`, `getElapsedTime`)
- Verifies performance measurement (`measureStart`, `measureEnd`)
- Demonstrates non-blocking delays (`nonBlockingDelayStateful`)
- Tests rate limiting (`shouldExecute`) and throttling (`throttle`)
- Shows time remaining calculations (`getTimeRemaining`)
- Validates time window checking (`isWithinTimeWindow`)
- Continuous loop testing with real-time examples

**Usage**:
1. Open `test_timing_utils.ino` in Arduino IDE
2. Select the XIAO ESP32-S3 board
3. Compile and upload
4. Open Serial Monitor (921600 baud)
5. Watch the comprehensive timing tests execute

### `test_power_management.ino`
**Purpose**: Comprehensive test of all power management utility functions and demonstrate power optimization.

**Features**:
- Tests power mode switching (`setPowerMode`, CPU frequency changes)
- Verifies power consumption estimation (`estimateCurrentConsumption`)
- Demonstrates battery-based power optimization (`optimizePowerForBattery`)
- Tests battery life estimation (`estimateBatteryLife`)
- Shows power saving decisions (`shouldEnterPowerSaving`)
- Continuous power monitoring with simulated conditions
- Wake-up reason detection (`getWakeupReason`)

**Note**: Sleep functionality tests are simulated to avoid RTC GPIO compilation conflicts with ESP32 Arduino Core 2.0.17. The actual sleep functions are available but may require manual GPIO configuration.

**Usage**:
1. Open `test_power_management.ino` in Arduino IDE
2. Select the XIAO ESP32-S3 board
3. Compile and upload
4. Open Serial Monitor (921600 baud)
5. Watch the comprehensive power management tests execute

## Compilation Notes

### Include Paths
From the examples directory, use relative paths to access the organized firmware structure:
```cpp
#include "../hardware/camera_pins.h"
#include "../platform/constants.h"
#include "../core/battery_code.h"
#include "../core/device_status.h"
#include "../utils/mulaw.h"
#include "../firmware/src/system/clock/timing.h"
#include "../utils/power_management.h"
```

### Required Definitions
All examples require these definitions at the top:
```cpp
#define CAMERA_MODEL_XIAO_ESP32S3
#define XIAO_ESP32S3_SENSE
```

### Dependencies
- Examples depend on the main firmware's `.cpp` files in the root directory
- Make sure `battery_code.cpp` and `device_status.cpp` are in `/firmware/`
- All header files should be in their respective subdirectories

## Troubleshooting

### Compilation Errors
- Ensure all required `.cpp` files are in the main firmware directory
- Check that all include paths use `../` to go up one directory level
- Verify that `CAMERA_MODEL_XIAO_ESP32S3` and `XIAO_ESP32S3_SENSE` are defined

### Runtime Issues
- Make sure the correct board (XIAO ESP32-S3) is selected
- Use the correct baud rate (921600) for Serial Monitor
- Check that the hardware constants match your actual hardware configuration

## Adding New Examples

When creating new example sketches:

1. **File Location**: Place in `/firmware/examples/`
2. **Include Paths**: Use `../` relative paths to access firmware files
3. **Required Definitions**: Include the necessary `#define` statements
4. **Documentation**: Update this README with the new example
5. **Testing**: Verify compilation and runtime behavior

## Hardware Requirements

All examples are designed for:
- **Board**: Seeed Studio XIAO ESP32-S3 Sense
- **Framework**: Espressif ESP32 Arduino Core 2.0.17
- **Features**: Camera, microphone, battery management, BLE

Make sure your hardware matches these requirements for proper operation. 