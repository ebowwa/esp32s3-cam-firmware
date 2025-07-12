#!/usr/bin/env python3
"""
I2S Debug Monitor
Monitors serial output for I2S debugging information to help diagnose microphone issues.
"""

import serial
import time
import sys
import re

def monitor_i2s_debug(port='/dev/tty.usbmodem101', baud_rate=921600, duration=60):
    """Monitor serial output for I2S debug information"""
    
    print(f"🔍 Monitoring I2S debug output on {port}")
    print(f"Baud rate: {baud_rate}")
    print(f"Duration: {duration} seconds")
    print("="*50)
    
    try:
        # Open serial connection
        ser = serial.Serial(port, baud_rate, timeout=1)
        time.sleep(2)  # Give time to connect
        
        start_time = time.time()
        i2s_messages = []
        audio_messages = []
        
        while time.time() - start_time < duration:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[{time.time() - start_time:.1f}s] {line}")
                    
                    # Look for I2S related messages
                    if 'I2S' in line.upper() or 'i2s' in line:
                        i2s_messages.append(line)
                        print(f"🎤 I2S: {line}")
                    
                    # Look for audio capture messages
                    if 'audio' in line.lower() or 'microphone' in line.lower():
                        audio_messages.append(line)
                        print(f"🔊 AUDIO: {line}")
                    
                    # Look for byte patterns
                    if 'bytes' in line.lower() and ('read' in line.lower() or 'recorded' in line.lower()):
                        print(f"📊 DATA: {line}")
                    
                    # Look for hex patterns (audio data)
                    if re.search(r'[0-9A-Fa-f]{2}\s+[0-9A-Fa-f]{2}', line):
                        print(f"🔢 HEX: {line}")
                        
            except Exception as e:
                print(f"Error reading line: {e}")
                continue
        
        ser.close()
        
        # Summary
        print("\n" + "="*50)
        print("SUMMARY")
        print("="*50)
        print(f"Total I2S messages: {len(i2s_messages)}")
        print(f"Total audio messages: {len(audio_messages)}")
        
        if i2s_messages:
            print("\nI2S Messages:")
            for msg in i2s_messages[-5:]:  # Show last 5
                print(f"  - {msg}")
        
        if audio_messages:
            print("\nAudio Messages:")
            for msg in audio_messages[-5:]:  # Show last 5
                print(f"  - {msg}")
        
        # Analysis
        print("\n" + "="*50)
        print("ANALYSIS")
        print("="*50)
        
        if not i2s_messages and not audio_messages:
            print("❌ No I2S or audio messages detected")
            print("   This suggests the microphone is not being read properly")
        elif i2s_messages:
            print("✅ I2S messages detected")
            print("   The I2S interface is active")
        
        if any('0 bytes' in msg for msg in i2s_messages):
            print("⚠️  Zero bytes detected in I2S reads")
            print("   This indicates the microphone is not providing data")
        
        if any('Failed' in msg for msg in i2s_messages):
            print("❌ I2S failures detected")
            print("   Check I2S configuration and hardware connections")
            
    except serial.SerialException as e:
        print(f"❌ Serial connection error: {e}")
        print("Make sure the device is connected and the correct port is specified")
        return False
    except KeyboardInterrupt:
        print("\n🛑 Monitoring stopped by user")
        return True
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return False
    
    return True

if __name__ == "__main__":
    port = '/dev/tty.usbmodem101'
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    print("🎤 ESP32S3 I2S Debug Monitor")
    print("="*40)
    
    success = monitor_i2s_debug(port, duration=30)
    
    if success:
        print("\n✅ Monitoring completed successfully")
    else:
        print("\n❌ Monitoring failed")
        sys.exit(1) 