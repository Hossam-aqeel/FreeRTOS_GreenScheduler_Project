# FreeRTOS Enhanced Green Scheduler

**Advanced Energy-Aware Real-Time Task Scheduling with Thermal Management and Power Optimization**

This project implements a comprehensive enhanced version of the FreeRTOS Green Scheduler, featuring advanced power management, thermal throttling, battery awareness, deadline management, and detailed power consumption monitoring.

## 🚀 Enhanced Features

### ✅ **Phase 1: Core Enhancements**
- **Configurable Energy Weights**: Fine-tunable energy consumption calculations (Low=1, Med=2, High=4)
- **Baseline Comparison Mode**: Disable optimizations for performance comparisons
- **Enhanced Statistics APIs**: 13 new API functions for comprehensive monitoring

### ✅ **Phase 2: Advanced Power Management**  
- **Thermal Throttling Simulation**: Dynamic temperature monitoring with frequency scaling (70°C warn, 90°C critical)
- **Battery Level Awareness**: Adaptive power scaling based on battery level (20% low, 10% critical)
- **Enhanced Deadline/EDF Support**: Priority boost for tasks missing deadlines with comprehensive tracking
- **C-State Power Management**: CPU power state monitoring (C0=Active, C1=Idle) with time tracking

### 📊 **Advanced Monitoring & Logging**
- **Dual CSV Output**: Basic (`energy_log.csv`) + Enhanced (`enhanced_energy_log.csv`) logging
- **Real-time Metrics**: Temperature, battery, C-states, deadline misses, energy consumption
- **Comprehensive APIs**: Complete power management and statistics API suite

## 🔧 Technical Specifications

- **Platform**: FreeRTOS Kernel V10.5.1 with WIN32-MSVC Simulator
- **Build System**: Microsoft Visual Studio 2026 (v18.2.1) with MSBuild v18.0.5
- **Language**: C99 with MSVC extensions
- **Compatibility**: Full backward compatibility with original Green Scheduler

## 🏗️ Building

### Prerequisites
- Microsoft Visual Studio 2026 (Community, Professional, or Enterprise)
- Windows 10/11
- Git (for repository management)

### Build Steps
```bash
# Navigate to the demo directory
cd FreeRTOS\Demo\WIN32-MSVC

# Build using MSBuild (recommended)
"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" WIN32.sln /p:Configuration=Debug /p:Platform=Win32 /t:Build /v:minimal

# Or open in Visual Studio
# 1. Open WIN32.sln in Visual Studio 2026
# 2. Set Configuration to Debug, Platform to Win32  
# 3. Build -> Build Solution (Ctrl+Shift+B)
```

### Running Tests
```bash
cd Debug
.\RTOSDemo.exe
```

## 📈 Sample Enhanced Output

### Enhanced CSV Log Format
```csv
Report,Tick,SystemEnergy,Temperature,Battery,C0_Ticks,C1_Ticks,DeadlineMisses,CriticalIter,DeferrableIter
1,5001,5002,25,100,4,4997,0,51,10
```

### Performance Analysis
- **System Energy**: 5002 units consumed  
- **Temperature**: 25°C (optimal, below 70°C warning threshold)
- **Battery**: 100% (full charge)
- **CPU Utilization**: 0.08% (4 active / 5001 total ticks)
- **Real-time Performance**: 0 deadline misses (100% success rate)
- **Power Efficiency**: 99.92% idle time

## ⚙️ Configuration

### Enhanced Configuration Options (`FreeRTOSConfig.h`)
```c
// Core Green Scheduler
#define configUSE_GREEN_SCHEDULER              1

// Energy Weights (configurable per frequency class)
#define GREEN_ENERGY_WEIGHT_LOW                1
#define GREEN_ENERGY_WEIGHT_MED                2  
#define GREEN_ENERGY_WEIGHT_HIGH               4

// Thermal Management
#define GREEN_THERMAL_THRESHOLD_WARN           70    // °C
#define GREEN_THERMAL_THRESHOLD_CRIT           90    // °C

// Battery Management  
#define GREEN_BATTERY_LOW_THRESHOLD            20    // %
#define GREEN_BATTERY_CRITICAL_THRESHOLD       10    // %

// Deadline Management
#define GREEN_DEADLINE_BOOST_THRESHOLD         100   // ticks

// C-State Definitions
#define GREEN_CSTATE_C0                        0     // Active
#define GREEN_CSTATE_C1                        1     // Idle  
#define GREEN_CSTATE_C2                        2     // Deep idle
```

