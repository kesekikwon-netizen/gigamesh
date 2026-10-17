# JOB-003 검토: correctness

결론: **PASS**

검토자: correctness-reviewer, 2026-10-17. 대상 `factory/JOB-003` HEAD `e9b655a`(기준 `58a33ae`, 커밋 `1ad49b4`·`e9b655a`). 명세 기준 6 은 decision.md 의 6a/6b 수정본으로 판정했다.
읽은 것: `git diff 58a33ae..HEAD`(15파일), spec.md, design-excerpt.md, decision.md, verify.md(추기 포함), build-log.md, K-Arch `DimensionAnalyzer.cpp`·`MeshLoader.cpp`(읽기만). 제품·테스트 코드는 고치지 않았다.
직접 실행(이미 빌드된 `D:\_dongarch_build\verify_job003\studio` 실행파일, 산출물은 `D:\_dongarch_build\verify_job003\review_scratch` 에만):

| # | 명령 (PowerShell, Qt6 bin·plugins·`QT_QPA_PLATFORM=offscreen`, 작업 폴더 review_scratch) | 종료 | 결과 |
|---|---|---|---|
| X1 | `test_studio_pose.exe <review_scratch>` | 0 | 90°×4·뒤집기×2 `max|d|=2.04e-14 mm`, 접지 `0`, 박스 중심 이동 `7.11e-15`, 처음 자세로 `0`, Y−74→자동→확정 `H=184.80 rim=142.00`, 6a `ok=0 H=200.50 rim=0.00 section=1 bboxMin.z=0`, 6b `ok=1 H=184.80 rim=142.00 slots=4`, loadFile bbox 차 `0`·`bboxMin.z=31.32`, 수동 확정 `H=184.80 rim=142.00`, `PASS` |
| X2 | `test_studio_pipeline.exe <review_scratch>` | 0 | `H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180`, `STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2`, `ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95` — 병합본 `verify_merged\...\LastTest.log` 의 같은 줄과 값 동일 |
| X3 | `DongArch3DStudio.exe --sample --base --export --shot rc_base_export.png` | 0 | 확정 전 Ctrl+P 주입 → **내보내기 창 없음**, ① 페이지·바닥판·도판 자리 문구 그대로(70,427 B) |
| X4 | `DongArch3DStudio.exe <review_scratch>\studio_pose_tilted.ply --export --shot rc_file_export.png` | 0 | 파일 열기(확정 전) + Ctrl+P → 내보내기 창 없음, 항아리가 바닥판 위 31 mm 에 떠 있음(접지 안 함) |
| X5 | `DongArch3DStudio.exe --sample --export --shot rc_sample_export.png` (대조) | 0 | 확정된 예제에서는 내보내기 창이 정상으로 뜸 → X3·X4 의 "창 없음"은 가드 때문 |

## 항목별 판정

