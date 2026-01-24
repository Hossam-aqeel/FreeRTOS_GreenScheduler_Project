# FreeRTOS Green Scheduler - Enhanced Version

## Overview

This is an enhanced version of the FreeRTOS Green Scheduler that extends energy-aware real-time task scheduling with advanced power management, thermal management, and deadline management capabilities.

## Enhanced Features Implemented

### ✅ Phase 1: Core Enhancements

#### 1.2 Configurable Energy Weights
- **Energy weights**: Low=1, Med=2, High=4 (configurable in FreeRTOSConfig.h)
- Allows fine-tuning of energy consumption calculations per task frequency class
- **Configuration**: `GREEN_ENERGY_WEIGHT_LOW`, `GREEN_ENERGY_WEIGHT_MED`, `GREEN_ENERGY_WEIGHT_HIGH`

#### 1.3 Baseline Comparison Mode
- **Baseline mode**: Can disable Green Scheduler optimizations for comparison testing
- **APIs**: `vSetBaselineMode(BaseType_t xEnable)`, `xIsBaselineMode()`

### ✅ Phase 2: Advanced Power Management

#### 2.1 Enhanced Deadline/EDF Support
- **Deadline boost**: Tasks missing deadlines get priority boost
- **Deadline tracking**: Per-task deadline miss counters
- **APIs**: `vTaskSetPeriod()`, `ulTaskGetDeadlineMisses()`
- **Configuration**: `GREEN_DEADLINE_BOOST_THRESHOLD` (100 ticks)

#### 2.2 Thermal Throttling Simulation
- **Temperature simulation**: Gradually increases with system load
- **Thermal thresholds**: Warning (70°C), Critical (90°C)
- **Frequency scaling**: Automatic reduction when overheating
- **APIs**: `ulGetSimulatedTemperature()`, `vSetSimulatedTemperature()`
- **Configuration**: `GREEN_THERMAL_THRESHOLD_WARN`, `GREEN_THERMAL_THRESHOLD_CRIT`

#### 2.3 Battery Level Awareness
- **Battery simulation**: Depletes based on energy consumption and frequency scaling
- **Battery thresholds**: Low (20%), Critical (10%)
- **Power scaling**: Automatic frequency reduction when battery low
- **APIs**: `ulGetSimulatedBatteryLevel()`, `vSetSimulatedBatteryLevel()`
- **Configuration**: `GREEN_BATTERY_LOW_THRESHOLD`, `GREEN_BATTERY_CRITICAL_THRESHOLD`

#### 2.5 Idle Power States (C-States)
- **C-State tracking**: C0 (active), C1 (idle), C2 (deep idle)
- **Time tracking**: Monitors time spent in each power state
- **APIs**: `ucGetCurrentCState()`, `vGetCStateStats()`
- **Configuration**: `GREEN_CSTATE_C0`, `GREEN_CSTATE_C1`, `GREEN_CSTATE_C2`

## Enhanced Monitoring & Logging

### Dual CSV Output
1. **Basic log** (`energy_log.csv`): Compatible with original format
2. **Enhanced log** (`enhanced_energy_log.csv`): Includes all new metrics

### Enhanced Metrics
- **System Energy**: Total system energy consumption
- **Temperature**: Current simulated temperature (0-100)
- **Battery Level**: Current battery percentage
- **C-State Times**: Time spent in C0, C1, C2 states
- **Deadline Misses**: Total deadline misses across all tasks

## Configuration Options

### FreeRTOSConfig.h Settings
```c
/* Energy Weights */
#define GREEN_ENERGY_WEIGHT_LOW        1
#define GREEN_ENERGY_WEIGHT_MED        2  
#define GREEN_ENERGY_WEIGHT_HIGH       4

/* Thermal Management */
#define GREEN_THERMAL_THRESHOLD_WARN   70
#define GREEN_THERMAL_THRESHOLD_CRIT   90

/* Battery Management */
#define GREEN_BATTERY_LOW_THRESHOLD    20    /* % */
#define GREEN_BATTERY_CRITICAL_THRESHOLD 10  /* % */

/* Deadline Management */
#define GREEN_DEADLINE_BOOST_THRESHOLD 100   /* ticks */

/* C-States */
#define GREEN_CSTATE_C0               0    /* Active */
#define GREEN_CSTATE_C1               1    /* Idle */
#define GREEN_CSTATE_C2               2    /* Deep idle */
```

