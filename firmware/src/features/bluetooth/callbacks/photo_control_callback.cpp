#include "photo_control_callback.h"

// Photo Control Callback Implementation
void PhotoControlCallback::onWrite(BLECharacteristic *characteristic) {
    Serial.printf("Photo control write received, length: %d\n", characteristic->getLength());
    if (characteristic->getLength() == 1) {
        uint8_t value = characteristic->getData()[0];
        Serial.printf("Photo control value: %d (0x%02X)\n", (int8_t)value, value);
        handlePhotoControl((int8_t)value);
    } else {
        Serial.println("Invalid photo control message length");
    }
} 