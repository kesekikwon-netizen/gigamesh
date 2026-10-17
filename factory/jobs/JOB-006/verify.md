# JOB-006 검증 기록 (test-runner) — 재작업 1회차 검증

> **재작업 1회차 검증이다.** 1차 검증(HEAD `eee6dd8`, 00:07-00:21)은 FAIL(기준 34 명령 판정, 33 관찰 판정)이었다. builder 가 재작업 커밋 `688b7bd`(`studio/src/main.cpp` 만 +6/−1: `--shot` 이면 창을 1845×920 으로 고정, 찍기 직전 밀린 `LayoutRequest` 처리)를 올렸고, 이 기록은 그 HEAD 를 빈 빌드 폴더에서 처음부터 다시 검증한 결과로 1차 기록을 통째로 대체한다.

결론: **PASS** — 수용 기준 34개 모두 통과.
- 명령으로 판정하는 기준 1-30·34 는 모두 기대값과 같다. 1차에서 실패한 **34 는 이번에 `True` 두 줄**이다.
- 31-33 은 명세상 ui-reviewer 가 예/아니오를 정한다. 이 기록에서는 배치를 화소로 확인했고 모두 충족이다. 오프스크린 캡처는 한글·숫자가 모두 □ 라서 글자 내용은 읽지 못했다(폰트 미탐색). 그래서 글자 칸 수로 맞는지 확인했다.
- 1차 비고 3(`shot_cut` 이 끄는 중 상태에서 찍혔을 가능성)은 해소됐다. ctx 오른쪽 글이 놓은 뒤 문형 길이인 **47칸**으로 찍혔다(1차 44칸).

## 환경
| 항목 | 값 |
|---|---|
| 검증 대상 | `D:\_dongarch_build\wt\.worktrees\JOB-006`, 브랜치 `factory/JOB-006`, HEAD `688b7bd`(기준 `804f60d`). `git status --porcelain` 은 시작·끝 모두 빈 출력. `eee6dd8..688b7bd` = `studio/src/main.cpp` 1개 파일(+6 −1) |
| OS | Windows 11 Pro 10.0.22631 |
| 컴파일러 | MSVC 19.44.35228.0 (`C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe`), C++20(`CMAKE_CXX_STANDARD 20`), Release |
| 생성기 | Ninja(VS BuildTools 동봉 `…/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe`), CMake 4.4.1 |
| Qt | `D:/OSGeo4W/apps/Qt6` (`Qt6_DIR=D:/OSGeo4W/apps/Qt6/lib/cmake/Qt6`). 시험·캡처는 `QT_QPA_PLATFORM=offscreen` |
| 빌드 폴더 `B` | `D:\_dongarch_build\verify_job006\studio`. `-Clean` 으로 지운 뒤 72단계 전체 빌드(`[72/72] Linking CXX executable test_studio_cutpage.exe`) |
| 로그 폴더 `L` | `D:\_dongarch_build\verify_job006\studio-logs` |
| 산출물 | `D:\_dongarch_build\wt\factory\jobs\JOB-006\artifacts\`(`-RecordRoot D:\_dongarch_build\wt`). 검증 대상 worktree 는 읽기만 했다 |

## 시간 (2026-10-18)
| 단계 | 시각 | 소요 |
|---|---|---|
| verify.ps1 `-Clean` 전체 | 00:39:37 → 00:42:05 | 148 s (configure 4 · build 23 · ctest 110 · shots 9) |
| `ctest -R studio_cutpage -V` | 00:42:25 | 15 s (시험 15.28 s) |
| `ctest --output-on-failure` 재실행 | 00:42:46 → 00:44:35 | 109 s (`Total Test time (real) = 109.34 sec`) |
| 검증 명령 3(캡처 2장) | 00:44:54 | `--cut` 4,080 ms · `--base` 3,052 ms |

## 경고 수
- 빌드 경고 **0**. `grep -ci "warning" D:/_dongarch_build/verify_job006/studio-logs/02_build.log` 의 출력이 `0` 이다. `C[0-9]{4}` 진단 코드도 0, `01_configure.log` 에서 `error|warn` 도 0.
- 콘솔에 `'vswhere.exe' is not recognized as an internal or external command` 가 2번 찍혔다(configure·build 시작 때). vcvars64 환경 메시지라서 로그와 종료 코드에는 영향이 없다(1차·JOB-005 와 같음).

## 실행한 명령
1. `powershell -NoProfile -ExecutionPolicy Bypass -File D:\_dongarch_build\wt\.worktrees\JOB-006\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-006 -BuildRoot D:\_dongarch_build\verify_job006 -Job JOB-006 -RecordRoot D:\_dongarch_build\wt -Clean` → **종료 코드 0**
2. 곧바로 `B\Testing\Temporary\LastTest.log` 를 `artifacts\LastTest_verify.log` 로 복사했고, 기준 19·20·21·34 를 판정했다.
3. (PowerShell) `Remove-Item Env:\CI -ErrorAction SilentlyContinue; $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job006\studio; ctest -R studio_cutpage -V` → **0** (`artifacts\ctest_studio_cutpage_V.log`)
4. 같은 환경·같은 폴더에서 `ctest --output-on-failure` → **0** (`artifacts\ctest_all_rerun.log`). 그 뒤 LastTest.log 를 `artifacts\LastTest_rerun.log` 로 복사했다.
5. 검증 명령 3(명세 §5-3): `--sample --cut --shot …\shots\shot_cut.png`, `--sample --base --shot …\shots\shot_base.png`, 크기 출력, artifacts 로 복사. 명세 명령과 다른 점은 종료 코드를 얻으려고 `Start-Process` 에 `-PassThru` 를 더한 것 하나다 → 두 실행 모두 **ExitCode 0**, 크기 `327857`·`70058`
6. 기준 22-30: Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-006` 한 뒤 명세 §4 C 의 명령을 글자 그대로 실행했다.
7. 캡처 판정(31-33): 이미지를 눈으로 보고, 세션 scratchpad 의 Python(`Python312\python.exe -I`, PIL·numpy) 스크립트로 화소를 분석했다. 대상은 가로 경계 y, 밑줄 위치, 글자 칸 수, 회색 최솟값, 빨강 화소 좌우 분포, JOB-005 와의 화소 차이다.

