# JOB-005 검증 기록 (test-runner)

결론: **PASS**. 수용 기준 1-23 모두 통과했다. 깨끗한 빌드에서 경고 0, CTest 15/15 통과. 기준 23 의 캡처 3장은 JOB-004 산출물과 MD5 가 같다.

## 환경
- 검증 대상: `D:\_dongarch_build\wt\.worktrees\JOB-005`, 브랜치 `factory/JOB-005`, HEAD `804f60d`, 기준 `b033aef`. 작업 트리 깨끗함(`git status --short` 0줄). 대상 worktree 는 읽기만 했다.
- 빌드 폴더: `D:\_dongarch_build\verify_job005\studio` (`-Clean`, 실행 전 폴더 없음 → 새로 만듦. ninja 62단계 전부 컴파일 = 깨끗한 빌드)
- 로그 폴더: `D:\_dongarch_build\verify_job005\studio-logs`
- 컴파일러: MSVC 19.44.35228.0 (`C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe`), Ninja(VS BuildTools 동봉), Release
- Qt: 6.11.1, `D:/OSGeo4W/apps/Qt6` (`Qt6_DIR=D:/OSGeo4W/apps/Qt6/lib/cmake/Qt6`), `QT_QPA_PLATFORM=offscreen`
- ctest: 4.4.1 (`C:/Program Files/CMake/bin/ctest`). git 2.55.0.windows.3. PowerShell 5.1 + Git Bash.
- 환경 변수 `CI`: 없음(개별 실행 전에 `Remove-Item Env:\CI`) → 시간 기준 `limit=16 ms (local)`
- 콘솔에 `'vswhere.exe' is not recognized …` 가 두 번 찍혔다(vcvars64.bat 이 낸 것). configure·build 종료 코드는 0 이고 로그에는 남지 않는다. 결과에 영향 없음.

## 실행 시간·경고
| 단계 | 명령 | 종료 코드 | 시간 | 시각 |
|---|---|---|---|---|
| verify.ps1 전체 | `powershell -NoProfile -ExecutionPolicy Bypass -File D:\_dongarch_build\wt\.worktrees\JOB-005\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-005 -BuildRoot D:\_dongarch_build\verify_job005 -Job JOB-005 -RecordRoot D:\_dongarch_build\wt -Clean` | **0** | 128 s | 22:07:42 – 22:09:50 |
| ├ configure | (스크립트) | 0 | 4 s | |
| ├ build | (스크립트) | 0 | 18 s | |
| ├ ctest | (스크립트, `ctest --output-on-failure`) | 0 | 96 s (studio_cut 65.71 s) | |
| └ shots | (스크립트, `--shot` ×3) | 0 | 9 s | |
| 개별 studio_cut | `ctest -R studio_cut -V` (§5-2 환경 변수) | **0** | 64.65 s | 22:10 – 22:11 |
| 전체 ctest 재실행 | `ctest --output-on-failure` | **0** | 94.30 s | 22:11 – 22:13 |

- 빌드 경고: **0** (`grep -ci "warning" D:/_dongarch_build/verify_job005/studio-logs/02_build.log` → `0`)

## 수용 기준별 결과
기준 1-12 는 `B` 에서 실행한 `ctest -R studio_cut -V`(종료 코드 0) 한 번의 출력이 근거다. 전문은 `artifacts/ctest_studio_cut_V.log`. `test_cut.cpp` 를 읽어 각 항목이 인쇄만 하지 않고 `CHECK` 로 판정하는지, 기준값이 명세와 같은지(1e-9 mm, `==`, 16 ms, 55-63, 16-17 ms 등)도 확인했다. 실패한 `CHECK` 가 하나라도 있으면 마지막 줄이 `FAILED`, 종료 코드가 1 이 된다.
grep 종료 코드: `grep -c` 는 일치가 0개면 1 을 낸다. 기대값이 `0` 인 기준에서 종료 코드 1 은 정상이다.

