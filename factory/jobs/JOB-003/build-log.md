# JOB-003 구현 기록 (builder)

작성: builder, 2026-10-17. worktree `D:/_dongarch_build/wt/.worktrees/JOB-003` (브랜치 `factory/JOB-003`, 기준 `58a33ae`).
명세 권고대로 **A(엔진·테스트) → B(화면) 커밋 2개**로 나눴고, 각 커밋 상태에서 빌드·ctest 를 통과시켰다.

## 재작업 1회차 (2026-10-17) — 결과: **차단(blocked)**, 코드 변경·새 커밋 없음

지적(verify.md): 수용 기준 6 의 "`loadSample(false)` → `confirmPose()` 뒤 `dims().ok`" 가 성립하지 않고(`ok=0 H=200.50 rim=0.00`), `test_pose.cpp` 162행은 `dims().height > 0` 으로 약화해 검사한다.

### 확인한 사실 (읽기·실행만)
| 확인 | 근거 |
|---|---|
| 예제는 일부러 7° 기울어져 들어온다 | `studio/src/ArtifactDoc.cpp:267` `sp.tiltDeg = 7.0; // 일부러 기울여 정위치 과정을 보여준다` (기존 코드, 이 JOB 이전부터) |
| 명세는 수동 확정 때 회전을 금지한다 | spec.md 1절 `confirmPose()` 규칙 2 "**회전은 하지 않는다**(수직은 사람이 정한 그대로)", 사람 검토 항목 1 |
| `dims().ok` 는 K-Arch 가 정한다 | `ArtifactDoc.cpp:636` `dims_ = KA::AnalyzeDimensions(...)` → `DimensionAnalyzer.cpp:223` `d.ok = d.rimDiameter > 0.0 && d.baseDiameter > 0.0 && d.vesselHeight > 0.0 && …` |
| 기울어진 구연은 K-Arch 가 구경으로 잡지 않는다 | `DimensionAnalyzer.cpp:85-111` `rimCandidateXy`: 구연 경계 고리의 z 폭이 **3.0 mm 이하**일 때만 사용. 7° 기울기면 구경 142 mm 기준 z 폭 ≈ 142·sin 7° ≈ 17.3 mm → 탈락 → `zMax − 0.5 mm` 띠 대체(기울어진 구연 꼭대기 일부 호만 남음) → 원 맞춤 실패 → `rimDiameter = 0` → `ok = false` |
| K-Arch 는 수정 금지 | spec.md "범위 밖"·영향 파일 표 하단(`GigaMesh/gui/src/dongarch/karch/*` 읽기만), 공통 규칙 |
| 재현 | 아래 명령 R1·R2 출력 `confirm as loaded (tilted 7deg): ok=0 H=200.50 rim=0.00 section=1 bboxMin.z=0` (verify.md 와 같음) |

따라서 명세의 세 조건 — (가) 예제는 7° 기울어져 들어옴, (나) 수동 확정은 회전하지 않음·"지금과 같은 계산", (다) K-Arch 수정 금지 — 을 모두 지키면서 `loadSample(false) → confirmPose()` 뒤 `dims().ok == true` 를 만들 방법이 없다. 명세 자체의 모순이며, builder 가 고칠 수 있는 구현 결함이 아니다.

### 하지 않은 우회 (모두 "추측으로 우회" 또는 명세 위반이라 거절)
- 수동 확정 때 수직 맞춤 회전 추가 → 명세 1절 규칙 2·사람 검토 항목 1 위반.
- 예제 기울기 7° → 0° 로 변경 → 기존 예제(정위치 과정 시연용)와 기준 5·11 의 전제를 바꾸는 범위 밖 변경.
- studio 쪽에서 `ok` 를 다시 정의하거나 구경을 다른 방법으로 계산 → "지금과 같은 계산" 위반, 의미 왜곡.
- 테스트를 명세 문자 그대로(`dims().ok`) 바꾸기 → CTest 실패 상태가 되어 커밋 불가(수용 기준 9·12 위반).
- 기울어진 자세 확정 시 안내·경고 추가(verify.md 선택지 나) → 설계 변경이며, 넣어도 `dims().ok` 는 여전히 false 라 기준 6 을 통과시키지 못한다.

