# JOB-008 검증 기록 (test-runner)

결론: **PASS**. 수용 기준 1-24 를 명세 §4·§5 의 명령 그대로 돌렸고, 모두 기대값을 만족했다.
- 명령으로 판정하는 기준(1-20·23·24): 모두 통과.
  - 깨끗한 빌드(`-Clean`) 종료 코드 0, 빌드 경고 0.
  - CTest 18/18 통과. 개별 실행 `studio_fit`·`studio_cutpage` 둘 다 마지막 줄 `PASS`.
  - 독립 파이썬 판정 `PY-FIT OK`, 실제 글꼴 화소 비교 `SHOTS OK`, `shot_home` 해시 `True`.
- 관찰 기준 21·22(최종 예/아니오는 ui-reviewer): 캡처를 눈으로 보고 화소를 재 보니 둘 다 충족이다.
- `[J8-n]` 인쇄값은 모두 명세 §3.8 의 예상값과 같다. 다른 칸은 없다.

## 환경
| 항목 | 값 |
|---|---|
| 검증 대상 | `D:\_dongarch_build\wt\.worktrees\JOB-008`. 브랜치 `factory/JOB-008`. HEAD `a913448`(JOB-008 builder 커밋) ← 기준 `89846ba`(JOB-007 끝, JOB-006·007 승인 대기). `git status --porcelain` 은 시작·끝 모두 빈 출력이다. 검증 대상 worktree 는 읽기만 했다 |
| OS | Windows 11 Pro 10.0.22631 |
| 컴파일러 | MSVC 19.44.35228.0 (`C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe`), C++20, Release |
| 생성기 | Ninja(VS BuildTools 동봉 `…/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe`), CMake 4.4.1 |
| Qt | 6.11.1, `D:/OSGeo4W/apps/Qt6` (`Qt6_DIR=D:/OSGeo4W/apps/Qt6/lib/cmake/Qt6`). 시험·캡처는 `QT_QPA_PLATFORM=offscreen`. 실제 글꼴 캡처에만 `QT_QPA_FONTDIR=C:\Windows\Fonts` 를 더했다 |
| 빌드 폴더 `B` | `D:\_dongarch_build\verify_job008\studio`. `-Clean` 으로 지운 뒤 76단계 전체를 빌드했다(`[76/76] Linking CXX executable test_studio_cutpage.exe`) |
| 로그 폴더 `L` | `D:\_dongarch_build\verify_job008\studio-logs` |
| 산출물 `A8` | `D:\_dongarch_build\wt\factory\jobs\JOB-008\artifacts\`(`-RecordRoot D:\_dongarch_build\wt`) |
| Python | `C:/Users/kyi25/AppData/Local/Programs/Python/Python312/python.exe -I`, PIL 12.3.0 |
| verify.ps1 | 작업이 지정한 대로 worktree 의 `factory\scripts\verify.ps1` 을 실행했다. 기준 worktree(`D:\_dongarch_build\wt\factory\scripts\verify.ps1`)와 비교하면 줄 끝(CRLF/LF)만 다르다(`diff --strip-trailing-cr` 결과 같음) |

## 시간 (2026-10-18)
| 단계 | 시각 | 소요 |
|---|---|---|
| verify.ps1 `-Clean` 전체 | 06:00:11 → 06:03:21 | 190 s (configure 4 · build 25 · ctest 152 · shots 9) |
| `ctest -R "studio_fit\|studio_cutpage" -V` | 06:03:53 → 06:04:38 | 45 s (`studio_fit` 19.58 s · `studio_cutpage` 25.39 s, `Total Test time (real) = 44.98 sec`) |
| `ctest --output-on-failure` 재실행 | 06:04:53 → 06:07:23 | 150 s (`Total Test time (real) = 150.33 sec`) |
| 독립 파이썬 판정(기준 9) | 06:08 | 1 s 안 |
| 실제 글꼴 캡처 3장(§5-4) | 06:08:27 → 06:08:40 | 13 s |
| 화소 비교(§5-5)·compare-shots.py·모서리 비교 그림(§5-6) | 06:09 → 06:10 | — |

## 경고 수
- 빌드 경고 **0**: `grep -ci "warning" D:/_dongarch_build/verify_job008/studio-logs/02_build.log` 의 출력이 `0` 이다. `02_build.log` 의 `C[0-9]{4}` 진단 코드와 `error` 도 0개, `01_configure.log` 의 `error|warn` 도 0개다.
- 콘솔에 `'vswhere.exe' is not recognized as an internal or external command` 가 2번 찍혔다(configure·build 시작 때). vcvars64 환경 메시지라 로그와 종료 코드에는 영향이 없다(JOB-005·006·007 과 같음).
- `04_shots.log` 는 빈 파일이다(오류 출력 없음).

## 실행한 명령
1. (PowerShell) `powershell -NoProfile -ExecutionPolicy Bypass -File D:\_dongarch_build\wt\.worktrees\JOB-008\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-008 -BuildRoot D:\_dongarch_build\verify_job008 -Job JOB-008 -RecordRoot D:\_dongarch_build\wt -Clean` → **종료 코드 0**. 직후 `B\Testing\Temporary\LastTest.log` 를 `A8\LastTest_verify.log` 로 복사하고 기준 11·12·13 을 판정했다.
2. (PowerShell, 명세 §5-2 그대로) `Remove-Item Env:\CI -ErrorAction SilentlyContinue; $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job008\studio; ctest -R "studio_fit|studio_cutpage" -V` → **0**. 출력은 `A8\ctest_fit_cutpage_V.log` 에 있다.
3. 같은 환경·같은 폴더에서 `ctest --output-on-failure` → **0**. 출력은 `A8\ctest_all_rerun.log` 에 있고, 그 뒤 LastTest.log 를 `A8\LastTest_rerun.log` 로 복사했다.
4. (Git Bash) 명세 §5-3 의 파이썬 블록을 0열에서 글자 그대로 heredoc 으로 실행했다(인자 `"D:/_dongarch_build/verify_job008/studio"`) → **0**. 스크립트에 백슬래시가 없어 heredoc 이 바꾼 글자는 없다.
5. (PowerShell) 명세 §5-4 실제 글꼴 캡처. §5-2 환경에 `QT_QPA_FONTDIR` 를 더했다. 명세 명령과 다른 점은 종료 코드를 얻으려고 `Start-Process` 에 `-PassThru` 를 더한 것 하나다 → 세 실행 모두 **ExitCode 0**. 크기는 `431413`·`434478`·`406641` 바이트이고, A8 로 복사했다.
6. (Git Bash) 명세 §5-5 화소 비교 블록을 글자 그대로 실행했다 → **0**, `SHOTS OK`. 이어서 `factory/scripts/compare-shots.py`(기준 worktree)를 세 장 각각 `A7/<이름> A8/<이름>` 로 돌렸다 → 모두 **0**(띠 정보는 아래).
7. 기준 14-17: Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-008` 한 뒤 명세 §4 D·E 의 명령을 글자 그대로 실행했다(HEAD `a913448`, 트리 깨끗함).
8. 기준 18: PowerShell `(Get-FileHash "D:\_dongarch_build\verify_job008\studio\shots\shot_home.png").Hash -eq (Get-FileHash "D:\_dongarch_build\wt\factory\jobs\JOB-007\artifacts\shot_home.png").Hash`.
9. 기준 21·22 관찰: 이미지를 눈으로 보고 세션 scratchpad 의 파이썬(PIL)으로 화소를 쟀다(빨강 `R−G>100` 의 y 범위, 기준선 행, 외곽선 왼쪽 끝 어두운 화소 행, 상황 줄 확대). §5-6 사람 검토 자료 `A8\fit_corners_before_after.png` 도 만들었다(4배 NEAREST, 왼쪽 = JOB-007, 오른쪽 = JOB-008. 상자 구연 모서리 x 1400–1480·y 270–310 / 바닥 모서리 x 1400–1480·y 810–840 / 외곽선 위 끝 x 1010–1060·y 270–310 / 외곽선 아래 끝 x 1010–1060·y 810–840).
10. 시험 코드 대조: `studio/tests/test_fit.cpp`(신규 613줄)와 `test_cutpage.cpp` 의 `git diff 89846ba`(+9)를 읽었다. 각 CHECK 가 명세 §4 판정식과 같은지(값·허용오차·경우 목록) 대조했고, 다른 곳은 찾지 못했다(아래 「시험 코드 대조」).

