# Windows Claude Code: 자동 빌드 워크플로우

이 커맨드는 Windows에서 실행됩니다.

## 작업:
1. `.claude/build-status.json` 확인
2. `linuxStatus`가 "ready_to_build"이면 빌드 실행:
   ```powershell
   cd A:\1105\GigaMesh\build_korean
   cmake --build . --config Release --target DongArch3D
   ```
3. 빌드 결과를 `build-status.json`에 업데이트:
   - 성공 시: `windowsStatus: "build_success"`
   - 실패 시: `windowsStatus: "build_failed"`, 에러 메시지 기록
4. Git commit & push

## 자동 실행 조건:
- Git pull 후 `.claude/build-status.json` 변경 감지
- `linuxStatus == "ready_to_build"`
- `windowsStatus == "pending"`
