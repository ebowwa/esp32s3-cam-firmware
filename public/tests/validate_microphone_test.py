#!/usr/bin/env python3
"""
Microphone Test Validation Script
Simple script to verify ESP32S3 microphone functionality is working.
"""

import asyncio
import sys
import time
from bleak import BleakClient, BleakScanner
import struct

# BLE Configuration
DEVICE_NAME = "OpenGlass"
AUDIO_SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
AUDIO_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

class MicrophoneValidator:
    def __init__(self):
        self.device = None
        self.client = None
        self.frame_count = 0
        self.start_time = None
        self.audio_data = []
        
    async def find_device(self):
        """Find the OpenGlass device"""
        print("Scanning for OpenGlass device...")
        devices = await BleakScanner.discover(timeout=10.0)
        
        for device in devices:
            if device.name == DEVICE_NAME:
                self.device = device
                print(f"Found device: {device.name} ({device.address})")
                return True
        
        print("OpenGlass device not found!")
        return False
    
    def audio_callback(self, sender, data):
        """Handle incoming audio data"""
        if self.start_time is None:
            self.start_time = time.time()
            print("Audio streaming started!")
        
        self.frame_count += 1
        self.audio_data.append(data)
        
        # Show progress every 50 frames
        if self.frame_count % 50 == 0:
            elapsed = time.time() - self.start_time
            rate = self.frame_count / elapsed
            print(f"Frames received: {self.frame_count}, Rate: {rate:.1f} fps")
    
    async def test_microphone(self, duration=10):
        """Test microphone for specified duration"""
        if not self.device:
            print("No device found!")
            return False
        
        try:
            print(f"Connecting to {self.device.address}...")
            async with BleakClient(self.device.address) as client:
                self.client = client
                
                # Subscribe to audio notifications
                await client.start_notify(AUDIO_CHARACTERISTIC_UUID, self.audio_callback)
                print(f"Subscribed to audio notifications. Testing for {duration} seconds...")
                
                # Wait for test duration
                await asyncio.sleep(duration)
                
                # Stop notifications
                await client.stop_notify(AUDIO_CHARACTERISTIC_UUID)
                
                # Show results
                if self.frame_count > 0:
                    elapsed = time.time() - self.start_time
                    avg_rate = self.frame_count / elapsed
                    total_bytes = sum(len(data) for data in self.audio_data)
                    
                    print("\n" + "="*50)
                    print("MICROPHONE TEST RESULTS")
                    print("="*50)
                    print(f"Test Duration: {elapsed:.1f} seconds")
                    print(f"Total Frames: {self.frame_count}")
                    print(f"Average Rate: {avg_rate:.1f} frames/second")
                    print(f"Total Data: {total_bytes} bytes")
                    print(f"Data Rate: {total_bytes/elapsed:.1f} bytes/second")
                    
                    if self.frame_count > 0:
                        frame_size = len(self.audio_data[0]) if self.audio_data else 0
                        print(f"Frame Size: {frame_size} bytes")
                        
                        # Check if we're getting expected audio data
                        if avg_rate > 15 and frame_size == 160:
                            print("\n✅ MICROPHONE TEST PASSED!")
                            print("Audio streaming is working correctly.")
                            return True
                        else:
                            print("\n⚠️  MICROPHONE TEST PARTIAL SUCCESS")
                            print("Audio data received but performance may be suboptimal.")
                            return True
                    else:
                        print("\n❌ MICROPHONE TEST FAILED")
                        print("No audio frames received.")
                        return False
                else:
                    print("\n❌ MICROPHONE TEST FAILED")
                    print("No audio data received.")
                    return False
                    
        except Exception as e:
            print(f"Error during test: {e}")
            return False

async def main():
    """Main test function"""
    print("ESP32S3 Microphone Validation Test")
    print("="*40)
    
    validator = MicrophoneValidator()
    
    # Find device
    if not await validator.find_device():
        print("Please ensure the ESP32S3 device is powered on and nearby.")
        sys.exit(1)
    
    # Test microphone
    success = await validator.test_microphone(duration=10)
    
    if success:
        print("\nMicrophone functionality verified! ✅")
        sys.exit(0)
    else:
        print("\nMicrophone test failed. Check troubleshooting guide. ❌")
        sys.exit(1)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nTest interrupted by user.")
        sys.exit(1) 