## 수용 기준별 결과
기준 1-8·23·24 는 `test_studio_fit` 한 번 실행 안의 CHECK 들이고, 기준 10 은 `test_studio_cutpage` 안의 CHECK 다. 실패한 CHECK 는 `FAIL <식>: …` 를 찍고 마지막 줄을 `FAILED` 로 만든다. 명령 1·2·3 의 출력 어디에도 시험 쪽 `FAIL` 줄은 없다. `LastTest` 의 `FAIL AC-…` 4줄은 `karch_test_dxf` 가 일부러 넣은 변이를 거부했다고 찍는 정상 출력이다. 두 시험의 마지막 줄은 `PASS` 다.

| 기준 | 명령 | 종료 코드 | 결과 | 근거 |
|---|---|---|---|---|
| 1 단면이 기준선을 넘지 않음 | 명령 2(·1·3) | 0 | 통과 | test_fit.cpp:197-205(`|bboxMin.z| ≤ 1e-9`, raw `[rLo, rHi] ⊂ [−1e-9, H+1e-9]`, 그린 점 raw 범위 ±1e-9·`x ≥ −1e-9`, 틈 ≤ 0.30). 6경우 인쇄 예: `[J8-1] sample 0° y=[0.127, 184.685] raw=[0.127, 184.685] H=184.821 gap 0.127/0.135` … `[J8-1] blind 12° y=[0.091, 190.152] … gap 0.091/0.103` |
| 2 단면 편차 ≤ 간격/2 | 명령 2 | 0 | 통과 | :229-235(짝 수 같음·`dev2 ≤ tol/2+1e-9`), 6경우와 간격 0.1·0.3·1.0(:472-476, 끝나면 0.5). dev 0.231·0.202·0.187·0.176·0.202·0.185, 간격별 0.049·0.149·0.458 |
| 3 외곽선 끝이 바닥선·구연선 높이까지 | 명령 2 | 0 | 통과 | :207-225(점마다 y ∈ [−1e-9, H+1e-9]·`x ≤ 1e-9`, front = 최소·back = 최대, 틈 ≤ 0.25). 예: `[J8-3] sample 0° ends=(-70.627,0.125)…(-71.033,184.625) gap 0.125/0.196`, `blind 0° … gap 0.125/0.131` |
| 4 외곽선 편차 ≤ 간격/2 | 명령 2 | 0 | 통과 | :237-244(원 선 `x ≤ 0`·z 범위 안·y 순증가, `dev2 ≤ tol/2+1e-9`). dev 0.1801·0.1914·0.1545·0.1539·0.1733·0.1845, 간격별 0.0497·0.1426·0.4328. n 739(예제)·761(미스캔) |
| 5 외곽선 = 반쪽 정사 실루엣(독립 계산) | 명령 2 | 0 | 통과 | :253-258·465-471(제품 ↔ 시험 쪽 `rawHalfOrtho`, 점 수 같음·좌표 ≤ 1e-9, 예제 0° 는 전수 구현과 `==`). `[J8-5] sample 0° n=739 maxdiff=0 · brute n=739 same=1`, 12°·−20° `n=739 maxdiff=0`, `blind 0° n=761 maxdiff=0` |
| 6 최대 편차 함수 | 명령 2 | 0 | 통과 | :246-249(`|maxDeviationMm − max(단면, 외곽선)| ≤ 1e-9`·`≤ tol/2+1e-9`, 기준 2·4 의 모든 상태), :451-458(`[J8-6] empty=0.0 unconfirmed=0.0`). 값 0.2309·0.2023·0.1868·0.1757·0.2022·0.1854, 간격별 0.0497·0.1491·0.4583 |
| 7 곡선 끔 = DP 그대로 · 점 수 | 명령 2 | 0 | 통과 | :479-497(꼭짓점마다 `==` 비교). `[J8-7] smooth off: section 20 · outline 10 · points 30 \| on: section 48 · outline 34 · points 82/1343` |
| 8 출력 길(DXF·래스터)·파일 | 명령 2 + `ls B` | 0 | 통과 | (a) :287-329. `[J8-8] studio_fit_s.dxf dxf base y 0.000000 rim y 184.820570 · section 1 polyline y [0.126692, 184.685244] · outline y [0.125000, 184.625000] gap 0.125/0.196`, `studio_fit_b.dxf … section 2 polyline y [0.189583, 190.148332] · outline y [0.125000, 190.125000] gap 0.125/0.131`. (b) :512-533, 세는 화소는 도판 칸 E 안만(:362·366). `[J8-8] raster E=[-90.7, 90.7]×[-8.0, 202.8] section above 0 below 0 top 193 bottom 196 · outline above 0 below 0 [0.3,1] 23 [H-1,H-0.3] 20`. (c) 파일 6개가 `B` 에 있다: `studio_fit_s.svg` 1,539,008 B · `studio_fit_s.dxf` 8,161 · `studio_fit_b.svg` 1,511,974 · `studio_fit_b.dxf` 10,465 · `studio_fit_s_raw.csv` 45,743 · `studio_fit_b_raw.csv` 67,341. 마지막 줄 `5: PASS`, `Test #5: studio_fit ... Passed 19.58 sec` |
| 9 독립 파이썬 판정 | 명령 4 | 0 | 통과 | `PY-FIT OK`. 출력 세 줄이 명세 예상 인쇄와 글자 그대로 같다(아래 원문) |
| 10 ③ 상황 줄 최대 편차 | 명령 2 | 0 | 통과 | test_cutpage.cpp:439(② `CtxRight` 에 `최대 편차` 없음), :470-476(정규식 `^점 [0-9,]+ → <b>[0-9]+개</b> \(0\.5 mm 간략화\) · 최대 편차 (0\.[0-9]{2}) mm$` 이고 잡힌 값 ≤ 0.25). `[J8-10] CtxRight '점 1,272 → <b>94개</b> (0.5 mm 간략화) · 최대 편차 0.21 mm'`. 마지막 줄 `7: PASS`, `Test #7: studio_cutpage ... Passed 25.39 sec`. JOB-006·007 인쇄도 그대로다(`[9] … ctx minimumSizeHint width=1618`, `단면 위치 x=919`, `[J7-13]`·`[J7-15] window minimumSizeHint width=1791`) |
| 11 CTest 18종 | 명령 1(`L\03_ctest.log`), 명령 3 | 0 / 0 | 통과 | 두 로그 모두 `100% tests passed out of 18`(같은 줄). `tests failed` 가 든 줄은 없다(grep 결과 0줄) |
| 12 바뀌는 로그 값 | 명령 1 직후 `B\Testing\Temporary\LastTest.log` 에서 `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=82/1343 lodFaces=26180"` · `grep -c "pts=85/1318"` · `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"` · `grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"` · `grep -cF "outline ends=(-70.627,0.125)…(-71.033,184.625) gap low 0.125 high 0.196 mm"` | 0 / 1(개수 0) / 0 / 0 / 0 | 통과 | `1`·`0`·`1`·`1`·`1`(기대 ≥1·=0·≥1·≥1·≥1). 명령 3 뒤 `LastTest_rerun.log` 도 `1`·`0`·`1`·`1`·`1` 이다. 두 로그의 `[J8-` 줄은 `diff` 로 서로 같다 |
| 13 verify 종료 0·PASS·경고 0 | 명령 1 / `tail -1 B\verify-summary.md` / `grep -ci "warning" D:/_dongarch_build/verify_job008/studio-logs/02_build.log` | 0 / 0 / 1(개수 0) | 통과 | 종료 0 / `결과: PASS` / `0` |
| 14 변경 범위 | `git status --porcelain` / `git diff --name-only 89846ba HEAD \| grep -vxF -e …`(허용 7개, 명세 그대로) / `git diff --name-only 89846ba HEAD -- GigaMesh studio/src/Plate.h … studio/tests/test_reflines.cpp`(명세 그대로) | 0 / 1(일치 없음) / 0 | 통과 | 셋 다 출력 없음. 바뀐 파일은 허용 목록 7개뿐이다(`7 files changed, 833 insertions(+), 43 deletions(-)`: UI_RULES.md +1, CMakeLists.txt +4, ArtifactDoc.cpp 211, ArtifactDoc.h 33, MainWindow.cpp 5, test_cutpage.cpp +9, test_fit.cpp +613) |
| 15 흐름 코드 | `grep -c` 8개 + `grep -rn "smoothCatmull" studio/src`(명세 그대로) | `TrueOrtho`·`sil_.leftOutline`·`smoothCatmull` 은 1(없음), 나머지 0 | 통과 | `SilhouetteMode::TrueOrtho` 0(=0) · `SilhouetteMode::MaxRadius` 1(=1) · `halfOrthoSilhouette(mesh_, 0.25)` 1(=1) · `sil_.leftOutline` 0(=0) · `smoothCatmull` 출력 없음 · `fitCurve(` 3(≥2) · h 의 `maxDeviationMm` 1(≥1) · MainWindow `maxDeviationMm()` 1(=1) · `· 최대 편차 %4 mm` 1(=1) |
| 16 R2 새 색 없음 | `git diff 89846ba -- studio/src \| grep -E '^\+.*(#[0-9A-Fa-f]{6}\|QColor\(\|qRgba?\(\|Qt::(red\|green\|blue\|yellow\|white\|black\|gray)\b)'` | 1(일치 없음) | 통과 | 출력 없음 |
| 17 정본 | `grep -c "막과 수직인 실루엣의 외면 반쪽"` · `grep -c "실측 점을 잇는 간략 꺾은선"` · `grep -c "최대 편차"` (`docs/ui/UI_RULES.md`) | 0 / 0 / 0 | 통과 | `1`·`1`·`1`(각 ≥1). R5 기준선 줄(74) 다음 75행에 「선 다듬기」 줄 하나가 들어갔다(명세 §3.6 문구와 같음) |
| 18 시작 화면 무변경 | 명령 8 | 0 | 통과 | `True`. 두 파일 모두 13,246 B · 1845×920 · SHA256 `00FE52AEFB6E790E92B16511E08017B7EE3CFD7B4AC7CD9EB2EDC992732DBA25` |
| 19 오프스크린 캡처 크기 | 명령 1 `shots`(종료 0) + 파일 크기 | 0 | 통과 | `shot_main.png` 367,514 B(1845×920) · `shot_export.png` 70,974 B(1296×680), 둘 다 > 5120. 오프스크린이라 한글은 모두 □(폰트 미탐색)이고 배치는 정상이다(리본·3D·슬롯 목록·도판·상태줄). 도판 단면 꼭대기가 구연선에서 평평하게 끝난다 |
| 20 실제 글꼴 화소 비교 | 명령 6(§5-5 블록) | 0 | 통과 | `shot_main_font.png inside 2593 outside 0` · `shot_cut_font.png inside 2523 outside 0` · `shot_edit_font.png inside 3971 outside 0` → `SHOTS OK`. 세 장 모두 1845×920 |
| 21 홈 0°·② 12° 모양 | 명령 5 + 눈 확인 + 화소 측정(명령 9) | 0 | **충족(관찰)**. 최종 예/아니오는 ui-reviewer | 빨강 단면 y 범위(화면 px)와 기준선 행: 홈은 **292–826**, 구연선 행 292–293·바닥선 행 826(JOB-007 은 283–832로 구연선 위 9 px·바닥선 아래 6 px 넘쳤다). ② 는 **311–844**, 구연선 310–311·바닥선 844(JOB-007 은 303–850). 왼쪽 외면 외곽선(x ≈ 1028–1031 의 어두운 화소)은 홈에서 y 294 → 826, ② 에서 311 → 844 로 구연선·바닥선 바로 앞까지 이어진다(JOB-007 은 홈 303 → 816, ② 321 → 834 로 위·아래 각각 9–10 px 틈). 구연 윗면·바닥 변은 곧은 가로 꺾은선으로 보인다(`fit_corners_before_after.png` 1·2행) |
| 22 ③ 12° 상황 줄·모양 | 명령 5 + 눈 확인 + 확대(명령 9) | 0 | **충족(관찰)**. 최종 예/아니오는 ui-reviewer | 상황 줄 오른쪽 `점 1,222 → 82개 (0.5 mm 간략화) · 최대 편차 0.20 mm`, `82개` 는 굵게(JOB-007 은 `점 1,197 → 82개 (0.5 mm 간략화)`). 도판 빨강 y 311–844 로 21 의 ② 와 같은 모양이다. 리본 5그룹(`파일`·`고정된 막`·`선 다듬기 · 간략화`·`도판 표시 (겹침)`·`내보내기`)과 `막 12° 고정`, 3D 보조창 x 176–506(330 px), 상태줄 `⑧ 치수 확인 — …`·`막 12° 고정`. 기준 20 에서 도판 칸·상황 줄 오른쪽 밖의 화소 차이가 0 이므로 나머지는 JOB-007 과 화소까지 같다 |
| 23 기운 바닥·구연 | 명령 2 | 0 | 통과 | test_fit.cpp:394-440, 5경우 (a)–(e) CHECK. 기울임 식은 §3.7 과 같다(:394-401, b = −0.3·−0.5, r = +0.5·+1.0, 앞쪽 `s = −y`). 인쇄 5줄(아래 원문)이 §3.8 기운 예제 표의 H·n·끝·틈·maxX·olDev·단면 y 범위·secDev·maxDev 와 모두 같고 `maxdiff 0` 이다 |
| 24 25만 면 초과 | 명령 2 | 0 | 통과 | :569-603(합성 매개변수가 §3.7 과 같음, `faces > 250000`·`lod < faces`, 원본 실루엣과 같음, `x ≤ 1e-9`, 틈 ≤ 0.25, olDev·maxDev ≤ 0.25). `[J8-24] faces=346320 lod=31103 n=739 ends=(-71.002,0.125)…(-71.027,184.625) gap 0.125/0.179 olDev=0.174 maxDev=0.222 maxdiff=0 · dims base=5.20 wall=3.85 (LOD 치수 — 판정 아님)` — 예상과 같다. LOD 치수 결함(저경 5.20·기벽 3.85)이 그대로 재현된다(명세 §2·§10 범위 밖) |

