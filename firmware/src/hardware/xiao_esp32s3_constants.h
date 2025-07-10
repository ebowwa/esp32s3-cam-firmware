#ifndef XIAO_ESP32S3_CONSTANTS_H
#define XIAO_ESP32S3_CONSTANTS_H

// ===================================================================
// SEEED STUDIO XIAO ESP32-S3 CHIP CONSTANTS
// ===================================================================

// Chip Information
#define XIAO_ESP32S3_CHIP_MODEL "ESP32-S3R8"
#define XIAO_ESP32S3_ARCHITECTURE "Xtensa LX7"
#define XIAO_ESP32S3_CORES 2
#define XIAO_ESP32S3_MAX_FREQ_MHZ 240
#define XIAO_ESP32S3_BOARD_NAME "XIAO_ESP32S3"

// Memory Configuration
#define XIAO_ESP32S3_FLASH_SIZE_MB 8
#define XIAO_ESP32S3_FLASH_SIZE_BYTES (8 * 1024 * 1024)
#define XIAO_ESP32S3_PSRAM_SIZE_MB 8
#define XIAO_ESP32S3_PSRAM_SIZE_BYTES (8 * 1024 * 1024)
#define XIAO_ESP32S3_SRAM_SIZE_KB 512

// Board Dimensions (mm)
#define XIAO_ESP32S3_LENGTH_MM 21.0
#define XIAO_ESP32S3_WIDTH_MM 17.8
#define XIAO_ESP32S3_HEIGHT_MM 3.5

// Power Specifications
#define XIAO_ESP32S3_OPERATING_VOLTAGE_V 3.3
#define XIAO_ESP32S3_USB_INPUT_VOLTAGE_V 5.0
#define XIAO_ESP32S3_BATTERY_VOLTAGE_V 4.2
#define XIAO_ESP32S3_MAX_CURRENT_3V3_MA 700

// Power Consumption (mA)
#define XIAO_ESP32S3_DEEP_SLEEP_UA 14
#define XIAO_ESP32S3_LIGHT_SLEEP_MA 2
#define XIAO_ESP32S3_MODEM_SLEEP_MA 25
#define XIAO_ESP32S3_WIFI_ACTIVE_MA 100
#define XIAO_ESP32S3_BLE_ACTIVE_MA 85
#define XIAO_ESP32S3_IDLE_READY_MA 19

// GPIO Configuration
#define XIAO_ESP32S3_TOTAL_GPIO_PINS 11
#define XIAO_ESP32S3_ADC_PINS 9
#define XIAO_ESP32S3_PWM_PINS 11
#define XIAO_ESP32S3_INTERRUPT_PINS 11

// Pin Definitions (GPIO Numbers)
#define XIAO_ESP32S3_PIN_A0 2    // GPIO2 - ADC1_CH1
#define XIAO_ESP32S3_PIN_A1 3    // GPIO3 - ADC1_CH2
#define XIAO_ESP32S3_PIN_A2 4    // GPIO4 - ADC1_CH3
#define XIAO_ESP32S3_PIN_A3 5    // GPIO5 - ADC1_CH4
#define XIAO_ESP32S3_PIN_SDA 6   // GPIO6 - I2C SDA
#define XIAO_ESP32S3_PIN_SCL 7   // GPIO7 - I2C SCL
#define XIAO_ESP32S3_PIN_SCK 8   // GPIO8 - SPI SCK
#define XIAO_ESP32S3_PIN_MISO 9  // GPIO9 - SPI MISO
#define XIAO_ESP32S3_PIN_MOSI 10 // GPIO10 - SPI MOSI
#define XIAO_ESP32S3_PIN_RX 44   // GPIO44 - UART RX
#define XIAO_ESP32S3_PIN_TX 43   // GPIO43 - UART TX

// Additional Pins (Sense Version)
#define XIAO_ESP32S3_SENSE_PIN_D11 42 // GPIO42 - Additional GPIO
#define XIAO_ESP32S3_SENSE_PIN_D12 41 // GPIO41 - Additional GPIO

// Built-in LEDs
#define XIAO_ESP32S3_USER_LED_PIN 21
#define XIAO_ESP32S3_CHARGE_LED_PIN -1 // Hardware controlled

// USB Configuration
#define XIAO_ESP32S3_USB_DP_PIN 20  // USB D+
#define XIAO_ESP32S3_USB_DM_PIN 19  // USB D-

// Wireless Capabilities
#define XIAO_ESP32S3_WIFI_STANDARD "802.11 b/g/n"
#define XIAO_ESP32S3_WIFI_FREQUENCY_GHZ 2.4
#define XIAO_ESP32S3_BLUETOOTH_VERSION "5.0 LE"
#define XIAO_ESP32S3_BLUETOOTH_CLASSIC false
#define XIAO_ESP32S3_BLUETOOTH_LE true

// Communication Interfaces
#define XIAO_ESP32S3_UART_COUNT 1
#define XIAO_ESP32S3_I2C_COUNT 1
#define XIAO_ESP32S3_SPI_COUNT 1
#define XIAO_ESP32S3_I2S_COUNT 1

// Serial Communication
#define XIAO_ESP32S3_SERIAL_BAUD_RATE 921600     // High-speed serial for debugging
#define XIAO_ESP32S3_SERIAL_BUFFER_SIZE 256      // Default serial buffer size

