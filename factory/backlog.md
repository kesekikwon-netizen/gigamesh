# factory 작업 목록 (backlog)

상태: `queued → spec → build → verify → review → approved → merged` / `rework` / `needs-human` / `dropped`. 승인은 사용자의 `/factory approve <ID>`로만.

| ID | 제목 | 상태 | 브랜치 | 승인 | 갱신 |
|---|---|---|---|---|---|
| JOB-001 | DXF 내보내기 단위 헤더(`$INSUNITS=4`)·R12·11 레이어 자동 검증 테스트 | **merged** (a244ce4) | `factory/JOB-001` @ 4ed86af | 권 2026-10-17 09:38 UTC | 2026-10-17 |
| JOB-002 | K-Arch 테스트 3종의 `D:\gigamesh` 절대 경로 제거 (CI run 39010516345 실패 수정) | **merged** (d651215) | `factory/JOB-002` @ 32d5d85 | 권 2026-10-17 09:38 UTC | 2026-10-17 |
| JOB-003 | ① 바닥 정하기 — 들어온 자세로 열기·X/Y/Z 90°·뒤집기·처음 자세로·자동 세우기·확정 | **review** (AI 검토 4/4 PASS, 사용자 승인 대기) | `factory/JOB-003` @ e9b655a | — | 2026-10-17 |
| JOB-004 | ② 막으로 자르기 004a — 도판 좌/우 D5·R 좌/우 반전 | **review** (AI 검토 4/4 PASS, 사용자 승인 대기, JOB-003 승인 전제) | `factory/JOB-004` @ b033aef | — | 2026-10-17 |
| JOB-005 | ② 막으로 자르기 004b — 실시간 단면(끄는 동안 LOD 단면 1장, 놓으면 원본 전체 계산) | spec (JOB-004 b033aef 위에 쌓음) | — | — | 2026-10-17 |
| JOB-006 | ② 막으로 자르기 004c — ② 페이지(막 방향 Y/X, 각도 ±, ← → 1°·Shift 15°, 원문 라벨, --cut) | queued | — | — | 2026-10-17 |
| JOB-007 | ③ 도면 편집 넘기기 — 지금 각도 고정 → 도면 편집, 기준선 레이어(중심선·구연선·바닥선) | queued | — | — | 2026-10-17 |

## 후보 (아직 등록하지 않음, 우선순위 순)
1. 단위 자동 환산을 "모호하면 묻기"로 변경 (`studio/src/MeshIO.cpp:1257-1273`) — 축척 신뢰성.
2. 파편(열린 호) 정위치·허위 저경 방지 (`AnnulusClassifier.cpp:43`, `DimensionAnalyzer.cpp:153-193`).
3. SVG 레이어 수·축척 칩·선 굵기 단위를 UI_RULES 하나로 통일하고 테스트로 고정.
4. GLM FetchContent 제거, Google Analytics 전송 코드 제거(GigaMesh 쪽).
5. Illustrator 실제 가져오기 검증 절차와 캡처 기록.
6. **[결함] Studio 도판의 좌/우 배치가 국내 기준과 반대** — D5 확정값은 "외면 왼쪽·단면 오른쪽"인데 현재 Studio 는 단면을 왼쪽에 그린다(`studio/src/ArtifactDoc.cpp:69` `mirrorX`, :444 "좌단면·우외곽", :473·485·498·508 호출; `docs/ui/UI_RULES.md:72` 도 "왼쪽 = 기벽 종단면"). 고칠 때 R 키 반전·기관 프리셋을 함께 넣고 UI_RULES R5 를 같은 커밋에서 갱신. **사용자 요청 전까지 구현하지 않음.**
7. '문양' 축(문양 전개/탁본 벡터화/능선) 로드맵 명세 — 보고서 표준 조사 결과 후.
8. 조사 문서 4종(`/mnt/project-files/gigamesh-docs/plan-설계초안.md`, `research-실측표준.md`, `research-자동화범위.md`, `research-도구비교.md`)을 저장소 `plan/`·`research/`로, **화면 디자인 완성본** `/mnt/project-files/gigamesh-docs/design/dongarch3d-design.html`(열람: https://claude.ai/artifact/C4b5eZFrQmF2NPaJZCmJQ8 — 화면 4개: 주 화면·4면 전개·내보내기 창·시작 화면, 디자인 가이드: 색 토큰 15개·글꼴·4 px 간격·아이콘 20종·도면 선 규약·용어표·단계 잠금 규칙)을 `docs/design/`으로 반입 — 코드 무변경, JOB-002 뒤 별도 작업. 반입 시 `docs/ui/UI_RULES.md`(R1~R8)와의 차이를 표로 남기고 정본은 하나로 정한다. 2026-10-17 09:10 **디자인 v2** 로 갱신됨(같은 링크): 주 화면이 바닥 정하기 → 막으로 자르기 → 도면 편집 순서로 재구성. 함께 반입할 파일: `gigamesh-docs/design/dongarch3d-design.html`(v2), `gigamesh-docs/design/스캔-단면-자르기-설명.html`(기존 제품 비교 포함, 열람 https://claude.ai/artifact/J7TtJ1fMbgSAYUPPVdT2yf).
9. 병합 기준 커밋 bd53453 고정 후 `feat/dongarch-studio`의 이후 커밋을 분류해 수용(체리픽 정책).
10. 1차 MVP 범위를 완형 토기로 한정하고 완료 게이트에 "사용자 Artec 완형 스캔 1점 통과"를 둔다(파편은 2차).
11. 단위 정책을 "추정 후 사용자 확정, 확정 전 잠금"으로 명세(후보 1과 합침 가능).
12. Illustrator 1:1 실측: 100 mm 보정 사각형이 든 SVG/PDF를 사용자 PC Illustrator에서 열어 치수 확인·캡처 기록(후보 5 구체화).
13. D5 반영(확정: 외면 왼쪽·단면 오른쪽): 기본값을 오른쪽 단면으로, R 키 반전 유지, 기관 프리셋(국내 기본·일본 우 단면 등), "(토기 표준)" 라벨을 "기관 프리셋 ○○"로 — 후보 6 과 한 작업으로.
14. **기준선 레이어**(사용자 요구): 중심선·구연부 수평선·바닥선·다면도 눈금·복원 파선·파단 표시를 독립 레이어로, 단면은 윤곽선 두 줄(외면·내면) 기본. SVG/DXF 레이어 규약(후보 3)과 함께 명세.
15. **핵심 작업 흐름**(사용자 2026-10-17 08:03): ① 바닥 정하기 — X·Y·Z 이동·회전으로 유물을 바닥판에 올려 확정 → ② 얇은 막(절단 평면)을 3D 뷰에서 보며 움직이면 오른쪽 도판에 단면이 실시간으로 다시 그려짐 → ③ 도면 편집으로 넘기기. 디자인 v2 주 화면이 이 순서. 1차 MVP 의 중심 기능으로 명세(후보 10·11 과 묶음).
