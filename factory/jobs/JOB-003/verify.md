# JOB-003 검증 기록 (test-runner)

결론: **FAIL** — 실패 기준 **6** (나머지 1-5·7-18 통과, 19-23 은 ui-reviewer 육안 판정 몫)

요약: 처음부터 다시 빌드(58/58, /W4 경고 0)·CTest 13/13 통과·캡처 5장 정상. 그러나 수용 기준 6 의 "`loadSample(false)` → `confirmPose()` 뒤 `dims().ok`" 는 직접 실행 결과 **`ok=0`(H=200.50, rim=0.00)** 으로 성립하지 않는다. `test_pose.cpp` 는 이 검사를 명세대로 하지 않고 `dims().height > 0`(162행)으로 약화하고, `dims().ok` 는 명세에 없는 "자동 세우기 → 확정" 경로(6b)에서만 검사하므로 CTest 통과가 기준 6 통과를 뜻하지 않는다. builder 도 build-log "주의 1" 에서 같은 사실을 밝혔다 — 명세 쪽 결함(수동 확정은 회전하지 않음 + K-Arch 치수 분석은 정위치 전제)일 가능성이 높으므로 명세 수정(6a/6b 로 분리) 또는 설계 변경 중 하나를 사람/코디네이터가 정해야 한다.

## 환경
- 컴파일러: MSVC 19.44.35228.0 (BuildTools 14.44.35207, Hostx64/x64), Ninja, CMake/CTest 4.4.1, Release(`/O2 /Ob2 /DNDEBUG`), 경고 수준 `/W4 /permissive-` (build.ninja 에서 `/W4` 40회 확인)
- Qt: `D:/OSGeo4W/apps/Qt6` (Qt6Core 6.11.1), 실행 시 `QT_QPA_PLATFORM=offscreen`
- 검증 대상: `D:\_dongarch_build\wt\.worktrees\JOB-003` 브랜치 `factory/JOB-003` HEAD `e9b655ae1b1feabba83cc39169f599dbfb6884c5` (기준 `58a33ae`, 커밋 2개 `1ad49b4`·`e9b655a`), `git status --porcelain` 0줄(작업 트리 깨끗, 검증 후에도 0줄)
- 빌드 폴더: `D:\_dongarch_build\verify_job003\studio` (로그 `…\verify_job003\studio-logs`). 실행 전 `studio`·`studio-logs` 두 폴더를 지우고 처음부터 구성·빌드(`[1/58]`…`[58/58]`). builder 의 `verify_job003\pose_out.txt` 는 손대지 않음.
- 실행 시각: 2026-10-17 19:25–19:28 (KST). verify.ps1 총 49 초(configure 4 · build 17 · ctest 18 · shots 9), ctest 단독 약 18 초.

## 실행한 명령
| # | 명령 | 종료 코드 |
|---|---|---|
| V1 | `rm -rf D:/_dongarch_build/verify_job003/studio D:/_dongarch_build/verify_job003/studio-logs` | 0 |
| V2 | `powershell -NoProfile -ExecutionPolicy Bypass -File D:/_dongarch_build/wt/.worktrees/JOB-003/factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-003 -BuildRoot D:\_dongarch_build\verify_job003 -Job JOB-003 -RecordRoot D:\_dongarch_build\wt` (캡처 포함) | **0** |
| V3 | (PowerShell, Qt6 bin/plugins·offscreen) `Set-Location D:\_dongarch_build\verify_job003\studio; ctest -N` | 0 (`Total Tests: 13`) |
| V4 | 같은 위치 `ctest --output-on-failure` (2회: 19:26, 19:28 최종) | 0 / 0 |
| V5 | 같은 위치 `ctest -R studio_pose --output-on-failure -V` | 0 |
| V6 | 같은 위치 `ctest -R studio_pipeline -V` | 0 |
| V7 | `Start-Process -Wait … DongArch3DStudio.exe --sample --base --shot …\shots\shot_base.png` | 0 (70,436 B) |
| V8 | `Start-Process -Wait … DongArch3DStudio.exe --blind --base --shot …\shots\shot_base_blind.png` (선택) | 0 (75,223 B) |
| V9 | 수용 기준 15-18 grep/git (Git Bash, worktree 에서) | 아래 표 |

