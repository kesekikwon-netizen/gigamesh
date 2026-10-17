# JOB-004 구현 기록 (builder)

작성: builder, 2026-10-17. worktree `D:/_dongarch_build/wt/.worktrees/JOB-004` (브랜치 `factory/JOB-004`, 기준 `e9b655a` = JOB-003 끝, **JOB-003 승인 전제**).
범위: spec.md §0 분할 결정에 따라 **JOB-004a 만**(도판 좌/우 D5 · `R` 좌/우 반전 · `studio_side` 시험). 004b(실시간 단면)·004c(② 페이지·`--cut`)는 손대지 않음.
커밋 1개: `b033aefa7468a5ea67e85b8352f28c12e4b83350`.

## 바꾼 파일
| 파일 | +/− | 내용 | 이유(명세) |
|---|---|---|---|
| `studio/src/ArtifactDoc.h` | +25 −3 | `enum class SectionSide { Right, Left }`, `setSectionSide/sectionSide/toggleSectionSide/sectionSign`(인라인), 멤버 `side_ = Right`, private `sectionNatural/scannedSolidNatural/estimatedInnerNatural`, 주석 24·111-112 를 D5 문구로 | §3.1 |
| `studio/src/ArtifactDoc.cpp` | +45 −14 | `mirrorX` 삭제 → `signedX(v, s)`(곱셈). 기존 `display*` 본문을 `…Natural()` 로 옮기고(자연 좌표에서 `simplifyDP`·`smoothCatmull`), 공개 `displaySection/ScannedSolid/EstimatedInner/Outline` 은 **마지막에** x 에 `sectionSign()` 곱함. `displayHatch` 는 `sectionNatural()` 로 계산 후 끝점 x 에 부호. 슬롯 meta `좌단면·우외곽` → `종단면`. `recompute`·`buildSectionLoops`·두께·치수·정면각·`load*`·`confirmPose` 는 무변경(로드가 좌/우를 되돌리지 않음) | §3.1 |
| `studio/src/Plate.h` · `Plate.cpp` | +2 −1 · +21 −11 | `renderShade` yaw = Right 0° / Left 180°, 화면 음영 자르기 = 외면 쪽 절반(Right `[rect.left, 0]`, Left `[0, rect.right]`), SVG 음영 층 Right 는 영상 왼쪽 절반을 `x = X(rect.left)`, Left 는 현행. 주석(Plate.h 35, Plate.cpp 음영 단계·630·713) 갱신 | §3.2 |
| `studio/src/SheetView.cpp` | +15 −6 | 머리 부제 Right `외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)` / Left `단면 왼쪽 · 외면 오른쪽 (R 반전)`(미스캔 꼬리 문구 유지), 기벽 풍선 조건 `s·x > 0 && s·x < rOut+15`, 점 `(s·rOut, h)`, 상자는 Right 면 점 오른쪽(화면 밖이면 왼쪽), Left 는 현행 | §3.3 |
| `studio/src/View3D.cpp` | +17 −6 | `view()` 확정 뒤에만 Left 면 yaw +180°. 절단선을 선분 중점 월드 x 로 연속 구간으로 나눠 x ≥ 0 = 흰 5 px + `ProfileRed` 2 px(기존 펜 줄 그대로), x < 0 = `ProfileRed` 1 px `DashLine`. 수평 슬롯은 전부 굵게(무변경). 새 색 없음 | §3.4 |
| `studio/src/MainWindow.cpp` | +19 −4 | 익명 `sideText()`; 리본 `combined` 글자 Right `외면 왼쪽·단면 오른쪽` / Left `단면 왼쪽·외면 오른쪽 (반전)` + kbd `R` + 툴팁; `needPose` 에 `combined`; `onAction("combined")` = `toggleSectionSide` → `setText` → `refreshAll(true)` → 상태줄 안내(되돌리기에 쌓지 않음); `Key_R` → `onAction("combined")`(옛 `pushUndo(); setFrontAngle(+180)` 삭제); ctx 단면 묶음에 `R`·`좌/우 반전`; `saveSlotAngle` meta `종단면`. MainWindow.h 는 영향 파일 표에 없어 손대지 않음(새 멤버 없이 onAction 으로 처리) | §3.5 |
| `studio/src/main.cpp` | +7 −1 | `--flip`: `--shot` 블록 안에서 200 ms 뒤 `Qt::Key_R` 키 이벤트, 사용법 주석 | §3.5 |
| `studio/src/ExportDialog.cpp` · `StartPage.cpp` | 각 +1 −1 | `(외면 쪽 300 DPI 이미지)`, `외면 쪽(왼쪽)에 D-Tak 탁본이 결합됩니다` | §3.5 |
| `studio/tests/test_side.cpp` | +424 (신규) | 수용 기준 1-9 (아래 측정값) | §4 A |
| `studio/tests/test_pipeline.cpp` | +17 −6 | 51-57 만: 주석 D5, `secMinX >= -0.5`("section must be right")·`olMaxX <= 0.5`("outline must be left"), `setSectionSide(Left)` 아래 옛 두 검사 숫자 그대로, `setSectionSide(Right)` 로 복귀 | §4 기준 10 |
| `studio/CMakeLists.txt` | +3 | `test_studio_side` 실행 파일·`add_test(NAME studio_side COMMAND test_studio_side ${CMAKE_CURRENT_BINARY_DIR})` | §6 |
| `docs/ui/UI_RULES.md` | +4 −1 | 머리말 예외 1줄, R5 72행 교체(명세 문장 그대로), R6 뒤 1줄, 체크리스트 1줄 | §3.6 |
| `CLAUDE.md` | +1 −1 | 10행 `미결(사용자 결정 대기)…` → `토기 도판 좌/우(D5, 2026-10-17 확정)…` | §3.6 |

