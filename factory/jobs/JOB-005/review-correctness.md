결론: PASS

# JOB-005 정확성 검토 (correctness-reviewer)

- 대상: `D:\_dongarch_build\wt\.worktrees\JOB-005`, `factory/JOB-005`, HEAD `804f60d`, 기준 `b033aef`
- 입력: `git diff b033aef..HEAD`(10개 파일, +937/−61)를 직접 읽었고, 명세 `spec.md`(기준 13 수정 문구 포함)와 `decision.md` 를 대조했다. build-log.md 와 verify.md 의 결론은 근거로 쓰지 않았다.
- 실행(빌드하지 않음): 이미 빌드된 `D:\_dongarch_build\verify_job005\studio\test_studio_cut.exe` 를 출력 폴더만 scratchpad 로 바꿔 다시 돌렸다(CI 변수 없음, offscreen). 결과는 **마지막 줄 `PASS`, 종료 코드 0**. 수용 기준 16-22 의 grep·git 명령, 기준 14 의 LastTest 세 줄, 기준 15 의 경고 0 과 `결과: PASS` 도 직접 다시 확인했다.
- 표기: **확인** = 코드를 읽어 유도했거나 직접 실행해 본 것. **추정** = 실행하지 못한 추론.

## 1. 점검 항목별 판정