## 수용 기준별 결과
| 기준 | 명령 | 종료 코드 | 결과 | 근거(로그 줄·산출물 경로) |
|---|---|---|---|---|
| 1 90°×4 = 원래 (X·Y·Z, ±90) | V5 | 0 | 통과 | `rotate X +90 x4: max|d|=2.04e-14 mm` … `rotate Z -90 x4: max|d|=2.04e-14 mm` 6줄 모두 ≤ 1e-9 (`artifacts/tr_ctest_pose_V.log`). 테스트 79-97행이 명세 순서(loadSample(false)→dropToFloor→P0→4회) 그대로 |
| 2 뒤집기×2 = 원래 | V5 | 0 | 통과 | `flip x2: max|d|=2.04e-14 mm` |
| 3 조작 뒤 접지·박스 중심 유지 | V5 | 0 | 통과 | `after each op: max|bboxMin.z|=0 mm, max|center xy shift|=7.11e-15 mm`; dropToFloor 직후 검사 81행(실패 출력 없음) |
| 4 처음 자세로 정확 복원 | V5 | 0 | 통과 | `reset: max|d|=0 mm`, 조작 순서 X+90·Y−90·뒤집기·내려놓기·Y−74 (113-117행), `poseConfirmed()==false` 검사 122행 |
| 5 눕힌 항아리 자동 세우기→확정 | V5 | 0 | 통과 | `ref: H=184.80 rim=142.00 belly=165.50 base=142.00` / `laid Y-74 auto: H=184.80 rim=142.00 belly=165.50 base=142.00` → 차 0 ≤ 1.0, `|184.80−184.5|=0.30 ≤ 1.5`, `|142.00−142.0|=0 ≤ 2.0`, `dims().ok` 검사 141행 |
| **6** 확정 전 미계산 / 확정 뒤 계산 | V5 + `studio/tests/test_pose.cpp` 읽기 | 0 (테스트는 통과) | **실패** | (가) 확정 전 7항목: `loadSample(false)`·`loadFile` 직후 모두 통과(`checkUnconfirmed` 60-68행, 실패 출력 없음). (나) 확정 뒤 다시 돌리면 `poseConfirmed()==false`·`dims().ok==false`: 통과(165-168, 186-188행). (다) **명세 문구 "`loadSample(false)` → `confirmPose()` 뒤 `dims().ok`" 불성립**: `confirm as loaded (tilted 7deg): ok=0 H=200.50 rim=0.00 section=1 bboxMin.z=0`. 테스트는 이 경로에서 `dims().ok` 대신 `dims().height > 0.0`(162행)만 검사하고, `dims().ok` 는 명세에 없는 `autoAlign()` 을 끼운 6b 경로(`auto -> confirm: ok=1 H=184.80 rim=142.00 section=1 slots=4 bboxMin.z=0`)에서만 검사함. `!displaySection().empty()`·`|bboxMin.z| ≤ 1e-9` 는 두 경로 모두 성립 |
| 7 파일 열기는 자세 불변 | V5 | 0 | 통과 | `loadFile: bbox min (-70.69, -100.59, 31.32) max (94.65, 68.10, 231.78) max|d| vs read=0 mm` → 6성분 ≤ 1e-9, `bboxMin.z=31.32 > 1.0`, 미확정(209행). PLY 는 `exportLegacyPly(…, false)` ASCII(196행) |
| 8 수동 확정(곧게 선 항아리) | V5 | 0 | 통과 | `manual confirm: H=184.80 rim=142.00 belly=165.50 base=142.00 note=바닥판 기준 (사람 확정) · 중심선 원 맞춤` → `|0.30| ≤ 1.5`, `0 ≤ 2.0`, 접지·단면 검사 220·224행 실패 출력 없음 |
| 9 마지막 줄 PASS·종료 0 | V5 | 0 | 통과 | 마지막 출력 줄 `PASS`, `1/1 Test #2: studio_pose ... Passed 8.91 sec`. 단, 이 PASS 는 기준 6 을 약화한 검사 기준이다(위 6 참고) |
| 10 test_pipeline STL 왕복만 변경·통과 | `git diff 58a33ae..HEAD -- studio/tests/test_pipeline.cpp` + V6 | 0 / 0 | 통과 | diff 는 138행 뒤 `d2.autoAlign();`·`d2.confirmPose();` 2줄 추가뿐, 기대값·허용오차 변경 없음. `studio_pipeline` Passed 7.35 s (`artifacts/tr_ctest_pipeline_V.log`) |
| 11 기존 값 유지 (LastTest.log) | `grep -cF` 3종 on `B\Testing\Temporary\LastTest.log` (최종 전체 실행 V4 뒤) | — | 통과 | 4 / 1 / 1 (모두 ≥ 1). 병합본 `verify_merged` 기준값도 1/1/1. `studio_pipeline` 출력 7줄이 병합본과 **글자 단위 동일**(aligned·note·rmse=0.0051·H/rim/belly/base·wall=6.80 pts=85/1318 lodFaces=26180·STL·ArtecBlind estWall=6.95). `artifacts/tr_LastTest_full.log`, `tr_LastTest_pipeline_only.log` |
| 12 CTest 13종 전부 통과 | V3, V4 | 0 / 0 | 통과(문구 형식 주석) | `ctest -N` → 13종(`studio_pipeline`, `studio_pose`, karch 11종). V4 → `100% tests passed out of 13`, `Test Passed` 13 · `Test Failed` 0. 명세의 문자열 `100% tests passed, 0 tests failed out of 13` 은 CTest 4.4.1 이 출력하지 않는 형식이라 그대로는 0건(JOB-001·002 검증과 같은 처리) — 실질 조건(13/13·실패 0·종료 0) 충족 |
| 13 verify.ps1 0·요약 PASS·경고 0 | V2, `tail -n1 verify-summary.md`, `grep -ci warning studio-logs/02_build.log` | 0 | 통과 | 요약 마지막 줄 `결과: PASS`, configure 0 / build 0 / ctest 0 / shots 0. 경고 `0` (기준선 `verify_merged/studio-logs/02_build.log` 도 0), `error` 0. 전체 재빌드 `[58/58] Linking CXX executable DongArch3DStudio.exe` |
| 14 캡처 크기 > 5120 B | V7, V2 | 0 / 0 | 통과 | `shot_base.png` 70,436 · `shot_home.png` 13,562 · `shot_main.png` 334,423 · `shot_export.png` 70,569 B |
| 15 원문 라벨 14개·옛 탭 0 | 명세 15 의 for-grep, `grep -c 'addPage(QStringLiteral("정위치·중심축"))' studio/src/MainWindow.cpp` | — | 통과 | `MISSING` 출력 없음 / `0` |
| 16 새 색 리터럴 없음·주 단추 1개 | 명세 16 의 두 파이프 | — | 통과 | 색 grep 출력 없음(grep 종료 1) / `grep -vc poseConfirm` = `0` (추가된 `ToolState::Primary` 2줄 모두 `poseConfirm`) |
| 17 변경 범위 | `git diff --name-only 58a33ae...HEAD` (+ `..HEAD` 동일), 금지 경로 diff | — | 통과 | 15개 파일 = 영향 파일 표 15개와 일치(UI_RULES.md, CMakeLists.txt, ArtifactDoc.h/.cpp, MainWindow.h/.cpp, MeshRaster.h/.cpp, Ribbon.cpp, SheetView.cpp, View3D.h/.cpp, main.cpp, test_pipeline.cpp, test_pose.cpp). `-- GigaMesh studio/src/Plate.cpp studio/src/Plate.h studio/src/ExportDialog.cpp` 출력 없음. 추가로 `GigaMesh/core`·`external`·`karch`·`ExportDialog.h`·`StartPage.*`·`Theme.h`·`factory/scripts` 도 출력 없음 |
| 18 UI_RULES 정본 동기화 | `grep -c "① 바닥 정하기" docs/ui/UI_RULES.md` | — | 통과 | `3` |
| 19-23 육안 검수 | `artifacts/shot_base.png` 열람 | — | ui-reviewer 판정 몫(참고 관찰만) | 한글 전부 □ (알려진 위험 7, 폰트 미탐색 — 불합격 사유 아님). 관찰: 탭 줄 세 번째 탭 밑줄(켜짐), 리본 3그룹(타일 2 / 타일 1+작은 단추 6 / 붉은 ✓ 타일 1) 중 Action 색은 확정 타일 하나, 3D 창에 방안 바닥판 위 항아리, ctx 줄 왼쪽 어두운 배지·오른쪽 모노 문구, 도판 자리 가운데 한 줄 문구, 도판 머리 오른쪽 경고색 테두리 배지, 상태줄 좌·우 문구 있음. 문구 내용은 □ 라 판독 불가 |

