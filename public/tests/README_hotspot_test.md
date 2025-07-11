# ESP32S3 Hotspot Functionality Test

This Python script comprehensively tests the hotspot functionality of the ESP32S3 camera firmware, including BLE integration features.

## Features Tested

### 🔧 Core Hotspot Functions
- **Hotspot Start/Stop** - Basic hotspot control via BLE commands
- **Status Monitoring** - Real-time hotspot status updates
- **WiFi Detection** - Verification that hotspot appears in WiFi scans
- **Toggle Functionality** - Hotspot on/off switching

### 🔗 BLE Integration Tests
- **BLE Connection Tracking** - Verifies BLE connection status in hotspot data
- **Data Statistics** - Tests combined BLE + WiFi data tracking
- **Interface Monitoring** - Validates active interface reporting
- **Command Tracking** - Confirms BLE command reception counting

## Requirements

### Python Dependencies
```bash
pip install bleak asyncio-mqtt pywifi
```

### Hardware Requirements
- ESP32S3 device with hotspot firmware loaded
- Computer with Bluetooth capability
- WiFi adapter (for network scanning)

## Usage

### Basic Test Run
```bash
python test_hotspot_functionality.py
```

### Expected Output
```
2024-01-15 10:30:00 - INFO - Starting ESP32S3 Hotspot Test Suite
2024-01-15 10:30:01 - INFO - Scanning for ESP32S3 device...
2024-01-15 10:30:02 - INFO - Found device: OpenGlass (XX:XX:XX:XX:XX:XX)
2024-01-15 10:30:03 - INFO - Connected to XX:XX:XX:XX:XX:XX
2024-01-15 10:30:04 - INFO - ✅ BLE Integration: PASSED - BLE connected, 1024 bytes transmitted, interfaces: 0x01
2024-01-15 10:30:07 - INFO - ✅ Hotspot Start: PASSED - Hotspot active with SSID: ESP32CAM-ABCD
2024-01-15 10:30:10 - INFO - ✅ Hotspot Status: PASSED - Status received: {...}
2024-01-15 10:30:13 - INFO - ✅ WiFi Detection: PASSED - Hotspot detected: ESP32CAM-ABCD
2024-01-15 10:30:16 - INFO - ✅ Hotspot Toggle: PASSED - Status changed from 2 to 0
2024-01-15 10:30:19 - INFO - ✅ Hotspot Stop: PASSED - Hotspot successfully stopped
```

## Test Sequence

1. **BLE Integration Test** - Verifies BLE connection data is properly integrated
2. **Hotspot Start Test** - Starts hotspot and verifies activation
3. **Status Request Test** - Requests and validates status data
4. **WiFi Detection Test** - Scans for hotspot in available networks
5. **Toggle Test** - Tests hotspot on/off switching
6. **Stop Test** - Stops hotspot and verifies deactivation

## BLE Protocol

### Control Commands
- `0x00` - Stop hotspot
- `0x01` - Start hotspot
- `0x02` - Toggle hotspot
- `0x03` - Get status update

### Status Response Format
```
Byte 0: Hotspot status (0=disabled, 1=starting, 2=active, 3=error)
Byte 1: Connected WiFi clients
Byte 2: Max WiFi clients
Byte 3: BLE connection status (0=disconnected, 1=connected)
Byte 4: Active interfaces bitmask (0x01=BLE, 0x02=WiFi)
Bytes 5-8: Total data transmitted (all interfaces, little endian)
Bytes 9-12: BLE data transmitted (little endian)
Byte 13: SSID length
Bytes 14+: SSID string
Byte X: IP address length
Bytes X+1: IP address string
```

## Output Files

### Log Files
- `hotspot_test.log` - Detailed test execution log
- `hotspot_test_results_YYYYMMDD_HHMMSS.json` - Structured test results

### Test Results JSON Structure
```json
{
  "timestamp": "2024-01-15T10:30:00",
  "tests_passed": 6,
  "tests_failed": 0,
  "test_details": [
    {
      "test_name": "BLE Integration",
      "passed": true,
      "details": "BLE connected, 1024 bytes transmitted, interfaces: 0x01",
      "timestamp": "2024-01-15T10:30:04"
    }
  ]
}
```

## Troubleshooting

### Common Issues

**Device Not Found**
- Ensure ESP32S3 is powered on and advertising
- Check device name is "OpenGlass"
- Verify Bluetooth is enabled on test computer

**Connection Failed**
- Device may already be connected to another client
- Try restarting ESP32S3 device
- Check Bluetooth permissions

**WiFi Scan Failed**
- `pywifi` library may not be compatible with your OS
- WiFi tests will be skipped automatically
- Manual verification can be done with system WiFi settings

**Status Parsing Errors**
- Firmware may be using different protocol version
- Check raw_data field in logs for debugging
- Verify BLE characteristics are properly implemented

## Integration with Firmware

This test script validates the BLE integration features added to the hotspot manager:

- **Connection Tracking** - BLE connect/disconnect events update hotspot statistics
- **Data Monitoring** - All BLE transmissions are tracked and reported
- **Command Counting** - BLE commands are counted for activity analysis
- **Combined Statistics** - Hotspot provides unified view of BLE + WiFi data

The enhanced hotspot manager provides comprehensive connectivity statistics that combine both BLE and WiFi data streams for better device monitoring and management. 