#include "video_control_callback.h"

// Video Control Callback Implementation
void VideoControlCallback::onWrite(BLECharacteristic *characteristic) {
    Serial.printf("Video control write received, length: %d\n", characteristic->getLength());
    if (characteristic->getLength() == 1) {
        uint8_t value = characteristic->getData()[0];
        Serial.printf("Video control value: %d (0x%02X)\n", value, value);
        handleVideoControl(value);
    } else {
        Serial.println("Invalid video control message length");
    }
} 