| # | 점검 | 판정 | 근거 (file:line · 출력) | 구분 |
|---|---|---|---|---|
| 1 | 정면각 부호 rotateZ(−θ)→rotateZ(θ) 의 일관성 | 이상 없음 | 회전은 `recompute` 한 곳(`ArtifactDoc.cpp:667-670`, mesh_·lodMesh_ 같은 부호)과 미리보기(`:625-626`)뿐이다. 두 곳이 같은 함수 `rotateZ`(`:21-28`, +deg = 반시계)와 같은 0 판정(`fabs>1e-9`)을 쓴다. 이 회전 결과를 받아 쓰는 곳: 도판 단면(`sliceSectionY0(mesh_)` `:845`), 3D(`View3D.cpp:40,92` → `lodMesh()`), 외면 음영(`Plate.cpp:112,139` → `mesh()`), 외곽·치수(recompute), 수평 슬롯(`horizontalSlice(mesh_)`). 슬롯 각은 `frontAngle()` 을 그대로 저장하고(`MainWindow.cpp:749,759`) `selectSlot`→`setFrontAngle`(`:618`)로 되돌린다. 내보내기 4종에는 각도 숫자가 들어가지 않는다(Plate/SheetView/ExportDialog grep). 시험 `[6] box +90°: (x,y,z)->(-y,x,z) max err 3.55e-15`, `[2] … lodDiff=0` ×12 | 확인 |
| 2 | 끌기·키·보는 방향 끌기 = "가까운 면이 오른쪽"(D5 Left 포함) | 이상 없음 | `MeshRaster.cpp:24` 는 x1=cos·px−sin·py(카메라 기준 반시계), 화면 x=+x1 이고, `:28` 깊이=−y1 이며 z-버퍼는 큰 값이 앞이다(`:99`). 따라서 가까운 쪽은 −y1 이다. 반시계 각속도 ω 에서 가까운 점의 속도는 ωr(ẑ×(0,−1,0)) = (+ωr, 0) → 화면 오른쪽이고, yaw 와 무관하다. Left 의 yaw+180(`View3D.cpp:77`)은 회전이지 거울이 아니라서 같다. 단면 끌기 `+0.5°/px`(`View3D.cpp:191`), 보는 방향 `yaw_ += 0.6°/px`(`:194`), 키 `. ] →` = +1(`MainWindow.cpp:54-60`)이 모두 반시계 = 오른쪽이다. 실제 감각은 사람 검토 9 | 확인(수식 유도), 감각은 추정 |
| 3 | 미리보기가 확정 상태를 바꾸지 않는가 | 이상 없음 | `previewFrontAngle`(`:618-632`)이 쓰는 멤버는 `previewAngle_/previewLod_/previewLines_/previewing_` 뿐이다. `sliceSectionY0` 는 멤버가 아닌 자유 함수라 멤버를 만질 수 없다(`.h` 끝 선언). `isWallEstimatedAt` 은 확정 `lines_` 를 본다(`:1039-1042`). `measureWallBySlice` 는 멤버 `lodMesh_` 를 직접 쓰고 `lodMesh()` 를 거치지 않는다(`:1060`). 시험 `[2] … frozen=same` 이 sample·blind 각 6줄이다(dims 6값, 외곽, 두께·추정 z=10…180, estWall, mesh, 슬롯 `==`) | 확인 |
| 4 | 순수 함수 `sliceSectionY0` = 옛 `buildSectionLoops` | 같음 | 옛 본문의 멤버 이름을 바꿔 넣고 새 본문과 diff 한 결과, 다른 곳은 서명 / `return s` / 씨앗 인수(`:964`) / `s.wallMm = realWall`(`:981`) 뿐이다. `realWall ≥ 2.0` 이라 `if (wallMm > 0) estimatedWallMm_ = …`(`:846-847`)는 옛 946 줄과 같은 동작이다. 0° 회귀: LastTest 세 줄 각 1개(직접 grep) | 확인 |
| 5 | 놓기 결과 == 새 문서 `setFrontAngle` (미스캔·추정 내벽 포함) | 성립 | `commitPreview`(`:634-637`)→`setFrontAngle`→`recompute`(첫 줄 `clearPreview` `:647`)는 base_/lodBase_ 에서 절대각으로 다시 계산한다. recompute 가 읽는 입력(base_, lodBase_, frontAngle_, estimatedWallMm_, inferInnerWall_, poseConfirmed_)을 미리보기가 하나도 바꾸지 않으므로 같은 이력에서는 구조상 같다. 시험 `[c] sample … 모두 0 … hasEst=0/0 raw=1197/1197`, `[c] blind … 모두 0 … hasEst=1/1 raw=1217/1217 pts=121/121`. 화면은 `cut_.target()` 로 확정한다(`MainWindow.cpp:714`) — 마지막 미리보기 뒤 입력까지 반영하는 의도된 동작 | 확인 |
| 6 | 미리보기를 지우는 곳·남기는 곳 | 명세대로 | 지우는 곳: `recompute`(`:647`), `buildSectionLoops`(`:844`, 호출처 `setInferInnerWall :371`·`setEstimatedWallMm :380`). 자세 조작·로드·단위 배율은 모두 `unconfirm`(`:473`)→recompute 를 거친다. lodBase_ 를 바꾸는 곳(`:361,399,404,444,518,598`)도 모두 뒤에 unconfirm/recompute 가 온다. 표시 옵션(smooth/simplify/side/heightRef)은 지우지 않는다. 시험 `[5]` 통과 | 확인 |
| 7 | 미리보기 중 다른 경로가 들어올 때(화면) | 꼬임 없음 | 먼저 확정하는 곳: `onAction`(`:775`, rotate·cutMove 제외), `onChip`(`:875`), 되돌리기·다시(`:254,260`), `selectSlot`(`:613`), 각도 키가 아닌 키(`:909`). 로드는 `cancelCut`(`:456,494,509`), 열기 실패 시 `commitPreview`(`:463`). Ctrl+Z 는 확정(1회 계산) 뒤 되돌린다. 엔진의 내보내기 거부(`Plate.cpp:495,516,538,742`)는 파일을 만들기 전에 막는다(`[8]` file=0 ×4). 미리보기가 켜진 채 제스처만 비활성이 되는 고아 상태는 찾지 못했다 | 확인(읽기), 실행 감각은 추정 |
| 8 | 각도 정규화·누적 오차 | 이상 없음 | `std::remainder` 는 `:610`·`:621` 두 곳이다. ±180 은 둘 다 나올 수 있다(remainder(540,360)=−180, (900,360)=+180 — Python IEEE 로 확인). 기하는 같고, `selectSlot` 의 1e-6 비교에서 불필요한 재계산이 한 번 생길 뿐이다. 360 누적이면 정확히 0(또는 −0.0)이 되어 회전하지 않으므로 0° 와 비트 단위로 같다. target 누적: 증분이 0.5°×정수 px, ±1, ±15(2진 정확)이고 base 는 remainder 결과(정확)라 반올림 누적 오차가 없다. 미리보기도 lodBase_ 에서 절대각으로 돌려 누적 오차가 없다 | 확인(수식) |
| 9 | `CutGesture` 규칙 ①-⑩ | 명세대로 | throttle 은 `CutGesture.h:76`(이미 잡혀 있으면 다시 잡지 않음), poll 은 확정을 먼저 본다(`:49-58`), nextDueMs 는 held 중 확정을 제외한다. 재실행 결과: `[11a] previews=58 first=16 commits=0`, `[11b] target=510`, `[11c] commit@540 target=2`, `[11d] keyDowns=77 commit@3300`, `[11e] target=8`, `[11f] next=-1` | 확인 |
| 10 | builder 의 명세 이탈 3건(decision.md) | 정확성상 타당 | (1) `release` 가 `held_=false`(`CutGesture.h:23`): 이것이 없으면 키를 누른 채 Ctrl+P·슬롯 저장을 할 때 flushCut 이 확정하지 못한다 → 사용자가 거부 문구를 보거나 옛 각도가 저장된다. 필요한 변경이다. (2) openPath 실패 → `commitPreview`: 이것이 없으면 cancelCut 뒤 미리보기만 남는 고아 상태가 되어, 다음 제스처가 옛 확정각에서 시작하고 내보내기가 거부된다. 다만 target 이 아니라 마지막 미리보기 각을 확정한다(16 ms 안의 입력은 버림, 경미). (3) 수식 키만 눌렀을 때 flush 하지 않음: 무해하다 | 확인(읽기) |
| 11 | 시험이 실패해야 할 때 실패하는가 | 핵심 위험은 잡음, 빈틈 있음 | 잡히는 변형: 확정 부호 되돌림([6], [2] lodDiff), 미리보기 부호만 반대([2] lodDiff), 미리보기가 estimatedWallMm 를 씀([2] blind estWall), 지우기 누락([5], [7]), debounce 함정([11a] 미리보기 0회), held 무시([11d]). 잡히지 않는 변형은 아래 2절 | 확인(읽기·실행) |
| 12 | 명세 수용 기준 ↔ 단언 1:1 | 일치 | 기준 1-12 의 각 항목이 `CHECK` 로 판정된다(허용오차 1e-9, `==`, 16/64 ms, 55-63, 16-17 ms 가 명세 값 그대로). 기준 13-22 는 직접 다시 실행했다: 17 `0/1`, 18 `0/2`, 19 `1/1/1`, 20 `4/1/1/1/1`, 21 출력 없음·`0`, 22 파일 10개 모두 §6 표에 있음·GigaMesh/docs/CLAUDE.md 출력 없음, 16 출력 없음, 14 세 줄 각 `1`, 15 경고 `0`·`결과: PASS` | 확인 |

