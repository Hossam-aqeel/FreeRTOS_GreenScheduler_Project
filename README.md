# FreeRTOS Green Scheduler

## Project Description
This project implements a Green (Energy-Aware) Scheduler by modifying the
FreeRTOS kernel scheduler. The scheduler incorporates task deadlines,
slack-time analysis, and simulated CPU frequency scaling to reduce energy
consumption while preserving real-time constraints.

## Scope
- Kernel-level scheduler modification
- FreeRTOS Classic
- Windows FreeRTOS Simulator (MSVC)
- No hardware, no drivers
- No application-only changes

## Repository Structure
- baseline/        → Original FreeRTOS scheduler
- green_scheduler/ → Modified kernel scheduler
- profiling/       → Energy model and profiler logs
- evaluation/      → Baseline vs green scheduler comparison
- docs/            → Figures and report material

## Build Instructions
1. Install Visual Studio Build Tools (MSVC v143)
2. Open Developer Command Prompt
3. Navigate to: "baseline/FreeRTOS/Demo/WIN32-MSVC"
4. Build: "msbuild Win32.sln /p:Configuration=Debug /p:Platform=Win32"
5. Run: "Debug/RTOSDemo.exe"


## Branching Strategy
- main     → stable results
- baseline → original FreeRTOS
- dev      → green scheduler development
