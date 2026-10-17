# JOB-003 검토: code

결론: **PASS**

검토 대상: `factory/JOB-003` `58a33ae..e9b655a`(커밋 A `1ad49b4` 엔진·테스트, 커밋 B `e9b655a` 화면). 15개 파일, +821/−109. 입력은 `git diff 58a33ae..HEAD`, spec.md(기준 6 → 6a/6b 수정본), decision.md, design-excerpt.md, verify.md(추기 포함), build-log.md다. 다른 review-*.md 는 읽지 않았다. 아래 줄 번호는 모두 HEAD `e9b655a` 기준이다.

요약: 명세 범위와 금지 구역을 지켰고 규약(한국어, Theme 토큰, 주 단추 하나)도 지켰다. `studio_pose` 는 등록됐고 UI_RULES R1 정본도 동기화됐다. 구조에서 중복·매직 넘버·상태 필드 과다가 조금 있지만 동작 결함은 아니므로 제안으로만 남긴다. 821줄은 명세가 예고한 크기 초과(≈545)를 다시 넘었지만, 커밋 2개가 엔진/화면으로 깨끗하게 나뉘고 각각 빌드·테스트를 통과해 정당하다고 본다.

## 항목별 판정

| # | 항목 | 판정 | 근거 (file:line) |
|---|---|---|---|
| 1 | 명세 범위(영향 파일 표 대비) | PASS | `git diff --name-only 58a33ae...HEAD` 15개가 영향 파일 표 15행과 일치한다. 표의 이유를 벗어난 추가 변경은 세 가지이고 모두 표에 있는 파일 안이며 목표에 종속된다. ① `Ribbon.cpp:182` 작은 단추 `setObjectName(id)`: 명세 2절이 요구한 작은 단추 툴팁을 붙이려면 `column()` 에 툴팁 인자가 없고 `Ribbon.h` 는 표에 없다. 타일은 이미 `Ribbon.cpp:18` 에서 같은 방식을 쓴다. ② `MainWindow.cpp:104-105` 확정 전 커서 R/H/두께 줄과 `MainWindow.cpp:507` `stDirty_` 비움: 도판 좌표가 없어서 생긴 귀결이다. ③ `ArtifactDoc.h:189-190` `rawLod_`·`rawLodColors_`: 명세는 `rawVertices_` 만 적었지만, `resetPose` 가 LOD 까지 정확히 되돌리게 하려는 것이고 35k 면 이하라 메모리 영향이 작다. 범위 밖 항목(축 색, 단축키 X·Y·Z·F, 자세 저장, 되돌리기)은 들어오지 않았다 |
| 2 | 금지 구역 | PASS | `git diff --name-only 58a33ae...HEAD -- GigaMesh studio/src/Plate.* studio/src/ExportDialog.* studio/src/StartPage.* studio/src/Theme.h factory/scripts` 출력이 없다. K-Arch(`karch/*`)는 호출만 한다(`ArtifactDoc.cpp:482` `FitHorizontalOuters`, `:229` `FillMeshStats`). `updateBbox`(`ArtifactDoc.cpp:63-81`)는 K-Arch 의 비공개 `computeBbox`(`MeshLoader.cpp:136`)와 같은 규칙을 다시 구현한 것인데, karch 수정이 금지돼 있어 불가피하다 |
| 3a | 규약 — 한국어 UI·약어 | PASS | 새 화면 문자열은 모두 한국어다(`MainWindow.cpp:187-207, 250-256, 414, 444-448, 506-511, 594, 602`, `SheetView.cpp:84, 250`, `View3D.cpp:253`, `ArtifactDoc.cpp:455-461, 494-495, 623`). 새 문자열에 PCA·LOD 같은 약어가 드러나지 않는다(`poseAuto` 툴팁 `MainWindow.cpp:189`). 축 글자 `X/Y/Z` 와 kbd `Enter/Shift` 는 명세가 정한 라벨이다. 테스트 출력(영문 printf)은 로그 전용이고 `test_pipeline.cpp` 와 같은 관례다 |
| 3b | 규약 — Theme 토큰만 | PASS | 바닥판은 `theme::Wash`·`theme::Grid10mm`·`theme::Edge` 만 쓴다(`View3D.cpp:103,105,112`). 자리 문구는 `theme::Muted`(`SheetView.cpp:81`). 새 색 리터럴은 없다. `View3D.cpp:133` `QColor(255, 0, 0, 22)` 는 기존 코드이고 변경 줄이 아니다. 축 색은 쓰지 않았다(명세 범위 밖) |
| 3c | 규약 — 주 단추 하나(R3) | PASS | 추가된 `ToolState::Primary` 는 `poseConfirm` 하나뿐이다(`MainWindow.cpp:217`, 확정 뒤 `Idle` 은 `:499`). ① 페이지 그룹은 `fileGroup`(열기·최근, `:123-128`)·바닥판에 올려놓기·확정이고 `export` 타일이 없어 페이지당 Action 하나가 지켜진다 |
| 3d | 규약 — mm·Z-up 일관성 | PASS | `kGroundTolMm`(`ArtifactDoc.cpp:97`), `heightAboveFloor() // mm`(`ArtifactDoc.h:69`), 파일 머리 주석 "mm · +Z 위 · 바닥판 Z = 0"(`ArtifactDoc.h:2-4`), `FloorPlate` 주석(`View3D.cpp:15-17`). 사소한 점: `ArtifactDoc.h:162` 의 `base_` 주석 "정위치까지 적용된 메시"는 이제 "현재 자세"라 낡았다(제안 7) |
| 4a | 구조 — 중복 | PASS(제안) | ① `confirmPose` 수동 경로의 원 중심 평균·박스 중심 대체·단위/노이즈 메모 덧붙이기(`ArtifactDoc.cpp:482-502`)가 `autoAlign`(`:560-583`)과 거의 같다(≈15줄). ② `unconfirm(); centered_ = false;` 쌍이 5번 반복된다(`:240-241, 358-359, 424-425, 437-438, 445-446`). ③ `openSample`/`openArtecBlindSample`(`MainWindow.cpp:452-481`)은 기존 중복에 한 줄씩 더 붙었다. 동작 결함은 아니다 |
| 4b | 구조 — 매직 넘버 | PASS(제안) | LOD 목표 면 수 `35000` 이 1곳에서 3곳으로 늘었다(`ArtifactDoc.cpp:234, 509, 589`). `zBand(…, 12)` 는 2곳(`:482, 560`). 바닥판 여유 `20.0`, 눈금 `10.0`(2곳), 맞춤 여유 `1.05`(`View3D.cpp:24, 70, 106-107`)는 명세 4절 값이고 주석이 있지만 이름 있는 상수가 아니다. 접지 판정 0.3 mm 는 `kGroundTolMm` 로 이름이 있다. 90° 판정 `1e-12`(`:85`)는 지역 허용오차라 괜찮다 |
| 4c | 구조 — 함수 길이 | PASS | `confirmPose` 43줄(`:470-512`), `rotatePose` 26줄(`:400-426`), `recompute` 약 70줄(`:603-672`, 확정 전 비우기 블록 21줄), `MainWindow::refreshAll` 42줄(`:489-530`, 확정 전 분기 17줄). 읽을 만하다. `recompute` 의 비우기 블록과 `refreshAll` 의 확정 전 분기는 함수로 떼어 내면 더 읽기 쉽다(제안 3) |
| 4d | 구조 — 확정 전/후 상태 관리 | PASS(제안) | 새 상태는 `poseConfirmed_`·`centered_`·`pendingAligned_/pendingNote_/pendingRmse_` 다섯 개다. 불변식은 지켜진다: `base_` 를 바꾸는 모든 경로(`loadMesh`, `rotatePose`, `resetPose`, `dropToFloor`, `rescaleUnits`)가 `centered_=false`, `autoAlign` 만 `true` 다. 확정 경로는 한 곳(`confirmPose`)에 모여 있다. 다만 `pending*` 세 개는 확정 전 `recompute` 가 `aligned_/alignNote_/axisRmse_` 를 "바닥 미정"으로 덮어쓰기(`ArtifactDoc.cpp:622-624`) 때문에만 존재한다. 화면은 이미 `poseConfirmed()` 로 직접 갈라진다(`SheetView.cpp:248-250`, `View3D.cpp:252-260`, `MainWindow.cpp:502`). 그래서 덮어쓰기를 없애면 `pending*` 와 `confirmPose` 의 centered 분기를 지울 수 있다(제안 1) |
| 4e | 구조 — 오류 처리(빈 메시·실패) | PASS | 자세 함수는 모두 `isLoaded()` 로 막는다(`ArtifactDoc.cpp:401, 431, 442, 471, 515`). 빈 메시는 `updateBbox` 가 처리한다(`:64-68`). 원 맞춤이 실패하면 박스 중심으로 대신하고 `aligned_=false`·"중심선 미검출"로 둔다(`:489-495`). `loadFile` 실패 경로는 그대로다(`:244-250`). `rescaleUnits` 에는 빈 메시·배율 가드가 있다(`:340-341`). 확정 전 도판·3D 경로(`fitView`, `paintEvent`, `setDoc`, 커서)도 모두 가드된다(`SheetView.cpp:45, 80-86`, `View3D.cpp:37, 97`, `MainWindow.cpp:104`) |
| 4f | 구조 — 확정 전 가드 | PASS(제안) | 동작 10종(`MainWindow.cpp:708-710`), 단축키(`:823-836`), 되돌리기/다시(`:222, 227`), Ctrl+P(`:816` → `onAction("export")` → 가드), `setFrontAngle`(`:651`)이 명세 2절 목록과 일치한다. 단, 금지 목록(블랙리스트)이 아래 본 switch(`:837-875`)와 따로 관리되는 평행 목록이라, 나중에 계산 단축키·동작을 더할 때 가드에서 빠뜨릴 위험이 있다(제안 4) |
| 5 | 테스트 — 수용 기준별 자동 테스트·CTest 등록 | PASS | `studio/CMakeLists.txt:91-93` 에 `test_studio_pose`/`studio_pose` 가 빌드 폴더 인자와 함께 등록됐다. `test_pose.cpp` 의 기준별 위치: 1·2·3 `:77-106`(±90°×4 세 축, 뒤집기×2, 매 조작 뒤 접지·박스 중심, `dropToFloor` 직후 `:81`), 4 `:109-123`(`d == 0.0` 정확 비교), 5 `:126-147`, 6a `:150-169`·`:209`(loadFile 직후), 6b `:170-189`(autoAlign 직후 미확정 `:176`), 7 `:192-210`, 8 `:213-227`, 9 `:229-230`. 산출 PLY 는 `argv[1]`(빌드 폴더)에 쓰고 절대 경로는 없다(`:74, 194`). `test_pipeline.cpp:139-140` 은 2줄 추가뿐이다(기준 10). UI 가드(Enter·needPose)는 자동 테스트가 없지만 명세가 요구하지 않았다(제안 6) |
| 6 | 문서 — UI_RULES R1 동기화 | PASS | 명세가 지정한 네 곳이 모두 반영됐다: R1 탭 줄 `docs/ui/UI_RULES.md:19`, ctxBar 비고 `:21`, R1.4 `▲ 바닥 미정` `:29`, R3 주 단추 예 `[이 자세로 확정]` `:58`. 코드의 탭 이름(`MainWindow.cpp:166`)과 일치한다. 목업(`docs/ui/mockup/dongarch3d_mockup.html:285,619`, `dongarch3d_design_spec.md:84`)의 옛 탭 이름은 명세 범위 밖(백로그 8)이고, StartPage 안내 문구도 명세상 수정 금지 파일이다. 둘 다 사람 검토 항목으로 올린다 |
| 7 | 크기 — 821줄이 300줄 지침을 넘은 것의 정당성 | PASS(정당) | 커밋 A `1ad49b4` +566/−47: ArtifactDoc 약 370, 테스트 약 236, `MainWindow` 임시 shim 7줄(`openPath` 자동 확정, B 에서 제거). 커밋 B `e9b655a` +259/−66: 화면만 바꿨고, `git diff 1ad49b4 e9b655a -- studio/src/ArtifactDoc.* studio/tests` 가 비어 있어 엔진·테스트를 건드리지 않는다. build-log 에 따르면 각 커밋 상태에서 configure·build(경고 0)·ctest 13/13 을 통과했다. 명세 "크기" 절이 이미 ≈545줄과 "A → B 커밋 2개" 대안을 제시했고 그대로 따랐다. 추정 초과(ArtifactDoc 150 → 370, MainWindow 140 → 220)의 대부분은 명세가 요구한 가드·문구·툴팁·바닥판·확정 전 비우기 몫이다. 제안 1·2 를 적용하면 30~40줄 정도 줄 여지가 있다. 다음에 이 정도 크기라면 JOB 을 나누기(003a/003b)를 권한다 |

