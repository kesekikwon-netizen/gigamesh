# JOB-007 검토: code

결론: **PASS**

검토 입력: `git -C D:\_dongarch_build\wt\.worktrees\JOB-007 diff 688b7bd..HEAD`(커밋 6542878 = 007a, 89846ba = 007b, 15개 파일 +977/−94), `spec.md`(수용 기준 35개, §0·§3·§8·§11). diff 를 직접 읽었다. build-log 는 builder 의 주장으로만 보고 diff 로 다시 확인했다. 기준 24-30 의 읽기 전용 명령은 worktree 에서 직접 실행했다. CTest 결과는 test-runner 의 verify.md 에서 가져왔다(17/17, `PY-SVG OK`·`PY-DXF OK`, 빌드 경고 0). job.json 의 CI 기록도 17/17 이다.

## 항목별 판정

| # | 항목 | 판정 | 근거 (file:line) |
|---|---|---|---|
| 1 | 명세 범위 | **pass** | 바뀐 파일 15개가 기준 30 의 허용 목록과 정확히 같다(직접 실행: 출력 없음, `git status --porcelain` 비어 있음). 제품 코드(studio/src)는 빈 줄·주석을 빼고 +277 / −77 이고, 직접 세어 확인했다. §0 의 300줄 지침 안이라 나누지 않은 판단이 맞다. 명세에서 벗어난 곳은 아래 「builder 이탈 판정」 표에 따로 적었고, 모두 수용할 만하다. |
| 2 | 금지 구역 | **pass** | `git diff --name-only 688b7bd HEAD -- GigaMesh` 의 출력이 없다(core·external·karch `DxfWriter` 포함). 기준 24 의 엔진·기존 시험 목록(ArtifactDoc·CutGesture·MeshIO·MeshRaster·Theme.h·test_pipeline/pose/side/cut)도 출력이 없다. `Plate.cpp` 에는 `CutPageText.h` include 가 0개다(기준 28 — 도판 엔진이 ② 페이지 헤더에 기대지 않음). |
| 3 | 규약(한국어·약어·Theme 토큰·주 단추·mm/Z-up) | **pass** | **한국어**: 새 화면 문자열은 모두 한국어다. 위치: CutPageText.h:35-44·58-63, MainWindow.cpp:276-279(툴팁)·294-300, ExportDialog.cpp:125·255·257, SheetView.cpp:232. 영어는 DXF 주석 `FRONT AZIMUTH … DEG (CCW, TOP VIEW)`(Plate.cpp:885) 하나뿐이다. 명세 §3.1 이 정한 글이고, 기존 DXF 주석 `RIM D.` 와 같은 R12 ASCII 관례다. **색**: 새 그리기는 `theme::OuterInk`·`theme::Caution`·`theme::hex()` 만 쓴다(Plate.cpp:349·430·761). 아이콘은 인자 색 `c` 를 쓴다(Ribbon.cpp:86-91). `QColor(숫자)`·`Qt::red` 류는 0, hex 는 21 → 19개로 줄었고 새 hex 값은 없다(기준 27 직접 실행). 기존 hex 줄(Plate.cpp:685-689·769)은 다시 배치하지 않았다(§3.3-5 지킴). **주 단추**: ② 는 `toEdit` 하나(MainWindow.cpp:310), ③ 은 기존 `export` 하나다. 시험 `primaryTiles` 가 판정한다(test_cutpage.cpp:428·481). diff 에 새로 들어간 `ToolState::Primary` 는 `toEdit` 뿐이다. **단위·좌표**: Plate.h 선언 주석이 「도면 mm, x = R, y = H」, 「바닥판 Z 0 → 최고점」이다. 중심선 연장은 화면 = 도면 3 mm(Plate.cpp:341-346), SVG = `3.0 / s` → 종이 3 mm(:755), DXF = 1:1 모델 3 mm(:850)로 셋 다 단위가 맞다. |
| 4 | 구조(중복·매직 넘버·함수 길이·에러 처리) | **pass** (경미한 지적은 제안으로) | **중복 제거**: 닫는 가로선 규칙을 `closingEnds()`(Plate.cpp:58-70) 하나로 화면·SVG·DXF 가 같이 쓴다. 기준선 층 id·이름·굵기·DXF 층은 `kRefStyles` 표(:73-81) 한 곳에 모았다. 고정 슬롯 찾기는 `firstVerticalSlot()`(MainWindow.cpp:684-688)으로 onPage·fixAngle 이 함께 쓴다. **에러 처리**: `plateRefLines` 는 미로드·확정 전이면 빈 값이고, 구경·저경이 0 이면 그 선을 만들지 않는다(:150-162). `closingEnds` 는 점이 2개 미만이면 빈 값이다. `fixAngle` 은 종단면이 없으면 return 한다(MainWindow.cpp:694). DXF 방위각은 확정 가드 안에서만 쓴다(Plate.cpp:883). 파일 열기 실패 경로는 기존 그대로다. **정수 0.1° 표기**: `llround` 로 부호·소수를 직접 조립해 `/fp:fast` 에서도 −0 이 나오지 않는다(:44-54). 기준 5·8 의 −0.04° 판정이 이를 확인한다. **매직 넘버**: 330·240(MainWindow.cpp:795-796, 240 은 :111 과 중복), 연장 3 mm(화면만 상수 `kRefExtPaperMm`, SVG·DXF 는 리터럴 `3.0`), 각 비교 `remainder(…, 360) <= 1e-9` 3곳(:681·721·991)이 남아 있다. 명세가 값 자체를 정했고 기존 코드 관례와 같아 FAIL 로 보지 않는다(→ 제안). 함수 길이: `refreshAll` 49줄로 늘었지만 읽을 만하다. `Plate::paint` 의 긴 길이는 원래부터다. |
| 5 | 테스트 | **pass** | **기준과 시험 1:1**: 기준 1-9 → test_reflines.cpp 의 `[J7-1]`(:321)·`[J7-2]`(:336)·`[J7-3]`(:433-437)·`[J7-4]`(:443-447)·`[J7-5]`(:340-341·410·420·424)·`[J7-6]`(:125-172·372-379)·`[J7-7]`(:381-382·414)·`[J7-8]`(:218-264·383·415·422·426)·`[J7-9]`(:387-406). 기준 12-20 → test_cutpage.cpp 의 `[8]`(:206)·`[J7-12]`(:222-233)·`[J7-13]`(:416-437)·`[J7-14]`(:440-454)·`[J7-15]`(:457-488)·`[J7-16]`(:490-501)·`[J7-17]`(:502-516)·`[J7-18]`(:518-546)·`[J7-19]`(:552-556)·`[15]`(:381-382). 기준 10·11 은 파이썬, 21-30 은 명령으로 판정한다. 꼬리표 `[J7-n]` 이 JOB-006 의 `[n]` 과 섞이지 않는다. **CTest 등록**: `studio_reflines`(studio/CMakeLists.txt:100-103) → 17종. **절대 경로 없음**: 출력 폴더는 `argv[1]` = `CMAKE_CURRENT_BINARY_DIR`, 없으면 cwd(test_reflines.cpp:315-316). **기존 SVG/DXF 시험**: test_side·test_pipeline 은 바뀌지 않았다. 판정 대상(`outline_axis` 의 polyline, DXF 02·04·06 층이 비지 않음, 층 존재 여부)은 새 출력에서도 그대로 참이다. 02 층에는 POLYLINE 이 남는다(Plate.cpp:855-858). verify CTest 17/17·CI 17/17 이다. |
| 6 | 문서 | **pass** | UI_RULES 정본을 갱신했다. R1 표 1·3·4행(docs/ui/UI_RULES.md:20·22·23), R3 주 단추(:59), R5 그리는 순서·기준선 규약(:71·74), R6 SVG 층·DXF 기준선 층·정면 방위각(:83-85). 기준 29 를 직접 실행해 모두 기대값이 나왔다. `docs/ui/mockup/` 에는 아직 `측정·탁본`·`외곽선·중심선` 이 남아 있다. 명세 §2·§6 이 목업 수정을 금지했으므로 정본(R1)과 목업이 어긋나는 문제는 사람 검토 1 의 몫이다. README 갱신 요구는 없다. |

