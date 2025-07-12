#!/usr/bin/env python3
"""
Comprehensive Microphone Diagnostic Tool
Analyzes the microphone data stream to identify specific issues.
"""

import asyncio
import sys
import time
import wave
import struct
import numpy as np
from bleak import BleakClient, BleakScanner
from datetime import datetime

# BLE Configuration
DEVICE_NAME = "OpenGlass"
AUDIO_SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
AUDIO_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

# Audio Configuration
SAMPLE_RATE = 16000
SAMPLE_WIDTH = 2
CHANNELS = 1

class MicrophoneDiagnostic:
    def __init__(self):
        self.device = None
        self.client = None
        self.frame_count = 0
        self.start_time = None
        self.audio_data = []
        self.frame_sizes = []
        self.data_patterns = []
        
    async def find_device(self):
        """Find the OpenGlass device"""
        print("🔍 Scanning for OpenGlass device...")
        devices = await BleakScanner.discover(timeout=10.0)
        
        for device in devices:
            if device.name == DEVICE_NAME:
                self.device = device
                print(f"✅ Found device: {device.name} ({device.address})")
                return True
        
        print("❌ OpenGlass device not found!")
        return False
    
    def audio_callback(self, sender, data):
        """Handle incoming audio data and analyze it"""
        if self.start_time is None:
            self.start_time = time.time()
            print("🎤 Audio stream started - analyzing data...")
        
        self.frame_count += 1
        self.audio_data.append(data)
        self.frame_sizes.append(len(data))
        
        # Analyze data patterns
        if len(data) >= 8:
            # Check for header (first 3 bytes)
            header = data[:3]
            audio_payload = data[3:]
            
            # Store pattern analysis
            pattern_info = {
                'frame_num': self.frame_count,
                'total_size': len(data),
                'header': header.hex(),
                'payload_size': len(audio_payload),
                'first_8_audio_bytes': audio_payload[:8].hex() if len(audio_payload) >= 8 else audio_payload.hex()
            }
            self.data_patterns.append(pattern_info)
            
            # Check for obvious issues
            if len(audio_payload) == 0:
                print(f"⚠️  Frame {self.frame_count}: No audio payload!")
            elif all(b == 0 for b in audio_payload):
                print(f"⚠️  Frame {self.frame_count}: All zeros in audio payload!")
            elif all(b == 0xFF for b in audio_payload):
                print(f"⚠️  Frame {self.frame_count}: All 0xFF in audio payload!")
        
        # Show progress every 50 frames
        if self.frame_count % 50 == 0:
            elapsed = time.time() - self.start_time
            rate = self.frame_count / elapsed
            print(f"📊 Frames: {self.frame_count}, Rate: {rate:.1f} fps, Size: {len(data)} bytes")
    
    async def diagnose_microphone(self, duration=10):
        """Diagnose microphone issues"""
        if not self.device:
            print("❌ No device found!")
            return False
        
        try:
            print(f"🔗 Connecting to {self.device.address}...")
            async with BleakClient(self.device.address) as client:
                self.client = client
                
                # Subscribe to audio notifications
                await client.start_notify(AUDIO_CHARACTERISTIC_UUID, self.audio_callback)
                print(f"🎤 Starting {duration} second diagnostic...")
                
                # Wait for diagnostic duration
                await asyncio.sleep(duration)
                
                # Stop notifications
                await client.stop_notify(AUDIO_CHARACTERISTIC_UUID)
                
                # Analyze results
                return self.analyze_results()
                
        except Exception as e:
            print(f"❌ Error during diagnosis: {e}")
            return False
    
    def analyze_results(self):
        """Analyze the collected data"""
        if self.frame_count == 0:
            print("\n❌ CRITICAL: No audio frames received!")
            print("   - Check BLE connection")
            print("   - Verify audio characteristic UUID")
            print("   - Check if audio capture is enabled in firmware")
            return False
        
        elapsed = time.time() - self.start_time
        avg_rate = self.frame_count / elapsed
        total_bytes = sum(len(data) for data in self.audio_data)
        
        print(f"\n{'='*60}")
        print("MICROPHONE DIAGNOSTIC RESULTS")
        print(f"{'='*60}")
        
        # Basic stats
        print(f"📊 Basic Statistics:")
        print(f"   Duration: {elapsed:.1f} seconds")
        print(f"   Total Frames: {self.frame_count}")
        print(f"   Frame Rate: {avg_rate:.1f} fps")
        print(f"   Total Data: {total_bytes} bytes")
        print(f"   Data Rate: {total_bytes/elapsed:.1f} bytes/second")
        
        # Frame size analysis
        if self.frame_sizes:
            avg_frame_size = sum(self.frame_sizes) / len(self.frame_sizes)
            min_frame_size = min(self.frame_sizes)
            max_frame_size = max(self.frame_sizes)
            
            print(f"\n📏 Frame Size Analysis:")
            print(f"   Average Frame Size: {avg_frame_size:.1f} bytes")
            print(f"   Min Frame Size: {min_frame_size} bytes")
            print(f"   Max Frame Size: {max_frame_size} bytes")
            
            # Check for consistent frame sizes
            if min_frame_size == max_frame_size:
                print(f"   ✅ Consistent frame sizes")
            else:
                print(f"   ⚠️  Variable frame sizes detected")
        
        # Data pattern analysis
        if self.data_patterns:
            print(f"\n🔍 Data Pattern Analysis:")
            
            # Show first few patterns
            print(f"   First 3 frames:")
            for i, pattern in enumerate(self.data_patterns[:3]):
                print(f"     Frame {pattern['frame_num']}: {pattern['total_size']}B, "
                      f"Header: {pattern['header']}, "
                      f"Audio: {pattern['first_8_audio_bytes']}")
            
            # Check for problematic patterns
            zero_payloads = sum(1 for p in self.data_patterns if p['payload_size'] == 0)
            if zero_payloads > 0:
                print(f"   ⚠️  {zero_payloads} frames with zero audio payload")
            
            # Check for repeated patterns (static)
            audio_patterns = [p['first_8_audio_bytes'] for p in self.data_patterns[:10]]
            unique_patterns = set(audio_patterns)
            if len(unique_patterns) == 1:
                print(f"   ❌ STATIC DETECTED: All frames have identical audio pattern")
                print(f"      Pattern: {list(unique_patterns)[0]}")
            elif len(unique_patterns) < 3:
                print(f"   ⚠️  Very few unique patterns ({len(unique_patterns)}) - likely static")
            else:
                print(f"   ✅ Multiple unique patterns ({len(unique_patterns)}) - good variation")
        
        # Audio quality analysis
        if self.audio_data:
            print(f"\n🎧 Audio Quality Analysis:")
            
            # Combine all audio data (skip headers)
            combined_audio = b''
            for data in self.audio_data:
                if len(data) > 3:
                    combined_audio += data[3:]  # Skip 3-byte header
            
            if len(combined_audio) > 0:
                # Convert to numpy array for analysis
                if len(combined_audio) % 2 == 0:
                    audio_samples = np.frombuffer(combined_audio, dtype=np.int16)
                    
                    # Calculate statistics
                    mean_amplitude = np.mean(np.abs(audio_samples))
                    max_amplitude = np.max(np.abs(audio_samples))
                    std_amplitude = np.std(audio_samples)
                    
                    print(f"   Mean Amplitude: {mean_amplitude:.1f}")
                    print(f"   Max Amplitude: {max_amplitude}")
                    print(f"   Std Deviation: {std_amplitude:.1f}")
                    
                    # Check for issues
                    if max_amplitude == 0:
                        print(f"   ❌ SILENT: All audio samples are zero")
                    elif max_amplitude < 100:
                        print(f"   ⚠️  Very low amplitude - microphone may not be working")
                    elif std_amplitude < 10:
                        print(f"   ⚠️  Low variation - likely static or no real audio")
                    else:
                        print(f"   ✅ Good amplitude variation - real audio detected")
                    
                    # Save diagnostic audio file
                    self.save_diagnostic_audio(combined_audio)
                else:
                    print(f"   ⚠️  Odd number of bytes - possible data corruption")
            else:
                print(f"   ❌ No audio payload data found")
        
        # Overall diagnosis
        print(f"\n{'='*60}")
        print("DIAGNOSIS SUMMARY")
        print(f"{'='*60}")
        
        issues_found = []
        
        if self.frame_count == 0:
            issues_found.append("No audio frames received")
        elif avg_rate < 15:
            issues_found.append(f"Low frame rate ({avg_rate:.1f} fps)")
        
        if self.frame_sizes and min(self.frame_sizes) < 10:
            issues_found.append("Very small frame sizes")
        
        if self.data_patterns:
            audio_patterns = [p['first_8_audio_bytes'] for p in self.data_patterns[:10]]
            if len(set(audio_patterns)) < 3:
                issues_found.append("Static audio pattern detected")
        
        if issues_found:
            print("❌ Issues Found:")
            for issue in issues_found:
                print(f"   - {issue}")
            print("\n🔧 Recommended Fixes:")
            print("   1. Check I2S microphone configuration")
            print("   2. Verify microphone hardware connection")
            print("   3. Update firmware with correct I2S API")
            print("   4. Check microphone power supply")
            return False
        else:
            print("✅ No major issues detected - microphone appears to be working")
            return True
    
    def save_diagnostic_audio(self, audio_data):
        """Save diagnostic audio file"""
        filename = f"diagnostic_audio_{int(time.time())}.wav"
        
        try:
            with wave.open(filename, 'wb') as wav_file:
                wav_file.setnchannels(CHANNELS)
                wav_file.setsampwidth(SAMPLE_WIDTH)
                wav_file.setframerate(SAMPLE_RATE)
                wav_file.writeframes(audio_data)
            
            duration = len(audio_data) / (SAMPLE_RATE * SAMPLE_WIDTH * CHANNELS)
            print(f"   💾 Diagnostic audio saved: {filename} ({duration:.1f}s)")
            
        except Exception as e:
            print(f"   ❌ Failed to save diagnostic audio: {e}")

async def main():
    """Main diagnostic function"""
    print("🔬 ESP32S3 Microphone Diagnostic Tool")
    print("="*50)
    
    diagnostic = MicrophoneDiagnostic()
    
    # Find device
    if not await diagnostic.find_device():
        print("❌ Please ensure the ESP32S3 device is powered on and nearby.")
        sys.exit(1)
    
    # Run diagnostic
    success = await diagnostic.diagnose_microphone(duration=15)
    
    if success:
        print("\n✅ Microphone diagnostic completed - no major issues found")
    else:
        print("\n❌ Microphone diagnostic found issues - see recommendations above")
    
    print(f"\n📁 Check the diagnostic audio file to verify audio quality")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n🛑 Diagnostic interrupted by user")
        sys.exit(1) 