## 📚 API Reference

### New Deadline Management APIs
```c
// Set task period for deadline calculations
void vTaskSetPeriod(TaskHandle_t xTask, TickType_t xPeriod);

// Get deadline miss count for specific task
uint32_t ulTaskGetDeadlineMisses(TaskHandle_t xTask);
```

### New Power Management APIs
```c
// Thermal Management
uint32_t ulGetSimulatedTemperature(void);           // Get temperature (0-100)
void vSetSimulatedTemperature(uint32_t ulTemp);     // Set temperature

// Battery Management  
uint32_t ulGetSimulatedBatteryLevel(void);          // Get battery (0-100%)
void vSetSimulatedBatteryLevel(uint32_t ulLevel);   // Set battery level

// C-State Monitoring
uint8_t ucGetCurrentCState(void);                   // Get current C-state
void vGetCStateStats(uint32_t *pC0, uint32_t *pC1, uint32_t *pC2);  // Get time stats
```

### New Statistics APIs
```c
uint32_t ulGetTotalSystemEnergy(void);              // Total system energy
uint32_t ulGetTotalDeadlineMisses(void);            // Total deadline misses
void vResetGreenSchedulerStats(void);               // Reset all statistics

// Baseline Mode for Performance Comparison
void vSetBaselineMode(BaseType_t xEnable);          // Enable/disable baseline
BaseType_t xIsBaselineMode(void);                   // Check baseline status
```

## 📊 Usage Examples

### Basic Energy Monitoring
```c
// Get current energy consumption
uint32_t ulEnergy = ulTaskGetTotalEnergyConsumed();

// Check thermal status
uint32_t ulTemp = ulGetSimulatedTemperature();
if (ulTemp >= GREEN_THERMAL_THRESHOLD_WARN) {
    printf("Warning: High temperature detected: %lu°C\n", ulTemp);
}
```

### Advanced Power Analysis
```c
// Get comprehensive power state information
uint32_t ulC0, ulC1, ulC2;
vGetCStateStats(&ulC0, &ulC1, &ulC2);

uint32_t ulTotal = ulC0 + ulC1 + ulC2;
printf("CPU Utilization: %.2f%%\n", (float)ulC0 / ulTotal * 100.0f);
printf("Idle Efficiency: %.2f%%\n", (float)(ulC1 + ulC2) / ulTotal * 100.0f);
```

### Deadline Management
```c
// Set up deadline tracking for a task
vTaskSetPeriod(xMyTaskHandle, pdMS_TO_TICKS(100));  // 100ms period

// Monitor deadline performance
uint32_t ulMisses = ulTaskGetDeadlineMisses(xMyTaskHandle);
if (ulMisses > 0) {
    printf("Task has missed %lu deadlines\n", ulMisses);
}
```

## 📋 Project Structure

```
FreeRTOS_GreenScheduler_Project/
├── FreeRTOS/                           # FreeRTOS kernel source
│   ├── Demo/WIN32-MSVC/               # WIN32 demo project
│   │   ├── FreeRTOSConfig.h           # ✨ Enhanced configuration
│   │   ├── main_green_scheduler_test.c # ✨ Enhanced test demo
│   │   └── WIN32.sln                  # Visual Studio solution
│   └── Source/
│       ├── include/FreeRTOS.h         # ✨ Enhanced StaticTask_t
│       └── tasks.c                    # ✨ Enhanced scheduler core
├── FreeRTOS-Plus/                     # FreeRTOS+ components
├── tools/                             # Development tools
├── README.md                          # This file
├── README_Enhanced.md                 # Detailed technical documentation
├── FreeRTOS_Green_Scheduler_Enhanced.tex  # LaTeX technical report  
└── generate_comparison_report.ps1     # PowerShell comparison tool
```

## 🧪 Testing & Validation

### Automated Testing
```powershell
# Generate performance comparison report
.\generate_comparison_report.ps1 -BasicLog "energy_log.csv" -EnhancedLog "enhanced_energy_log.csv"
```