## 시험 코드 대조 (판정식 ↔ 명세 §4)
- **경우 목록**: 기준 1·2·3·4·6 = 예제 0°·12°·−20°·105°, 미스캔 0°·12°(Right·간격 0.5·곡선 켬 기본), 기준 2·4·6 은 예제 0° 간격 0.1·0.3·1.0 도 있다. 기준 5 = 예제 0°·12°·−20°, 미스캔 0°(+ 0° 전수 구현). 명세와 같다.
- **원 자료**: `d.sectionLines()` 를 쓴다(§3.5·§3.7). 미스캔 경로 조건 `!solids.empty() && !estInner.empty()` 도 같다.
- **시험 쪽 반쪽 정사 실루엣** `rawHalfOrtho`(:97-114)·`rawHalfOrthoAll`(:116-127): 제품 함수를 부르지 않는다. 칸 식 `bboxMin.z + (k+0.5)·0.25 ≤ bboxMax.z`, 교점 규칙(`|u.z−zk| ≤ 1e-12 ∧ u.x ≤ 0`, `|dz| ≥ 1e-15`, `t ∈ (0,1]`, `x ≤ 0`), 면마다 ±1 칸 여유가 모두 §3.7 과 같다.
- **허용오차**: 1e-9(도면 mm)·2e-6(DXF)·`tol/2 + 1e-9`(편차)·0.30/0.25(틈)로 명세와 같다.
- **기준 24 의 틈 판정** :595: `front().y − bboxMin.z ≤ 0.25` 처럼 bboxMin.z 를 빼서 잰다. 명세는 `front().y ≤ 0.25` 다. 확정 뒤 바닥 Z = 0 이라 둘은 같은 판정이다. 인쇄 `ends=(…,0.125)` 로도 절대값 기준을 만족한다.
- **기준 8(b) 래스터**: `grid = shade = labels = false` 이고 `hatch` 는 기본 false(Plate.h:20)다. 외곽선 래스터는 `outline = true` 로 기준선도 함께 그리지만, 세는 열이 x ≤ −2·도판 칸 E 안이라 명세 「나머지 끔」 과 같은 판정이다(기준선은 y 0·H 행만 칠하고, 세는 행은 그 밖이다).
- **기준 10**: test_cutpage.cpp:439 는 [J7-13] 창 최소 폭 판정 바로 다음, :470-476 은 [J7-15] `startsWith("점 ")` 판정 바로 다음에 있다. JOB-006·007 판정 줄은 바뀌지 않았다(diff 는 +9줄, 삭제 0).

