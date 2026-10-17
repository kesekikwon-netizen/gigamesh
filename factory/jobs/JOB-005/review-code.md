결론: PASS

# JOB-005 코드 품질 검토 (code-reviewer, 읽기 전용)

- 검토 대상: `D:\_dongarch_build\wt\.worktrees\JOB-005`, 브랜치 `factory/JOB-005`, `b033aef..804f60d`. 커밋 1개, 10파일, +937/−61.
- 입력: `git diff b033aef..HEAD` 를 직접 읽었고, `spec.md`(기준 13 수정 문구 포함)와 `decision.md` 를 봤다. build-log.md 의 주장은 diff 와 산출물로 다시 확인했다.
- 표기: **확인** = diff·파일·명령 출력으로 직접 확인함. **추정** = 코드를 읽고 추론했지만 실행으로는 확인하지 않음.
- 기준 16-22 의 grep 명령은 이 검토에서 다시 실행했다. 결과: 16 출력 없음 / 17 `0`·`1` / 18 `0`·`2` / 19 `1`·`1`·`1` / 20 `4`·`1`·`1`·`1`·`1` / 21 출력 없음·`0` / 22 10파일 모두 §6 표 안, `GigaMesh docs CLAUDE.md` 출력 없음.
- 기준 13·14·15 는 독립 검증 산출물에서 확인했다.
  - 13: `artifacts/ctest_all_rerun.log:33` `100% tests passed out of 15`.
  - 14: `LastTest_rerun.log` 에서 세 문자열이 각 `1`번.
  - 15: `02_build.log` 의 `warning` 0개, `verify-summary-studio.md` 마지막 줄 `결과: PASS`.

## 1. 항목별 판정

