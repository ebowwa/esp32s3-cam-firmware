# Dual LED Patterns Reference

Complete guide to understanding the intelligent dual LED status indicators on the OpenGlass device for debugging and monitoring.

## 📋 Table of Contents

- [**Overview**](#overview)
- [**LED Hardware**](#led-hardware)
- [**Dual LED Configuration**](#dual-led-configuration)
- [**Operating Modes**](#operating-modes)
- [**Color Meanings**](#color-meanings)
- [**Pattern Categories**](#pattern-categories)
- [**Device Status Patterns**](#device-status-patterns)
- [**Connection Status Patterns**](#connection-status-patterns)
- [**Operation Status Patterns**](#operation-status-patterns)
- [**Error and Debug Patterns**](#error-and-debug-patterns)
- [**Power and Battery Patterns**](#power-and-battery-patterns)
- [**Special Dual LED Effects**](#special-dual-led-effects)
- [**Troubleshooting Guide**](#troubleshooting-guide)
- [**Pattern Reference Table**](#pattern-reference-table)

---

## Overview

The OpenGlass device uses an intelligent **Dual LED Management System** that makes optimal use of the two LEDs available on the XIAO ESP32-S3 hardware. This provides richer visual feedback for debugging device crashes and monitoring operations.

### Key Benefits
- **Dual LED status indication** using both available LEDs
- **Intelligent crash detection** through complementary LED patterns
- **Automatic charging feedback** via hardware-controlled charge LED
- **Enhanced debugging** with dual-color status combinations
- **Optional RGB enhancement** with external addressable LEDs
- **Power-efficient operation** optimized for battery use

### Hardware LEDs Available
- **User LED** (GPIO21) - Fully programmable for status indication
- **Charge LED** (hardware controlled) - Automatic charging status
- **External RGB LED** (optional) - Enhanced color capabilities

---

## LED Hardware

### Built-in Dual LEDs (XIAO ESP32-S3)
- **User LED**: GPIO21 (XIAO_ESP32S3_USER_LED_PIN)
  - Type: Single color LED (white/blue)
  - Control: Fully programmable by firmware
  - Usage: Primary status indication
  - Power: Low power consumption

- **Charge LED**: Hardware controlled
  - Type: Single color LED (typically red/orange)
  - Control: Automatic hardware charging indication
  - Usage: Shows charging status without firmware intervention
  - Power: Minimal impact

### External RGB LED (Optional Enhancement)
- **Supported Types**: WS2812B, WS2812, NeoPixel, and compatible addressable LEDs
- **Default GPIO**: 2 (configurable)
- **Control**: Full RGB color control with 256 brightness levels per channel
- **Library**: FastLED (recommended)
- **Usage**: Secondary status indication with full color support
- **Power**: Higher power consumption, requires adequate power supply

### RGB Enhancement Setup
To enable external RGB LED support:
1. Connect an external RGB LED to any available GPIO pin
2. Uncomment `#define RGB_LED_ENABLED` in `led_manager.h`
3. Set `RGB_LED_PIN` to your connected GPIO pin
4. Install FastLED library
5. Recompile and upload firmware

---

## Dual LED Configuration

### Operating Modes
The system automatically selects the appropriate mode based on configuration:

```cpp
typedef enum {
    DUAL_LED_MODE_SINGLE,      // Use only User LED (GPIO21)
    DUAL_LED_MODE_DUAL_BASIC,  // Use User LED + monitor Charge LED
    DUAL_LED_MODE_RGB_ENHANCED // Use User LED + external RGB LED
} dual_led_mode_t;
```

### Hardware Configuration
```cpp
// Basic dual LED mode (default)
// Uses User LED (GPIO21) + Charge LED monitoring

// Enhanced RGB mode (optional)
#define RGB_LED_ENABLED         // Enable external RGB support
#define RGB_LED_PIN 2           // GPIO pin for external RGB LED
#define RGB_LED_COUNT 1         // Number of RGB LEDs
#define RGB_LED_TYPE WS2812B    // LED type
#define RGB_COLOR_ORDER GRB     // Color order
#define RGB_LED_BRIGHTNESS 50   // Default brightness (0-255)
```

### Color Definitions
The system includes 19 predefined colors for intelligent status indication:
- **LED_COLOR_WHITE** - {255, 255, 255}
- **LED_COLOR_RED** - {255, 0, 0}
- **LED_COLOR_GREEN** - {0, 255, 0}
- **LED_COLOR_BLUE** - {0, 0, 255}
- **LED_COLOR_YELLOW** - {255, 255, 0}
- **LED_COLOR_PURPLE** - {128, 0, 128}
- **LED_COLOR_CYAN** - {0, 255, 255}
- **LED_COLOR_ORANGE** - {255, 165, 0}
- **LED_COLOR_PINK** - {255, 192, 203}
- **LED_COLOR_LIME** - {50, 205, 50}
- **LED_COLOR_INDIGO** - {75, 0, 130}
- **LED_COLOR_VIOLET** - {238, 130, 238}
- **LED_COLOR_GOLD** - {255, 215, 0}
- **LED_COLOR_SILVER** - {192, 192, 192}
- **LED_COLOR_MAROON** - {128, 0, 0}
- **LED_COLOR_NAVY** - {0, 0, 128}
- **LED_COLOR_TEAL** - {0, 128, 128}
- **LED_COLOR_OLIVE** - {128, 128, 0}
- **LED_COLOR_OFF** - {0, 0, 0}

---

## Operating Modes

### DUAL_LED_MODE_SINGLE
- Uses only the User LED (GPIO21)
- Charge LED operates independently (hardware controlled)
- Minimal power consumption
- Basic status indication

### DUAL_LED_MODE_DUAL_BASIC (Default)
- Uses User LED for primary status
- Monitors Charge LED for charging status
- Coordinated dual LED patterns
- Enhanced debugging capabilities

### DUAL_LED_MODE_RGB_ENHANCED
- Uses User LED for primary status
- Uses external RGB LED for secondary status
- Full color combinations available
- Maximum debugging information
- Higher power consumption

---

## Color Meanings

### Dual LED Status Coding
- **🔴 RED + MAROON**: Critical errors, system failures
- **🟢 GREEN + BLUE**: Ready and connected states
- **🔵 BLUE + CYAN**: Streaming and data operations
- **🟡 YELLOW + ORANGE**: Charging and initialization
- **🟣 PURPLE + VIOLET**: Camera operations
- **🟠 ORANGE + GOLD**: Microphone and audio operations
- **🔵 CYAN + YELLOW**: Startup and system ready
- **⚪ WHITE + SILVER**: Photo capture and general operations
- **🔴 RED + YELLOW**: Battery and power issues

### Pattern Intensity
- **Solid**: Stable state
- **Slow Blink**: Normal operation
- **Fast Blink**: Active processing
- **Pulse**: Breathing/standby mode
- **Dual Indication**: Alternating between LEDs
- **Rapid Flash**: Urgent attention needed

---

## Pattern Categories

### 1. Static Patterns
- **LED_OFF**: Both LEDs off
- **LED_ON**: Primary LED on with secondary color

### 2. Blinking Patterns
- **LED_BLINK_SLOW**: 1Hz blink (500ms on/off)
- **LED_BLINK_FAST**: 5Hz blink (100ms on/off)
- **LED_BLINK_VERY_FAST**: 10Hz blink (50ms on/off)

### 3. Breathing Patterns
- **LED_PULSE**: Smooth breathing effect on both LEDs
- **LED_CONNECTED**: Slow pulse when connected

### 4. Special Dual Patterns
- **LED_HEARTBEAT**: Double-pulse pattern
- **LED_SOS**: Morse code SOS (... --- ...)
- **LED_STARTUP**: Alternating startup sequence
- **LED_DUAL_INDICATION**: Alternating between User LED and secondary LED
- **LED_FACTORY_RESET**: Rapid alternating sequence

---

## Device Status Patterns

### System Status Indicators

| Status Code | Pattern | Primary Color | Secondary Color | Description |
|-------------|---------|---------------|-----------------|-------------|
| 0x01 | STARTUP | CYAN | YELLOW | Device initializing with alternating sequence |
| 0x02 | PULSE | YELLOW | ORANGE | System warming up |
| 0x03 | CONNECTED | GREEN | BLUE | Device ready and operational |
| 0x04 | ERROR | RED | MAROON | System error detected |
| 0x05 | BLINK_FAST | PURPLE | VIOLET | Camera initializing |
| 0x06 | BLINK_FAST | ORANGE | GOLD | Microphone initializing |
| 0x07 | BLINK_FAST | BLUE | CYAN | BLE initializing |
| 0x08 | SOS | RED | YELLOW | Battery not detected |
| 0x09 | PULSE | YELLOW | GREEN | Battery charging |
| 0x0A | DUAL_INDICATION | RED | YELLOW | Battery unstable/low |

### Timing Diagrams

#### Startup Sequence (CYAN + YELLOW)
```
Time:     0    0.1s  0.2s  0.3s  0.4s  0.5s  0.6s  0.7s  0.8s  0.9s  1.0s
User LED: ON   OFF   ON    OFF   ON    OFF   ON    OFF   ON    OFF   ON
RGB LED:  OFF  ON    OFF   ON    OFF   ON    OFF   ON    OFF   ON    OFF
```

#### Dual Indication Pattern
```
Time:     0    0.25s 0.5s  0.75s 1.0s  1.25s 1.5s  1.75s 2.0s
User LED: ON   OFF   ON    OFF   ON    OFF   ON    OFF   [repeat]
RGB LED:  OFF  OFF   OFF   ON    OFF   ON    OFF   ON    [repeat]
```

#### SOS Pattern (RED + YELLOW)
```
Time: 0   0.2s  0.4s  0.6s  0.8s  1.0s  1.2s  1.4s  1.6s  1.8s  2.0s
User: ON  OFF   ON    OFF   ON    OFF   ON    OFF   ON    OFF   ON
RGB:  OFF ON    OFF   ON    OFF   ON    OFF   ON    OFF   ON    OFF
      .   .     .     .     .     .     -     -     -     -     -
```

---

## Connection Status Patterns

### BLE Connection States
- **CONNECTED**: Green User LED + Blue RGB LED (slow pulse)
- **DISCONNECTED**: Red User LED + Maroon RGB LED (single blink)
- **STREAMING**: Blue User LED + Cyan RGB LED (fast pulse)
- **PAIRING**: Blue User LED + Cyan RGB LED (fast blink)

### Network Status
- **WIFI_CONNECTING**: Yellow User LED + Orange RGB LED (blink)
- **WIFI_CONNECTED**: Green User LED + Blue RGB LED (solid)
- **WIFI_FAILED**: Red User LED + Maroon RGB LED (blink)

---

## Operation Status Patterns

### Camera Operations
- **PHOTO_CAPTURE**: White User LED + Silver RGB LED (single flash)
- **STREAMING**: Blue User LED + Cyan RGB LED (fast pulse)
- **CAMERA_INIT**: Purple User LED + Violet RGB LED (fast blink)

### Audio Operations
- **MICROPHONE_INIT**: Orange User LED + Gold RGB LED (fast blink)
- **AUDIO_RECORDING**: Orange User LED + Gold RGB LED (pulse)
- **AUDIO_PROCESSING**: Orange User LED + Gold RGB LED (blink)

### System Operations
- **FACTORY_RESET**: Rapid alternating pattern between all LEDs
- **FIRMWARE_UPDATE**: Slow alternating rainbow cycle
- **CALIBRATION**: White User LED + Blue RGB LED (slow pulse)

---

## Error and Debug Patterns

### Critical Errors
- **SYSTEM_ERROR**: Red User LED + Maroon RGB LED (very fast blink)
- **MEMORY_ERROR**: Red User LED + Yellow RGB LED (SOS pattern)
- **HARDWARE_ERROR**: Alternating Red/White User LED + RGB LED

### Warning States
- **LOW_BATTERY**: Red User LED + Orange RGB LED (heartbeat pattern)
- **OVERHEATING**: Red User LED + Yellow RGB LED (slow pulse)
- **STORAGE_FULL**: Orange User LED + Yellow RGB LED (SOS pattern)

### Debug Patterns
- **BOOT_SEQUENCE**: 3-second alternating startup pattern
- **WATCHDOG_RESET**: 5 rapid white flashes on both LEDs
- **EXCEPTION_HANDLER**: Continuous red blink on User LED

---

## Power and Battery Patterns

### Battery Status
- **CHARGING**: Yellow User LED + Green RGB LED (breathing effect)
- **FULLY_CHARGED**: Green User LED + Blue RGB LED (solid for 3 seconds)
- **LOW_BATTERY**: Red User LED + Orange RGB LED (heartbeat pattern)
- **CRITICAL_BATTERY**: Red User LED + Yellow RGB LED (rapid blink)
- **BATTERY_NOT_DETECTED**: Red User LED + Yellow RGB LED (SOS pattern)

### Power Management
- **DEEP_SLEEP_ENTRY**: Fade out both LEDs over 2 seconds
- **WAKE_FROM_SLEEP**: Quick flash both LEDs
- **POWER_SAVING_MODE**: Dim pulse on User LED only
- **USB_POWER_DETECTED**: Brief green flash on both LEDs

---

## Special Dual LED Effects

### Convenience Functions
```cpp
// Quick dual LED patterns
setLedPatternDualRed();        // Red + Maroon dual indication
setLedPatternDualGreen();      // Green + Lime dual indication
setLedPatternDualBlue();       // Blue + Cyan dual indication
setLedPatternStreaming();      // Blue + Cyan streaming pulse
setLedPatternPhotoCapture();   // White + Silver flash
setLedPatternBatteryLow();     // Red + Orange heartbeat
setLedPatternConnected();      // Green + Blue connected pulse
setLedPatternDisconnected();   // Red + Maroon disconnected blink
```

### Advanced Patterns
```cpp
// Dual LED flash with custom colors
flashDualLed(LED_COLOR_WHITE, LED_COLOR_BLUE, 200);

// Set custom dual LED pattern
setLedPattern(LED_DUAL_INDICATION, LED_COLOR_RED, LED_COLOR_YELLOW);

// Check current mode
dual_led_mode_t mode = getCurrentLedMode();
```

---

## Troubleshooting Guide

### Device Crash Debugging

#### LED Patterns During Crashes
1. **Normal Operation**: Green + Blue connected pulse
2. **Crash Detected**: Pattern stops or changes to error pattern
3. **Watchdog Reset**: 5 rapid white flashes on both LEDs
4. **Boot Loop**: Repeated startup pattern (cyan + yellow alternating)

#### Common Crash Patterns
- **Memory Overflow**: Red + Yellow SOS pattern
- **Stack Overflow**: Red + Maroon very fast blink
- **Hardware Failure**: Alternating red/white pattern
- **Power Issues**: Red + Orange heartbeat pattern

### Hardware Issues

#### User LED Problems
- **LED not working**: Check GPIO21 connection
- **Dim LED**: Check power supply voltage
- **Flickering**: Check for loose connections

#### Charge LED Issues
- **Charge LED always on**: Normal when charging
- **Charge LED never on**: Check battery connection
- **Charge LED flickering**: Check power supply

#### External RGB LED Issues
- **No colors**: Verify FastLED library installation
- **Wrong colors**: Check color order (GRB vs RGB)
- **Flickering**: Check power supply capacity
- **Partial colors**: Check data line connection

### Performance Optimization

#### Reducing LED Update Overhead
```cpp
// Reduce update frequency for battery saving
if (millis() - lastLedUpdate > 50) {  // 20Hz instead of 50Hz
    updateLed();
    lastLedUpdate = millis();
}
```

#### Memory Usage
- Single LED mode: ~150 bytes
- Dual basic mode: ~200 bytes
- RGB enhanced mode: ~300 bytes + FastLED overhead

---

## Pattern Reference Table

| Pattern Name | Duration | Primary Color | Secondary Color | Use Case | Power |
|--------------|----------|---------------|-----------------|----------|-------|
| LED_OFF | Continuous | None | None | Device off | Minimal |
| LED_ON | Continuous | Configurable | Configurable | Solid status | Low |
| LED_BLINK_SLOW | 1Hz | Configurable | Configurable | Normal operation | Low |
| LED_BLINK_FAST | 5Hz | Configurable | Configurable | Active processing | Medium |
| LED_BLINK_VERY_FAST | 10Hz | Configurable | Configurable | Urgent status | Medium |
| LED_PULSE | 1Hz breathing | Configurable | Configurable | Standby mode | Low |
| LED_HEARTBEAT | 2-beat pattern | Configurable | Configurable | System alive | Low |
| LED_SOS | Morse SOS | RED | YELLOW | Emergency | Medium |
| LED_STARTUP | Alternating | CYAN | YELLOW | Boot sequence | Medium |
| LED_DUAL_INDICATION | Alternating | Configurable | Configurable | Dual status | Medium |
| LED_ERROR | 20Hz | RED | MAROON | System error | High |
| LED_CONNECTED | Slow pulse | GREEN | BLUE | Connected | Low |
| LED_DISCONNECTED | 0.5Hz | RED | MAROON | Disconnected | Low |
| LED_CHARGING | Breathing | YELLOW | GREEN | Charging | Medium |
| LED_LOW_BATTERY | Heartbeat | RED | ORANGE | Low battery | Low |
| LED_STREAMING | Fast pulse | BLUE | CYAN | Data streaming | Medium |
| LED_PHOTO_CAPTURE | Single flash | WHITE | SILVER | Photo taken | Low |
| LED_FACTORY_RESET | Rapid alternating | Multiple | Multiple | Factory reset | High |

### Convenience Functions Reference

| Function | Primary Color | Secondary Color | Description |
|----------|---------------|-----------------|-------------|
| `setLedPatternDualRed()` | RED | MAROON | Quick red dual pattern |
| `setLedPatternDualGreen()` | GREEN | LIME | Quick green dual pattern |
| `setLedPatternDualBlue()` | BLUE | CYAN | Quick blue dual pattern |
| `setLedPatternStreaming()` | BLUE | CYAN | Streaming indication |
| `setLedPatternPhotoCapture()` | WHITE | SILVER | Photo capture flash |
| `setLedPatternBatteryLow()` | RED | ORANGE | Battery low warning |
| `setLedPatternConnected()` | GREEN | BLUE | Connected status |
| `setLedPatternDisconnected()` | RED | MAROON | Disconnected status |

---

## Development Best Practices

### LED Pattern Design
1. **Use complementary colors** for primary and secondary LEDs
2. **Reserve red patterns** for errors and critical states
3. **Use green patterns** for success and ready states
4. **Implement consistent timing** across similar patterns
5. **Test patterns** in different lighting conditions

### Power Management
1. **Use pulse patterns** instead of solid on for battery saving
2. **Reduce update frequency** when not critical
3. **Turn off LEDs** during deep sleep
4. **Monitor power consumption** with dual LED patterns

### Debugging Guidelines
1. **Use distinct patterns** for different error types
2. **Implement pattern timeouts** to prevent infinite loops
3. **Log pattern changes** for debugging
4. **Test crash scenarios** with LED feedback
5. **Document custom patterns** for team reference

---

## Hardware Specifications

### XIAO ESP32-S3 LED Specifications
- **User LED**: GPIO21, 3.3V logic, ~20mA max current
- **Charge LED**: Hardware controlled, automatic operation
- **External RGB**: 5V compatible, WS2812B protocol
- **Update Rate**: Up to 50Hz for smooth patterns
- **Power Consumption**: 
  - User LED: ~20mA when on
  - External RGB: ~60mA per LED at full brightness
  - Charge LED: ~5mA (hardware controlled)

### Timing Specifications
- **Minimum Pattern Duration**: 50ms
- **Maximum Update Rate**: 50Hz
- **Pattern Transition Time**: <10ms
- **Startup Pattern Duration**: 1-3 seconds
- **SOS Pattern Duration**: 2.6 seconds
- **Heartbeat Pattern Period**: 800ms

---

## Conclusion

The Dual LED Management System provides comprehensive visual feedback for the OpenGlass device, making it easier to debug crashes, monitor operations, and understand device status. By intelligently using both the User LED and Charge LED (plus optional external RGB LED), the system offers rich status indication while maintaining power efficiency.

For development and debugging, the dual LED patterns provide immediate visual feedback without requiring serial monitor access, making it invaluable for troubleshooting device issues in the field.

---

*Last updated: [Current Date]*  
*Version: 2.0 - Dual LED System* 