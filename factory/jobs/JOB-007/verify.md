# JOB-007 검증 기록 (test-runner)

결론: **FAIL** — 실패 기준 **32**(캡처 관찰 판정, 최종 예/아니오는 ui-reviewer).
- 명령으로 판정하는 기준 1-31 은 모두 기대값과 같다(빌드 경고 0, CTest 17/17, 파이썬 `PY-SVG OK`·`PY-DXF OK`, `shot_home` 해시 `True`).
- 관찰 기준 33·34·35 는 충족이다(34 의 `✎` 글리프는 □ = 폰트 미탐색, 문자열은 기준 15 가 확인. 35 의 표제란 방위각 글은 판독이 어려워 기준 7 로 갈음).
- **기준 32 불충족 근거**: 오프스크린 `shot_main.png` 가 JOB-006 과 다른 곳이 (a)·(b)·(d) 말고도 더 있다. ① 3D 보조창 폭 271 → **451 px**(3D 모형이 크게 그려짐), ② 도판이 오른쪽으로 **+90 px**, 도판 머리줄·치수 띠 글은 +180 px 옮겨짐, ③ (c) 「그리는 순서」 글이 길어 보이지 않는다. 라벨 폭이 453 → 273 px 로 줄어 □ **23칸만** 보인다(JOB-006 은 38칸 전부, 새 글은 47자). 명세 32 는 「작업 공간·3D 같음」이라 적고 있다.
  원인은 builder 가 적은 「명세와 다르게 판단한 곳 1」이다(`studio/src/SheetView.cpp` 의 `order_->setMinimumWidth(0)` → `1`, build-log.md). 실제 글꼴 캡처(33·34)에서는 새 글 전체가 보이고, 3D 폭 451 px 은 R1 의 「≈27 %」 배치와 같다. 이 차이를 받아들일지(명세 32 고침) 재작업할지는 ui-reviewer·코디네이터가 정할 일이다. 이 기록에서는 고치지 않았다.

