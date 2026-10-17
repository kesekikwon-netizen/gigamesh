# JOB-003 명세 — ① 바닥 정하기 (자세 단계)

작성: spec-writer, 2026-10-17. 기준 커밋 `58a33ae`(factory/JOB-003, worktree `D:/_dongarch_build/wt/.worktrees/JOB-003`).
아래 경로·줄 번호는 이 worktree 에서 읽어 확인한 값이다. 이하 `W` = `D:/_dongarch_build/wt/.worktrees/JOB-003`, `B` = `D:\_dongarch_build\verify_job003\studio`(빌드 폴더), `L` = `D:\_dongarch_build\verify_job003\studio-logs`(verify.ps1 로그 폴더).

## 목표

사용자 요청(job.json `request`, 코디네이터 릴레이):
> 사용자 2026-10-17 09:39 "승인개발진행" + 08:03 핵심 요구: **바닥 정하기(X·Y·Z 이동·회전으로 바닥판에 올려 확정)** → 얇은 막 절단 실시간 단면 → 도면 편집. 07:46 손 실측 순서 그대로 원칙. 디자인 v2 주 화면 ① 단계.

디자인 v2 원칙(design-excerpt.md): "스캔은 위아래 없이 크기만 1:1(mm)로 들어온다 … 사람이 바닥을 정해 확정한다. 마지막 결정은 사람."

**재진술(한 문장)**: 스캔 파일을 **들어온 자세 그대로** 열어 두고, 사람이 X·Y·Z 90° 회전·뒤집기·처음 자세로·자동 세우기로 유물을 바닥판(Z = 0 평면) 위에 세운 뒤 「이 자세로 확정」(Enter)을 눌러야 비로소 단면·치수·도판을 계산하게 만든다.

- "이동"은 이번 조각에서 **Z = 바닥에 내려놓기(접지)**, **X·Y = 확정 시 중심선(회전축)을 원점으로 자동 이동**으로 대신한다. 이동 화살표 UI 는 범위 밖.
- 예제 유물(합성 토기·미스캔 예제)과 기존 테스트는 "자동 세우기 → 확정" 경로로 지금과 **같은 값**을 내야 한다.

### 상태 정의 (이 JOB 의 용어)

| 상태 | 조건 | 도판·치수 | 화면 문구(디자인 원문) |
|---|---|---|---|
| 확정 전 · 바닥 미정 | `!poseConfirmed()` 이고 `|bboxMin.z| > 0.3 mm` | 계산 안 함 | 상태줄 오른쪽 `바닥 미정` |
| 확정 전 · 놓음 | `!poseConfirmed()` 이고 `|bboxMin.z| ≤ 0.3 mm` | 계산 안 함 | `놓음 (확정 전)` |
| 확정 | `poseConfirmed()` | 지금과 같은 계산(단면·외곽·5대 치수·슬롯) | 기존 문구 |

## 범위 밖