## 주요 출력 원문

`L\03_ctest.log`(명령 1). 명령 3 재실행(`A8\ctest_all_rerun.log`)도 18개가 모두 `Passed` 이고 `100% tests passed out of 18`, `Total Test time (real) = 150.33 sec` 이다.
```
 1/18 Test  #1: studio_pipeline ..................   Passed    7.56 sec
 2/18 Test  #2: studio_pose ......................   Passed    8.70 sec
 3/18 Test  #3: studio_side ......................   Passed   11.97 sec
 4/18 Test  #4: studio_reflines ..................   Passed   10.27 sec
 5/18 Test  #5: studio_fit .......................   Passed   19.58 sec
 6/18 Test  #6: studio_cut .......................   Passed   65.87 sec
 7/18 Test  #7: studio_cutpage ...................   Passed   25.47 sec
 8/18 Test  #8: karch_test_taubin_annulus ........   Passed    0.04 sec
 ...
18/18 Test #18: karch_test_synthetic .............   Passed    0.19 sec

100% tests passed out of 18

Total Test time (real) = 151.60 sec
```

`LastTest_verify.log`(명령 1 직후)의 기준 12 줄과 `[J7-1]` 줄:
```
H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=82/1343 lodFaces=26180
STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2 unit=m→mm 자동환산(×1000)
ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95
[J7-1] sample H=184.8206 rim=142.00 base=142.00 outline ends=(-70.627,0.125)…(-71.033,184.625) gap low 0.125 high 0.196 mm
```
(JOB-007 은 `pts=85/1318`, `[J7-1] … ends=(-71.700,3.362)…(-71.697,181.612) gap low 3.362 high 3.208 mm` 였다. 치수 H·rim·belly·base·wall·STL·ArtecBlind 줄은 그대로다.)

