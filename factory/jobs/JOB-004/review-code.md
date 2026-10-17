# JOB-004 검토: code (code-reviewer)

결론: **PASS**

검토 대상: `factory/JOB-004` HEAD `b033aef`(커밋 1개) vs 기준 `e9b655a`(JOB-003 끝, **JOB-003 승인 전제**). 15파일 +602 −56(제품 `studio/src` +153 −48, 시험·CMake +444 −6, 문서 +5 −2). 입력: `git diff e9b655a..HEAD`, `spec.md`(JOB-004a 정의), `design-excerpt.md`, `verify.md`, `build-log.md`. 다른 review-*.md 는 읽지 않음. 빌드·테스트는 실행하지 않음(diff 읽기와 grep 만).

이번 JOB 은 명세 §0 분할에 따라 **004a(도판 좌/우 D5·R 반전)만** 구현한다. 실시간 단면(004b)·② 페이지(004c)는 범위 밖이며 diff 에도 없다(`git diff -- studio/src | grep -iE '^\+.*(preview|lod|debounce)'` 0건, `QTimer` 는 검수용 `--flip` 1줄뿐 — `main.cpp:38`).

| # | 항목 | 판정 | 근거 (file:line) |
|---|---|---|---|
| 1 | 명세 범위(§6 영향 파일 표) | PASS | 바뀐 15파일이 모두 §6 표 안에 있다(CLAUDE.md, docs/ui/UI_RULES.md, studio/CMakeLists.txt, studio/src/{ArtifactDoc.h/.cpp, Plate.h/.cpp, SheetView.cpp, View3D.cpp, MainWindow.cpp, main.cpp, ExportDialog.cpp, StartPage.cpp}, studio/tests/{test_side.cpp(신규), test_pipeline.cpp}). 금지 목록(`MeshIO.*`·`MeshRaster.*`·`Ribbon.*`·`Theme.h`·`test_pose.cpp`·`factory/**`·`docs/ui/mockup/**`) 무변경. `test_pipeline.cpp` 는 hunk 1개(48-71)만, 옛 두 검사를 숫자 그대로 Left 아래 둠(`test_pipeline.cpp:58-68`) — 명세 기준 10 그대로. 계산 함수(`recompute`·`buildSectionLoops`·두께·치수·`frontAngle`·`load*`·`confirmPose`)는 diff 에 없음 |
| 2 | 금지 구역(`GigaMesh/core`·`external`, K-Arch) | PASS | `git diff e9b655a..HEAD -- GigaMesh` 출력 0줄. K-Arch(`GigaMesh/gui/src/dongarch/karch/*`)도 무변경 |
| 3a | 규약 — 한국어 UI | PASS | 새 사용자 문자열 모두 한국어: 리본 `MainWindow.cpp:50`(sideText), 툴팁 `:216`, ctx `:260` `좌/우 반전`, 상태줄 `:764-766`, 부제 `SheetView.cpp:247-249`, 슬롯 meta `ArtifactDoc.cpp:686`·`MainWindow.cpp:678` `종단면`, `ExportDialog.cpp:126`, `StartPage.cpp:231`. 옛 문구(`좌단면·우외곽`·`토기 표준`·`좌측 종단면`·`우측 …`) 0건(verify 기준 15). 툴팁의 `(D5)` 는 명세 §3.5 지정 문구 — 확인 필요 1 |
| 3b | 규약 — Theme 토큰(R2) | PASS | 새 색은 `theme::ProfileRed` 만(`View3D.cpp:158`, `SheetView.cpp` 기존 펜). `+` 줄에 `QColor(숫자)`·`qRgb`·`#RRGGBB`·`Qt::white` 등 하드코딩 0건(직접 grep). `View3D.cpp:161` 의 `QColor(Qt::white)` 는 기존 줄(문맥 줄, `+` 아님) |
| 3c | 규약 — 주 단추 하나(R3) | PASS | `ToolState::Primary` 추가 0건. `combined` 는 기존대로 `On`(`MainWindow.cpp:224` 무변경). 확정 전 주 단추 `poseConfirm` 규칙(`:506`) 무변경 |
| 3d | 규약 — 좌표·단위 | PASS | 도면 x = 부호 있는 반경 R(mm), 자연 좌표(K-Arch: 단면 x ≥ 0, 외곽 x < 0)를 주석으로 명시(`ArtifactDoc.h:127-129, 167`), +Z 위·확정 뒤 중심선 x=y=0 은 손대지 않음 |
| 4a | 구조 — 부호 적용의 단순성 | PASS | `mirrorX` 삭제 → 공개 `display*()` 4개가 "자연 좌표 계산(`…Natural()`) → 마지막에 `signedX(…, sectionSign())`" 한 줄씩(`ArtifactDoc.cpp:750-769`). 간략화·곡선 다듬기는 부호와 무관한 자연 좌표에서만 돌아 Left = Right 의 정확한 부호 반전(시험 4 오차 0). 빗금은 자연 좌표 단면으로 계산 뒤 끝점에 같은 부호(`:776, 803-807`). 미스캔 합성 경로도 자연 좌표에서 합친 뒤 부호(`:702-713`) — 옛 동작과 동치 |
| 4b | 구조 — 상태 중복 | PASS | 좌/우 상태는 `ArtifactDoc::side_` 하나(`ArtifactDoc.h:204`). 리본 글자는 생성 때(`MainWindow.cpp:146`)와 바꿀 때(`:762`) 같은 `sideText()` 로 만들고, 상태를 바꾸는 경로는 `onAction("combined")` 하나(`Key_R` 도 이리로 — `:886`)라 어긋날 길이 없다. 되돌리기 `ViewState` 에 넣지 않음(`pushUndo` `:640` 무변경 — 명세 "되돌리기에 쌓지 않는다" 와 일치) |
| 4c | 구조 — 미리보기/전체 계산 분리 | 해당 없음 | 004b 범위(명세 §0·§2). 이번 diff 에 미리보기 경로 없음 |
| 4d | 구조 — 매직 넘버 | PASS | 새 상수는 의미가 분명하고 명세 값 그대로: yaw `0.0/180.0`(`Plate.cpp:123`, `View3D.cpp:77`), 파선 1 px(`View3D.cpp:158`), `--flip` 200 ms(`main.cpp:38`, 옆 `--max` 300 ms 와 같은 방식). 풍선 여유 `15`·`14`·`4` 는 기존 값의 부호 대칭 이전 |
| 4e | 구조 — 확정 전 가드·에러 처리 | PASS | `combined` 를 `needPose` 에 추가(`MainWindow.cpp:716`), `Key_R` 은 확정 전 분기(`:843`)에서 이미 막힘. `View3D::view()` 의 +180° 는 확정 전 조기 반환(`:64-75`) 뒤에만(`:77`). 빈 메시·빈 단면은 기존 경로(`display*` 가 빈 벡터 반환) 그대로 |
| 4f | 구조 — 함수 길이 | PASS | 새·바뀐 함수 모두 짧다(`signedX` 5-6줄, `display*` 1-4줄, `onAction` 분기 8줄). `View3D::paintEvent` 절단선 부분은 +11줄로 연속 구간 분할(`View3D.cpp:148-157`) — 읽을 만함 |
| 5 | 테스트 등록·내용 | PASS | `studio/CMakeLists.txt:94-96` `test_studio_side` + `add_test(NAME studio_side COMMAND test_studio_side ${CMAKE_CURRENT_BINARY_DIR})`. 출력은 인자(빌드 폴더)로만 씀(`test_side.cpp:314, 357, 392`) — 절대 경로 없음. 수용 기준 1-9 를 각각 검사: 1(`:212-216`), 2(`:219-235`), 3(`:238-251`), 4(`:254-298`, dims 7필드·두께 18점 `==`·toggle 뒤 `identical`·빗금 기울기), 5(`:301-303`), 6(`:308-349`), 7(`:352-385`), 8(`:398-420`), 9(`:422-423`). 허용오차는 이름 있는 상수(`kMirrorTol`·`kSmoothTol` `:39-40`)와 명세 근거 주석(`:4-5`). verify.md: CTest 14/14, studio_side PASS, 독립 파서 28/28 |
| 6 | 문서(UI_RULES R5 등) | PASS | `docs/ui/UI_RULES.md:73` R5 를 명세 §3.6 문장 그대로 교체, 머리말 예외 `:7`, R6 `:84`, 체크리스트 `:107`. `CLAUDE.md:10` 미결 줄 → D5 확정. 사용자 동작 변화(R 의미 변경)는 툴팁·ctx·상태줄로 노출 |
| 7 | 크기(300줄 지침) | PASS(주석) | 제품 코드 +153 −48 ≈ 200줄로 지침 안(명세 추정 ≈140). 시험이 +424 로 명세 추정(≈210)의 2배 — 명세 §9 가 "테스트를 떼면 배치를 검증할 수 없으므로 나누지 않는다" 로 이미 수용한 범주이고, 늘어난 이유(두 예제 모두 SVG·DXF 검사, 측정값 인쇄)가 build-log 제안 4 에 적혀 있다. 제안 1 참고 |

