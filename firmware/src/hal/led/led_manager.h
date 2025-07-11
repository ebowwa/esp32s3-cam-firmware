#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Arduino.h>
#include "../xiao_esp32s3_constants.h"
#include "../../system/clock/timing.h"

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

// ===================================================================
// FUNCTION DECLARATIONS
// ===================================================================

/**
 * Initialize dual LED manager
 */
void initLedManager();

/**
 * Set User LED state with intensity-based color simulation
 */
void setUserLed(rgb_color_t color, bool show = true);

/**
 * Set external RGB LED (if enabled)
 */
void setRgbLed(rgb_color_t color, bool show = true);

/**
 * Set dual LED colors intelligently
 */
void setDualLedColors(rgb_color_t primary, rgb_color_t secondary = (rgb_color_t)LED_COLOR_OFF, bool show = true);

/**
 * Set LED pattern with dual LED support
 */
void setLedPattern(led_pattern_t pattern, rgb_color_t primary_color = (rgb_color_t)LED_COLOR_WHITE, rgb_color_t secondary_color = (rgb_color_t)LED_COLOR_BLUE);

/**
 * Update LED patterns (call this in main loop)
 */
void updateLed();

/**
 * Enable/disable LED system
 */
void setLedEnabled(bool enabled);

/**
 * Get current LED pattern
 */
led_pattern_t getCurrentLedPattern();

/**
 * Get current LED mode
 */
dual_led_mode_t getCurrentLedMode();

/**
 * Flash dual LED briefly
 */
void flashDualLed(rgb_color_t primary = (rgb_color_t)LED_COLOR_WHITE, rgb_color_t secondary = (rgb_color_t)LED_COLOR_BLUE, int duration_ms = 100);

/**
 * Set LED pattern based on device status
 */
void setLedForDeviceStatus(uint8_t status);

/**
 * Convenience functions for common patterns
 */
void setLedPatternDualRed();
void setLedPatternDualGreen();
void setLedPatternDualBlue();
void setLedPatternStreaming();
void setLedPatternPhotoCapture();
void setLedPatternBatteryLow();
void setLedPatternConnected();
void setLedPatternDisconnected();

#endif // LED_MANAGER_H 