| 기준 | 명령 | 종료 코드 | 결과 | 근거(로그 줄·산출물 경로) |
|---|---|---|---|---|
| 1 (e) 확정 전·미로드 무시 | `ctest -R studio_cut -V` | 0 | 통과 | `[1] unloaded/unconfirmed: ignored (front=0 preview=0 section=0)`. 미로드 문서와 `loadSample(false)` 문서의 previewing/front/previewAngle/section/정점 `==` 를 CHECK 함 |
| 2 (c-1) 끄는 동안 불변 | 〃 | 0 | 통과 | sample·blind 각 6줄 `[2] … frozen=same … lodDiff=0` (θ=5,12,−20,90,137,37.5). 단면은 비어 있지 않음(loops=1), dims·외곽·두께·추정·estWall·mesh·slot `==` |
| 3 (c-2) 놓기 == 새 문서 | 〃 | 0 | 통과 | `[c] sample commit(37.5) vs fresh: section=0 solid=0 est=0 outline=0 hatch=0 dims=0 wall=0 estWall=0 mesh=0 lod=0 hasEst=0/0 raw=1197/1197 pts=79/79`, `[c] blind … 모두 0 … hasEst=1/1 raw=1217/1217 pts=121/121` |
| 4 (c-3) 같은 메시 = 같은 단면 | 〃 | 0 | 통과 | `[4] box θ=12: lodFaces=12 fullFaces=12 loops=1/1 section diff=0`, `[4] box θ=137: … section diff=0` |
| 5 확정값 바꾸면 미리보기 지움 | 〃 | 0 | 통과 | `[5] wall/infer/front/load end preview; smooth/simplify/side keep it (previewing=1)` (각 단계 CHECK, `frontAngle()==10.0` 포함) |
| 6 각도 부호(+ = 반시계) | 〃 | 0 | 통과 | `[6] box +90°: (x,y,z) -> (-y,x,z) max err 3.55e-15` (≤ 1e-9) |
| 7 (f) 막 맞춤 | 〃 | 0 | 통과 | `[7] box confirmed bbox center x=0 y=0`, `[7] alignCut Y: front=0 x[0.000000, 20.000000] \| X: front=90 max x=10.000000`, `[7] preview 90 max x=10.000000 mesh.bboxMax.x=20.000000 → alignCut(Y) previewing=0` |
| 8 미리보기 중 내보내기 거부 | 〃 | 0 | 통과 | svg/pdf/png/dxf 4줄 `ok=0 file=0 err=막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.`, `[8] svg after commit: ok=1 size=1539040 bytes` |
| 9 (d) 시간 ≤ 16 ms | 〃 | 0 | 통과 | `[d] sample preview lodFaces=26180 fullFaces=77120 median=0.70 p90=0.74 max=0.84 ms limit=16 ms (local)` (51회, i=0 예열 제외) |
| 10 LOD↔원본 거리(기록) | 〃 | 0 | 통과 | `lod-vs-full` 10줄 모두 단면이 비어 있지 않고 값이 유한함(아래 원문). 공식 허용오차 없음 → 사람 검토 3 |
| 11 입력 합치기 | 〃 | 0 | 통과 | `[11a] drag 1000 ms: previews=58 first=16 commits=0` (간격 16-17 CHECK), `[11b] release(1000): commit target=510 active=0`, `[11c] … commits(<540)=0 → commit@540 target=2`, `[11d] held 3000 ms: keyDowns=77 commits=0 → keyUp(3000) commit@3300 target=77`, `[11e] clicks 0/100/200: commit@500 target=8`, `[11f] commit before preview; cancel → active=0 next=-1` |
| 12 마지막 줄 PASS·종료 0 | 〃 | 0 | 통과 | 마지막 줄 `PASS`, `1/1 Test #4: studio_cut ... Passed 64.65 sec` |
| 13 CTest 15종 (수정 문구) | `ctest --output-on-failure` (B, 재실행) | 0 | 통과 | `100% tests passed out of 15` (`artifacts/ctest_all_rerun.log` 33행, verify 의 `03_ctest.log` 33행도 같음). `tests failed` 가 0 이 아닌 줄 0개 |
| 14 같은 값 (LastTest.log 세 줄) | `grep -c "<문자열>" B\Testing\Temporary\LastTest.log` ×3 (verify 직후 + 전체 재실행 뒤) | 0 / 0 / 0 | 통과 | 각 `1`: 11행 `H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180`, 14행 `STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2 …`, 15행 `ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95`. 사본: `artifacts/LastTest_verify.log`, `artifacts/LastTest_rerun.log`. JOB-004 기준 로그에도 세 줄 모두 1 |
| 15 verify 종료 0·PASS·경고 0 | verify.ps1 / `tail -1 B/verify-summary.md` / `grep -ci "warning" …/02_build.log` | 0 / 0 / 1(=0개) | 통과 | `VERIFY_EXIT=0`, 마지막 줄 `결과: PASS`, 경고 `0` |
| 16 기존 시험 무변경 | `git diff --name-only b033aef -- studio/tests/test_pipeline.cpp studio/tests/test_pose.cpp studio/tests/test_side.cpp` | 0 | 통과 | 출력 없음(0줄) |
| 17 각도 입력은 합치기 경유 | `grep -c "setFrontAngle(doc_.frontAngle()" studio/src/MainWindow.cpp` / `grep -c "previewFrontAngle" …` | 1 / 0 | 통과 | `0` / `1` |
| 18 미리보기 갱신 범위 | `sed -n '/^void MainWindow::refreshPreview/,/^}/p' … \| grep -cE "docChanged\|refreshAll\|refreshSlots"` / `… grep -cE "view3d_->refresh\|canvas\(\)->update"` | 1 / 0 | 통과 | `0` / `2`. 본문은 `sheet_->refresh`, `sheet_->canvas()->update()`, `view3d_->refresh`, `updateCutInfo()` 넷이다. `updateCutInfo` 는 `ctxRight_->setText(… doc_.previewAngle() …)` 만 하고 전체 갱신을 부르지 않는다(직접 읽음) |
| 19 놓기 신호·끌기 비율 | `grep -c "cutDragReleased" studio/src/View3D.h` / `… MainWindow.cpp` / `grep -c "cutAngleDragged(d.x() \* 0.5)" studio/src/View3D.cpp` | 0 / 0 / 0 | 통과 | `1` / `1` / `1` |
| 20 키·배지 | `grep -cE "Key_Left\|Key_Right"` / `grep -c "isAutoRepeat"` / `grep -c 'kbdLabel("← →"'` / `grep -c "막 1° · Shift 15°"` (MainWindow.cpp), `grep -c "놓은 뒤" studio/src/Plate.cpp` | 0 ×5 | 통과 | `4` (≥2) / `1` (≥1) / `1` / `1` / `1` (≥1) |
| 21 R2·R3 | `git diff b033aef -- studio/src \| grep -E '^\+.*(QColor\([0-9]\|qRgba?\([0-9]\|#[0-9A-Fa-f]{6}\|Qt::(red\|…\|lightGray)\b)'` / `git diff b033aef -- studio/src \| grep -c '^\+.*ToolState::Primary'` | 1(=일치 없음) / 1(=0개) | 통과 | 출력 없음 / `0` |
| 22 변경 범위 | `git diff --name-only b033aef...HEAD` / `git diff --name-only b033aef...HEAD -- GigaMesh docs CLAUDE.md` | 0 / 0 | 통과 | 10개 파일(`studio/CMakeLists.txt`, `studio/src/ArtifactDoc.cpp`, `ArtifactDoc.h`, `CutGesture.h`, `MainWindow.cpp`, `MainWindow.h`, `Plate.cpp`, `View3D.cpp`, `View3D.h`, `studio/tests/test_cut.cpp`). 모두 §6 표에 있다 / 출력 없음. 937줄 추가, 61줄 삭제 |
| 23 캡처 | `stat -c %s` ×3, `md5sum` (JOB-005 vs JOB-004 `artifacts/shot_*.png`) | 0 | 통과 | `shot_home.png` 13246 B, `shot_main.png` 333540 B, `shot_export.png` 70269 B (모두 > 5120). `shot_main.png` MD5 `1282d7709d0bf3561038d6918bae1a8e` = JOB-004 와 같음(비트 단위로 같은 이미지 → 같은 배치·같은 치수 띠). home·export 도 JOB-004 와 MD5 가 같다(`fb6f12b3…`, `ac459e94…`). 육안으로 봐도 0° 단면(빨강)이 오른쪽, 외면 음영이 왼쪽이다. 오프스크린 캡처라 한글이 □ 로 보인다 = **폰트 미탐색**(레이아웃 판정에는 영향 없음, JOB-004 와 같음) |