## builder 이탈 판정 (build-log 「명세와 다르게 판단한 곳」 → diff 로 확인)

| # | 이탈 | diff 확인 | 판정 |
|---|---|---|---|
| 1 | `order_->setMinimumWidth(0)` → `1`(SheetView.cpp:233-236). 명세 §3.3-7 은 글자만 바꾸라고 했다. | 실제 1줄 변경과 이유 주석 2줄이다. Qt 레이아웃(`qSmartMinSize`)은 최소 폭 0 을 「정하지 않음」으로 보고 `minimumSizeHint` 를 쓴다. 그래서 기존 줄은 「글이 창 폭을 밀지 않게」 라는 의도대로 동작하지 않았고, 이 변경은 그 잠재 결함을 고친 것이다. | **수용**. 오케스트레이터가 decision.md 에서 안 A 로 승인했고, 기준 32 는 실제 글꼴 캡처로 판정한다. 대가로 좁은 창에서 글 끝이 말줄임 없이 잘린다 → 사람 검토. |
| 2 | DXF 정면 방위각은 슬롯 조건 없이 확정 뒤면 늘 쓴다(Plate.cpp:883-886). | `exportDxf` 는 `r.opts.slot` 을 보지 않고 늘 종단면(`displayOutline`·`displaySection`)을 쓴다(:827-899). | **수용**. 명세 §3.3-6 에도 조건이 없어 일관된다. |
| 3 | SVG 굵기를 `0.100/0.130/0.250` 으로 적는다(`n3`, :762). | 숫자 값은 같다. | 수용 |
| 4 | `kRefStyles` 표 하나를 화면·SVG·DXF 가 같이 쓴다(:73-81). | 동작이 같고 중복이 줄었다. pointer-to-member(`refs.*k.line`)는 조금 영리한 코드지만 읽을 만하다. | 수용 |
| 5 | `setFixedWidth(330)` 대신 `setMinimumWidth`/`setMaximumWidth`(MainWindow.cpp:795-796)를 쓴다. | 같은 효과이고, ②·홈으로 돌아올 때 240 / `QWIDGETSIZE_MAX` 로 되돌린다. | 수용 |
| 6 | 시험의 슬롯 판정은 `itemWidget` 안만 본다(`slotHas`, test_cutpage.cpp). | `clear()` 뒤 지연 삭제 전의 옛 위젯 때문에 거짓 참이 나오는 것을 막는다. | 수용(시험 품질 향상) |

