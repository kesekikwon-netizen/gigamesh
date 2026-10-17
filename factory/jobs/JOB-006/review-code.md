결론: PASS

# JOB-006 코드 품질 검토 (code-reviewer)

- 대상: `D:\_dongarch_build\wt\.worktrees\JOB-006`, 브랜치 `factory/JOB-006`, `804f60d..688b7bd`(커밋 2개: `eee6dd8` 본 구현, `688b7bd` 재작업 1). `git status --porcelain` 은 빈 출력이다.
- 입력: `git diff 804f60d..HEAD`(14 files, +714/−57)를 직접 읽었고 명세 `spec.md`(§0~§11, 수용 기준 34개)와 대조했다. build-log 의 주장은 diff 로 다시 확인했다. verify.md·artifacts 는 실행 결과를 확인하는 데만 썼다.
- 크기: 제품 코드(`studio/src`)는 +315/−52 이다. 빈 줄·주석을 뺀 추가 줄을 직접 세면 **277줄**이다(`grep '^+' | 빈 줄·// 제외 | wc -l`). §0 의 300줄 지침 안이므로 006a/006b 로 나누지 않은 것이 맞다.

## 항목별 판정

| # | 항목 | 판정 | 근거 (file:line) |
|---|---|---|---|
| 1 | 명세 범위 | PASS | 바뀐 파일 14개가 §6 허용 목록(기준 30)과 정확히 같다. 명세에서 벗어난 곳은 넷이고 모두 작고 이유가 있다. ① `RibbonGroup::titleLabel()` 추가(Ribbon.h:59): 이름 칸 최소 폭을 주려는 것으로 §3.3 의 "방법은 builder 재량" 안이다. ② ① 확정 전 분기에도 `ctxHint_` 툴팁을 넣었다(MainWindow.cpp:605): 들고 있던 ✂ 툴팁이 남지 않게 하는 1줄이다. ③ `--shot` 이면 창을 1845×920 으로 고정한다(main.cpp:24). ④ 찍기 직전 `LayoutRequest` 를 처리한다(main.cpp:69). ③·④ 는 §3.9 에 없지만 검수 경로(`--shot`)에만 작용하고 실제 앱 창은 바꾸지 않는다. 기준 33·34 를 재현 가능하게 하려는 수정이다 → 범위 위반이 아닌 **사람 승인 항목**으로 둔다 |
| 2 | 금지 구역 | PASS | `GigaMesh/**` 변경 없음. 엔진(`ArtifactDoc.*`·`CutGesture.h`·`Plate.*`·`MeshIO.*`·`MeshRaster.*`), `StartPage`·`ExportDialog`, 기존 시험 4개도 `git diff --name-only 804f60d HEAD -- …` 출력이 비어 있다(기준 22). `alignCut`·`previewAngle`·`isPreviewing`·`lodMesh` 는 기존 공개 API 호출만 한다 |
| 3a | 한국어·약어 | PASS | ② 의 새 화면 문자열은 모두 `CutPageText.h:14-32` 한 곳에 있다(§3.2). 툴팁은 MainWindow.cpp:240-252 에 있다(명세가 허용). 모두 한국어다. 새 약어 노출은 없다(`3D`·`X/Y/Z 막`·키 이름 `Shift`·`Esc` 는 원문 그대로). 옛 용어(`정면 회전`·`절단 위치 이동`·`절단 각도`·`절단면 · 중심축` 등)가 `QStringLiteral` 에서 0건이다(기준 24 재확인) |
| 3b | Theme 토큰·R2 | PASS | 색은 `theme::hex(theme::Done/Muted/Ink)`(MainWindow.cpp:43-47·258-272), `Faint`(Ribbon.cpp:97·Theme.h:91)만 쓴다. 추가 줄에 `QColor(`·`#xxxxxx`·`Qt::색` 이 없고, 축 색 hex 0건, 새 `inline const QColor` 0건이다. QSS 잠김 줄은 `[on]` 다음에 있다(Theme.h:91, 같은 특이도라 뒤가 이김 — 기준 8·9·15 가 Faint/Ink/Ink2 로 판정) |
| 3c | 주 단추 1개(R3) | PASS | 추가 줄의 `ToolState::Primary` 는 0건이다. ② 페이지에는 `Action` 단추가 없다. 맞는 축 켜짐은 `On`(Oat)이다(MainWindow.cpp:636-637) |
| 3d | 단위·좌표 | PASS | 새 기하 계산이 없다. 각도 표기 −180~180·0.1°(CutPageText.h:43-46)와 `atAxis` 의 mod 360 → mod 180 판정(:58-61)은 §3.2·원문 1460 과 같다. Y 막 = 0°, X 막 = +90°(MainWindow.cpp:917)는 JOB-005 와 같다 |
| 4a | 구조·중복 | PASS | `RibbonGroup::column` 이 새 `button()` 을 다시 써서 기존 단추 생성 중복을 줄였다(Ribbon.cpp:180-195). `cut_.click(` 호출은 1곳(MainWindow.cpp:913), `previewFrontAngle` 도 1곳이다. X/Y 맞춤은 §3.7 의 코드와 글자 단위로 같다(:915-925): 진행 중이면 `cancelCut`, 아니면 `pushUndo`, 대기 커서, `alignCut` 한 번 |
| 4b | 매직 넘버 | PASS(제안 있음) | 판정 경계(1°)·표기(0.1°)는 함수 안에 원문 근거 주석과 함께 있다. 남은 숫자 리터럴은 모두 주석으로 뜻이 적혀 있다: `23`(Ribbon.cpp:205), 각도 칸 `+ 6`(MainWindow.cpp:263), `1845, 920`(main.cpp:24), ctx 묶음 색인 `[0][1][8][9]`(MainWindow.cpp:337). 이름 있는 상수로 바꾸는 것은 아래 제안 1 로 둔다. 동작 결함은 아니다 |
| 4c | 함수 길이 | PASS(제안 있음) | `buildRibbon` 이 158-308(약 150줄)로 길어졌다(② 부분 약 37줄). `onAction`·`refreshAll` 은 분기 몇 개가 늘었을 뿐이다. 분리 제안은 제안 2 |
| 4d | 에러 처리 | PASS | 문서가 없으면 `onAction` 이 일찍 돌아간다(기존). `onPage` 는 `!doc_.isLoaded()` 이면 ② 진입을 막는다(:727). 빈 메시면 `faceText(0)` = `0 면` 으로 깨지지 않는다. 확정 전에는 `requirePose` 가 ① 로 보낸다(재진입 경로 `onPage(②)→requirePose→setCurrentPage(①)→onPage(①)` 는 한 단계에서 끝난다). `SheetView.cpp:248` 의 `s.name.at(0)` 은 빈 이름을 막지 않는다. 슬롯 이름은 앱이 만들기 때문에 실제 위험은 낮다(제안 4) |
| 5 | 테스트 | PASS | `test_cutpage.cpp` 가 기준 1-18 과 1:1 이다: 1-4 :97-124, 5-7 :127-157, 8 :181-198, 9 :201-253, 10 :256-268, 11 :271-298, 12 :301-304, 13 :307-325, 14 :328-335, 15 :338-359, 16 :362-363, 17 :366-370, 18 :384-385. 판정식이 명세와 같다(`==`·`startsWith`·`endsWith`, 기다림은 상한 15 s 만 있고 시간에 따라 갈리는 판정은 없다. 끄는 중은 왼쪽 단추를 누른 채로 확인한다). CTest 에 `studio_cutpage` 로 등록됐고 `RUN_SERIAL TRUE` 다(CMakeLists.txt:106-110). 시험은 파일을 쓰지 않는다. QSettings 는 조직·앱 `DongArch3DTest` 로 분리했다(:377-378). 절대 경로 `D:\…` 쓰기는 없다. 기준 19-34 는 명령·캡처 판정이며 verify.md 에서 모두 통과했다(경고 0, `100% tests passed out of 16`, LastTest 세 줄 각 1, 34 `True`×2 — artifacts 로 직접 확인) |
| 6 | 문서 | PASS | 사용자에게 보이는 동작 변화(탭 추가·잠김·② = 도구 든 상태·`[ ]` 배지 삭제)는 정본 `UI_RULES.md` R1 표 1·3행과 R8(:20·22·95)에 §3.10 문구 그대로 반영됐다(기준 29). studio README 는 없다 |