## 수용 기준별 결과
기준 1-18 은 `test_studio_cutpage` 한 번 실행 안의 CHECK 들이다. 실패한 CHECK 는 `FAIL <식>: …` 를 찍고, 그러면 마지막 줄이 `FAILED` 가 된다. 이번 출력에는 `FAIL` 줄이 없고 마지막 줄은 `PASS` 다. `studio/tests/test_cutpage.cpp` 는 1차 검증 뒤로 바뀌지 않았다(`eee6dd8..688b7bd` 에 없음). 이번에도 파일을 읽어서 각 CHECK 가 명세 §4 판정식과 같은지 확인했다. 아래 행 번호는 그 파일 기준이다.

| 기준 | 명령 | 종료 코드 | 결과 | 근거 |
|---|---|---|---|---|
| 1 angleText | 명령 3 | 0 | 통과 | 값 9개를 `==` 로 비교(:98-101). FAIL 줄 없음 |
| 2 plateTitle/Sub | 명령 3 | 0 | 통과 | :103-105 |
| 3 atAxis·axisGroupName | 명령 3 | 0 | 통과 | :107-115 |
| 4 faceText·ctxRight | 명령 3 | 0 | 통과 | :117-123 |
| 5 pageChanged·currentPage | 명령 3 | 0 | 통과 | :128-139 |
| 6 setPageLocked | 명령 3 | 0 | 통과 | :140-143 |
| 7 setTitle·button·rows·state | 명령 3 | 0 | 통과 | :145-156 |
| 8 탭 순서·잠김(Faint) | 명령 3 | 0 | 통과 | :181-198 |
| 9 확정 → ②·라벨·리본 높이·고정 폭 | 명령 3 | 0 | 통과 | :201-253. `[9]` 인쇄 세 줄은 아래 원문 |
| 10 X 막·진행 중 제스처 취소 | 명령 3 | 0 | 통과 | :256-268 |
| 11 [+] 두 번·끄는 중·리본 고정 | 명령 3 | 0 | 통과 | :271-298. `[11] 단면 위치 x=919` 가 `[9]` 값과 같다 |
| 12 [−] | 명령 3 | 0 | 통과 | :301-304 |
| 13 키·오른쪽 끌기 | 명령 3 | 0 | 통과 | :307-325. `[13] 단면 위치 x=919` 가 `[9]` 값과 같다 |
| 14 R 반전 | 명령 3 | 0 | 통과 | :328-335 |
| 15 페이지 = 도구·ctx 겹침 숨김 | 명령 3 | 0 | 통과 | :338-359 |
| 16 옛 단추 없음 | 명령 3 | 0 | 통과 | :362-363 |
| 17 가로 슬롯에서 ② 로 | 명령 3 | 0 | 통과 | :366-370 |
| 18 마지막 줄 PASS·종료 0 | 명령 3 | 0 | 통과 | 마지막 줄 `5: PASS`, `1/1 Test #5: studio_cutpage ... Passed 15.28 sec` |
| 19 CTest 16종 | 명령 1(`L\03_ctest.log`), 명령 4 | 0 / 0 | 통과 | 두 로그 모두 35행이 `100% tests passed out of 16`. `tests failed` 가 든 줄은 없다(grep 종료 1) |
| 20 계산 값 그대로 | `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180"`·`grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"`·`grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"` 를 명령 1 직후 `B\Testing\Temporary\LastTest.log` 에 | 0 | 통과 | `1`·`1`·`1`. `LastTest_rerun.log`(명령 4 뒤)도 `1`·`1`·`1`. 기준인 JOB-005 `LastTest_verify.log` 도 `1`·`1`·`1` |
| 21 verify 종료 0·PASS·경고 0 | 명령 1 / `tail -1 B\verify-summary.md` / `grep -ci "warning" …/studio-logs/02_build.log` | 0 | 통과 | 종료 0 / `결과: PASS` / `0` |
| 22 엔진·기존 시험 무변경 | `git diff --name-only 804f60d -- studio/src/ArtifactDoc.h … studio/tests/test_cut.cpp`(13개 경로) | 0 | 통과 | 출력 없음 |
| 23 원문 라벨 있음 | `for s in "② 막으로 자르기" … ". ] 도 같음"; do grep -rqF "$s" studio/src \|\| echo "MISSING $s"; done` | 0 | 통과 | 출력 없음. 23개 모두 있다 |
| 24 옛 용어·단추 없음 | `grep -rnE 'QStringLiteral\("[^"]*(정면 회전\|절단 위치 이동\|절단면 1°\|절단 각도\|단면·외곽선 편집 중\|절단면 A–A′ 각도\|절단면 · 중심축)' studio/src` / `grep -cE '"rotate"\|"cutMove"' studio/src/MainWindow.cpp` | 1 / 1(일치 없음) | 통과 | 출력 없음 / `0` |
| 25 페이지 신호·잠김·그룹 이름 | `grep -c` 6개(명세 그대로) | 0 | 통과 | `1`·`1`·`1`·`1`·`1`·`1`. 기대는 =1·≥1·≥1·≥1·≥1·=1 |
| 26 입력 합치기 불변·흐름 | `grep -c` 5개 + `sed -n '/^void MainWindow::confirmPose/,/^}/p' studio/src/MainWindow.cpp \| grep -c "setCurrentPage(pCut_)"` | 0(`setFrontAngle…` 는 grep 1) | 통과 | `previewFrontAngle` 1 · `setFrontAngle(doc_.frontAngle()` 0 · `cut_.click(` 1 · `alignCut(` 1 · `setOverrideCursor(Qt::WaitCursor)` 7 · confirmPose 안 1 |
| 27 R2·R3 | `git diff 804f60d -- studio/src \| grep -E '^\+.*(QColor\([0-9]\|qRgba?\([0-9]\|#[0-9A-Fa-f]{6}\|Qt::(red\|…\|lightGray)\b)'` / `grep -rniE "c8641e\|3f8f4f\|3f6fb5\|6b4fc0" studio/src` / `git diff 804f60d -- studio/src \| grep -c '^\+.*ToolState::Primary'` / `git diff 804f60d -- studio/src/Theme.h \| grep -c '^+inline const QColor'` | 1 / 1 / 1 / 1(일치 없음) | 통과 | 출력 없음 · 출력 없음 · `0` · `0` |
| 28 ctx 폭·R8·`--cut`·3D 끌기 | `grep -c` 6개 + `grep -rqF "오른쪽 끌기 = 보는 방향만" studio/src && echo OK` | 0(`[ ]` 는 grep 1) | 통과 | `ctxHint_->setMinimumWidth(1)` 1 · `kbdLabel("[ ]"` 0 · `kbdLabel("Shift"` 2 · `kbdLabel("← →"` 2 · `"--cut"` 1 · `Qt::LeftButton` 1 · `OK` |
| 29 정본 | `docs/ui/UI_RULES.md` 에 `grep -c` 3개 + `grep -cF` 2개 | 0(마지막은 grep 1) | 통과 | `1`·`1`·`1`·`1`·`0` |
| 30 변경 범위 | `git status --porcelain` / `git diff --name-only 804f60d HEAD \| grep -vxF -e studio/src/CutPageText.h … -e docs/ui/UI_RULES.md`(14개) | 0 / 1(일치 없음) | 통과 | 둘 다 출력 없음. 바뀐 파일은 허용 목록 14개뿐이다(`14 files changed, 714 insertions(+), 57 deletions(-)`). 재작업 파일 `studio/src/main.cpp` 도 허용 목록에 있다 |
| 31 `shot_cut.png` | 명령 5 + 눈 확인 + 화소 분석 | 0 | **통과(배치)**. 글자는 폰트 미탐색. 최종 예/아니오는 ui-reviewer | 327,857 B(> 5120), 1845×920. **탭**: 밑줄이 y=32, x 245-389(4번째)에 있고, 4번째 탭 글자 회색 최솟값이 19(현재·`Ink`), 다른 탭은 60. **리본 그룹 4개**(그룹 이름 칸 수: 파일 2 · 막 방향 22 · 막 움직이기 12 · 단면 위치 5, 구분선 x 146·467·924). 파일 타일 2. 막 방향 타일 3개이고 글자는 한 줄이다. 셋째(axZ) 글자·아이콘 회색 최솟값은 112(흐림)이고 axX·axY 는 19. 막 움직이기 세 줄: 줄 1 16칸(`☑ 중심선에 고정 (돌리기만)` 16자), 줄 2 = `각도`(2칸) · [−] 단추 · 3칸(`12°`) · [+] 단추 · kbd 배지(54 px) · 4칸(`1° ·`) · kbd 배지(76 px) · 3칸(`15°`), 줄 3 9칸(`끄는 동안 실시간` 9자). 단면 위치: 단추+`R` 배지와 두 줄 글(둘째 줄 초록). **리본 높이**: 가로 경계 y=34·140 이 shot_main 과 같다. **ctx 줄** y 141-176 이 보이고, 왼쪽 끝 배지(x 12-151)는 어둡다(최솟값 47, 평균 65). **`12°`**: 글자는 □ 라서 읽을 수 없다. 칸 수는 맞는다. 그룹 이름 22칸 = `막 방향 · 지금 12° (누르면 맞춤)` 22자(맞춤 문형이면 16자). 각도 칸 3칸 = `12°`. 도판 머리 17칸 = `A–A′ 실시간 단면 — 12°` 17자. ctx 오른쪽 47칸 = `막 12° · 끄는 동안 2.6만 면 미리보기 · 놓으면 원본 7.7만 면 다시 계산` 47자(끄는 중 문형이면 44자). **도판**: 빨간 화소가 중심축 x=1146 왼쪽에 0개, 오른쪽에 1,149개. 먹 외곽(회색 외면)은 왼쪽이다. **3D**: 길게 이어진 빨간 실선 열이 x 373·386 에 있다(축 x≈311 의 오른쪽). 왼쪽은 막 테두리 파선뿐이다 |
| 32 `shot_base.png` | 명령 5 + 눈 확인 + 화소 분석 | 0 | **통과(배치)**. 최종 예/아니오는 ui-reviewer | 70,058 B(> 5120), 1845×920. 밑줄이 y=32, x 111-242(3번째 ① 바닥 정하기)에 있다. 탭 글자 회색 최솟값: 3번째 19(현재), **4번째 112(옅음, `Faint`)**, 나머지 60. 명령 판정은 기준 8 이 통과했다. ctx 줄 왼쪽 배지는 어둡다(최솟값 47, 평균 65) |
| 33 `shot_main.png` 와 JOB-005 비교 | verify `shots`(종료 0) + 눈 확인 + 화소 비교 | 0 | **통과(관찰)**. 최종 예/아니오는 ui-reviewer | 332,770 B(> 5120). 두 장 모두 1845×920 이다(1차의 창 폭 차이 1845 → 1814 는 해소). 다른 화소 36,930개가 **두 띠에만** 있다. ① y 11-19, x 324-548(탭 줄: (a) 탭 1개 추가). x<259 와 x≥1600 은 같다. ② y 40-134, x 243-1822(리본). 리본 그룹 구분선이 JOB-005 x 146·432·804·1109·1588 에서 JOB-006 146·410·748·1053·1532 로 바뀌었다. 첫 칸(x 0-242)은 다른 화소 0개다. `실측선 추출` 오른쪽 그룹들(JOB-005 x 800-1844)은 56 px 옮겨 대면 다른 화소 0개다. 바뀐 것은 `유물 정위치 · 중심축`·`실측선 추출` 두 그룹(작은 단추 칸 3 → 2)뿐이다 → (b). **y ≥ 141(도구 줄·작업 공간·도판·치수 띠·슬롯 목록·상태줄)은 다른 화소 0개**다. 가로 경계 y=33·34·59·140·141·173·200·201·890·891 도 두 장이 같다. (c) 3D 머리 부제 `절단면 · 중심축` → `막 · 중심축` 은 화소 차이가 없다. 이 라벨은 두 장 모두 13칸 뒤에서 잘리고, 오프스크린에서는 모든 글자가 같은 □ 로 그려지기 때문이다(View3D.cpp:224·276 의 문자열 변경은 기준 24 로 확인) |
| 34 `shot_home`·`shot_export` 해시 | `foreach ($n in 'shot_home.png','shot_export.png') { (Get-FileHash "D:\_dongarch_build\verify_job006\studio\shots\$n").Hash -eq (Get-FileHash "D:\_dongarch_build\wt\factory\jobs\JOB-005\artifacts\$n").Hash }` | 0 | **통과** | 출력 `True`·`True`. shot_home 13,246 B, SHA256 `00FE52AEFB6E790E…`(JOB-005 와 같음). shot_export 70,269 B, `A51858679DF4B810…`(같음) |