`ctest -R "studio_fit|studio_cutpage" -V`(명령 2, `A8\ctest_fit_cutpage_V.log`). 앞의 `5: `·`7: ` 접두를 떼고 옮겼다. 명령 1·3 의 LastTest 에서도 같은 줄이 나온다.
```
[J8-6] empty=0.0 unconfirmed=0.0
[J8-1] sample 0° y=[0.127, 184.685] raw=[0.127, 184.685] H=184.821 gap 0.127/0.135
[J8-3] sample 0° ends=(-70.627,0.125)…(-71.033,184.625) gap 0.125/0.196 · |x|−base/2 -0.373 · |x|−rim/2 0.033
[J8-2] sample 0° tol=0.5 dev=0.231
[J8-4] sample 0° tol=0.5 n=739 dev=0.1801
[J8-6] sample 0° tol=0.5 maxDeviationMm=0.2309 = max(section 0.2309, outline 0.1801)
[J8-5] sample 0° n=739 maxdiff=0 · brute n=739 same=1
[J8-2] sample 0° tol=0.1 dev=0.049
[J8-4] sample 0° tol=0.1 n=739 dev=0.0497
[J8-6] sample 0° tol=0.1 maxDeviationMm=0.0497 = max(section 0.0492, outline 0.0497)
[J8-2] sample 0° tol=0.3 dev=0.149
[J8-4] sample 0° tol=0.3 n=739 dev=0.1426
[J8-6] sample 0° tol=0.3 maxDeviationMm=0.1491 = max(section 0.1491, outline 0.1426)
[J8-2] sample 0° tol=1.0 dev=0.458
[J8-4] sample 0° tol=1.0 n=739 dev=0.4328
[J8-6] sample 0° tol=1.0 maxDeviationMm=0.4583 = max(section 0.4583, outline 0.4328)
[J8-7] smooth off: section 20 · outline 10 · points 30 | on: section 48 · outline 34 · points 82/1343
[J8-8] studio_fit_s.dxf dxf base y 0.000000 rim y 184.820570 · section 1 polyline y [0.126692, 184.685244] · outline y [0.125000, 184.625000] gap 0.125/0.196
[J8-8] raster E=[-90.7, 90.7]×[-8.0, 202.8] section above 0 below 0 top 193 bottom 196 · outline above 0 below 0 [0.3,1] 23 [H-1,H-0.3] 20
[J8-1] sample 12° y=[0.114, 184.690] raw=[0.114, 184.690] H=184.821 gap 0.114/0.130
[J8-3] sample 12° ends=(-71.003,0.125)…(-71.065,184.625) gap 0.125/0.196 · |x|−base/2 0.003 · |x|−rim/2 0.065
[J8-2] sample 12° tol=0.5 dev=0.202
[J8-4] sample 12° tol=0.5 n=739 dev=0.1914
[J8-6] sample 12° tol=0.5 maxDeviationMm=0.2023 = max(section 0.2023, outline 0.1914)
[J8-5] sample 12° n=739 maxdiff=0
[J8-1] sample −20° y=[0.153, 184.645] raw=[0.151, 184.711] H=184.821 gap 0.153/0.175
[J8-3] sample −20° ends=(-70.263,0.125)…(-71.061,184.625) gap 0.125/0.196 · |x|−base/2 -0.737 · |x|−rim/2 0.061
[J8-2] sample −20° tol=0.5 dev=0.187
[J8-4] sample −20° tol=0.5 n=739 dev=0.1545
[J8-6] sample −20° tol=0.5 maxDeviationMm=0.1868 = max(section 0.1868, outline 0.1545)
[J8-5] sample −20° n=739 maxdiff=0
[J8-1] sample 105° y=[0.129, 184.641] raw=[0.129, 184.641] H=184.821 gap 0.129/0.179
[J8-3] sample 105° ends=(-70.969,0.125)…(-71.058,184.625) gap 0.125/0.196 · |x|−base/2 -0.031 · |x|−rim/2 0.058
[J8-2] sample 105° tol=0.5 dev=0.176
[J8-4] sample 105° tol=0.5 n=739 dev=0.1539
[J8-6] sample 105° tol=0.5 maxDeviationMm=0.1757 = max(section 0.1757, outline 0.1539)
[J8-1] blind 0° y=[0.190, 190.148] raw=[0.190, 190.148] H=190.256 gap 0.190/0.107
[J8-3] blind 0° ends=(-62.078,0.125)…(-62.031,190.125) gap 0.125/0.131 · |x|−base/2 0.078 · |x|−rim/2 0.031
[J8-2] blind 0° tol=0.5 dev=0.202
[J8-4] blind 0° tol=0.5 n=761 dev=0.1733
[J8-6] blind 0° tol=0.5 maxDeviationMm=0.2022 = max(section 0.2022, outline 0.1733)
[J8-5] blind 0° n=761 maxdiff=0
[J8-8] studio_fit_b.dxf dxf base y 0.000000 rim y 190.255610 · section 2 polyline y [0.189583, 190.148332] · outline y [0.125000, 190.125000] gap 0.125/0.131
[J8-1] blind 12° y=[0.091, 190.152] raw=[0.091, 190.152] H=190.256 gap 0.091/0.103
[J8-3] blind 12° ends=(-62.010,0.125)…(-62.053,190.125) gap 0.125/0.131 · |x|−base/2 0.010 · |x|−rim/2 0.053
[J8-2] blind 12° tol=0.5 dev=0.185
[J8-4] blind 12° tol=0.5 n=761 dev=0.1845
[J8-6] blind 12° tol=0.5 maxDeviationMm=0.1854 = max(section 0.1854, outline 0.1845)
[J8-23] base 0.3° H=185.123 n=738 ends=(-49.203,0.625)…(-71.042,184.875) gap 0.625/0.248 maxX=-49.203 olDev=0.190 secDev=0.224 maxDev=0.224 (section y [0.094, 184.988], maxdiff 0)
[J8-23] base 0.5° H=185.365 n=738 ends=(-28.593,0.875)…(-71.042,185.125) gap 0.875/0.240 maxX=-28.593 olDev=0.161 secDev=0.224 maxDev=0.224 (section y [0.114, 185.230], maxdiff 0)
[J8-23] rim 0.5° H=185.349 n=739 ends=(-70.630,0.125)…(-12.018,184.625) gap 0.125/0.724 maxX=-12.018 olDev=0.239 secDev=0.231 maxDev=0.239 (section y [0.127, 185.248], maxdiff 0)
[J8-23] rim 1.0° H=185.951 n=739 ends=(-70.632,0.125)…(-6.047,184.625) gap 0.125/1.326 maxX=-6.047 olDev=0.180 secDev=0.231 maxDev=0.231 (section y [0.127, 185.863], maxdiff 0)
[J8-23] rim 1.0° front H=185.981 n=744 ends=(-70.626,0.125)…(-15.252,185.875) gap 0.125/0.106 maxX=-15.252 olDev=0.201 secDev=0.230 maxDev=0.230 (section y [0.127, 184.686], maxdiff 0)
[J8-24] faces=346320 lod=31103 n=739 ends=(-71.002,0.125)…(-71.027,184.625) gap 0.125/0.179 olDev=0.174 maxDev=0.222 maxdiff=0 · dims base=5.20 wall=3.85 (LOD 치수 — 판정 아님)
[J8-8] files: studio_fit_s/b .svg .dxf _raw.csv in D:/_dongarch_build/verify_job008/studio
PASS
1/2 Test #5: studio_fit .......................   Passed   19.58 sec
[9] ribbon height=107 px (JOB-005 계산값 107)
[9] ctxHint width=174 / sizeHint=612 px · ctx minimumSizeHint width=1618 px (offscreen 글꼴)
[9] 단면 위치 x=919
[11] 단면 위치 x=919
[13] 단면 위치 x=919
[J7-13] window minimumSizeHint width=1791
[J8-10] CtxRight '점 1,272 → <b>94개</b> (0.5 mm 간략화) · 최대 편차 0.21 mm'
[J7-15] window minimumSizeHint width=1791
PASS
2/2 Test #7: studio_cutpage ...................   Passed   25.39 sec
100% tests passed out of 2
Total Test time (real) =  44.98 sec
```
§3.8 예상값과 대조한 결과(단면 y 범위·틈·편차, 외곽선 끝·n·편차, maxDeviationMm, 점 수 82/1343·20/10/30·48/34, 간격별 값, 기운 예제 5줄, 34.6만 면 줄, ③ 시험 경로 `1,272 → 94개 · 0.21`): **모든 칸이 같다**. 인쇄 자리수는 예상값의 반올림 자리와 맞춰 봤다. 예: 0.1801 → 0.180, 0.1545 → 0.155.

