# ESP32-S3 Camera Firmware

This firmware is designed for the **Seeed Studio XIAO ESP32-S3** development board using the **Espressif ESP32 Arduino Core 2.0.17**.

## Hardware Specifications

The firmware leverages the XIAO ESP32-S3's capabilities:
- **Processor**: ESP32-S3R8 dual-core @ 240MHz
- **Memory**: 8MB PSRAM + 8MB Flash
- **Connectivity**: Wi-Fi 2.4GHz + Bluetooth 5.0 LE
- **GPIO**: 11 digital pins with ADC, PWM, I2C, SPI, UART support

## Using arduino-cli

### Install the board

```bash
arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core install esp32:esp32@2.0.17
```

### ESP32 Arduino Core 2.0.17 Features

This firmware is tested with ESP32 Arduino Core 2.0.17, which includes:
- Improved BLE stability and performance
- Better camera support for ESP32-S3
- Enhanced power management
- Optimized PSRAM usage
- Bug fixes for I2S audio processing

### Get board details

On Windows 11 board should be showing as ```esp32:esp32:XIAO_ESP32S3```
but instead might show as ```esp32:esp32:nora_w10```, ```esp32:esp32:wifiduino32c3```, or something else.

```bash
arduino-cli board list
arduino-cli board details -b esp32:esp32:XIAO_ESP32S3
```

### Compile and upload

Change COM5 to the port name from the board list output

```bash
arduino-cli compile --build-path ../build --output-dir ../dist -e -u -p COM5 -b esp32:esp32:XIAO_ESP32S3:PSRAM=opi
```

### Opus support

Go to your Arduino libraries folder.

You can get the libraries folder location with the following command:

```bash
arduino-cli config get directories.user
```

Note: You have to add ```/libraries``` to the path to get the libraries folder.

Then clone the two libraries needed to add Opus support:

```bash
git clone https://github.com/pschatzmann/arduino-libopus.git
git clone https://github.com/pschatzmann/arduino-audio-tools.git
```

## Hardware Constants

The firmware uses centralized hardware constants defined in `xiao_esp32s3_constants.h`:

### Key Constants Used:
- **GPIO Pins**: All pin definitions use named constants (e.g., `XIAO_ESP32S3_PIN_A0`)
- **ADC Configuration**: 12-bit resolution, 3.3V reference
- **Memory**: 8MB Flash, 8MB PSRAM specifications
- **Power**: Operating voltages and current consumption limits
- **Camera**: GPIO pin mappings for OV2640/OV5640 sensor
- **Audio**: I2S pin configurations for microphone

### Pin Mappings:
- **A0 (GPIO2)**: Battery voltage sensing
- **A1 (GPIO3)**: USB power detection
- **GPIO41/42**: I2S microphone (SCK/WS)
- **GPIO10**: Camera XCLK
- **GPIO40/39**: Camera I2C (SDA/SCL)
- **GPIO13-18, 38, 47-48**: Camera data and control pins

## Battery Detection

The firmware includes automatic lithium battery detection functionality to ensure the device is properly powered and can report accurate battery status.

### How it works

The system uses ADC pin A0 to read the battery voltage and determine if a lithium battery is connected:

- **Voltage Range**: Detects batteries between 3.0V - 4.3V
- **Sampling**: Takes 10 samples and averages them for accuracy
- **Status Reporting**: Updates device status via BLE when battery state changes

### Device Status Values

The device reports battery status through the Device Status BLE characteristic:

- `0x03` (READY) - Battery detected and device ready
- `0x08` (BATTERY_NOT_DETECTED) - No battery detected or voltage out of range

### Battery Level Calculation

When a battery is detected, the firmware estimates the charge level based on a typical Li-ion discharge curve:

- **4.15V+**: 95-100% charge
- **4.0V-4.15V**: 75-95% charge  
- **3.8V-4.0V**: 50-75% charge
- **3.6V-3.8V**: 25-50% charge
- **3.4V-3.6V**: 10-25% charge
- **3.2V-3.4V**: 5-10% charge
- **3.0V-3.2V**: 0-5% charge

### Calibration

The voltage divider ratio may need adjustment based on your specific hardware. Check the serial output for debugging information:

```
ADC raw: 2048, ADC voltage: 1.65V, Battery voltage: 3.30V
```

If the battery voltage reading doesn't match your multimeter reading:
- **Too high**: Decrease `BATTERY_VOLTAGE_DIVIDER` in `constants.h`
- **Too low**: Increase `BATTERY_VOLTAGE_DIVIDER` in `constants.h`

### Testing Battery Detection

Use the included test script to verify battery detection:

```bash
python3 test_battery_detection.py
```

This script will:
1. Scan for OpenGlass devices
2. Connect via BLE
3. Monitor device status and battery level
4. Report when battery is connected/disconnected

### Hardware Notes

- The XIAO ESP32S3 Sense can be powered via USB or battery
- Battery detection works by measuring voltage on the ADC pin
- If no battery is detected, the device will still function on USB power
- Battery detection is checked during startup and periodically during operation

## Charging Detection

The firmware includes USB charging detection to monitor power source:

### Circuit Requirements:
- **5V pin** → **100kΩ resistor** → **A1 pin** → **47kΩ resistor** → **GND**
- This creates a voltage divider: 5V × (47kΩ / 147kΩ) ≈ 1.6V

### Status Indicators:
- `0x09` (CHARGING) - USB power detected with battery voltage > 4.1V
- `0x0A` (BATTERY_UNSTABLE) - Connection issues detected

The system monitors for:
- USB power presence via voltage divider
- Battery voltage stability over time
- Rapid voltage changes indicating connection issues


TODO: 
    HAL 
        - Additional Interfaces: e-ink, LED(s)
        - Sensors
        - Haptic
        - Storage/Memory

    FIRMWARE
        - BLE Mesh
        - Toggle-able features
        - ML Battery Optimization
        - Hotspot Security
        - BLE Security

    USE CASES:
    - ML TRAININD DATA COLLECTIONS
    - ROBOTICS
    - CAD
    - 3D SCANNING
    - Agriculture
    - Space Tech
    - ACCELERATION
