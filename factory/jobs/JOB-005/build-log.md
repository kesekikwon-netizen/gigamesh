# JOB-005 구현 기록 (builder)

작업 장소: `D:\_dongarch_build\wt\.worktrees\JOB-005` (브랜치 `factory/JOB-005`, 기준 `b033aef`). 명세 §3 설계 결정을 그대로 구현했다.

## 바꾼 파일
| 파일 | 내용 | 이유 |
|---|---|---|
| `studio/src/ArtifactDoc.h` | `CutAxis`, `SectionLines`, `sliceSectionY0` 선언, `setFrontAngle` 부호 규약 주석(옛 `// 정면 회전 (, .)`), `alignCut`·`previewFrontAngle`·`commitPreview`·`isPreviewing`·`previewAngle`, `lodMesh()`·`hasEstimatedInner()` 가 미리보기를 따름, 멤버 `sectionLoops_/scannedSolidLines_/estimatedInnerLines_/estZMin_/estZMax_` → `SectionLines lines_` + `previewLines_` + `previewLod_` + `shown()` | §3.2·§3.3 |
| `studio/src/ArtifactDoc.cpp` | `recompute` 부호 `rotateZ(-θ)` → `rotateZ(θ)`(+Z 오른손 법칙), 첫 줄 `clearPreview()`; `setFrontAngle` 확정 전·미로드 가드; `alignCut`·`previewFrontAngle`(lodBase_ 복사 → 절대각 rotateZ → updateBbox → `sliceSectionY0`, FillMeshStats 없음)·`commitPreview`·`clearPreview`; `buildSectionLoops` = `clearPreview` + 순수 `sliceSectionY0(mesh_, …)` + `wallMm > 0` 이면 `estimatedWallMm_` 적용(옛 946 줄과 같은 동작); `…Natural()`·`rawPointCount` 는 `shown()`, `isWallEstimatedAt` 은 확정 `lines_` | §3.1·§3.2·§3.3 |
| `studio/src/CutGesture.h` (신규, 84줄) | 입력 합치기 상태 기계: 미리보기 16 ms throttle, 놓기 즉시·키 뗀 뒤/단추 300 ms debounce 확정, 누르는 동안 보류 | §3.4 |
| `studio/src/View3D.h/.cpp` | 신호 `cutDragReleased()` — `mouseReleaseEvent` 에서 `drag_ && cutTool_` 이었으면 | §3.5 |
| `studio/src/MainWindow.h/.cpp` | `CutGesture cut_`·단발 `QTimer* cutTimer_`(PreciseTimer)·`QElapsedTimer cutClock_`; `scheduleCut/onCutTimer/refreshPreview/updateCutInfo/beginCutInput/flushCut/cancelCut`; 끌기·놓기·키(`, . [ ] ← →`, ← → 만 Shift 15°)·`keyReleaseEvent`(isAutoRepeat 제외)·리본 `rotate`/`cutMove` 를 합치기로; onAction(rotate·cutMove 제외)·onChip·되돌리기/다시·selectSlot·각도 키가 아닌 keyPress 첫머리 `flushCut()`; 로드 3곳 첫머리 `cancelCut()`; 확정 전 가로채기에 ← →; ctx 배지 `← →` · `막 1° · Shift 15°`; `MainWindow::setFrontAngle` 삭제; ctx 오른쪽 각도 = `previewAngle()` | §3.5 |
| `studio/src/Plate.cpp` | `refusePreview` — `exportPdf`(QPdfWriter 전)·`exportPng`·`exportSvgLayered`(QFile::open 전)·`exportDxf`(Open 전) 첫 줄 | §3.6 |
| `studio/tests/test_cut.cpp` (신규) | 수용 기준 1-12 | §4 A |
| `studio/CMakeLists.txt` | `studio_core` 에 `CutGesture.h`, `test_studio_cut`·`studio_cut`·`RUN_SERIAL TRUE` | §3.7 |

크기(`git diff --numstat b033aef...HEAD`): 제품(studio/src) **+343 / −61** — 빈 줄·주석만인 줄을 빼면 추가 **292줄**. 테스트 +588, CMake +6. 명세 추정(≈260)보다 많지만 코드 줄은 300 이하라 "300줄을 크게 넘음"으로 보지 않고 005a/005b 로 나누지 않았다(늘어난 몫: CutGesture.h 주석 17줄, MainWindow 의 키 도우미 `cutKeySign/isModifierKey` 11줄, 아래 판단 2·3).