## 2. 반례·시험 빈틈 (수용 기준은 깨지지 않음 — 비차단)

1. **M3 "미리보기가 상대각으로 돈다"(`previewLod_ = lodMesh_` 로 바꾼 변형)를 시험이 잡지 못한다.** 미리보기 시험은 모두 확정각 0° 에서만 돈다(`[2]` 12줄이 모두 `front=0`). 확정각이 0 이면 lodMesh_ 정점 == lodBase_ 정점이다(FillMeshStats 는 정점을 바꾸지 않음, `MeshLoader.cpp:135-`). 그래서 이 변형도 모든 단언을 통과한다. 실제 앱에서는 두 번째 제스처부터 미리보기가 확정각+목표각으로 두 번 돌고, 놓는 순간 단면이 튄다. 지금 구현(lodBase_ 사용, `:624`)은 맞다. 재현 제안: `setFrontAngle(30)` → `previewFrontAngle(45)` → `lodMesh()` 를 새 문서 `setFrontAngle(45)` 의 lodMesh 와 1e-9 로 비교한다. 지금 코드는 통과, M3 는 실패해야 한다. (확인: 읽기)
2. **M4 "미리보기에서 `std::remainder` 를 뺀 변형"을 잡지 못한다.** 기준 2 의 θ 6개가 모두 [−180, 180] 안이다. θ=370, −190, 540 을 넣으면 잡힌다. (확인: 읽기)
3. **기준 4 의 상자는 y→−y 거울 대칭이라 ±θ 의 Y=0 단면이 같다.** 그래서 기준 4 만으로는 단면 쪽 부호 오류를 구분하지 못한다(부호는 기준 2 의 lodDiff 와 기준 6 이 대신 잡는다). (확인: 수식)
4. 미리보기 중 `hasEstimatedInner()`·`rawPointCount()` 가 미리보기를 따르는지는 단언하지 않는다(기준 10 은 인쇄만). 이탈 (1)도 시험하지 않는다(drag 가 확정 마감을 지우는지 포함).