## 환경
| 항목 | 값 |
|---|---|
| 검증 대상 | `D:\_dongarch_build\wt\.worktrees\JOB-007`, 브랜치 `factory/JOB-007`, HEAD `89846ba`(007b) ← `6542878`(007a) ← 기준 `688b7bd`(JOB-006 끝, 승인 대기). `git status --porcelain` 은 시작·끝 모두 빈 출력이다. 검증 대상 worktree 는 읽기만 했다 |
| OS | Windows 11 Pro 10.0.22631 |
| 컴파일러 | MSVC 19.44.35228.0 (`C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe`), C++20, Release |
| 생성기 | Ninja(VS BuildTools 동봉 `…/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe`), CMake 4.4.1 |
| Qt | `D:/OSGeo4W/apps/Qt6` (`Qt6_DIR=D:/OSGeo4W/apps/Qt6/lib/cmake/Qt6`). 시험·캡처는 `QT_QPA_PLATFORM=offscreen`. 실제 글꼴 캡처만 `QT_QPA_FONTDIR=C:\Windows\Fonts` 를 추가했다 |
| 빌드 폴더 `B` | `D:\_dongarch_build\verify_job007\studio`. `-Clean` 으로 지운 뒤 74단계 전체 빌드(`[74/74] Linking CXX executable DongArch3DStudio.exe`) |
| 로그 폴더 `L` | `D:\_dongarch_build\verify_job007\studio-logs` |
| 산출물 | `D:\_dongarch_build\wt\factory\jobs\JOB-007\artifacts\`(`-RecordRoot D:\_dongarch_build\wt`) |
| Python | `C:/Users/kyi25/AppData/Local/Programs/Python/Python312/python.exe -I` (기준 10·11, 캡처 화소 분석에 PIL 12.3.0·numpy 2.5.1) |

## 시간 (2026-10-18)
| 단계 | 시각 | 소요 |
|---|---|---|
| verify.ps1 `-Clean` 전체 | 03:14:02 → 03:16:50 | 168 s (configure 4 · build 23 · ctest 131 · shots 9) |
| `ctest -R "studio_reflines\|studio_cutpage" -V` | 03:17:12 | 36 s (`studio_reflines` 10.15 s · `studio_cutpage` 25.39 s, `Total Test time (real) = 35.55 sec`) |
| `ctest --output-on-failure` 재실행 | 03:18:01 → 03:20:10 | 129 s (`Total Test time (real) = 129.16 sec`) |
| 파이썬 기준 10·11 | 03:20 | 각 1 s 안 |
| 실제 글꼴 캡처 3장(§5-4) | 03:21:04 | `--cut` 4,088 ms · `--cut --edit` 4,044 ms · `--export` 4,032 ms |

## 경고 수
- 빌드 경고 **0**. `grep -ci "warning" D:/_dongarch_build/verify_job007/studio-logs/02_build.log` 의 출력이 `0` 이다. `C[0-9]{4}` 진단 코드도 0개, `01_configure.log` 의 `error|warn` 도 0개다.
- 콘솔에 `'vswhere.exe' is not recognized as an internal or external command` 가 2번 찍혔다(configure·build 시작 때). vcvars64 환경 메시지라서 로그와 종료 코드에는 영향이 없다(JOB-005·006 과 같음).

## 실행한 명령
1. `powershell -NoProfile -ExecutionPolicy Bypass -File D:\_dongarch_build\wt\.worktrees\JOB-007\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-007 -BuildRoot D:\_dongarch_build\verify_job007 -Job JOB-007 -RecordRoot D:\_dongarch_build\wt -Clean` → **종료 코드 0**. 곧바로 `B\Testing\Temporary\LastTest.log` 를 `artifacts\LastTest_verify.log` 로 복사하고 기준 21·22·23·31·32 를 판정했다.
2. (PowerShell, 명세 §5-2 그대로) `Remove-Item Env:\CI -ErrorAction SilentlyContinue; $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job007\studio; ctest -R "studio_reflines|studio_cutpage" -V` → **0** (`artifacts\ctest_reflines_cutpage_V.log`)
3. 같은 환경·같은 폴더에서 `ctest --output-on-failure` → **0** (`artifacts\ctest_all_rerun.log`). 그 뒤 LastTest.log 를 `artifacts\LastTest_rerun.log` 로 복사했다.
4. (Git Bash) 명세 §5-3 의 파이썬 두 블록을 0열에서 글자 그대로 heredoc 으로 실행했다 → 둘 다 **0**. heredoc 이 백슬래시를 줄이지 않았는지는 같은 방식으로 `print(repr('\n'))` 를 돌려 `'\n' 1` 로 확인했다. DXF 블록의 `split('\n')` 은 그대로 전달됐다.
5. (PowerShell) 명세 §5-4 실제 글꼴 캡처. §5-2 환경 변수에 `QT_QPA_FONTDIR='C:\Windows\Fonts'` 를 더했다. 명세 명령과 다른 점은 종료 코드를 얻으려고 `Start-Process` 에 `-PassThru` 를 더한 것 하나다 → 세 실행 모두 **ExitCode 0**, 크기 `434828`·`407040`·`116051`. artifacts 로 복사했다.
6. 기준 24-30: Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-007` 한 뒤 명세 §4 E 의 명령을 글자 그대로 실행했다(트리 깨끗한 상태).
7. 기준 31: PowerShell `(Get-FileHash "D:\_dongarch_build\verify_job007\studio\shots\shot_home.png").Hash -eq (Get-FileHash "D:\_dongarch_build\wt\factory\jobs\JOB-006\artifacts\shot_home.png").Hash`.
8. 캡처 판정(32-35): 이미지를 눈으로 보고, 세션 scratchpad 의 파이썬(PIL·numpy) 스크립트로 화소를 분석했다. 분석한 것은 가로 경계 y, 3D 보조창 오른쪽 끝, 이동량을 보정한 도판 화소 차이, 주 단추 빨강(`#B5573A` ±30) 타일 수, 단면 빨강 화소 모양, 「그리는 순서」 □ 칸 수다.
9. 시험 코드 대조: `studio/tests/test_reflines.cpp`(신규 453줄)와 `test_cutpage.cpp` 의 `git diff 688b7bd`(+191) 를 읽었다. 각 CHECK 가 명세 §4 판정식과 같은지(값·허용오차·순서) 확인했고, 다른 곳은 찾지 못했다. 조금 더 엄격한 곳이 둘 있다. 기준 6 은 층 안 자식 요소가 `<line>` 1개뿐인지까지 보고, 기준 13·15 는 Primary 타일 목록이 `[toEdit]`·`[export]` 와 정확히 같은지 본다. `cp::kEditBadge` 등 상수의 글자는 `CutPageText.h` 를 읽어 명세 §3.2 와 같음을 확인했다.

## 수용 기준별 결과
기준 1-9 는 `test_studio_reflines` 한 번 실행 안의 CHECK 들이다. 기준 12-20 은 `test_studio_cutpage` 한 번 실행 안의 CHECK 들이다(JOB-006 [1]-[17] 에 이어서 실행). 실패한 CHECK 는 `FAIL <식>: …` 를 찍고, 그러면 마지막 줄이 `FAILED` 가 된다. 명령 1·2·3 출력 어디에도 `FAIL` 줄이 없고, 두 시험의 마지막 줄은 `PASS` 다. 행 번호는 그 파일 기준이다.

