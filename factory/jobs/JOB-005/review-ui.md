결론: PASS

# JOB-005 UI 규칙 검토 (ui-reviewer)

- 대상: `D:\_dongarch_build\wt\.worktrees\JOB-005`, `git diff b033aef..HEAD`(커밋 804f60d, 10개 파일, +937/−61)를 직접 읽었다. 명세 `factory/jobs/JOB-005/spec.md`, 결정 기록 `decision.md`, 정본 `docs/ui/UI_RULES.md`·`docs/ui/mockup/`과 대조했다.
- 캡처: `factory/jobs/JOB-005/artifacts/shot_*.png`(= `D:\_dongarch_build\verify_job005\studio\shots\`, 22:09 생성, exe는 22:08 빌드)와 JOB-004 `artifacts/shot_*.png`.
- 이번 작업은 ② 리본 페이지를 만들지 않는다(JOB-006). 그래서 페이지·탭 이름이 없는 것은 판정에 넣지 않았다.
- 오프스크린 캡처에서는 한글이 모두 □로 보인다. 폰트를 못 찾은 문제로 기록하고 레이아웃만 판정했다(JOB-004 때와 같다).

## 1. 규칙별 판정

| 규칙 | 판정 | 근거 (file:line · 캡처) | 확인/추정 |
|---|---|---|---|
| R1 구조 | pass | 새 Dock·패널·창이 없다(diff에 `addDock`·`QDockWidget`·`setFixed` 추가 0건). 새 위젯은 ctx 줄 안의 라벨 2개뿐이다(`MainWindow.cpp:285`). ctx 줄은 그대로 36 px(`MainWindow.cpp:270` `setFixedHeight(36)`)이고, 도구를 들 때만 보이는 규칙도 그대로다(`:668`, `:677`, `:559`). `shot_main.png`는 1845×920이고 sha256 `32c4d7b9…`가 JOB-004와 같다. 탭·리본·슬롯·3D·도판·상태줄 배치가 같다. | 확인 |
| R1 ctx 줄 폭 | pass(주의) | 단면 도구를 들면 키 안내가 6쌍에서 7쌍이 된다(목업 `dongarch3d_mockup.html:382-384`은 3쌍). Malgun Gothic 12 px·Consolas 11 px로 재 보면 ctx 줄 최소 폭이 약 1712 px에서 약 1847 px로 는다. 기본 창은 1560 px(`MainWindow.cpp:68`)다. | 추정(PIL 글꼴 측정, 실행 캡처 없음) |
| R2 색상 | pass | 수용 기준 21 grep(QColor 숫자·qRgb·hex·Qt 색 이름)의 추가 줄 0건. diff에 `theme::`·`setStyleSheet`·`style=` 추가 0건이다. 단면선 색 코드도 바뀌지 않았다. | 확인 |
| R3 도구 3단계 | pass | diff에 `ToolState`·`setState` 변경 0건(`ToolState::Primary` 추가 0). 손에 든 도구 처리(`setHandTool`, `MainWindow.cpp:664-678`)는 그대로다. 각도 키·끌기는 도구 상태를 바꾸지 않는다. | 확인 |
| R4 글꼴 | pass | 새 라벨 `막 1° · Shift 15°`는 기존 ctx 키 설명 라벨(`절단면 1° 이동`, `추정 두께 ±0.5mm`)과 같은 꼴이다. 오른쪽 각도 수치 `ctxRight_`의 글꼴·형식(`'f',1`)은 그대로이고, 값만 `previewAngle()`로 바뀌었다(`MainWindow.cpp:571-577`). | 확인 |
| R5 도판 | pass | 그리는 순서 코드(`Plate::paint`)는 바뀌지 않았다. Plate.cpp diff는 거부 함수 추가뿐이다(`Plate.cpp:41-49`, `:495`, `:516`, `:538`, `:742`). 0° 캡처가 JOB-004와 비트 단위로 같다. 외면(음영)이 왼쪽, 단면(빨강)이 오른쪽이고 D5와 맞다. 미리보기는 단면 버퍼만 바꾸고(`ArtifactDoc.h` `shown()`) 그리는 순서는 그대로다. | 확인 |
| R6 내보내기 | pass | `verify_job004`와 `verify_job005` 결과물 sha256을 비교했다. `studio_plate.svg/.dxf/.png`, `studio_side_sample_Left.svg`, `studio_side_blind_Right.dxf`, `studio_blind.svg`가 같다. `studio_plate.pdf`는 XMP 날짜·UUID·`/ID` 140바이트만 다르고 크기(462733 B)는 같다. 미리보기 중 내보내기는 파일을 만들기 전에 거부한다(`ctest_studio_cut_V.log` `[8] … ok=0 file=0`). | 확인 |
| R7 홈 | pass | `shot_home.png` sha256 `00fe52ae…`가 JOB-004와 같다. | 확인 |
| R8 문구 | pass(경미한 편차) | 새 문자열 2개는 모두 한국어이고 약어가 없다. ① `막 1° · Shift 15°`(`MainWindow.cpp:285`, 앞에 kbd 배지 `← →`). ② `막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.`(`Plate.cpp:47`) — 다음 할 일을 말해 준다. 편차: `Shift`가 kbd 배지가 아니라 평문이다. 같은 줄의 ① 바닥 정하기 ctx는 `kbdLabel("Shift")`로 쓴다(`:292`). 다만 명세 기준 20이 이 문자열 그대로를 요구하고, 디자인 v2(`design-excerpt.md:13,15`)도 `← → 1° · Shift 15°`를 평문으로 쓴다. | 확인 |
| 언어·사용성 | pass(제안 있음) | §2·§4 참고. 한국어 100 %다. 용어 `막`이 현재 화면에서 처음 나오고, 바로 옆 `[ ] 절단면 1° 이동`과 같은 동작을 다른 말로 부른다. | 확인 |
| 캡처(기준 23) | pass | 세 장 모두 5120 B를 넘는다(13246 / 333540 / 70269). `shot_main.png`는 JOB-004와 같은 배치이고 치수 띠도 같은 이미지다(해시 같음). | 확인 |

## 2. 끄는 동안 바뀌는 것과 그대로인 것 (코드 근거)

끄는 동안에는 `refreshPreview()`(`MainWindow.cpp:721-726`)만 돈다. 이 함수는 `sheet_->refresh`, `canvas()->update`, `view3d_->refresh`, `updateCutInfo`를 부르고 `docChanged`·`refreshAll`·`refreshSlots`는 부르지 않는다(기준 18 grep 0/2).

| 바뀜 (실시간, LOD) | 그대로 (놓은 뒤 갱신) |
|---|---|
| 도판 단면선(빨강)·실선·추정 내벽 파선·빗금(`shown()` → 미리보기 버퍼) | 외면 음영: 캐시 `shade_`는 `docChanged()`(`MainWindow.cpp:538`)에서만 다시 그린다 |
| 3D 보조창 메시와 빨간 절단선(`lodMesh()`가 미리보기 LOD를 줌, `ArtifactDoc.h:134`) | 외곽선(`displayOutline`)·치수 띠 5개 값·두께 풍선·커서 두께 |
| ctx 오른쪽 `절단 각도 X°`·`점 N → M개`(`MainWindow.cpp:571-577`) | 슬롯 목록 문구, 상태줄 안내, 3D 머리의 `메시 N면`·`중심축 오차` |
| 도판 머리 부제 `· 내벽 미스캔 파선 보정(e mm)` 유무(`SheetView.cpp:251`이 `hasEstimatedInner()`를 따름, `ArtifactDoc.h:125`) | 3D 화면 맞춤(원본 `mesh()` bbox 기준, `View3D.cpp:62,78`). 비원형 유물은 놓을 때 배율이 조금 바뀔 수 있다(추정) |

판단: 각도 숫자·도판 단면·3D 절단선이 같이 움직이므로 지금 어디를 자르는지는 헷갈리지 않는다. 다만 화면 어디에도 "음영·외곽·치수는 놓은 뒤 바뀝니다"라는 표시가 없다. 문양이 있는 토기에서는 단면만 돌고 왼쪽 무늬는 멈춰 있다가 놓는 순간 바뀌어 보일 것이다(추정). 명세가 이 표시를 범위 밖으로 둔다(§2 "앞쪽 반 흐리게", 도판 머리 `A–A′ 실시간 단면 — 12°`, 하단 "끄는 동안 3.5만 면 미리보기 · 놓으면 원본으로 다시 계산"은 JOB-006). 사람 검토 5에 올라 있으므로 FAIL 사유로 삼지 않는다.

## 3. builder가 명세에서 벗어난 3곳 (decision.md) — UI 관점

| # | 내용 | UI 판단 |
|---|---|---|
| 1 | `release`가 키 누름 상태도 푼다(`CutGesture.h` `release`) | 좋다. 키를 누른 채 Ctrl+P·슬롯 저장을 해도 먼저 확정되므로 거부 메시지를 볼 일이 없다. |
| 2 | `openPath` 실패 시 미리보기를 확정한다(`MainWindow.cpp:456-466`) | 좋다. 이것이 없으면 `cancelCut()` 뒤 문서가 미리보기 상태로 남아 영영 확정되지 않는다. 확정한 다음 경고 대화상자를 띄우는 순서도 자연스럽다. |
| 3 | 수식 키만 누르면 확정하지 않는다(`MainWindow.cpp:909`, `isModifierKey`) | 필요하다. 끄는 도중이나 → 를 누르는 도중에 Shift를 눌러도 끊기지 않는다. |

## 4. 확인과 추정

- 확인(직접 실행·읽기):
  - 기준 17–22 grep 재실행 결과: 0/1, 0/2, 1/1/1, 4/1/1/1/1, 출력 없음, 0. 변경 파일 10개가 §6 표 안에 있고 GigaMesh·docs·CLAUDE.md 변경은 0이다.
  - 캡처 해시 3장이 JOB-004와 같다.
  - 내보내기 결과물 해시가 같다(PDF는 메타데이터만 다름).
  - `03_ctest.log:33`에 `100% tests passed out of 15`, `02_build.log` warning 0건.
  - `ctest_studio_cut_V.log` 값: 미리보기 중앙값 0.70 ms, 3D 한 장 14.94 ms, 원본 확정 1364.8 ms(예제 77,120면)·930.6 ms(미스캔 예제), lod-vs-full 미스캔 예제 θ=45° hausdorff 7.053 mm.
- 추정(실행 화면 캡처 없음):
  - ctx 줄 폭은 약 1847 px다. 125 % 배율 노트북(논리 1536 px)이나 1366 px 화면에서 단면 도구를 들면 창이 화면 밖으로 넓어지거나 오른쪽 `절단 각도` 표시가 잘릴 수 있다. 기존에도 약 1712 px로 넘쳤고, 이번에 약 135 px 늘었다.
  - 놓은 뒤 UI가 약 1.4 s 멈추는 동안 대기 커서가 없다(`onCutTimer`의 Commit 경로, `MainWindow.cpp:708-719`). 키는 뗀 뒤 300 ms를 기다렸다가 멈추므로 "왜 멈췄지"로 느낄 수 있다.
  - Shift+→를 누르고 있으면 자동 반복(약 33 ms)마다 15°씩 돌아 약 450°/s가 된다. 원하는 각에서 멈추기 어렵다.
  - → 를 누른 채 창 포커스를 잃으면 `keyUp`이 오지 않아 미리보기 상태가 남는다. 다음 동작에서 확정되기는 한다.

## 5. 고치지 않아도 되는 제안 (JOB-006 때 같이)

1. **용어 통일**: 지금 화면에서 같은 각도를 네 가지로 부른다 — 리본 `정면 회전 ±1°`(`MainWindow.cpp:165`)·`절단 위치 이동 (1°)`(`:173`), ctx `절단면 1° 이동`(`:284`), 이번 `막 1°`(`:285`)·`막을 움직이는 중`(`Plate.cpp:47`). ② 페이지가 들어올 때 `막` 하나로 맞추고, ctx 키 안내를 `[ ] ← →  막 1° · Shift ← → 15°`처럼 한 쌍으로 합치기를 권한다. 폭 문제도 함께 준다.
2. **R8**: `Shift`도 kbd 배지로 쓰기를 권한다(① 바닥 정하기 ctx `:292`와 같은 꼴). 명세 기준 20의 grep 문자열과 함께 바꿔야 한다.
3. **끄는 중 표시**: JOB-006의 도판 머리 `A–A′ 실시간 단면 — 12°`와 하단 "놓으면 원본으로 다시 계산" 문구로, 음영·치수가 아직 옛 각이라는 것을 알려 주기를 권한다.
4. **놓은 뒤 멈춤**: 원본 계산 동안 `QApplication::setOverrideCursor(Qt::WaitCursor)`를 쓰기를 권한다. `openPath`가 이미 같은 방식을 쓴다.
5. **포커스를 잃을 때**: `WindowDeactivate`·`FocusOut`에서 `cut_.keyUp` 또는 `flushCut()`을 부르기를 권한다.
6. **ctx 줄 폭**: 목업은 키 안내 3쌍인데 코드는 7쌍이다. ② 페이지 ctx로 옮길 때 줄이기를 권한다.

## 6. 사람 검토로 넘김

- 사람 검토 5·9: 끄는 동안 단면만 바뀌고 음영·외곽·치수는 그대로인 화면을 손으로 확인한다.
- 화면 폭: 1366 px나 125 % 배율 환경에서 `S`로 단면 도구를 들었을 때 창 폭과 ctx 오른쪽 각도 표시를 손으로 확인한다.
- 놓을 때 멈춤과 튐: 멈춤 약 1.4 s(77k 면), 미스캔 예제 θ=45° LOD↔원본 7.05 mm 차이를 받아들일지(사람 검토 3·6).
- 정면각 부호 반전: `.`·`]`·리본 `정면 회전 ±1°`이 JOB-004와 반대 방향으로 돈다(사람 검토 1). 리본 단추는 이름이 `±1°`인데 늘 +1°만 한다(기존 성질).
- Shift+→ 길게 누르기의 회전 속도(약 450°/s 추정)가 괜찮은지.