### 결정이 필요한 것 (코디네이터·사람)
1. **명세 수정(권고)**: 기준 6 을 6a/6b 로 분리 — 6a "들어온 자세 그대로 확정 → `poseConfirmed()`·치수 계산 수행(`dims().height > 0`)·`!displaySection().empty()`·`|bboxMin.z| ≤ 1e-9`", 6b "자동 세우기 → 확정 → `dims().ok`·단면·슬롯·접지". 현재 `test_pose.cpp` 149-189행이 이미 이 형태라, 이 안이 채택되면 **코드 변경 없이** 현 HEAD `e9b655a` 그대로 기준 6 통과.
2. **설계 변경**: 수동 확정 때 수직을 자동 보정할지(사람 검토 항목 1 "수직은 사람이 정한 그대로" 를 뒤집음), 또는 기울어진 확정에 경고만 띄울지(기준 6 문구도 함께 고쳐야 함). 결정되면 그 내용을 명세에 반영한 뒤 재작업.

### 이번 회차에 실행한 명령
| # | 명령 | 종료 코드 | 결과 |
|---|---|---|---|
| R0 | `git -C D:/_dongarch_build/wt/.worktrees/JOB-003 status --porcelain` / `log --oneline -4` | 0 | 깨끗, HEAD `e9b655a` |
| R1 | (PowerShell, Qt6 bin·plugins·offscreen) `Set-Location D:\_dongarch_build\verify_job003\studio; ctest --output-on-failure` | 0 | `100% tests passed out of 13` (HEAD `e9b655a`, test-runner 빌드 그대로 — 재빌드 안 함) |
| R2 | 같은 위치 `ctest -R studio_pose -V` | 0 | `confirm as loaded (tilted 7deg): ok=0 H=200.50 rim=0.00 …` / `auto -> confirm: ok=1 H=184.80 rim=142.00 …` / `manual confirm: H=184.80 rim=142.00 …` / `PASS` |
| — | K-Arch·studio 소스 읽기(`DimensionAnalyzer.cpp:85-111,129-225`, `ArtifactDoc.cpp:261-285,612-644`) | — | 위 표 근거 |

- 바꾼 파일: 없음(제품·테스트 코드 무변경, worktree `git status` 깨끗). 이 기록 파일만 갱신.
- 경고: 해당 없음(재빌드 안 함, 직전 전체 빌드 /W4 경고 0 그대로).
- 커밋: 새 커밋 없음. 최신 SHA 그대로 `e9b655ae1b1feabba83cc39169f599dbfb6884c5`.
- **차단 사유**: 수용 기준 6 의 `loadSample(false) → confirmPose()` 뒤 `dims().ok` 는 명세 1절 "수동 확정은 회전하지 않음" + 7° 기울어진 예제 + K-Arch `AnalyzeDimensions` 의 구연 z 폭 3 mm 조건(수정 금지) 때문에 성립 불가. 명세 수정(6a/6b 분리) 또는 설계 변경을 코디네이터·사람이 정해야 진행 가능.

---

## 최초 구현 기록

