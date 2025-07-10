# ESP32-S3 Camera Firmware Documentation

Welcome to the comprehensive documentation for the ESP32-S3 Camera Firmware project. This documentation covers everything you need to know about building, deploying, and extending the firmware.

## 📚 Documentation Index

### Getting Started
- [**Hardware Setup**](hardware-setup.md) - Hardware requirements and wiring
- [**Quick Start Guide**](quick-start.md) - Get up and running in minutes
- [**Installation**](installation.md) - Development environment setup

### Development
- [**API Reference**](api-reference.md) - Complete function and constant reference
- [**Architecture**](architecture.md) - Firmware structure and design patterns
- [**Utilities Guide**](utilities.md) - Using timing, power, and audio utilities

### Hardware
- [**XIAO ESP32-S3 Specs**](hardware-specs.md) - Complete hardware specifications
- [**Pin Mappings**](pin-mappings.md) - GPIO pin assignments and configurations
- [**Camera Integration**](camera-integration.md) - Camera module setup and usage

### Power Management
- [**Power Optimization**](power-optimization.md) - Battery life and power saving
- [**Sleep Modes**](sleep-modes.md) - Deep sleep and light sleep configuration
- [**Battery Monitoring**](battery-monitoring.md) - Battery level and charging detection

### Connectivity
- [**BLE Communication**](ble-communication.md) - Bluetooth Low Energy implementation
- [**Audio Streaming**](audio-streaming.md) - Audio capture and compression
- [**Photo Transfer**](photo-transfer.md) - Image capture and BLE transmission

### Debugging & Monitoring
- [**LED Patterns**](led-patterns.md) - Dual LED status indication and debugging
- [**Device Status**](device-status.md) - System status monitoring and codes

### Examples & Testing
- [**Example Sketches**](examples.md) - Test and verification sketches
- [**Troubleshooting**](troubleshooting.md) - Common issues and solutions
- [**Testing Guide**](testing.md) - How to test firmware functionality

### Advanced Topics
- [**Customization**](customization.md) - Adapting firmware for your needs
- [**Performance Tuning**](performance-tuning.md) - Optimization techniques
- [**Contributing**](contributing.md) - How to contribute to the project

## 🚀 Quick Navigation

| Topic | Description | Link |
|-------|-------------|------|
| **First Time Setup** | Hardware + Software setup | [Quick Start](quick-start.md) |
| **API Functions** | Complete function reference | [API Reference](api-reference.md) |
| **Power Saving** | Battery optimization guide | [Power Optimization](power-optimization.md) |
| **Camera Usage** | Photo capture and streaming | [Camera Integration](camera-integration.md) |
| **BLE Protocol** | Communication implementation | [BLE Communication](ble-communication.md) |
| **LED Debugging** | Visual status indication | [LED Patterns](led-patterns.md) |
| **Common Issues** | Troubleshooting guide | [Troubleshooting](troubleshooting.md) |

## 📋 Project Overview

This firmware is designed for the **Seeed Studio XIAO ESP32-S3 Sense** development board and provides:

- **Camera capture** with OV2640/OV5640 support
- **Audio recording** with I2S microphone
- **BLE communication** for data streaming
- **Battery management** with charging detection
- **Power optimization** for extended battery life
- **Dual LED status indication** for debugging and monitoring
- **Modular architecture** for easy customization

## 🔧 Key Features

### Hardware Support
- ✅ XIAO ESP32-S3 Sense (primary target)
- ✅ OV2640/OV5640 camera modules
- ✅ I2S microphone (PDM)
- ✅ Lithium battery with charging
- ✅ BLE 5.0 connectivity

### Firmware Capabilities
- 📷 **Photo Capture**: Single shot and interval capture
- 🎵 **Audio Recording**: Multiple codec support (PCM, μ-law, Opus)
- 🔋 **Battery Monitoring**: Voltage, level, and charging status
- ⚡ **Power Management**: Multiple power modes and sleep states
- 📡 **BLE Streaming**: Real-time data transmission
- 💡 **LED Debugging**: Dual LED status indication for crash detection
- 🛠️ **Utilities**: Timing, power, and debugging tools

## 🎯 Use Cases

This firmware is perfect for:
- **Wearable cameras** and action cameras
- **IoT surveillance** devices
- **Audio/video streaming** applications
- **Battery-powered** remote monitoring
- **Prototype development** for ESP32-S3 projects

## 📖 Documentation Standards

All documentation follows these standards:
- **Clear examples** with code snippets
- **Step-by-step instructions** for procedures
- **Troubleshooting sections** for common issues
- **Cross-references** between related topics
- **Version compatibility** notes where applicable

## 🤝 Community & Support

- **GitHub Issues**: Report bugs and request features
- **Discussions**: Ask questions and share ideas
- **Contributions**: Submit improvements and extensions
- **Examples**: Share your projects and modifications

---

**Next Steps**: Start with the [Quick Start Guide](quick-start.md) to get your firmware running, or jump to the [API Reference](api-reference.md) if you're ready to dive into development. 