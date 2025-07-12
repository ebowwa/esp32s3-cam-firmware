# ESP32-S3 Camera Firmware

A camera firmware project for the **Seeed Studio XIAO ESP32-S3** development board with battery management, BLE connectivity, and camera functionality.

## Hardware Requirements

- [Seeed Studio XIAO ESP32 S3 Sense](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5639.html)
- Compatible Li-ion battery (optional)
- Camera module (OV2640/OV5640)

## Features

- **Camera Support**: Image capture and streaming
- **Battery Management**: Voltage monitoring and charge level detection
- **Power Management**: USB charging detection and power optimization
- **BLE Connectivity**: Bluetooth Low Energy for device communication
- **LED Management**: Status indication and user feedback
- **Hardware Abstraction**: Centralized pin definitions and constants

## Getting Started

### Hardware Setup

1. Connect your XIAO ESP32-S3 board
2. Optional: Connect a compatible Li-ion battery
3. Ensure camera module is properly connected

### Software Setup

1. Install Arduino IDE or arduino-cli
2. Add ESP32 board package:
   ```bash
   arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   arduino-cli core install esp32:esp32@2.0.17
   ```

3. Compile and upload:
   ```bash
   cd firmware
   arduino-cli compile --build-path ../build --output-dir ../dist -e -u -p COM5 -b esp32:esp32:XIAO_ESP32S3:PSRAM=opi
   ```

### Firmware Details

See the [firmware README](firmware/readme.md) for detailed build instructions, hardware specifications, and feature documentation.

## Project Structure

- `firmware/` - Main Arduino firmware code
- `public/` - Documentation, examples, and test files
- `LICENSE` - MIT License

## Development

This project uses:
- **ESP32 Arduino Core 2.0.17**
- **PSRAM**: 8MB OPI PSRAM configuration
- **Flash**: 8MB storage
- **Connectivity**: Wi-Fi 2.4GHz + Bluetooth 5.0 LE

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
# esp32s3-cam-firmware
