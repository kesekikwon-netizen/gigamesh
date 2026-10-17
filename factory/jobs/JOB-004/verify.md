# JOB-004 검증 기록 (test-runner)

결론: **PASS** — 수용 기준 1-20 모두 통과(기준 12 는 CTest 4.4.1 요약 문구 형식 차이만 있고 실질 조건 충족, JOB-001~003 과 같은 처리). 21-23 은 ui-reviewer 육안 판정 몫이며 test-runner 관찰로는 명세 배치와 일치(아래 표).

요약: 빌드 폴더(`studio`·`studio-logs`)를 지우고 처음부터 구성·빌드(`[1/60]`…`[60/60]`, /W4 경고 0) → CTest 14/14 통과(`studio_side` PASS 포함) → 캡처 4장(> 5,120 B) 정상. `studio_side` 가 만든 SVG·DXF 8개를 별도 파이썬 파서로 다시 읽어 기준 6·7 을 독립 판정한 결과도 28/28 OK. 기준 11 의 기존 값 3줄(`pts=85/1318` 포함) 그대로. 코드·문구·정본 grep(14-19) 전부 기대값. builder 주장(build-log.md)과 측정값이 모두 일치한다.

## 환경
- 컴파일러: MSVC 19.44.35228.0 (BuildTools 14.44.35207, Hostx64/x64), Ninja, CMake/CTest 4.4.1, Release, 경고 수준 `/W4 /permissive-`(build.ninja 41회)
- Qt: `D:/OSGeo4W/apps/Qt6` (Qt6Core 6.11.1), 실행 시 `QT_QPA_PLATFORM=offscreen`
- 검증 대상: `D:\_dongarch_build\wt\.worktrees\JOB-004` 브랜치 `factory/JOB-004` HEAD `b033aefa7468a5ea67e85b8352f28c12e4b83350` (기준 `e9b655a` = JOB-003 끝, **JOB-003 승인 전제**), 커밋 1개. `git status --porcelain` 검증 전·후 0줄.
- 빌드 폴더: `D:\_dongarch_build\verify_job004\studio` (= B), 로그 `D:\_dongarch_build\verify_job004\studio-logs` (= L). 실행 전 두 폴더 삭제.
- 실행 시각: 2026-10-17 20:46–20:52 (KST). verify.ps1 벽시계 61 초(configure 4 · build 17 · ctest 30 · shots 9).
- 참고: vcvars64 가 콘솔에 `'vswhere.exe' … 아닙니다` 를 두 번 찍지만 configure/build 종료 코드 0 — 이전 JOB 과 같은 환경 잡음.

## 실행한 명령
| # | 명령 | 종료 코드 |
|---|---|---|
| V1 | `rm -rf D:/_dongarch_build/verify_job004/studio D:/_dongarch_build/verify_job004/studio-logs` | 0 |
| V2 | `powershell -NoProfile -ExecutionPolicy Bypass -File D:/_dongarch_build/wt/.worktrees/JOB-004/factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-004 -BuildRoot D:\_dongarch_build\verify_job004 -Job JOB-004 -RecordRoot D:\_dongarch_build\wt` (캡처 포함) | **0** |
| V3 | V2 직후 `B\Testing\Temporary\LastTest.log` 에 기준 11 grep 3종 → `artifacts/LastTest_verify.log` 복사 | — (각 1) |
| V4 | (PowerShell, `PATH=Qt6\bin;…`, `QT_PLUGIN_PATH`, `QT_QPA_PLATFORM=offscreen`, `Set-Location D:\_dongarch_build\verify_job004\studio`) `ctest -N` | 0 (`Total Tests: 14`) |
| V5 | 같은 위치 `ctest --output-on-failure` | 0 (`100% tests passed out of 14`, 29.80 s) |
| V6 | `ctest -R studio_side -V` | 0 (`PASS`, 11.71 s) |
| V7 | `ctest -R studio_pipeline -V` | 0 (`PASS`) |
| V8 | `ctest -R studio_side --output-on-failure` × 3 (시간 측정) | 0 / 0 / 0 |
| V9 | `ctest --output-on-failure` (최종, LastTest.log 를 전체 결과로 복원) | 0 (14/14, 29.52 s) |
| V10 | 독립 검사 `python.exe -I -X utf8 indep_check.py D:/_dongarch_build/verify_job004/studio` (studio_side 의 SVG 4·DXF 4 를 별도 파서로 판정, 스크립트는 세션 scratchpad) | 0 (`INDEPENDENT PASS`, OK 28 · FAIL 0) |
| V11 | 명세 검증 명령 3: `Start-Process -Wait -WorkingDirectory B -FilePath B\DongArch3DStudio.exe -ArgumentList '--sample','--flip','--shot','B\shots\shot_flip.png'; (Get-Item …).Length` | 0 (`336525`, 4.1 s) |
| V12 | (Git Bash, `cd D:/_dongarch_build/wt/.worktrees/JOB-004`) 수용 기준 14-19 grep·`git diff` | 아래 표 |

