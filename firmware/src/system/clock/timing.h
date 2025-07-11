#ifndef TIMING_H
#define TIMING_H

#include <Arduino.h>

// ===================================================================
// TIMING UTILITIES
// ===================================================================

/**
 * Check if a timeout has occurred since a start time
 * @param startTime The time when the operation started (from millis())
 * @param timeout The timeout duration in milliseconds
 * @return true if timeout has occurred, false otherwise
 */
static inline bool hasTimedOut(unsigned long startTime, unsigned long timeout) {
    return (millis() - startTime) >= timeout;
}

/**
 * Get elapsed time since a start time
 * @param startTime The time when the operation started (from millis())
 * @return Elapsed time in milliseconds
 */
static inline unsigned long getElapsedTime(unsigned long startTime) {
    return millis() - startTime;
}

/**
 * Check if a timeout has occurred since a start time (microsecond precision)
 * @param startTime The time when the operation started (from micros())
 * @param timeout The timeout duration in microseconds
 * @return true if timeout has occurred, false otherwise
 */
static inline bool hasTimedOutMicros(unsigned long startTime, unsigned long timeout) {
    return (micros() - startTime) >= timeout;
}

/**
 * Get elapsed time since a start time (microsecond precision)
 * @param startTime The time when the operation started (from micros())
 * @return Elapsed time in microseconds
 */
static inline unsigned long getElapsedTimeMicros(unsigned long startTime) {
    return micros() - startTime;
}

/**
 * Non-blocking delay implementation
 * Use this in loops where you need to wait without blocking other operations
 * @param duration Duration to wait in milliseconds
 * @return true if delay period has completed, false if still waiting
 */
static inline bool nonBlockingDelay(unsigned long duration) {
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

/**
 * Stateful non-blocking delay for multiple instances
 * @param lastTime Pointer to store the last time (pass a static variable)
 * @param duration Duration to wait in milliseconds
 * @return true if delay period has completed, false if still waiting
 */
static inline bool nonBlockingDelayStateful(unsigned long* lastTime, unsigned long duration) {
    unsigned long currentTime = millis();
    if (currentTime - *lastTime >= duration) {
        *lastTime = currentTime;
        return true;
    }
    return false;
}

/**
 * Check if enough time has passed since last execution
 * Useful for rate limiting operations
 * @param lastTime Pointer to store the last execution time
 * @param interval Minimum interval between executions in milliseconds
 * @return true if enough time has passed, false otherwise
 */
static inline bool shouldExecute(unsigned long* lastTime, unsigned long interval) {
    unsigned long currentTime = millis();
    if (currentTime - *lastTime >= interval) {
        *lastTime = currentTime;
        return true;
    }
    return false;
}

/**
 * Measure execution time of a code block
 * Usage: 
 *   unsigned long startTime = measureStart();
 *   // ... your code ...
 *   unsigned long duration = measureEnd(startTime);
 */
static inline unsigned long measureStart() {
    return millis();
}

static inline unsigned long measureEnd(unsigned long startTime) {
    return millis() - startTime;
}

/**
 * Measure execution time with microsecond precision
 */
static inline unsigned long measureStartMicros() {
    return micros();
}

static inline unsigned long measureEndMicros(unsigned long startTime) {
    return micros() - startTime;
}

/**
 * Wait for a condition with timeout
 * @param condition Function pointer that returns bool
 * @param timeout Timeout in milliseconds
 * @return true if condition became true, false if timeout occurred
 */
static inline bool waitForCondition(bool (*condition)(), unsigned long timeout) {
    unsigned long startTime = millis();
    while (!condition()) {
        if (hasTimedOut(startTime, timeout)) {
            return false;
        }
        delay(1); // Small delay to prevent busy waiting
    }
    return true;
}

/**
 * Debounce helper for input signals
 * @param currentState Current state of the input
 * @param lastState Pointer to store the last stable state
 * @param lastChangeTime Pointer to store the last change time
 * @param debounceDelay Debounce delay in milliseconds
 * @return true if state has changed and is stable
 */
static inline bool debounce(bool currentState, bool* lastState, unsigned long* lastChangeTime, unsigned long debounceDelay) {
    if (currentState != *lastState) {
        *lastChangeTime = millis();
        *lastState = currentState;
    }
    
    return (millis() - *lastChangeTime) > debounceDelay;
}

/**
 * Calculate time remaining until timeout
 * @param startTime The time when the operation started
 * @param timeout The total timeout duration
 * @return Time remaining in milliseconds (0 if timeout has occurred)
 */
static inline unsigned long getTimeRemaining(unsigned long startTime, unsigned long timeout) {
    unsigned long elapsed = getElapsedTime(startTime);
    return (elapsed >= timeout) ? 0 : (timeout - elapsed);
}

/**
 * Check if we're within a time window
 * @param startTime Start of the time window
 * @param windowDuration Duration of the time window in milliseconds
 * @return true if current time is within the window
 */
static inline bool isWithinTimeWindow(unsigned long startTime, unsigned long windowDuration) {
    return getElapsedTime(startTime) <= windowDuration;
}

/**
 * Throttle function calls to a maximum rate
 * @param lastCallTime Pointer to store the last call time
 * @param minInterval Minimum interval between calls in milliseconds
 * @return true if call should proceed, false if throttled
 */
static inline bool throttle(unsigned long* lastCallTime, unsigned long minInterval) {
    unsigned long currentTime = millis();
    if (currentTime - *lastCallTime >= minInterval) {
        *lastCallTime = currentTime;
        return true;
    }
    return false;
}

// Timing constants for common operations
#define TIMING_IMMEDIATE 0
#define TIMING_VERY_SHORT 10      // 10ms
#define TIMING_SHORT 100          // 100ms
#define TIMING_MEDIUM 1000        // 1 second
#define TIMING_LONG 5000          // 5 seconds
#define TIMING_VERY_LONG 30000    // 30 seconds

// Timeout constants for common operations
#define TIMEOUT_IMMEDIATE 0
#define TIMEOUT_QUICK 100        // 100ms
#define TIMEOUT_SHORT 1000       // 1 second
#define TIMEOUT_MEDIUM 5000      // 5 seconds
#define TIMEOUT_LONG 30000       // 30 seconds
#define TIMEOUT_VERY_LONG 60000  // 1 minute

#endif // TIMING_H 