| # | 점검 | 판정 | 근거 (file:line, 확인/추정) |
|---|---|---|---|
| 1 | 회전 행렬 = 고정 월드축·오른손 법칙 | 통과(확인-코드) | `ArtifactDoc.cpp:409` X: y'=c·y−s·z, z'=s·y+c·z (+90: +Y→+Z) · `:412` Y: x'=c·x+s·z, z'=−s·x+c·z (+90: +Z→+X) · `:415` Z: x'=c·x−s·y, y'=s·x+c·y (+90: +X→+Y). 매번 현재 정점에 R 을 곱함 = 왼쪽 곱 누적(외재적). 툴팁 방향 설명(`MainWindow.cpp:203-206`)도 카메라 −Y→+Y 규약(`MeshRaster.h:22`)과 일치 |
| 2 | 회전 중심 = 현재 AABB 중심 | 통과(확인) | `ArtifactDoc.cpp:419-422` 회전 전 `base_` 박스 중심, `rotateAbout`(379-391) 이 원본·LOD 에 같은 c·R 적용. X1 박스 중심 x·y 이동 7.11e-15 mm |
| 3 | 90° 정수배 = 정수 행렬, 누적 오차 | 통과(확인) | `cosSinDeg` `ArtifactDoc.cpp:83-95`: `|deg/90−k|<1e-12` 면 cos·sin ∈ {−1,0,1}(±90·180·270·음수 모두 `fmod` 로 0..3). X1 90°×4(X·Y·Z, ±)·뒤집기×2 = 2.04e-14 mm(허용 1e-9). 남는 오차는 `p−c`·`c+d` 두 번의 반올림뿐 → 조작 N 번에 ulp 수준 |
| 4 | 뒤집기 | 통과 | `ArtifactDoc.cpp:428` = X 180°: (x, y, z)→(x, −y, −z)(중심 기준). 반사가 아닌 회전이라 손잡이 방향(좌우손) 보존, 정면(−Y 카메라)에서 좌우 유지·위아래 반전 — 명세 사람 검토 2 와 같음 |
| 5 | 처음 자세로 오차 0 | 통과(확인) | `ArtifactDoc.cpp:433` 정점 사본 그대로 대입, LOD·LOD 색도 원 사본(`:435-436`), 접지 안 함. X1 `max|d|=0` |
| 6 | 매 조작 뒤 접지 | 통과(확인) | 회전 `:423`, 내려놓기 `:444`, 확정 `:504-505`, 자동 세우기 `:586`(기존). 최저점에서 `z−z` 는 IEEE 에서 정확히 0 → X1 `max|bboxMin.z|=0` |
| 7 | 원본·LOD·색 배열 일관성 | 통과 | 강체 변환은 정점 순서를 바꾸지 않아 `baseColors_`·`lodColors_` 색인 유지. 회전·평행이동은 `base_`·`lodBase_` 에 같은 변환(`:379-398`), 처음 자세로는 `rawLod_/rawLodColors_` 복원, 자동 세우기·확정은 LOD 재생성(`:589`, `:509`). `FillMeshStats` 생략(`:60-80` 주석)은 MeshData 통계가 bbox·평균 변 길이·경계 유무뿐이라(`RimDetector.h:19-26`, `MeshLoader.cpp:135-169`) 강체 변환에 안전 |
| 8 | 확정의 x·y 원점 맞춤 ↔ 단면 Y=0 평면 | 통과 | 수동 경로 `ArtifactDoc.cpp:482-492` = 자동 세우기 마지막 단계(`:560-568`)와 같은 규칙(`zBand(·,12)` 수평 외곽 원 중심 평균, 실패 시 박스 중심). 도판 단면은 `Slice(mesh_, Y=0)` 후 x≥0 반평면(`:790-809`), 3D 절단도 Y=0(`View3D.cpp:44`) → 중심선이 x=y=0 에 와야 자오 단면이 됨. 수동 확정(기울기 0°, 이동 (12,−5,3)) 이 rim 142.00 을 내므로 원점 맞춤이 실제로 작동함(X1, 구경은 Z 축 기준 실루엣·원 맞춤에 의존) |
| 9 | 자동 세우기→확정 = 기존 값 | 통과(확인) | `centered_` 경로(`:473-478`)는 재중심·재접지 없이(`bboxMin.z` 가 정확히 0 이라 `:504` 건너뜀) 기존 `autoAlign→resetSlots→recompute` 와 같은 순서. 확정 전 `recompute` 는 `estimatedWallMm_` 를 건드리지 않아(`:606-626`) 확정된 recompute 횟수도 기존과 같음. X2 값 병합본과 동일, X1 Y−74 눕힘→자동→확정 동일 |
| 10 | 확정 전 단면·치수·슬롯 비움 (엔진) | 통과(확인) | `recompute` 확정 전 분기 `:606-626`(sil·sec·dims·d5·기벽·단면 고리·추정 내벽 비움, `alignNote_="바닥 미정"`), `unconfirm` 이 슬롯 비움(`:464-468`), 내벽 보정 2함수 확정 전 생략(`:362-376`). 테스트 `checkUnconfirmed`(test_pose.cpp:60-68) 7항목이 loadSample(false)·loadFile·autoAlign 직후·확정 뒤 회전에서 통과 |
| 11 | 확정 전 단면·치수·슬롯 비움 (UI·내보내기) | 통과(확인) | 동작 가드 `MainWindow.cpp:708-710`(cut·outline·cutMove·addSlot·rotate·refBase·refRim·export·svg·dxf), Ctrl+P 도 `onAction("export")` 경유(`:816`), 단축키 `:823-836`, 되돌리기/다시 `:221-230`, 3D 끌기 `setFrontAngle` `:650-651`, 커서 좌표 `:104`, 도판 `SheetView.cpp:80-86`, 맞춤 `:45`, 3D 절단 `View3D.cpp:38`. `openExport` 호출처는 위 3곳뿐(grep). X3·X4 로 확정 전 Ctrl+P 가 창을 열지 않음을 실행 확인, X5 대조 |
| 12 | 확정 뒤 자세 조작 → 확정 해제 | 통과(확인) | `rotatePose/flip/reset/drop/autoAlign/rescaleUnits` 모두 `unconfirm()`. test_pose.cpp:165-168·186-188 통과. UI 는 `refreshAll` 이 확정 전 분기로 돌아가고 손에 든 도구를 내려놓음(`MainWindow.cpp:498-514`) |
| 13 | 단위 mm 불변 | 통과 | 자세 조작은 강체 변환뿐(배율 없음). `rescaleUnits` 는 호출처 없음(grep). 파일 열기의 단위 자동 환산은 이 JOB 이 바꾸지 않은 기존 `readMeshAny`(알려진 위험 2) — X2 `unit=m→mm 자동환산(×1000)` 그대로 |
| 14 | 파일 열기는 자세 불변 | 통과(확인) | `loadFile` `:244-259` autoAlign 제거, X1 bbox 6성분 차 0·`bboxMin.z=31.32`, X4 캡처에서 바닥판 위에 떠 있음 |
| 15 | 3D 화면 맞춤 변경이 확정 뒤 그림을 바꾸지 않음 | 통과 | `MeshRaster.cpp:23` `p.x − 0.0` 은 IEEE 에서 원값 그대로. verify.md: shot_home·shot_export 바이트 동일 |
| 16 | 테스트 ↔ 수용 기준 1:1 | 통과(약점 있음, 아래 제안 4·5) | 1→test_pose.cpp:90-97, 2→98-101, 3→81·84-104, 4→109-123, 5→126-147, 6a→150-169, 6b→170-189, 7→192-210, 8→213-227, 9→229-230, 10→test_pipeline.cpp:139-140 |

