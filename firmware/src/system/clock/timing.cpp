#include "timing.h"

// ===================================================================
// TIMING UTILITIES IMPLEMENTATION
// ===================================================================

bool hasTimedOut(unsigned long startTime, unsigned long timeout) {
    return (millis() - startTime) >= timeout;
}

unsigned long getElapsedTime(unsigned long startTime) {
    return millis() - startTime;
}

bool hasTimedOutMicros(unsigned long startTime, unsigned long timeout) {
    return (micros() - startTime) >= timeout;
}

unsigned long getElapsedTimeMicros(unsigned long startTime) {
    return micros() - startTime;
}

bool nonBlockingDelay(unsigned long duration) {
    static unsigned long lastTime = 0;
    static bool initialized = false;
    
    if (!initialized) {
        lastTime = millis();
        initialized = true;
        return false;
    }
    
    if (millis() - lastTime >= duration) {
        initialized = false;
        return true;
    }
    
    return false;
}

bool nonBlockingDelayStateful(unsigned long* lastTime, unsigned long duration) {
    unsigned long currentTime = millis();
    if (currentTime - *lastTime >= duration) {
        *lastTime = currentTime;
        return true;
    }
    return false;
}

bool shouldExecute(unsigned long* lastTime, unsigned long interval) {
    unsigned long currentTime = millis();
    if (currentTime - *lastTime >= interval) {
        *lastTime = currentTime;
        return true;
    }
    return false;
}

unsigned long measureStart() {
    return millis();
}

unsigned long measureEnd(unsigned long startTime) {
    return millis() - startTime;
}

unsigned long measureStartMicros() {
    return micros();
}

unsigned long measureEndMicros(unsigned long startTime) {
    return micros() - startTime;
}

bool waitForCondition(bool (*condition)(), unsigned long timeout) {
    unsigned long startTime = millis();
    while (!condition()) {
        if (hasTimedOut(startTime, timeout)) {
            return false;
        }
        delay(1); // Small delay to prevent busy waiting
    }
    return true;
}

bool debounce(bool currentState, bool* lastState, unsigned long* lastChangeTime, unsigned long debounceDelay) {
    if (currentState != *lastState) {
        *lastChangeTime = millis();
        *lastState = currentState;
    }
    
    return (millis() - *lastChangeTime) > debounceDelay;
}

unsigned long getTimeRemaining(unsigned long startTime, unsigned long timeout) {
    unsigned long elapsed = getElapsedTime(startTime);
    return (elapsed >= timeout) ? 0 : (timeout - elapsed);
}

bool isWithinTimeWindow(unsigned long startTime, unsigned long windowDuration) {
    return getElapsedTime(startTime) <= windowDuration;
}

bool throttle(unsigned long* lastCallTime, unsigned long minInterval) {
    unsigned long currentTime = millis();
    if (currentTime - *lastCallTime >= minInterval) {
        *lastCallTime = currentTime;
        return true;
    }
    return false;
} 