# JOB-004 명세 — ② 막으로 자르기: 분할 제안과 **JOB-004a (도판 좌/우 D5 수정 · R 좌/우 반전)**

작성: spec-writer, 2026-10-17. 기준 커밋 `e9b655a`(factory/JOB-004, worktree `D:/_dongarch_build/wt/.worktrees/JOB-004`, **JOB-003 승인 전제**).
아래 경로·줄 번호는 이 worktree 에서 직접 읽어 확인한 값이다. 이하 `W` = `D:/_dongarch_build/wt/.worktrees/JOB-004`, `B` = `D:\_dongarch_build\verify_job004\studio`(빌드 폴더), `L` = `D:\_dongarch_build\verify_job004\studio-logs`(verify.ps1 로그 폴더).

## 0. 분할 결정 (먼저 읽을 것)

요청 전체(D5 수정 + 실시간 단면 + ② 페이지 + 시험 6종 + `--cut` 캡처)를 코드에서 세어 보면 **제품 ≈ 420줄 + 테스트 ≈ 330줄 ≈ 750줄**로 300줄 지침의 두 배를 넘는다. 실시간 단면은 엔진 상태 오염 함정(아래 §후속 1)이 있어 D5 와 한 묶음으로 검토하기 어렵다. 그래서 세 조각으로 나누고 **이번 JOB 은 004a 만 정의한다**(한 JOB 안에 커밋 2개로 나누는 대안은 두지 않는다 — 직전 JOB 리뷰 권고).

| 조각 | 내용 | 예상 줄 | 이 명세 |
|---|---|---|---|
| **004a** | D5: 도판·3D·내보내기 기본 **외면 왼쪽 · 단면 오른쪽**, `R` = 좌/우 반전(sectionSide), 라벨·UI_RULES R5 갱신, `studio_side` 시험 | 제품 ≈ 140 · 테스트 ≈ 210 | **정의함** |
| 004b | 실시간 단면 엔진·연결: 끄는 동안 LOD 단면 1장, 놓으면 원본 전체 계산, 디바운스, 확정 전 무시, Y 막 0°/X 막 90° | 제품 ≈ 170 · 테스트 ≈ 130 | split_out (씨앗 §후속 1) |
| 004c | `② 막으로 자르기` 리본 페이지·ctx·상태줄 원문 라벨·잠김 안내·`--cut` 캡처 | 제품 ≈ 200 · 테스트 ≈ 10 | split_out (씨앗 §후속 2) |

004a 를 먼저 두는 이유: 디자인 원칙 "3D 의 왼쪽·오른쪽이 도판의 외면·단면과 늘 같다"(design-excerpt.md 원칙 1)는 D5 배치가 먼저 맞아야 성립하고, 004b·004c 의 시험(단면 점 x ≥ 0 등)이 이 부호 규약 위에 선다.

## 1. 목표 (004a)

사용자 요청(job.json `request`, 코디네이터 릴레이):
> 07:45 도판 규약 D5(외면 왼쪽·단면 오른쪽, R 반전) … 코디네이터 기본값: … **mirrorX 결함 수정 포함**.

D5 확정값(project-profile.md 결정 D5): "**외면 왼쪽 · 단면 오른쪽**, R 키로 좌우 반전 지원 … **현재 Studio·UI_RULES R5 는 반대(왼쪽 단면)** → 백로그 결함 6".

**재진술(한 문장)**: 화면 도판·3D 보조창·내보내기(SVG/PDF/PNG/DXF) 모두 기본으로 단면을 중심축 오른쪽(x ≥ 0)에, 외면(외곽선·음영)을 왼쪽(x ≤ 0)에 그리고, `R`(또는 리본 단추)은 같은 단면을 좌우 거울상으로 뒤집는 표시 전환으로 바꾼다 — 계산값(치수·단면 형상)은 그대로다.

### D5 적용 범위 (기본 = `SectionSide::Right`)

| 출력 | 기본(Right) | `R` 뒤(Left) | 현재(e9b655a) |
|---|---|---|---|
| `displaySection/ScannedSolid/EstimatedInner/Hatch` | x ≥ 0 | x ≤ 0 (x 부호만 바뀜) | x ≤ 0 (`mirrorX`) |
| `displayOutline` | x ≤ 0 | x ≥ 0 | x ≥ 0 |
| 도판 음영(`renderShade`) | 정면(카메라 −Y, yaw 0°)에서 본 x<0 반쪽 → 왼쪽 절반 | 뒤(yaw 180°) → 오른쪽 절반 | yaw 180° → 오른쪽 |
| SVG/PDF/PNG/DXF·내보내기 미리보기 | 화면과 같음 | 화면과 같음 | 화면과 같음(왼쪽 단면) |
| 3D 보조창 | 단면 쪽 반(물리 x ≥ 0)을 굵게, 보는 방향 그대로 | 보는 방향 +180°(뒤에서) → 굵은 반이 화면 왼쪽 | 양쪽 같은 굵기 |