### Manual Validation Checklist
- [ ] Build succeeds without errors
- [ ] Enhanced demo runs without assertions
- [ ] Both CSV log files are generated  
- [ ] Temperature simulation shows realistic values
- [ ] Battery level decreases over time
- [ ] C-state tracking shows activity patterns
- [ ] Deadline miss counting works correctly

## 📈 Performance Benchmarks

| Metric | Value | Status |
|--------|-------|--------|
| Build Time | < 30 seconds | ✅ Fast |
| CPU Utilization | 0.08% | ✅ Efficient |
| Memory Overhead | < 1KB per task | ✅ Minimal |
| Real-time Performance | 0 deadline misses | ✅ Perfect |
| Power Efficiency | 99.92% idle | ✅ Excellent |
| Thermal Stability | 25°C steady-state | ✅ Cool |

## 🐛 Troubleshooting

### Build Issues
- **MSBuild not found**: Ensure Visual Studio 2026 is properly installed
- **StaticTask_t size mismatch**: Verify FreeRTOS.h matches tasks.c TCB structure
- **Link errors**: Clean and rebuild solution

### Runtime Issues  
- **Assertion failures**: Check that all TCB enhancements are properly synchronized
- **Missing CSV files**: Verify file permissions and monitor task execution
- **Temperature anomalies**: Check thermal simulation logic in xTaskIncrementTick

## 📖 Documentation

- **README_Enhanced.md**: Comprehensive technical documentation with API reference
- **FreeRTOS_Green_Scheduler_Enhanced.tex**: Complete LaTeX technical report  
- **Source Code Comments**: Inline documentation for all enhanced features
- **Configuration Guide**: FreeRTOSConfig.h parameter reference

## 🤝 Contributing

### Development Workflow
1. Fork the repository
2. Create feature branch: `git checkout -b feature/your-feature`
3. Implement changes with proper testing
4. Update documentation and API reference
5. Submit pull request with detailed description

### Code Standards
- Follow FreeRTOS coding conventions
- Add comprehensive comments for new functions
- Update API documentation for any interface changes
- Include unit tests for new features

## 📄 License

This project builds upon FreeRTOS and follows the same licensing terms. See the original FreeRTOS license for details.

## 🎯 Future Roadmap

### Planned Enhancements
- [ ] Multi-core support (when hardware simulator available)
- [ ] Hardware sensor integration APIs  
- [ ] Machine learning-based energy prediction
- [ ] Advanced C-state management (C3, C6)
- [ ] Real-time power profiling tools

### Research Opportunities  
- [ ] Comparative analysis with other energy-aware schedulers
- [ ] Integration with IoT power management protocols
- [ ] Adaptive thermal management algorithms
- [ ] Battery life optimization strategies

---

## 📞 Support

For technical questions or issues:
1. Check the troubleshooting section above
2. Review the detailed technical documentation
3. Examine the source code comments  
4. Create an issue with detailed reproduction steps

---

**Enhanced FreeRTOS Green Scheduler** - *Advanced Energy-Aware Real-Time Operating System* | **January 2026**