| 기준 | 명령 | 종료 코드 | 결과 | 근거 |
|---|---|---|---|---|
| 1 기하(예제) | 명령 2(·1·3) | 0 | 통과 | test_reflines.cpp:47-58(bboxMin.z·H≈dims.height·center/rim/base 1e-9·ext 9). 인쇄 `[J7-1] sample H=184.8206 rim=142.00 base=142.00 outline ends=(-71.700,3.362)…(-71.697,181.612) gap low 3.362 high 3.208 mm` |
| 2 좌/우 | 명령 2 | 0 | 통과 | :331-338 `==` 비교. `[J7-2] Left == Right: 1` |
| 3 미스캔 예제 | 명령 2 | 0 | 통과 | :430-438. `[J7-3] blind H=190.2556 rim=124.00 base=124.00 rim.a.x=-62.000 base line=1`(기대 −62.0) |
| 4 확정 전·빈 문서 | 명령 2 | 0 | 통과 | :440-449. `[J7-4] empty/unconfirmed: none` |
| 5 정면 방위각 문구 | 명령 2 | 0 | 통과 | :340-341·410·420·424(0°·가로 슬롯 ""·12°·−20°(U+2212)·−0.04° → 0°). `[J7-5] notes: 0° · 12° · −20° · −0.04° → 정면 방위각 0° (위에서 보아 반시계)` |
| 6 SVG 층 | 명령 2 | 0 | 통과 | :125-172·372-380(`_s1`·`_s3`·`_L` 층 id 7개 순서, 층마다 line 1, 레이블·`#141413`·0.10/0.13/0.25·dasharray 없음·좌표 ±0.002, `outline_axis` line 0·polyline 1, 방안 먹 바닥선 없음, `6 1 1 1` 없음, `_h` = `[grid_dims_scale, section_profile]`). `[J7-6] svg layers checked (_s1 · _s3 · _L · _h)` |
| 7 SVG 정면 방위각 | 명령 2 | 0 | 통과 | :381-382·414. FAIL 줄 없음 |
| 8 DXF | 명령 2 | 0 | 통과 | :218-264·383·415·422·426(LAYER 12개 이름 순서, 01/07/08 ACI 7·CONTINUOUS, LINE 1개씩·코드 6 없음, 02 LINE 없음, 좌표 ±2e-6, 방위각 TEXT 정확히 1·`FRONT AZIMUTH*` 1. `_m20` `-20.0`·`_m004` `0.0`). `[J7-8] dxf 0deg checked` · `[J7-8] dxf 12deg checked` |
| 9 래스터·머리표 | 명령 2 | 0 | 통과 | :387-406(s=1 ink 8개, s=1/3 ink 4개, labels caution·ink 5개 — 명세 식과 같음). `[J7-9] raster s=1 1 · s=1/3 1 · labels 1`, 마지막 줄 `4: PASS`, `Test #4: studio_reflines ... Passed 10.15 sec` |
| 10 파이썬 SVG | 명령 4(기준 10 블록) | 0 | 통과 | `PY-SVG OK` |
| 11 파이썬 DXF | 명령 4(기준 11 블록) | 0 | 통과 | `PY-DXF OK` |
| 12 탭·③ 잠김 | 명령 2 | 0 | 통과 | test_cutpage.cpp:206(JOB-006 `[8]` 탭 글자 `… ② 막으로 자르기 · ③ 도면 편집 · 내보내기`), :219-233(③ locked·툴팁 `바닥 확정 후 사용`·`Faint`·누르면 페이지 2·`먼저 ① 바닥 정하기에서`, 확정 뒤 풀림) |
| 13 ② 고정·넘기기 | 명령 2 | 0 | 통과 | :409-437(fixText 3·slotMemo·kStatus, `widget(3)` 그룹 5개·마지막 `고정 · 넘기기`, 타일 글자·Idle/Primary·높이 = 홈 cut, Primary = `[toEdit]`, 페이지 높이 ≤ ①, StFix `막 고정 전`, StHint, ctx `지금 각도 고정` 보임). `[J7-13] window minimumSizeHint width=1791`(≤ 1845) |
| 14 고정·풀림 | 명령 2 | 0 | 통과 | :439-454(Enter → `막 105° 고정`·On·슬롯 표시, → → 풀림·Idle·슬롯 `막 105°` 만, Ctrl+Z 제목 105°·풀림 유지, 타일 → 다시 고정) |
| 15 ③ 화면 | 명령 2 | 0 | 통과 | :456-485(페이지 4, ToolBadge·CtxHint·툴팁·StHint·StFix·View3DTitle·SheetTitle·CtxRight, ctx 숨김/보임, 3D 툴팁 없음, 3D 폭 330, 그룹 5개 순서, FixedAngle·backToCut 글자, Primary = `[export]`, 높이). `[J7-15] window minimumSizeHint width=1791`(≤ 1845) |
| 16 ③ 에서 막 안 돎 | 명령 2 | 0 | 통과 | :487-500(→ . ] 뒤 StFix·CtxRight 그대로, 왼쪽 끌기 뒤 3D 그림 바뀜·StFix 그대로) |
| 17 다시 자르기 | 명령 2 | 0 | 통과 | :502-513(backToCut → 3·배지·제목·StFix·3D 폭 > 330, ③ 탭 → 4, Esc → 1·CtxBar·StFix 숨김) |
| 18 자동 고정·슬롯 기억 | 명령 2 | 0 | 통과 | :516-546(104° 자동 고정, B–B′ ↔ A–A′, ② 가로 슬롯에서 Enter 103° → ③ A–A′ 103° 유지, S → `A–A′ 실시간 단면 — 103°`) |
| 19 새 문서 = 풀림 | 명령 2 | 0 | 통과 | :548-555(`막 0° 고정` → 다시 열면 `막 고정 전`) |
| 20 JOB-006 [15] 갱신·끝 | 명령 2 | 0 | 통과 | :381-382 `tb[5]->click()` → `[15] 내보내기 → CtxBar 숨김`. 마지막 줄 `6: PASS`, `Test #6: studio_cutpage ... Passed 25.39 sec`. JOB-006 `[9]`·`[11]`·`[13]` 인쇄도 그대로(`단면 위치 x=919`) |
| 21 CTest 17종 | 명령 1(`L\03_ctest.log`), 명령 3 | 0 / 0 | 통과 | 두 로그 모두 37행이 `100% tests passed out of 17`. `tests failed` 가 든 줄은 없다(grep 종료 1) |
| 22 계산 값 그대로 | `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180"` · `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"` · `grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"`, 명령 1 직후 `B\Testing\Temporary\LastTest.log` | 0 | 통과 | `1`·`1`·`1`. `LastTest_rerun.log`(명령 3 뒤)도 `1`·`1`·`1`, 기준인 `A6/LastTest_verify.log` 도 `1`·`1`·`1` |
| 23 verify 종료 0·PASS·경고 0 | 명령 1 / `tail -1 B\verify-summary.md` / `grep -ci "warning" D:/_dongarch_build/verify_job007/studio-logs/02_build.log` | 0 | 통과 | 종료 0 / `결과: PASS` / `0` |
| 24 엔진·기존 시험 무변경 | `git diff --name-only 688b7bd -- studio/src/ArtifactDoc.h … studio/tests/test_cut.cpp GigaMesh`(13개 경로, 명세 그대로) | 0 | 통과 | 출력 없음 |
| 25 새 문자열 있음 | `for s in "③ 도면 편집" … "기준선: 중심선 01"; do grep -rqF "$s" studio/src \|\| echo "MISSING $s"; done`(21개) | 0 | 통과 | 출력 없음 |
| 26 옛 것 없음 | `grep -rnF "측정·탁본" studio/src` / `grep -rnF "외곽선·중심선" studio/src` / `grep -rnF "6 1 1 1" studio/src` / `grep -cF '다음: 「도면」 내보내기 (Ctrl+P)"' studio/src/CutPageText.h` | 1 / 1 / 1 / 0 | 통과 | 출력 없음 · 없음 · 없음 · `1` |
| 27 R2·R3 | `git diff 688b7bd -- studio/src \| grep -E '^\+.*(QColor\([0-9]\|qRgba?\([0-9]\|Qt::(red\|…\|lightGray)\b)'` / `grep -rhoE '#[0-9A-Fa-f]{6}' studio/src \| wc -l` / 기준값 `git grep -h -o -E '#[0-9A-Fa-f]{6}' 688b7bd -- studio/src \| wc -l` / `comm -13 <(…688b7bd… \| sort -u) <(… \| sort -u)` / `grep -rniE "c8641e\|3f8f4f\|3f6fb5\|6b4fc0" studio/src` / `git diff 688b7bd -- studio/src \| grep '^\+.*ToolState::Primary' \| grep -vc toEdit` / `grep -c '"toEdit", ToolState::Primary' studio/src/MainWindow.cpp` | 1 / 0 / 0 / 0 / 1 / 1 / 0 | 통과 | 출력 없음 · `19`(≤ 19) · `21` · 출력 없음 · 출력 없음 · `0` · `1`. 더한 Primary 줄은 `ribbon_->setState("toEdit", ToolState::Primary);` 하나다 |
| 28 흐름 코드 | `grep -c` 8개(명세 그대로) | 0(`CutPageText.h` 는 grep 1) | 통과 | `plateRefLines(` 4(≥4) · `azimuthNote(` 3(≥3) · `CutPageText.h` 0(=0) · `paperPxPerMm` 1(≥1) · `slotMemo(` 2(≥1) · `firstVerticalSlot()` 4(≥2) · `"--edit"` 1(≥1) · `setEditPage` 1(=1) |
| 29 정본 | `docs/ui/UI_RULES.md` 에 `grep -c` 8개 + `grep -cF` 1개(명세 그대로) | 0(`측정·탁본` 은 grep 1) | 통과 | `1`·`0`·`1`·`1`·`1`·`1`·`1`·`1`·`1`(기대 =1·=0·≥1×7) |
| 30 변경 범위 | `git status --porcelain` / `git diff --name-only 688b7bd HEAD \| grep -vxF -e studio/src/CutPageText.h … -e docs/ui/UI_RULES.md`(15개) | 0 / 1(일치 없음) | 통과 | 둘 다 출력 없음. 바뀐 파일은 허용 목록 15개뿐이다(`15 files changed, 977 insertions(+), 94 deletions(-)`) |
| 31 `shot_home` 해시 | 명령 7 | 0 | **통과** | 출력 `True`. 13,246 B · 1845×920 · SHA256 `00FE52AEFB6E790E…`(JOB-006 과 같음) |
| 32 `shot_main` vs `A6/shot_main` | 명령 1 `shots`(종료 0) + 눈 확인 + 화소 비교(명령 8) | 0 | **실패(관찰)**. 최종 예/아니오는 ui-reviewer | 368,572 B(> 5120), 두 장 모두 1845×920. **같은 곳**: 리본 y 34-140 은 다른 화소 0개, 슬롯 목록(x 0-175)·상태줄(y ≥ 890) 도 0개다. 가로 경계 y 33·140·200·890 이 두 장에서 같다. **(a)** 탭 줄 y 11-19, x 471-574 만 다르다 → 충족. **(b)·(d)** 도판을 +90 px 옮겨 대면 다른 화소가 6,332개이고 모두 다음 자리에 있다. y 274-302(머리표: 갈색 파선이 구연선 바깥 끝 앞에서 멈춤, 라벨 상자가 구연선 위로, 구연선이 외면·단면 양쪽을 가로지름, 옛 닫는 가로선 y 302 없어짐), y 816-817(옛 아래 닫는 선 없어짐), y 826-827(도판 전체 폭 먹선 → 저경 폭 바닥선), y 835-838(중심선 아래 연장). 확대해 보면 외곽선 위·아래 끝과 구연선·바닥선 사이에 틈이 있다 → 충족. **(c) 불충족**: 「그리는 순서」 라벨 폭이 453 → 273 px 로 줄었다. 새 글은 47자인데 □ 23칸만 보인다. JOB-006 은 38칸(38자 전부)이 보였다. 글이 잘려 오히려 짧게 보인다. **명세 밖의 차이**: 3D 보조창 271 → 451 px, 3D 모형이 크게 그려짐, 도판 +90 px, 도판 머리줄(y 141-172)과 치수 띠 `dims_`(y 180-188) 위치가 +180 px. 명세는 「작업 공간·3D 같음」 → 불충족. `dims_` 폭은 910 px 로 같다(오프스크린 □ 라 값은 읽지 못함. 실제 글꼴 캡처 33 에서 `기고 184.8 · 구경 142.0 · 최대복경 165.5 · 저경 142.0 · 평균 기벽두께 6.8`) |
| 33 `shot_cut_font.png` | 명령 5 + 눈 확인 + 화소 분석 | 0 | **충족(관찰)**. 최종 예/아니오는 ui-reviewer | 434,828 B · 1845×920. 리본 그룹 5개: `파일`·`막 방향 · 지금 12° (누르면 맞춤)`·`막 움직이기 (회전판)`·`단면 위치`·`고정 · 넘기기`. 마지막 그룹에 `지금 각도 고정`(자물쇠, 옅은 대기 타일)과 `도면 편집으로`(→, 빨강)가 있다. 주 단추 빨강 화소 띠는 화면 전체에서 x 1351-1398·y 59-105 **하나**뿐이다. 가로 경계 y 33·140·176·236·890 이 `A6/shot_cut.png` 와 같다 → 리본 높이 같음. ctx: `Enter` 배지 + `지금 각도 고정`. 상태줄 왼쪽은 `⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 「지금 각도 고정」 → 「도면 편집으로」`, 오른쪽 끝은 `막 고정 전`. 도판의 기준선 셋과 머리표는 32 (b)·(d) 와 같은 모양이다(확대 확인). 치수 띠 오른쪽에 `그리는 순서: 방안눈금 → 탁본음영 → 기준선(먹) → 외곽선(먹) → 단면선(빨강)` 전체가 보인다. 머리는 `A–A′ 실시간 단면 — 12°`. 단면 빨강 모양이 `A6/shot_cut.png` 와 IoU 0.9985 로 같다 |
| 34 `shot_edit_font.png` | 명령 5 + 눈 확인 + 화소 분석 | 0 | **충족(관찰)**. `✎` 는 폰트 미탐색. 최종 예/아니오는 ui-reviewer | 407,040 B · 1845×920. 현재 탭은 5번째 `③ 도면 편집`(밑줄). 리본 그룹 5개: `파일`·`고정된 막`·`선 다듬기 · 간략화`·`도판 표시 (겹침)`·`내보내기`. 주 단추 빨강은 `도면`(x 1263-1310) 하나다. 「고정된 막」 그룹: 위 `막 12° 고정`, 아래 [`↩ 3D로 돌아가 다시 자르기` `S`](`↩` 는 대체 글리프로 보임). ctx 왼쪽 어두운 배지는 **`□ 도면 편집`**(`✎` U+270E 가 □. 글자는 기준 15 `ToolBadge == kEditBadge`(`✎ 도면 편집`) 가 확인) + `단면이 기준선 위에 놓였습니다 · 중심선 = 3D 중심축 · 바닥선 = 바닥판 · 구연선 = …`(오른쪽이 잘림). 3D 보조창 폭은 **330 px**(33 은 451 px). 3D 머리 `3D — 고정된 막`. 도판 머리 `A–A′ 정면 실측도`. 치수 띠 오른쪽에 `그리는 순서: … → 기준선(먹) → …` 전체가 보인다. 상태줄은 `⑧ 치수 확인 — 치수 띠를 보고 다음: 「도면」 내보내기 (Ctrl+P)`, 오른쪽은 `막 12° 고정`. 슬롯 목록 A–A′ 메모는 `막 12° 고정`. 단면 빨강 화소(33: 1,347 · 34: 1,346)는 모두 서로 1 px 안에 있고, 행별 좌우 끝 차이는 0-1 px 다 → 같은 12° 모양 |
| 35 `shot_export_font.png` | 명령 5 + 눈 확인 | 0 | **충족(관찰)**. 방위각 글은 판독 어려움 → 기준 7 로 갈음. 최종 예/아니오는 ui-reviewer | 116,051 B · 940×680. 오른쪽 칸에 `외곽선·기준선 Layer (먹색 · 외곽선 0.50 · 기준선 0.10~0.25 mm)` 가 보인다. 미리보기 도판에 구연선(외면·단면 가로지름)·저경 폭 바닥선·짧은 중심선 연장이 보인다(4배 확대 확인). 표제란 첫 줄은 글자가 너무 작다. 끝이 `… · 정면 방위각 0° (위에서 보아 반시계)` 길이와 맞아 보이지만 글자는 판독할 수 없다 → 명세대로 기준 7(통과)로 갈음 |