- **D5 토기 도판 좌/우 배치**: `studio/src/Plate.cpp` 는 이 JOB 에서 한 줄도 바꾸지 않는다(수용 기준 17 로 검사).
- 면 누르기 「이 면을 바닥으로」(3D 피킹), 바닥 후보 ①②③ 점수(평평함·넓이·무게중심 가중치 미정), 그림자로 닿음 확인.
- 오른쪽 도판의 "지금 자세로 그리면 — 정면 미리보기" 두 장, 종류별 규칙 카드 4개.
- 수평 맞추기(구연 수평 T / 바닥 수평 B / 회전축 그대로) 를 자세 단계에서 고르는 기능, 중심선 신뢰도·3점 찍기. (홈 페이지의 기존 `바닥 수평 맞춤 B / 구연부 수평 맞춤 T` 는 높이 기준 토글로 그대로 둔다.)
- 색 고리 끌기 15° 회전·Shift 자유 회전, 색 화살표 이동(X·Y·Z), 3D 창 위 도구줄([X ↻ 90°] … 리본과 중복).
- 축 색(X 주황 #c8641e · Y 초록 #3f8f4f · Z 파랑 #3f6fb5): `Theme.h` 토큰에 없고 UI_RULES R2 가 토큰 외 색을 금지하므로 이번엔 쓰지 않는다(토큰 추가 = 정본 변경, 후속).
- 자세 각도 표시 `자세 X 0° · Y −74° · Z 0°`(누적 회전의 오일러 분해 규약 결정 필요), 자세 단계 단축키 X·Y·Z·F.
- 단위 확인 창, 유물 종류 선택(토기 완형/편/기와/석기), 레이아웃 폭(왼쪽 200 px · 3D 540 px), 단계 띠(3단계 10걸음), 단계 잠금의 "재확인 필요(주황)" 표시와 값 보존.
- 확정 자세 저장(파일을 다시 열면 확정 자세 복원), 자세 조작의 되돌리기/다시.
- ② 막으로 자르기(JOB-004), `factory/scripts/verify.ps1` 에 바닥 단계 캡처 추가(factory 도구 변경은 별도), `docs/ui/mockup/` 갱신(백로그 8).
- `GigaMesh/core`, `GigaMesh/external`, `GigaMesh/gui/src/dongarch/karch/*` 수정(K-Arch 엔진은 읽기만, 기존 함수 재사용).

## 설계 결정 (builder 가 그대로 구현할 기본값)

### 1) ArtifactDoc 자세 API (`studio/src/ArtifactDoc.h`)

```cpp
enum class PoseAxis { X, Y, Z };
void loadMesh(KA::MeshData m, std::vector<QRgb> colors, const QString& name); // 들어온 자세 그대로 보관, 확정 전
bool loadFile(const QString& path, QString* err);   // 시그니처 그대로 — 내부에서 loadMesh, autoAlign 호출 안 함
void loadSample(bool confirm = true);               // true: loadMesh → autoAlign → confirmPose (기존 결과와 같음)
void loadArtecBlindSample(bool confirm = true);     // 위와 같음
void rotatePose(PoseAxis a, double deg);            // 현재 자세의 바운딩 박스 중심을 지나는 고정 월드축 기준, 뒤에 접지
void flipPose();                                    // = rotatePose(PoseAxis::X, 180)
void resetPose();                                   // 처음 자세로: 읽은 정점 좌표를 그대로 복원(접지하지 않음)
void dropToFloor();                                 // 바닥에 내려놓기: z 만 평행이동해 bboxMin.z = 0
void autoAlign();                                   // 자동 세우기: 기존 알고리즘(구연부 PCA → ODR 축 → 원 중심 → 접지), 확정은 안 함
void confirmPose();                                 // 이 자세로 확정 (아래 규칙)
bool poseConfirmed() const;
double heightAboveFloor() const;                    // = mesh().bboxMin.z (mm)
bool isGrounded() const;                            // |bboxMin.z| ≤ 0.3 mm (디자인 v2 접지 판정 권고값)
```

- **원 자세 보관**: `loadMesh` 는 읽은 **정점 좌표만** 별도 사본(`rawVertices_`)으로 보관한다(면은 자세 조작으로 바뀌지 않으므로 공유 — 메모리 절약). `resetPose` 는 이 사본을 그대로 되돌린다(오차 0).
- **회전 행렬**: `deg` 가 90° 의 정수배이면 cos·sin ∈ {−1, 0, 1} 정수 행렬을 쓴다(반복 회전 누적 오차 방지). 그 밖의 각도(테스트의 −74°, 후속 15° 끌기)는 cos·sin 으로 계산.
- **회전 중심 = 현재 자세의 축 정렬 바운딩 박스 중심** (근거·대안은 "사람 검토 항목" 2).
- **매 회전·뒤집기 뒤 접지**: z 만 평행이동해 `bboxMin.z = 0`. `resetPose` 는 접지하지 않는다(들어온 자세 그대로).
- **자세 조작은 모두 확정을 푼다**: `rotatePose/flipPose/resetPose/dropToFloor/autoAlign/rescaleUnits` 뒤 `poseConfirmed() == false`, 슬롯 목록 비움, 치수·단면·외곽 비움. (디자인의 "재확인 필요·값 유지"는 범위 밖.)
- **LOD(3D 보조창용)**: `loadMesh` 에서 한 번 만들고 자세 조작 때 같은 강체 변환을 LOD 에도 적용한다. `autoAlign` 은 지금처럼 끝에서 LOD 를 다시 만든다(`ArtifactDoc.cpp:381`). `confirmPose` 는 확정 자세 메시로 LOD 를 **다시 만든다**(25만 면 초과 유물은 치수를 LOD 로 계산하므로 — 알려진 위험 3, `ArtifactDoc.cpp:401,760` — 확정 자세에 대해 결정적이어야 함).
- **`confirmPose()` 규칙**:
  1. 직전 조작이 `autoAlign` 이면(내부 플래그 `centered_`) 중심 맞춤을 다시 하지 않는다 → 예제 경로가 지금 코드와 비트 단위로 같은 순서가 된다.
  2. 아니면 **중심선 원점 맞춤**: 기존 `autoAlign` 의 마지막 단계와 같은 규칙(`ArtifactDoc.cpp:351-359`) — `FitHorizontalOuters(m, zBand(m, 12))` 의 유효 원 중심 평균 (x̄, ȳ) 만큼 x·y 평행이동. 유효 원이 없으면 바운딩 박스 중심 x·y 를 0 으로(`ArtifactDoc.cpp:366-368` 과 같음). **회전은 하지 않는다**(수직은 사람이 정한 그대로).
  3. 접지(`bboxMin.z = 0`).
  4. `frontAngle_ = 0`, `poseConfirmed_ = true`, LOD 재생성, `resetSlots()`, `recompute()`.
  5. `aligned_`·`alignNote_`·`axisRmse_`: autoAlign 경로는 지금 값 그대로. 수동 경로는 원 맞춤 성공 시 `aligned_ = true`, `alignNote_ = "바닥판 기준 (사람 확정) · 중심선 원 맞춤"`, 실패 시 `aligned_ = false`, `"바닥판 기준 (사람 확정) · 중심선 미검출"`; 두 경우 모두 `axisRmse_ = -1`(3D 정보 줄에 `—` 표시). 단위·부유 노이즈 메모는 지금처럼 뒤에 붙인다(`ArtifactDoc.cpp:371-375`).
- **확정 전 계산 금지**: `recompute()`(`ArtifactDoc.cpp:391-438`)는 확정 전이면 `mesh_ = base_`, `lodMesh_ = lodBase_`, `FillMeshStats` 만 하고 `sil_`, `dims_`, `d5_`(→ `ok=false`, 모두 0), 기벽 프로파일, 단면 고리·추정 내벽선을 비운다. `setInferInnerWall`(291-298)·`setEstimatedWallMm`(300-305)도 확정 전이면 `buildSectionLoops()` 를 부르지 않는다. 확정 전 `alignNote_ = "바닥 미정"`, `aligned_ = false`.
- `loadFile`(187-204): 읽기 결과(`scannerProfile_`, `unitNote_`, `removedNoiseFaces_`, `path_`, `name_`)를 넣고 `loadMesh` 로 들어온 자세 그대로 둔다. **`autoAlign()` 호출 삭제.**
- `loadSample`(206-227)·`loadArtecBlindSample`(229-277): 합성 메시를 만든 뒤 `loadMesh` → `confirm` 이면 `autoAlign()` → `confirmPose()`.
- `rescaleUnits`(279-289, 호출처 없음 — grep 확인): 원 자세 사본과 현재 자세에 같은 배율을 곱하고 확정을 푼다(`autoAlign` 호출 삭제).

### 2) 리본 (`studio/src/MainWindow.cpp`)

- 탭 `정위치·중심축`(166) → **`① 바닥 정하기`**. 이 페이지에 붙던 `alignGroup(pAlign)`·`lineGroup(pAlign)`(185-186)은 빼고(둘 다 홈 페이지에 그대로 남음), 아래 세 그룹을 왼→오로 둔다.
  1. `파일` — 기존 `fileGroup` 재사용(열기 · 최근 ▾).
  2. **`바닥판에 올려놓기`** — 타일 `자동 세우기`(id `poseAuto`, 아이콘 `axis`) · 작은 단추 열 `X 90°`(`poseX`) / `Y 90°`(`poseY`) / `Z 90°`(`poseZ`) · 작은 단추 열 `뒤집기`(`poseFlip`) / `처음 자세로`(`poseReset`) / `바닥에 내려놓기`(`poseDrop`). 90° 단추는 누르면 +90°(오른손 법칙), **Shift 를 누른 채 누르면 −90°**(`QGuiApplication::keyboardModifiers()`). 툴팁에 축 뜻과 Shift 안내를 한국어로.
  3. **`확정`** — 타일 `이 자세로 확정`(id `poseConfirm`), 툴팁 `이 자세로 확정 (Enter)`.
- **주 단추(R3)**: `poseConfirm` 은 확정 전 `ToolState::Primary`, 확정 후 `ToolState::Idle`. 이 페이지에는 `export` 타일이 없으므로 화면당 Action 하나가 지켜진다.
- **활성 규칙(판단)**: 메시가 열려 있으면 `poseConfirm` 은 **항상 누를 수 있다**. 근거: 이 조각에서는 "놓기"가 별도 동작이 아니라 확정이 접지(`bboxMin.z = 0`)와 중심선 원점 맞춤을 함께 하므로 "바닥 미정"이어도 확정 결과가 정의된다. 디자인 v2 의 "바닥 미정이면 비활성"은 면 누르기·바닥 후보가 들어오는 후속 JOB 에서 다시 적용한다(사람 검토 항목 4). 확정 뒤 다시 누르면 아무 일도 하지 않고 상태줄에 `이미 확정했습니다 — 자세를 바꾸면 다시 확정하세요.`
- 동작 연결(`onAction`, 608-674): `poseX/Y/Z` → `rotatePose(축, Shift ? −90 : +90)`, `poseFlip` → `flipPose()`, `poseReset` → `resetPose()`, `poseDrop` → `dropToFloor()`, `poseAuto` → `autoAlign()`(확정 안 함), `poseConfirm` → `confirmPose()` 후 리본을 **홈 페이지**로, 도판 맞춤(fitView), 최근 목록 갱신(경로가 있으면 `pushRecent` 로 치수 기록). 자세 조작은 `pushUndo` 하지 않는다.
- 홈 페이지의 `중심축 맞춤`(id `axis`, 628-634) = `autoAlign()` → `confirmPose()` (지금의 한 번 누르기 동작 보존).
- **확정 전 가드**: 확정 전에는 `cut, outline, cutMove, addSlot, rotate, refBase, refRim, export, svg, dxf` 동작과 단축키 `S O B T N R , . [ ] 1~5 W - +`, 되돌리기/다시(196-205)를 실행하지 않고 상태줄에 `먼저 ① 바닥 정하기에서 「이 자세로 확정」 (Enter)` 를 띄운 뒤 리본을 ① 페이지로 돌린다. `shade, lines, grid, shadeMode`(표시 토글)는 그대로 허용.
- **Enter**(`Qt::Key_Return`/`Key_Enter`, keyPressEvent 707-759): 확정 전이면 `poseConfirm` 과 같다. 확정 뒤에는 아무 일도 안 함. 확정 전 `Esc` 는 무시(바닥 정하기 ctx 유지).

### 3) 화면 문구 (디자인 v2 원문 라벨 기준)

미구현 기능을 가리키는 원문 구절은 이번 기능으로 바꿔 쓴다(바꾼 곳은 ★ 표시, 후속 JOB 에서 원문 복원).

| 위치 | 확정 전 · 바닥 미정 | 확정 전 · 놓음 |
|---|---|---|
| ctx 배지 (`ctxBadge_`) | `▣ 바닥 정하기` | 같음 |
| ctx 안내 (`ctxHint_`) | ★ `스캔은 위아래 없이 들어옵니다 · X·Y·Z 90° 단추나 「자동 세우기」로 세우세요 · Enter 확정` (원문: "… 바닥 후보를 누르거나 색 고리를 끌어 세우세요 · Enter 확정") | 같음 |
| ctx 단축키 줄 | 단면 도구용 `[ ] · W · - + · 1~5 · Esc` 묶음은 숨기고 `Enter 확정`, `Shift −90°` kbd 배지만 | 같음 |
| ctx 오른쪽 (`ctxRight_`, 모노) | `바닥판에서 45.0 mm 위` (음수면 `바닥판 아래 7.2 mm`) | `바닥판에 닿음` (+ 직전이 자동 세우기이고 축을 찾았으면 ` · 중심선 자동`) |
| 상태줄 왼쪽 (`stHint_`) | ★ `③ 바닥 놓기 — 입이 위로 오게 X·Y·Z 90°·뒤집기로 돌리거나 「자동 세우기」를 누르세요` (원문: "③ 바닥 놓기 — ① 저부 접지면을 누르면 바닥판에 섭니다") | ★ `④ 수평·기준선 — 3D 바닥판에서 자세를 보고 「이 자세로 확정」 (Enter)` (원문: "… 미리보기를 보고 「이 자세로 확정」") |
| 상태줄 오른쪽 (`stRef_`) | `단위 mm · 바닥 미정` | `단위 mm · 놓음 (확정 전)` |
| 3D 머리글 (View3D.cpp:163) | `3D — 바닥판(실측판)에 올려놓기` | 같음 |
| 3D 정보 줄 오른쪽 (`info2_`, 209-210) | ctx 오른쪽과 같은 접지 문구 | 같음 |
| 도판 자리 (SheetView.cpp:74-79 와 같은 자리 문구 방식) | `바닥을 정하기 전입니다 — 「이 자세로 확정」(Enter)을 누르면 실측 도판이 그려집니다` | 같음 |
| 도판 머리 배지 (SheetView.cpp:241-247) | `▲ 바닥 미정` (`BadgeWarn`) | 같음 |
| 치수 띠 | 다섯 칸 모두 `—`(지금 0 이하 값 표시 규칙, SheetView.cpp:256) | 같음 |

- 확정 뒤 문구·배지·ctx 는 지금 코드 그대로(refreshAll 452-474). 확정 전에는 ctx(=「▣ 바닥 정하기」를 든 상태, R1 "도구를 손에 들었을 때만")를 보이고, 확정하면 숨긴다.
- `openPath`(392-416): 열면 확정 전 상태로 ① 페이지를 연다. 최근 목록 기록은 `kind = "바닥 미정 · <스캐너 프로파일>"`, 치수 칸 `치수 — (바닥 확정 전)`. Artec 감시 폴더 자동 열기 문구(387-388)를 `✓ Artec Studio 연동: <파일> 자동 로드 — ① 바닥 정하기에서 자세를 확정하세요` 로.
- `openSample/openArtecBlindSample`(418-444): `bool poseStage = false` 인자 추가. 기본은 지금처럼 확정 상태로 홈 페이지, `true` 면 `loadSample(false)`/`loadArtecBlindSample(false)` 후 ① 페이지.

### 4) 3D 보조창 바닥판 (`studio/src/View3D.cpp`, `MeshRaster.h/.cpp`)

- 확정 전에만: 절단면 사각형·중심축·단면선·A/A′ 표지(View3D.cpp:78-120)와 `setDoc` 의 절단 계산(21-35)을 생략하고 **바닥판**을 그린다.
- 바닥판 = Z = 0 평면 위 정사각형. 중심 = 현재 메시 바운딩 박스 중심 (x, y), 반쪽 길이 `S = ceil((max(반폭x, 반폭y) + 20) / 10) × 10` mm. 10 mm(1 cm) 간격 방안선 `theme::Grid10mm` 1 px, 테두리 `theme::Edge` 1 px, 채움 `theme::Wash`. 메시 그림보다 **먼저** 그린다(뒤쪽). 새 색 리터럴 금지(R2).
- 화면 맞춤: `RasterView` 에 `double centerX = 0, centerY = 0` 을 더하고 `toCam`(MeshRaster.cpp:20-29)에서 `p.x - centerX`, `p.y - centerY` 로 쓴다(기본값 0 → 확정 뒤 3D·도판 음영 결과 불변). 확정 전에는 (메시 바운딩 박스 ∪ 바닥판 사각형)의 중심·크기로 맞춘다(view() 44-57).
- 확정 뒤 3D 보조창은 지금 그림 그대로(바닥판 없음).

### 5) 검수용 CLI (`studio/src/main.cpp`)

- `--base`: `--sample` 또는 `--blind` 뒤에 붙으면 해당 예제를 **확정 전(들어온 자세)** 으로 열고 ① 페이지를 보인 채 `--shot` 캡처. 예: `DongArch3DStudio.exe --sample --base --shot <png>`. 파일 경로 인자는 원래 확정 전으로 열리므로 `--base` 와 무관. 24행 주석에 사용법 추가.

## 수용 기준 (각 항목은 명령 하나로 참/거짓)

허용오차 근거: 기하 항등(회전 왕복·접지·좌표 보존)은 기존 K-Arch 테스트의 **1e-9 mm** 규약(`test_synthetic.cpp:126,136,155,167`, `test_dimensions.cpp:51-54,125-126`, `test_rim_detector.cpp:100,113`)을 쓴다. 두 파이프라인 결과 비교는 기존 STL 왕복 기준 **1.0 mm**(`test_pipeline.cpp:144`), 절대값은 기존 기준 **기고 184.5 ± 1.5 mm · 구경 142.0 ± 2.0 mm**(`test_pipeline.cpp:44-45`). 새로 만든 허용오차 없음.

### A. 엔진 — 새 CTest `studio_pose` (`studio/tests/test_pose.cpp`, 실행 파일 `test_studio_pose`, 인자 = 출력 폴더)

1. **(a) 90° 네 번 = 원래**: `loadSample(false)` → `dropToFloor()` → 정점 P0 저장 → X 축 `rotatePose(+90)` 4번 → 모든 정점 `max|Δ| ≤ 1e-9 mm`. Y 축, Z 축도 각각 같은 검사. (−90° 4번도 같은 검사.)
2. **(b) 뒤집기 두 번 = 원래**: P0 에서 `flipPose()` 2번 → `max|Δ| ≤ 1e-9 mm`.
3. **(c) 조작 뒤 접지·중심 유지**: 1·2 의 매 `rotatePose(±90)`/`flipPose()` 직후 `|bboxMin.z| ≤ 1e-9 mm` 이고, 바운딩 박스 중심 x·y 가 조작 전과 `≤ 1e-9 mm` 차이(회전 중심 = 박스 중심 규약 검증). `dropToFloor()` 직후도 `|bboxMin.z| ≤ 1e-9 mm`.
4. **처음 자세로**: 불러온 직후 정점 사본 R0 → 임의 조작(X+90, Y−90, 뒤집기, 내려놓기, Y −74°) → `resetPose()` → 정점이 R0 와 **정확히 같음**(`max|Δ| == 0`), `poseConfirmed() == false`.
5. **(d) 눕힌 항아리 자동 세우기**: 기준 `ref.loadSample()`(확정) 치수 H_ref, rim_ref. 시험 `doc.loadSample(false)` → `rotatePose(Y, −74)` → `autoAlign()` → `confirmPose()` → `|H − H_ref| ≤ 1.0 mm`, `|rim − rim_ref| ≤ 1.0 mm`, 그리고 `|H − 184.5| ≤ 1.5`, `|rim − 142.0| ≤ 2.0`, `dims().ok`. 값(H, rim, belly, base)을 표준 출력에 인쇄.
6. **(e) 확정 전 미계산** — *2026-10-17 오케스트레이터 수정(decision.md 참조): 원래 문구는 '들어온 자세(7° 기울어짐)로 수동 확정 → dims().ok' 를 요구했으나, 수동 확정은 회전하지 않는다(설계 결정 1) 와 K-Arch 의 구연 판정(구연 고리 z 폭 ≤ 3 mm, DimensionAnalyzer.cpp:85-111, 수정 금지)이 함께 성립할 수 없어 6a/6b 로 나눈다.*
   - **6a** `loadSample(false)` 직후와 아래 7 의 `loadFile` 직후 각각 `poseConfirmed() == false`, `dims().ok == false`, `dims().height == 0`, `displaySection().empty()`, `displayOutline().empty()`, `slotList().empty()`, `rawPointCount() == 0`. 들어온 자세 그대로 `confirmPose()` 하면 `poseConfirmed()`, `dims().height > 0`, `!displaySection().empty()`, `|bboxMin.z| ≤ 1e-9` 이고, 구연이 기울어 있으면 `dims().ok == false`(구경 미측정)가 **정상**이다. 확정 뒤 `rotatePose(X, 90)` 하면 다시 미확정·`dims().ok == false`.
   - **6b** `loadSample(false) → autoAlign() → confirmPose()` 뒤 `dims().ok`, `!displaySection().empty()`, `!slotList().empty()`, `|bboxMin.z| ≤ 1e-9`. 확정 뒤 `rotatePose(X, 90)` 하면 다시 미확정·`dims().ok == false`. 자동 세우기만으로는 계산하지 않는다(`autoAlign()` 직후 미확정).
7. **(f) 파일 열기는 자세를 바꾸지 않음**: 합성 항아리(기울기 7°, 이동 (12, −5, 40) mm)를 `exportLegacyPly(…, binary=false)`(MeshIO.h:92-95)로 출력 폴더에 쓴 뒤 `doc.loadFile(path)` → `doc.mesh().bboxMin/bboxMax` 가 `readMeshAny(path).mesh.bboxMin/bboxMax` 와 6개 성분 모두 `≤ 1e-9 mm`, `doc.mesh().bboxMin.z > 1.0 mm`(접지 안 됨), `poseConfirmed() == false`.
8. **수동 확정(자동 세우기 없이)**: 곧게 선 합성 항아리(기울기 0°, 이동 (12, −5, 3), 잡음 0.05) `loadMesh` → `confirmPose()` → `|bboxMin.z| ≤ 1e-9`, `dims().ok`, `|H − 184.5| ≤ 1.5`, `|rim − 142.0| ≤ 2.0`, `!displaySection().empty()`.
9. `test_studio_pose` 는 위 실패가 하나도 없으면 마지막 줄 `PASS`, 종료 코드 0.

### B. 회귀

10. `test_pipeline.cpp` 는 **STL 왕복 부분만** 바꾼다(138행 `loadFile` 뒤 `d2.autoAlign(); d2.confirmPose();` 추가). 다른 기대값·허용오차는 그대로이고 통과.
11. 지금과 같은 값: `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00" B\Testing\Temporary\LastTest.log` ≥ 1, `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2" …LastTest.log` ≥ 1, `grep -c "ArtecBlind: H=190.30 rim=124.00" …LastTest.log` ≥ 1 (기준값 = 병합본 `D:\_dongarch_build\verify_merged\studio\Testing\Temporary\LastTest.log`, 이 값들은 0.1 mm 로 반올림된 치수라 부동소수 재배열에 흔들리지 않음).
12. CTest 13종(기존 12 + `studio_pose`) 전부 통과: `B` 에서 `ctest --output-on-failure` 출력에 `100% tests passed, 0 tests failed out of 13`.
13. verify.ps1 종료 코드 0, `D:\_dongarch_build\verify_job003\studio\verify-summary.md` 마지막 줄 `결과: PASS`; 빌드 경고 0: `grep -ci "warning" D:/_dongarch_build/verify_job003/studio-logs/02_build.log` = `0` (기준선: 병합본 `verify_merged/studio-logs/02_build.log` 도 0).

### C. 화면·CLI·규칙

14. 바닥 단계 캡처: 아래 "검증 명령 3" 이 `B\shots\shot_base.png` 를 만들고 크기 `> 5120` 바이트. verify.ps1 기존 캡처 `shot_home.png`·`shot_main.png`·`shot_export.png` 도 각각 `> 5120` 바이트(`--sample` 은 여전히 확정 상태로 도판이 그려짐).
15. 원문 라벨 존재: `cd W && for s in "① 바닥 정하기" "바닥판에 올려놓기" "자동 세우기" "뒤집기" "처음 자세로" "바닥에 내려놓기" "이 자세로 확정" "▣ 바닥 정하기" "바닥판에서" "바닥판에 닿음" "바닥 미정" "놓음 (확정 전)" "3D — 바닥판(실측판)에 올려놓기" "바닥을 정하기 전"; do grep -rqF "$s" studio/src || echo "MISSING $s"; done` → 출력 없음. 그리고 `grep -c 'addPage(QStringLiteral("정위치·중심축"))' studio/src/MainWindow.cpp` = `0`.
16. R2·R3: `git -C W diff 58a33ae -- studio/src | grep -E '^\+.*(QColor\([0-9]|qRgba?\([0-9]|#[0-9A-Fa-f]{6}|Qt::(red|green|blue|yellow|white|black|gray|darkGray|lightGray)\b)'` → 출력 없음(새 색 리터럴 없음; 기존 코드를 옮기기만 해서 생긴 줄이 걸리면 code-reviewer 가 이동임을 확인). `git -C W diff 58a33ae -- studio/src | grep -E '^\+.*ToolState::Primary' | grep -vc poseConfirm` → `0`(새 주 단추는 `poseConfirm` 하나).
17. 변경 범위: `git -C W diff --name-only 58a33ae...HEAD` 의 모든 줄이 아래 "영향 파일" 표에 있다. `git -C W diff --name-only 58a33ae...HEAD -- GigaMesh studio/src/Plate.cpp studio/src/Plate.h studio/src/ExportDialog.cpp` → 출력 없음.
18. 정본 동기화: `grep -c "① 바닥 정하기" W/docs/ui/UI_RULES.md` ≥ 1 (R1 탭 줄).

### D. 육안 검수 (ui-reviewer, `shot_base.png` 기준 — 항목마다 예/아니오)

19. 탭 줄에 `① 바닥 정하기` 가 켜져 있고 리본에 `파일 · 바닥판에 올려놓기 · 확정` 그룹, `이 자세로 확정` 만 Action 색. 20. 3D 보조창에 1 cm 방안 바닥판과 그 위(또는 걸친) 합성 항아리. 21. ctx 줄에 `▣ 바닥 정하기` 배지와 접지 문구. 22. 도판 자리에 `바닥을 정하기 전…` 문구, 치수 띠 `—`, 배지 `▲ 바닥 미정`. 23. 상태줄 `③ 바닥 놓기 …` 또는 `④ 수평·기준선 …` 과 `바닥 미정`/`놓음 (확정 전)`. (오프스크린 캡처의 한글 □ 는 알려진 위험 7 이므로 불합격 사유가 아님.)

## 검증 명령

1. 표준 검증(구성→빌드→ctest→기존 캡처, 기록 복사):
   `powershell -NoProfile -ExecutionPolicy Bypass -File D:/_dongarch_build/wt/.worktrees/JOB-003/factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-003 -BuildRoot D:\_dongarch_build\verify_job003 -Job JOB-003 -RecordRoot D:\_dongarch_build\wt`
   (템플릿의 `factory/scripts/verify.ps1 -Job JOB-003` 와 같은 절차. 로그 `L\01_configure.log … 04_shots.log`, 요약 `D:\_dongarch_build\verify_job003\studio\verify-summary.md`, 산출물 복사 `D:/_dongarch_build/wt/factory/jobs/JOB-003/artifacts/`.)
2. CTest 단독 재확인(빌드 폴더로 이동, `--test-dir` 금지). PowerShell 에서:
   `$env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job003\studio; ctest --output-on-failure`
   개별: `ctest -R studio_pose --output-on-failure -V` (수용 기준 1-9 의 인쇄값 확인), `ctest -R studio_pipeline -V`.
3. 바닥 단계 캡처(verify.ps1 은 고치지 않음). PowerShell 에서:
   `$env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Start-Process -Wait -WorkingDirectory D:\_dongarch_build\verify_job003\studio -FilePath D:\_dongarch_build\verify_job003\studio\DongArch3DStudio.exe -ArgumentList '--sample','--base','--shot','D:\_dongarch_build\verify_job003\studio\shots\shot_base.png'; (Get-Item D:\_dongarch_build\verify_job003\studio\shots\shot_base.png).Length`
   → 숫자가 5120 초과면 참. 그 뒤 `Copy-Item D:\_dongarch_build\verify_job003\studio\shots\shot_base.png D:\_dongarch_build\wt\factory\jobs\JOB-003\artifacts\`. (Git Bash 에서 부를 때는 `$env` 가 bash 에 먹히지 않게 명령 전체를 작은따옴표로 감싼 `powershell -NoProfile -Command '…'`.)
   선택: `--blind --base` 로 `shot_base_blind.png`.
4. 캡처 비교: `shot_main.png`·`shot_export.png` 를 JOB-002 산출물(`D:/_dongarch_build/wt/factory/jobs/JOB-002/artifacts/shot_main.png`, `shot_export.png`)과 나란히 보아 도판·치수 띠·리본 홈 페이지가 같은지 육안 확인(픽셀 비교 아님). `shot_base.png` 는 수용 기준 19-23.
5. 수용 기준 15-18 의 grep/git 명령은 Git Bash 에서 그대로 실행.

## 영향 파일

| 파일 | 변경 | 이유 |
|---|---|---|
| `studio/src/ArtifactDoc.h` | 수정 | `PoseAxis`, 자세 API(1절), `loadSample/loadArtecBlindSample(bool confirm=true)`, 멤버 `rawVertices_`·`poseConfirmed_`·`centered_` (현재 API 39-60, 멤버 128-151) |
| `studio/src/ArtifactDoc.cpp` | 수정 | `loadFile`(187-204) autoAlign 제거·`loadMesh` 경유, `loadSample`(206-227)·`loadArtecBlindSample`(229-277) 자동 세우기→확정, `rescaleUnits`(279-289), `setInferInnerWall`(291-298)·`setEstimatedWallMm`(300-305) 확정 전 생략, `autoAlign`(307-384) 끝의 `resetSlots/recompute`(382-383)를 `confirmPose` 로 이동, `recompute`(391-438) 확정 전 계산 생략, 새 함수 rotate/flip/reset/drop/confirm |
| `studio/src/MeshRaster.h` · `MeshRaster.cpp` | 수정 | `RasterView::centerX/centerY`(13-19), `toCam`(20-29) — 기본 0 이라 기존 결과 불변 |
| `studio/src/View3D.h` · `View3D.cpp` | 수정 | 확정 전 바닥판·화면 맞춤(`view()` 44-57, `paintEvent` 59-121, `setDoc` 21-35), 머리글(163)·정보 줄(203-211) 문구 |
| `studio/src/SheetView.cpp` | 수정 | 확정 전 자리 문구(`paintEvent` 71-79), `fitView`(44-52) 생략, `refresh`(227-263) 배지 `▲ 바닥 미정` |
| `studio/src/Ribbon.cpp` | 수정(선택) | `확정` 타일 아이콘 1종 추가 시 `drawIcon`(36-86) — 2 px 단색 선 규칙(R3). 기존 아이콘 재사용이면 무변경 |
| `studio/src/MainWindow.h` | 수정 | 페이지 번호 멤버(`pBase_`, `pHome_`), ctx 묶음 위젯 포인터, `openSample/openArtecBlindSample(bool poseStage=false)` |
| `studio/src/MainWindow.cpp` | 수정 | 탭·그룹(122-206), ctx(208-239), `openPath`(392-416)·예제 열기(418-444)·감시 폴더 문구(387-388), `refreshAll` 자세 분기(452-474), `onAction` 자세 id·가드(608-674), Enter·가드(707-759), 되돌리기 가드(196-205) |
| `studio/src/main.cpp` | 수정 | `--base` (23-30) |
| `studio/tests/test_pose.cpp` | 신규 | 수용 기준 1-9 |
| `studio/tests/test_pipeline.cpp` | 수정 | 138행 STL 왕복에 `autoAlign()`+`confirmPose()` (수용 기준 10) |
| `studio/CMakeLists.txt` | 수정 | 88-90 다음에 `add_executable(test_studio_pose tests/test_pose.cpp)`, `target_link_libraries(… studio_core)`, `add_test(NAME studio_pose COMMAND test_studio_pose ${CMAKE_CURRENT_BINARY_DIR})` |
| `docs/ui/UI_RULES.md` | 수정 | R1 표 1행 탭 이름 `정위치·중심축` → `① 바닥 정하기`, 3행 ctxBar 비고에 "① 바닥 정하기 확정 전 = 「▣ 바닥 정하기」를 든 상태", R1.4 미확정 배지 `▲ 바닥 미정`, R3 주 단추 예에 `[이 자세로 확정]` |

코어(`GigaMesh/core`, `external`)·K-Arch 엔진(`GigaMesh/gui/src/dongarch/karch`)·`Plate.*`·`ExportDialog.*`·`StartPage.*`·`Theme.h`·`factory/scripts/*` 는 수정하지 않는다. K-Arch 함수(`DetectRim`, `ComputePcaAlignment`, `FitHorizontalOuters`, `OptimizeRotationAxis`, `FillMeshStats`, `MakeTorusProfileVessel`)는 호출만 한다.

## 위험·가정

- **좌표계**: 단위 mm(double), +Z 위, 바닥판 = Z = 0 평면. 들어온 스캔은 축이 임의(Artec 출력 좌표 그대로)이며 `loadFile` 은 이를 바꾸지 않는다(단, 기존 `readMeshAny` 의 단위 자동 환산·부유 노이즈 제거는 그대로 — MeshIO.cpp:1244-1273, 알려진 위험 2 는 이 JOB 과 무관하게 남음). 확정 뒤 좌표는 "중심선이 x = y = 0, 바닥이 z = 0" 인 도면 좌표로 바뀐다(원 좌표는 메모리에만 보관, 파일로 저장하지 않음).
- **단면 평면 가정**: 도판 단면은 Y = 0 평면(`ArtifactDoc.cpp:557`), 3D 절단도 같은 평면(View3D.cpp:31). 그래서 확정은 반드시 중심선을 원점으로 옮겨야 한다 — 수동 확정의 원 맞춤이 실패하면 박스 중심으로 대신하고 배지를 경고(Caution)로 둔다.
- **자동 세우기의 자세 의존성**: `DetectRim(Auto)` 는 경계 고리 중 평균 z 가 가장 큰 것을 구연으로 고르고(RimDetector.cpp:192-200), 경계가 없으면 z 기준 곡률 방식으로 넘어가며(469-484, 264-321), 축 정밀화도 z 띠 수평 단면(`zBand`, ArtifactDoc.cpp:60-67)을 쓴다. 따라서 **닫힌(워터타이트) 스캔이나 경계 고리가 여럿인 스캔(미스캔 예제처럼 바닥이 뚫린 경우)** 은 처음 자세에 따라 결과가 달라질 수 있다. 수용 기준 5 는 경계 고리가 구연뿐인 합성 항아리(`capBottom=true`)에서만 보장한다. 사용 안내: "입이 대략 위로 오게 돌린 뒤 자동 세우기".
- **흐름 변화**: 파일을 열면 곧바로 도판이 나오지 않고 Enter(확정)가 한 번 더 필요하다. Artec 감시 폴더 자동 열기도 ① 단계에서 멈춘다. 확정 자세를 저장하지 않으므로 같은 파일을 다시 열면 다시 정해야 한다(후속 후보).
- **메모리**: 원 정점 사본으로 정점당 24 바이트 추가(500만 정점 ≈ 120 MB). 면은 복사하지 않는다.
- **대형 메시(25만 면 초과)**: 치수를 LOD 로 계산하는 기존 위험 3 그대로. 확정 시 LOD 재생성으로 결정성만 보장.
- **부동소수**: Release `/fp:fast`·LTO 로 값이 미세하게 바뀔 수 있으나 수용 기준 11 은 0.1 mm 반올림 치수(`RoundTenths`)만 비교한다. 수용 기준 1-4 는 90° 정수 행렬을 쓰므로 1e-9 mm 여유가 수천 배.
- **UI 규칙**: R1 탭 이름 변경은 정본(UI_RULES.md) 변경 → 같은 JOB 에서 문서를 먼저 고친다(CLAUDE.md §1, 사람 검토 항목 6). 목업(`docs/ui/mockup/`)은 옛 탭 이름 그대로 남아 "목업과 코드가 다르면 코드가 틀린 것" 규칙과 일시 충돌 — 백로그 8(디자인 v2 반입)에서 해소. R2: 새 색 없음(바닥판은 `Wash`·`Grid10mm`·`Edge`). R3: 주 단추는 페이지마다 하나. R8: Enter·Shift 를 kbd 배지로.
- **오프스크린 캡처 한글 □**(알려진 위험 7): 캡처 판정은 파일 크기·배치로만 한다.
- **라이선스**: 새 의존성 없음. triangle 미사용 유지.
- **D5**: `Plate.*` 무변경으로 좌/우 배치는 지금 상태(왼쪽 단면) 그대로 — 백로그 결함 6 은 별도.

## 사람 검토 항목

0. **(오케스트레이터 추가) 기울어진 채 확정하면 구경이 측정되지 않는다(6a).** 손 실측과 같은 동작(입 둘레를 수평으로 맞추지 않으면 구경을 잴 수 없음)으로 보고 정상으로 정했다. 화면에 "구연 수평 필요" 같은 안내를 띄우는 것은 후속(수평 맞추기 T/B 조각)에서 다룬다.


job.json `human_review_required` 3건에 대한 기본값과, 이 명세가 내린 판단. 모두 승인(`/factory approve`) 때 확인받는다.

1. **좌표계·수직 기준**: 바닥판 = Z = 0 평면, +Z 위, mm. "수직"은 사람이 정한 자세 그대로이며(수동 확정은 회전하지 않음), 자동 세우기를 쓴 경우에만 구연부 PCA + 회전축 ODR 로 회전축을 +Z 로 맞춘다. 확정 시 중심선(수평 외곽 원 중심 평균)을 x = y = 0 으로 옮긴다.
2. **회전 합성·축 규약·회전 중심**: 단추 한 번 = 고정 월드축(바닥판의 X·Y·Z, 외재적) 기준 **오른손 법칙 +90°**, Shift = −90°. 연속 조작은 매번 현재 자세에 왼쪽 곱으로 누적. `뒤집기` = X 축 180°(정면 보기 — 카메라가 −Y 에서 +Y 를 봄, MeshRaster.h:22 — 에서 위아래만 바뀌고 좌우는 유지). 회전 중심 = **현재 자세의 축 정렬 바운딩 박스 중심**. 근거: (가) 90° 배수 회전에서 박스 중심이 정확히 고정되어 "4번 = 원래"·"바닥판 위 발자국 중심 유지"가 성립, (나) 정점 평균 무게중심(`MeshCentroid`, AxisPca.cpp:81-96)은 Artec 메시의 고르지 않은 정점 밀도·구멍 쪽으로 쏠림, (다) `FillMeshStats` 가 이미 계산해 추가 비용 0. 대안: 면적 가중 무게중심 — 자유 각도 회전(후속 15° 고리 끌기)에서는 박스 중심이 회전마다 바뀌므로 그때 다시 검토. 리본 글자는 `X 90°` 로 두고 디자인의 `↻` 기호는 쓰지 않는다(보는 방향에 따라 시계/반시계가 바뀌어 오해 소지).
3. **바닥에 내려놓기(접지) 수치 규칙**: 동작은 z 평행이동으로 `bboxMin.z` 를 **정확히 0**(검사 1e-9 mm). "바닥판에 닿음" 표시 판정은 `|bboxMin.z| ≤ 0.3 mm`(디자인 v2 권고값). 회전·뒤집기·자동 세우기·확정 뒤에는 자동 접지, `처음 자세로` 와 파일 열기 직후는 접지하지 않음.
4. **확정 단추 활성 규칙**(디자인과 다름): 디자인 v2 는 "바닥 미정이면 비활성"이지만, 이 조각에서는 확정이 접지와 중심 맞춤을 겸하므로 메시가 있으면 항상 활성. 면 누르기·바닥 후보가 들어오는 후속 JOB 에서 디자인대로 되돌릴지 결정 필요.
5. **단계 잠금 단순화**(디자인과 다름): 확정 뒤 자세를 바꾸면 확정이 풀리고 슬롯·치수가 지워진다(디자인은 "재확인 필요(주황)·값 유지").
6. **UI 정본 변경**: UI_RULES R1 탭 이름 `정위치·중심축` → `① 바닥 정하기`(CLAUDE.md §1 "바꾸려면 먼저 사용자 승인" 대상 — 사용자 "승인개발진행"·디자인 v2 를 근거로 진행하되 승인 때 재확인).
7. **원문 라벨 대체 4곳**(★): ctx 안내, 상태줄 ③·④ 문구 — 미구현 기능(바닥 후보·색 고리·미리보기)을 가리키지 않게 바꿈.

## 크기

| 묶음 | 파일 | 예상 줄(추가+수정) |
|---|---|---|
| A 엔진 | ArtifactDoc.h/.cpp | ≈ 150 |
| A 테스트 | test_pose.cpp 신규, test_pipeline.cpp, CMakeLists.txt | ≈ 160 |
| B 화면 | MainWindow.h/.cpp | ≈ 140 |
| B 화면 | View3D.h/.cpp, MeshRaster.h/.cpp | ≈ 60 |
| B 화면 | SheetView.cpp, Ribbon.cpp, main.cpp | ≈ 30 |
| 문서 | UI_RULES.md | ≈ 5 |
| **합계** | | **≈ 545 (제품 ≈ 380, 테스트 ≈ 160)** |

**300줄 초과 → 분할 제안**: 
- **JOB-003a 엔진·테스트(≈ 310, 테스트 절반)**: 1절 자세 API + `test_pose` + `test_pipeline` + CMake. 이때 `MainWindow::openPath` 는 임시로 `loadFile` 뒤 `autoAlign(); confirmPose();` 를 불러 앱 동작을 지금과 같게 유지. 수용 기준 1-13, 16-17.
- **JOB-003b 화면(≈ 235)**: 2~5절(리본 ①, 3D 바닥판, ctx·상태줄·도판 자리 문구, `--base`, UI_RULES) + `openPath` 를 확정 전 열기로 전환. 수용 기준 14-23.

나누지 않고 한 JOB 으로 갈 경우에도 builder 는 **A → B 순서로 커밋 2개**(각 커밋에서 빌드·ctest 통과)로 나눠 검토를 쉽게 한다. 결정은 코디네이터 몫.