## API Reference

### Core Green Scheduler APIs (Existing)
- `ulTaskGetTotalEnergyConsumed()` - Get total system energy
- `vTaskSetGreenPriority()` - Set task energy class
- `xTaskGetSlackTime()` - Get task slack time

### New Deadline Management APIs
- `vTaskSetPeriod(TaskHandle_t xTask, TickType_t xPeriod)` - Set task period
- `ulTaskGetDeadlineMisses(TaskHandle_t xTask)` - Get deadline miss count

### New Simulation APIs
- `ulGetSimulatedTemperature()` - Get current temperature (0-100)
- `vSetSimulatedTemperature(uint32_t ulTemperature)` - Set temperature
- `ulGetSimulatedBatteryLevel()` - Get battery level (0-100%)
- `vSetSimulatedBatteryLevel(uint32_t ulLevel)` - Set battery level

### New Power State APIs
- `ucGetCurrentCState()` - Get current C-State
- `vGetCStateStats(uint32_t *pC0, uint32_t *pC1, uint32_t *pC2)` - Get time stats

### New Statistics APIs
- `ulGetTotalSystemEnergy()` - Get total system energy consumption
- `ulGetTotalDeadlineMisses()` - Get total deadline misses
- `vResetGreenSchedulerStats()` - Reset all statistics

### New Baseline Mode APIs
- `vSetBaselineMode(BaseType_t xEnable)` - Enable/disable baseline mode
- `xIsBaselineMode()` - Check if baseline mode active

## Building & Running

### Prerequisites
- Microsoft Visual Studio 2026 (v18.2.1)
- MSBuild v18.0.5

### Build Commands
```powershell
cd "FreeRTOS\Demo\WIN32-MSVC"
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" WIN32.sln /p:Configuration=Debug /p:Platform=Win32 /t:Build /v:minimal
```

### Running Tests
```powershell
cd "Debug"
.\RTOSDemo.exe
```

### Output Files
- **energy_log.csv**: Basic energy consumption data
- **enhanced_energy_log.csv**: Advanced power management metrics

## Sample Enhanced Output

```
Report,Tick,SystemEnergy,Temperature,Battery,C0_Ticks,C1_Ticks,DeadlineMisses,CriticalIter,DeferrableIter
1,5001,5002,25,100,4,4997,0,51,10
```

This shows:
- Report #1 at tick 5001
- System energy: 5002 units
- Temperature: 25°C (cool)
- Battery: 100% (full)
- Active time: 4 ticks, Idle time: 4997 ticks
- No deadline misses
- 51 critical iterations, 10 deferrable iterations

## Performance Characteristics

### Simulation Behaviors
- **Temperature**: Increases with CPU usage, decreases when idle
- **Battery**: Depletes faster at higher frequencies, affected by thermal conditions
- **C-States**: Automatic transitions based on task activity
- **Deadline Management**: Priority boost for tasks approaching deadlines

### Power Management Logic
1. **Normal operation**: Standard Green Scheduler behavior
2. **Thermal warning**: Reduce CPU frequency to 75%
3. **Thermal critical**: Reduce CPU frequency to 50%
4. **Battery low**: Reduce CPU frequency to 75%
5. **Battery critical**: Reduce CPU frequency to 50%

## Architecture

### Files Modified
- **FreeRTOSConfig.h**: Added configuration macros
- **FreeRTOS.h**: Updated StaticTask_t structure
- **tasks.c**: Enhanced TCB structure, added simulation logic, new APIs
- **main_green_scheduler_test.c**: Enhanced monitoring and logging

### Integration Points
- Integrated with existing Green Scheduler infrastructure
- Compatible with WIN32-MSVC simulator
- Maintains backward compatibility with original APIs

## Future Enhancements (Not Implemented)
- Multi-core support (incompatible with WIN32 simulator)
- Hardware integration for real thermal/battery sensors
- Machine learning-based energy prediction
- Advanced C-State management (C3, C6 states)

---

## Project Status
✅ **All feasible enhancements implemented and tested**
✅ **Enhanced monitoring working**
✅ **Dual CSV logging functional**
✅ **Build successful with all features**

For questions or support, refer to the original FreeRTOS documentation and Green Scheduler implementation notes.