## 3. 고치지 않아도 되는 제안

1. `test_cut.cpp` 에 위 2절 1·2 의 두 줄 시험을 추가한다(확정각 ≠ 0 에서 미리보기, 범위 밖 θ).
2. 기준 4 에 비대칭 메시(상자를 x 로 옮기거나 한쪽 모서리를 깎은 것)를 쓰거나, 미리보기 단면을 `sliceSectionY0(lodMesh(), …)` 와 직접 비교한다.
3. `CutGesture` 시험에 "keyDown 중 release → 즉시 Commit"(이탈 1)과 "click 뒤 drag → 놓기 전에는 Commit 없음"을 추가한다.
4. 키를 누른 채 창이 비활성이 되면 `held_` 가 남는다 → `changeEvent(ActivationChange)`/`focusOut` 에서 `cut_.keyUp(now)` 를 부르는 안(추정, 재현하지 않음).
5. 제스처 중 Ctrl+Z 는 확정 계산(≈1.4 s) 뒤 되돌리기 계산(≈1.4 s)으로 두 번 멈춘다. 정확성 문제는 없고 성능만의 문제다. cancel + 미리보기 지우기로 한 번으로 줄일 수 있다.
6. openPath 실패 경로는 `cancelCut` 을 로드 성공 뒤로 미루면 `cut_.target()` 까지 정확히 확정할 수 있다(지금은 마지막 미리보기 각, 차이 ≤ 16 ms 분량의 입력).
7. `remainder(−360,360) = −0.0` 이라 ctx 각도가 `-0.0°` 로 보일 수 있다(추정, 기존 동작).
8. 3D 확대 배율·절단면 사각형이 확정 `mesh()` bbox 를 쓴다(`View3D.cpp:78-82, 120-122`). 회전체가 아닌 유물은 놓는 순간 3D 확대가 바뀐다(표시만의 문제).

## 4. 실행 기록 (재실행 원문 요약)

```
[c] sample commit(37.5) vs fresh: section=0 solid=0 est=0 outline=0 hatch=0 dims=0 wall=0 estWall=0 mesh=0 lod=0 hasEst=0/0 raw=1197/1197 pts=79/79
[c] blind  commit(37.5) vs fresh: … 모두 0 … hasEst=1/1 raw=1217/1217 pts=121/121
[4] box θ=12 / 137: section diff=0      [6] box +90° max err 3.55e-15
[d] sample preview lodFaces=26180 fullFaces=77120 median=0.69 p90=0.76 max=0.88 ms limit=16 ms (local)
[d] sample full median=1393.5 ms   [d] blind full median=937.0 ms   3D render 14.61 / 12.25 ms
lod-vs-full blind θ=45 hausdorff=7.053 mean=0.739 mm (다른 9줄 0.738–2.562)
[11a] previews=58 first=16 commits=0 … [11f] cancel → next=-1
PASS  (EXIT=0)
```
명령: PowerShell, `Remove-Item Env:\CI; PATH/QT_PLUGIN_PATH=OSGeo4W Qt6; QT_QPA_PLATFORM=offscreen; & D:\_dongarch_build\verify_job005\studio\test_studio_cut.exe <scratchpad>\cutrun`

## 5. 차단 사유

없다. 수용 기준 1-22 는 직접 확인했고, 23(캡처)은 ui-reviewer 몫이다. 정확성 관점의 결함은 찾지 못했다. 2절의 시험 빈틈은 명세가 요구한 단언 밖이므로 비차단이다.
