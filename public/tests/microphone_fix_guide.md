# Microphone Static Fix Guide

## Problem Diagnosis

The audio file contains static noise because the I2S microphone reading is not working correctly. The issue is in the `read_microphone()` function in `firmware.ino`.

## Root Cause

1. **Wrong I2S API**: Using `esp_i2s::i2s_read()` instead of Arduino I2S library
2. **Incorrect I2S Configuration**: PDM mode may not be properly configured
3. **Buffer Management**: Audio data processing may have issues

## Fix Implementation

### Step 1: Fix the I2S Read Function

Replace the `read_microphone()` function in `firmware/firmware.ino`:

```cpp
size_t read_microphone() {
  size_t bytes_recorded = 0;
  
  // Use the Arduino I2S library's read function
  bytes_recorded = I2S.read(s_recording_buffer, RECORDING_BUFFER_SIZE);
  
  // Optional: Add some debug logging to understand what we're getting
  static unsigned long lastDebugLog = 0;
  if (millis() - lastDebugLog > 10000) { // Log every 10 seconds
    Serial.printf("I2S read: %d bytes requested, %d bytes received\n", RECORDING_BUFFER_SIZE, bytes_recorded);
    if (bytes_recorded > 0) {
      // Show first few bytes as hex to see the data pattern
      Serial.printf("First 8 bytes: ");
      for (int i = 0; i < min(8, (int)bytes_recorded); i++) {
        Serial.printf("%02X ", s_recording_buffer[i]);
      }
      Serial.println();
    }
    lastDebugLog = millis();
  }
  
  return bytes_recorded;
}
```

### Step 2: Improve I2S Configuration

Replace the `configure_microphone()` function in `firmware/firmware.ino`:

```cpp
void configure_microphone() {
  Serial.println("Configuring I2S microphone...");
  
  // Configure I2S pins for XIAO ESP32-S3
  I2S.setAllPins(-1, I2S_WS_PIN, I2S_SCK_PIN, -1, -1);
  
  // Start I2S in PDM mode for microphone
  if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("❌ Failed to initialize I2S!");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1); // halt
  }
  
  Serial.printf("✅ I2S initialized: %d Hz, %d bits, PDM mode\n", SAMPLE_RATE, SAMPLE_BITS);
  Serial.printf("I2S Pins: WS=%d, SCK=%d\n", I2S_WS_PIN, I2S_SCK_PIN);
  
  // Allocate audio buffers with tracking
  s_recording_buffer = (uint8_t *) PS_CALLOC_TRACKED(RECORDING_BUFFER_SIZE, sizeof(uint8_t), "AudioRecording");
  s_compressed_frame = (uint8_t *) PS_CALLOC_TRACKED(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t), "AudioCompressed");
  s_compressed_frame_2 = (uint8_t *) PS_CALLOC_TRACKED(COMPRESSED_BUFFER_SIZE, sizeof(uint8_t), "AudioCompressed2");
  
  if (!s_recording_buffer || !s_compressed_frame || !s_compressed_frame_2) {
    Serial.println("❌ Failed to allocate audio buffers!");
    updateDeviceStatus(DEVICE_STATUS_ERROR);
    while (1); // halt
  }
  
  Serial.printf("✅ Audio buffers allocated: %d bytes each\n", RECORDING_BUFFER_SIZE);
  
  // Give I2S time to stabilize
  delay(1000);
  
  // Test I2S reading
  Serial.println("Testing I2S microphone reading...");
  size_t test_bytes = I2S.read(s_recording_buffer, RECORDING_BUFFER_SIZE);
  Serial.printf("I2S test read: %d bytes\n", test_bytes);
  
  if (test_bytes > 0) {
    Serial.println("✅ I2S microphone test successful");
  } else {
    Serial.println("⚠️  I2S microphone test returned 0 bytes");
  }
}
```

### Step 3: Fix Audio Data Processing

Update the `prepareAudioFrame()` function in `firmware/src/features/bluetooth/ble_data_handler.cpp`:

```cpp
void prepareAudioFrame(uint8_t *compressedFrame, uint8_t *audioBuffer, size_t bytesRecorded, int &encodedBytes) {
    encodedBytes = 0;
    
    // Ensure we have valid data
    if (!audioBuffer || bytesRecorded == 0) {
        return;
    }
    
#ifdef CODEC_OPUS
    // Opus encoding
    int16_t samples[FRAME_SIZE];
    for (size_t i = 0; i < bytesRecorded && i < FRAME_SIZE * 2; i += 2) {
        samples[i / 2] = ((audioBuffer[i + 1] << 8) | audioBuffer[i]) << VOLUME_GAIN;
    }
    
    encodedBytes = opus_encode(opus_encoder, samples, FRAME_SIZE, &compressedFrame[3], COMPRESSED_BUFFER_SIZE - 3);
    
    if (encodedBytes <= 0) {
        Serial.printf("Opus encoding failed: %d\n", encodedBytes);
        encodedBytes = 0;
    }
    
#else
#ifdef CODEC_MULAW
    // μ-law encoding
    for (size_t i = 0; i < bytesRecorded && i < RECORDING_BUFFER_SIZE; i += 2) {
        int16_t sample = ((audioBuffer[i + 1] << 8) | audioBuffer[i]) << VOLUME_GAIN;
        compressedFrame[i / 2 + 3] = linear2ulaw(sample);
    }
    encodedBytes = bytesRecorded / 2;
    
#else
    // PCM encoding (16-bit to 16-bit with volume gain)
    // Process in pairs of bytes (16-bit samples)
    size_t samples_to_process = min(bytesRecorded / 2, (size_t)FRAME_SIZE);
    
    for (size_t i = 0; i < samples_to_process; i++) {
        // Read 16-bit sample (little-endian)
        int16_t sample = ((int16_t*)audioBuffer)[i];
        
        // Apply volume gain
        sample = sample << VOLUME_GAIN;
        
        // Store back as little-endian
        compressedFrame[i * 2 + 3] = sample & 0xFF;
        compressedFrame[i * 2 + 4] = (sample >> 8) & 0xFF;
    }
    encodedBytes = samples_to_process * 2;
#endif
#endif
}
```

## Testing the Fix

### Step 1: Monitor Serial Output

Run the I2S debug monitor to see what's happening:

```bash
python test_i2s_debug.py
```

Look for:
- I2S initialization messages
- Audio capture debug output
- Byte count information
- Hex data patterns

### Step 2: Test Audio Capture

After uploading the fixed firmware, test audio capture:

```bash
python save_audio_test.py 10
```

### Step 3: Verify Audio Quality

Check the generated WAV file:
- Should not be static noise
- Should contain actual audio data
- File size should be reasonable (not too small)

## Expected Results

After applying the fix:

1. **Serial Output**: Should show I2S initialization and periodic debug messages
2. **Audio Capture**: Should capture real audio data, not static
3. **BLE Streaming**: Should transmit meaningful audio data
4. **WAV Files**: Should contain playable audio

## Troubleshooting

### If Still Getting Static:

1. **Check Hardware**: Ensure microphone is properly connected
2. **Verify Pins**: Confirm I2S pins are correct (GPIO41/42)
3. **Check Power**: Ensure adequate power supply
4. **Test Different Modes**: Try different I2S configurations

### If No Audio Data:

1. **Check I2S Initialization**: Look for error messages
2. **Verify Buffer Allocation**: Ensure buffers are allocated successfully
3. **Test I2S Read**: Check if `I2S.read()` returns data
4. **Hardware Issues**: May need to check microphone hardware

## Hardware Verification

The XIAO ESP32-S3 Sense should have:
- Built-in PDM microphone
- I2S interface on GPIO41/42
- Proper power supply (3.3V)

If the microphone is external, verify:
- Correct wiring to GPIO41/42
- PDM microphone type
- Power supply connections 