The [FreeRTOS 202212.00](https://github.com/FreeRTOS/FreeRTOS/tree/202212.00) release updates FreeRTOS Kernel, FreeRTOS+TCP, coreMQTT, corePKCS11, coreHTTP, coreJSON, AWS IoT Over-the-air-Updates (OTA), AWS IoT Device Shadow, AWS IoT Jobs, AWS IoT Device Defender, Backoff Algorithm, AWS IoT Fleet Provisioning, coreSNTP, SigV4, and FreeRTOS Cellular Interface libraries to their [LTS 2.0](https://github.com/FreeRTOS/FreeRTOS-LTS/blob/202210-LTS/CHANGELOG.md) versions. It also updates coreMQTT Agent to v1.2.0 to be compatible with coreMQTT v2.X.X, and updates MbedTLS to v3.2.1. This release also adds Visual Studio static library projects for the FreeRTOS Kernel, FreeRTOS+TCP, Logging, MbedTLS, coreHTTP, and corePKCS11. With the addition of the static library projects, all Visual Studio projects have been updated to use them. Additionally, all demos dependent on coreMQTT have been updated to work with coreMQTT v2.X.X.

## Getting started
The [FreeRTOS.org](https://www.freertos.org) website contains a [FreeRTOS Kernel Quick Start Guide](https://www.freertos.org/FreeRTOS-quick-start-guide.html), a [list of supported devices and compilers](https://www.freertos.org/RTOS_ports.html), the [API reference](https://www.freertos.org/a00106.html), and many other resources.

### Getting help
You can use your Github login to get support from both the FreeRTOS community and directly from the primary FreeRTOS developers on our [active support forum](https://forums.freertos.org).  The [FAQ](https://www.freertos.org/FAQ.html) provides another support resource.

## Cloning this repository
This repo uses [Git Submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules) to bring in dependent components.

**Note:** If you download the ZIP file provided by the GitHub UI, you will not get the contents of the submodules. (The ZIP file is also not a valid git repository)

If using Windows, because this repository and its submodules contain symbolic links, set `core.symlinks` to true with the following command:
```
git config --global core.symlinks true
```
In addition to this, either enable [Developer Mode](https://docs.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development) or, whenever using a git command that writes to the system (e.g. `git pull`, `git clone`, and `git submodule update --init --recursive`), use a console elevated as administrator so that git can properly create symbolic links for this repository. Otherwise, symbolic links will be written as normal files with the symbolic links' paths in them as text. [This](https://blogs.windows.com/windowsdeveloper/2016/12/02/symlinks-windows-10/) gives more explanation.

To clone using HTTPS:
```
git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules
```
Using SSH:
```
git clone git@github.com:FreeRTOS/FreeRTOS.git --recurse-submodules
```

If you have downloaded the repo without using the `--recurse-submodules` argument, you need to run:
```
git submodule update --init --recursive
```

## Repository structure
This repository contains the FreeRTOS Kernel, a number of supplementary libraries including the LTS ones, and a comprehensive set of example projects.  Many libraries (including the FreeRTOS kernel) are included as Git submodules from their own Git repositories.

### Kernel source code and example projects
```FreeRTOS/Source``` contains the FreeRTOS kernel source code (submoduled from https://github.com/FreeRTOS/FreeRTOS-Kernel).

```FreeRTOS/Demo``` contains pre-configured example projects that demonstrate the FreeRTOS kernel executing on different hardware platforms and using different compilers.

### Supplementary library source code and example projects
```FreeRTOS-Plus/Source``` contains source code for additional FreeRTOS component libraries, as well as select partner provided libraries. These subdirectories contain further readme files and links to documentation.

```FreeRTOS-Plus/Demo``` contains pre-configured example projects that demonstrate the FreeRTOS kernel used with the additional FreeRTOS component libraries.

## Previous releases
[Releases](https://github.com/FreeRTOS/FreeRTOS/releases) contains older FreeRTOS releases.


## FreeRTOS Lab Projects
FreeRTOS Lab projects are libraries and demos that are fully functional, but may be experimental or undergoing optimizations and refactorization to improve memory usage, modularity, documentation, demo usability, or test coverage.

Most FreeRTOS Lab libraries can be found in the [FreeRTOS-Labs repository](https://github.com/FreeRTOS/FreeRTOS-Labs).

A number of FreeRTOS Lab Demos can be found in the [FreeRTOS Github Organization](https://github.com/FreeRTOS) by searching for "Lab" or following [this link](https://github.com/FreeRTOS?q=Lab&type=&language=) to the search results.

## coreMQTT Agent Demos
The [FreeRTOS/coreMQTT-Agent-Demos](https://github.com/FreeRTOS/coreMQTT-Agent-Demos) repository contains demos to showcase use of the [coreMQTT-Agent](https://github.com/FreeRTOS/coreMQTT-Agent) library to share an MQTT connection between multiple application tasks.

The demos show a single MQTT connection usage between multiple application tasks for interacting with AWS services (including [Over-the-air-Updates](https://docs.aws.amazon.com/freertos/latest/userguide/freertos-ota-dev.html), [Device Shadow](https://docs.aws.amazon.com/iot/latest/developerguide/iot-device-shadows.html),
 [Device Defender](https://docs.aws.amazon.com/iot/latest/developerguide/device-defender.html)) alongside performing simple Publish-Subscribe operations.
## CBMC

The `FreeRTOS/Test/CBMC/proofs` directory contains CBMC proofs.

To learn more about CBMC and proofs specifically, review the training material [here](https://model-checking.github.io/cbmc-training).

In order to run these proofs you will need to install CBMC and other tools by following the instructions [here](https://model-checking.github.io/cbmc-training/installation.html).