## studio_cut 주요 출력 (원문, `ctest -R studio_cut -V`)
```
4: [1] unloaded/unconfirmed: ignored (front=0 preview=0 section=0)
4: [2] sample θ=5: preview=5 front=0 frozen=same loops=1 pts=60 lodFaces=26180 lodDiff=0
4: [2] sample θ=12: preview=12 front=0 frozen=same loops=1 pts=57 lodFaces=26180 lodDiff=0
4: [2] sample θ=-20: preview=-20 front=0 frozen=same loops=1 pts=60 lodFaces=26180 lodDiff=0
4: [2] sample θ=90: preview=90 front=0 frozen=same loops=1 pts=54 lodFaces=26180 lodDiff=0
4: [2] sample θ=137: preview=137 front=0 frozen=same loops=1 pts=51 lodFaces=26180 lodDiff=0
4: [2] sample θ=37.5: preview=37.5 front=0 frozen=same loops=1 pts=57 lodFaces=26180 lodDiff=0
4: [c] sample commit(37.5) vs fresh: section=0 solid=0 est=0 outline=0 hatch=0 dims=0 wall=0 estWall=0 mesh=0 lod=0 hasEst=0/0 raw=1197/1197 pts=79/79
4: [2] blind θ=5: preview=5 front=0 frozen=same loops=1 pts=80 lodFaces=26181 lodDiff=0
4: [2] blind θ=12: preview=12 front=0 frozen=same loops=1 pts=80 lodFaces=26181 lodDiff=0
4: [2] blind θ=-20: preview=-20 front=0 frozen=same loops=1 pts=77 lodFaces=26181 lodDiff=0
4: [2] blind θ=90: preview=90 front=0 frozen=same loops=1 pts=74 lodFaces=26181 lodDiff=0
4: [2] blind θ=137: preview=137 front=0 frozen=same loops=1 pts=83 lodFaces=26181 lodDiff=0
4: [2] blind θ=37.5: preview=37.5 front=0 frozen=same loops=1 pts=83 lodFaces=26181 lodDiff=0
4: [c] blind commit(37.5) vs fresh: section=0 solid=0 est=0 outline=0 hatch=0 dims=0 wall=0 estWall=0 mesh=0 lod=0 hasEst=1/1 raw=1217/1217 pts=121/121
4: [4] box θ=12: lodFaces=12 fullFaces=12 loops=1/1 section diff=0
4: [4] box θ=137: lodFaces=12 fullFaces=12 loops=1/1 section diff=0
4: [6] box +90°: (x,y,z) -> (-y,x,z) max err 3.55e-15 (front face y=-10 -> x=+10)
4: [7] box confirmed bbox center x=0 y=0
4: [7] alignCut Y: front=0 x[0.000000, 20.000000] | X: front=90 max x=10.000000
4: [7] preview 90 max x=10.000000 mesh.bboxMax.x=20.000000 → alignCut(Y) previewing=0
4: [5] wall/infer/front/load end preview; smooth/simplify/side keep it (previewing=1)
4: [8] svg during preview: ok=0 file=0 err=막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.
4: [8] pdf during preview: ok=0 file=0 err=막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.
4: [8] png during preview: ok=0 file=0 err=막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.
4: [8] dxf during preview: ok=0 file=0 err=막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.
4: [8] svg after commit: ok=1 size=1539040 bytes
4: [d] sample preview lodFaces=26180 fullFaces=77120 median=0.70 p90=0.74 max=0.84 ms limit=16 ms (local)
4: [d] sample 3D render lodFaces=26180 420x640 median=14.94 ms (기록만)
4: [d] sample full fullFaces=77120 median=1364.8 ms (n=3, 기록만)
4: [d] blind preview lodFaces=26181 fullFaces=51840 median=0.63 p90=0.71 max=0.74 ms limit=16 ms (local) — 기록만
4: [d] blind 3D render lodFaces=26181 420x640 median=12.45 ms (기록만)
4: [d] blind full fullFaces=51840 median=930.6 ms (n=3, 기록만)
4: lod-vs-full sample θ=0 hausdorff=2.225 mean=0.178 mm hasEst preview/full=0/0
4: lod-vs-full sample θ=12 hausdorff=2.562 mean=0.199 mm hasEst preview/full=0/0
4: lod-vs-full sample θ=45 hausdorff=0.894 mean=0.128 mm hasEst preview/full=0/0
4: lod-vs-full sample θ=90 hausdorff=1.094 mean=0.105 mm hasEst preview/full=0/0
4: lod-vs-full sample θ=137 hausdorff=0.909 mean=0.115 mm hasEst preview/full=0/0
4: lod-vs-full blind θ=0 hausdorff=1.373 mean=0.072 mm hasEst preview/full=1/1
4: lod-vs-full blind θ=12 hausdorff=0.894 mean=0.051 mm hasEst preview/full=1/1
4: lod-vs-full blind θ=45 hausdorff=7.053 mean=0.739 mm hasEst preview/full=1/1
4: lod-vs-full blind θ=90 hausdorff=1.019 mean=0.090 mm hasEst preview/full=1/1
4: lod-vs-full blind θ=137 hausdorff=0.738 mean=0.086 mm hasEst preview/full=1/1
4: [11a] drag 1000 ms: previews=58 first=16 commits=0
4: [11b] release(1000): commit target=510 active=0
4: [11c] keys 0/40/200/240: previews=2 commits(<540)=0 → commit@540 target=2
4: [11d] held 3000 ms: keyDowns=77 commits=0 → keyUp(3000) commit@3300 target=77
4: [11e] clicks 0/100/200: commit@500 target=8
4: [11f] commit before preview; cancel → active=0 next=-1
4: PASS
1/1 Test #4: studio_cut .......................   Passed   64.65 sec
100% tests passed out of 1
```

