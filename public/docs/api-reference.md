# API Reference

Complete reference for all functions, constants, and data structures in the ESP32-S3 Camera Firmware.

## 📋 Table of Contents

- [**Hardware Constants**](#hardware-constants)
- [**Platform Constants**](#platform-constants)
- [**Timing Utilities**](#timing-utilities)
- [**Power Management**](#power-management)
- [**Battery Management**](#battery-management)
- [**Device Status**](#device-status)
- [**Audio Utilities**](#audio-utilities)
- [**LED Manager**](#led-manager)
- [**Camera Functions**](#camera-functions)
- [**BLE Services**](#ble-services)

---

## Hardware Constants

### XIAO ESP32-S3 Specifications
```cpp
// Chip Information
#define XIAO_ESP32S3_CHIP_MODEL "ESP32-S3R8"
#define XIAO_ESP32S3_ARCHITECTURE "Xtensa LX7"
#define XIAO_ESP32S3_CORES 2
#define XIAO_ESP32S3_MAX_FREQ_MHZ 240

// Memory Configuration
#define XIAO_ESP32S3_FLASH_SIZE_MB 8
#define XIAO_ESP32S3_PSRAM_SIZE_MB 8
#define XIAO_ESP32S3_SRAM_SIZE_KB 512
```

### GPIO Pin Definitions
```cpp
// Analog Pins
#define XIAO_ESP32S3_PIN_A0 2    // GPIO2 - ADC1_CH1
#define XIAO_ESP32S3_PIN_A1 3    // GPIO3 - ADC1_CH2
#define XIAO_ESP32S3_PIN_A2 4    // GPIO4 - ADC1_CH3
#define XIAO_ESP32S3_PIN_A3 5    // GPIO5 - ADC1_CH4

// Digital Pins
#define XIAO_ESP32S3_PIN_SDA 6   // GPIO6 - I2C SDA
#define XIAO_ESP32S3_PIN_SCL 7   // GPIO7 - I2C SCL
#define XIAO_ESP32S3_PIN_SCK 8   // GPIO8 - SPI SCK
#define XIAO_ESP32S3_PIN_MISO 9  // GPIO9 - SPI MISO
#define XIAO_ESP32S3_PIN_MOSI 10 // GPIO10 - SPI MOSI

// UART Pins
#define XIAO_ESP32S3_PIN_RX 44   // GPIO44 - UART RX
#define XIAO_ESP32S3_PIN_TX 43   // GPIO43 - UART TX

// Sense Version Additional Pins
#define XIAO_ESP32S3_SENSE_PIN_D11 42 // GPIO42 - I2S WS
#define XIAO_ESP32S3_SENSE_PIN_D12 41 // GPIO41 - I2S SCK
```

### Camera Pin Mappings
```cpp
// Camera GPIO Pins (XIAO ESP32-S3 Sense)
#define XIAO_ESP32S3_CAMERA_XCLK_PIN 10
#define XIAO_ESP32S3_CAMERA_SIOD_PIN 40
#define XIAO_ESP32S3_CAMERA_SIOC_PIN 39
#define XIAO_ESP32S3_CAMERA_Y9_PIN 48
#define XIAO_ESP32S3_CAMERA_Y8_PIN 11
#define XIAO_ESP32S3_CAMERA_Y7_PIN 12
#define XIAO_ESP32S3_CAMERA_Y6_PIN 14
#define XIAO_ESP32S3_CAMERA_Y5_PIN 16
#define XIAO_ESP32S3_CAMERA_Y4_PIN 18
#define XIAO_ESP32S3_CAMERA_Y3_PIN 17
#define XIAO_ESP32S3_CAMERA_Y2_PIN 15
#define XIAO_ESP32S3_CAMERA_VSYNC_PIN 38
#define XIAO_ESP32S3_CAMERA_HREF_PIN 47
#define XIAO_ESP32S3_CAMERA_PCLK_PIN 13
```

---

## Platform Constants

### Audio Configuration
```cpp
// Codec Selection (define one)
#define CODEC_PCM          // Default - uncompressed
#define CODEC_MULAW        // μ-law compression
#define CODEC_OPUS         // Opus compression (requires libraries)

// Audio Parameters
#define SAMPLE_RATE 16000       // 16 kHz sampling rate
#define SAMPLE_BITS 16          // 16-bit samples
#define FRAME_SIZE 160          // Samples per frame
#define VOLUME_GAIN 2           // Audio gain multiplier
```

### BLE Service UUIDs
```cpp
// Main Service
#define SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"

// Characteristics
#define AUDIO_DATA_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"
#define AUDIO_CODEC_UUID "19B10002-E8F2-537E-4F6C-D104768A1214"
#define PHOTO_DATA_UUID "19B10005-E8F2-537E-4F6C-D104768A1214"
#define PHOTO_CONTROL_UUID "19B10006-E8F2-537E-4F6C-D104768A1214"
#define DEVICE_STATUS_UUID "19B10007-E8F2-537E-4F6C-D104768A1214"

// Standard Services
#define DEVICE_INFORMATION_SERVICE_UUID 0x180A
#define BATTERY_SERVICE_UUID 0x180F
```

### Device Information
```cpp
#define MANUFACTURER_NAME "Based Hardware"
#define MODEL_NUMBER "OpenGlass"
#define FIRMWARE_VERSION "1.0.1"
#define HARDWARE_VERSION "XIAO ESP32-S3 Sense"
#define DEVICE_NAME "OpenGlass"
```

---

## Timing Utilities

### Timeout Functions
```cpp
bool hasTimedOut(unsigned long startTime, unsigned long timeout);
// Check if timeout occurred since startTime
// Returns: true if timeout occurred, false otherwise

unsigned long getElapsedTime(unsigned long startTime);
// Get elapsed time since startTime
// Returns: elapsed time in milliseconds

bool hasTimedOutMicros(unsigned long startTime, unsigned long timeout);
// Microsecond precision timeout check
// Returns: true if timeout occurred, false otherwise
```

### Non-blocking Delays
```cpp
bool nonBlockingDelayStateful(unsigned long* lastTime, unsigned long duration);
// Multi-instance non-blocking delay
// Parameters: lastTime - pointer to store last time, duration - delay in ms
// Returns: true if delay completed, false if still waiting

bool shouldExecute(unsigned long* lastTime, unsigned long interval);
// Rate limiting for periodic operations
// Parameters: lastTime - pointer to last execution time, interval - minimum interval
// Returns: true if enough time has passed, false otherwise
```

### Performance Measurement
```cpp
unsigned long measureStart();
// Start performance measurement
// Returns: current time in milliseconds

unsigned long measureEnd(unsigned long startTime);
// End performance measurement
// Parameters: startTime - time from measureStart()
// Returns: elapsed time in milliseconds

unsigned long measureStartMicros();
// Start microsecond precision measurement
// Returns: current time in microseconds

unsigned long measureEndMicros(unsigned long startTime);
// End microsecond precision measurement
// Returns: elapsed time in microseconds
```

### Timing Constants
```cpp
#define TIMING_IMMEDIATE 0
#define TIMING_VERY_SHORT 10      // 10ms
#define TIMING_SHORT 100          // 100ms
#define TIMING_MEDIUM 1000        // 1 second
#define TIMING_LONG 5000          // 5 seconds
#define TIMING_VERY_LONG 30000    // 30 seconds

#define TIMEOUT_QUICK 100         // 100ms
#define TIMEOUT_SHORT 1000        // 1 second
#define TIMEOUT_MEDIUM 5000       // 5 seconds
#define TIMEOUT_LONG 30000        // 30 seconds
#define TIMEOUT_VERY_LONG 60000   // 1 minute
```

---

## Power Management

### Power Modes
```cpp
typedef enum {
    POWER_MODE_PERFORMANCE,    // Maximum performance
    POWER_MODE_BALANCED,       // Balanced performance/power
    POWER_MODE_POWER_SAVE,     // Power saving mode
    POWER_MODE_ULTRA_LOW       // Ultra low power mode
} power_mode_t;
```

### Power Management Functions
```cpp
void initializePowerManagement();
// Initialize power management system
// Call once during setup

void setPowerMode(power_mode_t mode);
// Set power mode and adjust CPU frequency
// Parameters: mode - power mode to set

uint32_t getCurrentCpuFrequency();
// Get current CPU frequency
// Returns: CPU frequency in MHz

float estimateCurrentConsumption(bool wifi_active, bool ble_active, bool camera_active);
// Estimate current power consumption
// Parameters: component activity flags
// Returns: estimated current in mA
```

### Power Statistics
```cpp
typedef struct {
    float voltage;             // Battery voltage
    float current_ma;          // Current consumption in mA
    float power_mw;            // Power consumption in mW
    unsigned long timestamp;   // Timestamp of measurement
    power_mode_t mode;         // Current power mode
} power_stats_t;

void updatePowerStats(float battery_voltage, bool wifi_active, bool ble_active, bool camera_active);
// Update power statistics
// Parameters: battery voltage and component activity flags

power_stats_t getPowerStats();
// Get current power statistics
// Returns: power statistics structure

void printPowerStats();
// Print power statistics to serial output
```

### Sleep Functions
```cpp
void enterLightSleep(uint32_t duration_ms, wakeup_source_t wakeup_source, int wakeup_pin);
// Enter light sleep mode
// Parameters: duration in ms, wake-up source, wake-up pin (if applicable)

void enterDeepSleep(uint32_t duration_ms, wakeup_source_t wakeup_source, int wakeup_pin);
// Enter deep sleep mode (will reset on wake-up)
// Parameters: duration in ms, wake-up source, wake-up pin (if applicable)

const char* getWakeupReason();
// Get reason for wake-up from sleep
// Returns: string describing wake-up reason
```

### Battery Optimization
```cpp
void optimizePowerForBattery(uint8_t battery_level, bool is_charging);
// Optimize power settings based on battery level
// Parameters: battery level (0-100), charging status

bool shouldEnterPowerSaving(uint8_t battery_level, unsigned long idle_time_ms);
// Check if device should enter power saving mode
// Parameters: battery level, idle time in ms
// Returns: true if should enter power saving

float estimateBatteryLife(uint16_t battery_capacity_mah, uint8_t current_level);
// Estimate remaining battery life
// Parameters: battery capacity in mAh, current level (0-100)
// Returns: estimated life in hours
```

---

## Battery Management

### Battery Monitoring Functions
```cpp
void setupBatteryService(BLEServer *server);
// Initialize BLE battery service
// Parameters: BLE server instance

void updateBatteryLevel();
// Update and notify battery level via BLE
// Call periodically to update battery status

bool checkBatteryPresence();
// Check if battery is connected
// Returns: true if battery detected, false otherwise

float readBatteryVoltage();
// Read current battery voltage
// Returns: battery voltage in volts

bool checkChargingStatus();
// Check if device is charging
// Returns: true if charging, false otherwise
```

### Battery Status Variables
```cpp
extern uint8_t batteryLevel;           // Battery level (0-100)
extern bool batteryDetected;           // Battery presence flag
extern bool isCharging;                // Charging status flag
extern bool connectionStable;          // Connection stability flag
extern unsigned long lastBatteryUpdate; // Last update timestamp
```

### Battery Constants
```cpp
#define BATTERY_ADC_PIN XIAO_ESP32S3_PIN_A0      // GPIO2
#define USB_POWER_ADC_PIN XIAO_ESP32S3_PIN_A1    // GPIO3
#define BATTERY_UPDATE_INTERVAL 60000             // 60 seconds
#define BATTERY_STABILITY_SAMPLES 5               // Stability samples
```

---

## Device Status

### Device Status Functions
```cpp
void updateDeviceStatus(uint8_t status);
// Update and notify device status via BLE
// Parameters: status code

void setupDeviceStatusService(BLEService *service);
// Initialize device status BLE service
// Parameters: BLE service instance
```

### Device Status Codes
```cpp
#define DEVICE_STATUS_INITIALIZING 0x01
#define DEVICE_STATUS_WARMING_UP 0x02
#define DEVICE_STATUS_READY 0x03
#define DEVICE_STATUS_ERROR 0x04
#define DEVICE_STATUS_CAMERA_INIT 0x05
#define DEVICE_STATUS_MICROPHONE_INIT 0x06
#define DEVICE_STATUS_BLE_INIT 0x07
#define DEVICE_STATUS_BATTERY_NOT_DETECTED 0x08
#define DEVICE_STATUS_CHARGING 0x09
#define DEVICE_STATUS_BATTERY_UNSTABLE 0x0A
```

### Device Status Variables
```cpp
extern uint8_t deviceStatus;           // Current device status
extern bool deviceReady;               // Device ready flag
```

---

## Audio Utilities

### μ-law Compression
```cpp
unsigned char linear2ulaw(int pcm_val);
// Convert linear PCM to μ-law
// Parameters: 16-bit PCM value
// Returns: μ-law encoded byte

#define BIAS 0x84                      // μ-law bias constant
```

### Audio Configuration
```cpp
// Buffer Sizes (depend on codec)
#define RECORDING_BUFFER_SIZE 320      // Input buffer size
#define COMPRESSED_BUFFER_SIZE 403     // Output buffer size

// I2S Configuration
#define I2S_WS_PIN XIAO_ESP32S3_SENSE_PIN_D11   // GPIO42
#define I2S_SCK_PIN XIAO_ESP32S3_SENSE_PIN_D12  // GPIO41
```

---

## LED Manager

### Dual LED System
The LED Manager provides intelligent control over the two LEDs available on the XIAO ESP32-S3:
- **User LED** (GPIO21) - Fully programmable
- **Charge LED** (hardware controlled) - Automatic charging indication
- **External RGB LED** (optional) - Enhanced color support

### Initialization
```cpp
void initLedManager();
// Initialize the dual LED management system
// Call once during setup

void updateLed();
// Update LED state (call in main loop)
// Non-blocking, handles timing automatically
```

### LED Pattern Control
```cpp
void setLedPattern(led_pattern_t pattern, led_color_t primaryColor, led_color_t secondaryColor);
// Set LED pattern with dual colors
// Parameters: pattern type, primary LED color, secondary LED color

void setLedPattern(led_pattern_t pattern, led_color_t color);
// Set LED pattern with single color
// Parameters: pattern type, color for both LEDs

void setLedOff();
// Turn off all LEDs

void setLedOn(led_color_t color);
// Turn on LEDs with specified color
```

### Operating Modes
```cpp
typedef enum {
    DUAL_LED_MODE_SINGLE,      // Use only User LED (GPIO21)
    DUAL_LED_MODE_DUAL_BASIC,  // Use User LED + monitor Charge LED
    DUAL_LED_MODE_RGB_ENHANCED // Use User LED + external RGB LED
} dual_led_mode_t;

dual_led_mode_t getCurrentLedMode();
// Get current LED operating mode
// Returns: current mode
```

### LED Patterns
```cpp
typedef enum {
    LED_OFF,                   // All LEDs off
    LED_ON,                    // Solid color
    LED_BLINK_SLOW,           // 1Hz blink
    LED_BLINK_FAST,           // 5Hz blink
    LED_BLINK_VERY_FAST,      // 10Hz blink
    LED_PULSE,                // Breathing effect
    LED_HEARTBEAT,            // Double-pulse pattern
    LED_SOS,                  // Morse code SOS
    LED_STARTUP,              // Startup sequence
    LED_DUAL_INDICATION,      // Alternating between LEDs
    LED_ERROR,                // Error indication
    LED_CONNECTED,            // Connected status
    LED_DISCONNECTED,         // Disconnected status
    LED_CHARGING,             // Charging indication
    LED_LOW_BATTERY,          // Low battery warning
    LED_STREAMING,            // Data streaming
    LED_PHOTO_CAPTURE,        // Photo capture flash
    LED_FACTORY_RESET         // Factory reset sequence
} led_pattern_t;
```

### LED Colors
```cpp
typedef enum {
    LED_COLOR_OFF,            // {0, 0, 0}
    LED_COLOR_WHITE,          // {255, 255, 255}
    LED_COLOR_RED,            // {255, 0, 0}
    LED_COLOR_GREEN,          // {0, 255, 0}
    LED_COLOR_BLUE,           // {0, 0, 255}
    LED_COLOR_YELLOW,         // {255, 255, 0}
    LED_COLOR_PURPLE,         // {128, 0, 128}
    LED_COLOR_CYAN,           // {0, 255, 255}
    LED_COLOR_ORANGE,         // {255, 165, 0}
    LED_COLOR_PINK,           // {255, 192, 203}
    LED_COLOR_LIME,           // {50, 205, 50}
    LED_COLOR_INDIGO,         // {75, 0, 130}
    LED_COLOR_VIOLET,         // {238, 130, 238}
    LED_COLOR_GOLD,           // {255, 215, 0}
    LED_COLOR_SILVER,         // {192, 192, 192}
    LED_COLOR_MAROON,         // {128, 0, 0}
    LED_COLOR_NAVY,           // {0, 0, 128}
    LED_COLOR_TEAL,           // {0, 128, 128}
    LED_COLOR_OLIVE           // {128, 128, 0}
} led_color_t;
```

### Convenience Functions
```cpp
void setLedPatternDualRed();
// Quick red dual pattern (RED + MAROON)

void setLedPatternDualGreen();
// Quick green dual pattern (GREEN + LIME)

void setLedPatternDualBlue();
// Quick blue dual pattern (BLUE + CYAN)

void setLedPatternStreaming();
// Streaming indication (BLUE + CYAN pulse)

void setLedPatternPhotoCapture();
// Photo capture flash (WHITE + SILVER)

void setLedPatternBatteryLow();
// Battery low warning (RED + ORANGE heartbeat)

void setLedPatternConnected();
// Connected status (GREEN + BLUE pulse)

void setLedPatternDisconnected();
// Disconnected status (RED + MAROON blink)
```

### Advanced Functions
```cpp
void flashDualLed(led_color_t primaryColor, led_color_t secondaryColor, unsigned long duration);
// Flash both LEDs with specified colors for duration
// Parameters: primary color, secondary color, duration in ms

bool isLedPatternActive();
// Check if LED pattern is currently active
// Returns: true if pattern is running, false if static

void setLedBrightness(uint8_t brightness);
// Set LED brightness (RGB mode only)
// Parameters: brightness level (0-255)
```

### RGB LED Configuration (Optional)
```cpp
#define RGB_LED_ENABLED         // Enable external RGB LED support
#define RGB_LED_PIN 2           // GPIO pin for RGB LED
#define RGB_LED_COUNT 1         // Number of RGB LEDs
#define RGB_LED_TYPE WS2812B    // LED type
#define RGB_COLOR_ORDER GRB     // Color order
#define RGB_LED_BRIGHTNESS 50   // Default brightness (0-255)
```

---

## Camera Functions

### Camera Configuration
```cpp
void configure_camera();
// Initialize camera with predefined settings
// Call once during setup

bool take_photo();
// Capture a photo with retry logic
// Returns: true if successful, false if failed
```

### Camera Settings
```cpp
#define CAMERA_JPEG_QUALITY 10
#define CAMERA_FRAME_SIZE_HIGH FRAMESIZE_UXGA
#define CAMERA_FRAME_SIZE_LOW FRAMESIZE_SVGA
#define CAMERA_XCLK_FREQ 20000000
#define CAMERA_FB_COUNT 1
```

### Photo Control
```cpp
#define PHOTO_SINGLE_SHOT -1
#define PHOTO_STOP 0
#define PHOTO_MIN_INTERVAL 5
#define PHOTO_MAX_INTERVAL 300

#define PHOTO_CHUNK_SIZE 200
#define PHOTO_END_MARKER_LOW 0xFF
#define PHOTO_END_MARKER_HIGH 0xFF
```

---

## BLE Services

### Service Setup
```cpp
void configure_ble();
// Initialize all BLE services and characteristics
// Call once during setup
```

### Characteristic References
```cpp
extern BLECharacteristic *audioDataCharacteristic;
extern BLECharacteristic *photoDataCharacteristic;
extern BLECharacteristic *photoControlCharacteristic;
extern BLECharacteristic *batteryLevelCharacteristic;
extern BLECharacteristic *deviceStatusCharacteristic;
```

### Connection Management
```cpp
extern bool connected;                 // BLE connection status

class ServerHandler : public BLEServerCallbacks {
    void onConnect(BLEServer *server);
    void onDisconnect(BLEServer *server);
};
```

---

## Usage Examples

### Basic Timing
```cpp
// Timeout checking
unsigned long startTime = millis();
if (hasTimedOut(startTime, TIMEOUT_MEDIUM)) {
    // Handle timeout
}

// Performance measurement
unsigned long perfStart = measureStart();
// ... do work ...
unsigned long duration = measureEnd(perfStart);
```

### Power Management
```cpp
// Initialize power management
initializePowerManagement();

// Set power mode
setPowerMode(POWER_MODE_POWER_SAVE);

// Update power statistics
updatePowerStats(3.7, false, true, false);
printPowerStats();
```

### Battery Monitoring
```cpp
// Check battery
if (checkBatteryPresence()) {
    float voltage = readBatteryVoltage();
    bool charging = checkChargingStatus();
    updateBatteryLevel();
}
```

### Rate Limiting
```cpp
static unsigned long lastUpdate = 0;
if (shouldExecute(&lastUpdate, 1000)) {
    // Execute once per second
    updateSomething();
}
```

---

## Error Handling

### Return Values
- **Boolean functions**: `true` for success, `false` for failure
- **Numeric functions**: Valid values or `0`/`-1` for errors
- **Pointer functions**: Valid pointer or `nullptr` for errors

### Error Checking
```cpp
// Always check return values
if (!take_photo()) {
    Serial.println("Photo capture failed");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
}

// Check for null pointers
if (audioDataCharacteristic != nullptr) {
    audioDataCharacteristic->notify();
}
```

### Debug Output
All functions provide serial output for debugging. Set baud rate to **921600** for proper output.

---

**Note**: This API reference covers the current firmware version. Some functions may require specific hardware configurations or additional setup. Refer to the [Quick Start Guide](quick-start.md) for setup instructions and [Examples](examples.md) for usage demonstrations. 