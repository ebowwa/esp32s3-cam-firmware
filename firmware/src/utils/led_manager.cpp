#include "led_manager.h"

// Global dual LED state variable
dual_led_state_t dualLedState;

#ifdef RGB_LED_ENABLED
// RGB LED array for FastLED
CRGB leds[RGB_LED_COUNT];
#endif 