@echo off
echo ================================================
echo    FreeRTOS GREEN SCHEDULER TEST
echo ================================================
echo.
cd /d "%~dp0Debug"
echo Running from: %CD%
echo.
RTOSDemo.exe
echo.
echo ================================================
echo Demo ended.
echo ================================================
pause
