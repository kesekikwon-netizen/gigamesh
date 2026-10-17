# factory 작업 목록 (backlog)

상태: `queued → spec → build → verify → review → approved → merged` / `rework` / `needs-human` / `dropped`. 승인은 사용자의 `/factory approve <ID>`로만.

| ID | 제목 | 상태 | 브랜치 | 승인 | 갱신 |
|---|---|---|---|---|---|
| JOB-001 | DXF 내보내기 단위 헤더(`$INSUNITS=4`)·R12·11 레이어 자동 검증 테스트 | **review** (AI 검토 4/4 PASS, 사용자 승인 대기) | `factory/JOB-001` @ 4ed86af | — | 2026-10-17 |
| JOB-002 | K-Arch 테스트 3종의 `D:\gigamesh` 절대 경로 제거 (CI run 39010516345 실패 수정) | **review** (AI 검토 4/4 PASS, 사용자 승인 대기) | `factory/JOB-002` @ 32d5d85 | — | 2026-10-17 |

## 후보 (아직 등록하지 않음, 우선순위 순)
1. 단위 자동 환산을 "모호하면 묻기"로 변경 (`studio/src/MeshIO.cpp:1257-1273`) — 축척 신뢰성.
2. 파편(열린 호) 정위치·허위 저경 방지 (`AnnulusClassifier.cpp:43`, `DimensionAnalyzer.cpp:153-193`).
3. SVG 레이어 수·축척 칩·선 굵기 단위를 UI_RULES 하나로 통일하고 테스트로 고정.
4. GLM FetchContent 제거, Google Analytics 전송 코드 제거(GigaMesh 쪽).
5. Illustrator 실제 가져오기 검증 절차와 캡처 기록.
6. 토기 도판 좌/우 배치 — **D5 결정 후**.
7. '문양' 축(문양 전개/탁본 벡터화/능선) 로드맵 명세 — 보고서 표준 조사 결과 후.
8. 조사 문서 4종(`/mnt/project-files/gigamesh-docs/plan-설계초안.md`, `research-실측표준.md`, `research-자동화범위.md`, `research-도구비교.md`)을 저장소 `plan/`·`research/`로 반입 — 코드 무변경, JOB-002 뒤 별도 작업.
9. 병합 기준 커밋 bd53453 고정 후 `feat/dongarch-studio`의 이후 커밋을 분류해 수용(체리픽 정책).
10. 1차 MVP 범위를 완형 토기로 한정하고 완료 게이트에 "사용자 Artec 완형 스캔 1점 통과"를 둔다(파편은 2차).
11. 단위 정책을 "추정 후 사용자 확정, 확정 전 잠금"으로 명세(후보 1과 합침 가능).
12. Illustrator 1:1 실측: 100 mm 보정 사각형이 든 SVG/PDF를 사용자 PC Illustrator에서 열어 치수 확인·캡처 기록(후보 5 구체화).
13. D5 설계 권고 반영(결정 후): 프리셋 `sectionSide` 기본값 비움·첫 실행 선택·R 키 반전·일본 프리셋 우 고정·"(토기 표준)" 라벨을 "기관 프리셋 ○○"로.
