# JOB-005 명세 — ② 막으로 자르기 004b: **실시간 단면** (끄는 동안 LOD 단면 1장, 놓으면 원본 전체 계산)

작성: spec-writer, 2026-10-17. 기준 커밋 `b033aef`(factory/JOB-005 = JOB-004 끝, worktree `D:/_dongarch_build/wt/.worktrees/JOB-005`, **JOB-003·JOB-004 승인 전제**).
아래 경로·줄 번호는 모두 이 worktree 에서 직접 읽고 Grep 으로 확인한 값이다. 이하 `W` = `D:/_dongarch_build/wt/.worktrees/JOB-005`, `B` = `D:\_dongarch_build\verify_job005\studio`(빌드 폴더), `L` = `D:\_dongarch_build\verify_job005\studio-logs`(verify.ps1 로그 폴더).
출발점: JOB-004 명세 §10 후속 조각 1(004b 씨앗)과 §8 사람 검토 2·3.

## 0. 분할 판단

씨앗 범위 그대로 세면 **제품 ≈ 260줄 + 테스트 ≈ 320줄**(§9)이다. 제품 코드는 300줄 지침 안이라 **더 나누지 않는다**. 테스트가 큰 것은 엔진 같음 비교(두 예제 × 단면 5종·치수·두께·정점)와 시간 합치기 상태 기계 시험을 한 실행 파일에 담기 때문이다. builder 실측이 제품 300줄을 넘으면 그 자리에서 멈추고 **005a(엔진·내보내기 거부·수용 기준 1-10)** / **005b(입력 합치기·화면 연결·수용 기준 11)** 로 나누자고 코디네이터에게 알린다.

## 1. 목표

사용자 요청(job.json `request`, 코디네이터 릴레이):
> 사용자 08:03 핵심 요구(얇은 막 절단을 3D 뷰에서 보며 오른쪽 도판에 단면 실시간 표시), 09:39 "승인개발진행", 10:59 "전부 다되면 개발진행하라". … 코디네이터 기본값: 끄는 동안은 단면 1장만 계산하고 두께·치수는 놓은 뒤 계산.

설계 발췌(design-excerpt.md 원칙 1·3): "막은 보는 사람 앞에 고정되고 유물이 회전판 위에서 돈다." "끄는 동안 3.5만 면 미리보기(LOD)로 단면 1장만 다시 계산하고, 놓으면 원본으로 다시 계산한다. 두께·치수는 놓은 뒤 계산."

**재진술(한 문장)**: 단면 도구로 3D 를 끄는 동안(또는 키·단추로 각도를 바꾸는 동안) 유물이 +Z 축 회전판처럼 돌고, 오른쪽 도판의 단면과 3D 절단선이 LOD(≤ 3.5만 면) Y=0 단면 1장으로 16 ms 단위로 바뀌며, 손을 놓으면(키·단추는 마지막 입력 300 ms 뒤, 누르고 있는 동안은 보류) 원본 메시로 단면·외곽·치수·두께를 한 번 다시 계산한다 — 끄는 동안 치수·두께·외곽·원본 메시·확정 정면각은 바뀌지 않는다.

## 2. 범위 밖

- `② 막으로 자르기` 리본 페이지·탭 이름·그룹(`막 방향 (누르면 바로 맞춤)` [Y 막][X 막] 단추 포함)·ctx 원문 라벨(`✂ 막으로 자르기` …)·상태줄 원문(`⑥ 단면 보기 — …`)·도판 머리 `A–A′ 실시간 단면 — 12°`·잠김 안내·`--cut` 캡처 → **JOB-006(004c)**. 이번에는 `alignCut` **엔진 함수와 시험만** 만든다.
- `지금 각도 고정`·`도면 편집으로` 넘기기·기준선 레이어 → **JOB-007**.
- Z 막, 각도 띠, 추천, 한 바퀴 훑기, 앞쪽 반 흐리게, 보라 점, 실시간 확인 줄, 회전판 빨간 손잡이 그리기, 중심선 고정 풀기(막 밀기·"보고서 단면 아님" 경고), ↑ ↓ 1 mm, V 도판 방향 보기 → 그 뒤.
- 끄는 동안 외면 음영·외곽선·치수 띠·두께 풍선·미스캔 추정 구간 갱신(놓은 뒤 갱신 — 코디네이터 기본값). 수평 슬롯(횡단면)을 보는 중의 도판 평면 단면 실시간 갱신(놓은 뒤 갱신, 3D 는 따라감).
- 원본 계산의 백그라운드 스레드화(놓는 순간 UI 가 원본 계산 시간만큼 멈춘다 — 측정·기록만).
- 정면각 규약을 `CLAUDE.md` §2 에 적는 일(사람 검토 1 승인 뒤 별도), `docs/ui/**` 변경, 새 `Theme.h` 색 토큰(축 색은 UI_RULES R2 변경 — 사용자 승인 대상).
- 기존 별건: 모달 내보내기 중 감시 폴더 자동 로드(JOB-004 review-security 권고, `MainWindow.cpp:70-76`·`:684-692`), Left 상태줄 커서 R 부호, 3D 반대쪽 파선 색 → 백로그 후보.
- `GigaMesh/core`·`GigaMesh/external`·`GigaMesh/gui/src/dongarch/karch/*` 수정. `factory/**` 기록 갱신(오케스트레이터 몫).

## 3. 설계 결정 (builder 가 그대로 구현할 기본값)

### 3.1 정면각 부호·0° 기준 (결정 — 사람 검토 1)

**기본값: 정면각 θ > 0 = 위(+Z)에서 보아 반시계 = +Z 축 오른손 법칙.** 0° = 확정 자세 그대로(막 = 확정 자세의 Y=0 면, 정면 = −Y 에서 봄). 범위 `std::remainder(θ, 360)` ∈ [−180, 180] 그대로.

