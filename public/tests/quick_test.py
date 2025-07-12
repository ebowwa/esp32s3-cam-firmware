#!/usr/bin/env python3
"""
Quick ESP32S3 Charging Test
Simple script to quickly test basic charging functionality
"""

import serial
import time
import sys

def find_serial_ports():
    """Find available serial ports"""
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

def quick_test(port, duration=30):
    """Run a quick charging test"""
    print(f"🚀 Quick Charging Test on {port}")
    print("=" * 50)
    
    try:
        # Connect to device
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(2)
        print(f"✅ Connected to {port}")
        
        # Monitor for charging-related messages
        print(f"📖 Monitoring for {duration} seconds...")
        print("Look for charging status updates, USB events, and sensor readings...")
        print("-" * 50)
        
        start_time = time.time()
        charging_messages = 0
        usb_events = 0
        sensor_readings = 0
        
        while time.time() - start_time < duration:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    # Count different types of messages
                    if any(keyword in line.lower() for keyword in ['charging', 'charge']):
                        charging_messages += 1
                        print(f"🔋 {line}")
                    elif 'usb' in line.lower():
                        usb_events += 1
                        print(f"🔌 {line}")
                    elif any(keyword in line.lower() for keyword in ['voltage', 'current', 'temperature']):
                        sensor_readings += 1
                        print(f"📊 {line}")
                    elif any(keyword in line.lower() for keyword in ['safety', 'error', 'warning']):
                        print(f"⚠️  {line}")
                    else:
                        print(f"📥 {line}")
            else:
                time.sleep(0.1)
        
        # Summary
        print("-" * 50)
        print("📊 QUICK TEST SUMMARY")
        print(f"Charging messages: {charging_messages}")
        print(f"USB events: {usb_events}")
        print(f"Sensor readings: {sensor_readings}")
        
        if charging_messages > 0:
            print("✅ Charging system appears to be working")
        else:
            print("❌ No charging messages detected")
        
        ser.close()
        
    except Exception as e:
        print(f"❌ Error: {e}")
        return False
    
    return True

def main():
    if len(sys.argv) < 2:
        print("Available serial ports:")
        ports = find_serial_ports()
        for i, port in enumerate(ports):
            print(f"  {i+1}. {port}")
        
        if ports:
            print(f"\nUsage: python {sys.argv[0]} <port> [duration]")
            print(f"Example: python {sys.argv[0]} {ports[0]} 30")
        else:
            print("No serial ports found!")
        return
    
    port = sys.argv[1]
    duration = int(sys.argv[2]) if len(sys.argv) > 2 else 30
    
    print("🔍 ESP32S3 Quick Charging Test")
    print(f"Port: {port}")
    print(f"Duration: {duration} seconds")
    print()
    
    if quick_test(port, duration):
        print("\n🎉 Test completed successfully!")
    else:
        print("\n❌ Test failed!")

if __name__ == "__main__":
    main() 