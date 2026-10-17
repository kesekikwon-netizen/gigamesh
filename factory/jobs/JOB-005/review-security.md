결론: PASS

# JOB-005 보안·안전 검토 (security-reviewer)

- 대상: `D:\_dongarch_build\wt\.worktrees\JOB-005`, `factory/JOB-005` HEAD `804f60d`, 기준 `b033aef`
- 입력: `git diff b033aef..HEAD`(10개 파일, +937/−61)를 직접 읽었고, 명세 `spec.md` 와 `decision.md` 를 함께 보았다. 독립 검증 로그 `artifacts/ctest_studio_cut_V.log`·`03_ctest.log`·`02_build.log` 는 대조용으로만 썼다. build-log.md 는 참고하지 않았다.
- 표기: **확인** = diff·코드·로그를 직접 읽거나 grep 해서 확인한 것. **추정** = 코드 흐름으로 추론했지만 실행해 보지는 않은 것.

## 1. 점검 요약

| # | 점검 항목 | 판정 | 근거 (file:line) | 구분 |
|---|---|---|---|---|
| 1 | 무한 0 ms 타이머 / 바쁜 반복 | 문제 없음 | `CutGesture.h:48-65` 에서 `poll` 과 `nextDueMs` 의 조건이 같다(확정: `active_ && !held_ && commitAt_>=0`, 미리보기: `previewAt_>=0`). 그래서 `nextDueMs` 가 돌려준 마감은 now ≥ due 인 `poll` 에서 반드시 소비된다. 비활성이면 `previewAt_=commitAt_=-1`(`:43-46`, `:48-52`)이고, 이때 `scheduleCut` 이 타이머를 멈춘다(`MainWindow.cpp:699-706`). `onCutTimer` 반복은 많아야 2회다(Commit 이 미리보기 마감도 지우고, Preview 는 자기 마감을 지운다, `MainWindow.cpp:708-719`). 대기 시간 `int(max(0, due−now))` 는 최대 300 ms 라 넘치지 않는다 | 확인 |
| 2 | 키를 누르고 있는 동안의 타이머 | 문제 없음 | `keyDown` 이 `commitAt_=-1`(`CutGesture.h:28-32`)로 두고, held 중에는 `nextDueMs` 가 확정 마감을 빼므로 미리보기 마감만 남는다. 자동 반복은 16 ms 합치기를 거친다. 시험 11d 에서 3000 ms 동안 확정 0회 | 확인 |
| 3 | 재진입: 타이머 콜백 중 중첩 이벤트 루프 | 문제 없음 | `onCutTimer` 가 부르는 `previewFrontAngle`·`refreshPreview`·`setFrontAngle`·`refreshAll` 경로에 `processEvents`·`exec()`·`QMessageBox` 가 없다. grep 결과 `exec()` 는 `MainWindow.cpp:767`(ExportDialog)와 QMenu 둘뿐이다. 그래서 콜백 안에서 문서가 바뀌지 않는다 | 확인 |
| 4 | 재진입: 문서 교체 뒤 남은 타이머 | 문제 없음 | 세 로드 함수가 모두 첫 줄에서 `cancelCut()` 을 부른다(제스처 비움 + 타이머 정지, `MainWindow.cpp:456, 494, 509, 738-741`). 로드 성공은 `loadMesh→unconfirm→recompute→clearPreview` 를 거친다(`ArtifactDoc.cpp:235-251, 473-477, 647`). 엔진도 확정 전·미로드 입력을 무시한다(`ArtifactDoc.cpp:608-609, 619-620`). 화면 입력은 확정 전 끌기(`MainWindow.cpp:124`)와 각도 키(`:920-931`)를 막는다. 그래서 옛 각도가 새 문서에 적용되지 않는다 | 확인 |
| 5 | 파일 열기 실패 중 미리보기 | 안전 | `loadFile` 은 `readMeshAny` 가 실패하면 상태를 하나도 바꾸지 않고 돌아간다(`ArtifactDoc.cpp:253-259`). 실패 시 `commitPreview()`→`refreshAll`(`MainWindow.cpp:461-465`)로 미리보기를 확정한 뒤 경고창을 띄우므로, 경고창의 중첩 루프에 들어가기 전에 제스처·타이머가 이미 비어 있다 | 확인 |
| 6 | 미리보기 중 내보내기 거부가 모든 내보내기 함수 첫머리에 있는가 | 충족 | `Plate.cpp` 에서 파일을 쓰는 곳은 497(QPdfWriter)·527(img.save)·540(QFile)·745(DxfWriter::Open) 넷이고, 넷 모두 각 함수 첫 줄에서 `refusePreview` 를 부른다: `exportPdf` 494-496, `exportPng` 514-517, `exportSvgLayered` 536-539, `exportDxf` 741-743. `refusePreview`(`:42-49`)는 `err==nullptr` 도 견딘다. 다른 파일 쓰기 경로를 grep 했다(ExportDialog 는 이 네 함수만 부른다, `ExportDialog.cpp:279-282, 297`). 시험 8 에서 4종 모두 `ok=0 file=0` | 확인 |
| 7 | 화면에서 내보내기 전에 먼저 확정하는가 | 충족 | `onAction` 첫 줄(`MainWindow.cpp:775`, rotate·cutMove 제외), Ctrl+P(`:909`→`:913`), 되돌리기·다시(`:254, 260`), `onChip`(`:875`), `selectSlot`(`:613`)에서 먼저 확정한다. ExportDialog 는 모달이라 열린 동안 각도 입력이 들어오지 않는다 | 확인 |
| 8 | 경로 하드코딩 | 없음 | diff 의 추가 줄에서 `[A-Za-z]:[\\/]`, `/media/`, `/home/`, `/tmp`, URL, `QProcess`, `system(`, `ShellExecute`, `QNetwork`, `QDesktopServices` 를 grep 했다 → 일치 0. `qEnvironmentVariableIsSet("CI")`(시험 기준 배수)와 시험 프로세스 안의 `qputenv("QT_QPA_PLATFORM","offscreen")` 만 있다 | 확인 |
| 9 | 시험이 출력 폴더 밖에 쓰는가 | 밖에 쓰지 않음 | `test_cut.cpp:570` 에서 출력 폴더 = `argv[1]`(CTest 는 `${CMAKE_CURRENT_BINARY_DIR}` 를 넘긴다, `CMakeLists.txt:100`). 쓰기·지우기는 `out + "/cut_refuse.{svg,pdf,png,dxf}"` 뿐이다(`:393-394, 404`). 다른 파일 I/O·QSettings 는 없다 | 확인 |
| 10 | 새 입력 경로: 마우스 놓기 신호 | 문제 없음 | `View3D.cpp:200-205` 는 상태를 정리한 뒤 마지막 줄에서 신호를 보낸다. 받는 쪽(`MainWindow.cpp:129-132`)은 비활성일 때 `release` 가 아무것도 하지 않고 `poll` 은 None 을 돌려준다 | 확인 |
| 11 | 새 입력 경로: 키 자동 반복·뗌 | 문제 없음 | 키 누름은 자동 반복을 포함해 `keyDown` 으로 간다(`MainWindow.cpp:935-943`). 뗌은 자동 반복이 아닌 것만 `keyUp` 으로 간다(`:982-989`). Ctrl 조합은 각도 키에서 뺀다(`:908`) | 확인 |
| 12 | 3D 정점 색 인덱스 범위 | 안전 | `previewLod_ = lodBase_` 복사본이라 정점 수와 순서가 `lodColors_` 와 같다. 또 `View3D.cpp:92-95` 가 크기를 먼저 확인한다 | 확인 |
| 13 | 새 파서·외부 실행·네트워크 | 없음 | 미리보기는 기존 `KA::Slice`·`buildLodMesh` 결과만 쓴다. 새 파일 파서는 없다 | 확인 |
| 14 | 의존성·라이선스·바이너리·비밀값 | 문제 없음 | 새 라이브러리 없음. `CutGesture.h` 는 시각을 밖에서 넣는 독자 상태 기계로, 참고 코드 `CutlineController.cpp`(디바운스 QTimer)와 구조가 다르다(복사 아님). 바이너리·토큰 없음. 변경 파일은 `studio/**` 10개뿐(GigaMesh·docs·CLAUDE.md·.github 변경 0) | 확인 |
| 15 | CI·스크립트 | 문제 없음 | 워크플로 변경 없음. `RUN_SERIAL TRUE`(`CMakeLists.txt:102`)로 `--parallel 4` 와 시간 측정이 겹치지 않는다 | 확인 |
| 16 | `/utf-8` (한글 문자열) | 충족 | `CMakeLists.txt:16` 에 `/utf-8` 이 있다. 새 한글 리터럴(Plate 거부 문구, ctx 배지, 시험 출력)은 로그에서 정상이고, `02_build.log` 의 warning 은 0 | 확인 |