## 캡처 비교 (검증 명령 4)
- `shot_home.png`·`shot_export.png`: JOB-002 산출물과 MD5 동일(바이트 동일).
- `shot_main.png`: 같은 크기 1854×920, 픽셀 차이 영역 `(216,11)-(402,20)` 160 px 뿐(탭 이름 `정위치·중심축` → `① 바닥 정하기` 자리). 도판·치수 띠·홈 리본은 그대로. 비교 스크립트: scratchpad `imgdiff.py`(PIL, 읽기 전용).
- `shot_base_blind.png`(선택): `--blind --base` 종료 0, 확정 전 화면이 `shot_base.png` 와 같은 구성.

## 산출물 (artifacts/, 커밋하지 않음)
- verify.ps1 복사: `01_configure.log`, `02_build.log`, `03_ctest.log`, `04_shots.log`(빈 파일 — 캡처 명령은 표준 출력 없음), `verify-summary-studio.md`, `shot_home.png`, `shot_main.png`, `shot_export.png`
- test-runner 직접 실행: `tr_ctest_full.log`(19:26), `tr_ctest_full_final.log`(19:28), `tr_ctest_pose_V.log`, `tr_ctest_pipeline_V.log`, `tr_LastTest_full.log`, `tr_LastTest_pipeline_only.log`, `shot_base.png`, `shot_base_blind.png`