## 고치지 않아도 되는 제안
1. **시험 공용 헤더**: `CHECK` 매크로가 `test_pipeline.cpp:16`·`test_pose.cpp:20`·`test_side.cpp:25` 세 곳에 복제돼 있다. 다음 시험 추가 때 `studio/tests/check.h` 로 모으면 `test_side.cpp` 도 조금 줄어든다. SVG 층·DXF VERTEX 파서(`test_side.cpp:104-170`)도 004b·004c 시험이 재사용할 가능성이 높아 같은 헤더 후보.
2. **문구 한 곳에**: Right/Left 문구가 `SheetView.cpp:247-249`(부제)와 `MainWindow.cpp:764-766`(상태줄)에 같은 글자로 두 번, 리본 글자가 `MainWindow.cpp:50` 에 따로 있다. `sideText()` 처럼 부제 문구도 한 함수(예: `ArtifactDoc` 옆 자유 함수)에 두면 004c 에서 `단면 위치` 그룹을 만들 때 어긋날 일이 줄어든다.
3. **빗금 부호 루프**: `ArtifactDoc.cpp:803-807` 는 `signedX` 와 같은 일을 쌍(pair)에 한다. 크기가 작아 그대로 둬도 되지만 `signedX` 오버로드 하나로 맞추면 "부호는 한 곳에서" 원칙이 더 분명해진다.
4. **주석 정합**: `View3D.cpp:146-147` "흰 3 px 테" 는 실제 펜(흰 5 px 위 빨강 2 px — `:160-161`, 명세 §3.4 "흰 5 px + ProfileRed 2 px")과 표현이 다르다(테 두께 1.5 px×2 를 뜻한 옛 문구). 다음에 손댈 때 "흰 5 px 밑선 + 빨강 2 px" 로.
5. **커밋 제목**: `feat(studio): ② 막으로 자르기 — 도판 좌/우 D5…` 는 004a(좌/우만)인데 ② 페이지가 들어간 것처럼 읽힌다. 병합 때 메시지를 고칠 기회가 있으면 `JOB-004a` 를 제목에 넣기를 권한다(본문은 이미 004a 라고 밝힘).
6. **SVG 음영 홀수 폭**: `Plate.cpp:640-643` 은 영상 폭이 홀수면 Right 쪽이 `floor(w/2)` 화소를 폭 `rectMm.width()/2` 로 늘려 그린다(300 DPI 에서 ≤ 0.04 mm). 옛 코드(Left 쪽 `ceil`)와 같은 성질이고 시험 6 의 0.01 mm 경계도 통과하므로 무해.

