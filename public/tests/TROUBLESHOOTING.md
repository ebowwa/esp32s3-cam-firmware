# ESP32S3 Charging Test Troubleshooting Guide

## Issue: No Serial Output Detected

### Symptoms
- Test scripts connect successfully but capture no output
- `debug_serial.py` shows "No output detected"
- Device appears connected but silent

### Possible Causes & Solutions

#### 1. Device Not Running / Needs Reset
**Cause**: The ESP32S3 firmware might not be running or is stuck
**Solution**: 
```bash
# Reset the device by pressing the reset button
# Or disconnect and reconnect USB power
# Wait 5-10 seconds after reconnecting before testing
```

#### 2. Device in Sleep Mode
**Cause**: Power management might have put the device into deep sleep
**Solution**:
```bash
# Try pressing the boot button to wake the device
# Or disconnect/reconnect USB power
# Check if device responds to physical interaction
```

#### 3. Serial Output Disabled
**Cause**: The firmware might not be configured to output to serial
**Solution**:
```bash
# Check if Serial.begin() is called in firmware setup()
# Verify XIAO_ESP32S3_SERIAL_BAUD_RATE is set correctly
# Ensure Serial.println() statements are present
```

#### 4. Wrong Baud Rate
**Cause**: The test script might be using wrong baud rate
**Solution**:
```bash
# Try different baud rates
python3 debug_serial.py /dev/ttyUSB0 10 --baudrate 9600
python3 debug_serial.py /dev/ttyUSB0 10 --baudrate 38400
python3 debug_serial.py /dev/ttyUSB0 10 --baudrate 115200
```

#### 5. USB Port Issues
**Cause**: USB connection might be unstable
**Solution**:
```bash
# Try different USB ports
# Check USB cable quality
# Look for connection messages in system logs:
# macOS: tail -f /var/log/system.log | grep -i usb
# Linux: dmesg | tail -20
```

#### 6. Permission Issues
**Cause**: No permission to access serial port
**Solution**:
```bash
# macOS: Usually not needed, but try:
sudo chmod 666 /dev/tty.usbmodem101

# Linux: Add user to dialout group:
sudo usermod -a -G dialout $USER
# Then logout and login again
```

## Testing Steps

### Step 1: Basic Connection Test
```bash
# Check if device is detected
ls -la /dev/tty.usbmodem* # macOS
ls -la /dev/ttyUSB* # Linux
ls -la /dev/ttyACM* # Linux alternative
```

### Step 2: Raw Serial Test
```bash
# Test raw serial connection
timeout 10 cat /dev/tty.usbmodem101
```

### Step 3: Reset and Monitor
```bash
# Start monitoring, then reset device
python3 debug_serial.py /dev/tty.usbmodem101 30 &
# Press reset button on ESP32S3
# Wait for initialization messages
```

### Step 4: Arduino IDE Serial Monitor
```bash
# Open Arduino IDE
# Tools > Serial Monitor
# Set baud rate to 115200
# Press reset button and look for output
```

### Step 5: Test Different Scenarios
```bash
# Test with USB power connected/disconnected
# Test with battery connected/disconnected
# Test while charging vs not charging
```

## Expected Output

When working correctly, you should see:

### During Initialization:
```
OpenGlass starting up...
Wake-up reason: ...
Initializing Power Management...
Initializing Modular Charging Manager...
Charging Sensors initialized
Charging State Machine initialized
Charging Safety System initialized
```

### During Operation:
```
🔍 Modular charging status update...
Voltage: 3.85 V
Current: 245.3 mA
USB Connected: Yes
State: NOT_CHARGING
```

### USB Events:
```
🔌 USB POWER CONNECTED - FLASHING GREEN!
USB voltage detected: 5.00V
🔌 USB power disconnected
```

## Common Issues

### Issue: "Permission denied" Error
```bash
# Solution: Fix permissions
sudo chmod 666 /dev/tty.usbmodem101
# Or add user to dialout group (Linux)
```

### Issue: "Device not found" Error
```bash
# Solution: Check connection
ls -la /dev/tty*usb*
# Try different USB port
# Check USB cable
```

### Issue: Garbled Output
```bash
# Solution: Wrong baud rate
# Try 9600, 38400, 115200, 230400
```

### Issue: Intermittent Output
```bash
# Solution: USB power issues
# Try powered USB hub
# Check for loose connections
```

## Advanced Debugging

### Check System Logs
```bash
# macOS
tail -f /var/log/system.log | grep -i usb

# Linux
dmesg | grep -i usb
journalctl -f | grep -i usb
```

### Test with Different Tools
```bash
# Use screen (exit with Ctrl+A then K)
screen /dev/tty.usbmodem101 115200

# Use minicom (Linux)
minicom -D /dev/ttyUSB0 -b 115200

# Use cu (macOS/Linux)
cu -l /dev/tty.usbmodem101 -s 115200
```

### Arduino IDE Troubleshooting
```bash
# Check if Arduino IDE can see the device
# Tools > Port > should show your device
# If not visible, driver issues likely
```

## Hardware Checks

### ESP32S3 Status LEDs
- Power LED should be on
- Any status LEDs should indicate normal operation

### USB Connection
- Try different USB cables
- Try different USB ports
- Check for physical damage

### Battery Connection
- Ensure battery is properly connected
- Check battery voltage (should be 3.0V - 4.2V)

## Firmware Verification

### Check Firmware Upload
```bash
# Re-upload firmware using Arduino IDE
# Verify no compilation errors
# Check upload success messages
```

### Verify Serial Configuration
```cpp
// Ensure these are in firmware setup():
Serial.begin(115200);
Serial.println("OpenGlass starting up...");
```

## Contact Information

If none of these solutions work:
1. Check the firmware compilation logs
2. Verify the ESP32S3 board is properly selected in Arduino IDE
3. Try uploading a simple "Hello World" sketch to test basic functionality
4. Check if the device appears in Device Manager (Windows) or System Information (macOS) 