@echo off
REM ========================================
REM DongArch3D Windows 클린 재빌드 스크립트
REM 버전: 4.0.0
REM ========================================

echo.
echo ================================================
echo DongArch3D v4.0 Clean Rebuild (Windows)
echo ================================================
echo.

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
cd /d %PROJECT_ROOT%

set BUILD_DIR=GigaMesh\build_windows

REM ========================================
REM 1. 기존 빌드 삭제
REM ========================================
echo [1/2] 기존 빌드 디렉토리 삭제...
if exist %BUILD_DIR% (
    echo     - 삭제 중: %BUILD_DIR%
    rmdir /s /q %BUILD_DIR%
    echo     ✅ 삭제 완료
) else (
    echo     ℹ️ 빌드 디렉토리 없음
)
echo.

REM ========================================
REM 2. 새로 빌드
REM ========================================
echo [2/2] 새로운 빌드 시작...
call %SCRIPT_DIR%build_windows.bat

exit /b %errorlevel%