### 명세에서 판단이 필요했던 곳
1. **`CutGesture::release` 가 `held` 도 푼다.** 규칙 ⑨ 는 `!held` 일 때만 확정하므로, 키를 누르고 있는 중에 다른 동작(내보내기 등)이 `flushCut()` 하면 확정이 안 되고 내보내기가 "놓은 뒤" 로 거부된다. 명세의 "화면은 내보내기 전에 flushCut 으로 먼저 확정하므로 사용자는 거부를 보지 않는다" 를 지키려고 release = 모든 누름 끝으로 했다. 기준 11a-f 에는 영향 없음(11d 는 release 를 부르지 않음). `keyUp` 은 비활성이면 확정 마감을 잡지 않는다(잡으면 `nextDueMs` 가 지난 시각을 계속 돌려 타이머가 0 ms 로 돈다).
2. **`openPath` 가 파일을 열지 못하면** 문서가 그대로이므로 미리보기 상태가 남는다(앞에서 `cancelCut` 했으므로 확정할 길이 없음) → 그때만 `doc_.commitPreview(); refreshAll(true);` 4줄. 끌어놓기·감시 폴더 경로가 onAction 을 거치지 않아 생기는 틈.
3. **수식 키(Shift·Ctrl·Alt·Meta)만 누른 keyPress 는 확정하지 않는다.** → 를 누른 채 Shift 를 더 누르면(15° 로 바꾸기) 그 순간 확정되지 않게. Ctrl 조합(Ctrl+Z 등)은 각도 키가 아니므로 확정한다. 키 뗌 판정은 수식 키와 무관(Ctrl 을 누른 채 → 를 떼도 keyUp).

## 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| `powershell -NoProfile -ExecutionPolicy Bypass -File …\JOB-005\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-005 -BuildRoot D:\_dongarch_build\build_job005 -NoShots` (1회차) | 1 | 빌드 실패: 시험 구조체 멤버 이름 `slots` 가 Qt 키워드 매크로와 충돌(C2208) → `slotCopy` 로 바꿈 |
| 같은 명령 (2회차) | 0 | 빌드 경고 0, ctest 15/15 |
| 같은 명령 + `-Clean` (shots 포함) | 0 | 깨끗한 빌드 62단계(CXX 42개) 17 s, **경고 0**(`grep -ci warning studio-logs\02_build.log` = 0), ctest **100% tests passed out of 15**(95 s; 이 ctest 판은 ", 0 tests failed" 를 찍지 않음 — JOB-004 CI 로그와 같은 형식), shots 3장 |
| `git -C … commit -F …` | 0 | `804f60d` |

검증 결과 (빌드 폴더 `D:\_dongarch_build\build_job005\studio`):
- 기준 14: `LastTest.log` 에 `H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180`·`STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2`·`ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95` 각 1회.
- 기준 16-22(Git Bash): 16 출력 없음 · 17 `0`/`1` · 18 `0`/`2` · 19 `1`/`1`/`1` · 20 `4`/`1`/`1`/`1`/`1` · 21 출력 없음/`0` · 22 변경 10파일 모두 §6 표 안, `GigaMesh docs CLAUDE.md` 출력 없음.
- 기준 23: `shot_home.png` 13,246 · `shot_main.png` 333,540 · `shot_export.png` 70,269 바이트. `shot_main.png` 의 MD5 가 JOB-004 `artifacts/shot_main.png` 와 **같음**(1282d770…) — 0° 화면 비트 단위 동일.
- studio_cut 주요 출력(64 s):
```
[c] sample commit(37.5) vs fresh: section=0 solid=0 est=0 outline=0 hatch=0 dims=0 wall=0 estWall=0 mesh=0 lod=0 hasEst=0/0 raw=1197/1197 pts=79/79
[c] blind commit(37.5) vs fresh: section=0 solid=0 est=0 outline=0 hatch=0 dims=0 wall=0 estWall=0 mesh=0 lod=0 hasEst=1/1 raw=1217/1217 pts=121/121
[4] box θ=12 / 137: lodFaces=12 fullFaces=12 loops=1/1 section diff=0
[6] box +90°: (x,y,z) -> (-y,x,z) max err 3.55e-15
[7] box confirmed bbox center x=0 y=0 · alignCut Y x[0, 20] · X max x=10 · preview 90 max x=10, mesh.bboxMax.x=20
[d] sample preview lodFaces=26180 fullFaces=77120 median=0.80 p90=0.87 max=0.98 ms limit=16 ms (local)
[d] sample 3D render lodFaces=26180 420x640 median=14.56 ms (기록만)
[d] sample full fullFaces=77120 median=1349.5 ms (n=3, 기록만)
[d] blind preview lodFaces=26181 fullFaces=51840 median=0.63 p90=0.71 max=0.75 ms limit=16 ms (local) — 기록만
[d] blind 3D render lodFaces=26181 420x640 median=12.39 ms (기록만)
[d] blind full fullFaces=51840 median=919.3 ms (n=3, 기록만)
lod-vs-full sample θ=0 hausdorff=2.225 mean=0.178 mm hasEst preview/full=0/0
lod-vs-full sample θ=12 hausdorff=2.562 mean=0.199 mm hasEst preview/full=0/0
lod-vs-full sample θ=45 hausdorff=0.894 mean=0.128 mm hasEst preview/full=0/0
lod-vs-full sample θ=90 hausdorff=1.094 mean=0.105 mm hasEst preview/full=0/0
lod-vs-full sample θ=137 hausdorff=0.909 mean=0.115 mm hasEst preview/full=0/0
lod-vs-full blind θ=0 hausdorff=1.373 mean=0.072 mm hasEst preview/full=1/1
lod-vs-full blind θ=12 hausdorff=0.894 mean=0.051 mm hasEst preview/full=1/1
lod-vs-full blind θ=45 hausdorff=7.053 mean=0.739 mm hasEst preview/full=1/1
lod-vs-full blind θ=90 hausdorff=1.019 mean=0.090 mm hasEst preview/full=1/1
lod-vs-full blind θ=137 hausdorff=0.738 mean=0.086 mm hasEst preview/full=1/1
[11a] drag 1000 ms: previews=58 first=16 commits=0   [11b] commit target=510
[11c] commit@540 target=2   [11d] keyDowns=77 commit@3300 target=77   [11e] commit@500 target=8   [11f] cancel → next=-1
PASS
```
- 상자 확정 경로(명세 §7 미확인 위험): 비회전체 상자에서 `confirmPose`→`recompute` 가 죽지 않고 중심선 이동 0 확인.

