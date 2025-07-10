# OpenGlass Firmware Structure

This document describes the organized folder structure for the OpenGlass firmware, designed for the Seeed Studio XIAO ESP32-S3.

## Folder Organization

```
firmware/
├── firmware.ino              # Main firmware file
├── battery_code.cpp          # Battery management implementation
├── device_status.cpp         # Device status implementation
├── power_management.cpp      # Power management implementation
├── STRUCTURE.md              # This file - structure documentation
├── readme.md                 # Build instructions and hardware info
├── hardware/                 # Hardware abstraction layer
│   ├── xiao_esp32s3_constants.h    # XIAO ESP32-S3 chip specifications
│   ├── camera_pins.h               # Camera pin definitions for all models
│   └── camera_index.h              # Camera configuration data
├── platform/                # Platform-specific configurations
│   └── constants.h                 # Application constants and configurations
├── core/                     # Core application logic (headers only)
│   ├── battery_code.h              # Battery management interface
│   └── device_status.h             # Device status management interface
├── utils/                    # Utility functions and helpers
│   └── mulaw.h                     # Audio compression utilities
├── examples/                 # Example sketches and verification tools
│   └── verify_constants.ino        # Constants verification sketch
└── src/                      # Reserved for future expansion
```

## Arduino IDE Compatibility Note

**Important**: Due to Arduino IDE limitations, `.cpp` implementation files must be in the main firmware directory. The Arduino IDE has specific rules about how it discovers and compiles `.cpp` files in subdirectories, which can cause linking issues.

**Structure Convention:**
- **Header files (`.h`)** → Organized in logical subdirectories
- **Implementation files (`.cpp`)** → Main firmware directory
- **Main sketch (`.ino`)** → Main firmware directory

This hybrid approach gives us the benefits of organized headers while maintaining full Arduino IDE compatibility.

## Folder Purposes

### 📱 `hardware/`
**Hardware Abstraction Layer** - Contains all hardware-specific definitions and configurations.

- **`xiao_esp32s3_constants.h`** - Complete XIAO ESP32-S3 chip specifications
  - GPIO pin definitions
  - Memory configuration
  - Power specifications
  - ADC configuration
  - Camera pin mappings

- **`camera_pins.h`** - Camera pin definitions for all supported ESP32 camera models
  - Uses constants from `xiao_esp32s3_constants.h` for XIAO ESP32-S3
  - Supports multiple camera models with conditional compilation

- **`camera_index.h`** - Camera configuration data and lookup tables

### ⚙️ `platform/`
**Platform Configuration** - Application-level constants and platform-specific settings.

- **`constants.h`** - Application constants that use hardware definitions
  - BLE service UUIDs
  - Audio configuration
  - Photo control commands
  - Device status codes
  - Battery and charging detection settings

### 🔧 `core/`
**Core Application Logic** - Main business logic and device functionality (headers only).

- **`battery_code.h`** - Battery management system interface
  - Battery presence detection
  - Voltage monitoring and level calculation
  - Charging status detection
  - Connection stability analysis
  - BLE battery service

- **`device_status.h`** - Device status management interface
  - Status reporting via BLE
  - State transitions
  - Error handling

**Note**: Implementation files (`battery_code.cpp`, `device_status.cpp`) are in the main directory for Arduino IDE compatibility.

### 🛠️ `utils/`
**Utility Functions** - Reusable helper functions and utilities.

- **`mulaw.h`** - Audio compression utilities
  - μ-law encoding/decoding
  - Audio format conversions

- **`timing.h`** - Timing and delay utilities
  - Timeout checking and elapsed time calculations
  - Non-blocking delays and rate limiting
  - Performance measurement tools
  - Throttling and debouncing helpers

- **`power_management.h`** - Power management and optimization utilities
  - Power mode switching (Performance, Balanced, Power Save, Ultra Low)
  - Sleep mode management (Light sleep, Deep sleep)
  - Power consumption estimation and monitoring
  - Battery-based power optimization
  - Wake-up source configuration

### 📚 `examples/`
**Examples and Verification** - Test sketches and verification tools.

- **`verify_constants.ino`** - Verification sketch for hardware constants
  - Tests all major constant categories
  - Provides runtime verification
  - Useful for debugging hardware configurations

### 📁 `src/`
**Future Expansion** - Reserved for additional source files as the project grows.

## Include Path Conventions

### From Main Firmware (`firmware.ino`)
```cpp
#include "hardware/camera_pins.h"
#include "platform/constants.h"
#include "core/battery_code.h"
#include "core/device_status.h"
#include "utils/mulaw.h"
#include "utils/timing.h"
#include "utils/power_management.h"
```

### From Implementation Files (`.cpp` in main directory)
```cpp
// From main directory to subdirectories
#include "core/battery_code.h"
#include "hardware/xiao_esp32s3_constants.h"
#include "platform/constants.h"
```

### From Subfolder Files
```cpp
// From core/ or platform/ to hardware/
#include "../hardware/xiao_esp32s3_constants.h"

// From core/ to platform/
#include "../platform/constants.h"

// From examples/ to hardware/
#include "../hardware/xiao_esp32s3_constants.h"
```

## Benefits of This Structure

### 🎯 **Clear Separation of Concerns**
- Hardware definitions separated from application logic
- Platform configuration isolated from core functionality
- Utilities clearly identified and reusable

### 📈 **Scalability**
- Easy to add new hardware support in `hardware/`
- Core functionality can grow without cluttering
- Examples and tests have dedicated space

### 🔍 **Maintainability**
- Quick to locate specific functionality
- Dependencies are clear from include paths
- Reduced risk of circular dependencies

### 🚀 **Development Efficiency**
- New developers can quickly understand the structure
- Related files are grouped together
- Clear naming conventions throughout

### ⚡ **Arduino IDE Compatibility**
- All `.cpp` files in main directory for proper compilation
- Headers organized in logical subdirectories
- Works with both Arduino IDE and arduino-cli

## Compilation Notes

### Arduino IDE
- The Arduino IDE automatically finds all `.cpp` files in the main directory
- Header files in subdirectories are found through include paths
- No special configuration needed

### arduino-cli
- All `.cpp` files in the main directory are automatically compiled
- Subdirectory headers work through relative include paths
- No special configuration needed for the folder structure

### Dependencies
- Hardware layer has no dependencies on application code
- Platform layer depends on hardware layer
- Core layer depends on both hardware and platform layers
- Utils are standalone and dependency-free

## Future Expansion

As the project grows, additional folders can be added:

- `drivers/` - Device drivers for sensors, displays, etc. (headers only)
- `protocols/` - Communication protocol implementations (headers only)
- `tests/` - Unit tests and integration tests
- `docs/` - Additional documentation and schematics

**Remember**: New `.cpp` files should be added to the main firmware directory, while `.h` files can be organized in logical subdirectories.

This structure provides a solid foundation for the OpenGlass firmware while maintaining clarity, organization, and full Arduino IDE compatibility. 