## 주요 출력 원문

`L\03_ctest.log`(명령 1). 명령 3 재실행(`artifacts\ctest_all_rerun.log`)도 17개가 모두 `Passed` 이고 `100% tests passed out of 17`, `Total Test time (real) = 129.16 sec` 이다.
```
 1/17 Test  #1: studio_pipeline ..................   Passed    7.46 sec
 2/17 Test  #2: studio_pose ......................   Passed    8.88 sec
 3/17 Test  #3: studio_side ......................   Passed   11.49 sec
 4/17 Test  #4: studio_reflines ..................   Passed   10.22 sec
 5/17 Test  #5: studio_cut .......................   Passed   65.28 sec
 6/17 Test  #6: studio_cutpage ...................   Passed   25.45 sec
 7/17 Test  #7: karch_test_taubin_annulus ........   Passed    0.03 sec
 ...
17/17 Test #17: karch_test_synthetic .............   Passed    0.19 sec

100% tests passed out of 17

Total Test time (real) = 130.91 sec
```

`ctest -R "studio_reflines|studio_cutpage" -V`(명령 2, `artifacts\ctest_reflines_cutpage_V.log`). `[J7-1]`(사람 검토 6 자료)·`[J7-3]`·`[J7-13]`·`[J7-15]` 인쇄를 그대로 옮긴다. JOB-006 의 `[9]`·`[11]`·`[13]` 줄은 따로 적었다.
```
4: Test command: D:\_dongarch_build\verify_job007\studio\test_studio_reflines.exe "D:/_dongarch_build/verify_job007/studio"
4: [J7-1] sample H=184.8206 rim=142.00 base=142.00 outline ends=(-71.700,3.362)…(-71.697,181.612) gap low 3.362 high 3.208 mm
4: [J7-2] Left == Right: 1
4: [J7-6] svg layers checked (_s1 · _s3 · _L · _h)
4: [J7-8] dxf 0deg checked
4: [J7-9] raster s=1 1 · s=1/3 1 · labels 1
4: [J7-8] dxf 12deg checked
4: [J7-5] notes: 0° · 12° · −20° · −0.04° → 정면 방위각 0° (위에서 보아 반시계)
4: [J7-3] blind H=190.2556 rim=124.00 base=124.00 rim.a.x=-62.000 base line=1
4: [J7-4] empty/unconfirmed: none
4: PASS
1/2 Test #4: studio_reflines ..................   Passed   10.15 sec
6: Test command: D:\_dongarch_build\verify_job007\studio\test_studio_cutpage.exe "D:/_dongarch_build/verify_job007/studio"
6: [J7-13] window minimumSizeHint width=1791
6: [J7-15] window minimumSizeHint width=1791
6: PASS
2/2 Test #6: studio_cutpage ...................   Passed   25.39 sec
100% tests passed out of 2
Total Test time (real) =  35.55 sec
```
같은 실행의 JOB-006 인쇄(참고, 판정은 JOB-006 몫):
```
6: [9] ribbon height=107 px (JOB-005 계산값 107)
6: [9] ctxHint width=174 / sizeHint=612 px · ctx minimumSizeHint width=1618 px (offscreen 글꼴)
6: [9] 단면 위치 x=919
6: [11] 단면 위치 x=919
6: [13] 단면 위치 x=919
```