### 오케스트레이터 지정 확인점

| 확인점 | 판정 | 근거 |
|---|---|---|
| 엔진 ArtifactDoc·CutGesture·Plate 무변경 | PASS | 위 2 |
| Ribbon API 구조 | PASS | §3.3 목록 `pageChanged`(Ribbon.h:100, `setCurrentPage` 끝에서 늘 내보냄 Ribbon.cpp:332), `currentPage()`(:85), `setPageLocked`(:87, 누르는 것은 막지 않음), `RibbonGroup::setTitle/button/rows`(:58-65)가 그대로 있다. `rows` 는 줄 높이 23 px 고정 + `AlignVCenter` 다 → 3줄 = 77 px. 시험 9 가 ② 페이지 sizeHint ≤ ① 로 리본 높이가 그대로임을 확인했다(`[9] ribbon height=107`). 추가 API `titleLabel()` 하나는 위 1 |
| 시험이 기준과 1:1 | PASS | 위 5 |
| builder 이탈(build-log) | PASS(사람 확인 1건) | ① 빌드 폴더 `build_job006` 사용: 독립 검증은 명세대로 `verify_job006` 에서 다시 했으므로 영향이 없다. ② 각도 칸 글꼴을 위젯 스타일시트로 준 것(MainWindow.cpp:255): 전역 QSS 글꼴 규칙이 `setFont` 를 이기는 Qt 동작 때문이라는 설명이 맞다. 색이 아니고 R4 의 모노 `Consolas` 와 같다. ③ `titleLabel()`·ctx 툴팁 1줄: 문제없다. ④ 재작업 1 의 `--shot` 1845×920 고정: diff 로 확인했다(main.cpp:19-24, 표시 전 resize, `args` 를 `show()` 앞으로 옮김). 제품 화면 동작은 그대로이고 캡처 경로만 바뀐다 → 사람 검토 1 |