// ADC Configuration
#define XIAO_ESP32S3_ADC_RESOLUTION_BITS 12
#define XIAO_ESP32S3_ADC_MAX_VALUE 4095
#define XIAO_ESP32S3_ADC_VREF_MV 3300

// Temperature Range
#define XIAO_ESP32S3_TEMP_MIN_C -40
#define XIAO_ESP32S3_TEMP_MAX_C 65

// Bootloader and Flash
#define XIAO_ESP32S3_BOOTLOADER_ADDR 0x0
#define XIAO_ESP32S3_FLASH_MODE "QIO"
#define XIAO_ESP32S3_FLASH_FREQ_MHZ 80
#define XIAO_ESP32S3_FLASH_SIZE_DETECT "8MB"

// Arduino Core Requirements
#define XIAO_ESP32S3_MIN_ARDUINO_CORE_VERSION "2.0.8"
#define XIAO_ESP32S3_RECOMMENDED_CORE_VERSION "3.0.0"

// Camera GPIO Pin Definitions (for OV2640/OV5640 camera module)
// These are always defined for the XIAO ESP32-S3 Sense version
#define XIAO_ESP32S3_CAMERA_PWDN_PIN -1      // Power down pin (not used)
#define XIAO_ESP32S3_CAMERA_RESET_PIN -1     // Reset pin (not used)
#define XIAO_ESP32S3_CAMERA_XCLK_PIN 10      // External clock pin
#define XIAO_ESP32S3_CAMERA_SIOD_PIN 40      // I2C SDA for camera control
#define XIAO_ESP32S3_CAMERA_SIOC_PIN 39      // I2C SCL for camera control

// Camera Data Pins (8-bit parallel interface)
#define XIAO_ESP32S3_CAMERA_Y9_PIN 48        // Data bit 7 (MSB)
#define XIAO_ESP32S3_CAMERA_Y8_PIN 11        // Data bit 6
#define XIAO_ESP32S3_CAMERA_Y7_PIN 12        // Data bit 5
#define XIAO_ESP32S3_CAMERA_Y6_PIN 14        // Data bit 4
#define XIAO_ESP32S3_CAMERA_Y5_PIN 16        // Data bit 3
#define XIAO_ESP32S3_CAMERA_Y4_PIN 18        // Data bit 2
#define XIAO_ESP32S3_CAMERA_Y3_PIN 17        // Data bit 1
#define XIAO_ESP32S3_CAMERA_Y2_PIN 15        // Data bit 0 (LSB)

// Camera Control Pins
#define XIAO_ESP32S3_CAMERA_VSYNC_PIN 38     // Vertical sync
#define XIAO_ESP32S3_CAMERA_HREF_PIN 47      // Horizontal reference
#define XIAO_ESP32S3_CAMERA_PCLK_PIN 13      // Pixel clock

// Built-in LED for camera flash (if needed)
#define XIAO_ESP32S3_CAMERA_LED_PIN 21       // Same as user LED

// Sense Version Specific Constants
#ifdef XIAO_ESP32S3_SENSE
  #define XIAO_ESP32S3_HAS_CAMERA true
  #define XIAO_ESP32S3_HAS_MICROPHONE true
  #define XIAO_ESP32S3_HAS_SD_CARD true
  #define XIAO_ESP32S3_CAMERA_MODEL "OV2640/OV3660"
  #define XIAO_ESP32S3_CAMERA_RESOLUTION "1600x1200/2048x1536"
  #define XIAO_ESP32S3_SD_CARD_MAX_SIZE_GB 32
#else
  #define XIAO_ESP32S3_HAS_CAMERA false
  #define XIAO_ESP32S3_HAS_MICROPHONE false
  #define XIAO_ESP32S3_HAS_SD_CARD false
#endif

// Plus Version Specific Constants
#ifdef XIAO_ESP32S3_PLUS
  #define XIAO_ESP32S3_FLASH_SIZE_MB 16
  #define XIAO_ESP32S3_FLASH_SIZE_BYTES (16 * 1024 * 1024)
  #define XIAO_ESP32S3_UART_COUNT 2
  #define XIAO_ESP32S3_SPI_COUNT 2
  #define XIAO_ESP32S3_TOTAL_GPIO_PINS 18
#endif

// Manufacturer Information
#define XIAO_ESP32S3_MANUFACTURER "Seeed Studio"
#define XIAO_ESP32S3_SERIES "XIAO"
#define XIAO_ESP32S3_PRODUCT_ID "113991114"

// Utility Macros
#define XIAO_ESP32S3_DIGITAL_PIN(pin) (pin)
#define XIAO_ESP32S3_ANALOG_PIN(pin) (pin)
#define XIAO_ESP32S3_IS_VALID_GPIO(pin) ((pin >= 0) && (pin <= 48))
#define XIAO_ESP32S3_IS_ADC_PIN(pin) ((pin >= 2 && pin <= 5) || (pin >= 6 && pin <= 10))

// Error Codes
#define XIAO_ESP32S3_ERROR_NONE 0
#define XIAO_ESP32S3_ERROR_INVALID_PIN -1
#define XIAO_ESP32S3_ERROR_PIN_IN_USE -2
#define XIAO_ESP32S3_ERROR_INSUFFICIENT_POWER -3

#endif // XIAO_ESP32S3_CONSTANTS_H 