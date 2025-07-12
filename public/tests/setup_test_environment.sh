#!/bin/bash

# ESP32S3 Charging Test Environment Setup Script

echo "🔧 Setting up ESP32S3 Charging Test Environment"
echo "================================================"

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is not installed. Please install Python 3.6+ first."
    exit 1
fi

echo "✅ Python 3 found: $(python3 --version)"

# Check if pip is installed
if ! command -v pip3 &> /dev/null; then
    echo "❌ pip3 is not installed. Please install pip first."
    exit 1
fi

echo "✅ pip3 found: $(pip3 --version)"

# Install Python dependencies
echo "📦 Installing Python dependencies..."
pip3 install pyserial

# Check if pyserial was installed successfully
if python3 -c "import serial" 2>/dev/null; then
    echo "✅ pyserial installed successfully"
else
    echo "❌ Failed to install pyserial"
    exit 1
fi

# Make scripts executable
echo "🔧 Making test scripts executable..."
chmod +x test_charging_serial.py
chmod +x quick_test.py

echo "✅ Test scripts are now executable"

# List available serial ports
echo "🔍 Scanning for available serial ports..."
python3 -c "
import serial.tools.list_ports
ports = serial.tools.list_ports.comports()
if ports:
    print('Available serial ports:')
    for i, port in enumerate(ports):
        print(f'  {i+1}. {port.device} - {port.description}')
else:
    print('No serial ports found.')
"

echo ""
echo "🎉 Setup complete!"
echo ""
echo "Usage examples:"
echo "  # Quick test (30 seconds)"
echo "  python3 quick_test.py /dev/ttyUSB0"
echo ""
echo "  # Comprehensive test"
echo "  python3 test_charging_serial.py --port /dev/ttyUSB0"
echo ""
echo "  # Specific test"
echo "  python3 test_charging_serial.py --port /dev/ttyUSB0 --test usb"
echo ""
echo "Replace /dev/ttyUSB0 with your actual ESP32S3 serial port." 