| # | 항목 | 판정 | 근거 (file:line) |
|---|---|---|---|
| 1 | 명세 범위 | PASS (크기는 사람 확인 필요) | **확인**: 바뀐 파일 10개가 모두 §6 표에 있다(기준 22). 명세 밖 추가는 다섯 가지이고 모두 작고 이유가 있다. ① `updateCutInfo()` 분리(`MainWindow.cpp:571-577`, `refreshPreview` 에서 각도 숫자만 갱신하려고). ② 키 도우미 `cutKeySign`/`isModifierKey`(`MainWindow.cpp:53-63`). ③ `setTimerType(Qt::PreciseTimer)`(`:120`). ④ `keyUp` 이 비활성 상태에서는 확정 마감을 잡지 않는 가드(`CutGesture.h:36`). ⑤ 아래 §2 의 이탈 3곳. **크기**: 제품(studio/src) 추가는 +343줄이고, 빈 줄과 주석만인 줄을 빼면 292줄이다(이 검토에서 다시 셈). §0 의 "제품 300줄을 넘으면 멈추고 알린다"를 어느 수로 볼지가 갈린다. builder 는 292 로 보고 build-log 에 적었을 뿐 멈추지 않았다 → 사람 확인 |
| 2 | 금지 구역 | PASS | **확인**: `git diff --name-only b033aef...HEAD -- GigaMesh docs CLAUDE.md` 출력 없음. `karch/*` 변경 없음 |
| 3 | 규약 | PASS | **확인**: 새 UI 문자열은 2개이고 둘 다 한국어다. `MainWindow.cpp:285` `막 1° · Shift 15°`(앞에 kbd 배지 `← →` — R8 지킴), `Plate.cpp:47-48` 거부 문구. 화면 문자열에 "LOD" 같은 약어는 없다(LOD 는 주석에만). 새 색이나 `ToolState::Primary` 추가 없음(기준 21). 단위가 드러나는 이름(`nowMs`·`baseDeg`·`deltaDeg`·`kPreviewMs`·`wallSeedMm`)이 일관된다. 좌표 주석은 Y=0 막, +Z 오른손 법칙, 자연 좌표 x ≥ 0 = 단면으로 맞다(`ArtifactDoc.h:46-57, 94-96`, `ArtifactDoc.cpp:667`) |
| 4 | 구조 | PASS (비차단 지적 있음) | **확인**: 매직 넘버 점검 결과는 §3. `CutGesture.h` 는 Qt 에 의존하지 않는다(§4). `buildSectionLoops` 를 순수 함수 `sliceSectionY0` 로 나눴고 멤버를 읽거나 쓰지 않는다. 멤버 대신 인수 `inferInner`·`wallSeedMm` 과 반환값 `s.wallMm`·`s.estZMin/Max` 를 쓴다(`ArtifactDoc.cpp:850-1032`). 적용 쪽은 `:843-848` 이다. 에러 처리: 확정 전·미로드 가드(`ArtifactDoc.cpp:608-609, 619-620`). 파일 열기 실패 시 미리보기를 정리한다(`MainWindow.cpp:461-465`). 내보내기 거부는 파일을 만들기 전에 한다(`Plate.cpp:495, 516, 538, 742`). 작은 중복은 §6 제안에 적었다 |
| 5 | 테스트 | PASS | **확인**: 수용 기준 1-12 를 모두 `tests/test_cut.cpp` 에서 `CHECK` 로 판정한다. 인쇄만 하는 항목은 없다. 기준 10 도 비어 있지 않음과 유한함은 판정한다. 등록: `CMakeLists.txt:98-102`(`studio_cut`, `RUN_SERIAL TRUE`). 출력 경로는 argv 로 받은 빌드 폴더다(`test_cut.cpp:570`). 절대 경로 `D:\…` 에 쓰지 않는다. 기존 시험은 그대로다(기준 16). 빠진 점(비차단): builder 이탈 1(`release` 가 누름 상태를 풂)은 `CutGesture` 단위에서 시험할 수 있는데 시험이 없다 → §6 |
| 6 | 문서 | PASS | **확인**: 명세 §2·§6 이 `docs/**`·`CLAUDE.md` 수정을 금지한다. 정면각 부호 문장은 사람 검토 1 승인 뒤 별도로 한다. 새 단축키는 화면 kbd 배지로 알린다(UI_RULES R8). README 에는 단축키 절이 없다 |

## 2. builder 가 명세에서 벗어난 3곳 판정 (decision.md)