근거(코드 줄):
| 위치 | 지금 동작 |
|---|---|
| `ArtifactDoc.cpp:21-28` `rotateZ(m, deg)` | x′ = c·x − s·y, y′ = s·x + c·y → +deg = 위에서 보아 반시계 |
| `ArtifactDoc.cpp:636-639` `recompute` | `rotateZ(mesh_, -frontAngle_)`·`rotateZ(lodMesh_, -frontAngle_)` → **지금 θ+ = 시계** |
| `View3D.cpp:190-192` + `MainWindow.cpp:106` | 단면 도구 오른쪽 끌기 = `cutAngleDragged(+0.5°/px)` → θ+ → 시계 → **가까운(정면) 면이 화면 왼쪽** |
| `View3D.cpp:194` + `MeshRaster.cpp:21-24` | 보는 방향 오른쪽 끌기 = `yaw_ += 0.6°/px`, `toCam` x1 = cos·px − sin·py(세계를 카메라 기준 반시계) → **가까운 면이 화면 오른쪽** |
| `ArtifactDoc.cpp:409-426`(Z: 423-424) + `MainWindow.cpp:209` | ① 자세 회전은 오른손 법칙, 툴팁 "Z 축 … +90° 돌립니다 (위에서 보면 반시계)" |
| `MainWindow.cpp:882-885` | `,` `[` = −1°, `.` `]` = +1° |

수학: 카메라가 어느 수평 방향 d 에 있든(보는 방향 yaw, D5 Left 의 +180° `View3D.cpp:77` 포함) 반시계 회전에서 가까운 점의 속도는 θ̇·r·(ẑ × d) = θ̇·r·(화면 오른쪽 벡터)다. 그래서 반시계를 + 로 두면 **오른쪽 끌기 · → · `.` · `]` · 보는 방향 오른쪽 끌기가 모두 "가까운 면이 오른쪽으로" 같은 감각**(디자인 "회전판 손잡이")이 되고, 앱 안 회전 규약이 하나(오른손 법칙)로 맞는다.

구현: `recompute` 636-639 의 `-frontAngle_` → `frontAngle_`(두 줄). 미리보기도 **같은 함수·같은 부호·같은 0 판정**(`std::fabs(θ) > 1e-9`)을 쓴다. 끌기 비율 `+0.5°/px`(View3D.cpp:191)와 키 부호(882-885)는 그대로 둔다. `ArtifactDoc.h:81` 주석 `// 정면 회전 (, .)` 을 부호 규약 문장으로 바꾼다.
영향: 0° 결과는 그대로(기존 시험·`--shot` 캡처·LastTest 값 불변 — `studio/tests` 에 0° 아닌 `setFrontAngle` 호출 없음, Grep 확인). 세션 안에 저장한 슬롯 각(`MainWindow.cpp:663-682`)·되돌리기 스택의 각은 같은 숫자가 반대 방향을 뜻하게 되나 프로젝트 파일 저장이 없어 세션 밖 영향은 없다.
대안 (나) "엔진은 두고 끌기 부호만 뒤집기"는 채택하지 않는다 — `[+]`·→ 가 화면에서 반대로 돌게 된다.

**막 방향(엔진만)**: `alignCut(CutAxis::Y)` = `setFrontAngle(0)`, `alignCut(CutAxis::X)` = `setFrontAngle(90)`. 위 부호에서 X 막 +90° 는 "유물의 원래 −X 쪽(정면에서 보아 왼쪽 옆)이 보는 사람을 향하고, 도판 오른쪽 단면 = 원래 X=0 면의 정면(−Y) 쪽 반"이다(사람 검토 2).

### 3.2 엔진 API (`studio/src/ArtifactDoc.h/.cpp`)

```cpp
// ② 막으로 자르기 — 막은 보는 사람 앞 Y=0 면에 고정, 유물이 +Z 회전판처럼 돈다
enum class CutAxis { Y, X };  // Y 막 = 정면에서 본 종단면(0°), X 막 = 옆에서(90°). Z 막은 뒤 작업

  // 정면각 θ (deg, [−180, 180]): 확정 자세를 +Z 오른손 법칙으로 θ 돌린(위에서 보아 반시계 —
  // 정면(−Y)의 면이 화면 오른쪽 +X 로 가는) 메시의 Y=0 면 x ≥ 0 반쪽이 단면. 0° = 확정 자세.
  void setFrontAngle(double deg);   // 확정 전·미로드는 무시(새 가드). 원본 전체 다시 계산
  void alignCut(CutAxis a);         // = setFrontAngle(a == CutAxis::Y ? 0.0 : 90.0)
  // 실시간 미리보기: 확정 전·미로드 무시. LOD 만 돌려 Y=0 단면 1장 → 미리보기 버퍼.
  // 외곽·치수·두께·estimatedWallMm·슬롯·원본 메시·frontAngle 은 바꾸지 않는다.
  void previewFrontAngle(double deg);
  void commitPreview();             // 미리보기 중이면 setFrontAngle(previewAngle()), 아니면 아무것도 안 함
  bool isPreviewing() const;
  double previewAngle() const;      // 미리보기 중이 아니면 frontAngle()
```

- `setFrontAngle`(607-610) 첫 줄에 `if (!isLoaded() || !poseConfirmed_) return;` — 지금은 확정 전에도 `frontAngle_` 를 바꾼다(MainWindow 가드 657-661 에만 의존).
- `previewFrontAngle(θ)`: `θn = std::remainder(θ, 360)` → `previewLod_ = lodBase_`(면·통계 복사) → `std::fabs(θn) > 1e-9` 이면 `rotateZ(previewLod_, θn)` → `updateBbox(previewLod_)`(60-80) → 순수 함수로 단면 1벌(§3.3) → `previewing_ = true`. **`FillMeshStats` 금지**(std::map 변 표, `MeshLoader.cpp:135-` — 미리보기 시간 대부분을 먹는다; `KA::Slice` 는 통계를 쓰지 않고 bbox 도 스스로 잰다 `MeshSlicer.cpp:25-40, 102-104`). `lodMesh_` 가 아니라 `lodBase_` 에서 매번 절대각으로 돌린다(누적 오차 없음).
- 미리보기 중 **따라 바뀌는 것**: `displaySection/ScannedSolid/EstimatedInner/Hatch`(702-748 의 `…Natural()` 이 읽는 단면 버퍼), `hasEstimatedInner()`(h:102), `rawPointCount()`(1054-1059)·`pointCount()`, **`lodMesh()`**(h:110-112 → 미리보기 중이면 `previewLod_`; 3D 보조창은 이것으로 그리고 자르므로 `View3D.cpp:40, 44, 92` 가 자동으로 따른다).
- 미리보기 중 **그대로인 것**: `displayOutline`, `dims/rawDims/silhouette/section`, `wallThicknessAt`·`isWallEstimatedAt`(1003-1021, 두께는 놓은 뒤), `estimatedWallMm`, `mesh()`, `horizontalSlice`, `frontAngle`, `slotList`, `lodVertexColors`(정점 순서 같음).
- 미리보기를 지우는 곳: `recompute()` 첫 줄(→ `setFrontAngle`·`alignCut`·`commitPreview`·`unconfirm`(473-477, 로드·자세 조작·단위 배율 모두 거침)·`confirmPose`), `buildSectionLoops()`(→ `setInferInnerWall` 376, `setEstimatedWallMm` 383). 표시 옵션(`setSmooth/setSimplifyTol/setSectionSide/setHeightRef`)은 지우지 않는다 — 미리보기 단면에도 그대로 적용.

