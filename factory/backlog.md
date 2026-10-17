# factory 작업 목록 (backlog)

상태: `queued → spec → build → verify → review → approved → merged` / `rework` / `needs-human` / `dropped`. 승인은 사용자의 `/factory approve <ID>`로만.

| ID | 제목 | 상태 | 브랜치 | 승인 | 갱신 |
|---|---|---|---|---|---|
| JOB-001 | DXF 내보내기 단위 헤더(`$INSUNITS=4`)·R12·11 레이어 자동 검증 테스트 | **review** (AI 검토 4/4 PASS, 사용자 승인 대기) | `factory/JOB-001` @ 4ed86af | — | 2026-10-17 |

## 후보 (아직 등록하지 않음, 우선순위 순)
1. 단위 자동 환산을 "모호하면 묻기"로 변경 (`studio/src/MeshIO.cpp:1257-1273`) — 축척 신뢰성.
2. 파편(열린 호) 정위치·허위 저경 방지 (`AnnulusClassifier.cpp:43`, `DimensionAnalyzer.cpp:153-193`).
3. SVG 레이어 수·축척 칩·선 굵기 단위를 UI_RULES 하나로 통일하고 테스트로 고정.
4. GLM FetchContent 제거, Google Analytics 전송 코드 제거(GigaMesh 쪽).
5. Illustrator 실제 가져오기 검증 절차와 캡처 기록.
6. 토기 도판 좌/우 배치 — **D5 결정 후**.
7. '문양' 축(문양 전개/탁본 벡터화/능선) 로드맵 명세 — 보고서 표준 조사 결과 후.