## 남은 경고
- 빌드 경고 0. verify.ps1 이 찍는 `'vswhere.exe' is not recognized` 는 vcvars 스크립트의 안내로 빌드와 무관(이전 작업과 같음).
- `lod-vs-full blind θ=45` 하우스도르프 7.05 mm(평균 0.74 mm) — 미스캔 예제의 LOD 에서 추정 내벽 경로가 원본과 달라 놓는 순간 단면이 눈에 띄게 "튈" 수 있다(사람 검토 3·4 자료). 예제(LOD 26,180 면)는 최대 2.56 mm.
- 3D 보조창 다시 그리기 14.6 ms(420×640, LOD) — 미리보기 0.8 ms 보다 훨씬 커서 끄는 동안 프레임은 3D 래스터가 정한다(사람 검토 6).
- 놓을 때 원본 계산 1.35 s(77,120 면) 동안 UI 가 멈춘다(범위 밖 — 비동기는 뒤 작업).
- studio_cut 시험 시간 64 s(확정 로드 15회 + 원본 setFrontAngle·commitPreview 34회). CI(15분 제한, RUN_SERIAL)에서 약 1분 추가.
- 손 확인 못 함(오프스크린): ← → 가 다른 위젯에 먹히는지, 끄는 감각(사람 검토 9).

## 제안 (범위 밖, 하지 않음)
- 창이 포커스를 잃은 사이 각도 키를 떼면 `keyRelease` 가 오지 않아 제스처가 "누르는 중" 으로 남는다(다음 동작 때 flushCut 으로 확정되긴 함). `changeEvent(ActivationChange)` 에서 `flushCut()` 권장.
- 3D 보조창 `view()`·절단면 사각형이 `mesh()`(확정 각) 바운딩 박스로 크기를 정해, 끄는 동안 배율이 놓은 뒤와 아주 조금 다를 수 있다 — 004c 에서 `lodMesh()` 기준으로 맞추기.
- 리본 라벨 `정면 회전 ±1°`·`절단 위치 이동 (1°)` 와 ctx 의 `[ ] 절단면 1° 이동` 은 이제 같은 막 회전이다 — 문구 정리는 JOB-006(② 리본 페이지·ctx 원문 라벨).
- 미스캔 예제 LOD 단면 차이가 크면 미리보기 LOD 면 수를 올리거나(시간 0.6 ms 라 여유 큼) 미리보기에서 추정 내벽을 끄는 안.
- 놓을 때 멈춤(1.35 s)은 원본 계산 백그라운드화로(사람 검토 6).

## 커밋
- `804f60d` feat(studio): ② 막으로 자르기 — 실시간 단면, 끄는 동안 LOD 단면 1장·놓으면 원본 계산 (JOB-005) — 10파일 +937/−61 (제품 +343/−61, 테스트 +588, CMake +6)

## 차단 사유 (있으면)
- 없음