## 고치지 않아도 되는 제안

1. **`pending*` 세 필드 없애기**: 확정 전 `recompute` 가 `aligned_/alignNote_/axisRmse_` 를 덮어쓰지 않게 바꾼다(`ArtifactDoc.cpp:622-624` 삭제). 접근자를 `aligned() { return poseConfirmed_ && aligned_; }` 처럼 두거나, 화면이 이미 하듯 `poseConfirmed()` 로 가르면 된다. 그러면 `autoAlign` 은 예전처럼 `aligned_` 등을 직접 쓰고, `confirmPose` 의 centered 분기(`:473-478`)와 `ArtifactDoc.h:193-196` 이 사라진다.
2. **중복 공용화**: (가) "수평 외곽 원 중심 평균 → (dx, dy, n)" 함수 하나를 `autoAlign`(`:560-567`)과 `confirmPose`(`:482-492`)가 같이 쓴다. (나) "단위·부유노이즈 메모 덧붙이기" 함수(`:498-502` ≡ `:580-583`). (다) `unconfirm()` 이 `centered_ = false` 까지 맡고 `autoAlign` 만 그 뒤 `true` 로 바꾼다(5곳 반복 제거).
3. **이름 있는 상수·함수 떼기**: `kLodTargetFaces = 35000`(`ArtifactDoc.cpp:234, 509, 589`), `kPlateMarginMm = 20`, `kPlateGridMm = 10`, `kBaseFitMargin = 1.05`(`View3D.cpp:24, 70, 106-107`). `recompute` 의 확정 전 비우기 블록(`:606-626`)은 `clearComputed()` 로, `refreshAll` 의 확정 전 분기(`MainWindow.cpp:502-515`)는 `refreshPoseStage()` 로 떼면 읽기 쉽다.
4. **가드를 허용 목록으로**: `needPose`(`MainWindow.cpp:708-710`)와 단축키 목록(`:828-833`)을 "확정 전 허용 = pose*·shade·lines·grid·shadeMode·axis·파일 동작, I·L·V·Tab·Enter"로 뒤집으면, 나중에 계산 동작을 더할 때 가드에서 빠뜨릴 위험이 없다.
5. **`ArtifactDoc::confirmPose` 멱등성**: 이미 확정된 상태에서 다시 부르면 `resetSlots()` 로 사람이 저장한 슬롯 각도가 지워진다. 지금은 `MainWindow::confirmPose`(`:601-604`)가 막아 주지만, 문서 계층에서 `if (poseConfirmed_) return;` 을 두는 편이 안전하다. 홈 `axis` 경로는 `autoAlign` 이 먼저 확정을 풀기 때문에 영향이 없다.
6. **UI 가드 자동 테스트**: 오프스크린 `MainWindow` 에 `QTest::keyClick(Enter)`·`onAction("export")` 를 보내 확정 전/후를 확인하는 테스트를 후속으로 둔다(build-log 의 일회성 `--enter` 수동 확인을 대신함).
7. 낡은 주석: `ArtifactDoc.h:162` `base_` → "현재 자세(확정 전: 사람이 돌리는 자세, 확정 뒤: 도면 좌표)".
8. 자동 세우기 → 확정 경로에서 LOD 를 세 번 만든다(`:234, 589, 509`). 직전이 자동 세우기면 확정 때 재생성을 건너뛸 수 있다(결과 같음, builder 제안과 같음).