### 3.3 함정: `buildSectionLoops` 의 상태 오염 → 순수 함수로 분리

`buildSectionLoops`(811-997)는 단면을 계산하면서 멤버를 바꾼다: 812-816(고리·실선·추정선·`estZMin_/Max_` 비움), 929(`estimatedWallMm_` 를 씨앗으로 읽음), **946(`estimatedWallMm_ = realWall`)**, **972-973(`estZMin_/estZMax_`)**. 미리보기가 이 함수를 부르면 놓은 뒤 결과가 달라진다(`recompute` 675-677 이 다음 계산의 씨앗으로 다시 쓴다). 그래서:

```cpp
struct SectionLines {  // Y=0 단면 한 벌 (자연 좌표: x ≥ 0 = 단면 쪽, K-Arch 좌표)
  std::vector<std::vector<KA::Point2>> loops, solids, estInner;
  double estZMin = -1.0, estZMax = -1.0;
  double wallMm = -1.0;  // 미스캔 보정 경로(899 이후)가 정한 기벽 두께. 그 경로가 아니면 −1
};
// 멤버를 읽지도 쓰지도 않는 순수 함수 — 지금 811-997 본문을 그대로 옮기고 멤버 대신 인수·반환값
SectionLines sliceSectionY0(const KA::MeshData& m, bool inferInner, double wallSeedMm);
```

- `buildSectionLoops()` = `s = sliceSectionY0(mesh_, inferInnerWall_, estimatedWallMm_)` → 멤버에 적용, `if (s.wallMm > 0) estimatedWallMm_ = s.wallMm;`(그 경로의 `realWall` 은 항상 ≥ 2.0 이라 946 과 정확히 같은 동작).
- 미리보기 = `sliceSectionY0(previewLod_, inferInnerWall_, estimatedWallMm_)`, 결과의 `wallMm` 은 버린다(씨앗은 지금 확정값 — 사람 검토 4).
- 멤버 묶기(`sectionLoops_/scannedSolidLines_/estimatedInnerLines_/estZMin_/estZMax_`(h:172-178) → `SectionLines lines_` + `previewLines_` + `const SectionLines& shown() const`)는 권장, 이름은 builder 재량. 0° 출력·LastTest 값(수용 기준 14)은 한 글자도 바뀌면 안 된다.

### 3.4 입력 합치기 `CutGesture` (`studio/src/CutGesture.h` 신규, Qt 무의존, 시각 ms 를 밖에서 넣음 → 시험 가능)

```cpp
class CutGesture {
public:
  static constexpr long long kPreviewMs = 16;   // 미리보기 합치기 (≈ 60 fps)
  static constexpr long long kCommitMs = 300;   // 키·단추 마지막 입력 뒤 확정
  enum class Due { None, Preview, Commit };
  void drag(double baseDeg, double deltaDeg, long long nowMs);
  void release(long long nowMs);
  void keyDown(double baseDeg, double deltaDeg, long long nowMs);  // 첫 누름·자동 반복 모두
  void keyUp(long long nowMs);                                     // 자동 반복이 아닌 뗌만
  void click(double baseDeg, double deltaDeg, long long nowMs);    // 리본 단추
  void cancel();
  Due poll(long long nowMs);        // 마감이 지난 일 하나
  long long nextDueMs() const;      // 다음 마감, 없으면 −1
  bool active() const;              // 시작 ~ 확정 사이
  double target() const;            // 목표각(누적, 정규화 안 함)
};
```

규칙: ① 비활성에서 첫 입력이 `target = baseDeg` 로 시작(활성 중 base 무시) ② 모든 입력 `target += delta` ③ **미리보기 마감은 비어 있을 때만 `now + 16` — 이미 잡혀 있으면 다시 잡지 않는다(throttle)**. 참고 코드 `GigaMesh/gui/src/dongarch/cutline/CutlineController.cpp:23-26` 의 16 ms 는 맞지만 `:159-161` 처럼 요청마다 타이머를 다시 시작(debounce)하면 계속 끄는 동안 한 번도 그리지 않는다 ④ drag: 확정 마감 지움 ⑤ keyDown: `held = true`, 확정 마감 지움 ⑥ keyUp: `held = false`, 확정 마감 `now + 300`(debounce) ⑦ click: ①-③ + 확정 마감 `now + 300` ⑧ release: 활성이면 확정 마감 `now`, 미리보기 마감 지움 ⑨ poll: 활성·`!held`·확정 마감 ≤ now → 비활성으로, 마감 모두 지우고 `Commit`(target 은 계속 읽힘); 아니면 미리보기 마감 ≤ now → 지우고 `Preview`; 아니면 `None` ⑩ cancel: 모두 비움. 300 ms 근거: 씨앗(KLM 평균 타건 0.28 s 보다 길게).

### 3.5 화면 연결

- **View3D**(`View3D.h:19-20`, `View3D.cpp:200-203`): 새 신호 `void cutDragReleased();` — `mouseReleaseEvent` 에서 `drag_ && cutTool_` 이었으면 내보낸다. 끌기 비율·`cutAngleDragged` 는 그대로.
- **MainWindow**(`MainWindow.h/.cpp`): 멤버 `CutGesture cut_`, 단발 `QTimer* cutTimer_`, `QElapsedTimer cutClock_`(생성자에서 start). 도우미:
  - `scheduleCut()`: `nextDueMs()` < 0 이면 타이머 정지, 아니면 `max(0, due − now)` ms 로 시작.
  - `onCutTimer()`: `poll(now)` 를 `None` 까지 반복 — `Preview` → `doc_.previewFrontAngle(cut_.target()); refreshPreview();`, `Commit` → `doc_.setFrontAngle(cut_.target()); refreshAll(true);` — 뒤 `scheduleCut()`. **`previewFrontAngle` 호출은 MainWindow 안에서 이 한 곳뿐.**
  - `refreshPreview()`: `sheet_->refresh(doc_, slot_)`, `sheet_->canvas()->update()`, `view3d_->refresh(doc_, slot_)`, `ctxRight_` 의 각도 숫자를 `previewAngle()` 로(문구는 그대로). **`docChanged()`·`refreshAll`·`refreshSlots` 를 부르지 않는다** — 음영(`SheetView.cpp:38-42, 87-90`)은 놓은 뒤 `refreshAll(true)`(502)에서만 다시 그린다.
  - `beginCutInput()`: `if (!cut_.active()) pushUndo();` — 제스처 하나 = 되돌리기 한 칸(지금은 끌기가 되돌리기에 안 쌓임, 106).
  - `flushCut()`: 활성이면 `cut_.release(now); onCutTimer();`(즉시 확정). `cancelCut()`: `cut_.cancel(); cutTimer_->stop();`.