파이썬 판정(명령 4):
```
PY-SVG OK
exit=0
PY-DXF OK
exit=0
```

기준 22(`LastTest_verify.log`, 명령 1 직후):
```
11:H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180
14:STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2 unit=m→mm 자동환산(×1000)
15:ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95
```

`B\verify-summary.md`:
```
# verify 요약 — studio (2026-10-18 03:16:50)
| configure | 0 | 4 | D:\_dongarch_build\verify_job007\studio-logs\01_configure.log |
| build | 0 | 23 | D:\_dongarch_build\verify_job007\studio-logs\02_build.log |
| ctest | 0 | 131 | D:\_dongarch_build\verify_job007\studio-logs\03_ctest.log |
| shots | 0 | 9 | D:\_dongarch_build\verify_job007\studio-logs\04_shots.log |
결과: PASS
```

기준 31:
```
True
```

기준 32 화소 비교(명령 8) 요약:
```
diff pixels 554702
row bands [(11, 19), (141, 172), (180, 188), (201, 889)]        # 리본 y 34-140 · 상태줄 y ≥ 890 · 슬롯 목록 x < 176 은 0
3D pane x 176 -> 447 width 271  (JOB-006)   /   x 176 -> 627 width 451  (JOB-007)
plate shift dx 90 diff px 6332  rows [(274,285),(287,302),(816,817),(826,827),(835,838)]
그리는 순서 라벨: JOB-006 x 1377-1830 □ 38칸  /  JOB-007 x 1557-1830 □ 23칸 (새 글 47자)
```