## 주요 출력 원문

`L\03_ctest.log`(명령 1) 발췌. 명령 4 재실행(`artifacts\ctest_all_rerun.log`)도 16개가 모두 `Passed` 이고 `100% tests passed out of 16`, `Total Test time (real) = 109.34 sec` 이다.
```
 1/16 Test  #1: studio_pipeline ..................   Passed    7.37 sec
 2/16 Test  #2: studio_pose ......................   Passed    8.90 sec
 3/16 Test  #3: studio_side ......................   Passed   11.82 sec
 4/16 Test  #4: studio_cut .......................   Passed   64.67 sec
 5/16 Test  #5: studio_cutpage ...................   Passed   15.29 sec
...
16/16 Test #16: karch_test_synthetic .............   Passed    0.20 sec

100% tests passed out of 16

Total Test time (real) = 110.17 sec
```

`ctest -R studio_cutpage -V`(명령 3, `artifacts\ctest_studio_cutpage_V.log`). 사람 검토 7 자료인 `[9]`·`[11]`·`[13]` 를 그대로 옮긴다.
```
5: Test command: D:\_dongarch_build\verify_job006\studio\test_studio_cutpage.exe "D:/_dongarch_build/verify_job006/studio"
5: Working Directory: D:/_dongarch_build/verify_job006/studio
5: [9] ribbon height=107 px (JOB-005 계산값 107)
5: [9] ctxHint width=372 / sizeHint=612 px · ctx minimumSizeHint width=1420 px (offscreen 글꼴)
5: [9] 단면 위치 x=919
5: [11] 단면 위치 x=919
5: [13] 단면 위치 x=919
5: PASS
1/1 Test #5: studio_cutpage ...................   Passed   15.28 sec
100% tests passed out of 1
Total Test time (real) =  15.29 sec
```