`Left` 의 출력은 현재 코드의 출력과 같은 배치("도판을 뒤(+Y)에서 본 모양")다.

## 2. 범위 밖 (004a)

- 실시간 단면(LOD 미리보기·놓으면 원본·디바운스·속도 기준), `② 막으로 자르기` 탭·그룹·ctx·상태줄·`--cut`, 막 방향 Y/X 단추, ← → 1°·Shift 15° 키 → 004b·004c.
- Z 막·각도 띠·추천·실시간 확인 줄·앞쪽 반 흐리게·보라 점·한 바퀴 훑기·지금 각도 고정·도면 편집으로·회전판 빨간 손잡이·중심선 고정 풀기·V 도판 방향 보기 → JOB-005 이후.
- 정면각(`frontAngle`)의 부호·0° 기준·3D 끌기 방향 변경(현행 그대로, 사람 검토 항목 2 에 기록만).
- 기관 프리셋(국내·일본·영국 …)과 좌/우 설정의 디스크 저장(QSettings)·되돌리기 스택 편입.
- `docs/ui/mockup/*`·`dongarch3d_design_spec.md` 의 좌/우 문구 갱신(백로그 8 디자인 v2 반입 때), `DEV_PROGRESS.md`, 축 색 토큰(R2 변경).
- `GigaMesh/core`·`external`·`GigaMesh/gui/src/dongarch/karch/*` 수정. (K-Arch 자체 출력 `WritePlateSvg/WritePlateDxf` 는 이미 외곽 x<0·단면 오른쪽 — `Silhouette.h:21` `leftOutline … x < 0` — 이라 D5 와 맞고, Studio 는 쓰지 않는다.)
- `factory/project-profile.md`·`backlog.md` 기록 갱신(오케스트레이터 몫, builder 는 손대지 않음).

## 3. 설계 결정 (builder 가 그대로 구현할 기본값)

### 3.1 ArtifactDoc (`studio/src/ArtifactDoc.h/.cpp`)

```cpp
// 도판에서 단면이 놓이는 쪽. D5(2026-10-17 확정): 기본 Right = 외면 왼쪽·단면 오른쪽(국내 보고서 표준),
// Left = R 로 좌/우 반전(같은 단면의 거울상 = 도판을 뒤에서 본 모양). 계산값은 바꾸지 않는다.
enum class SectionSide { Right, Left };

void setSectionSide(SectionSide s);          // 다시 계산하지 않는다 — 그리기 부호만
SectionSide sectionSide() const;
void toggleSectionSide();
double sectionSign() const;                  // Right → +1.0, Left → −1.0
```