독립 파이썬 판정(명령 4, 기준 9):
```
s section y [0.127, 184.685] H 184.821 dev 0.231 | outline y [0.125, 184.625] dev(ortho) 0.180
b section y [0.190, 190.148] H 190.256 dev 0.202 | outline y [0.125, 190.125] dev(ortho) 0.173
svg rim y 47.090 base y 231.910 | section [47.225, 231.784] outline [47.285, 231.785]
PY-FIT OK
EXIT=0
```

화소 비교(명령 6, §5-5 블록, 기준 20):
```
shot_main_font.png inside 2593 outside 0 first outside None
shot_cut_font.png inside 2523 outside 0 first outside None
shot_edit_font.png inside 3971 outside 0 first outside None
SHOTS OK
EXIT=0
```
`compare-shots.py`(A7 → A8) 의 차이 띠. 모두 도판 단면·외곽선 자리이고, ③ 은 상황 줄 오른쪽 y 154–165 · x 1530–1831 이 더 있다. 판정 근거가 아닌 참고다.
```
shot_main_font.png: band y 283-439 x 1005-1457 · y 442-512 x 998-1005 · y 521-577 x 996-998 · y 592-593 · y 604-615 · y 623-664 · y 670-837 x 1003-1481
shot_cut_font.png : band y 302-476 x 1002-1456 · y 479-565 x 996-1002 · y 581-634 · y 639-694 · y 698-851 x 1004-1456
shot_edit_font.png: band y 154-165 x 1530-1831 · y 302-476 x 941-1395 · y 479-565 · y 580-590 · y 592-634 · y 639-694 · y 697-851 x 944-1396
```