| # | 이탈 | 판정 | 근거 |
|---|---|---|---|
| 1 | `CutGesture::release` 가 `held_` 도 푼다 | **타당** (확인) | 명세 규칙 ⑧은 release 를 "확정 마감 now"로만 정했고, 규칙 ⑨는 `!held` 일 때만 확정한다. 그래서 명세 글자대로면, 키를 누른 채 Ctrl+P 를 누를 때 `flushCut()`(`MainWindow.cpp:909`)이 확정하지 못한다. 그러면 `openExport` 가 미리보기 상태로 열리고, `refusePreview`(`Plate.cpp:42-49`)의 거부 문구가 사용자에게 보인다. 이는 명세 §3.6 "사용자는 거부를 보지 않는다"와 충돌한다. builder 는 `CutGesture.h:23`(`held_ = false;`)으로 flush 가 늘 확정되게 해서 이 충돌을 풀었고, 이유를 주석(`:20`)에 적었다. 부작용(**추정**): 키를 누른 채 단면 도구로 3D 를 클릭하거나 끌다 놓으면 그 순간 확정된다. 이후 자동 반복이 오면 새 제스처가 시작되어 되돌리기 칸이 2개가 된다. 드물고 해가 없다. 기준 11a-f 에는 영향이 없다(11d 는 release 를 부르지 않음, 독립 검증 `[11d] … commit@3300` 통과) |
| 2 | `openPath` 가 파일을 못 열면 미리보기를 확정한다 | **타당** (확인) | `ArtifactDoc::loadFile` 은 실패하면 멤버를 하나도 바꾸지 않고 돌아간다(`ArtifactDoc.cpp:253-258`). `openPath` 는 첫 줄에서 `cancelCut()`(`MainWindow.cpp:456`)으로 제스처를 비활성으로 만든다. 그래서 이 4줄(`:462-465`)이 없으면 문서는 `isPreviewing()==true` 인 채로 남는다. 이후 `flushCut()` 은 비활성 제스처라 아무것도 하지 않고(`:733`), 다음 내보내기에서 거부 문구가 보이게 된다. 이 틈이 생기는 경로는 끌어놓기(`:997`)와 감시 폴더(`:449`)다 — onAction 을 거치지 않으므로 flush 가 앞서지 않는다. 확정을 모달 `QMessageBox` 앞에서 하는 것도 맞다. 대안으로 "로드가 성공한 뒤에만 cancelCut"이 있으나, 모달 창이 마우스·키 뗌을 가로채면 제스처가 "누르는 중"으로 남을 수 있어 builder 안이 더 단단하다(**추정**). 남는 점(**추정**, 비차단): 확정하는 각은 마지막으로 보인 미리보기 각(`previewAngle_`)이라, 그 뒤 ≤ 16 ms 사이의 입력은 버려진다. 첫 미리보기 전(첫 입력 뒤 16 ms 안)에 실패하면 각도 변화 없는 되돌리기 칸이 1개 남는다 |
| 3 | Shift·Ctrl·Alt·Meta 만 누르면 확정하지 않는다 | **타당** (확인) | `MainWindow.cpp:908-909` 를 보면 Ctrl 조합은 `cutSign` 이 0 이므로 본 키(Z·P·O·Y)에서 flush 된다. 수식 키에서 flush 하지 않아도 결과는 같고 시점만 한 키 늦어진다. Shift 단독에서 flush 하면 → 를 누른 채 Shift 를 더할 때(15° 로 바꾸기) 그 자리에서 원본 계산(≈1.36 s 멈춤)이 끼어든다 — builder 의 판단이 명세 의도(제스처 하나 = 되돌리기 한 칸)에 더 맞다. 목록 밖 키(AltGr, CapsLock, 한/영)는 flush 되는데, 일찍 확정될 뿐 해는 없다(**추정**) |

## 3. 매직 넘버

| 값 | 위치 | 판정 |
|---|---|---|
| 16 ms | `CutGesture.h:11` `kPreviewMs` | 이름 있는 상수 (확인) |
| 300 ms | `CutGesture.h:12` `kCommitMs` | 이름 있는 상수 (확인) |
| 0.5°/px | `View3D.cpp:191` `cutAngleDragged(d.x() * 0.5)` | 리터럴. 이번 diff 가 바꾼 줄이 아니고(기존 줄), 기준 19 가 이 문자열 그대로를 요구한다 → 이번 결함 아님 (확인) |
| 15° (와 1°) | `MainWindow.cpp:938` `? 15.0 : 1.0`, 단추 `:834` `1.0` | 리터럴, 바로 위 주석(`:936`)이 뜻을 설명한다. 임계값이 아니라 입력 한 걸음 크기이고, 기존 코드도 걸음을 리터럴로 쓴다(±0.5 mm `:964, 970`). → **비차단 지적**. 화면 문구 `막 1° · Shift 15°`(`:285`)와 값이 따로 놓여 함께 바꾸지 않으면 어긋날 수 있다 |
| 1e-9° (0 판정) | `ArtifactDoc.cpp:625`, `:667` | 명세는 "같은 함수·같은 부호·같은 0 판정"을 요구하는데, 같은 판정식이 두 곳에 복사돼 있다 → 비차단 (§6 도우미 제안) |

## 4. `CutGesture.h` Qt 무의존

