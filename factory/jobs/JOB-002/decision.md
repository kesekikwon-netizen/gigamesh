# JOB-002 결정 기록

| 시각 | 누가 | 결정 | 사유/원문 |
|---|---|---|---|
| 2026-10-17 15:3x | factory | 등록 | GitHub Actions run 39010516345 에서 기존 테스트 2개가 `D:\gigamesh` 절대 경로 때문에 실패. 사용자 지시 "다되면 깃커밋 깃액션까지실행"(Actions 녹색이 목표) |
| 2026-10-17 16:2x | factory | AI 검토 4/4 PASS → **사용자 승인 대기** | 비차단 제안: outPath 헬퍼 공용 헤더화(JOB-001 병합 후 별도 JOB), 실패 메시지에 경로 포함, `/argtest/` gitignore 또는 실행 위치 문서화, test_*.cpp GPLv3+ 표기 통일, karch 테스트 SVG 작성기의 R2/R6 불일치는 테스트 전용 여부 확인 후 별도 JOB. 승인: `/factory approve JOB-002`, 재작업: `/factory rework JOB-002` |