## 확인한 동작 (FAIL 없음)

- **③ 에서 막이 돌지 않음**: 각도 키는 `keyPressEvent` 의 각도 키 처리보다 먼저 막힌다(MainWindow.cpp:1106). `- +` 는 두께 키라 각도와 무관하다. 3D 왼쪽 끌기는 `setCutTool(id == "cut")` 때문에 보는 방향이 된다(:819). `axX/axY` 타일은 ③ 리본에 없다. 키를 뗄 때의 `keyUp` 은 제스처가 비활성이면 `scheduleCut` 이 곧 멈춘다(:843-850).
- **고정 풀림 지점**은 명세 §3.5 의 8곳과 같다. 위치: :547·576·592(열기 3곳)·779(확정)·834(되돌리기)·876(beginCutInput, 슬롯 목록 갱신 포함)·976(중심축 맞춤)·994(X/Y 막, 「이미 그 각」 return 뒤). ③ 자동 고정은 `refreshAll` 맨 처음(:610)에서 `refreshSlots` 보다 먼저 일어나, 같은 갱신에서 슬롯 메모가 맞게 보인다.
- **가로 슬롯을 보며 고정**하면 `k = firstVerticalSlot()` 이 된다(:693). 그래서 `selectSlot` 이 옛 각으로 되돌아가 다시 고정하던 문제가 막힌다(기준 18 이 판정).
- **「기고 H」 머리표**: 구연선이 있으면 파선은 `−구경/2 − 2` 까지만 긋는다(:431-434). 라벨은 `AlignBottom` 으로 구연선 위에 놓인다. `labelBox` 는 `AlignBottom` 을 지원한다(:115-116). 구연선이 없으면 예전 동작 그대로다.
- `--edit`(main.cpp:53-55)은 200 ms 의 `--cut` 키 뒤 300 ms 에 실제 단추 경로로 들어간다. `onAction` → `flushCut` → ③ 자동 고정 순서이므로 12° 로 고정된다.

## 고치지 않아도 되는 제안

1. 3D 보조창 폭 `330`·`240` 을 이름 있는 상수로 둔다(예: `k3dEditWidth`, `k3dMinWidth`). 지금 `240` 이 MainWindow.cpp:111·795 두 곳에 있다.
2. 중심선 연장 3 mm 를 `kRefExtPaperMm` 하나로 묶는다. 지금은 `Plate::paint` 안의 지역 상수(:341)이고 SVG(:755)·DXF(:850)는 리터럴 `3.0` 이다. 익명 이름공간으로 올리거나 `kRefStyles` 옆에 두면, 값을 바꿀 때 세 곳을 따로 고치지 않아도 된다.
3. `std::fabs(std::remainder(a - b, 360.0)) <= 1e-9`(MainWindow.cpp:681·721·991)를 `sameAngle(a, b)` 작은 함수로 묶는다.
4. ③ 에서 `← →` 묶음을 숨길 때 `ctxCutKeys_[0]`·`[1]` 같은 번호를 쓴다(:625). 기존 `ctxDupKeys_` 처럼 이름 있는 묶음(예: `ctxMoveKeys_`)으로 두면 ctx 순서를 바꿀 때 깨지지 않는다.
5. `restore()` 의 무조건 `fixedAngle_.reset()`(:834): `angleFixed()` 가 이미 각을 비교하므로, 각이 실제로 바뀔 때만 풀어도 된다. 지금은 ② 에서 각과 무관한 되돌리기(B/T 높이 기준 등, :979 의 `pushUndo`)도 고정을 푼다. 명세 §3.5 그대로라 FAIL 은 아니다.
6. `azimuthText`(Plate.cpp:44-49)는 `cutpage::angleText` 와 같은 논리를 의도적으로 중복했다(명세 §3.3-3·C5, 계층 역전 방지). 후속 작업에서 Qt Core 만 쓰는 공통 표기 헤더로 내리면 두 곳이 어긋날 위험이 없어진다.
7. 「그리는 순서」 라벨(SheetView.cpp:232-236)이 잘릴 때 말줄임 표시와 툴팁(전체 글)을 붙이면 좁은 화면에서도 내용을 알 수 있다.
8. (확인 필요·사소) 007a 커밋 6542878 메시지 끝에 `Co-Authored-By` 줄이 없다(CLAUDE.md §6). 007b 커밋에는 있다. 코드 품질과는 무관하다.
