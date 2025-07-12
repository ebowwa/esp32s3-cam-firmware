#!/usr/bin/env python3
"""
ESP32S3 Serial Debug Monitor
Captures all serial output to help debug charging system issues
"""

import serial
import time
import sys
from datetime import datetime

def debug_monitor(port, duration=60):
    """Monitor all serial output for debugging"""
    print(f"🔍 ESP32S3 Debug Monitor")
    print(f"Port: {port}")
    print(f"Duration: {duration} seconds")
    print("=" * 60)
    
    try:
        # Connect to device
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(2)
        print(f"✅ Connected to {port}")
        print(f"📖 Monitoring ALL serial output for {duration} seconds...")
        print("=" * 60)
        
        start_time = time.time()
        line_count = 0
        charging_keywords = ['charging', 'charge', 'battery', 'voltage', 'current', 'usb', 'power', 'safety']
        
        while time.time() - start_time < duration:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    line_count += 1
                    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                    
                    # Highlight charging-related messages
                    if any(keyword in line.lower() for keyword in charging_keywords):
                        print(f"🔋 [{timestamp}] {line}")
                    else:
                        print(f"📥 [{timestamp}] {line}")
            else:
                time.sleep(0.1)
        
        print("=" * 60)
        print(f"📊 SUMMARY")
        print(f"Total lines captured: {line_count}")
        print(f"Monitoring duration: {duration} seconds")
        
        if line_count == 0:
            print("❌ No output detected - check device connection and firmware")
        else:
            print("✅ Device is outputting data")
        
        ser.close()
        
    except Exception as e:
        print(f"❌ Error: {e}")
        return False
    
    return True

def main():
    if len(sys.argv) < 2:
        print("Usage: python debug_serial.py <port> [duration]")
        print("Example: python debug_serial.py /dev/ttyUSB0 30")
        return
    
    port = sys.argv[1]
    duration = int(sys.argv[2]) if len(sys.argv) > 2 else 60
    
    print("🐛 ESP32S3 Serial Debug Monitor")
    print("This will capture ALL serial output to help debug charging system issues")
    print()
    
    if debug_monitor(port, duration):
        print("\n🎉 Debug monitoring complete!")
    else:
        print("\n❌ Debug monitoring failed!")

if __name__ == "__main__":
    main() 