## 확인 필요
1. **툴팁의 `(D5)`**(`MainWindow.cpp:216`): 내부 결정 ID 가 사용자 툴팁에 노출된다. 명세 §3.5 가 지정한 문구라 FAIL 사유는 아니지만, "약어 노출 없음" 규약과 부딪히는지 ui-reviewer/사람이 판단.
2. **반전 시 `dirtyExport_`**: `onAction("combined")` 이 `refreshAll(true)`(`MainWindow.cpp:763`)를 불러 상태줄에 `● 도면 저장 전` 이 켜진다(`:497, 527`). 내보내기 결과가 실제로 달라지므로 맞는 동작으로 보이나, "계산을 바꾸지 않는 표시 전환"이 저장 전 표시를 켜는 것이 의도인지 사람 확인.
3. **Left 에서 단면 도구 끌기 방향**: Left 는 3D 를 뒤(+180°)에서 보므로(`View3D.cpp:77`) 같은 오른쪽 끌기(`cutAngleDragged(+0.5°/px)`, `:191`)가 화면에서는 반대로 도는 것처럼 보인다. 명세 사람 검토 2(막 회전 부호, 004b 결정)와 함께 볼 것.

## 사람 검토 항목
1. D5 적용 범위와 `R` 의 의미 변경: `R` = "정면각 +180°" → "같은 단면의 거울상(도판·3D 모두 뒤에서 본 모양)"(`MainWindow.cpp:759-766, 886`). 비대칭 유물에서 반대쪽 반을 단면으로 보던 옛 기능은 사라진다. 좌/우 선택은 세션 동안 유지(로드해도 그대로 — `test_side.cpp:301-303`), 디스크·되돌리기에 저장 안 함.
2. UI 정본 변경: `docs/ui/UI_RULES.md` R5 반대로 뒤집기·머리말 목업 예외·R6·체크리스트, `CLAUDE.md:10` "미결" → "D5 확정"(CLAUDE.md §1 "바꾸려면 먼저 사용자 승인" 대상).
3. 슬롯 meta `좌단면·우외곽` → `종단면`(`ArtifactDoc.cpp:686`, `MainWindow.cpp:678`).
4. builder 제안(범위 밖, 미구현): Left 에서 상태줄 커서 `R` 부호가 단면 쪽 −로 나옴, `기고 H` 파선·라벨이 D5 기본에서 외면(음영) 위에 겹침(`Plate.cpp:343-348`), Left 리본 글자(16자)가 실제 한글 폰트에서 1560 px 리본을 넘치는지.
5. 이 브랜치는 JOB-003(e9b655a) 승인 전제 — JOB-003 이 rework 되면 다시 쌓아야 한다.