## 바꾼 파일
| 파일 | 커밋 | 내용 | 이유 |
|---|---|---|---|
| `studio/src/ArtifactDoc.h` | A | `PoseAxis`, `loadMesh`, `loadSample/loadArtecBlindSample(bool confirm=true)`, `rotatePose/flipPose/resetPose/dropToFloor/autoAlign/confirmPose`, `poseConfirmed/heightAboveFloor/isGrounded`, 보조 `autoCentered()`·`groundNote()`, 멤버 `rawVertices_`·`rawLod_`·`rawLodColors_`·`poseConfirmed_`·`centered_`·`pendingAligned_/pendingNote_/pendingRmse_`, 내부 `unconfirm/rotateAbout/shiftBoth` | 명세 1절 자세 API. 자동 세우기 결과(문구·오차)는 확정 전 "바닥 미정"으로 가려지므로 확정 때 옮기려고 pending 값을 둠 |
| `studio/src/ArtifactDoc.cpp` | A | `loadMesh`(들어온 자세·원 정점 사본·원 LOD), `loadFile` 의 autoAlign 삭제, 예제 2종은 `loadMesh → (confirm 이면) autoAlign → confirmPose`, `rescaleUnits`(원 사본·현재·LOD 같은 배율, 확정 풀기), 내벽 보정 2함수 확정 전 생략, 회전(박스 중심·고정 월드축·90° 정수배 정수 행렬)·접지·처음 자세로·확정(자동 세우기 직후면 그대로, 아니면 수평 외곽 원 중심 평균→원점, 실패 시 박스 중심, 접지, LOD 재생성, 슬롯·계산), `autoAlign` 은 확정 안 함, `recompute` 확정 전 계산 생략(alignNote "바닥 미정", axisRmse −1) | 명세 1절 |
| `studio/tests/test_pose.cpp` | A(신규) | 수용 기준 1-9 (아래 "주의 1" 참고) | 새 CTest `studio_pose` |
| `studio/tests/test_pipeline.cpp` | A | STL 왕복 `loadFile` 뒤 `d2.autoAlign(); d2.confirmPose();` | 수용 기준 10 |
| `studio/CMakeLists.txt` | A | `test_studio_pose` 실행 파일·`studio_pose` 테스트 등록 | 수용 기준 12 |
| `studio/src/MainWindow.cpp` | A·B | A: 홈 「중심축 맞춤」 = autoAlign → confirmPose, openPath 임시 자동 확정. B: 탭 `① 바닥 정하기`(파일 · 바닥판에 올려놓기 · 확정), 작은 단추 툴팁, `poseConfirm` 주 단추 상태, ctx 단축키 묶음 전환(Enter 확정 · Shift −90°), 확정 전 refreshAll 분기(ctx·상태줄·접지 문구), 가드(동작 10종·단축키·되돌리기/다시), Enter=확정·Esc 무시, `confirmPose`(홈으로·맞춤·최근 기록), `recordRecent`, openPath 확정 전 열기(임시 코드 제거)·감시 폴더 문구, 예제 `poseStage` | 명세 2·3절 |
| `studio/src/MainWindow.h` | B | `openSample/openArtecBlindSample(bool poseStage=false)`, `requirePose/confirmPose/recordRecent`, `ctxCutKeys_/ctxBaseKeys_`(QVector), `pHome_/pBase_` | 명세 영향 파일 표 |
| `studio/src/View3D.h` · `View3D.cpp` | B | 확정 전 바닥판(Wash 채움 + Grid10mm 1 cm 방안 + Edge 테두리, 메시보다 먼저), 절단 계산·절단면·중심축·A/A′ 생략, (메시 박스 ∪ 바닥판) 화면 맞춤, 머리글 `3D — 바닥판(실측판)에 올려놓기`, 정보 줄 접지 문구, 확정 뒤 axisRmse < 0 이면 `—` | 명세 4절 |
| `studio/src/MeshRaster.h` · `MeshRaster.cpp` | B | `RasterView::centerX/centerY`(기본 0), `toCam` 에서 `p − center` | 명세 4절 — 기본 0 이라 확정 뒤 그림 불변(shot_home·shot_export 바이트 동일로 확인) |
| `studio/src/SheetView.cpp` | B | 확정 전 자리 문구, `fitView` 생략, 배지 `▲ 바닥 미정` | 명세 3절 |
| `studio/src/Ribbon.cpp` | B | 아이콘 `check`(2 px 단색 선) 1종, 작은 단추에 `setObjectName(id)` 1줄 | 확정 타일 아이콘(명세 허용) + 작은 단추 툴팁을 id 로 붙이기 위함(아래 "주의 4") |
| `studio/src/main.cpp` | B | `--sample|--blind --base` (확정 전 ① 페이지), 주석에 사용법 | 명세 5절 |
| `docs/ui/UI_RULES.md` | B | R1 탭 줄 `① 바닥 정하기`, ctxBar 비고, R1.4 `▲ 바닥 미정`, R3 주 단추 예 `[이 자세로 확정]` | 수용 기준 18, 정본 동기화 |

변경하지 않음: `GigaMesh/**`(core·external·karch), `studio/src/Plate.*`, `ExportDialog.*`, `StartPage.*`, `Theme.h`, `factory/scripts/*`.