- 입력 연결:
  - 끌기(106): 확정 전이면 무시, `beginCutInput(); cut_.drag(doc_.frontAngle(), d, now); scheduleCut();`. 놓기: `cutDragReleased` → `cut_.release(now); onCutTimer();`.
  - 키(882-885 를 바꾸고 ← → 추가): `,` `[` `←` = −, `.` `]` `→` = +, 크기 1°, **← → 만 Shift 면 15°**(`,` `.` `[` `]` 의 Shift 조합은 다른 키 코드가 되므로 지원 안 함). `beginCutInput(); cut_.keyDown(doc_.frontAngle(), ±step, now); scheduleCut();`(자동 반복 포함).
  - 새 `keyReleaseEvent`: `!e->isAutoRepeat()` 이고 위 각도 키면 `cut_.keyUp(now); scheduleCut();`.
  - 리본 `rotate`·`cutMove`(751-758): `beginCutInput(); cut_.click(doc_.frontAngle(), +1.0, now); scheduleCut();`.
  - **다른 동작 전에 먼저 확정**: `onAction`(rotate·cutMove 제외 — 내보내기 Ctrl+P·`svg`·`dxf` 포함), `onChip`, 되돌리기/다시(227-236), `selectSlot`(572), 각도 키가 아닌 `keyPressEvent`(1~5 슬롯 저장·R·W·− + 포함) 첫머리에 `flushCut()`. 로드(`openPath` 426, `openSample` 459, `openArtecBlindSample` 473) 첫머리는 `cancelCut()`(문서가 바뀌므로 계산하지 않음).
  - 확정 전 가로채기(843-846)에 `Qt::Key_Left, Qt::Key_Right` 추가.
  - R8(단축키는 kbd 배지): ctx 단면 도구 묶음(256-261)에 `kbdLabel("← →", w), new QLabel(QStringLiteral("막 1° · Shift 15°"), w)` 2개.
  - `0°` 이 아닌 모든 각도 입력이 `CutGesture` 를 거치므로 `MainWindow::setFrontAngle`(657-661, h:62)은 쓰이지 않게 된다 → 지운다(남겨도 되나 `setFrontAngle(doc_.frontAngle()` 꼴 호출은 0개여야 함).

### 3.6 미리보기 중 내보내기 거부 (`studio/src/Plate.cpp`)

익명 이름공간 도우미 `bool refusePreview(const ArtifactDoc&, QString* err)` — `doc.isPreviewing()` 이면 `*err = QStringLiteral("막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.")`, `true`. `exportPdf`(484, `QPdfWriter` 만들기 전), `exportPng`(502), `exportSvgLayered`(522, 524 의 `QFile::open` 전), `exportDxf`(725, 727 의 `Open` 전) 첫 줄에서 부른다 — **파일이 생기지 않는다**. 화면은 내보내기 전에 `flushCut()` 으로 먼저 확정하므로 사용자는 거부를 보지 않는다(엔진 보호 + 시험용).

### 3.7 빌드 (`studio/CMakeLists.txt`)

- `studio_core`(55-60)에 `src/CutGesture.h` 추가.
- 96 다음: `add_executable(test_studio_cut tests/test_cut.cpp)`, `target_link_libraries(test_studio_cut PRIVATE studio_core)`, `add_test(NAME studio_cut COMMAND test_studio_cut ${CMAKE_CURRENT_BINARY_DIR})`, `set_tests_properties(studio_cut PROPERTIES RUN_SERIAL TRUE)` — CI 는 `ctest --parallel 4`(`.github/workflows/studio-ci.yml:65`)라 시간 측정이 다른 시험과 겹치지 않게(JOB-004 review-security 권고).

## 4. 수용 기준 (각 항목은 명령 하나로 참/거짓)

허용오차 근거(새로 만든 값 없음): 기하 같음 **1e-9 mm**(K-Arch `test_synthetic.cpp`·`test_dimensions.cpp`, JOB-004 와 같은 값), 바뀌면 안 되는 값은 **정확히 같음(`==`)**(계산을 다시 하지 않으므로), 미리보기 시간 **16 ms**(60 Hz 한 프레임, `CutlineController.cpp:23` 주석 "16ms = ~60fps"; 디자인 노트 실측 3.5만 면 단면 1장 0.8 ms), CI 공유 러너는 ×4 = **64 ms**(JOB-004 review-security 권고 — 판정 끄기 대신 배수).

### A. 새 CTest `studio_cut` (`studio/tests/test_cut.cpp`, 실행 파일 `test_studio_cut`, 인자 = 출력 폴더, 오프스크린 `QGuiApplication`)

공통: 예제 S ∈ {`loadSample()`, `loadArtecBlindSample()`}(확정 상태, 기본 옵션: 간략화 0.5 mm·곡선 켬·Right). **상자** = 정점 V0(−20,−10,0) V1(20,−10,0) V2(20,10,0) V3(−20,10,0) V4(−20,−10,20) V5(20,−10,20) V6(20,10,20) V7(−20,10,20) mm, 면 (0,2,1)(0,3,2)(4,5,6)(4,6,7)(0,1,5)(0,5,4)(2,3,7)(2,7,6)(1,2,6)(1,6,5)(3,0,4)(3,4,7) — Z 축 180° 회전에 대칭인 삼각분할(확정의 중심선 이동이 0 이 되게). `loadMesh(상자, {}, "상자")` → `confirmPose()`. 12면 ≤ 35,000 이라 LOD = 원본 사본(`MeshIO.cpp:969-972`). 문서 A = 미리보기용, B = 같은 예제를 새로 열어 `setFrontAngle` 한 번만, C = 같은 예제를 새로 열어 여러 각을 차례로 `setFrontAngle`.