기준 20(`LastTest_verify.log`, 명령 1 직후):
```
11:H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180
14:STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2 unit=m→mm 자동환산(×1000)
15:ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95
```

`B\verify-summary.md`:
```
# verify 요약 — studio (2026-10-18 00:42:05)
| configure | 0 | 4 | D:\_dongarch_build\verify_job006\studio-logs\01_configure.log |
| build | 0 | 23 | D:\_dongarch_build\verify_job006\studio-logs\02_build.log |
| ctest | 0 | 110 | D:\_dongarch_build\verify_job006\studio-logs\03_ctest.log |
| shots | 0 | 9 | D:\_dongarch_build\verify_job006\studio-logs\04_shots.log |
결과: PASS
```

기준 34:
```
True
True
```

캡처 크기·해시(SHA256 앞 16자리):
| 파일 | JOB-006 재작업 1 | JOB-005 | 1차 검증(`eee6dd8`) |
|---|---|---|---|
| shot_home.png | 13,246 B · 1845×920 · `00FE52AEFB6E790E` | 13,246 B · 1845×920 · `00FE52AEFB6E790E` | 13,117 B · 1813×920 |
| shot_export.png | 70,269 B · 1205×680 · `A51858679DF4B810` | 같음 | 같음 |
| shot_main.png | 332,770 B · 1845×920 · `EE170E7AED3F3B6E` | 333,540 B · 1845×920 · `32C4D7B9DCCA3048` | 325,851 B · 1814×920 |
| shot_cut.png | 327,857 B · 1845×920 · `6757D05F6B496C2D` | — | 321,062 B |
| shot_base.png | 70,058 B · 1845×920 · `E069A5DECE844E91` | — | 68,502 B |

