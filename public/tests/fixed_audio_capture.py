#!/usr/bin/env python3
"""
Fixed Audio Capture Script
Addresses DC offset and sample format issues to produce clear audio.
"""

import asyncio
import sys
import time
import wave
import struct
import numpy as np
from bleak import BleakClient, BleakScanner

# BLE Configuration
DEVICE_NAME = "OpenGlass"
AUDIO_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

# Audio Configuration
SAMPLE_RATE = 16000
SAMPLE_WIDTH = 2
CHANNELS = 1

class FixedAudioCapture:
    def __init__(self, duration=10):
        self.device = None
        self.audio_data = []
        self.duration = duration
        self.output_file = f"fixed_audio_{int(time.time())}.wav"
        
    async def find_device(self):
        """Find the OpenGlass device"""
        print("🔍 Scanning for device...")
        devices = await BleakScanner.discover(timeout=10.0)
        
        for device in devices:
            if device.name == DEVICE_NAME:
                self.device = device
                print(f"✅ Found: {device.name}")
                return True
        
        print("❌ Device not found!")
        return False
    
    def audio_callback(self, sender, data):
        """Handle incoming audio data"""
        if len(data) > 3:
            # Skip 3-byte header and store audio payload
            self.audio_data.append(data[3:])
    
    def process_audio_data(self):
        """Process and fix the audio data"""
        if not self.audio_data:
            print("❌ No audio data to process!")
            return None
        
        # Combine all audio data
        combined_audio = b''.join(self.audio_data)
        
        # Convert to 16-bit samples
        samples = struct.unpack(f'<{len(combined_audio)//2}h', combined_audio)
        samples_array = np.array(samples, dtype=np.int16)
        
        print(f"📊 Original audio statistics:")
        print(f"   Samples: {len(samples_array)}")
        print(f"   Min: {np.min(samples_array)}")
        print(f"   Max: {np.max(samples_array)}")
        print(f"   Mean: {np.mean(samples_array):.2f}")
        
        # Fix 1: Remove DC offset (center around zero)
        dc_offset = np.mean(samples_array)
        samples_centered = samples_array - dc_offset
        
        print(f"📊 After DC offset removal:")
        print(f"   DC offset removed: {dc_offset:.2f}")
        print(f"   New min: {np.min(samples_centered)}")
        print(f"   New max: {np.max(samples_centered)}")
        print(f"   New mean: {np.mean(samples_centered):.2f}")
        
        # Fix 2: Normalize amplitude (but keep reasonable range)
        max_amplitude = np.max(np.abs(samples_centered))
        if max_amplitude > 0:
            # Scale to use about 50% of the 16-bit range to avoid clipping
            target_amplitude = 16000
            scale_factor = target_amplitude / max_amplitude
            samples_normalized = samples_centered * scale_factor
            
            print(f"📊 After normalization:")
            print(f"   Scale factor: {scale_factor:.2f}")
            print(f"   Final min: {np.min(samples_normalized)}")
            print(f"   Final max: {np.max(samples_normalized)}")
        else:
            samples_normalized = samples_centered
        
        # Convert back to int16
        samples_fixed = samples_normalized.astype(np.int16)
        
        return samples_fixed.tobytes()
    
    def save_audio_file(self, audio_data):
        """Save the fixed audio data to WAV file"""
        try:
            with wave.open(self.output_file, 'wb') as wav_file:
                wav_file.setnchannels(CHANNELS)
                wav_file.setsampwidth(SAMPLE_WIDTH)
                wav_file.setframerate(SAMPLE_RATE)
                wav_file.writeframes(audio_data)
            
            # Calculate actual duration
            duration_seconds = len(audio_data) / (SAMPLE_RATE * SAMPLE_WIDTH * CHANNELS)
            file_size_kb = len(audio_data) / 1024
            
            print(f"\n🎵 Fixed audio saved: {self.output_file}")
            print(f"📊 File size: {file_size_kb:.1f} KB")
            print(f"⏱️  Duration: {duration_seconds:.1f} seconds")
            print(f"🎧 Format: {SAMPLE_RATE} Hz, {SAMPLE_WIDTH*8}-bit, Mono")
            
            return True
            
        except Exception as e:
            print(f"❌ Error saving audio: {e}")
            return False
    
    async def capture_audio(self):
        """Capture and fix audio"""
        if not self.device:
            return False
        
        try:
            print(f"🔗 Connecting to {self.device.address}...")
            async with BleakClient(self.device.address) as client:
                
                print(f"🎤 Starting {self.duration} second capture...")
                await client.start_notify(AUDIO_CHARACTERISTIC_UUID, self.audio_callback)
                
                start_time = time.time()
                while time.time() - start_time < self.duration:
                    await asyncio.sleep(0.1)
                    # Show progress
                    elapsed = time.time() - start_time
                    remaining = self.duration - elapsed
                    if int(elapsed) % 2 == 0 and elapsed > 0:
                        print(f"🎤 Recording... {elapsed:.1f}s / {self.duration}s ({remaining:.1f}s remaining)")
                
                await client.stop_notify(AUDIO_CHARACTERISTIC_UUID)
                
                print(f"\n📊 Capture complete - processing audio...")
                
                # Process and fix the audio data
                fixed_audio = self.process_audio_data()
                
                if fixed_audio:
                    if self.save_audio_file(fixed_audio):
                        print(f"\n✅ SUCCESS! Fixed audio saved to: {self.output_file}")
                        print(f"🎧 Play with: open {self.output_file}")
                        return True
                    else:
                        print(f"\n❌ Failed to save audio file")
                        return False
                else:
                    print(f"\n❌ No audio data to save")
                    return False
                    
        except Exception as e:
            print(f"❌ Error during capture: {e}")
            return False

async def main():
    """Main function"""
    print("🎤 ESP32S3 Fixed Audio Capture")
    print("="*40)
    
    # Get duration from user or use default
    duration = 10
    if len(sys.argv) > 1:
        try:
            duration = int(sys.argv[1])
        except ValueError:
            print("Invalid duration, using default 10 seconds")
    
    print(f"Recording duration: {duration} seconds")
    
    capturer = FixedAudioCapture(duration)
    
    # Find device
    if not await capturer.find_device():
        print("❌ Please ensure the ESP32S3 device is powered on and nearby.")
        sys.exit(1)
    
    # Capture and fix audio
    success = await capturer.capture_audio()
    
    if success:
        print(f"\n🎉 Audio capture and processing completed successfully!")
    else:
        print(f"\n❌ Audio capture failed.")
        sys.exit(1)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n🛑 Capture interrupted by user")
        sys.exit(1) 