1. **(e) 확정 전·미로드 무시**: (i) 새 `ArtifactDoc`(미로드)에서 `previewFrontAngle(30)`·`commitPreview()`·`alignCut(CutAxis::X)`·`setFrontAngle(30)` 뒤 죽지 않고 `isPreviewing()==false`, `frontAngle()==0.0`. (ii) `loadSample(false)`(확정 전)에서 같은 네 호출 뒤 `isPreviewing()==false`, `frontAngle()==0.0`, `previewAngle()==0.0`, `displaySection()` 비어 있음, `mesh()`·`lodMesh()` 정점이 호출 전과 `==`.
2. **(c-1) 끄는 동안 불변 · 미리보기는 따라감**: 각 S 에서 확정값을 저장한 뒤 θ = 5, 12, −20, 90, 137, 37.5 순으로 A 에 `previewFrontAngle(θ)` 할 때마다 — `isPreviewing()==true`, `previewAngle()==std::remainder(θ,360.0)`, `frontAngle()==0.0`; `dims()` 6값·`displayOutline()` 모든 점·`wallThicknessAt(z)`·`isWallEstimatedAt(z)`(z = 10, 20, …, 180 mm)·`estimatedWallMm()`·`mesh()` 모든 정점·`slotList()`(크기·`angleDeg`·`z`)가 저장값과 `==`; `displaySection()` 비어 있지 않음; **`lodMesh()` 정점이 C 에 `setFrontAngle(θ)` 한 `lodMesh()` 정점과 모두 ≤ 1e-9 mm**(3D 가 미리보기 각을 따른다).
3. **(c-2) 놓기 == 새 문서 `setFrontAngle`**: 2 의 끝에서 A `commitPreview()` → `isPreviewing()==false`, `frontAngle()==37.5`. B(새로 열어 `setFrontAngle(37.5)` 만)와 비교해 `displaySection/ScannedSolid/EstimatedInner/Outline/Hatch` 의 고리·점 수가 같고 모든 좌표 차 ≤ 1e-9 mm, `dims()` 6값 차 ≤ 1e-9 mm, `wallThicknessAt(z)`(z = 10…180) 차 ≤ 1e-9 mm, `estimatedWallMm()` 차 ≤ 1e-9 mm, `hasEstimatedInner()`·`rawPointCount()`·`pointCount()` 같음, `mesh()`·`lodMesh()` 정점 차 ≤ 1e-9 mm. 항목별 최대 차를 `[c] <예제> …` 한 줄로 인쇄.
4. **(c-3) 같은 메시면 같은 단면**: 상자(LOD = 원본)에서 θ = 12°, 137° 각각 `previewFrontAngle(θ)` 의 `displaySection()` 이 새 상자 문서 `setFrontAngle(θ)` 의 `displaySection()` 과 고리·점 수가 같고 차 ≤ 1e-9 mm(미리보기 순수 경로 = 확정 경로).
5. **확정값을 바꾸면 미리보기를 지움**: `loadSample()` 문서에서 매번 `previewFrontAngle(30)` 을 새로 한 뒤 (i) `setEstimatedWallMm(5.0)`(이때 `inferInnerWall()==true`) (ii) `setInferInnerWall(false)` (iii) `setFrontAngle(10)`(→ `frontAngle()==10.0`) (iv) `loadSample()` 각각 뒤 `isPreviewing()==false`. `setSmooth(false)`·`setSimplifyTol(0.3)`·`setSectionSide(SectionSide::Left)` 뒤에는 `isPreviewing()==true` 유지.
6. **각도 부호(+ = 위에서 보아 반시계)**: 상자에서 0° 정점 (x⁰, y⁰, z⁰) 를 저장하고 `setFrontAngle(90)` 뒤 `mesh()` 8정점 모두 x = −y⁰, y = x⁰, z = z⁰ (≤ 1e-9 mm) — 정면(y = −10)의 면이 +X(x = +10)로 간다.
7. **(f) 막 맞춤**: 상자 확정 직후 bbox 중심 |x|·|y| ≤ 1e-9 mm(인쇄). `setSmooth(false)` 에서 `alignCut(CutAxis::Y)` → `frontAngle()==0.0`, `displaySection()` 최대 x = 20 mm·최소 x = 0 mm(각 ≤ 1e-9); `alignCut(CutAxis::X)` → `frontAngle()==90.0`, 최대 x = 10 mm(≤ 1e-9). 상자 0° 에서 `previewFrontAngle(90)` 중 `displaySection()` 최대 x = 10 mm(≤ 1e-9), `mesh().bboxMax.x` = 20 mm(≤ 1e-9, 원본 그대로), 그 상태에서 `alignCut(CutAxis::Y)` 뒤 `isPreviewing()==false`.
8. **미리보기 중 내보내기 거부**: `loadSample()` 문서 `previewFrontAngle(20)` 중 `exportSvgLayered`·`exportPdf`·`exportPng`(150 dpi)·`exportDxf`(기본 `ExportRequest`, 경로 = 출력 폴더 `cut_refuse.svg/.pdf/.png/.dxf`, 미리 지움)가 각각 `false`, `err` 에 `놓은 뒤` 포함, 파일이 존재하지 않음. `commitPreview()` 뒤 `exportSvgLayered` 는 `true`, 파일 크기 > 0.
9. **(d) 시간**: `loadSample()` 확정 문서에서 `previewFrontAngle(θᵢ)` + `displaySection()` 1회를 θᵢ = 3.7·i° (i = 1…51) 로 51회 `std::chrono::steady_clock` 측정(i = 0 은 예열, 제외). **중앙값 ≤ 16 ms**(환경 변수 `CI` 가 있으면 ≤ 64 ms). 인쇄 `[d] sample preview lodFaces=… fullFaces=… median=… p90=… max=… ms limit=16 ms (local)`(CI 면 `limit=64 ms (CI)`), 원본 `setFrontAngle` 3회(θ = 10, 20, 30) 중앙값 `[d] sample full …`, 3D 한 장 `renderMesh(lodMesh(), 420×640, ShadeMode::Real)` 11회 중앙값 `[d] sample 3D render …` — 뒤 둘과 미스캔 예제의 같은 줄은 기준 없음(기록만).
10. **LOD↔원본 거리(기록만)**: 두 예제 × θ ∈ {0, 12, 45, 90, 137}° 에서 A 의 `previewFrontAngle(θ)`·`displaySection()` 과 C 의 `setFrontAngle(θ)`·`displaySection()` 사이 양방향 최대 최근접 거리(각 점 → 상대 닫힌 꺾은선의 선분)와 평균을 `lod-vs-full <예제> θ=… hausdorff=… mean=… mm hasEst preview/full=…/…` 로 **10줄** 인쇄. 판정은 두 단면이 비어 있지 않고 값이 유한한 것만(공식 허용오차 없음 — 사람 검토 3).
11. **입력 합치기 `CutGesture`**(시각은 정수 ms):
    - a. 끌기 합치기: base 10°, t = 0…999 매 1 ms 마다 `drag(+0.5)` 다음 `poll(t)` → 첫 `Preview` 가 t = 16, 이웃 `Preview` 간격 모두 16–17 ms, `Preview` 55–63번, `Commit` 0번.
    - b. 놓기 즉시: 이어서 `release(1000)` → `poll(1000) == Commit`, `target() == 510.0`, `active()==false`, `poll(5000) == None`.
    - c. 키 300 ms: base 0, `keyDown(+1,0)`·`keyUp(40)`·`keyDown(+1,200)`·`keyUp(240)`, `poll` 을 0…539 매 1 ms → `Commit` 0번(`Preview` ≥ 1번), `poll(540) == Commit`, `target() == 2.0`.
    - d. 누르고 있는 동안 보류: base 0, `keyDown(+1)` 을 t = 0 과 500 ≤ t ≤ 3000 에서 33 ms 마다(자동 반복), `keyUp` 없이 `poll` 0…3000 매 1 ms → `Commit` 0번; `keyUp(3000)` 뒤 `poll(3299) != Commit`, `poll(3300) == Commit`, `target()` = `keyDown` 횟수.
    - e. 단추: base 5, `click(+1)` 을 0·100·200 → `poll(499) != Commit`, `poll(500) == Commit`, `target() == 8.0`.
    - f. 확정이 먼저·취소: `click(+1,0)` 뒤 `poll(400) == Commit`, 이어 `poll(401) == None`. `drag(+1,0)` 뒤 `cancel()` → `active()==false`, `poll(1000) == None`, `nextDueMs() == -1`.