## 산출물 (artifacts/, 커밋하지 않음)
- 이번 검증에서 덮어쓴 것: verify.ps1 복사본 `01_configure.log`, `02_build.log`, `03_ctest.log`, `04_shots.log`(0 B), `verify-summary-studio.md`, `shot_home.png`, `shot_main.png`, `shot_export.png`. 그리고 `LastTest_verify.log`(명령 1 직후), `ctest_studio_cutpage_V.log`(명령 3), `ctest_all_rerun.log`·`LastTest_rerun.log`(명령 4), `shot_cut.png`·`shot_base.png`(명령 5). artifacts 사본은 빌드 폴더 `shots` 원본과 MD5 가 같다.
- 1차 검증에서 남은 것: `job005exe_shot_home_rerun.png`. JOB-005 실행 파일로 다시 찍은 시작 화면이고, 1차 비고 1 의 근거다. 이번 판정에는 쓰지 않았다.

## 비고
1. **재작업으로 1차 실패 두 건이 해소됐다.** `--shot` 창 크기를 1845×920 으로 고정하자 시작 화면·작업 화면 캡처의 창 폭이 JOB-005 와 같아졌다. 그 결과 shot_home·shot_export 해시가 같아졌고(34), shot_main 은 탭 줄·리본 두 띠만 다르다(33). 이 변경은 `--shot` 에만 걸리므로 실제 앱 창은 그대로다. 대신 홈 리본이 좁아져 실제 창 최소 폭이 31 px 줄어든 것은 그대로 남아 있다(1차 비고 1, 제품 동작 — 판정 대상 아님).
2. **`shot_cut` 은 확정(놓은 뒤) 상태로 찍혔다.** ctx 오른쪽이 47칸으로, 놓은 뒤 문형 글자 수와 같다. 끄는 중 문형은 44자라서 1차와 구분된다. 재작업 수정 2(찍기 전 `LayoutRequest` 처리)의 효과로 본다.
3. **ctx 폭(사람 검토 7 자료, 기록만)**: 오프스크린 글꼴 기준으로 ctx 줄 최소 폭은 1420 px(> 1366)이고 `CtxHint` 는 372 px(sizeHint 612)다. 1845 px 캡처 `shot_cut` 에서도 ctx 안내 글은 35칸째에서 잘린다(마지막 칸 6 px). 툴팁으로 전체를 볼 수 있다는 설계(§3.6-5)대로이며, 1366 px 실측은 사람 몫이다.
4. 디자인 원본 `H` 와 나란히 보는 것(§5-5, 기준 31)과 31-33 의 최종 예/아니오는 ui-reviewer 몫이다. 이 기록은 배치·화소 사실만 적었다.
5. 검증 중 제품·시험 코드는 고치지 않았다. 검증 대상 worktree 의 `git status --porcelain` 은 시작과 끝 모두 빈 출력이다. 이 기록과 artifacts 말고는 아무것도 쓰지 않았다(분석 스크립트는 세션 scratchpad 에만 있다).