## 확인 필요 (결함 가능성 — 막지는 않음)

1. **`cutDrag_` 가 누를 때 한 번 정해지고 바뀌지 않는다(View3D.cpp:183·192·203)** — JOB-005 에서는 판정이 그때그때의 `cutTool_` 이었는데, 이번 변경으로 생긴 가장자리 경우가 둘 있다.
   - (a) ② 에서 왼쪽 단추로 끄는 중에 키(`Esc`·`O`·`S`)로 홈으로 가면 `onPage` 가 먼저 확정하고 도구를 내려놓는다. 그런데 마우스를 아직 누른 채 움직이면 `cutDrag_` 가 그대로라서 `cutAngleDragged` 가 계속 나간다. 그 결과 **홈(ctx 숨김)에서 막 각도가 바뀌고**, 놓을 때 확정된다. `Ctrl+Z` 로 되돌릴 수 있어 자료가 사라지지는 않는다.
   - (b) 왼쪽 단추로 끄는 중에 오른쪽 단추를 더 누르면 `cutDrag_ = false` 로 덮인다. 그러면 놓아도 `cutDragReleased` 가 나오지 않아, 다음 동작(`flushCut`)까지 "끄는 중" 미리보기가 남는다.
   - 고치기 1-2줄: `setCutTool(false)` 때 `cutDrag_ = false` 로 두고, `mousePressEvent` 에서 이미 `drag_` 면 `cutDrag_` 를 덮지 않는다.
2. **CI 에서 처음 도는 GUI 통합 시험**: `studio_cutpage` 는 실제 `MainWindow` 를 오프스크린으로 띄운다. `studio-ci.yml` 은 `QT_QPA_PLATFORM: offscreen`, install-qt-action 의 플러그인 경로, `RUN_SERIAL` 을 갖춰 통과할 것으로 보이지만, CI 에서 실제로 돈 기록은 아직 없다. 다음 push 의 CI 결과를 확인할 것.

## 고치지 않아도 되는 제안

1. 매직 넘버를 이름 있는 상수로 바꾼다.
   - 줄 높이 `23`(Ribbon.cpp:205): `SmallButton::sizeHint` 의 23(Ribbon.cpp:118)과 같은 값을 공용 `constexpr` 로.
   - 각도 칸 여백 `+ 6`(MainWindow.cpp:263).
   - 캡처 크기 `1845, 920`(main.cpp:24): `kShotSize` 같은 이름으로.
   - ctx 겹침 묶음 색인 `ctxCutKeys_[0],[1],[8],[9]`(MainWindow.cpp:337): 목록 순서가 바뀌면 조용히 어긋난다. 만들 때 포인터를 변수로 받아 두 목록에 함께 넣는 편이 안전하다.
2. `buildRibbon`(MainWindow.cpp:158-308, 약 150줄)에서 ② 페이지 부분(:237-273)을 `buildCutPage()` 로 떼면 읽기 쉽다.
3. `RibbonGroup::button()` 은 줄(`column`/`rows`)에 넣지 않으면 그룹 왼쪽 위에 떠 있는 위젯이 된다. 주석이 있지만, `rows` 가 id·글자 묶음도 받게 하는 방법도 있다.
4. `SheetView.cpp:248` `s.name.at(0)` 앞에 빈 이름 방어(`s.name.isEmpty() ? QChar('A') : s.name.at(0)`)를 둔다.
5. R4("모든 수치 모노"): ② 줄 2 의 `1° ·`·`15°` 는 고딕 `textLabel` 이다. 엄밀히 적용하면 모노다. 원문 시안과 비교한 판단은 ui-reviewer 몫이다.
6. `updateCutInfo` 는 미리보기 한 장마다 `setTitle`·`setText`·`setState` 를 부르고 그때마다 리본을 다시 배치한다. 이름 칸·각도 칸 폭을 고정해 비용은 작다. 끄는 동안 프레임이 떨어지면 값이 바뀔 때만 부르게 한다.
7. 시험이 레지스트리 `HKCU\Software\DongArch3DTest` 에 남기는 값(창 위치 등)은 지우지 않는다. 사용자 값과 섞이지 않으므로 문제는 없다.

## 사람 검토로 넘길 것
- (아래 human_review 목록과 같음)