- 멤버 `SectionSide side_ = SectionSide::Right;`. **`loadMesh/loadFile/loadSample/loadArtecBlindSample/confirmPose/unconfirm` 은 바꾸지 않는다**(한 세션 동안의 사용자 선택 — 사람 검토 1). 확정 전에도 값은 바뀔 수 있으나 그릴 것이 없다.
- **`mirrorX`(ArtifactDoc.cpp:108-112) 삭제.** 부호는 간략화(`simplifyDP`)·곡선 다듬기(`smoothCatmull`)가 **모두 끝난 뒤 마지막에** x 에 `sectionSign()` 을 곱한다(곱셈 `×(−1.0)` 은 정확한 부호 반전 → Left = Right 의 x 부호만 바꾼 값, 오차 0).
  - 자연 좌표: 단면 고리는 이미 x ≥ 0 반평면으로 잘려 있다(`buildSectionLoops` 798-809, `estInner` 는 `std::max(0.0, …)` 936). 외곽 `sil_.leftOutline` 은 x < 0(`Silhouette.h:21`). 그래서 **단면과 외곽 모두 같은 인수 `sectionSign()`** 을 곱한다(Right: 그대로, Left: 둘 다 부호 반전).
  - `displaySection`(692-713, 706 의 mirrorX), `displayScannedSolid`(715-725, 718), `displayEstimatedInner`(727-738, 731), `displayOutline`(740-745, 741) 을 위 규칙으로. 미스캔 합성 경로(693-703)는 자연 좌표 실선·파선으로 다각형을 만든 뒤 부호를 곱한다(private 도우미 `…Natural()` 계열로 나눠도 됨 — 이름은 builder 재량).
  - `displayHatch`(747-778): **부호 적용 전(자연 좌표) 단면**으로 빗금을 계산한 뒤 끝점 x 에 같은 부호를 곱한다(빗금도 정확한 거울상 — Right 는 기울기 +1 "/", Left 는 "\").
- `resetSlots` 의 수직 슬롯 meta `"좌단면·우외곽"`(677) → `"종단면"`(좌/우는 도판 머리에 표시하므로 슬롯에 넣지 않는다). 헤더 주석(ArtifactDoc.h:24 `"좌단면·우외곽 · 184.5 mm"`, 111-112 `토기 표준: 단면 왼쪽 / 외곽 오른쪽`)을 D5 문구로.
- 그 밖의 계산(`recompute`, `buildSectionLoops`, `wallThicknessAt`, `dims`, `frontAngle`, `pointCount/rawPointCount`)은 한 글자도 바꾸지 않는다.

### 3.2 Plate (`studio/src/Plate.h/.cpp`)

- `renderShade`(113-131): `v.yawDeg = doc.sectionSide() == SectionSide::Right ? 0.0 : 180.0`(121). yaw 0° 는 카메라가 −Y 에서 +Y 를 보므로(MeshRaster.h:22) 영상 왼쪽 절반 = 월드 x<0 반쪽의 정면 외면. Plate.h:35 주석 갱신.
- `paint` 음영 자르기(252-262, 255-256): 외면 쪽 절반만 — Right: `x ∈ [rectMm.left, 0]`, Left: `x ∈ [0, rectMm.right]`(현행).
- SVG 음영 층(625-646): Right 는 영상 **왼쪽** 절반 `copy(0, 0, half, h)` 을 `x = X(sh.rectMm.left())`, 폭 `rectMm.width()/2 × s` 로, Left 는 현행(오른쪽 절반을 `X(0)`). 630 주석 `오른쪽 절반만` → `외면 쪽 절반만`.
- DXF·SVG 선 층은 `display*()` 를 그대로 쓰므로 자동으로 따라온다. 713 주석 `화면과 같은 좌단면·우외곽` → `화면과 같은 배치(D5 기본 외면 왼쪽·단면 오른쪽, R 반전 따름)`. `extentMm`·`maxR` 는 `fabs` 라 좌우 대칭(20-28, 98-111) — 무변경.

### 3.3 SheetView (`studio/src/SheetView.cpp`)

- 머리 부제(239): 수직 슬롯이면 Right `외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)`, Left `단면 왼쪽 · 외면 오른쪽 (R 반전)`. 미스캔 꼬리 문구(241-242)는 그대로 덧붙인다.
- 기벽 두께 풍선(94-125): 조건 `hoverMm_.x() < 0 && > −rOut−15`(101) → `s·x > 0 && s·x < rOut + 15`(s = `sectionSign()`), 점 `(s·rOut, h)`(103). 풍선 상자는 축에서 먼 쪽(Right: 점 오른쪽 `dot.x + 14`, 화면 밖이면 왼쪽; Left: 현행)(113-114). 94 주석 `단면 쪽(왼쪽)` → `단면 쪽`.

### 3.4 View3D (`studio/src/View3D.cpp`)

- `view()`(57-82): **확정 뒤에만** `v.yawDeg = yaw_ + (sectionSide()==Left ? 180.0 : 0.0)` — R 로 도판을 뒤집으면 3D 도 뒤에서 보아 "3D 왼쪽·오른쪽 = 도판" 을 지킨다. 확정 전(바닥판) 화면은 무변경.
- 수직 슬롯 절단선(144-153): 각 선분의 중점 x(월드, `lodMesh` 좌표)로 나눈다 — **x ≥ 0(단면 쪽) = 지금처럼 흰 5 px + `ProfileRed` 2 px**, x < 0 = `ProfileRed` 1 px 파선(흰 테 없음). 수평 슬롯은 무변경. 새 색 없음(R2).

### 3.5 MainWindow·문구 (`studio/src/MainWindow.cpp`, `main.cpp`, `ExportDialog.cpp`, `StartPage.cpp`)

- 리본 small 단추 `combined`(142, id 유지): 글자 Right `외면 왼쪽·단면 오른쪽` / Left `단면 왼쪽·외면 오른쪽 (반전)`, kbd `R`, 툴팁 `국내 보고서 표준 (D5) · R 로 좌/우 반전 — 같은 단면을 거울상으로 그립니다`. 상태는 지금처럼 `On`(218). 누르면 `R` 과 같다. `needPose`(708-709)에 `"combined"` 추가.
- **`R` 키(871)**: `pushUndo(); setFrontAngle(frontAngle()+180)` → `doc_.toggleSectionSide(); ribbon_->setText("combined", …); refreshAll(true);` + 상태줄 안내 Right `단면 위치: 외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)` / Left `단면 위치: 단면 왼쪽 · 외면 오른쪽 (R 반전) — R 을 다시 누르면 되돌립니다`. 되돌리기 스택에 쌓지 않는다. 확정 전 가드(828)는 그대로(R 은 이미 막힘).
- ctx 단면 도구 단축키 묶음(250-254)에 `kbdLabel("R")` + `좌/우 반전` 2개 위젯 추가(R8).
- `saveSlotAngle` meta `"좌단면·우외곽"`(671) → `"종단면"`.
- `ExportDialog.cpp:126` `(우측 300 DPI 이미지)` → `(외면 쪽 300 DPI 이미지)`. `StartPage.cpp:231` `우측에 D-Tak 탁본이 결합됩니다` → `외면 쪽(왼쪽)에 D-Tak 탁본이 결합됩니다`.
- **검수용 CLI `--flip`**(`main.cpp` 34-51 의 `--shot` 블록 안, 24-26 주석에 사용법): `QTimer::singleShot(200, …)` 로 `Qt::Key_R` 키 이벤트를 창에 보낸다(`--max` 와 같은 방식). 예: `DongArch3DStudio.exe --sample --flip --shot shot_flip.png`.

### 3.6 문서 정본 (같은 커밋)

- `docs/ui/UI_RULES.md` R5 72행을 다음으로 바꾼다: `- **회전체(토기·도자기)**: 중심축 기준 **왼쪽 = 외면(외곽선 + 문양/탁본, 먹색), 오른쪽 = 기벽 종단면(빨강 + 연한 빨강 채움)** — 국내 보고서 표준(D5, 2026-10-17 확정). \`R\` 로 좌/우 반전(같은 단면의 거울상 — 도판을 뒤에서 본 모양). 중심축은 먹색 실선.`
- 머리말(3-6행) 끝에 한 줄: `> 단, 토기 도판의 좌/우 배치는 목업(mockup_01·02, design_spec — 왼쪽 단면)이 아니라 R5(D5)를 따른다. 목업 갱신은 디자인 v2 반입 때.`
- R6(81-82) 뒤에 한 줄: `- SVG·PDF·PNG·DXF 와 내보내기 미리보기의 좌/우는 화면 도판(R5)과 같다.` 검수 체크리스트(97-104)에 `- [ ] 토기 도판 외면 왼쪽·단면 오른쪽(R5), R 반전 시 거울상`.
- `CLAUDE.md` 10행(`미결(사용자 결정 대기): 토기 도판 좌/우 배치 … 결정 전에는 배치 코드를 바꾸지 않는다.`)을 `- **토기 도판 좌/우(D5, 2026-10-17 확정)**: 외면 왼쪽 · 단면 오른쪽(국내 보고서 표준), \`R\` 로 좌/우 반전. 근거: \`factory/project-profile.md\` D5.` 로(사람 검토 4).

## 4. 수용 기준 (각 항목은 명령 하나로 참/거짓)

허용오차 근거(새로 만든 값 없음): 기하 항등은 K-Arch 시험의 **1e-9 mm**(`test_synthetic.cpp`·`test_dimensions.cpp`), 거울상은 코디네이터 지정 **1e-12 mm**(부호 곱셈만이라 실제 0), 단면·외곽이 축 너머로 나가는 곡선 다듬기 여유는 기존 **0.5 mm**(`test_pipeline.cpp:56-57`), 파일 좌표는 기록 자릿수(SVG 소수 3자리 `n3`, DXF 6자리 `DxfWriter.cpp:59`)의 반올림 2배.

### A. 새 CTest `studio_side` (`studio/tests/test_side.cpp`, 실행 파일 `test_studio_side`, 인자 = 출력 폴더, 오프스크린 `QGuiApplication`)

예제 S ∈ {`loadSample()`, `loadArtecBlindSample()`} 각각에 대해(둘 다 확정 상태):

1. **기본값**: 새 `ArtifactDoc` 의 `sectionSide() == SectionSide::Right`.
2. **(a) D5 부호, 곡선 끔**: `setSmooth(false)` 뒤 `displaySection()`·`displayScannedSolid()`·`displayEstimatedInner()` 의 모든 점 `x ≥ 0.0`(허용오차 없음), `displayOutline()` 의 모든 점 `x ≤ 0.0`. 각각 비어 있지 않음(미스캔 예제는 `hasEstimatedInner()`).
3. **(a) D5 부호, 곡선 켬(기본)**: `setSmooth(true)` 뒤 단면 3종·`displayHatch()` 끝점 모두 `x ≥ −0.5`, 외곽 모두 `x ≤ +0.5`. 최솟값·최댓값을 인쇄.
4. **(a) R = 거울상**: 곡선 켬·끔 각각에서 Right 의 단면 3종·외곽·빗금을 저장 → `setSectionSide(Left)` → 크기가 같고 모든 점 `|x_L + x_R| ≤ 1e-12`, `|y_L − y_R| ≤ 1e-12`. 그리고 `dims()` 6개 값·`frontAngle()`·`rawPointCount()`·`pointCount()`·`wallThicknessAt(z)`(z = 10, 20, …, 180 mm) 가 반전 전과 **정확히 같음**(`==`). `toggleSectionSide()` 한 번 더 → Right 출력이 처음 저장값과 정확히 같음.
5. **세션 유지**: `setSectionSide(Left)` → `loadSample()` → `sectionSide() == Left`(로드가 되돌리지 않음).
6. **(b) SVG 같은 쪽**: Right·Left 각각 `exportSvgLayered`(기본 `ExportRequest`, A4 세로 1:1) → 축의 SVG x `X0 = layoutPaper(doc,0,A4Portrait,1).plateRectMm.left() − Plate::extentMm(doc,0).left()`. `section_profile` 층의 모든 `points` x: Right `≥ X0 − 0.5`, Left `≤ X0 + 0.5`. `outline_axis` 층 polyline `points` x: Right `≤ X0 + 0.5`, Left `≥ X0 − 0.5`. `shade_dtak` 의 `<image>`: Right `x + width ≤ X0 + 0.01`, Left `x ≥ X0 − 0.01`. 두 파일의 단면·외곽 점 목록은 길이가 같고 `|(xL−X0) + (xR−X0)| ≤ 0.002`, `|yL − yR| ≤ 0.001`.
7. **(b) DXF 같은 쪽**: Right·Left 각각 `exportDxf` → `VERTEX` 의 코드 10 값(층별): `04_SECTION_WALL`·`06_INTERIOR_OUTLINE` 은 Right `≥ −0.5` / Left `≤ 0.5`, `02_EXTERIOR_OUTLINE` 은 Right `≤ 0.5` / Left `≥ −0.5`. 층별 VERTEX 수가 같고 `|xL + xR| ≤ 2e-6`.
8. **(b) 래스터(화면·PNG·PDF 공용 `paintSheet`→`Plate::paint`)**: `loadSample()` 로 A4 1:1, 4 px/mm `QImage` 에 (i) 단면만(grid·shade·outline·labels 끔) (ii) 음영만 그린다. 도판 사각형(`plateRectMm × 4`) 안에서 비흰 화소(R·G·B 중 하나 < 245)를 축 x 기준 여유 m = 5 px(= 0.5 mm × 4 + 3) 밖 왼쪽·오른쪽으로 센다. Right: (i) 왼쪽 0 · 오른쪽 ≥ 1000, (ii) 오른쪽 0 · 왼쪽 ≥ 1000. Left: 반대. 개수 인쇄.
9. `test_studio_side` 는 실패가 없으면 마지막 줄 `PASS`, 종료 코드 0.

### B. 회귀

10. `studio/tests/test_pipeline.cpp:51-57` 만 바꾼다: 주석을 `D5: 외면(외곽) 왼쪽 x<0 · 단면 오른쪽 x>0` 로, 검사를 `secMinX >= -0.5`("section must be right") · `olMaxX <= 0.5`("outline must be left") 로 뒤집고, 이어서 `doc.setSectionSide(SectionSide::Left)` 아래에 **원래 두 검사(`secMaxX <= 0.5`, `olMinX >= -0.5`)를 숫자 그대로** 두고 `setSectionSide(Right)` 로 되돌린다. 다른 기대값·허용오차(H92 교차 폭 등 부호 무관)는 그대로이고 통과.
11. 지금과 같은 값(verify.ps1 직후 `B\Testing\Temporary\LastTest.log`): `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180"` ≥ 1, `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"` ≥ 1, `grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"` ≥ 1 (기준: `D:\_dongarch_build\verify_merged\studio\Testing\Temporary\LastTest.log`. `pts=85/1318` 은 간략화·곡선 연산이 x 부호에 대칭이라 같아야 한다 — 다르면 builder 가 원인을 build-log.md 에 적는다).
12. CTest 14종(기존 13 + `studio_side`): `B` 에서 `ctest --output-on-failure` 출력에 `100% tests passed, 0 tests failed out of 14`.
13. verify.ps1 종료 코드 0, `D:\_dongarch_build\verify_job004\studio\verify-summary.md` 마지막 줄 `결과: PASS`, 빌드 경고 0: `grep -ci "warning" D:/_dongarch_build/verify_job004/studio-logs/02_build.log` = `0`.

### C. 코드·문구·정본 (Git Bash, `cd W`)

14. 결함 기호 제거: `grep -c "mirrorX" studio/src/ArtifactDoc.cpp` = `0`, `grep -c "frontAngle() + 180" studio/src/MainWindow.cpp` = `0`.
15. 옛 문구 없음: `grep -rnE "좌단면·우외곽|토기 표준|좌측 종단면|우측 외곽|우측 300 DPI|우측에 D-Tak" studio/src` → 출력 없음.
16. 새 문구 있음: `for s in "외면 왼쪽·단면 오른쪽" "단면 왼쪽·외면 오른쪽 (반전)" "외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)" "단면 왼쪽 · 외면 오른쪽 (R 반전)" "좌/우 반전" "외면 쪽 300 DPI"; do grep -rqF "$s" studio/src || echo "MISSING $s"; done` → 출력 없음.
17. 정본: `grep -c "왼쪽 = 기벽 종단면" docs/ui/UI_RULES.md` = `0`, `grep -c "오른쪽 = 기벽 종단면" docs/ui/UI_RULES.md` ≥ 1, `grep -c "D5, 2026-10-17 확정" CLAUDE.md` ≥ 1, `grep -c "미결(사용자 결정 대기)" CLAUDE.md` = `0`.
18. R2·R3: `git diff e9b655a -- studio/src | grep -E '^\+.*(QColor\([0-9]|qRgba?\([0-9]|#[0-9A-Fa-f]{6}|Qt::(red|green|blue|yellow|white|black|gray|darkGray|lightGray)\b)'` → 출력 없음(옮기기만 한 줄이면 code-reviewer 가 확인), `git diff e9b655a -- studio/src | grep -c '^\+.*ToolState::Primary'` = `0`.
19. 변경 범위: `git diff --name-only e9b655a...HEAD` 의 모든 줄이 §6 영향 파일 표에 있고, `git diff --name-only e9b655a...HEAD -- GigaMesh studio/tests/test_pose.cpp` → 출력 없음.

### D. 캡처·육안 (ui-reviewer, 항목마다 예/아니오)

20. 검증 명령 3 이 `B\shots\shot_flip.png` 를 만들고 크기 `> 5120` 바이트, verify.ps1 의 `shot_home.png`·`shot_main.png`·`shot_export.png` 도 각각 `> 5120`.
21. `shot_main.png`: 도판의 빨간 단면이 중심축 **오른쪽**, 먹 외곽선·음영이 **왼쪽**. 머리 부제 `외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)`. 리본 `외면 왼쪽·단면 오른쪽` + `R`. 3D 의 굵은(흰 테) 빨간 절단선이 유물 오른쪽 반에 있고 왼쪽 반은 가는 파선.
22. `shot_flip.png`: 도판이 21 의 좌우 거울상(단면 왼쪽), 부제 `단면 왼쪽 · 외면 오른쪽 (R 반전)`, 리본 `단면 왼쪽·외면 오른쪽 (반전)`, 3D 를 뒤에서 보아 굵은 절단선이 화면 왼쪽 반.
23. `shot_export.png`(내보내기 미리보기)가 21 과 같은 배치. (오프스크린 한글 □ 는 알려진 위험 7 — 불합격 사유 아님.)

## 5. 검증 명령

1. 표준 검증: `powershell -NoProfile -ExecutionPolicy Bypass -File D:/_dongarch_build/wt/.worktrees/JOB-004/factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-004 -BuildRoot D:\_dongarch_build\verify_job004 -Job JOB-004 -RecordRoot D:\_dongarch_build\wt` (= 템플릿의 `verify.ps1 -Job JOB-004`. 로그 `L\01_configure.log … 04_shots.log`, 산출물 `D:/_dongarch_build/wt/factory/jobs/JOB-004/artifacts/`.) 직후 수용 기준 11 의 grep.
2. CTest 재확인(PowerShell, 빌드 폴더로 이동 — `--test-dir` 금지): `$env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job004\studio; ctest --output-on-failure` · 개별 `ctest -R studio_side -V`(기준 1-9 인쇄값), `ctest -R studio_pipeline -V`. 개별 실행은 LastTest.log 를 덮어쓰므로 기준 11 은 1 직후 또는 전체 ctest 재실행 뒤에 본다.
3. 반전 캡처(verify.ps1 무변경): 위와 같은 환경 변수 뒤 `Start-Process -Wait -WorkingDirectory D:\_dongarch_build\verify_job004\studio -FilePath D:\_dongarch_build\verify_job004\studio\DongArch3DStudio.exe -ArgumentList '--sample','--flip','--shot','D:\_dongarch_build\verify_job004\studio\shots\shot_flip.png'; (Get-Item D:\_dongarch_build\verify_job004\studio\shots\shot_flip.png).Length` → 5120 초과면 참, 그 뒤 `artifacts\` 로 복사. (Git Bash 에서는 명령 전체를 작은따옴표로 감싼 `powershell -NoProfile -Command '…'`.)
4. 캡처 비교: `shot_main.png` 를 JOB-003 산출물 `D:/_dongarch_build/wt/factory/jobs/JOB-003/artifacts/shot_main.png` 와 나란히 — 도판이 좌우 거울상이고 치수 띠 값은 같아야 한다(육안).
5. 수용 기준 14-19 는 Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-004` 뒤 그대로.

## 6. 영향 파일

| 파일 | 변경 | 이유 |
|---|---|---|
| `studio/src/ArtifactDoc.h` | 수정 | `SectionSide`·`setSectionSide/sectionSide/toggleSectionSide/sectionSign`, 멤버 `side_`, 주석 24·111-112 |
| `studio/src/ArtifactDoc.cpp` | 수정 | `mirrorX` 삭제(108-112), `display*`(692-745) 부호 마지막 적용, `displayHatch`(747-778) 자연 좌표 계산, 슬롯 meta(677) |
| `studio/src/Plate.h` · `Plate.cpp` | 수정 | 음영 yaw(121)·자르기(255-256)·SVG 음영 층(625-646), 주석(Plate.h:35, Plate.cpp:630·713) |
| `studio/src/SheetView.cpp` | 수정 | 머리 부제(239), 기벽 풍선 쪽(94-125) |
| `studio/src/View3D.cpp` | 수정 | 반전 시 yaw +180°(`view()` 57-82), 단면 쪽 반 굵게(144-153) |
| `studio/src/MainWindow.cpp` | 수정 | `combined` 글자·동작(142, 708-709, onAction), `R`(871), ctx 키 묶음(250-254), 슬롯 meta(671) |
| `studio/src/main.cpp` | 수정 | `--flip`(24-51) |
| `studio/src/ExportDialog.cpp` · `StartPage.cpp` | 수정 | 문구 1줄씩(126 · 231) |
| `studio/tests/test_side.cpp` | 신규 | 수용 기준 1-9 |
| `studio/tests/test_pipeline.cpp` | 수정 | 51-57(수용 기준 10) |
| `studio/CMakeLists.txt` | 수정 | 91-93 다음에 `test_studio_side` 실행 파일·`add_test(NAME studio_side COMMAND test_studio_side ${CMAKE_CURRENT_BINARY_DIR})` |
| `docs/ui/UI_RULES.md` | 수정 | 머리말 예외 1줄, R5 72행, R6 1줄, 체크리스트 1줄 |
| `CLAUDE.md` | 수정 | 10행 D5 확정 문구 |

`GigaMesh/**`, `studio/src/MeshIO.*`·`MeshRaster.*`·`Ribbon.*`·`Theme.h`, `studio/tests/test_pose.cpp`, `factory/**`, `docs/ui/mockup/**` 는 수정하지 않는다.

## 7. 위험·가정

- **좌표계**: mm·+Z 위·바닥판 Z=0·확정 뒤 중심선 x=y=0 그대로. 단면 데이터는 지금처럼 "정면각 θ 만큼 돌린 메시의 Y=0 평면, x ≥ 0 반쪽"(ArtifactDoc.cpp:627-630, 790-809)이고 이 JOB 은 **도면 좌표의 x 부호만** 바꾼다. 도면 x = 부호 있는 반경 R(상태줄 커서 `R` 은 단면 쪽이 +).
- **곡선 다듬기 넘침**: Catmull–Rom 이 축(x=0) 너머로 조금 나갈 수 있어 곡선 켬 검사는 기존 0.5 mm 여유를 쓴다(곡선 끔은 0 여유).
- **부동소수 대칭 가정**: `simplifyDP`·`smoothCatmull`·빗금은 x 에 대해 홀함수 연산(곱·합·`fabs`·`hypot`)이라 `/fp:fast`·AVX2 에서도 Right 결과 = 옛 Left 결과의 정확한 거울상 → 수용 기준 11 의 `pts=85/1318` 불변. 부호는 마지막에 곱하므로 기준 4 는 이 가정과 무관하게 0 오차.
- **UI 정본 충돌**: UI_RULES 머리말 "목업과 코드가 다르면 코드가 틀린 것"과 목업(왼쪽 단면)이 부딪힌다 → 예외 1줄로 R5 우선을 명시(사람 검토 4). 목업 갱신은 백로그 8.
- **사용자 흐름 변화**: `R` 이 "정면각 +180°" 에서 "좌/우 반전" 으로 바뀐다. 이미 내보낸 SVG/DXF 는 옛 배치 그대로 남고, 다시 내보내면 D5 배치가 된다(프로젝트 파일 저장 기능 없음).
- **리본 폭**: 새 글자(11자)가 옛 글자(17자)보다 짧아 1560 px 에서 넘침 없음(육안 21).
- **오프스크린 한글 □**(알려진 위험 7): 캡처는 배치·크기로만 판정.
- **라이선스**: 새 의존성 없음.

## 8. 사람 검토 항목

1. **D5 적용 범위와 R 의 의미**: 화면 도판·3D·SVG/PDF/PNG/DXF·내보내기 미리보기 모두 기본 외면 왼쪽·단면 오른쪽. `R` = **같은 단면의 거울상**(도판을 뒤에서 본 모양, 3D 도 뒤로 돌아감) — 대안 (나) 현행 "정면각 +180°"(유물을 반 바퀴 돌려 반대쪽 반을 단면으로; 비대칭 유물이면 단면 모양이 달라짐)는 없앤다. 좌/우 선택은 세션 동안 유지(파일을 바꿔 열어도), 디스크에 저장하지 않고 되돌리기에 쌓지 않는다. 빗금도 거울상(Right "/", Left "\").
2. **막 회전 = 유물 회전 각도 부호·0° 기준**(004a 는 바꾸지 않음, 004b 에서 결정): 현행 `frontAngle` θ 는 메시를 +Z 축으로 −θ(위에서 보아 시계 방향) 돌리고(ArtifactDoc.cpp:627-630) 0° 는 확정 자세의 Y=0 평면(정면 = −Y 쪽, 카메라 MeshRaster.h:22)이다. 3D 에서 단면 도구로 오른쪽으로 끌면(View3D.cpp:179-181, +0.5°/px) **앞면이 왼쪽으로** 가는데, 보는 방향 끌기(yaw, 183)는 앞면이 오른쪽으로 간다 — 두 끌기의 방향이 반대다. 디자인의 "회전판 손잡이" 감각과 맞추려면 004b 에서 부호를 정해야 한다. 004b 기본값 제안: Y 막 = 0°, X 막 = 90°(원래 X=0 평면, 옆(+X)에서 봄).
3. **미리보기(LOD)와 원본 단면의 차이**: 004a 해당 없음. 004b 에서 측정·기록만(공식 허용오차 없음).
4. **UI 정본 변경**: UI_RULES R5 반대로 뒤집기, 머리말 목업 예외, R6·체크리스트 줄 추가, CLAUDE.md §1 "미결" 줄을 D5 확정으로 — CLAUDE.md §1 "바꾸려면 먼저 사용자 승인" 대상(D5 확정·JOB-004 요청을 근거로 진행, 승인 때 재확인).
5. **슬롯 meta**: `좌단면·우외곽` → `종단면`(좌/우는 슬롯이 아니라 도판 전체 설정이므로).

## 9. 크기

| 묶음 | 파일 | 예상 줄(추가+수정) |
|---|---|---|
| 엔진 | ArtifactDoc.h/.cpp | ≈ 45 |
| 그리기·내보내기 | Plate.h/.cpp | ≈ 25 |
| 화면 | SheetView.cpp, View3D.cpp, MainWindow.cpp, main.cpp, ExportDialog.cpp, StartPage.cpp | ≈ 65 |
| 정본 | UI_RULES.md, CLAUDE.md | ≈ 6 |
| 테스트 | test_side.cpp(신규), test_pipeline.cpp, CMakeLists.txt | ≈ 210 |
| **합계** | | **≈ 350 (제품 ≈ 140, 테스트 ≈ 210)** |

제품 코드는 300줄 지침 안이다. 테스트가 절반을 넘는 것은 SVG·DXF·래스터 세 출력의 좌/우를 각각 파싱해 확인하기 때문이며, 테스트를 떼어 내면 바뀐 배치를 검증할 수 없으므로 더 나누지 않는다.

## 10. 후속 조각 씨앗 (이번 JOB 아님 — 등록·명세는 코디네이터 몫)

1. **JOB-004b 실시간 단면**: `previewFrontAngle(deg)`(확정 전 무시, `lodBase_` 만 −deg 회전 → Y=0 단면 1장 → 미리보기 전용 버퍼, 외곽·치수·두께·슬롯·`mesh_` 그대로) / `commitPreview()` = `setFrontAngle(같은 각)`. **함정**: `buildSectionLoops` 는 `estimatedWallMm_`·`estZMin_/Max_` 를 바꾼다(ArtifactDoc.cpp:915, 939-943) → 미리보기가 이를 건드리면 놓은 뒤 결과가 달라진다; 순수 함수로 떼어 미리보기는 상태를 바꾸지 않게. 연결: 3D 끌기 놓기 = 즉시 원본, 키(`, . [ ] ← →`, Shift 15°)·단추 = 마지막 입력 뒤 300 ms(KLM 평균 타건 0.28 s 보다 길게) + 키를 누르고 있는 동안 보류, 미리보기 요청은 16 ms 로 합침(`GigaMesh/gui/src/dongarch/cutline/CutlineController.cpp:23-25` 16 ms ≈ 60 fps), 미리보기 중 음영 재계산 금지, 미리보기 중 내보내기 거부. 시험(`studio_cut`): (c) 미리보기 여러 번 뒤 놓기 == 새 문서 `setFrontAngle` (단면 3종·외곽·빗금·dims·두께·LOD 정점 ≤ 1e-9 mm, 두 예제), 끄는 동안 dims·외곽 불변 (d) 예제 LOD(26,180면) 미리보기 1장(`previewFrontAngle`+`displaySection`) 51회 중앙값 ≤ 16 ms, p90·최대·원본 `setFrontAngle` 시간 인쇄 (e) 확정 전 미리보기·놓기·막 맞춤·`setFrontAngle` 무시 (f) `alignCut(Y)`=0°, `alignCut(X)`=90°, 20×10×20 mm 상자에서 Y 막 단면 최대 x = 20, X 막 = 10 (≤ 1e-9). LOD↔원본 단면 거리(양방향 최대 최근접 거리)는 각도 0·12·45·90·137° 에서 측정·기록만.
2. **JOB-004c ② 막으로 자르기 페이지**: 탭 `② 막으로 자르기`(UI_RULES R1 탭 줄 갱신), 확정 전 잠김 안내, 그룹 `막 방향 (누르면 바로 맞춤)` [Y 막 정면에서][X 막 옆에서] · `막 움직이기 (회전판)` 각도 [−][+] 표시·`← → 1° · Shift 15°`·`☑ 중심선에 고정 (돌리기만)`(표시만) · `단면 위치` [외면 왼쪽·단면 오른쪽] R · `국내 보고서 표준`, ctx `✂ 막으로 자르기`·상태줄 `⑥ 단면 보기 — …`·도판 머리 `A–A′ 실시간 단면 — 12°`, 리본 페이지 전환 신호(`Ribbon` 에 없음 — 추가 필요), `--cut` 캡처.
3. **JOB-005 이후**: Z 막, 각도 띠, 추천, 실시간 확인 줄, 앞쪽 반 흐리게, 보라 점, 한 바퀴 훑기, 지금 각도 고정, 도면 편집으로, 회전판 빨간 손잡이, 중심선 고정 풀기("보고서 단면 아님"), V 도판 방향 보기, 기관 프리셋 저장, 축 색 토큰.
