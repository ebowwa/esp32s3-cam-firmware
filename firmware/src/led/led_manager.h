#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include "../platform/xiao_esp32s3_constants.h"

// ===================================================================
// DUAL LED MANAGEMENT FOR XIAO ESP32-S3
// ===================================================================

/**
 * Dual LED Configuration for XIAO ESP32-S3
 * 
 * The XIAO ESP32-S3 has two LEDs:
 * 1. User LED (GPIO21) - Programmable, controlled by firmware
 * 2. Charge LED (hardware controlled) - Shows charging status automatically
 * 
 * This manager intelligently uses both LEDs for comprehensive status indication:
 * - Primary status indication via User LED (GPIO21)
 * - Secondary/complementary indication via external RGB LED (optional)
 * - Charge LED provides automatic charging status
 * 
 * For enhanced RGB support, connect an external WS2812/NeoPixel LED to any GPIO
 */

// Uncomment the next line to enable external RGB LED support
// #define RGB_LED_ENABLED

#ifdef RGB_LED_ENABLED
#include <FastLED.h>
#define RGB_LED_PIN 2           // GPIO pin for external RGB LED (configurable)
#define RGB_LED_COUNT 1         // Number of RGB LEDs
#define RGB_LED_TYPE WS2812B    // LED type (WS2812B, WS2812, etc.)
#define RGB_COLOR_ORDER GRB     // Color order (GRB, RGB, etc.)
#define RGB_LED_BRIGHTNESS 50   // Default brightness (0-255)
#endif

/**
 * LED status patterns for different device states
 */
typedef enum {
    LED_OFF,                    // Both LEDs off
    LED_ON,                     // User LED on (solid)
    LED_BLINK_SLOW,            // Slow blink (1Hz)
    LED_BLINK_FAST,            // Fast blink (5Hz)
    LED_BLINK_VERY_FAST,       // Very fast blink (10Hz)
    LED_PULSE,                 // Breathing pattern
    LED_HEARTBEAT,             // Double blink pattern
    LED_SOS,                   // SOS pattern (... --- ...)
    LED_STARTUP,               // Startup pattern (rainbow or fast blink sequence)
    LED_ERROR,                 // Error pattern (rapid blink)
    LED_CONNECTED,             // Connected pattern (slow pulse)
    LED_DISCONNECTED,          // Disconnected pattern (single blink)
    LED_CHARGING,              // Charging pattern (handled by hardware charge LED)
    LED_LOW_BATTERY,           // Low battery pattern (slow double blink)
    LED_STREAMING,             // Streaming pattern (fast pulse)
    LED_PHOTO_CAPTURE,         // Photo capture pattern (single flash)
    LED_FACTORY_RESET,         // Factory reset pattern (rapid sequence)
    LED_DUAL_INDICATION        // Special dual LED pattern
} led_pattern_t;

/**
 * Dual LED mode configuration
 */
typedef enum {
    DUAL_LED_MODE_SINGLE,      // Use only User LED (GPIO21)
    DUAL_LED_MODE_DUAL_BASIC,  // Use User LED + monitor charge LED status
    DUAL_LED_MODE_RGB_ENHANCED // Use User LED + external RGB LED
} dual_led_mode_t;

/**
 * RGB LED colors with full color support
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

// Predefined colors
#define LED_COLOR_OFF       {0, 0, 0}
#define LED_COLOR_WHITE     {255, 255, 255}
#define LED_COLOR_RED       {255, 0, 0}
#define LED_COLOR_GREEN     {0, 255, 0}
#define LED_COLOR_BLUE      {0, 0, 255}
#define LED_COLOR_YELLOW    {255, 255, 0}
#define LED_COLOR_PURPLE    {128, 0, 128}
#define LED_COLOR_CYAN      {0, 255, 255}
#define LED_COLOR_ORANGE    {255, 165, 0}
#define LED_COLOR_PINK      {255, 192, 203}
#define LED_COLOR_LIME      {50, 205, 50}
#define LED_COLOR_INDIGO    {75, 0, 130}
#define LED_COLOR_VIOLET    {238, 130, 238}
#define LED_COLOR_GOLD      {255, 215, 0}
#define LED_COLOR_SILVER    {192, 192, 192}
#define LED_COLOR_MAROON    {128, 0, 0}
#define LED_COLOR_NAVY      {0, 0, 128}
#define LED_COLOR_TEAL      {0, 128, 128}
#define LED_COLOR_OLIVE     {128, 128, 0}

/**
 * Dual LED state structure
 */