## 2. 지적 (모두 low/info — FAIL 사유 없음)

| ID | 심각도 | 위치 | 내용 · 오동작 시나리오 | 권고 | 구분 |
|---|---|---|---|---|---|
| L1 | low | `CutGesture.h:28-37`, `MainWindow.cpp:982-989` | 화살표 키를 누른 채 창 포커스를 잃으면(Alt+Tab 등) 뗌 이벤트가 오지 않아 `held_` 가 남는다. 그 사이 제스처는 계속 활성이고 미리보기 단면이 남으며, 다음 동작 때까지 원본으로 확정되지 않는다. 마찬가지로 끄는 중 모달 경고창(감시 폴더 열기 실패)이 뜨면 View3D 의 `drag_` 가 남아, 버튼 없이 마우스만 움직여도 각도가 바뀔 수 있다. 내보내기·슬롯 저장·되돌리기는 `flushCut` 으로 먼저 확정하므로 결과물에는 영향이 없다 | `WindowDeactivate`/`focusOutEvent` 에서 `cut_.keyUp(now); scheduleCut();`(또는 `flushCut()`)을 부른다. View3D 는 `mouseMoveEvent` 에서 `e->buttons()==Qt::NoButton` 이면 놓은 것으로 처리한다 | 추정(Qt 의 포커스·모달 동작은 실행 확인하지 않음) |
| L2 | low | `MainWindow.cpp:456, 461-465` | 열기에 실패하면 `cancelCut()` 이 먼저 목표각을 버리기 때문에, 확정되는 각은 마지막으로 그린 미리보기 각(`previewAngle_`)이다. 마지막 16 ms 사이에 들어온 입력은 잃을 수 있다. 안전 문제는 아니다(상태는 일관됨) | `cancelCut()` 전에 `cut_.active()`·`cut_.target()` 을 저장해 두고, 실패하면 `doc_.setFrontAngle(target)` 을 부른다 | 확인 |
| L3 | info | `MainWindow.cpp:775-779` | `sample`·`blindSample` 은 `onAction` 첫 줄의 `flushCut` 으로 원본 전체를 계산한 뒤(예제 ≈ 1.36 s, 로그 `[d] sample full median=1364.8 ms`) 곧바로 문서를 버리고 새로 연다. 시간 낭비일 뿐 위험은 없다(`open`·`recent` 는 취소할 수 있으므로 먼저 확정하는 것이 맞다) | 예제 열기 두 id 는 flush 대상에서 빼도 된다(로드에서 `cancelCut` 을 부르므로) | 확인 |
| L4 | info | `test_cut.cpp:403-409, 570` | 시험이 끝나도 출력 폴더에 `cut_refuse.svg`(1.5 MB)가 남는다. 인자 없이 직접 실행하면 현재 폴더에 쓴다(CTest 는 늘 빌드 폴더를 넘김) | 확인이 끝나면 `QFile::remove` 로 지운다. 인자가 없으면 `QDir::temp()` 의 하위 폴더를 쓴다 | 확인 |
| L5 | info | `test_cut.cpp:415-416` | 환경 변수 `CI` 만 있으면 시간 기준이 16 ms 에서 64 ms 로 풀린다(명세 그대로). 로컬에서 `CI` 를 켜 두면 판정이 느슨해진다 | 인쇄 줄에 `(CI)` 표시가 이미 있으므로 기록만 남긴다 | 확인 |
| L6 | info | `MainWindow.cpp:344-346, 612-613, 579-581` | `selectSlot`→`flushCut`→`refreshAll`→`refreshSlots` 가 QListWidget 을 그 위젯 자신의 `currentRowChanged` 처리 안에서 비우고 다시 만든다. 같은 패턴이 이미 `selectSlot`→`refreshAll`(`:618-619`)에 있었다. `QSignalBlocker` 로 다시 들어오는 것은 막혀 있다 | 필요하면 `QTimer::singleShot(0, …)` 으로 늦춘다. 이번에 고칠 필요는 없다 | 추정(기존 패턴) |