## 고치지 않아도 되는 제안

1. `MainWindow::openExport`(`MainWindow.cpp:677-679`)에 `poseConfirmed()` 검사를 한 줄 더 두면 이후 새 호출처가 생겨도 확정 전 내보내기가 막힌다(지금은 호출처 3곳 모두 가드 안이라 결함 아님).
2. 홈의 「중심축 맞춤」(`MainWindow.cpp:731-738`)은 `MainWindow::confirmPose()` 가 아니라 `doc_.confirmPose()` 를 직접 불러 `fitView()`·`recordRecent()` 가 빠진다. 확정 → ①에서 다시 돌림 → 홈 「중심축 맞춤」 순서면 `fitted_` 가 이미 참이라 도판 맞춤이 갱신되지 않는다(표시만의 문제).
3. 다시 확정해도 되돌리기/다시 스택(`undo_/redo_`)을 비우지 않아, 새 자세에서 Ctrl+Z 가 옛 자세 기준 정면 각도를 되살린다(`MainWindow.cpp:599-613`). 확정 때 비우는 편이 자연스럽다.
4. 회전 **방향**을 고정하는 테스트가 없다: 부호를 뒤집은 행렬(예: `R[1][2]=s, R[2][1]=−s`)도 90°×4·뒤집기×2·접지·박스 중심·Y−74→자동 세우기 검사를 모두 통과한다. "X+90 뒤 +Y 쪽 점이 +Z 쪽으로" 같은 단언 하나를 test_pose.cpp:90-97 근처에 추가 권장(코드는 읽어서 맞음을 확인함).
5. 수동 확정 뒤 중심선이 x=y=0 인지를 직접 단언하지 않는다(test_pose.cpp:213-227 은 rim 값으로 간접 확인). 박스 중심 x·y ≈ 0(±0.5 mm) 단언 추가 권장.
6. 확정 전에는 `alignNote_="바닥 미정"` 이라(`ArtifactDoc.cpp:623`) m→mm ×1000 같은 단위 자동 환산 메모가 바닥 정하기 단계 화면에 보이지 않고 확정 뒤 배지에서야 보인다(이전에는 열자마자 배지에 표시). 바닥판 1 cm 방안으로 크기를 보는 단계라 ctx 오른쪽이나 상태줄에 단위 메모를 함께 두면 좋다.
7. 3.5만 면 초과 메시는 자세 조작 때 LOD 를 원본 `bboxMin.z` 만큼 내리므로(`:423`, `:444`) LOD(군집 평균점)의 바닥이 판 위로 셀 크기 절반쯤 뜰 수 있다 — 3D 보조창 표시만의 문제.
8. 확정 경로에서 LOD 를 세 번 만든다(열기·자동 세우기·확정, builder 제안과 같음) — 대형 메시 시간 절약 여지.

## 확인 필요 (추정 — 실행으로 확인하지 못함)