`GigaMesh/**`(K-Arch 포함), `MeshIO.*`·`MeshRaster.*`·`Ribbon.*`·`Theme.h`·`MainWindow.h`, `test_pose.cpp`, `factory/**`, `docs/ui/mockup/**` 무변경.

## 실행한 명령과 종료 코드
| # | 명령 | 종료 코드 | 결과 |
|---|---|---|---|
| 1 | `powershell -NoProfile -ExecutionPolicy Bypass -File D:/_dongarch_build/wt/.worktrees/JOB-004/factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-004 -BuildRoot D:\_dongarch_build\verify_job004 -NoShots` (변경 전 기준선) | 0 | configure 0 · build 0(18 s) · ctest 0 → `100% tests passed out of 13`, 경고 0 |
| 2 | 같은 명령 `-NoShots` (구현 직후) | 0 | `100% tests passed out of 14`, studio_side PASS, 경고 0 |
| 3 | 같은 명령(캡처 포함) | 0 | PASS, 캡처 3장 |
| 4 | 같은 명령 `-Clean`(빌드 폴더 지우고 전체 60단계 재빌드, **최종**) | 0 | configure 0(4 s) · build 0(17 s) · ctest 0(30 s) · shots 0(9 s), `verify-summary.md` 마지막 줄 `결과: PASS` |
| 5 | `grep -ci warning D:/_dongarch_build/verify_job004/studio-logs/02_build.log` (4 의 전체 빌드) | — | `0` |
| 6 | 4 직후 `LastTest.log` grep 3종(기준 11) | — | 각 `1`: `H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180`, `STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2`, `ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95` — `pts=85/1318` 불변(간략화·곡선이 x 부호에 대칭이라는 명세 가정 확인) |
| 7 | `powershell -NoProfile -Command '…QT_QPA_PLATFORM=offscreen; Start-Process -Wait … DongArch3DStudio.exe --sample --flip --shot …\shots\shot_flip.png; (Get-Item …).Length'` (명세 검증 명령 3) | 0 | `336525` 바이트 |
| 8 | `ls -l D:/_dongarch_build/verify_job004/studio/shots/` | 0 | `shot_home.png` 13,246 · `shot_main.png` 333,540 · `shot_export.png` 70,269 · `shot_flip.png` 336,525 (모두 > 5,120) |
| 9 | 수용 기준 14-19 grep(Git Bash, `cd` worktree) | 0 | 14: `mirrorX` 0 · `frontAngle() + 180` 0 / 15: 출력 없음 / 16: MISSING 없음 / 17: `왼쪽 = 기벽 종단면` 0 · `오른쪽 = 기벽 종단면` 1 · `D5, 2026-10-17 확정` 1 · `미결(사용자 결정 대기)` 0 / 18: 색 하드코딩 grep 출력 없음(View3D 의 `QColor(Qt::white)` 펜 줄은 그대로 두어 `+` 줄 아님) · `ToolState::Primary` 0 / 19: 변경 15파일 모두 §6 표 안, `GigaMesh`·`test_pose.cpp` 변경 0 |
| 10 | `git diff --check` | 0 | 공백 문제 없음 |
| 11 | `git -c user.name=kesekikwon-netizen -c user.email=kesekikwon@gmail.com commit -F <메시지>` | 0 | `b033aef` |

캡처(육안, builder 확인): `shot_main.png` 빨간 단면이 축 오른쪽·음영·외곽선이 왼쪽, 3D 는 굵은 절단선이 유물 오른쪽 반·왼쪽 반은 가는 파선. `shot_flip.png` 는 그 거울상(단면 왼쪽), 3D 는 뒤에서 보아 굵은 선이 화면 왼쪽. `shot_export.png` 미리보기도 단면 오른쪽. 오프스크린 한글은 □(알려진 위험 7). 산출물 복사(`artifacts/`)는 test-runner 몫이라 하지 않음(verify.ps1 에 `-Job` 을 주지 않음).