12. `test_studio_cut` 은 실패가 없으면 마지막 줄 `PASS`, 종료 코드 0.

### B. 회귀

13. CTest 15종(기존 14 + `studio_cut`): `B` 에서 `ctest --output-on-failure` 출력에 `100% tests passed, 0 tests failed out of 15`.
    - **(오케스트레이터 수정, decision.md)**: 이 PC·CI 의 CTest 는 실패가 없을 때 `100% tests passed out of 15` 처럼 `, 0 tests failed` 없이 찍는다(JOB-004 CI 로그 같음). 판정은 `100% tests passed` 와 `out of 15` 가 같은 줄에 있고 `tests failed` 가 0 이 아닌 줄이 없는 것으로 한다.
14. 지금과 같은 값(verify.ps1 직후 `B\Testing\Temporary\LastTest.log`): `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180"` ≥ 1, `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"` ≥ 1, `grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"` ≥ 1 (기준: `D:\_dongarch_build\verify_job004\studio\Testing\Temporary\LastTest.log` — 세 줄 모두 있음 확인).
15. verify.ps1 종료 코드 0, `B\verify-summary.md` 마지막 줄 `결과: PASS`, 빌드 경고 0: `grep -ci "warning" D:/_dongarch_build/verify_job005/studio-logs/02_build.log` = `0`.
16. 기존 시험 무변경: `git diff --name-only b033aef -- studio/tests/test_pipeline.cpp studio/tests/test_pose.cpp studio/tests/test_side.cpp` → 출력 없음.

### C. 코드 (Git Bash, `cd W`)

17. 각도 입력은 모두 합치기를 거친다: `grep -c "setFrontAngle(doc_.frontAngle()" studio/src/MainWindow.cpp` = `0`, `grep -c "previewFrontAngle" studio/src/MainWindow.cpp` = `1`.
18. 미리보기 갱신은 음영·전체 갱신을 부르지 않는다: `sed -n '/^void MainWindow::refreshPreview/,/^}/p' studio/src/MainWindow.cpp | grep -cE "docChanged|refreshAll|refreshSlots"` = `0`, 같은 범위 `grep -cE "view3d_->refresh|canvas\(\)->update"` ≥ `2`.
19. 놓기 신호·끌기 비율: `grep -c "cutDragReleased" studio/src/View3D.h` ≥ 1, `grep -c "cutDragReleased" studio/src/MainWindow.cpp` ≥ 1, `grep -c "cutAngleDragged(d.x() \* 0.5)" studio/src/View3D.cpp` = `1`.
20. 키·배지: `grep -cE "Key_Left|Key_Right" studio/src/MainWindow.cpp` ≥ `2`, `grep -c "isAutoRepeat" studio/src/MainWindow.cpp` ≥ `1`, `grep -c 'kbdLabel("← →"' studio/src/MainWindow.cpp` = `1`, `grep -c "막 1° · Shift 15°" studio/src/MainWindow.cpp` = `1`, `grep -c "놓은 뒤" studio/src/Plate.cpp` ≥ `1`.
21. R2·R3: `git diff b033aef -- studio/src | grep -E '^\+.*(QColor\([0-9]|qRgba?\([0-9]|#[0-9A-Fa-f]{6}|Qt::(red|green|blue|yellow|white|black|gray|darkGray|lightGray)\b)'` → 출력 없음, `git diff b033aef -- studio/src | grep -c '^\+.*ToolState::Primary'` = `0`.
22. 변경 범위: `git diff --name-only b033aef...HEAD` 의 모든 줄이 §6 표에 있고, `git diff --name-only b033aef...HEAD -- GigaMesh docs CLAUDE.md` → 출력 없음.

### D. 캡처 (ui-reviewer, 예/아니오)

23. verify.ps1 의 `shot_home.png`·`shot_main.png`·`shot_export.png` 가 각각 `> 5120` 바이트이고, `shot_main.png` 가 JOB-004 산출물 `D:/_dongarch_build/wt/factory/jobs/JOB-004/artifacts/shot_main.png` 와 같은 배치(0° 단면 오른쪽·외면 왼쪽, 치수 띠 값 같음). 실시간 동작 자체의 캡처는 004c `--cut` 몫 — 이번에는 사람 검토 9 의 손 확인.

## 5. 검증 명령