캡처 크기(바이트, 모두 1845×920, export 만 1296×680): `shot_home.png` 13,246 · `shot_main.png` 367,514 · `shot_export.png` 70,974 · `shot_main_font.png` 431,413 · `shot_cut_font.png` 434,478 · `shot_edit_font.png` 406,641.

## 산출물 (`A8` = `D:\_dongarch_build\wt\factory\jobs\JOB-008\artifacts\`, 커밋하지 않음)
- 로그: `01_configure.log` · `02_build.log` · `03_ctest.log` · `04_shots.log`(빈 파일) · `verify-summary-studio.md` · `LastTest_verify.log`(명령 1 직후) · `ctest_fit_cutpage_V.log`(명령 2) · `ctest_all_rerun.log`·`LastTest_rerun.log`(명령 3)
- 캡처: `shot_home.png` · `shot_main.png` · `shot_export.png`(오프스크린, □ = 폰트 미탐색) · `shot_main_font.png` · `shot_cut_font.png` · `shot_edit_font.png`(실제 글꼴)
- 파이썬 판정 입력(`B` 에서 복사): `studio_fit_s.svg` · `studio_fit_s.dxf` · `studio_fit_b.svg` · `studio_fit_b.dxf` · `studio_fit_s_raw.csv` · `studio_fit_b_raw.csv`
- 사람 검토 자료(§5-6, 판정 아님): `fit_corners_before_after.png`(652×596). 행 순서는 구연 모서리 → 바닥 모서리 → 외곽선 위 끝 → 외곽선 아래 끝이고, 각 행 왼쪽이 JOB-007, 오른쪽이 JOB-008 이다

