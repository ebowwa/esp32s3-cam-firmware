# Centralized Cycle Management System

The ESP32S3 camera firmware now includes a comprehensive centralized cycle management system that unifies all periodic operations, timing-based tasks, and state management into a single, efficient framework.

## Overview

The cycle manager provides a unified approach to handling all types of recurring operations in the firmware:

- **Interval-based cycles** - Execute at regular time intervals
- **Condition-based cycles** - Execute when specific conditions are met
- **Pattern-based cycles** - Follow predefined patterns (LED sequences, etc.)
- **Timeout-based cycles** - Execute after a timeout period (one-shot or recurring)
- **Circular buffer cycles** - Manage circular buffer operations
- **State machine cycles** - Handle complex state transitions

## Benefits

### 🎯 **Unified Management**
- Single point of control for all periodic operations
- Consistent timing and execution model
- Centralized statistics and monitoring

### ⚡ **Performance Optimized**
- Priority-based execution ensures critical tasks run first
- Efficient scheduling reduces CPU overhead
- Non-blocking execution prevents system lockups

### 🔧 **Maintainable**
- Clear separation of concerns
- Easy to add, modify, or remove cycles
- Comprehensive debugging and monitoring tools

### 📊 **Observable**
- Detailed execution statistics
- Performance monitoring
- Error tracking and reporting

## Architecture

### Core Components

```cpp
// Cycle execution modes
typedef enum {
    CYCLE_MODE_INTERVAL,        // Execute at regular intervals
    CYCLE_MODE_TIMEOUT,         // Execute after timeout
    CYCLE_MODE_CONDITION,       // Execute when condition is met
    CYCLE_MODE_PATTERN,         // Execute following a pattern
    CYCLE_MODE_CIRCULAR_BUFFER, // Manage circular buffer updates
    CYCLE_MODE_STATE_MACHINE    // State machine transitions
} cycle_mode_t;

// Priority levels
typedef enum {
    CYCLE_PRIORITY_CRITICAL = 0,    // Always execute (LED updates, safety)
    CYCLE_PRIORITY_HIGH = 1,        // High priority (BLE, audio)
    CYCLE_PRIORITY_NORMAL = 2,      // Normal priority (photos, video)
    CYCLE_PRIORITY_LOW = 3,         // Low priority (statistics, cleanup)
    CYCLE_PRIORITY_BACKGROUND = 4   // Background tasks (history, logging)
} cycle_priority_t;
```

### Specialized Managers

The system includes specialized managers for different functional areas:

- **ChargingCycles** - Battery charging and safety monitoring
- **LEDCycles** - LED pattern management and updates
- **PowerCycles** - Power management and battery monitoring
- **DataCycles** - Audio, photo, and video data acquisition
- **CommCycles** - BLE communication and data transmission

## Usage Examples

### Basic Interval Cycle

```cpp
// Register a cycle that runs every 5 seconds
int cycle_id = registerIntervalCycle(
    "MyTask",
    5000, // 5 seconds
    []() {
        Serial.println("Task executed!");
    },
    CYCLE_PRIORITY_NORMAL
);
```

### Condition-Based Cycle

```cpp
// Register a cycle that runs when a condition is met
int cycle_id = registerConditionCycle(
    "ConditionalTask",
    []() {
        return digitalRead(BUTTON_PIN) == LOW; // Condition
    },
    []() {
        Serial.println("Button pressed!");
    },
    CYCLE_PRIORITY_HIGH
);
```

### Pattern-Based Cycle (LED)

```cpp
// Define LED pattern
pattern_step_t led_pattern[] = {
    {500, 255, true},   // 500ms ON
    {500, 0, false},    // 500ms OFF
    {200, 255, true},   // 200ms ON
    {1000, 0, false}    // 1000ms OFF
};

// Register pattern cycle
int cycle_id = registerPatternCycle(
    "LEDPattern",
    led_pattern,
    4, // Number of steps
    []() {
        // Update LED based on current pattern step
        updateLEDFromPattern();
    },
    CYCLE_PRIORITY_CRITICAL
);
```

### Timeout Cycle (One-shot)

```cpp
// Register a one-shot cycle that runs after 10 seconds
int cycle_id = registerTimeoutCycle(
    "DelayedTask",
    10000, // 10 seconds
    []() {
        Serial.println("Delayed task executed!");
    },
    CYCLE_PRIORITY_LOW
);
```

## Integration in Main Firmware

### Setup Phase

```cpp
void setup() {
    // ... other initialization ...
    
    // Initialize cycle manager
    initializeCycleManager();
    
    // ... device setup ...
    
    // Initialize specialized cycle managers
    ChargingCycles::initialize();
    LEDCycles::initialize();
    PowerCycles::initialize();
    DataCycles::initialize();
    CommCycles::initialize();
}
```

### Main Loop

```cpp
void loop() {
    // Update all cycles using centralized manager
    updateCycles();
    
    // Handle any additional main loop tasks
    handleSpecialCases();
    
    // Small delay to prevent excessive CPU usage
    delay(MAIN_LOOP_DELAY);
}
```

## Cycle Management

### Control Functions

```cpp
// Enable/disable a cycle
setCycleEnabled(cycle_id, true);

// Pause/resume a cycle
setCyclePaused(cycle_id, true);

// Get cycle state
cycle_state_t state = getCycleState(cycle_id);

// Get cycle statistics
const cycle_runtime_t* stats = getCycleStats(cycle_id);
```

### Monitoring and Debugging