## 3. decision.md 의 builder 명세 이탈 3건 — 보안·안전 관점 의견

1. **`release` 가 키 누름 상태도 푼다**(`CutGesture.h:21-26`): 안전한 쪽이다. 이것이 없으면 키를 누른 채 Ctrl+P 를 눌렀을 때 `flushCut` 이 확정하지 못하고(poll 이 `!held_` 를 요구), 엔진이 내보내기를 거부한다.
2. **`openPath` 가 실패하면 미리보기를 확정한다**(`MainWindow.cpp:461-465`): 꼭 필요하다. 이것이 없으면 제스처는 `cancelCut` 으로 비었는데 문서만 미리보기 상태로 남는다. 그러면 `flushCut` 은 아무것도 하지 않고, 각도를 다시 만질 때까지 모든 내보내기가 거부된다.
3. **수식 키만 눌렀을 때는 확정하지 않는다**(`MainWindow.cpp:61-63, 909`): 안전하다. 끄는 중 Shift 로 15° 단계로 바꿀 수 있고, Ctrl+문자 단축키는 문자 키가 확정을 일으킨다.

## 4. 미확인

- 실제 GUI 에서 포커스를 잃거나 모달 창이 뜰 때 키·마우스 놓기가 전달되는지(L1). 손 확인(사람 검토 9)에 맡긴다.
- 기존 별건: 모달 ExportDialog 가 열린 동안 감시 폴더 자동 로드로 문서가 바뀌는 문제(JOB-004 권고, 명세 §2 범위 밖). 이번 변경으로 나빠지지 않았다.

## 5. 고치지 않아도 되는 제안

- L1: 창 비활성 시 `keyUp`, View3D 버튼 상태 확인.
- L2: 열기 실패 시 목표각으로 확정.
- L3: 예제 열기 두 id 는 flush 대상에서 빼기.
- L4: 시험 산출물 정리.