## 비고
- 코드·시험·명세는 고치지 않았다. 이 검증에서 쓴 파일은 이 `verify.md` 와 `A8` 의 산출물뿐이다. 검증용 보조 스크립트(모서리 비교 그림·확대·화소 측정)는 세션 scratchpad 에만 두었다.
- 명세와 다르게 한 점은 셋이다.
  1. 명령 5 에서 종료 코드를 얻으려고 `Start-Process -PassThru` 를 더했다.
  2. verify.ps1 은 작업 지시대로 worktree 경로의 것을 절대 경로로 실행했다. 기준 worktree 의 것과 내용이 같다(줄 끝만 다름).
  3. 기준 20 은 명세 §5-5 블록으로 판정했고, 작업 지시의 `compare-shots.py` 는 띠 위치 참고용으로 함께 돌렸다.
- 기준 21·22 는 관찰 판정이다. 「충족」 은 이 기록의 관찰이고, 최종 예/아니오는 ui-reviewer 몫이다.
- 기준 24 인쇄의 `dims base=5.20 wall=3.85` 는 25만 면 초과 메시의 LOD 치수 결함(실제 142·6.8)이다. 명세 §2·§8-10·§10 이 범위 밖 후속으로 적은 것이 그대로 재현됐다(판정 아님).
- builder 가 build-log 에 적은 인쇄값·캡처 크기·`SHOTS OK` 수치(inside 2593/2523/3971)와 이번 깨끗한 빌드의 값이 모두 같다.

## 추기 (오케스트레이터) — 완전성 점검이 더한 근거
- **정본 R5 문장 반례**: 미스캔 예제 간격 0.1 에서 추정 내벽선 편차 **0.0563 mm > 0.05**(간격/2). 곡선 켬·끔 공통, `artifacts/studio_fit_b_raw.csv` 로 파이썬 재현이 제품 값과 일치(간격 0.5 에서 5e-7). 기준 2·4·6 은 예제만, 미스캔은 간격 0.5 만 보므로 수용 기준 밖이다. 화면은 `최대 편차 0.06 mm` 로 정직하게 보인다.
- **성능**: `test_cut [d]` 미리보기 중앙값 0.72 → 0.91 ms(한도 16 ms), 원본 다시 계산 1340.7 → 1389.7 ms(JOB-007 `LastTest_verify.log` ↔ JOB-008). `maxDeviationMm` 시간은 재지 않았다.