## 비고
- builder build-log 의 주장(13/13·경고 0·shot_base 70,436 B·shot_main 탭 영역만 차이·홈/내보내기 바이트 동일·기준 6 문자 그대로 불가)은 이번 재실행에서 모두 같은 결과로 재현됐다. 단, builder 가 "기준 6 은 6a/6b 로 나눠 검사"한 것은 명세 문구를 바꾼 것이라 test-runner 는 통과로 적지 않는다.
- 기준 6 실패의 원인 판단(참고): 명세 1절 `confirmPose()` 규칙 2 "회전은 하지 않는다" 때문에 예제(7° 기울어 들어옴)를 그대로 확정하면 기울어진 채 치수를 계산하고, K-Arch 치수 분석이 구연을 못 찾아 `rim=0`·`ok=false` 가 된다. 명세 자체가 서로 맞지 않는 것으로 보이며, 해결은 (가) 명세 기준 6 을 "확정 뒤 `dims().height > 0`·단면·접지, `dims().ok` 는 자동 세우기→확정 경로" 로 고치거나 (나) 기울어진 자세 확정 시 안내/경고를 넣는 설계 변경 — 사람 검토 항목으로 올림.
- 3D 보조창 관찰(판정 아님): 기존 3D 래스터 시점이 약간 아래에서 올려다보는 각도라(확정 뒤 `shot_main` 에서도 바닥 원판 쪽 타원이 보임) 바닥판이 항아리 아랫부분을 가리지 않고 뒤에 깔린다(명세 4절 "메시보다 먼저 그림" 그대로). ui-reviewer 참고.
- 콘솔의 `'vswhere.exe' … 아닙니다` 메시지는 vcvars64 기존 동작, 종료 코드·빌드 영향 없음.
- 제품·테스트 코드는 수정하지 않았다. worktree 는 검증 전후 모두 `git status` 깨끗.

---

## 추기 (2026-10-17, 오케스트레이터) — 수용 기준 6 재판정

명세 기준 6 이 6a/6b 로 수정됐다(decision.md). 코드는 그대로(HEAD e9b655a)이며, 같은 빌드 폴더 `D:\_dongarch_build\verify_job003\studio` 에서 `ctest -R studio_pose -V` 를 다시 실행했다(종료 0).

| 기준 | 근거(출력 줄) | 판정 |
|---|---|---|
| 6a | `confirm as loaded (tilted 7deg): ok=0 H=200.50 rim=0.00 section=1 bboxMin.z=0` — 확정·단면·접지 성립, 기울어진 구연이라 구경 미측정(ok=0)은 수정된 기준상 정상. 확정 뒤 회전 시 미확정 검사 통과 | 통과 |
| 6b | `auto -> confirm: ok=1 H=184.80 rim=142.00 section=1 slots=4 bboxMin.z=0` | 통과 |

**수정된 명세 기준으로 결론: PASS** (나머지 1-5·7-18 은 위 본문대로 통과, 19-23 은 ui-reviewer 판정 몫).