## studio_side 측정값 (최종 ctest, LastTest.log)
| 기준 | 예제 1(합성 토기) | 예제 2(미스캔 단경호) |
|---|---|---|
| 2 곡선 끔 Right | 단면 x[64.1584, 82.6495]·외곽 x[−82.6446, −71.6971] | 단면 x[54.9657, 84.0003]·실선 x[54.9657, 84.0003]·추정 x[55.4866, 77.0906]·외곽 x[−83.8973, −64.9351] |
| 3 곡선 켬 Right | 단면 x[64.1584, 82.6747]·빗금 x[64.2591, 82.6711]·외곽 최대 −71.6971 | 단면 최소 54.9657·빗금 x[55.2775, 84.0289]·외곽 최대 −64.3832 |
| 4 거울상 오차(끔/켬) | 0 / 0 (단면·실선·추정·외곽·빗금 모두), 치수·정면각·점 수·두께 18높이 `==`, 다시 누르면 처음과 동일 | 같음 |
| 5 로드 유지 | Left 유지 | Left 유지 |
| 6 SVG (X0 = 105.000) | Right 단면 x[169.158, 187.675]·외곽 x[22.355, 33.303]·영상 x+w = 105.000 / Left 단면 x[22.325, 40.842]·외곽 x[176.697, 187.645]·영상 x = 105.000 / 거울상 최대 오차 0.0000 mm | Right 단면 x[159.966, 189.036]·영상 x+w = 105.000 / Left 영상 x = 105.000 / 오차 0.0000 mm |
| 7 DXF | 04 Right x[64.158356, 82.674713]·02 Right x[−82.644562, −71.697099], Left 부호 반대, 층별 개수 같음, `|xL+xR|` 최대 0 | 04·06·02 모두 같은 조건, 06 n=34, 최대 0 |
| 8 래스터(예제 1, 4 px/mm, m = 5 px) | Right: 단면만 왼 0 / 오 23,105 · 음영만 왼 227,754 / 오 0 — Left: 단면만 왼 23,106 / 오 0 · 음영만 왼 0 / 오 227,754 | — |
| 9 | 마지막 줄 `PASS`, 종료 0 (11.7 s) | |

## 미리보기 측정값(시간·LOD 대비 차이)
- 해당 없음: 실시간(LOD) 단면은 004b 범위(명세 §2 범위 밖). 이번 조각은 계산을 바꾸지 않는다(부호만).

## 남은 경고
- 빌드 경고 0 (/W4, 전체 재빌드 기준).
- CTest 요약 문구: 이 PC 의 CMake 4.4.1 ctest 는 `100% tests passed out of 14` 로 출력하고 `, 0 tests failed` 를 붙이지 않는다. 명세 기준 12 의 문자열 `100% tests passed, 0 tests failed out of 14` 는 그대로는 0건이다(JOB-001~003 검증과 같은 처리 — 실질 조건 14/14·실패 0·종료 0 충족).

## 제안 (범위 밖, 하지 않음)
1. 상태줄 커서 `R`: 명세 §7 은 "단면 쪽이 +" 라 적었다. 기본(Right)에서는 성립하지만 Left 에서는 도면 x 그대로라 단면 쪽이 −로 나온다. 반전 때도 +로 보이게 하려면 `MainWindow.cpp` cursorMm 람다에서 `r × sectionSign()` — 사람 검토 1 과 함께 결정 필요.
2. 머리 치수 `기고 H` 파선·라벨은 도판 왼쪽 끝 → 축으로 그려져, D5 기본에서는 외면(음영) 위에 겹친다(예전엔 단면 쪽). 오른쪽(단면 쪽)으로 옮길지 디자인 v2 반입(백로그 8) 때 결정.
3. Left 의 리본 글자(16자)는 Right(11자)보다 길어 오프스크린 캡처에서 창 폭이 26 px 늘었다(□ 글리프 폭 탓, 옛 글자 17자와 비슷한 수준). 실제 한글 폰트 화면에서 1560 px 넘침 여부는 ui-reviewer 육안 확인 권장.
4. 시험 줄 수: `test_side.cpp` 424줄(명세 추정 ≈210). SVG·DXF 를 두 예제 모두에서 검사하고 측정값을 인쇄하느라 늘었다 — 줄이려면 예제 2 의 SVG/DXF 검사를 빼면 되나 미스캔 실선·추정선 층(06) 검증이 빠진다.
5. 3D 절단선 흰 테는 기존처럼 `QColor(Qt::white)` 그대로다(R2 토큰 `Surface` = #FFFFFF 로 바꿀 수 있으나 기존 줄이라 손대지 않음).

## 커밋
- `b033aefa7468a5ea67e85b8352f28c12e4b83350` — `feat(studio): ② 막으로 자르기 — 도판 좌/우 D5(외면 왼쪽·단면 오른쪽)·R 좌/우 반전 (JOB-004)` (worktree `D:/_dongarch_build/wt/.worktrees/JOB-004`, 브랜치 `factory/JOB-004`, 부모 `e9b655a`). push 하지 않음.

## 차단 사유 (있으면)
- 없음.