```cpp
// Print overall statistics
printCycleManagerStats();

// Print individual cycle statistics
printCycleStats(cycle_id);

// Reset statistics
resetCycleStats(cycle_id);
```

## Specialized Cycle Managers

### Charging Cycles

Manages all battery charging related operations:

- **ChargingStatus** - Updates charging state every 5 seconds
- **ChargingHistory** - Records charging cycles when complete
- **ChargingSafety** - Monitors safety parameters every second

```cpp
ChargingCycles::initialize();
```

### LED Cycles

Handles LED pattern updates and visual feedback:

- **LEDUpdate** - Updates LED patterns every 20ms for smooth animation
- **PatternCycles** - Manages complex LED sequences

```cpp
LEDCycles::initialize();
```

### Power Cycles

Manages power consumption and battery monitoring:

- **BatteryUpdate** - Updates battery level and power statistics
- **PowerStats** - Prints power statistics periodically
- **SleepManagement** - Handles power saving and sleep modes

```cpp
PowerCycles::initialize();
```

### Data Cycles

Handles data acquisition from sensors and camera:

- **AudioCapture** - Continuous audio recording and transmission
- **PhotoCapture** - Photo capture based on user requests
- **VideoStream** - Video streaming at specified frame rates

```cpp
DataCycles::initialize();
```

### Communication Cycles

Manages BLE communication and data transmission:

- **BLEUpdate** - Updates BLE characteristics and notifications
- **DataTransmission** - Handles photo/video data transmission

```cpp
CommCycles::initialize();
```

## Performance Characteristics

### Execution Model

- **Priority-based scheduling** ensures critical tasks run first
- **Non-blocking execution** prevents system lockups
- **Efficient timing** using millisecond precision
- **Error handling** with automatic recovery

### Statistics Tracking

Each cycle tracks:
- Execution count
- Error count
- Total execution time
- Maximum execution time
- Average execution time
- Current state

### Memory Usage

- **Fixed memory allocation** - no dynamic allocation during runtime
- **Configurable cycle limit** - default 32 cycles maximum
- **Minimal overhead** - efficient data structures

## Configuration

### Cycle Limits

```cpp
#define MAX_CYCLES 32  // Maximum number of managed cycles
```

### Priority Levels

Cycles are executed in strict priority order:
1. **CRITICAL** - Safety checks, LED updates
2. **HIGH** - BLE communication, audio capture
3. **NORMAL** - Photo/video capture
4. **LOW** - Statistics, cleanup
5. **BACKGROUND** - History logging, diagnostics

## Migration from Legacy Code

### Before (Legacy Approach)

```cpp
void loop() {
    updateLed();
    
    if (shouldExecute(&lastBatteryUpdate, BATTERY_UPDATE_INTERVAL)) {
        updateBatteryLevel();
    }
    
    if (isCapturingPhotos && hasTimedOut(lastCaptureTime, captureInterval)) {
        take_photo();
    }
    
    // ... many more timing checks ...
}
```

### After (Centralized Approach)

```cpp
void loop() {
    updateCycles(); // Handles all timing and execution
    delay(MAIN_LOOP_DELAY);
}
```

## Error Handling

The cycle manager includes comprehensive error handling:

- **Execution errors** are caught and logged
- **Error counters** track problematic cycles
- **Error callbacks** allow custom error handling
- **Automatic recovery** for transient errors

## Testing and Validation

Use the provided test sketch to validate cycle manager functionality:

```cpp
#include "test_cycle_manager.ino"
```

The test includes:
- Interval cycle testing
- Condition cycle testing
- Pattern cycle testing
- Timeout cycle testing
- Performance benchmarking
- Interactive debugging commands

## Best Practices

### Cycle Design

1. **Keep execution time short** - avoid blocking operations
2. **Use appropriate priorities** - critical tasks should be CRITICAL priority
3. **Handle errors gracefully** - provide error callbacks
4. **Monitor performance** - use statistics to optimize

### Naming Conventions

- Use descriptive names: "BatteryUpdate", "LEDPattern", "AudioCapture"
- Include component prefix: "Charging_Status", "LED_Update"
- Be consistent across related cycles

### Performance Optimization

- Group related operations into single cycles when possible
- Use condition-based cycles for event-driven tasks
- Avoid unnecessary work in condition functions
- Monitor execution times and optimize slow cycles

## Future Enhancements

Planned improvements include:

- **Dynamic cycle registration** during runtime
- **Cycle dependencies** and execution ordering
- **Advanced scheduling algorithms**
- **Real-time performance monitoring**
- **Cycle load balancing**
- **Power-aware scheduling**

## Troubleshooting

### Common Issues

**Cycle not executing:**
- Check if cycle is enabled: `getCycleState(cycle_id)`
- Verify condition function returns true
- Check priority level and execution order

**Performance issues:**
- Monitor execution times: `printCycleStats(cycle_id)`
- Reduce cycle frequency if needed
- Optimize execution function

**Memory issues:**
- Check cycle count: `printCycleManagerStats()`
- Verify MAX_CYCLES limit not exceeded
- Look for memory leaks in execution functions

### Debug Commands

```cpp
printCycleManagerStats();     // Overall statistics
printCycleStats(cycle_id);    // Individual cycle stats
resetCycleStats(cycle_id);    // Reset statistics
setCycleEnabled(cycle_id, false); // Disable problematic cycle
```

The centralized cycle management system provides a robust, efficient, and maintainable foundation for all periodic operations in the ESP32S3 camera firmware. 