## 확인 필요 (확신 없는 지적)

- **홈 「중심축 맞춤」 경로의 최근 목록**: `MainWindow.cpp:731-738` 은 `doc_.autoAlign(); doc_.confirmPose();` 로 확정하므로 `recordRecent()`·`fitView()` 를 부르지 않는다. 파일을 연 뒤(최근 목록에 "바닥 미정 · 치수 —"로 기록됨, `:428`) ① 대신 홈 탭에서 중심축 맞춤으로 확정하면 최근 목록이 "바닥 미정"으로 남을 수 있다. 명세 문구("`autoAlign()` → `confirmPose()`")와는 맞는다. `doc_.confirmPose()` 를 `MainWindow::confirmPose()` 로 바꾸는 한 줄이면 일관된다(그러면 페이지도 홈으로 바뀌는데, 이미 홈이라 영향이 없다).
- **3D 정보 줄 숫자 글꼴(R4)**: 확정 전 `info2_` 가 접지 문구 전체를 `Malgun Gothic` span 으로 감싼다(`View3D.cpp:259`). 그래서 "45.0 mm" 숫자도 고딕이 된다. R4 는 수치를 모노로 정한다. ui-reviewer 판단에 넘긴다.
- **큰 메시 성능**: 90° 단추 한 번마다 `rotateAbout`(원본·LOD) → 바운딩 박스 재측정 2회 → `recompute` 의 `mesh_ = base_` 전체 복사(`ArtifactDoc.cpp:604`)가 돈다. 500만 정점 스캔이면 클릭마다 약 120 MB 를 복사한다. 기존 `recompute` 패턴 그대로이고 정확성 문제는 아니다. 실측 스캔으로 응답 시간을 확인할 필요가 있다.
- **작은 단추 objectName 전면 부여**(`Ribbon.cpp:182`): 지금 전역 QSS 의 id 선택자(`#Ribbon`, `#Tabs`, `#UndoBtn`, `#StatusBar`, `#SlotList`, `#Badge*` …)와 단추 id(`refBase`, `cutMove`, `shadeMode`, `gridInfo`, `pose*` …)는 겹치지 않는다. verify.md 기준 `shot_home`·`shot_export` 도 바이트가 같다. 나중에 QSS 에 `#<동작 id>` 선택자가 생기면 의도치 않게 걸릴 수 있다.