## 수용 기준별 결과
| 기준 | 명령 | 종료 코드 | 결과 | 근거(로그 줄·산출물 경로) |
|---|---|---|---|---|
| 1 기본값 Right | V6 | 0 | 통과 | `test_side.cpp:212-216` 새 문서·로드 뒤 `sectionSide()==Right`, `sectionSign()==1.0` 검사, FAIL 줄 0 → `3: PASS` (`artifacts/ctest_studio_side_V.log`) |
| 2 D5 부호·곡선 끔(0 여유) | V6 | 0 | 통과 | 예제1 `[2] smooth=off Right: section x[64.1584, 82.6495] n=20 … outline x[-82.6446, -71.6971] n=7`. 예제2 section `x[54.9657, 84.0003] n=30`, solid `x[54.9657, 84.0003] n=18`, est `x[55.4866, 77.0906] n=12`, outline `x[-83.8973, -64.9351] n=11`(hasEstimatedInner 검사 포함) |
| 3 D5 부호·곡선 켬(0.5 mm) | V6 | 0 | 통과 | 예제1 section `x[64.1584, 82.6747]`, hatch `x[64.2591, 82.6711] n=182`, outline `x[-82.6446, -71.6971]`. 예제2 section `x[54.9657, 84.0357]`, est `x[55.4866, 77.1499]`, hatch `x[55.2775, 84.0289]`, outline `x[-83.9967, -64.3832]` — 단면 최소 > 0, 외곽 최대 < 0(여유도 쓰지 않음) |
| 4 R = 거울상, 계산값 불변 | V6 | 0 | 통과 | `[4] smooth=0 mirror err: section 0 solid 0 est 0 outline 0 hatch 0`, `smooth=1 … 0` (두 예제). 코드 확인: dims 7필드(ok·height·rim·belly·bellyZ·base·meanWall — 명세 6개 이상)·frontAngle·rawPointCount·pointCount·wallThicknessAt(10…180, 18점) `==`, toggle 뒤 첫 Right 와 `identical`, 빗금 방향 Right "/"·Left "\\" (`test_side.cpp:253-298`) — FAIL 0 |
| 5 세션 유지 | V6 | 0 | 통과 | `test_side.cpp:301-303` `setSectionSide(Left)` → 같은 예제 재로드 → `Left` 검사, FAIL 0 |
| 6 SVG 같은 쪽 | V6, V10 | 0 / 0 | 통과 | 예제1 Right `X0=105.000`, section `x[169.158, 187.675]`, outline `x[22.355, 33.303]`, image `x+w=105.000` / Left section `x[22.325, 40.842]`, outline `x[176.697, 187.645]`, image `x=105.000`, 거울상 최대 0.0000 mm. 예제2 Right section `x[159.966, 189.036]`, `x+w=105.000` / Left image `x=105.000`, 0.0000 mm. 독립 파서: SVG 의 중심축 `<line x1="105.000">` 으로 X0 를 따로 얻어 같은 판정 → OK (`artifacts/indep_svg_dxf_check.log`, `studio_side_*_{Right,Left}.svg`) |
| 7 DXF 같은 쪽 | V6, V10 | 0 / 0 | 통과 | 예제1 `04_SECTION_WALL` Right `x[64.158356, 82.674713] n=60`, `02_EXTERIOR_OUTLINE` Right `x[-82.644562, -71.697099] n=25`, Left 부호 반대, `\|xL+xR\| max 0`. 예제2 `04 n=52 · 06 x[55.486582, 77.149908] n=34 · 02 n=41`, 모두 `max 0`. 독립 파서도 개수 같음·오차 0 (`artifacts/studio_side_*.dxf`) |
| 8 래스터(paintSheet) | V6 | 0 | 통과 | Right: section-only `left=0 right=23105`, shade-only `left=227754 right=0`. Left: section-only `left=23106 right=0`, shade-only `left=0 right=227754` |
| 9 `studio_side` PASS·종료 0 | V5, V6 | 0 | 통과 | 마지막 줄 `3: PASS`, `1/1 Test #3: studio_side … Passed 11.71 sec` |
| 10 test_pipeline 회귀 | V7 + `git diff e9b655a..HEAD -- studio/tests/test_pipeline.cpp` | 0 | 통과 | diff 는 hunk 1개(`@@ -48,13 +48,24 @@`)뿐: 주석 D5, `secMinX >= -0.5`("section must be right")·`olMaxX <= 0.5`("outline must be left"), `setSectionSide(Left)` 아래 옛 두 검사 숫자 그대로, `setSectionSide(Right)` 복귀. 실행 `1: PASS` (`artifacts/ctest_studio_pipeline_V.log`) |
| 11 기존 값 유지 | V3 | — | 통과 | V2 직후 LastTest.log: `H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180` 1 · `STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2` 1 · `ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95` 1 (기준 `verify_merged` LastTest.log 도 각 1). `artifacts/LastTest_verify.log`, 최종 `LastTest_final.log` 도 1 |
| 12 CTest 14종 전부 통과 | V4, V5, V9 | 0 / 0 / 0 | 통과(문구 형식 주석) | `ctest -N` → `Total Tests: 14`(studio_pipeline·studio_pose·studio_side + karch 11). 출력 `100% tests passed out of 14`, LastTest_final `Test Passed` 14 · `Test Failed` 0. 명세 문자열 `100% tests passed, 0 tests failed out of 14` 는 CTest 4.4.1 이 출력하지 않는 형식이라 그대로는 0건(JOB-001~003 과 같은 처리) — 실질 조건(14/14·실패 0·종료 0) 충족 |
| 13 verify.ps1 0·PASS·경고 0 | V2 + `tail -1 B/verify-summary.md` + `grep -ci "warning" L/02_build.log` | 0 | 통과 | 종료 0, 요약 마지막 줄 `결과: PASS`, 경고 grep `0`(`: warning`·`warning C####` 도 0). `artifacts/verify-summary-studio.md`, `artifacts/02_build.log` |
| 14 결함 기호 제거 | `grep -c "mirrorX" studio/src/ArtifactDoc.cpp`; `grep -c "frontAngle() + 180" studio/src/MainWindow.cpp` | 1 / 1 (일치 0건) | 통과 | `0`, `0` |
| 15 옛 문구 없음 | 명세의 `grep -rnE "좌단면·우외곽\|토기 표준\|좌측 종단면\|우측 외곽\|우측 300 DPI\|우측에 D-Tak" studio/src` | 1 | 통과 | 출력 없음 |
| 16 새 문구 있음 | 명세의 `for s in …; do grep -rqF "$s" studio/src \|\| echo "MISSING $s"; done` | 0 | 통과 | `MISSING` 출력 없음(6개 모두 있음) |
| 17 정본 | `grep -c` 4종(UI_RULES.md·CLAUDE.md) | — | 통과 | `왼쪽 = 기벽 종단면` 0 · `오른쪽 = 기벽 종단면` 1 · `D5, 2026-10-17 확정` 1 · `미결(사용자 결정 대기)` 0 |
| 18 R2·R3 | 명세의 `git diff e9b655a -- studio/src \| grep -E '^\+.*(QColor…\|qRgba?…\|#[0-9A-Fa-f]{6}\|Qt::(red…lightGray)\b)'`; `… \| grep -c '^\+.*ToolState::Primary'` | 1 / — | 통과 | 색 하드코딩 추가 줄 출력 없음, `ToolState::Primary` 추가 `0` |
| 19 변경 범위 | `git diff --name-only e9b655a...HEAD`; `… -- GigaMesh studio/tests/test_pose.cpp` | 0 / 0 | 통과 | 15파일(CLAUDE.md, docs/ui/UI_RULES.md, studio/CMakeLists.txt, studio/src/{ArtifactDoc.cpp, ArtifactDoc.h, ExportDialog.cpp, MainWindow.cpp, Plate.cpp, Plate.h, SheetView.cpp, StartPage.cpp, View3D.cpp, main.cpp}, studio/tests/{test_pipeline.cpp, test_side.cpp}) 모두 §6 표 안. GigaMesh·test_pose.cpp 출력 없음. `git diff --name-only e9b655a..HEAD` 도 같은 15파일, `15 files changed, 602 insertions(+), 56 deletions(-)` |
| 20 캡처 크기 | V2, V11 | 0 / 0 | 통과 | `shot_flip.png` 336,525 · `shot_home.png` 13,246 · `shot_main.png` 333,540 · `shot_export.png` 70,269 바이트(모두 > 5,120) |
| 21 shot_main 배치 | 육안(test-runner 관찰) | — | 관찰 일치 (ui-reviewer 판정 몫) | 빨간 단면이 축 **오른쪽**, 음영·먹 외곽선 **왼쪽**. 3D 의 흰 테 굵은 빨간 절단선이 유물 오른쪽 반, 왼쪽 반은 가는 빨간 파선. JOB-003 `shot_main.png` 와 도판이 좌우 거울상. 글자는 □(폰트 미탐색, 알려진 위험 7) → 부제·리본 문구는 캡처로 판독 불가(문구 자체는 기준 16 grep 으로 확인) |
| 22 shot_flip 배치 | 육안(test-runner 관찰) | — | 관찰 일치 (ui-reviewer 판정 몫) | 도판이 21 의 거울상(단면 왼쪽·음영 오른쪽), 3D 는 뒤에서 보아 굵은 절단선이 화면 왼쪽 반. 문구는 □ |
| 23 shot_export 배치 | 육안(test-runner 관찰) | — | 관찰 일치 (ui-reviewer 판정 몫) | 내보내기 미리보기 도판도 음영 왼쪽·빨간 단면 오른쪽(21 과 같음). 문구는 □ |