캡처 크기·해시(SHA256 앞 16자리). artifacts 사본은 빌드 폴더 `shots` 원본과 해시가 같다.
| 파일 | JOB-007 | JOB-006(`A6`) |
|---|---|---|
| shot_home.png | 13,246 B · 1845×920 · `00FE52AEFB6E790E` | 같음 |
| shot_main.png | 368,572 B · 1845×920 · `038368B13B67CCC1` | 332,770 B · 1845×920 · `EE170E7AED3F3B6E` |
| shot_export.png | 71,150 B · 1296×680 · `DC0E49C5CC88C4FD` | 70,269 B · 1205×680 · `A51858679DF4B810` (명세 §7: 일부러 다름) |
| shot_cut_font.png | 434,828 B · 1845×920 · `750077A019A4935B` | (비교 대상 `shot_cut.png` 327,857 B · `6757D05F6B496C2D`) |
| shot_edit_font.png | 407,040 B · 1845×920 · `812F68DF417B6F79` | — |
| shot_export_font.png | 116,051 B · 940×680 · `F866DB780BB29CE4` | — |

builder 기록(build-log.md)의 `[J7-1]`·`[J7-3]`·`[J7-13]`·`[J7-15]` 인쇄와 캡처 크기(shot_main 368,572 · 실제 글꼴 434,828 / 407,040 / 116,051)는 이번 빈 빌드 결과와 글자·바이트가 같다.