## 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| `powershell -NoProfile -ExecutionPolicy Bypass -File D:/_dongarch_build/wt/.worktrees/JOB-003/factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-003 -BuildRoot D:\_dongarch_build\verify_job003 -NoShots` (A 1차) | 8 | ctest 실패 1건: 수용 기준 6 의 `dims().ok`(아래 "주의 1") |
| 같은 명령 (A 2차, 테스트 6 을 6a/6b 로 정리) | 0 | configure 0 · build 0 · ctest 13/13 |
| `git commit` (커밋 A `1ad49b4`) | 0 | |
| 같은 명령, `-NoShots` 없이 (B) | 0 | configure 0 · build 0 · ctest 13/13 · shots 0 |
| `DongArch3DStudio.exe --sample --base --shot …\shots\shot_base.png` (PowerShell, Qt6 bin·offscreen) | 0 | 70,436 B |
| `DongArch3DStudio.exe --blind --base --shot …\shots\shot_base_blind.png` | 0 | 75,223 B |
| (로컬 일회성, 커밋 안 함) main.cpp 에 Enter 주입 플래그를 임시로 넣어 `--sample --base --enter --shot` / `--sample --base --export --shot` 캡처 → 원복 | 0 | Enter → 확정 → 홈 페이지·도판·슬롯 4개 표시 확인, 확정 전 Ctrl+P 는 내보내기 창 없이 안내만 표시 확인 |
| 최종: `verify.ps1 -Root … -BuildRoot D:\_dongarch_build\verify_job003 -Clean` | 0 | configure 0 · build 0 (58/58 전체 빌드) · ctest 0 · shots 0, `verify-summary.md` 마지막 줄 `결과: PASS` |
| `cd D:\_dongarch_build\verify_job003\studio; ctest --output-on-failure` | 0 | `100% tests passed out of 13` (CMake 4.4.1 의 ctest 는 `, 0 tests failed` 없이 이 형식으로 출력) |
| 수용 기준 15 원문 라벨 grep 14개 | — | MISSING 0, `addPage(QStringLiteral("정위치·중심축"))` 0 |
| 수용 기준 16 새 색 리터럴 grep / Primary grep | — | 출력 없음 / 0 |
| 수용 기준 17 `git diff --name-only 58a33ae...HEAD` | — | 15개 파일 모두 영향 파일 표 안, GigaMesh·Plate·ExportDialog 0 |
| 수용 기준 18 `grep -c "① 바닥 정하기" docs/ui/UI_RULES.md` | — | 3 |
| 수용 기준 11 LastTest.log grep 3종 | — | 4 / 1 / 1 (≥ 1) |

테스트 출력 요지 (`studio_pose`):
- 90°×4 (X·Y·Z, ±) · 뒤집기×2: max|Δ| = 2.04e-14 mm, 매 조작 뒤 max|bboxMin.z| = 0, 박스 중심 x·y 이동 7.11e-15 mm, 처음 자세로 max|Δ| = 0
- 예제(자동 세우기→확정) H=184.80 rim=142.00 belly=165.50 base=142.00 / 눕힌 항아리(Y −74°)→자동 세우기→확정 같은 값
- 파일 열기: bbox 가 readMeshAny 와 max|Δ| 0, bboxMin.z = 31.32 mm(접지 안 함), 확정 전
- 수동 확정(곧게 선 항아리): H=184.80 rim=142.00, 배지 문구 `바닥판 기준 (사람 확정) · 중심선 원 맞춤`

