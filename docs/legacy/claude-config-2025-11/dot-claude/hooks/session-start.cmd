@echo off
REM DongArch3D SessionStart Hook (Windows)
REM 간단한 환영 메시지 + 핵심 정보만 표시 (100자 이내)

setlocal enabledelayedexpansion

REM 프로젝트 루트
set PROJECT_ROOT=A:\1105

REM Progress 파일에서 Phase 정보 추출
for /f "tokens=2 delims=:, " %%a in ('findstr /C:"\"phase\"" "%PROJECT_ROOT%\.dongarch3d-progress.json"') do (
  set PHASE=%%a
  goto :phase_found
)
:phase_found

REM Phase 이름 추출
for /f "tokens=2 delims=:" %%a in ('findstr /C:"\"phase_name\"" "%PROJECT_ROOT%\.dongarch3d-progress.json"') do (
  set PHASE_NAME=%%a
  set PHASE_NAME=!PHASE_NAME:"=!
  set PHASE_NAME=!PHASE_NAME:,=!
  goto :name_found
)
:name_found

REM 완료율 추출
for /f "tokens=2 delims=:, " %%a in ('findstr /C:"\"completion_percentage\"" "%PROJECT_ROOT%\.dongarch3d-progress.json"') do (
  set COMPLETION=%%a
  goto :completion_found
)
:completion_found

REM 간단한 환영 메시지 (100자 이내)
echo DongArch3D - Phase !PHASE!: !PHASE_NAME! - !COMPLETION!%% 완료

REM 상세 로그는 파일에 기록
echo === Session Start: %date% %time% === >> "%PROJECT_ROOT%\.claude\hooks\session-start.log"
git -C "%PROJECT_ROOT%" log -5 --oneline >> "%PROJECT_ROOT%\.claude\hooks\session-start.log" 2>&1

exit /b 0
