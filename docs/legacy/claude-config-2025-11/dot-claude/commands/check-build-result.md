# Linux Claude Code: 빌드 결과 확인

이 커맨드는 Linux에서 실행됩니다.

## 작업:
1. Git pull (Windows의 최신 변경사항 받기)
2. `.claude/build-status.json` 읽기
3. `windowsStatus` 확인:
   - "build_success" → 다음 Phase 작업 진행
   - "build_failed" → 에러 메시지 분석 후 수정
   - "pending" → 대기 중 (Windows 빌드 진행 중)

## 자동 실행 조건:
- 코드 push 후 1분 대기
- Windows 빌드 완료 확인