캡처 비교(JOB-002 산출물): `shot_home.png`·`shot_export.png` 바이트 동일, `shot_main.png` 은 탭 이름 영역 (216,11)-(402,20)만 다름(탭 이름 변경). 캡처 파일은 `D:\_dongarch_build\verify_job003\studio\shots\` 에 있다(builder 는 기록 폴더에 build-log 외 파일을 쓰지 않으므로 artifacts 로 복사하지 않음 — test-runner 몫).

## 남은 경고
- 빌드 경고 0 (`grep -ci warning studio-logs/02_build.log` = 0, -Clean 전체 빌드 기준).
- verify.ps1 실행 시 `'vswhere.exe' … 아닙니다` 콘솔 메시지가 나오지만 vcvars64 의 기존 동작이며 종료 코드·빌드에 영향 없음(기존과 같음).

### 주의 (명세와 다르게/더 한 것 — 검토자 확인 필요)
1. **수용 기준 6 의 `confirmPose()` 뒤 `dims().ok`**: 문자 그대로(`loadSample(false)` → 곧바로 `confirmPose()`) 하면 예제가 7° 기울어진 채 확정되고(명세: 수동 확정은 회전하지 않음), K-Arch `AnalyzeDimensions` 가 정위치 메시를 전제로 zMax 부근 구연 점 20개 이상을 요구해 **구경 0 → `dims().ok == false`** 가 된다(H=200.50, 단면 1개, 접지 0). K-Arch 는 수정 금지이므로 테스트를 둘로 나눴다: **6a** 들어온 자세 그대로 확정 → 확정됨·치수 계산 수행(`height > 0`)·단면 있음·접지 1e-9, 다시 돌리면 확정 풀림·계산 비움 / **6b** 예제의 확정 경로(자동 세우기 → 확정) → 자동 세우기 직후에도 미계산, 확정 뒤 `dims().ok`·단면·슬롯·접지, 다시 돌리면 풀림. 즉 "기울어진 자세를 사람이 그대로 확정하면 구경이 —" 는 현재 설계의 결과이며, 사용자 흐름 관점에서 경고·안내가 필요한지는 사람 검토 항목으로 올린다.
2. 확정 전 `recompute()` 는 `FillMeshStats` 를 다시 돌리지 않는다. 자세 조작마다 바운딩 박스를 갱신하고(K-Arch computeBbox 와 같은 규칙), 강체 변환은 평균 변 길이·경계 유무를 바꾸지 않기 때문. 25만 면 이상 메시에서 90° 단추 한 번마다 변 표(std::map)를 다시 만드는 비용을 피하려는 것.
3. 확정 전 `axisRmse_ = −1`, 3D 정보 줄은 확정 전 접지 문구 / 확정 뒤 rmse < 0 이면 `중심축 오차 —`.
4. `Ribbon.cpp` 는 아이콘 외에 작은 단추 `setObjectName(id)` 1줄을 더했다(`Ribbon.h` 는 영향 파일 표에 없어 공개 API 를 늘리지 않고 `findChildren<QPushButton*>(id)` 로 툴팁을 붙임). 전역 QSS 에 같은 이름의 id 선택자는 없음을 grep 으로 확인.
5. 확정 전 상태줄 커서 줄(R/H/두께)은 비운다(도판 좌표가 없으므로). 확정 전 `stDirty_`(도면 저장 전 표시)도 비운다.
6. 축 색(X·Y·Z)은 명세 "범위 밖"에 따라 쓰지 않았고 `Theme.h` 토큰도 추가하지 않았다.

## 제안 (범위 밖, 하지 않음)
- 자동 세우기 경로에서 LOD 를 세 번 만든다(열기·자동 세우기·확정). 확정 때 직전이 자동 세우기면 재생성을 건너뛰면 대형 메시에서 시간 절약(결과 같음).
- 들어온 자세가 기울어진 채 「이 자세로 확정」하면 구경이 — 가 될 수 있음 → 확정 전 미리보기 또는 "기울어 보입니다 — 자동 세우기?" 안내(주의 1 과 연결, 후속 JOB 의 바닥 후보·정면 미리보기와 함께).
- 확정 뒤 배지 문구가 길면(수동 확정 + 단위·노이즈 메모) 오프스크린 창 폭이 2169 px 로 늘어난다(라벨 생략 없음, 기존 레이아웃 동작). 배지 생략(elide) 검토.
- 확정 전/뒤 3D 보조창 폭이 다르다(도판 머리·치수 띠 최소 폭 영향, 기존 R1 ≈27 % 와 차이) — 레이아웃 폭 작업(디자인 v2 3D 540 px)에서 함께.
- verify.ps1 에 `--sample --base` 캡처 추가(명세상 factory 도구 변경은 별도 작업).
- 자세 각도 표시·단축키 X·Y·Z·F·되돌리기, 확정 자세 저장 — 명세 범위 밖 목록 그대로.

## 커밋
- A `1ad49b4` feat(studio): ① 바닥 정하기 — 자세 API·확정 전 계산 금지·studio_pose 테스트 (JOB-003)
- B `e9b655ae1b1feabba83cc39169f599dbfb6884c5` feat(studio): ① 바닥 정하기 — 리본 탭·3D 바닥판·확정 전 화면·--base 캡처 (JOB-003) ← 최신

## 차단 사유 (있으면)
- 없음. 단, 위 "주의 1"(수용 기준 6 문자 그대로는 현재 설계·K-Arch 제약상 불가능, 테스트를 6a/6b 로 나눠 검사)은 사람 검토로 확인받아야 한다.