## 산출물 (artifacts/, 커밋하지 않음)
- verify.ps1 복사본: `01_configure.log`, `02_build.log`, `03_ctest.log`, `04_shots.log`(0 B), `verify-summary-studio.md`, `shot_home.png`, `shot_main.png`, `shot_export.png`.
- 이 기록에서 더한 것: `LastTest_verify.log`(명령 1 직후), `ctest_reflines_cutpage_V.log`(명령 2), `ctest_all_rerun.log`·`LastTest_rerun.log`(명령 3), `shot_cut_font.png`·`shot_edit_font.png`·`shot_export_font.png`(명령 5).
- 시험이 `B` 에 남긴 파이썬 판정 입력: `studio_reflines_s1.svg`·`_s3.svg`·`_L.svg`·`_12.svg`·`_h.svg`, `studio_reflines.dxf`·`_12.dxf`·`_m20.dxf`·`_m004.dxf`(명령 3 재실행 뒤의 것).

## 비고
1. **기준 32 실패 원인과 판단 자료**: `SheetView.cpp` 의 `order_->setMinimumWidth(0)` → `1`(builder 판단 1) 때문에 「그리는 순서」 글이 창 최소 폭과 3D 보조창 폭을 밀지 않게 됐다. 그래서 오프스크린 캡처에서 3D 보조창이 JOB-006 의 271 px 에서 451 px(≈ 27 %)로 넓어졌다. JOB-006 에서는 □ 글꼴 글 폭이 작업 공간을 밀어 3D 보조창이 좁았다. 대신 「그리는 순서」 글이 1845 px 창에서 잘린다(실제 글꼴 캡처 33·34 에서는 전부 보임). builder 의 007a 커밋(6542878)은 최소 폭 = 옛 글 폭이라 shot_main 배치가 JOB-006 과 같았다. 007b 에서 기준 13·15(창 최소 폭 ≤ 1845)와 17(② 3D > 330)을 세우려고 바꿨다(build-log.md). 명세 32 의 「작업 공간·3D 같음」 과 (c) 「길어짐」 은 이 변경을 반영하지 못한다 → ui-reviewer·코디네이터가 판단할 일이다.
2. 34 의 `✎`(U+270E) 는 `QT_QPA_FONTDIR=C:\Windows\Fonts` 실제 글꼴 캡처에서도 □ 다(`✂` 는 보임). builder 도 같은 관찰을 적었다. 실제 Windows 화면 확인은 사람 검토 11 몫이다. 33·34 의 `구경 ⌀` 의 `⌀` 와 `최근 ▾` 류 기호가 □ 인 것은 JOB-006 캡처와 같은 기존 모양이다.
3. 오프스크린 ② ctx 안내(ctxHint) 폭이 372 → 174 px 로 줄었다(JOB-006 `[9]` 인쇄, ctx 에 `Enter 지금 각도 고정` 이 늘어서). 툴팁에 전체 글이 있다. JOB-006 판정 `[9]` 은 통과다.
4. 검증 중 제품·시험 코드는 고치지 않았다. 검증 대상 worktree 의 `git status --porcelain` 은 끝에도 빈 출력이다. 쓴 파일은 이 `verify.md` 와 artifacts 사본뿐이다.