## 시간 측정
- **미리보기(LOD) 단면 시간: 해당 없음.** 이번 조각은 JOB-004a(D5 좌/우·R 반전)이고 실시간 미리보기는 004b 범위(spec.md §0·§2). 코드에 미리보기 경로·측정 출력이 없다(`test_side.cpp` 에 시간 측정 없음, builder 도 "해당 없음").
- 대신 3회 측정(중앙값):
  - `ctest -R studio_side`: 11.69 / 11.68 / 11.66 s → **중앙값 11.68 s** (`artifacts/ctest_studio_side_run{1,2,3}.log`)
  - 전체 `ctest --output-on-failure`: 30.18(verify.ps1) / 29.80 / 29.52 s → **중앙값 29.80 s**
  - `--sample --flip --shot` 캡처 1회 4.1 s

## 경고·범위
- 빌드 경고 0 (/W4, 60단계 전체 재빌드 `artifacts/02_build.log`).
- `git diff --name-only e9b655a..HEAD` = 15파일, +602 −56. 그중 `studio/tests/test_side.cpp` +424(신규). GigaMesh·factory·test_pose.cpp·docs/ui/mockup 무변경.

## 산출물 (artifacts/, 커밋하지 않음)
- verify.ps1 복사본: `01_configure.log`, `02_build.log`, `03_ctest.log`, `04_shots.log`(0 B, 캡처 명령은 출력 없음), `verify-summary-studio.md`, `shot_home.png`, `shot_main.png`, `shot_export.png`
- test-runner 추가: `shot_flip.png`(V11), `ctest_N.log`, `ctest_all.log`, `ctest_all_final.log`, `ctest_studio_side_V.log`, `ctest_studio_pipeline_V.log`, `ctest_studio_side_run{1,2,3}.log`, `LastTest_verify.log`(V2 직후), `LastTest_ctest_all.log`, `LastTest_final.log`, `indep_svg_dxf_check.log`, `studio_side_{sample,blind}_{Right,Left}.{svg,dxf}`, `studio_plate.svg`, `studio_plate.dxf`