## 산출물 (`factory/jobs/JOB-005/artifacts/`, 커밋하지 않음)
- verify.ps1 이 복사한 것: `01_configure.log`, `02_build.log`, `03_ctest.log`, `04_shots.log`(0 B, 정상), `verify-summary-studio.md`, `shot_home.png`, `shot_main.png`, `shot_export.png`
- test-runner 가 추가한 것: `ctest_studio_cut_V.log`(개별 실행 전문), `ctest_all_rerun.log`(전체 재실행), `LastTest_verify.log`(verify 직후 LastTest.log 사본), `LastTest_rerun.log`(재실행 뒤 사본)

## 비고 (판정에는 영향 없음 — 사람 검토용 자료)
- **사람 검토 3 (LOD↔원본)**: 미스캔 예제 `blind θ=45` 의 hausdorff 가 **7.053 mm**(평균 0.739 mm)로 다른 9줄(0.74–2.56 mm)보다 크다. 이 각도에서 손을 놓으면 단면이 눈에 띄게 "튈" 수 있다. 공식 허용오차가 없으므로 기록만 한다.
- **사람 검토 6 (놓을 때 멈춤·3D)**: 원본 다시 계산 중앙값은 예제(77,120 면) 1364.8 ms, 미스캔(51,840 면) 930.6 ms 다. 손을 놓으면 UI 가 약 1.4 s 멈춘다. 3D 보조창 한 장(420×640) 중앙값은 14.94 / 12.45 ms 로, 미리보기 단면(0.70 ms)보다 3D 다시 그리기가 한 프레임(16 ms) 예산을 거의 다 쓴다.
- `studio_cut` 한 번에 약 65 s 가 걸려 CTest 전체 시간이 JOB-004 보다 늘었다(대부분 원본 `setFrontAngle` 반복). CMakeLists 102행에 `RUN_SERIAL TRUE` 가 있는 것을 확인했다(§3.7).
- decision.md 에 적힌 builder 의 명세 이탈 3건(release 가 키 누름 상태도 풂, openPath 실패 시 확정, 수식 키만 누르면 확정 안 함)은 이 수용 기준들이 직접 시험하지 않는다. 검토자 판정 대상이고, 이 검증은 판단하지 않았다.
- 실시간 동작(끌기·키 반복·Ctrl+Z) 자체는 캡처나 자동 시험 대상이 아니다 → 사람 검토 9 손 확인.
- 제품 코드·테스트 코드·다른 기록 파일은 고치지 않았다. git checkout/stash/reset/push 도 하지 않았다.

## 추기 (오케스트레이터) — GitHub Actions
- run [39035205560](https://github.com/kesekikwon-netizen/gigamesh/actions/runs/39035205560) (HEAD 804f60d): **성공 — `100% tests passed out of 15`**, 2026-10-17T13:06:31Z → 13:10:02Z.
- CI 는 `ctest --output-on-failure` 로 돌아 `studio_cut` 의 `[d] … limit=64 ms (CI)` 줄이 로그에 남지 않는다. 기준 9 의 CI 쪽은 통과 여부만 있고 수치는 없다(사람 검토 8).
