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
bool hasTimedOut(unsigned long startTime, unsigned long timeout);

/**
 * Get elapsed time since a start time
 * @param startTime The time when the operation started (from millis())
 * @return Elapsed time in milliseconds
 */
unsigned long getElapsedTime(unsigned long startTime);

/**
 * Check if a timeout has occurred since a start time (microsecond precision)
 * @param startTime The time when the operation started (from micros())
 * @param timeout The timeout duration in microseconds
 * @return true if timeout has occurred, false otherwise
 */
bool hasTimedOutMicros(unsigned long startTime, unsigned long timeout);

/**
 * Get elapsed time since a start time (microsecond precision)
 * @param startTime The time when the operation started (from micros())
 * @return Elapsed time in microseconds
 */
unsigned long getElapsedTimeMicros(unsigned long startTime);

/**
 * Non-blocking delay implementation
 * Use this in loops where you need to wait without blocking other operations
 * @param duration Duration to wait in milliseconds
 * @return true if delay period has completed, false if still waiting
 */
bool nonBlockingDelay(unsigned long duration);

/**
 * Stateful non-blocking delay for multiple instances
 * @param lastTime Pointer to store the last time (pass a static variable)
 * @param duration Duration to wait in milliseconds
 * @return true if delay period has completed, false if still waiting
 */
bool nonBlockingDelayStateful(unsigned long* lastTime, unsigned long duration);

/**
 * Check if enough time has passed since last execution
 * Useful for rate limiting operations
 * @param lastTime Pointer to store the last execution time
 * @param interval Minimum interval between executions in milliseconds
 * @return true if enough time has passed, false otherwise
 */
bool shouldExecute(unsigned long* lastTime, unsigned long interval);

/**
 * Measure execution time of a code block
 * Usage: 
 *   unsigned long startTime = measureStart();
 *   // ... your code ...
 *   unsigned long duration = measureEnd(startTime);
 */
unsigned long measureStart();

unsigned long measureEnd(unsigned long startTime);

/**
 * Measure execution time with microsecond precision
 */
unsigned long measureStartMicros();

unsigned long measureEndMicros(unsigned long startTime);

/**
 * Wait for a condition with timeout
 * @param condition Function pointer that returns bool
 * @param timeout Timeout in milliseconds
 * @return true if condition became true, false if timeout occurred
 */
bool waitForCondition(bool (*condition)(), unsigned long timeout);

/**
 * Debounce helper for input signals
 * @param currentState Current state of the input
 * @param lastState Pointer to store the last stable state
 * @param lastChangeTime Pointer to store the last change time
 * @param debounceDelay Debounce delay in milliseconds
 * @return true if state has changed and is stable
 */
bool debounce(bool currentState, bool* lastState, unsigned long* lastChangeTime, unsigned long debounceDelay);

/**
 * Calculate time remaining until timeout
 * @param startTime The time when the operation started
 * @param timeout The total timeout duration
 * @return Time remaining in milliseconds (0 if timeout has occurred)
 */
unsigned long getTimeRemaining(unsigned long startTime, unsigned long timeout);

/**
 * Check if we're within a time window
 * @param startTime Start of the time window
 * @param windowDuration Duration of the time window in milliseconds
 * @return true if current time is within the window
 */
bool isWithinTimeWindow(unsigned long startTime, unsigned long windowDuration);

/**
 * Throttle function calls to a maximum rate
 * @param lastCallTime Pointer to store the last call time
 * @param minInterval Minimum interval between calls in milliseconds
 * @return true if call should proceed, false if throttled
 */
bool throttle(unsigned long* lastCallTime, unsigned long minInterval);

/**
 * Wait for a condition to become true with timeout
 * @param condition Function pointer to condition check
 * @param timeout Maximum time to wait in milliseconds
 * @return true if condition became true, false if timeout occurred
 */
bool waitForCondition(bool (*condition)(), unsigned long timeout);

/**
 * Debounce a boolean state change
 * @param currentState Current state to check
 * @param lastState Pointer to store the last state
 * @param lastChangeTime Pointer to store the last change time
 * @param debounceDelay Debounce delay in milliseconds
 * @return true if state has changed and is stable
 */
bool debounce(bool currentState, bool* lastState, unsigned long* lastChangeTime, unsigned long debounceDelay);

/**
 * Get remaining time before timeout
 * @param startTime Start time of the operation
 * @param timeout Total timeout duration
 * @return Time remaining in milliseconds (0 if timeout has occurred)
 */
unsigned long getTimeRemaining(unsigned long startTime, unsigned long timeout);

/**
 * Check if current time is within a time window
 * @param startTime Start time of the window
 * @param windowDuration Duration of the window
 * @return true if current time is within the window
 */
bool isWithinTimeWindow(unsigned long startTime, unsigned long windowDuration);

// ===================================================================
// TIMING CONSTANTS
// ===================================================================

// Common timing intervals for use with the timing functions
#define TIMING_VERY_SHORT   100   // 100ms
#define TIMING_SHORT        250   // 250ms
#define TIMING_MEDIUM       500   // 500ms
#define TIMING_LONG         1000  // 1 second
#define TIMING_VERY_LONG    5000  // 5 seconds

// Timeout constants for common operations
#define TIMEOUT_IMMEDIATE 0
#define TIMEOUT_QUICK 100        // 100ms
#define TIMEOUT_SHORT 1000       // 1 second
#define TIMEOUT_MEDIUM 5000      // 5 seconds
#define TIMEOUT_LONG 30000       // 30 seconds
#define TIMEOUT_VERY_LONG 60000  // 1 minute

#endif // TIMING_H 