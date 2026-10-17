# JOB-001 검토: ui (ui-reviewer, 2026-10-17)

결론: **해당 없음(PASS)**

## 범위 확인 (diff 로 직접 확인)
`git diff --name-only e7e641b..HEAD` = `GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp`(신규 589줄), `studio/CMakeLists.txt`(+2/−1). `studio/src/*`, `docs/ui/*`, `Theme.h`, `DxfWriter.*`, `Plate.cpp`, `ExportDialog.cpp` 변경 없음 → 사용자에게 보이는 문자열·색상·레이아웃·SVG/DXF 산출물 중 바뀐 것 없음. CTest 전용 실행파일 1개 추가이며 앱 본체에 링크되지 않음(`studio/CMakeLists.txt:104-108`).

## 규칙별 판정
| 규칙 | 판정 | 근거 |
|---|---|---|
| R1 화면 구조 | 해당 없음 | UI 코드 변경 0. 참고 캡처 `shot_main.png`: 탭 줄 → 리본 → 슬롯 목록·3D 보조창·도판 → 상태줄 순서 유지, 새 Dock 없음 |
| R2 색상 토큰 | 해당 없음 | diff 에 hex/QColor/스타일시트 없음 |
| R3 도구 3단계 | 해당 없음 | 리본·타일 코드 변경 없음 |
| R4 글꼴 | 해당 없음 | 변경 없음 |
| R5 도판·좌/우 배치(D5) | pass | 배치 코드 변경 없음. 테스트는 부호를 잠그지 않음(AC-5 블록 "sign-neutral: absolute values, distances, radii only"; 외곽선 `fabs(vx)` 최댓값, 치수선 `fabs` 차, 원 중심 `fabs(rim.x) ≤ 0.1`). "표준/정본/좌/우" 단정 문구 없음(`sil.leftOutline`은 기존 엔진 API 식별자 1건뿐) |
| R6 내보내기 | pass(테스트 기대값) / 해당 없음(코드) | 11 레이어 이름 전수 대조 일치(아래). ACI/선종류는 명세 §2 허용대로 karch `DxfWriter.cpp:269-279` 를 회귀 잠금; 어느 쪽을 표준이라 쓰지 않음 |
| R7 홈 화면 | 해당 없음 | 변경 없음 |
| R8 문구 | 해당 없음 | 새 UI 문자열 0 |
| 언어(CLAUDE.md §4-2) | pass | 테스트 stderr 는 개발자용 CTest 출력이라 UI 문자열이 아님. CLAUDE.md 머리말이 코드 식별자 영문 허용, 명세 §3 이 `FAIL <항목> <이유>` 형식을 규정, 기존 karch 테스트도 같은 관례(`test_dxf.cpp:62,74,84,88,92`). 비ASCII 0건. 커밋 제목·본문은 한국어 |
| 오프스크린 캡처 | 기록 | 3장 모두 한글이 □(폰트 미탐색). 레이아웃만 판정 |

## 11 레이어 이름 대조
테스트 `kLayers[1..11]` = `01_AXIS_CENTER, 02_EXTERIOR_OUTLINE, 03_EXTERIOR_FEATURES, 04_SECTION_WALL, 05_SECTION_HATCH, 06_INTERIOR_OUTLINE, 07_PROJECTION_RIM, 08_PROJECTION_BASE, 09_DIMENSION_MEASURE, 10_ANNOTATION_TEXT, 11_SCALE_BAR` — karch `DxfWriter.cpp:269-279`, Studio `Plate.cpp:723-733`, 기존 `test_dxf.cpp:67-72`, 사용자 문자열 `ExportDialog.cpp:257` 양 끝과 모두 일치. 불일치 0.

## 고치지 않아도 되는 제안
1. `kLayers` 위 주석에 "ACI/선종류는 karch 작성기 현재값의 회귀 잠금이며 Studio `Plate.cpp:723-733` 과 다름 — 통일은 별도 작업(spec §2)" 한 줄 보강 권장.
2. verify.ps1 오프스크린 캡처에 `QT_QPA_FONTDIR` 또는 Noto Sans KR 경로를 주입하면 이후 UI 작업의 R8 문구 검토가 가능(인프라 백로그 후보).
3. UI 작업 때 확인 권장(이번 diff 와 무관): `shot_main.png` 의 3D 보조창 폭이 약 15 %로 R1 "≈27 %"보다 좁음; 리본 아래 36 px 줄이 도구를 들지 않은 상태에서도 보이는지(□ 렌더링 탓에 단정 못 함).
4. `.github/workflows/studio-ci.yml:31,60,63` 의 "11 Suites/10 K-Arch" 문구가 12/11 로 낡음(명세 §2 가 이번엔 제외).

파일 생성·수정·빌드·테스트·git 쓰기 없음(읽기 명령만 사용).
