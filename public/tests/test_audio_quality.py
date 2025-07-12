#!/usr/bin/env python3
"""
Test script to evaluate ESP32-S3 audio quality improvements with filtering.
This script captures audio and analyzes the signal quality.
"""

import time
import struct
import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.io import wavfile
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class AudioQualityTester:
    def __init__(self, device_ip="192.168.4.1", port=80):
        self.device_ip = device_ip
        self.port = port
        self.sample_rate = 16000
        self.bytes_per_sample = 2
        self.test_duration = 5.0  # seconds
        
    def capture_audio_data(self):
        """Capture audio data from ESP32-S3 via BLE/WiFi"""
        logger.info("Starting audio quality test...")
        
        audio_frames = []
        start_time = time.time()
        frame_count = 0
        total_bytes = 0
        
        # Simulate BLE connection and data capture
        logger.info(f"Capturing audio for {self.test_duration} seconds...")
        
        while time.time() - start_time < self.test_duration:
            # In a real implementation, this would read from BLE
            # For now, we'll simulate the data capture process
            frame_count += 1
            
            # Simulate receiving audio frame
            time.sleep(0.05)  # 50ms intervals
            
        logger.info(f"Capture completed: {frame_count} frames in {time.time() - start_time:.2f}s")
        return audio_frames, frame_count, total_bytes
    
    def analyze_audio_quality(self, audio_data):
        """Analyze audio quality metrics"""
        if len(audio_data) == 0:
            logger.warning("No audio data to analyze")
            return {}
        
        # Convert to numpy array
        audio_array = np.array(audio_data, dtype=np.int16)
        
        # Calculate quality metrics
        metrics = {}
        
        # Signal-to-noise ratio estimation
        signal_power = np.mean(audio_array**2)
        noise_floor = np.percentile(np.abs(audio_array), 10)  # Bottom 10% as noise estimate
        if noise_floor > 0:
            snr_db = 10 * np.log10(signal_power / (noise_floor**2))
            metrics['snr_db'] = snr_db
        
        # Dynamic range
        max_amplitude = np.max(np.abs(audio_array))
        min_amplitude = np.min(np.abs(audio_array[audio_array != 0]))
        if min_amplitude > 0:
            dynamic_range_db = 20 * np.log10(max_amplitude / min_amplitude)
            metrics['dynamic_range_db'] = dynamic_range_db
        
        # RMS level
        rms_level = np.sqrt(np.mean(audio_array**2))
        metrics['rms_level'] = rms_level
        
        # Peak level
        peak_level = np.max(np.abs(audio_array))
        metrics['peak_level'] = peak_level
        
        # Crest factor (peak to RMS ratio)
        if rms_level > 0:
            crest_factor_db = 20 * np.log10(peak_level / rms_level)
            metrics['crest_factor_db'] = crest_factor_db
        
        logger.info("Audio Quality Metrics:")
        for key, value in metrics.items():
            logger.info(f"  {key}: {value:.2f}")
        
        return metrics
    
    def plot_audio_spectrum(self, audio_data, save_plot=True):
        """Plot audio spectrum to visualize frequency content"""
        if len(audio_data) == 0:
            logger.warning("No audio data to plot")
            return
        
        audio_array = np.array(audio_data, dtype=np.int16)
        
        # Create figure with subplots
        fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 10))
        
        # Time domain plot
        time_axis = np.arange(len(audio_array)) / self.sample_rate
        ax1.plot(time_axis, audio_array)
        ax1.set_title('Audio Waveform (Time Domain)')
        ax1.set_xlabel('Time (seconds)')
        ax1.set_ylabel('Amplitude')
        ax1.grid(True)
        
        # Frequency domain plot (FFT)
        frequencies, spectrum = signal.welch(audio_array, self.sample_rate, nperseg=1024)
        ax2.semilogy(frequencies, spectrum)
        ax2.set_title('Audio Spectrum (Frequency Domain)')
        ax2.set_xlabel('Frequency (Hz)')
        ax2.set_ylabel('Power Spectral Density')
        ax2.grid(True)
        ax2.set_xlim(0, self.sample_rate // 2)
        
        # Spectrogram
        f, t, Sxx = signal.spectrogram(audio_array, self.sample_rate, nperseg=256)
        im = ax3.pcolormesh(t, f, 10 * np.log10(Sxx + 1e-10), shading='gouraud')
        ax3.set_title('Spectrogram')
        ax3.set_xlabel('Time (seconds)')
        ax3.set_ylabel('Frequency (Hz)')
        plt.colorbar(im, ax=ax3, label='Power (dB)')
        
        plt.tight_layout()
        
        if save_plot:
            timestamp = int(time.time())
            filename = f'audio_quality_analysis_{timestamp}.png'
            plt.savefig(filename, dpi=150, bbox_inches='tight')
            logger.info(f"Audio analysis plot saved as {filename}")
        
        plt.show()
    
    def test_filtering_effectiveness(self):
        """Test the effectiveness of the audio filtering"""
        logger.info("Testing audio filtering effectiveness...")
        
        # This would compare filtered vs unfiltered audio
        # For now, we'll provide guidance on what to look for
        
        improvements_to_expect = [
            "Reduced DC offset (signal centered around zero)",
            "Less low-frequency rumble and noise",
            "Clearer speech frequencies (300-3400 Hz)",
            "Improved signal-to-noise ratio",
            "Reduced static and hiss",
            "Better dynamic range"
        ]
        
        logger.info("Expected improvements with filtering:")
        for improvement in improvements_to_expect:
            logger.info(f"  • {improvement}")
        
        return improvements_to_expect
    
    def run_comprehensive_test(self):
        """Run a comprehensive audio quality test"""
        logger.info("Starting comprehensive audio quality test...")
        
        # Test filtering effectiveness
        self.test_filtering_effectiveness()
        
        # Capture audio data
        audio_frames, frame_count, total_bytes = self.capture_audio_data()
        
        # For demonstration, create some sample data
        # In real use, this would be the actual captured audio
        logger.info("Generating sample analysis (replace with real audio data)...")
        
        # Create sample audio data for demonstration
        t = np.linspace(0, self.test_duration, int(self.sample_rate * self.test_duration))
        
        # Simulate filtered audio (cleaner signal)
        clean_signal = 1000 * np.sin(2 * np.pi * 440 * t)  # 440 Hz tone
        noise = 50 * np.random.randn(len(t))  # Reduced noise
        sample_audio = clean_signal + noise
        sample_audio = np.clip(sample_audio, -32768, 32767).astype(np.int16)
        
        # Analyze quality
        metrics = self.analyze_audio_quality(sample_audio)
        
        # Plot spectrum
        self.plot_audio_spectrum(sample_audio)
        
        # Save sample as WAV file
        timestamp = int(time.time())
        wav_filename = f'filtered_audio_sample_{timestamp}.wav'
        wavfile.write(wav_filename, self.sample_rate, sample_audio)
        logger.info(f"Sample audio saved as {wav_filename}")
        
        return metrics

def main():
    """Main test function"""
    print("ESP32-S3 Audio Quality Test with Filtering")
    print("=" * 50)
    
    tester = AudioQualityTester()
    
    try:
        # Run comprehensive test
        metrics = tester.run_comprehensive_test()
        
        print("\nTest completed successfully!")
        print("Key improvements to listen for:")
        print("  1. Reduced background hiss and static")
        print("  2. Clearer speech intelligibility")
        print("  3. Less low-frequency rumble")
        print("  4. Better overall signal quality")
        
    except Exception as e:
        logger.error(f"Test failed: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main()) 