#include "led_manager.h"

// Global dual LED state variable
dual_led_state_t dualLedState;

#ifdef RGB_LED_ENABLED
// RGB LED array for FastLED
CRGB leds[RGB_LED_COUNT];
#endif

// ===================================================================
// LED MANAGER IMPLEMENTATION
// ===================================================================

void initLedManager() {
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
    dualLedState.lastUpdate = measureStart();
    dualLedState.step = 0;
    dualLedState.brightness = 255;
    dualLedState.user_led_state = false;
    dualLedState.charge_led_detected = false;
}

void setUserLed(rgb_color_t color, bool show) {
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

void setRgbLed(rgb_color_t color, bool show) {
#ifdef RGB_LED_ENABLED
    leds[0] = CRGB(color.r, color.g, color.b);
    if (show) FastLED.show();
#endif
}

void setDualLedColors(rgb_color_t primary, rgb_color_t secondary, bool show) {
    setUserLed(primary, show);
    
    if (dualLedState.mode == DUAL_LED_MODE_RGB_ENHANCED) {
        setRgbLed(secondary, show);
    }
}

void setLedPattern(led_pattern_t pattern, rgb_color_t primary_color, rgb_color_t secondary_color) {
    dualLedState.pattern = pattern;
    dualLedState.primary_color = primary_color;
    dualLedState.secondary_color = secondary_color;
    dualLedState.step = 0;
    dualLedState.lastUpdate = measureStart();
}

void updateLed() {
    if (!dualLedState.enabled) return;
    
    unsigned long elapsed = getElapsedTime(dualLedState.lastUpdate);
    
    switch (dualLedState.pattern) {
        case LED_OFF:
            setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
            break;
            
        case LED_ON:
            setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
            break;
            
        case LED_BLINK_SLOW:
            if (elapsed > TIMING_MEDIUM) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = measureStart();
            }
            break;
            
        case LED_BLINK_FAST:
            if (elapsed > TIMING_SHORT) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = measureStart();
            }
            break;
            
        case LED_BLINK_VERY_FAST:
            if (elapsed > TIMING_VERY_SHORT) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = measureStart();
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
                dualLedState.lastUpdate = measureStart();
            }
            break;
            
        case LED_HEARTBEAT:
            if (elapsed > TIMING_SHORT) {
                int pattern[] = {1, 0, 1, 0, 0, 0, 0, 0};
                int index = dualLedState.step % 8;
                if (pattern[index]) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = measureStart();
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
                dualLedState.lastUpdate = measureStart();
            }
            break;
            
        case LED_STARTUP:
            if (elapsed > TIMING_SHORT) {
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
                dualLedState.lastUpdate = measureStart();
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
                dualLedState.lastUpdate = measureStart();
            }
            break;
            
        default:
            // Default to simple blink
            if (elapsed > TIMING_MEDIUM) {
                if (dualLedState.step % 2 == 0) {
                    setDualLedColors(dualLedState.primary_color, dualLedState.secondary_color);
                } else {
                    setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
                }
                dualLedState.step++;
                dualLedState.lastUpdate = measureStart();
            }
            break;
    }
}

void setLedEnabled(bool enabled) {
    dualLedState.enabled = enabled;
    if (!enabled) {
        setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
    }
}

led_pattern_t getCurrentLedPattern() {
    return dualLedState.pattern;
}

dual_led_mode_t getCurrentLedMode() {
    return dualLedState.mode;
}

void flashDualLed(rgb_color_t primary, rgb_color_t secondary, int duration_ms) {
    static unsigned long flashStartTime = 0;
    static bool flashActive = false;
    
    if (!flashActive) {
        // Start flash
        setDualLedColors(primary, secondary);
        flashStartTime = measureStart();
        flashActive = true;
    } else if (getElapsedTime(flashStartTime) >= duration_ms) {
        // End flash
        setDualLedColors((rgb_color_t)LED_COLOR_OFF, (rgb_color_t)LED_COLOR_OFF);
        flashActive = false;
    }
}

void setLedForDeviceStatus(uint8_t status) {
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

// ===================================================================
// CONVENIENCE FUNCTIONS
// ===================================================================

void setLedPatternDualRed() {
    setLedPattern(LED_DUAL_INDICATION, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_MAROON);
}

void setLedPatternDualGreen() {
    setLedPattern(LED_DUAL_INDICATION, (rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_LIME);
}

void setLedPatternDualBlue() {
    setLedPattern(LED_DUAL_INDICATION, (rgb_color_t)LED_COLOR_BLUE, (rgb_color_t)LED_COLOR_CYAN);
}

void setLedPatternStreaming() {
    setLedPattern(LED_PULSE, (rgb_color_t)LED_COLOR_BLUE, (rgb_color_t)LED_COLOR_CYAN);
}

void setLedPatternPhotoCapture() {
    flashDualLed((rgb_color_t)LED_COLOR_WHITE, (rgb_color_t)LED_COLOR_SILVER, TIMING_VERY_SHORT);
}

void setLedPatternBatteryLow() {
    setLedPattern(LED_HEARTBEAT, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_ORANGE);
}

void setLedPatternConnected() {
    setLedPattern(LED_CONNECTED, (rgb_color_t)LED_COLOR_GREEN, (rgb_color_t)LED_COLOR_BLUE);
}

void setLedPatternDisconnected() {
    setLedPattern(LED_DISCONNECTED, (rgb_color_t)LED_COLOR_RED, (rgb_color_t)LED_COLOR_MAROON);
} 