## 추기 (오케스트레이터) — 기준 32 수정 판정: 실제 글꼴 캡처 비교 → **통과**
- 명세 기준 32 를 실제 글꼴 캡처로 판정하도록 고쳤다(decision.md). 같은 방법(`QT_QPA_PLATFORM=offscreen`, `QT_QPA_FONTDIR=C:\Windows\Fonts`, `--sample --shot`, 창 1845×920)으로 JOB-006 실행 파일(`verify_job006`)과 JOB-007 실행 파일(`verify_job007`)을 찍었다(종료 코드 0). 산출물: `artifacts/job006_shot_main_font.png`, `artifacts/shot_main_font.png`, `artifacts/shot_home_font.png`.
- PIL 화소 비교(문턱 8) 결과 다른 띠: y 12-25 (x 375-528, 탭 글자 `측정·탁본` → `③ 도면 편집`) · y 182-193 (x 1411-1669, 「그리는 순서」 글) · y 272-302 (x 983-1439, 기고 H 머리표·구연선) · y 816-838 (x 973-1498, 바닥선). 그 밖(리본·3D 보조창·슬롯 목록·상태줄·단면) 차이 0. 3D 보조창 세로 경계 위치가 두 캡처에서 같다.
- 결론: 기준 32 (a)(b)(c)(d) 만 다르다 → **통과**. 1차 FAIL 의 3D 폭 변화(271 → 451 px)는 □ 글꼴에서만 생긴다. 이로써 수용 기준 35개 모두 통과.

