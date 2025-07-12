#!/usr/bin/env python3
"""
ESP32S3 Audio Capture and Save Script
Captures audio from the microphone and saves it as a WAV file.
"""

import asyncio
import sys
import time
import wave
import struct
from bleak import BleakClient, BleakScanner
from datetime import datetime

# BLE Configuration
DEVICE_NAME = "OpenGlass"
AUDIO_SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
AUDIO_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

# Audio Configuration (from firmware)
SAMPLE_RATE = 16000
SAMPLE_WIDTH = 2  # 16-bit = 2 bytes
CHANNELS = 1      # Mono

class AudioCapture:
    def __init__(self, duration=30):
        self.device = None
        self.client = None
        self.frame_count = 0
        self.start_time = None
        self.audio_data = []
        self.duration = duration
        self.output_file = f"captured_audio_{int(time.time())}.wav"
        
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
            print(f"Audio capture started! Recording to: {self.output_file}")
        
        self.frame_count += 1
        self.audio_data.append(data)
        
        # Show progress every 100 frames
        if self.frame_count % 100 == 0:
            elapsed = time.time() - self.start_time
            remaining = self.duration - elapsed
            rate = self.frame_count / elapsed
            print(f"Recording... {elapsed:.1f}s / {self.duration}s - {rate:.1f} fps - {remaining:.1f}s remaining")
    
    def save_audio_file(self):
        """Save captured audio data to WAV file"""
        if not self.audio_data:
            print("No audio data to save!")
            return False
        
        try:
            # Combine all audio data, skipping the 3-byte BLE headers
            raw_audio = b''
            for data in self.audio_data:
                if len(data) > 3:
                    raw_audio += data[3:]  # Skip 3-byte BLE header
                else:
                    raw_audio += data  # Keep short frames as-is
            
            # Create WAV file
            with wave.open(self.output_file, 'wb') as wav_file:
                wav_file.setnchannels(CHANNELS)
                wav_file.setsampwidth(SAMPLE_WIDTH)
                wav_file.setframerate(SAMPLE_RATE)
                wav_file.writeframes(raw_audio)
            
            # Calculate file info
            duration_seconds = len(raw_audio) / (SAMPLE_RATE * SAMPLE_WIDTH * CHANNELS)
            file_size_kb = len(raw_audio) / 1024
            
            print(f"\n🎵 Audio file saved: {self.output_file}")
            print(f"📊 File size: {file_size_kb:.1f} KB")
            print(f"⏱️  Duration: {duration_seconds:.1f} seconds")
            print(f"🎧 Format: {SAMPLE_RATE} Hz, {SAMPLE_WIDTH*8}-bit, {'Mono' if CHANNELS == 1 else 'Stereo'}")
            
            return True
            
        except Exception as e:
            print(f"Error saving audio file: {e}")
            return False
    
    async def capture_audio(self):
        """Capture audio for specified duration"""
        if not self.device:
            print("No device found!")
            return False
        
        try:
            print(f"Connecting to {self.device.address}...")
            async with BleakClient(self.device.address) as client:
                self.client = client
                
                # Subscribe to audio notifications
                await client.start_notify(AUDIO_CHARACTERISTIC_UUID, self.audio_callback)
                print(f"🎤 Starting {self.duration} second audio capture...")
                
                # Wait for capture duration
                await asyncio.sleep(self.duration)
                
                # Stop notifications
                await client.stop_notify(AUDIO_CHARACTERISTIC_UUID)
                
                # Show capture results
                if self.frame_count > 0:
                    elapsed = time.time() - self.start_time
                    avg_rate = self.frame_count / elapsed
                    total_bytes = sum(len(data) for data in self.audio_data)
                    
                    print("\n" + "="*50)
                    print("AUDIO CAPTURE COMPLETE")
                    print("="*50)
                    print(f"Capture Duration: {elapsed:.1f} seconds")
                    print(f"Total Frames: {self.frame_count}")
                    print(f"Average Rate: {avg_rate:.1f} frames/second")
                    print(f"Total Data: {total_bytes} bytes")
                    print(f"Data Rate: {total_bytes/elapsed:.1f} bytes/second")
                    
                    # Save the audio file
                    if self.save_audio_file():
                        print(f"\n✅ SUCCESS! Audio saved to: {self.output_file}")
                        print("You can now play this file with any audio player!")
                        return True
                    else:
                        print("\n❌ Failed to save audio file")
                        return False
                else:
                    print("\n❌ No audio data captured")
                    return False
                    
        except Exception as e:
            print(f"Error during capture: {e}")
            return False

async def main():
    """Main capture function"""
    print("🎤 ESP32S3 Audio Capture Tool")
    print("="*40)
    
    # Get duration from user or use default
    duration = 30
    if len(sys.argv) > 1:
        try:
            duration = int(sys.argv[1])
        except ValueError:
            print("Invalid duration, using default 30 seconds")
    
    print(f"Recording duration: {duration} seconds")
    
    capturer = AudioCapture(duration)
    
    # Find device
    if not await capturer.find_device():
        print("Please ensure the ESP32S3 device is powered on and nearby.")
        sys.exit(1)
    
    # Capture audio
    success = await capturer.capture_audio()
    
    if success:
        print(f"\n🎉 Audio capture completed successfully!")
        print(f"📁 File saved: {capturer.output_file}")
        print(f"🎧 Play with: open {capturer.output_file}")
        sys.exit(0)
    else:
        print("\n❌ Audio capture failed.")
        sys.exit(1)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nCapture interrupted by user.")
        sys.exit(1) 