**확인**: `#include` 가 하나도 없다(`#pragma once` 다음에 바로 `namespace studio`, `CutGesture.h:5-7`). `long long`·`double`·`bool`·enum class 만 쓰고, 시각은 밖에서 넣는다. 헤더만 있는 파일로 `studio_core` 소스 목록에 올라 있다(`CMakeLists.txt:57`). 상태 기계 규칙 ①-⑩ 을 줄마다 대조했다.
- 시작할 때 base 를 쓰고 활성 중에는 무시한다: `:71-74`.
- 미리보기 마감은 이미 잡혀 있으면 다시 잡지 않는다(throttle): `:77`.
- drag·keyDown 은 확정 마감을 지운다: `:18, :31`.
- keyUp·click 은 +300 ms 로 잡는다: `:36, :41`.
- poll 은 확정을 먼저 본다: `:48-59`.
- `nextDueMs` 는 누르는 중이면 확정 마감을 빼고 본다: `:61-65` — 0 ms 타이머가 계속 도는 일은 없다.

명세 규칙과 다른 곳은 release(이탈 1)와 keyUp 의 `if (active_)` 가드뿐이다. 이 가드는 `nextDueMs` 에 이미 `active_` 조건이 있어 동작은 같고, 안전을 겹으로 둔 것이다.

## 5. MainWindow `flushCut` / `cancelCut` 호출 자리

| 명세 §3.5 요구 | 위치 | 결과 |
|---|---|---|
| `onAction`(rotate·cutMove 제외, export·svg·dxf 포함) | `MainWindow.cpp:775` | 있음 (확인) |
| `onChip` | `:875` | 있음 |
| 되돌리기 / 다시 | `:254`, `:260` | 있음 |
| `selectSlot` | `:613` | 있음 |
| 각도 키가 아닌 `keyPressEvent`(1~5·R·W·−·+·Esc·S·Ctrl 조합 포함) | `:909` (수식 키 단독만 예외 — 이탈 3) | 있음 |
| 로드 첫머리 `cancelCut` | `openPath :456`, `openSample :494`, `openArtecBlindSample :509` | 있음 |
| 놓기 | `cutDragReleased` → `release` + `onCutTimer` (`:129-132`) | 있음 |

명세 목록에 없는 진입점도 따라가 봤다(확인).
- 끌어놓기 `dropEvent :997`, 감시 폴더 `:449`, 시작 화면 신호 `:76-78`, 최근 메뉴 `:783-786`: 모두 `openPath`/`openSample` 로 가므로 cancelCut 이 걸린다(+ 이탈 2).
- 슬롯 순서 끌기 `rowsMoved :347-355`: 각도를 건드리지 않으므로 해가 없다.
- Tab 최대화 `eventFilter :895-901`, `closeEvent :1000`: 문서를 바꾸지 않는다.
- 커서 좌표 `cursorMm`: 읽기만 한다.

→ **빠진 자리 없음**. 남는 틈 하나(**추정**, 비차단): 각도 키를 누른 채 창이 포커스를 잃으면 `keyRelease` 가 오지 않아 제스처가 "누르는 중"으로 남는다. 도판은 미리보기 상태로 남지만, 다음에 하는 어떤 동작이든 flush 를 거치므로 내보내기는 안전하다.

나머지 연결도 명세대로다(확인).
- `previewFrontAngle` 을 부르는 곳은 1곳(`:711`).
- `refreshPreview` 는 `docChanged`/`refreshAll`/`refreshSlots` 를 부르지 않는다(`:721-726`).
- `beginCutInput` 덕분에 제스처 하나가 되돌리기 한 칸이다(`:728-730`).
- `MainWindow::setFrontAngle` 은 지워졌다.
- 확정 전 가로채기에 ← → 를 넣었다(`:927`).
- `keyReleaseEvent` 는 자동 반복이 아닌 뗌만 받고, 아니면 부모에게 넘긴다(`:982-989`).

## 6. 고치지 않아도 되는 제안