## 사람 검토 항목

1. **기울어진 채 확정하면 구경 —(6a)**: 확정 자체는 되지만 화면에 "구연 수평 필요" 같은 단서가 없다. 기고 200.5·구경 — 가 조용히 표시된다(decision.md, 사람 검토 0). 후속(수평 맞추기 T/B 조각)에서 안내를 넣을지 정해야 한다.
2. **StartPage 안내 카드가 실제 흐름과 어긋남**: `StartPage.cpp:228-229` 의 "① Artec 파일 열기 · 자동 전처리 — … 회전 중심축 정렬이 한 번에 끝납니다"는 이제 사실이 아니다. 열면 ① 바닥 정하기에서 Enter 확정이 필요하다. 명세가 StartPage 수정을 금지했으므로 후속 JOB 이 필요하다. 같은 맥락에서 `README.md:12` 의 "빌드·테스트 11종"도 13종으로 바뀌었다(이 JOB 이전부터 낡음).
3. **UI 정본 변경 승인**: UI_RULES R1 탭 이름 `정위치·중심축` → `① 바닥 정하기`(CLAUDE.md §1 "바꾸려면 먼저 사용자 승인"). 목업(`docs/ui/mockup/`)은 옛 이름 그대로라 "목업과 코드가 다르면 코드가 틀린 것" 규칙과 일시 충돌한다(백로그 8).
4. **흐름 변화 수용**: 파일 열기, 드래그 앤 드롭, Artec 감시 폴더 자동 열기가 모두 ① 단계에서 멈추고 Enter 가 한 번 더 필요하다. 확정 자세는 저장되지 않아 다시 열면 다시 정해야 한다.
5. **크기 정책**: 명세 추정 ≈545줄 → 실제 821줄. 커밋 분리는 깨끗하지만, 이 크기의 다음 작업은 JOB 분할을 기본으로 할지 코디네이터가 정해야 한다.
