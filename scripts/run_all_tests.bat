@echo off
REM DongArch3D - Run All Tests (Windows)
REM Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute

echo ==========================================
echo DongArch3D v4.0 - Test Runner
echo ==========================================

REM Find project root
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..

echo Project Root: %PROJECT_ROOT%

REM Check build directory
set BUILD_DIR=%PROJECT_ROOT%\GigaMesh\build_korean

if not exist "%BUILD_DIR%" (
    echo Error: Build directory not found
    echo Please run CMake first:
    echo   mkdir GigaMesh\build_korean
    echo   cd GigaMesh\build_korean
    echo   cmake .. -G "Visual Studio 16 2019" -A x64
    echo   cmake --build . --config Release
    exit /b 1
)

cd /d "%BUILD_DIR%"

REM Step 1: Build tests
echo.
echo Step 1: Building tests...
cmake --build . --config Release --target test_dongarch

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo [OK] Build successful

REM Step 2: Run CTest
echo.
echo Step 2: Running CTest...
ctest -C Release --output-on-failure --verbose

if %ERRORLEVEL% neq 0 (
    echo Tests failed!
    exit /b 1
)

echo [OK] All tests passed

REM Summary
echo.
echo ==========================================
echo All tests completed successfully!
echo ==========================================

pause
