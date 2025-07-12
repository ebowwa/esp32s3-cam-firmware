#!/usr/bin/env python3
"""
Audio Data Analysis Tool
Analyzes the raw audio data to understand what's actually being captured.
"""

import asyncio
import sys
import time
import struct
import numpy as np
from bleak import BleakClient, BleakScanner

# BLE Configuration
DEVICE_NAME = "OpenGlass"
AUDIO_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

class AudioDataAnalyzer:
    def __init__(self):
        self.device = None
        self.raw_frames = []
        self.frame_count = 0
        
    async def find_device(self):
        """Find the OpenGlass device"""
        print("🔍 Scanning for device...")
        devices = await BleakScanner.discover(timeout=10.0)
        
        for device in devices:
            if device.name == DEVICE_NAME:
                self.device = device
                print(f"✅ Found: {device.name} ({device.address})")
                return True
        
        print("❌ Device not found!")
        return False
    
    def audio_callback(self, sender, data):
        """Collect raw audio data for analysis"""
        self.frame_count += 1
        self.raw_frames.append(data)
        
        # Show first few frames in detail
        if self.frame_count <= 5:
            print(f"\n📊 Frame {self.frame_count}:")
            print(f"   Size: {len(data)} bytes")
            print(f"   Raw hex: {data.hex()}")
            
            if len(data) >= 3:
                header = data[:3]
                payload = data[3:]
                print(f"   Header: {header.hex()}")
                print(f"   Payload: {payload[:16].hex()}... ({len(payload)} bytes)")
                
                # Analyze payload as 16-bit samples
                if len(payload) >= 8:
                    samples = struct.unpack('<4h', payload[:8])  # Little-endian 16-bit
                    print(f"   First 4 samples: {samples}")
                    
                    # Check if all samples are the same (static)
                    if len(set(samples)) == 1:
                        print(f"   ⚠️  All samples identical: {samples[0]}")
                    else:
                        print(f"   ✅ Sample variation detected")
    
    async def analyze_audio_stream(self, duration=10):
        """Analyze the audio stream"""
        if not self.device:
            return False
        
        try:
            print(f"🔗 Connecting to {self.device.address}...")
            async with BleakClient(self.device.address) as client:
                
                print(f"🎤 Starting {duration} second analysis...")
                await client.start_notify(AUDIO_CHARACTERISTIC_UUID, self.audio_callback)
                
                await asyncio.sleep(duration)
                
                await client.stop_notify(AUDIO_CHARACTERISTIC_UUID)
                
                # Detailed analysis
                self.detailed_analysis()
                return True
                
        except Exception as e:
            print(f"❌ Error: {e}")
            return False
    
    def detailed_analysis(self):
        """Perform detailed analysis of collected data"""
        print(f"\n{'='*60}")
        print("DETAILED AUDIO DATA ANALYSIS")
        print(f"{'='*60}")
        
        if not self.raw_frames:
            print("❌ No data collected!")
            return
        
        print(f"📊 Total frames analyzed: {len(self.raw_frames)}")
        
        # Extract all audio payloads
        all_payloads = []
        for frame in self.raw_frames:
            if len(frame) > 3:
                all_payloads.append(frame[3:])
        
        if not all_payloads:
            print("❌ No audio payloads found!")
            return
        
        # Combine all payloads
        combined_audio = b''.join(all_payloads)
        print(f"📏 Total audio data: {len(combined_audio)} bytes")
        
        # Convert to 16-bit samples
        if len(combined_audio) % 2 == 0:
            samples = struct.unpack(f'<{len(combined_audio)//2}h', combined_audio)
            print(f"🎵 Total samples: {len(samples)}")
            
            # Statistical analysis
            samples_array = np.array(samples)
            
            print(f"\n📈 Statistical Analysis:")
            print(f"   Min value: {np.min(samples_array)}")
            print(f"   Max value: {np.max(samples_array)}")
            print(f"   Mean: {np.mean(samples_array):.2f}")
            print(f"   Std deviation: {np.std(samples_array):.2f}")
            print(f"   Unique values: {len(np.unique(samples_array))}")
            
            # Check for patterns
            print(f"\n🔍 Pattern Analysis:")
            
            # Check if all values are the same
            unique_values = np.unique(samples_array)
            if len(unique_values) == 1:
                print(f"   ❌ STATIC: All samples are identical ({unique_values[0]})")
            elif len(unique_values) < 10:
                print(f"   ⚠️  Very few unique values ({len(unique_values)})")
                print(f"      Values: {unique_values[:10]}")
            else:
                print(f"   ✅ Good variation: {len(unique_values)} unique values")
            
            # Check for zero data
            zero_count = np.sum(samples_array == 0)
            if zero_count == len(samples_array):
                print(f"   ❌ SILENT: All samples are zero")
            elif zero_count > len(samples_array) * 0.9:
                print(f"   ⚠️  Mostly silent: {zero_count}/{len(samples_array)} samples are zero")
            else:
                print(f"   ✅ Active audio: {zero_count}/{len(samples_array)} zero samples")
            
            # Check for saturation
            max_val = np.max(np.abs(samples_array))
            if max_val > 30000:
                print(f"   ⚠️  Possible saturation: max amplitude {max_val}")
            elif max_val < 100:
                print(f"   ⚠️  Very low amplitude: max amplitude {max_val}")
            else:
                print(f"   ✅ Good amplitude range: max {max_val}")
            
            # Show first 20 samples
            print(f"\n🔢 First 20 samples:")
            print(f"   {samples[:20]}")
            
            # Check for repeating patterns
            if len(samples) >= 40:
                first_20 = samples[:20]
                second_20 = samples[20:40]
                if first_20 == second_20:
                    print(f"   ❌ REPEATING PATTERN: First 20 samples repeat exactly")
                else:
                    print(f"   ✅ No obvious repeating pattern")
            
            # Final diagnosis
            print(f"\n{'='*60}")
            print("DIAGNOSIS")
            print(f"{'='*60}")
            
            if len(unique_values) == 1:
                print("❌ PROBLEM: Static audio - all samples identical")
                print("   Cause: I2S microphone not reading real audio data")
                print("   Solution: Fix I2S configuration in firmware")
            elif zero_count > len(samples_array) * 0.8:
                print("❌ PROBLEM: Silent audio - mostly zero samples")
                print("   Cause: Microphone not connected or not powered")
                print("   Solution: Check hardware connections")
            elif max_val < 100:
                print("⚠️  PROBLEM: Very low amplitude")
                print("   Cause: Microphone gain too low or hardware issue")
                print("   Solution: Increase volume gain or check microphone")
            else:
                print("✅ AUDIO LOOKS GOOD: Reasonable variation and amplitude")
                print("   The microphone appears to be working correctly")
        else:
            print("❌ Odd number of bytes - data corruption")

async def main():
    """Main analysis function"""
    print("🔬 ESP32S3 Audio Data Analyzer")
    print("="*40)
    
    analyzer = AudioDataAnalyzer()
    
    if not await analyzer.find_device():
        sys.exit(1)
    
    success = await analyzer.analyze_audio_stream(duration=10)
    
    if success:
        print("\n✅ Analysis completed")
    else:
        print("\n❌ Analysis failed")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n🛑 Analysis interrupted")
        sys.exit(1) 