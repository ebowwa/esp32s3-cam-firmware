# ESP32S3 Charging System Test Script

This Python script tests the modular charging manager functionality of the ESP32S3 camera firmware via serial communication.

## Prerequisites

1. **Python 3.6+** installed on your system
2. **ESP32S3 device** with the firmware loaded and connected via USB
3. **pyserial** library installed

## Installation

1. Install the required Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Find your ESP32S3 device's serial port:
   - **Linux/macOS**: Usually `/dev/ttyUSB0`, `/dev/ttyACM0`, or `/dev/cu.usbserial-*`
   - **Windows**: Usually `COM3`, `COM4`, etc.

## Usage

### Run All Tests (Comprehensive)
```bash
python test_charging_serial.py --port /dev/ttyUSB0
```

### Run Specific Tests
```bash
# Test initialization only
python test_charging_serial.py --port /dev/ttyUSB0 --test init

# Test USB connection detection
python test_charging_serial.py --port /dev/ttyUSB0 --test usb

# Test charging status updates
python test_charging_serial.py --port /dev/ttyUSB0 --test status
```

### Save Results to File
```bash
python test_charging_serial.py --port /dev/ttyUSB0 --output test_results.json
```

### Custom Baud Rate
```bash
python test_charging_serial.py --port /dev/ttyUSB0 --baudrate 9600
```

## Available Tests

| Test | Description | Duration |
|------|-------------|----------|
| `init` | Tests charging manager initialization | 5 seconds |
| `status` | Tests charging status updates | 15 seconds |
| `usb` | Tests USB connection detection | 20 seconds |
| `safety` | Tests charging safety system | 10 seconds |
| `state` | Tests charging state machine | 15 seconds |
| `sensors` | Tests sensor readings | 10 seconds |
| `ble` | Tests BLE integration | 8 seconds |
| `all` | Runs all tests above | ~90 seconds |

## What the Tests Check

### 1. Initialization Test
- Verifies that all modular components initialize properly
- Looks for initialization messages from:
  - Charging Manager
  - Charging Sensors
  - Charging State Machine
  - Charging Safety System
  - Charging History
  - Charging BLE Interface

### 2. Status Updates Test
- Monitors for regular charging status updates
- Checks for voltage, current, and charge level readings
- Verifies USB connection status updates

### 3. USB Connection Test
- Tests USB power connection detection
- Monitors for USB connect/disconnect events
- Verifies voltage readings when USB is connected
- **Interactive**: Prompts you to connect/disconnect USB during test

### 4. Safety System Test
- Verifies charging safety system is active
- Looks for safety status messages
- Checks for safety-related warnings or errors

### 5. State Machine Test
- Tests charging state transitions
- Monitors for state change messages
- Verifies current charging state reporting

### 6. Sensor Readings Test
- Tests sensor data collection
- Verifies voltage, current, and temperature readings
- Checks for consistent sensor updates

### 7. BLE Integration Test
- Tests BLE charging service integration
- Looks for BLE-related messages
- Verifies BLE notifications and characteristics

## Example Output

```
🚀 Starting Comprehensive Charging System Test
============================================================

🔋 Testing Charging Manager Initialization...
📖 Reading serial output for 5 seconds...
📥 Initializing Modular Charging Manager...
📥 Charging Sensors initialized
📥 Charging State Machine initialized
✅ Initialization test: 3/6 components found

🔍 Testing Charging Status Updates...
📖 Reading serial output for 15 seconds...
📥 🔍 Modular charging status update...
📥 Voltage: 3.85 V
📥 Current: 245.3 mA
✅ Status updates test: 12 updates found

... (additional tests) ...

============================================================
📊 TEST SUMMARY
============================================================
Total Tests: 7
Passed: 6
Failed: 1
Success Rate: 85.7%
```

## Troubleshooting

### Connection Issues
- Ensure the ESP32S3 is properly connected via USB
- Check that the correct serial port is specified
- Try different baud rates if connection fails
- Make sure no other programs are using the serial port

### No Output Detected
- Verify the firmware is running on the ESP32S3
- Check that the charging manager is enabled in the firmware
- Ensure the serial monitor isn't already open in Arduino IDE

### Permission Errors (Linux/macOS)
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Or run with sudo (not recommended)
sudo python test_charging_serial.py --port /dev/ttyUSB0
```

### Test Failures
- Some tests may fail if the device is not actively charging
- USB connection test requires manual interaction
- BLE test may fail if BLE is not properly configured

## Advanced Usage

### Continuous Monitoring
```bash
# Run status test in a loop
while true; do
    python test_charging_serial.py --port /dev/ttyUSB0 --test status
    sleep 30
done
```

### Custom Test Duration
Modify the `read_serial_output()` duration parameter in the script for longer/shorter monitoring periods.

## Output Files

When using `--output`, the script saves detailed test results in JSON format:

```json
{
  "test": "charging_initialization",
  "timestamp": "2024-01-15T10:30:45.123456",
  "success": true,
  "found_messages": [
    "Initializing Modular Charging Manager",
    "Charging Sensors initialized"
  ],
  "total_output_lines": 25
}
```

This data can be used for automated testing, CI/CD pipelines, or detailed analysis of charging system behavior. 