## 추기 (오케스트레이터) — 실제 한글 글꼴 캡처
- 같은 빌드(`D:\_dongarch_build\verify_job006\studio\DongArch3DStudio.exe`)를 `QT_QPA_PLATFORM=offscreen` 에 `QT_QPA_FONTDIR=C:\Windows\Fonts` 를 더해 `--sample --cut --shot`·`--sample --base --shot`·`--sample --shot` 으로 다시 찍었다(종료 코드 0·0·0). 산출물 `artifacts/realfont/shot_cut.png`(430,536 B)·`shot_base.png`(109,487 B)·`shot_main.png`(431,281 B).
- 한글·숫자가 실제로 보인다: 탭 `② 막으로 자르기`, 그룹 `막 방향 · 지금 12° (누르면 맞춤)`, `X 막 옆에서`·`Y 막 정면에서`·`Z 막 높이로`(옅음), `막 움직이기 (회전판)`, `각도 [−] 12° [+]`, `단면 위치`·`외면 왼쪽·단면 오른쪽 R`·`국내 보고서 표준`, ctx `✂ 막으로 자르기 · 3D 를 좌우로 끌어 회전판을 돌리면 …`, 오른쪽 `막 12° · 끄는 동안 2.6만 면 미리보기 · 놓으면 원본 7.7만 면 다시 계산`, 도판 머리 `A–A′ 실시간 단면 — 12°`, 상태줄 `⑥ 단면 보기 — …`. 확정 전(`--base`)에는 ② 탭이 옅게 보인다.
- 일부 기호(☑ 체크, 되돌리기 화살표 등)는 이 방식에서도 □ 로 나온다 — Malgun Gothic 에 없는 기호의 대체 글꼴을 offscreen 이 찾지 않기 때문으로 보인다(추정). 실제 화면에서는 확인 필요.

## 추기 (오케스트레이터) — GitHub Actions
- run [39048200995](https://github.com/kesekikwon-netizen/gigamesh/actions/runs/39048200995) (HEAD 688b7bd): **성공 — `100% tests passed out of 16`**. CI 구성(LTO·AVX2·PCH)으로 새 시험 `studio_cutpage` 를 빌드·실행한 첫 기록.