## 비고
- 테스트 코드 검토: `test_side.cpp` 의 검사들이 명세 기준 1-9 를 약화 없이(일부는 더 엄격히 — dims 7필드, 빗금 방향, SVG 음영 영상 존재) 구현함을 줄 단위로 확인했다. JOB-003 처럼 "CTest 통과 ≠ 기준 통과" 인 항목은 찾지 못했다.
- PowerShell 로 캡처한 ctest -V 로그에서 test_pipeline 의 한국어 출력(`note=…`, `advice=…`)은 코드 페이지 차이로 깨져 보인다(로그 표시 문제, 판정 무관 — 판정 문자열은 ASCII).
- `shot_flip.png` 폭 1,871 px 은 `shot_main.png` 1,845 px 보다 26 px 넓다(Left 리본 글자가 더 김, □ 글리프 폭 기준). builder 제안 3 과 같은 관찰 — 실제 한글 폰트 화면 넘침 여부는 ui-reviewer 확인 권장.
- builder 제안 1(Left 에서 상태줄 커서 `R` 부호)·제안 2(`기고 H` 라벨이 D5 기본에서 음영 위에 겹침 — `shot_main.png` 좌상단에서 관찰됨)는 수용 기준 밖이며 사람 검토 항목으로 남긴다.
- 기준 저장소 `D:/_dongarch_build/wt` 의 `factory/jobs/JOB-004/build-log.md`·`spec.md` 미커밋 수정은 다른 역할의 것이며 test-runner 는 손대지 않았다(이 파일과 `artifacts/` 만 씀).