typedef struct {
    led_pattern_t pattern;
    rgb_color_t primary_color;    // Color for User LED (intensity-based)
    rgb_color_t secondary_color;  // Color for RGB LED (if enabled)
    dual_led_mode_t mode;
    bool enabled;
    unsigned long lastUpdate;
    int step;
    int brightness;
    bool user_led_state;
    bool charge_led_detected;     // Status of hardware charge LED
} dual_led_state_t;

// Global LED state
extern dual_led_state_t dualLedState;

#ifdef RGB_LED_ENABLED
extern CRGB leds[RGB_LED_COUNT];
#endif

/**
 * Initialize dual LED manager
 */
static inline void initLedManager() {
    // Initialize User LED (GPIO21)
    pinMode(XIAO_ESP32S3_USER_LED_PIN, OUTPUT);
    digitalWrite(XIAO_ESP32S3_USER_LED_PIN, LOW);
    
#ifdef RGB_LED_ENABLED
    // Initialize external RGB LED
    FastLED.addLeds<RGB_LED_TYPE, RGB_LED_PIN, RGB_COLOR_ORDER>(leds, RGB_LED_COUNT);
    FastLED.setBrightness(RGB_LED_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    dualLedState.mode = DUAL_LED_MODE_RGB_ENHANCED;
    Serial.println("Dual LED Manager initialized: User LED (GPIO21) + RGB LED (GPIO" + String(RGB_LED_PIN) + ")");
#else
    dualLedState.mode = DUAL_LED_MODE_DUAL_BASIC;
    Serial.println("Dual LED Manager initialized: User LED (GPIO21) + Charge LED monitoring");
#endif
    
    // Initialize state
    dualLedState.pattern = LED_OFF;
    dualLedState.primary_color = (rgb_color_t)LED_COLOR_WHITE;
    dualLedState.secondary_color = (rgb_color_t)LED_COLOR_BLUE;
    dualLedState.enabled = true;
    dualLedState.lastUpdate = 0;
    dualLedState.step = 0;
    dualLedState.brightness = 255;
    dualLedState.user_led_state = false;
    dualLedState.charge_led_detected = false;
}

/**
 * Set User LED state with intensity-based color simulation
 */
static inline void setUserLed(rgb_color_t color, bool show = true) {
    // For single LED, use brightness based on color intensity
    int intensity = (color.r + color.g + color.b) / 3;
    if (intensity > 0) {
        digitalWrite(XIAO_ESP32S3_USER_LED_PIN, HIGH);
        dualLedState.user_led_state = true;
    } else {
        digitalWrite(XIAO_ESP32S3_USER_LED_PIN, LOW);
        dualLedState.user_led_state = false;
    }
}

/**
 * Set external RGB LED (if enabled)
 */
static inline void setRgbLed(rgb_color_t color, bool show = true) {
#ifdef RGB_LED_ENABLED
    leds[0] = CRGB(color.r, color.g, color.b);
    if (show) FastLED.show();
#endif
}

/**
 * Set dual LED colors intelligently
 */
static inline void setDualLedColors(rgb_color_t primary, rgb_color_t secondary = (rgb_color_t)LED_COLOR_OFF, bool show = true) {
    setUserLed(primary, show);
    
    if (dualLedState.mode == DUAL_LED_MODE_RGB_ENHANCED) {
        setRgbLed(secondary, show);
    }
}

/**
 * Set LED pattern with dual LED support
 */
static inline void setLedPattern(led_pattern_t pattern, rgb_color_t primary_color = (rgb_color_t)LED_COLOR_WHITE, rgb_color_t secondary_color = (rgb_color_t)LED_COLOR_BLUE) {
    dualLedState.pattern = pattern;
    dualLedState.primary_color = primary_color;
    dualLedState.secondary_color = secondary_color;
    dualLedState.step = 0;
    dualLedState.lastUpdate = millis();
}

/**
 * Update LED patterns (call this in main loop)
 */
static inline void updateLed() {
    if (!dualLedState.enabled) return;
    
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - dualLedState.lastUpdate;
    
    switch (dualLedState.pattern) {
        case LED_OFF:
            setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
            break;
            
        case LED_ON:
            setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
            break;
            
        case LED_BLINK_SLOW:
            if (elapsed > 500) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        case LED_BLINK_FAST:
            if (elapsed > 100) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        case LED_BLINK_VERY_FAST:
            if (elapsed > 50) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        case LED_PULSE:
            if (elapsed > 20) {
                int brightness = (sin(dualLedState.step * 0.1) + 1) * 127.5;
                rgb_color_t dimmed_primary = {
                    (uint8_t)(dualLedState.primary_color.r * brightness / 255),
                    (uint8_t)(dualLedState.primary_color.g * brightness / 255),
                    (uint8_t)(dualLedState.primary_color.b * brightness / 255)
                };
                rgb_color_t dimmed_secondary = {
                    (uint8_t)(dualLedState.secondary_color.r * brightness / 255),
                    (uint8_t)(dualLedState.secondary_color.g * brightness / 255),
                    (uint8_t)(dualLedState.secondary_color.b * brightness / 255)
                };
                setDualLedColors(dimmed_primary, dimmed_secondary);
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        case LED_HEARTBEAT:
            if (elapsed > 100) {
                int pattern[] = {1, 0, 1, 0, 0, 0, 0, 0};
                int index = dualLedState.step % 8;
                if (pattern[index]) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        case LED_SOS:
            if (elapsed > 200) {
                // SOS pattern: ... --- ...
                int pattern[] = {1,0,1,0,1,0,0,1,1,0,1,1,0,1,1,0,0,1,0,1,0,1,0,0,0,0};
                int index = dualLedState.step % 26;
                if (pattern[index]) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        case LED_STARTUP:
            if (elapsed > 100) {
                // Startup sequence: alternating pattern between LEDs
                if (dualLedState.step < 10) {
                    if (dualLedState.step % 2 == 0) {
                        setDualLedColors(dualLedState.primary_color, (rgb_color_t)LED_COLOR_OFF);
                    } else {
                        setDualLedColors((rgb_color_t)LED_COLOR_OFF, dualLedState.secondary_color);
                    }
                } else if (dualLedState.step < 15) {
                    // Final flash both
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                    dualLedState.pattern = LED_OFF; // End pattern
                }
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        case LED_DUAL_INDICATION:
            // Special pattern: User LED shows primary status, RGB LED shows secondary
            if (elapsed > 250) {
                if (dualLedState.step % 4 < 2) {
                    setUserLed(dualLedState.primary_color);
                } else {
                    setUserLed((rgb_color_t)LED_COLOR_OFF);
                }
                
                if ((dualLedState.step + 2) % 4 < 2) {
                    setRgbLed(dualLedState.secondary_color);
                } else {
                    setRgbLed((rgb_color_t)LED_COLOR_OFF);
                }
                
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
            
        default:
            // Default to simple blink
            if (elapsed > 500) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = currentTime;
            }
            break;
    }
}

/**
 * Enable/disable LED manager
 */
static inline void setLedEnabled(bool enabled) {
    dualLedState.enabled = enabled;
    if (!enabled) {
        setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
    }
}

/**
 * Get current LED pattern
 */
static inline led_pattern_t getCurrentLedPattern() {
    return dualLedState.pattern;
}

/**
 * Get current LED mode
 */
static inline dual_led_mode_t getCurrentLedMode() {
    return dualLedState.mode;
}

/**
 * Immediate LED flash with dual colors
 */
static inline void flashDualLed(rgb_color_t primary = (rgb_color_t)LED_COLOR_WHITE, rgb_color_t secondary = (rgb_color_t)LED_COLOR_BLUE, int duration_ms = 100) {
    setDualLedColors(primary, secondary);
    delay(duration_ms);
    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
}

/**
 * Set LED for device status with intelligent dual LED patterns
 */
static inline void setLedForDeviceStatus(uint8_t status) {
    switch (status) {
        case 0x01: // DEVICE_STATUS_INITIALIZING
            setLedPattern(LED_STARTUP, (rgb_color_t)LED_COLOR_CYAN, (rgb_color_t)LED_COLOR_YELLOW);
            break;
        case 0x02: // DEVICE_STATUS_WARMING_UP
            setLedPattern(LED_PULSE, (rgb_color_t)LED_COLOR_YELLOW, (rgb_color_t)LED_COLOR_ORANGE);
            break;
        case 0x03: // DEVICE_STATUS_READY
            setLedPattern(LED_CONNECTED, (rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_BLUE);
            break;
        case 0x04: // DEVICE_STATUS_ERROR
            setLedPattern(LED_ERROR, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_MAROON);
            break;
        case 0x05: // DEVICE_STATUS_CAMERA_INIT
            setLedPattern(LED_BLINK_FAST, (rgb_color_t)LED_COLOR_PURPLE, (rgb_color_t)LED_COLOR_VIOLET);
            break;
        case 0x06: // DEVICE_STATUS_MICROPHONE_INIT
            setLedPattern(LED_BLINK_FAST, (rgb_color_t)LED_COLOR_ORANGE, (rgb_color_t)LED_COLOR_GOLD);
            break;
        case 0x07: // DEVICE_STATUS_BLE_INIT
            setLedPattern(LED_BLINK_FAST, (rgb_color_t)LED_COLOR_BLUE, (rgb_color_t)LED_COLOR_CYAN);
            break;
        case 0x08: // DEVICE_STATUS_BATTERY_NOT_DETECTED
            setLedPattern(LED_SOS, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_YELLOW);
            break;
        case 0x09: // DEVICE_STATUS_CHARGING
            setLedPattern(LED_PULSE, (rgb_color_t)LED_COLOR_YELLOW, (rgb_color_t)LED_COLOR_GREEN);
            break;
        case 0x0A: // DEVICE_STATUS_BATTERY_UNSTABLE
            setLedPattern(LED_DUAL_INDICATION, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_YELLOW);
            break;
        default:
            setLedPattern(LED_BLINK_SLOW, (rgb_color_t)LED_COLOR_WHITE, (rgb_color_t)LED_COLOR_BLUE);
            break;
    }
}

/**
 * Convenience functions for common dual LED patterns
 */
static inline void setLedPatternDualRed() {
    setLedPattern(LED_DUAL_INDICATION, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_MAROON);
}

static inline void setLedPatternDualGreen() {
    setLedPattern(LED_DUAL_INDICATION, (rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_LIME);
}

static inline void setLedPatternDualBlue() {
    setLedPattern(LED_DUAL_INDICATION, (rgb_color_t)LED_COLOR_BLUE, (rgb_color_t)LED_COLOR_CYAN);
}

static inline void setLedPatternStreaming() {
    setLedPattern(LED_PULSE, (rgb_color_t)LED_COLOR_BLUE, (rgb_color_t)LED_COLOR_CYAN);
}

static inline void setLedPatternPhotoCapture() {
    flashDualLed((rgb_color_t)LED_COLOR_WHITE, (rgb_color_t)LED_COLOR_SILVER, 50);
}

static inline void setLedPatternBatteryLow() {
    setLedPattern(LED_HEARTBEAT, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_ORANGE);
}

static inline void setLedPatternConnected() {
    setLedPattern(LED_CONNECTED, (rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_BLUE);
}

static inline void setLedPatternDisconnected() {
    setLedPattern(LED_DISCONNECTED, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_MAROON);
}

#endif // LED_MANAGER_H 