1. 15°·1° 걸음을 `constexpr double kCutKeyStepDeg = 1.0, kCutKeyBigStepDeg = 15.0;`(익명 이름공간, `cutKeySign` 옆)으로 두면 단추(`:834`)와 키(`:938`)가 한 값을 쓴다. 문구 `:285` 는 기준 20 때문에 리터럴로 남겨야 한다.
2. `rotateZ` 와 0 판정을 묶은 도우미 `rotateFront(KA::MeshData&, double deg)` 를 하나 두고 `recompute`(`ArtifactDoc.cpp:667-670`)와 `previewFrontAngle`(`:625-626`)이 함께 쓰면, "같은 함수·부호·0 판정"을 코드가 보장한다.
3. `cutDragReleased` 처리(`MainWindow.cpp:129-132`)는 `flushCut()` 본문과 같다 → `flushCut()` 한 줄로 줄일 수 있다.
4. (**추정**) 세로로만 끌어도 `cutAngleDragged(0)` 이 나와 제스처가 시작된다. 그러면 놓을 때 원본을 다시 계산하고(≈1.36 s), 빈 되돌리기 칸이 생긴다(전에는 마우스 이동마다 원본을 계산했으니 지금이 훨씬 낫다). 끌기 처리(`:123`)에서 `d == 0` 이면 돌아가게 하면 된다. 확정 각이 지금 `frontAngle()` 과 같으면 `setFrontAngle` 을 건너뛰는 안도 있다.
5. `test_cut.cpp` 에 이탈 1 시험을 더하면 좋다: `keyDown(0,1,0); release(10); poll(10)==Commit`. 11a 의 간격 판정 `16`/`17`(`test_cut.cpp:500-501`)은 `CutGesture::kPreviewMs` 로 쓰는 편이 낫다.
6. 포커스를 잃을 때(`changeEvent(QEvent::ActivationChange)`) `flushCut()` 을 부르면 위 틈이 닫힌다(builder 제안과 같음, 후속 작업).
7. 놓을 때 원본을 계산하는 동안(≈1.36 s) `QApplication::setOverrideCursor(Qt::WaitCursor)` 를 켜면 멈춤이 사용자에게 보인다. 비동기 계산은 범위 밖이다.
8. `previewFrontAngle` 은 16 ms 마다 `lodBase_` 를 면까지 통째로 복사한다(`ArtifactDoc.cpp:624`). 실측 0.70 ms 라 여유는 충분하지만, 정점만 복사하면 더 줄어든다.

## 7. 사람이 판단할 항목

- 제품 줄 수: +343줄(빈 줄·주석 제외 292줄). 명세 §0 의 "300줄 넘으면 멈추고 005a/005b" 를 어느 수로 볼지 정하고, 나누지 않은 판단을 승인할지.
- builder 이탈 3곳: 검토자는 셋 다 타당하다고 판정했다. 사람 검토 7(입력 규칙)과 함께 승인할지.
- [좌표계] 정면각 부호를 반전했다(+ = 위에서 보아 반시계, `ArtifactDoc.cpp:667-670`). X 막 = +90° 의 뜻(사람 검토 1·2). 세션 안에 저장한 슬롯 각의 뜻이 바뀐다.
- [수치] 미리보기(LOD)와 원본 단면의 차이: 미스캔 예제 θ=45° 에서 하우스도르프 7.053 mm(평균 0.739 mm), 다른 9줄은 0.74-2.56 mm(사람 검토 3·4).
- 놓을 때 멈춤 1364.8 ms(77,120 면)와 3D 다시 그리기 14.94 ms — 한 프레임 16 ms 예산을 거의 다 쓴다(사람 검토 6).
- 포커스를 잃었을 때 "누르는 중"이 남는 틈을 후속 작업에서 막을지.
- 손 확인(사람 검토 9): 끄는 감각, ← → 가 다른 위젯에 먹히는지, Ctrl+Z 를 누르면 제스처 전 각도로 돌아가는지.
