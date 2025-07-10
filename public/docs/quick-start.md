# Quick Start Guide

Get your ESP32-S3 Camera Firmware up and running in minutes!

## 🎯 Prerequisites

### Hardware Required
- **Seeed Studio XIAO ESP32-S3 Sense** development board
- **USB-C cable** for programming and power
- **Lithium battery** (optional, for portable operation)
- **Computer** with Arduino IDE or VS Code

### Software Required
- **Arduino IDE** 2.0+ or **VS Code** with PlatformIO
- **ESP32 Arduino Core** 2.0.17
- **Git** (for cloning the repository)

## 🚀 5-Minute Setup

### Step 1: Clone the Repository
```bash
git clone https://github.com/ebowwa/esp32s3-cam-firmware.git
cd esp32s3-cam-firmware
```

### Step 2: Install ESP32 Board Support
In Arduino IDE:
1. Go to **File → Preferences**
2. Add this URL to **Additional Board Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search for "ESP32" and install **ESP32 by Espressif Systems** version **2.0.17**

### Step 3: Configure Board Settings
1. Select **Tools → Board → ESP32 Arduino → XIAO_ESP32S3**
2. Configure these settings:
   - **PSRAM**: "OPI PSRAM"
   - **Partition Scheme**: "Huge APP (3MB No OTA/1MB SPIFFS)"
   - **Upload Speed**: "921600"
   - **CPU Frequency**: "240MHz (WiFi/BT)"

### Step 4: Connect Hardware
1. Connect XIAO ESP32-S3 to your computer via USB-C
2. Select the correct **Port** in Arduino IDE
3. Optional: Connect lithium battery to JST connector

### Step 5: Upload Firmware
1. Open `firmware.ino` in Arduino IDE
2. Click **Upload** button (or Ctrl+U)
3. Wait for compilation and upload to complete

## ✅ Verify Installation

### Test 1: Serial Monitor
1. Open **Tools → Serial Monitor**
2. Set baud rate to **921600**
3. You should see:
   ```
   OpenGlass starting up...
   Wake-up reason: Reset or power-on
   Power management initialized
   BLE configured
   Camera configured
   OpenGlass ready!
   ```

### Test 2: BLE Advertisement
1. Use a BLE scanner app on your phone
2. Look for device named **"OpenGlass"**
3. You should see these services:
   - Device Information Service
   - Battery Service
   - Main OpenGlass Service

### Test 3: Battery Detection (if connected)
In Serial Monitor, you should see:
```
Battery detected and connected
Battery Voltage: 4.15V
Battery Level: 95%
```

## 🔧 Basic Usage

### Taking Photos
The firmware supports:
- **Single photo**: Triggered via BLE
- **Interval photos**: Configurable timing
- **Manual trigger**: Button press (if wired)

### Audio Recording
- **Real-time streaming** via BLE
- **Multiple codecs**: PCM, μ-law, Opus
- **Adjustable quality** and compression

### Power Management
- **Automatic optimization** based on battery level
- **Sleep modes** for power saving
- **Charging detection** and status reporting

## 🎛️ Configuration Options

### Audio Codec Selection
In `platform/constants.h`:
```cpp
#define CODEC_PCM          // Default
// #define CODEC_MULAW     // Compressed
// #define CODEC_OPUS      // High quality (requires libraries)
```

### Camera Settings
```cpp
#define CAMERA_JPEG_QUALITY 10        // 1-63 (lower = better quality)
#define CAMERA_FRAME_SIZE_HIGH FRAMESIZE_UXGA
#define CAMERA_FRAME_SIZE_LOW FRAMESIZE_SVGA
```

### Power Management
```cpp
#define BATTERY_UPDATE_INTERVAL 60000  // 60 seconds
#define POWER_IDLE_TIMEOUT_MS 300000   // 5 minutes
```

## 📱 Client Applications

### Compatible Apps
- **OpenGlass mobile app** (recommended)
- **nRF Connect** (for testing)
- **LightBlue** (iOS/Android)
- **Custom applications** using BLE

### BLE Service UUIDs
- **Main Service**: `19B10000-E8F2-537E-4F6C-D104768A1214`
- **Audio Data**: `19B10001-E8F2-537E-4F6C-D104768A1214`
- **Photo Data**: `19B10005-E8F2-537E-4F6C-D104768A1214`
- **Photo Control**: `19B10006-E8F2-537E-4F6C-D104768A1214`

## 🔍 Testing Examples

### Run Verification Tests
1. Open `examples/verify_constants.ino`
2. Upload and run to verify hardware constants
3. Check Serial Monitor for test results

### Test Timing Utilities
1. Open `examples/test_timing_utils.ino`
2. Upload and run to test timing functions
3. Observe timing measurements and delays

### Test Power Management
1. Open `examples/test_power_management.ino`
2. Upload and run to test power features
3. Monitor power consumption estimates

## ⚠️ Common Issues

### Upload Fails
- **Check cable**: Ensure USB-C cable supports data
- **Reset board**: Hold BOOT button while connecting
- **Driver issues**: Install CH340 drivers if needed

### No Serial Output
- **Baud rate**: Must be 921600
- **Cable connection**: Ensure good USB connection
- **Board selection**: Verify XIAO_ESP32S3 is selected

### BLE Not Visible
- **Reset device**: Power cycle the board
- **Clear cache**: Clear Bluetooth cache on phone
- **Range**: Ensure device is within 10 meters

### Camera Not Working
- **Connections**: Verify camera module connections
- **Power**: Ensure adequate power supply
- **Compatibility**: Use OV2640 or OV5640 modules

## 📚 Next Steps

### Learn More
- [**API Reference**](api-reference.md) - Function documentation
- [**Architecture**](architecture.md) - Code structure
- [**Utilities Guide**](utilities.md) - Using built-in utilities

### Customize
- [**Customization**](customization.md) - Adapt for your needs
- [**Pin Mappings**](pin-mappings.md) - GPIO configurations
- [**Power Optimization**](power-optimization.md) - Battery life

### Advanced
- [**Performance Tuning**](performance-tuning.md) - Optimization
- [**Contributing**](contributing.md) - Join development
- [**Troubleshooting**](troubleshooting.md) - Detailed problem solving

## 🎉 Success!

If you've made it this far, your ESP32-S3 Camera Firmware should be running successfully! 

**What's working:**
- ✅ Firmware compiled and uploaded
- ✅ Serial output showing startup messages
- ✅ BLE advertising active
- ✅ Camera and audio systems initialized
- ✅ Power management active

**Ready for:** Photo capture, audio streaming, BLE communication, and battery monitoring!

---

**Need help?** Check the [Troubleshooting Guide](troubleshooting.md) or open an issue on GitHub. 