- **기울어진 채 수동 확정할 때의 표시**: 6a 경로(7° 기울어짐)는 `H=200.50`(실제 184.5, +16 mm)·구경 0 이다(X1 확인). 이때 수평 단면이 거의 원이라 `FitHorizontalOuters` 원 맞춤이 성공해 `aligned_=true`(`ArtifactDoc.cpp:493`)가 될 것으로 **추정** → 도판 배지가 녹색 `✓ 기준: 바닥판 기준 (사람 확정) · 중심선 원 맞춤`(`SheetView.cpp:251-253`), 상태줄 오른쪽이 `단위 mm · 정위치 바닥수평·중심축`(`MainWindow.cpp:517-519`), 안내가 "실측선이 완성되었습니다 — 다음: Ctrl+P …"(`:522-523`)로 나와, 틀린 기고(200.5)를 정상 측정처럼 보이게 한다. 명세 1절 규칙 5(원 맞춤 성공 시 `aligned_=true`)와 "확정 뒤 문구는 지금 그대로"를 따른 결과라 이번 JOB 의 결함으로 잡지 않았다. CLI 로 확정을 주입할 수 없어 화면은 보지 못했다.
- **입이 아래로 향한 채 확정**: `AnalyzeDimensions` 는 구연 경계 고리가 `zMax−2 mm` 위에 있을 때만 구연으로 쓰고 아니면 `zMax−0.5 mm` 띠(=거꾸로 놓인 바닥)를 구경으로, `zMin+0.5 mm` 띠(=입)를 저경으로 맞춘다(`DimensionAnalyzer.cpp:85-111`, `:160-170`) → 구경·저경이 서로 바뀐 채 `dims().ok=true` 가 될 수 있다고 **추정**(예제는 구경=저경=142.00 이라 드러나지 않음).

## 사람 검토 항목

1. **좌표계**: 단위 mm(double), +Z 위, 바닥판 = Z = 0 평면. 확정하면 중심선(수평 외곽 원 중심 평균, z 12~88 % 구간 12 단면)을 x = y = 0 으로 옮기고(실패 시 박스 중심) 바닥을 Z = 0 에 접지한다. 도판 단면은 Y = 0 평면의 x ≥ 0 반쪽(`ArtifactDoc.cpp:790-809`), 3D 절단도 Y = 0(`View3D.cpp:44`), 카메라는 −Y 에서 +Y 를 봄(화면 x = 월드 x, 화면 위 = 월드 z). 원 좌표는 메모리에만 있고 확정 자세는 저장되지 않는다.
2. **수직 기준**: 수동 확정은 회전하지 않는다(사람이 정한 자세 = 수직). 자동 세우기를 쓴 경우에만 구연부 PCA + 수평 외곽 원 ODR 로 회전축을 +Z 로 맞춘다. 결과: 기울어진 채 확정하면 기고가 기울어진 높이(예제 7° → 200.5 mm, 실제 184.5)로 표시되고 구경은 "—"; 그때 배지·상태줄이 정상(✓·"바닥수평·중심축")으로 보일 수 있다(위 "확인 필요" 1). 입이 아래인 채 확정하면 구경/저경이 뒤바뀔 수 있다(추정). 기울기·상하 경고를 언제 넣을지 결정 필요(명세 사람 검토 0 의 후속).
3. **회전 합성 규약**: 단추 한 번 = 바닥판 고정 월드축 기준 오른손 법칙 +90°(Shift −90°), 연속 조작은 왼쪽 곱 누적(외재적), 회전 중심 = 회전 직전 자세의 축 정렬 바운딩 박스 중심, 매 회전 뒤 z 평행이동으로 접지. 뒤집기 = X 축 180°(정면에서 좌우 유지·위아래 반전, 앞뒤 면은 바뀜). 자유 각도(후속 15° 끌기)에서는 박스 중심이 회전마다 바뀌므로 그때 회전 중심을 다시 정해야 한다.
4. **수치 알고리즘·허용오차**: 90° 정수배는 cos·sin ∈ {−1, 0, 1} 정수 행렬(판정 `|deg/90 − k| < 1e-12`), 실측 누적 오차 2.04e-14 mm / 4회. 처음 자세로는 정점 사본 대입(오차 0). 접지는 최저점 z 를 빼서 정확히 0. "바닥판에 닿음" 표시는 `|bboxMin.z| ≤ 0.3 mm`(디자인 권고값, 표시 판정에만 사용). 중심선 원점 맞춤은 K-Arch `FitHorizontalOuters`(Taubin 원 맞춤) 유효 원 중심의 단순 평균. 치수는 K-Arch `AnalyzeDimensions`(0.1 mm `RoundTenths`, 구연 고리 z 폭 ≤ 3 mm 조건)를 그대로 씀.
5. **확정 해제 때 값 삭제**: 확정 뒤 자세를 바꾸면 슬롯·저장한 절단 각도(1~5)·치수가 모두 지워진다(디자인의 "재확인 필요·값 유지"와 다름, 명세 사람 검토 5).
6. **확정 단추 항상 활성**: 바닥 미정(떠 있음)이어도 확정 가능 — 확정이 접지를 겸함(명세 사람 검토 4).
7. **단위 자동 환산**: 파일 열기의 m/cm→mm 자동 환산(알려진 위험 2)은 그대로이며, 그 메모가 바닥 정하기 단계에서는 보이지 않고 확정 뒤 배지에만 나온다(제안 6).