1. 표준 검증(작업 폴더 `W`): `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-005 -BuildRoot D:\_dongarch_build\verify_job005 -Job JOB-005 -RecordRoot D:\_dongarch_build\wt` (로그 `L\01_configure.log … 04_shots.log`, 산출물 `D:/_dongarch_build/wt/factory/jobs/JOB-005/artifacts/`). 직후 수용 기준 13·14·15 확인.
2. 개별 시험(PowerShell, `--test-dir` 금지 — 빌드 폴더로 이동): `Remove-Item Env:\CI -ErrorAction SilentlyContinue; $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job005\studio; ctest -R studio_cut -V` → 수용 기준 1-12(`[c]`·`[d]`·`lod-vs-full` 줄, `limit=16 ms (local)`, 마지막 `PASS`). 개별 실행은 LastTest.log 를 덮어쓰므로 기준 14 는 1 직후 또는 전체 `ctest --output-on-failure` 재실행 뒤에 본다.
3. 수용 기준 16-22 는 Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-005` 뒤 그대로.
4. 캡처 비교: `B\shots\shot_main.png` 와 JOB-004 `artifacts\shot_main.png` 를 나란히(육안, 기준 23).
5. verify.md 에 `[d]` 세 줄(예제·미스캔)과 `lod-vs-full` 10줄을 그대로 옮겨 적는다(사람 검토 3·6 자료).

## 6. 영향 파일

| 파일 | 변경 | 이유 |
|---|---|---|
| `studio/src/ArtifactDoc.h` | 수정 | `CutAxis`, `setFrontAngle` 부호 주석(81)·`alignCut`·`previewFrontAngle/commitPreview/isPreviewing/previewAngle`, `lodMesh()`(110-112)·`hasEstimatedInner()`(102)가 미리보기를 따름, `SectionLines`·미리보기 버퍼 멤버(172-178 부근) |
| `studio/src/ArtifactDoc.cpp` | 수정 | 부호(636-639), `setFrontAngle` 가드(607-610), `alignCut`·미리보기 함수, `buildSectionLoops`(811-997) → 순수 `sliceSectionY0` + 적용, `…Natural()`(702-748)·`rawPointCount`(1054-1059)가 보이는 단면 사용, `recompute`(612)·`buildSectionLoops` 가 미리보기 지움 |
| `studio/src/CutGesture.h` | 신규 | 입력 합치기 상태 기계(Qt 무의존, §3.4) |
| `studio/src/View3D.h` · `View3D.cpp` | 수정 | `cutDragReleased` 신호(h:19-20), `mouseReleaseEvent`(200-203) |
| `studio/src/MainWindow.h` · `MainWindow.cpp` | 수정 | `cut_`·`cutTimer_`·`cutClock_`, `keyReleaseEvent`, 끌기·놓기(106), 키(843-846, 882-885, ← →), 단추(751-758), `refreshPreview/flushCut/cancelCut/scheduleCut/onCutTimer/beginCutInput`, 로드 취소(426·459·473), ctx 배지(256-261), `setFrontAngle`(657-661, h:62) 정리 |
| `studio/src/Plate.cpp` | 수정 | 미리보기 중 내보내기 거부(484·502·522·725) |
| `studio/tests/test_cut.cpp` | 신규 | 수용 기준 1-12 |
| `studio/CMakeLists.txt` | 수정 | `studio_core` 에 `CutGesture.h`(55-60), 96 다음 `test_studio_cut`·`studio_cut`·`RUN_SERIAL` |

`GigaMesh/**`, `studio/src/MeshIO.*`·`MeshRaster.*`·`Ribbon.*`·`Theme.h`·`SheetView.*`·`StartPage.*`·`ExportDialog.*`·`main.cpp`, 기존 `studio/tests/*.cpp`, `docs/**`, `CLAUDE.md`, `factory/**` 는 수정하지 않는다.

## 7. 위험·가정

- **좌표계**: mm·+Z 위·바닥판 Z=0·확정 뒤 중심선 x=y=0 그대로. 단면 = "정면각 θ 만큼 **+Z 오른손 법칙으로**(이번에 부호 반전) 돌린 메시의 Y=0 면, x ≥ 0 반쪽", 도판 x 부호는 JOB-004 `sectionSign()`(D5 외면 왼쪽·단면 오른쪽) 그대로. 0° 출력은 비트 단위로 같다.
- **0° 의 뜻**: 확정 자세의 방위 그대로(`confirmPose` 515). 자동 세우기는 구연부 PCA·축 정렬 결과라 방위각이 임의일 수 있다 — 0° 가 유물의 의미 있는 정면이라는 보장은 없다(추천·각도 띠는 뒤 작업).
- **상태 오염**: 미리보기가 `estimatedWallMm_`·`estZMin_/Max_` 를 건드리면 놓은 뒤 결과가 달라진다(§3.3). 수용 기준 2·3 이 `==`/1e-9 로 막는다.
- **이력 의존(기존 성질, 바꾸지 않음)**: `setFrontAngle` 결과는 직전 `estimatedWallMm_` 에 기댄다(`recompute` 675-677, `buildSectionLoops` 929). 그래서 기준 3 은 같은 이력(로드 → 한 번 확정 계산)끼리 비교한다. 미리보기는 지금 확정값을 씨앗으로 쓰므로 미스캔 예제는 놓을 때 추정 내벽이 LOD 차이 외에 씨앗 차이로도 움직일 수 있다(기록만).
- **LOD 구조 차이**: 미스캔 예제 LOD(정점 군집)는 열린 가지가 닫히거나 끊겨 미리보기에서 추정 내벽 경로가 원본과 다를 수 있다 → `hasEst preview/full` 인쇄. 3.5만 면 이하 유물은 LOD = 원본 사본이라 차이 0.
- **상자 확정 경로**: 비회전체 상자에서 `confirmPose`→`recompute`(실루엣·치수·원 맞춤)가 죽지 않는지 **미확인**. 죽으면 그것은 엔진 결함이므로 builder 가 build-log 에 적고 코디네이터에게 알린다(이 JOB 에서 K-Arch 를 고치지 않음).
- **시간 판정**: 이 PC(Release, MSVC, verify.ps1)에서 16 ms. CI 공유 러너는 `CI` 환경 변수로 ×4, `RUN_SERIAL`. 미리보기 경로에서 `FillMeshStats` 나 `recompute` 를 부르면 기준 9 가 잡는다.
- **3D 다시 그리기 비용**: 미리보기마다 3D 보조창이 LOD 를 소프트웨어 래스터로 다시 그린다(`View3D.cpp:91-97`). 기준 9 가 인쇄만 한다 — 16 ms 를 크게 넘으면 끄는 감각이 끊길 수 있다(사람 검토 6).
- **놓을 때 멈춤**: 원본 계산은 UI 스레드에서 돈다(디자인 노트 25만 면 recompute ≈ 1.3 s). 비동기는 범위 밖.
- **키 입력**: ← → 가 다른 위젯에 먹히지 않는지는 미확인(슬롯 목록·리본은 `NoFocus`, `MainWindow.cpp:307` 등) — 손 확인(사람 검토 9).
- **UI 규칙**: 새 색·새 주 단추 없음(R2·R3). 새 문자열은 ctx 배지 1개·내보내기 거부 문구 1개, 모두 한국어(R8 단축키 배지 준수).
- **라이선스**: 새 의존성 없음.

## 8. 사람 검토 항목

1. **[좌표계] 정면각 부호·0° 기준 (이번 결정)**: + = 위에서 보아 반시계(+Z 오른손 법칙) — 근거 §3.1 표(`ArtifactDoc.cpp:21-28, 636-639, 409-426`, `View3D.cpp:191, 194`, `MeshRaster.cpp:21-24`, `MainWindow.cpp:106, 209, 882-885`). 결과: 오른쪽 끌기·→·`.`·`]`·보는 방향 끌기가 D5 Left(3D 를 뒤에서 봄)를 포함해 모든 보기에서 "가까운 면이 오른쪽으로". 지금(시계)과 반대이므로 세션 안 슬롯 각의 뜻이 바뀐다. 0° = 확정 자세 그대로. 승인되면 `CLAUDE.md` §2 에 한 줄 추가를 별도로 제안한다.
2. **[좌표계] X 막 = +90° 의 뜻**: 유물의 원래 −X 쪽(정면에서 보아 왼쪽 옆)이 보는 사람을 향하고, 도판 오른쪽 단면 = 원래 X=0 면의 정면(−Y) 쪽 반. 오른쪽 옆에서 보는 것을 원하면 −90°. Y 막 0° 는 정면(−Y)에서 봄, 단면 = 원래 +X 쪽 반, 외면 음영 = 정면에서 본 왼쪽(−X) 반.
3. **[수치] 미리보기(LOD)↔원본 단면 차이**: 공식 허용오차 없음 — 기준 10 의 10줄(예제 LOD 26,180 면)을 보고, 놓는 순간 단면이 "튀는" 크기가 받아들일 만한지 판단. 크면 미리보기 LOD 면 수를 올리는 안(시간 기준 9 와 맞바꿈).
4. **[수치] 미리보기의 미스캔 보정**: 미리보기는 지금 확정된 추정 기벽 두께를 씨앗으로 쓰고 바꾸지 않는다. 놓으면 `recompute` 순서(평균 기벽 → 단면)로 다시 정하므로 추정 내벽 파선이 놓을 때 조금 움직일 수 있다.
5. **끄는 동안 그대로인 것**: 외면 음영·외곽선·치수 띠·두께 풍선·추정 구간은 놓기 전 값. 회전체는 외곽(최대 반경 포락선)·치수·두께(수평 단면)가 각도와 거의 무관해 오해 소지는 작지만, 음영(외면 쪽 반)은 놓을 때까지 옛 각도 그림이다.
6. **놓을 때 멈춤과 3D 다시 그리기 시간**: 기준 9 의 `[d] … full`·`3D render` 값. 25만 면 유물은 놓을 때 ≈ 1.3 s 멈출 수 있다(디자인 노트). 받아들일지, 비동기 계산을 별도 작업으로 할지.
7. **입력 규칙**: 끄는 동안 16 ms 합치기(throttle), 키·단추 마지막 입력 300 ms 뒤 확정, 누르고 있는 동안 보류, Shift 15° 는 ← → 만, 제스처 하나 = 되돌리기 한 칸, 다른 동작(내보내기·슬롯 저장·R·W·되돌리기 등) 전에 먼저 확정 — 그래서 화면에서는 "미리보기 중 내보내기 거부"가 보이지 않고 엔진·시험에서만 거부한다.
8. **시간 기준 환경**: 16 ms 는 이 PC Release 기준, CI 는 ×4(64 ms). 다른 PC·Debug 빌드는 판정 대상 아님.
9. **손 확인(캡처 없음)**: 예제 열기 → `S`(단면 도구) → 3D 를 오른쪽으로 끌기 = 가까운 면이 오른쪽으로 돌고 도판 단면·3D 절단선이 끄는 동안 바뀜 → 놓으면 잠깐 뒤 원본으로 → `→` 세 번 빠르게 = 약 0.3 s 뒤 한 번 계산 → `→` 누르고 있기 = 떼기 전에는 계산 안 함 → `Ctrl+Z` = 제스처 전 각도. `R`(Left) 상태에서도 오른쪽 끌기 = 가까운 면 오른쪽.

## 9. 크기

| 묶음 | 파일 | 예상 줄(추가+수정) |
|---|---|---|
| 엔진 | ArtifactDoc.h/.cpp | ≈ 100 |
| 입력 합치기 | CutGesture.h(신규) | ≈ 55 |
| 화면 연결 | MainWindow.h/.cpp, View3D.h/.cpp | ≈ 95 |
| 내보내기 거부 | Plate.cpp | ≈ 12 |
| **제품 합계** | | **≈ 260** |
| 테스트·빌드 | test_cut.cpp(신규), CMakeLists.txt | ≈ 320 |

제품 코드는 300줄 지침 안이라 쪼개지 않는다. 300줄을 넘으면 §0 의 005a/005b 로 나눈다. 테스트를 줄이지 않는 이유: 엔진 같음(기준 2·3)과 합치기 규칙(기준 11)이 이 JOB 의 핵심 위험(상태 오염·디바운스 함정)을 직접 막는다.

## 10. 후속 메모 (등록·명세는 코디네이터 몫)

- **JOB-006(004c)**: [Y 막 정면에서][X 막 옆에서] 단추 = `alignCut`, 각도 [−][+] = `CutGesture::click(±1)`, 도판 머리 `A–A′ 실시간 단면 — 12°` = `previewAngle()`, ctx 오른쪽 "끄는 중 3.5만 면 약 1 ms → 놓으면 25만 면 약 6 ms" 문구의 숫자는 기준 9 실측으로 맞춘다. 이번 ctx 배지 `← → 막 1° · Shift 15°` 는 004c 의 ② 페이지 ctx 로 옮긴다.
- 사람 검토 1 승인 시 `CLAUDE.md` §2 에 "정면각 + = +Z 오른손 법칙(위에서 보아 반시계), 0° = 확정 자세" 한 줄.
