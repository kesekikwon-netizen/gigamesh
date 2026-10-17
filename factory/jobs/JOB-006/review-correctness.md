결론: PASS

# JOB-006 정확성 검토 (correctness-reviewer)

- 대상: `D:\_dongarch_build\wt\.worktrees\JOB-006`, `git diff 804f60d..HEAD`(HEAD `688b7bd`, 커밋 2개, 14개 파일). 작업 트리는 깨끗하다(`git status --porcelain` 출력 없음).
- 입력: 명세 `factory/jobs/JOB-006/spec.md`, diff 원문, 이미 빌드된 `D:\_dongarch_build\verify_job006\studio\test_studio_cutpage.exe`(00:40:05 빌드 = HEAD `688b7bd`). 제품 코드·시험은 고치지 않았고 빌드도 하지 않았다.
- 표기: **확인** = 코드를 읽거나 실행해서 직접 본 것. **추정** = 코드에서 추론했고 실행으로 재현하지는 않은 것.

## 0. 직접 실행
```
(PowerShell) $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'
Set-Location D:\_dongarch_build\verify_job006\studio; .\test_studio_cutpage.exe D:\_dongarch_build\verify_job006\studio
→ [9] ribbon height=107 px (JOB-005 계산값 107)
  [9] ctxHint width=372 / sizeHint=612 px · ctx minimumSizeHint width=1420 px (offscreen 글꼴)
  [9] 단면 위치 x=919 / [11] 단면 위치 x=919 / [13] 단면 위치 x=919
  PASS   (exit=0, 15.4 s)
```
- 엔진은 바뀌지 않았다(확인). `git diff --name-only 804f60d HEAD -- ArtifactDoc.* CutGesture.h Plate.* MeshIO.* MeshRaster.* studio/tests/` 에는 새 파일 `test_cutpage.cpp` 만 나온다.
- 계산값도 그대로다(확인). `LastTest_verify.log` 에서 `H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180` 가 1건 나온다.

## 1. 각도 표시(−180~180, 0.1°)가 실제 previewAngle/frontAngle 과 같은가 — 맞음(확인)
- **표시의 출처가 하나다.** 각도 칸 `CutAngle`, `막 방향` 그룹 이름, `axX`/`axY` 켜짐, ctx 오른쪽은 모두 `MainWindow::updateCutInfo`(MainWindow.cpp:623-638)에서 `a = doc_.previewAngle()` 하나로 만든다. 도판 머리도 같은 `doc.previewAngle()` 을 쓴다(SheetView.cpp:248-249). `previewAngle()` 은 미리보기 중이면 `previewAngle_`, 아니면 `frontAngle_` 을 돌려준다(ArtifactDoc.h:104). 두 값 모두 `std::remainder(·,360)` 로 정규화되어 −180~180 범위다(ArtifactDoc.cpp:610·621).
- **각도가 바뀌는 모든 경로가 표시를 갱신한다.**
  - 미리보기: `refreshPreview` → `updateCutInfo`, `sheet_->refresh`
  - 확정·X/Y 맞춤·되돌리기·슬롯 각도 복원: `refreshAll(true)`
  - 바닥 확정 전 분기(MainWindow.cpp:595-608)는 `updateCutInfo` 를 부르지 않는다. 그러나 이때 `setFrontAngle` 은 거부되므로(ArtifactDoc.cpp:608) 각도가 바뀌지 않는다. ② 페이지도 잠겨서 그 사이 오래된 글이 보이지 않는다. 잠긴 ② 를 누르면 같은 호출 안에서 ① 로 다시 옮기므로 ② 가 그려지지 않는다.
- **화면의 모든 각도 문자열이 `angleText` 를 거친다.** `°` 를 Grep 하면 슬롯 메모 `막 ` + `angleText(frontAngle())`(MainWindow.cpp:823)뿐이고, 옛 `'f', 1` 표기는 남아 있지 않다.
- **반올림이 맞다.** `angleText`(CutPageText.h:40)는 `llround(deg*10)` 를 쓰고, 음수는 U+2212 로 쓴다. −0.04 → `0°`. 앱에서 생기는 각은 모두 0.5° 의 배수다(끌기 0.5°/px, 키 1·15, 맞춤 0·90). 그래서 2진 표현 오차로 반올림이 갈리는 경우(예 12.05)는 생기지 않는다. 경계값 180/−180 은 `remainder` 가 짝수 쪽으로 정해 두 표기가 모두 나올 수 있는데, 둘 다 맞는 값이다.

## 2. X/Y 맞춤 = alignCut 절대각, 진행 중 제스처를 먼저 처리 — 맞음(확인)
- **절대각이 맞다.** `axX` → `CutAxis::X` → `alignCut` → `setFrontAngle(90)`, `axY` → `setFrontAngle(0)` 이다(MainWindow.cpp:917·923, ArtifactDoc.cpp:614-616). 툴팁 「회전판을 90° 로」, 「0° 로」와 같다. 정면각 + = 위에서 보아 반시계이므로 +90° 에서는 정면에서 보아 왼쪽 옆이 보는 사람 쪽으로 온다. JOB-005 의 규약과 같다.
- **진행 중 제스처를 취소한다.** `cutInput` 목록 때문에 `axX`/`axY` 앞에서는 `flushCut` 이 돌지 않는다(:853-854). `pending = cut_.active()` 이면 `cancelCut()`(제스처 취소·타이머 정지) 뒤에 `alignCut` 하고, `recompute → clearPreview` 로 미리보기도 지운다(:918-925). 되돌리기 한 칸은 `beginCutInput` 이 이미 쌓았으므로 `pushUndo` 를 생략한다. 원본 계산은 한 번이고 대기 커서가 뜬다.
- **이미 그 각이면 계산하지 않는다.** 판정식은 `fabs(remainder(goal − frontAngle, 360)) ≤ 1e-9` 이다. 각은 0.5° 배수라 비교가 정확하다.
- **시험이 이 동작을 실제로 가른다.** 시험 [10]은 `cutPlus` 를 누른 직후 `axX` 를 눌러 90° 를 확인한다. 이어서 500 ms 를 기다린 뒤에도 90° 인지 확인하므로, 취소하지 않았다면 16 ms 미리보기가 1° 를 띄워 FAIL 한다. `Ctrl+Z` 뒤 0° 도 확인하므로, 먼저 확정하고 `pushUndo` 하는 구현이라면 1° 가 되어 FAIL 한다.

## 3. 모든 각도 입력이 CutGesture 를 거치는가 — 맞음(확인)
`setFrontAngle|previewFrontAngle|alignCut|commitPreview|cut_.*` 의 호출 지점 전체(MainWindow.cpp 13곳)를 확인했다.
- `[−][+]` → `cut_.click`(한 곳, :913), 키 → `cut_.keyDown/keyUp`, 3D 왼쪽 끌기 → `cut_.drag/release`.
- `previewFrontAngle` 은 `onCutTimer` 한 곳(:789)에서만 부른다.
- 직접 `setFrontAngle` 을 부르는 곳은 두 가지이고 둘 다 기존 코드다. 하나는 슬롯 각도 복원(`selectSlot`)이고, 다른 하나는 `restore`(되돌리기)다. 둘 다 먼저 `flushCut` 을 한다.
- 이번에 새로 생긴 직접 경로는 절대각 `alignCut` 하나다. 명세 §3.7 에 따라 제스처를 취소한 뒤에 부른다.
- `onPage` 가 가로 슬롯을 첫 종단면으로 옮길 때 `slot_` 만 바꾸고 각도는 바꾸지 않는다(:731-737). 명세와 같다.

## 4. ② 탭 잠김 · 확정 뒤 ② · 탭 이동 때 도구 상태 — 맞음(확인)
- **잠김.** `refreshAll` 의 확정 전 분기 앞에서 `setPageLocked(pCut_, base, kLockTip)` 를 부른다(:591). QSS `[locked="true"]` 는 `[on="true"]` 뒤에 있다(Theme.h:91).
- **잠긴 ② 를 누를 때.** `onPage(pCut)` → `requirePose` 가 먼저 `setCurrentPage(pBase_)` 하고(중첩 `onPage(pBase)` 는 확정 전이라 화면을 갱신하지 않는다) 그다음 안내를 쓴다(:701-706). 그래서 안내가 남는다. 시험 [8]이 확인한다.
- **확정.** `confirmPose` 가 `setCurrentPage(pCut_)` 를 부르고(:718), `onPage` 가 도구를 든다. 그 뒤 `refreshAll(true)` 가 돈다. 원본 계산은 한 번이다.
- **② 를 떠날 때.**
  - 다른 탭·`Esc`·`S` 는 `onPage` 에서 `flushCut` 뒤 `hand_=="cut"` 이면 도구를 내려놓는다.
  - `O` 는 홈으로 간 뒤 외곽선 도구를 든다(:928-930).
  - 파일 열기·예제 열기는 `setCurrentPage` 를 거쳐 도구를 내려놓는다.
  - 결과적으로 바닥을 확정한 뒤에는 `hand_=="cut"` 과 「현재 페이지가 ②」가 늘 함께 참이다. 코드 경로를 전부 따라가서 확인했다.
- **ctx 의 겹치는 묶음.** `← →`·`R` 묶음 인덱스는 `ctxCutKeys_[0,1,8,9]`(:337)로 맞다. 보임은 `refreshAll` 이 현재 페이지로 다시 계산하고, 홈에서 외곽선 도구를 들면 다시 보인다(시험 [15]).
- **떠나는 순간 잠깐 다른 글이 들어간다.** 대기 중이던 제스처를 `onPage` 안의 `flushCut` 이 확정하면, `refreshAll(true)` 는 새 페이지 번호와 아직 바뀌기 전의 `cutPage_` 깃발로 한 번 돈다. 이어서 `setCutPage` → `refreshAll(false)` 가 다시 써서 최종 상태는 맞다(확인). 화면에 그려지기 전이라 사용자는 보지 못한다.

## 5. 오른쪽 단추 끌기 = 보는 방향만 — 맞음, 단 두 단추를 함께 누르는 경우에 작은 결함(권고)
- `cutDrag_ = cutTool_ && button == LeftButton`(View3D.cpp:183)이다. 오른쪽·가운데 단추는 yaw·pitch 만 바꾸고, 놓아도 `cutDragReleased` 를 내보내지 않는다(:203).
- 시험 [13]이 이를 확인한다. 오른쪽으로 끌고 놓은 뒤에도 ctx 오른쪽이 `막 105° · 끄는 동안`(확정 상태)이고 도판 머리가 105° 이며, 3D 그림은 달라져야 한다. 오른쪽 끌기가 막을 돌린다면 125° 가 되거나 「끄는 중」 문형이 되어 FAIL 한다.
- **권고 R1 (추정 — 코드 근거, 실행 재현은 안 함).** 왼쪽을 누른 채 끌다가 오른쪽 단추를 함께 누르면, Qt 가 둘째 누름도 `mousePressEvent` 로 보내므로 `cutDrag_` 가 false 로 덮인다. 그다음 어느 단추를 떼든 `cutDragReleased` 가 나가지 않는다. `CutGesture::drag` 는 확정 마감이 없으므로(`commitAt_=-1`, CutGesture.h:16-19) 막이 「끄는 중」 미리보기에 머문다. 이 상태는 다음 동작(탭 이동·내보내기·되돌리기·슬롯 저장 등은 모두 `flushCut`)에서 풀리므로 잘못된 값이 내보내지지는 않는다. JOB-005 에서는 `drag_ && cutTool_` 이었으므로 이번 JOB 에서 생긴 회귀다.
  - 고치는 방법: `mousePressEvent` 에서 `if (!drag_)` 일 때만 `cutDrag_` 를 정한다. 또는 놓을 때 `e->buttons()==Qt::NoButton` 이고 이번 누름 동안 막을 돌린 적이 있으면 `cutDragReleased` 를 내보낸다.
  - 차단 사유로 보지 않은 이유: 드문 조작이고, 화면 문구(「끄는 중 … 놓은 뒤」)가 실제 문서 상태와 일치하며, 산출물에 영향이 없다.
- 참고(확인): `cutDrag_` 는 누르는 순간 고정된다. 그래서 왼쪽으로 끄는 중에 `Esc`·`S` 로 ② 를 떠나도, 그 끌기는 홈에서 계속 막을 돌리고 놓을 때 확정된다. 이전 동작(도구를 내려놓으면 보는 방향 돌리기로 바뀜)과 다르지만 값이 틀리지는 않는다.

## 6. 도판 머리 각도·ctx 문구 — 미리보기 중·놓은 뒤 모두 맞음(확인)
- **도판 머리.** `live = cutPage_ && Vertical` 이면 `plateTitle(s.name.at(0), doc.previewAngle())` 와 `plateSub(sectionSide)` 를 쓰고, 미스캔 꼬리는 그대로 붙인다(SheetView.cpp:248-255). `refreshPreview` 가 `sheet_->refresh` 를 부르므로 끄는 동안에도 각을 따라간다.
- **ctx 오른쪽.** `ctxRight(a, lod?lod:full, full, isPreviewing())` 이다. 면 수는 실제 `lodMesh()` 와 `mesh()` 에서 읽는다. 미리보기 중에는 `previewLod_` 이고 크기는 `lodMesh_` 와 같다.
- **시험이 둘 다 확인한다.** 시험 [11]은 왼쪽 단추를 누른 채로 `막 92° · 끄는 중 … — 음영·외곽·치수·두께는 놓은 뒤` 와 도판 머리 92° 를 확인하고, 500 ms 뒤에도 그대로인지(확정 없음) 본다. 놓은 뒤에는 `막 92° · 끄는 동안 …` 을 확인한다. `updateCutInfo` 나 도판 머리가 `frontAngle` 을 쓰도록 바뀌면 끄는 중에 90° 가 나와 FAIL 한다.
- **D5 문형.** Right = `외면 왼쪽 · 단면 오른쪽`, Left = `단면 왼쪽 · 외면 오른쪽` 이다(CutPageText.h). 시험 [14]가 `R` 반전 두 번으로 확인한다.

## 7. 표기 함수의 경계값(확인)
- `atAxis`(CutPageText.h:55)는 세 번 `fmod` 해 −180·−0.5·179.5 를 올바르게 접는다. 시험 [3]이 표의 모든 값을 확인한다.
- `faceText`: 10000 → `1만 면`, 9999 → `9,999 면`, 99950 → `10만 면` 이다(소수가 없으면 정수로 쓴다).
- 이번 JOB 의 단위·축척(mm, SVG/DXF)은 바뀌지 않았다. 엔진 파일이 무변경이고 LastTest 세 줄이 같다.

## 8. 시험의 힘·수용 기준과 1:1
- 기준 1-18 은 `test_cutpage.cpp` 의 CHECK 와 1:1로 대응한다. 판정식(==·startsWith·endsWith·sizeHint·width·x)이 명세 §4 와 같다(확인).
- 머릿속 변이 시험(확인 — 코드 경로 추적. 재빌드 금지라 실제 변이 빌드는 하지 않음):

| 변이 | 무엇이 잡는가 |
|---|---|
| X·Y 맞춤의 축을 서로 바꿈 | [10] |
| 진행 중 제스처를 취소하지 않음 | [10] settle |
| 먼저 확정한 뒤 `pushUndo` | [10] Ctrl+Z |
| [+] 를 직접 `setFrontAngle` | [11] 「이벤트 처리 전 90°」·Ctrl+Z 한 칸 |
| 표시가 `frontAngle` 을 씀 | [11] 끄는 중 |
| 오른쪽 끌기가 막을 돌림 | [13] |
| 잠김이 없음 | [8] |
| 확정 뒤 홈으로 감 | [9] |
| 페이지를 떠나도 도구를 내려놓지 않음 | [15] |

- 빈틈 1: 진행 중에 X/Y 를 누를 때 「취소」 대신 「확정만 하고 `pushUndo` 하지 않는」 억지 변이는 [10]을 통과한다. 원본 계산이 두 번 도는 성능 차이일 뿐이라 정확성 영향은 없다.
- 빈틈 2: 위 5-R1(두 단추 동시 누름)은 시험하지 않는다.

## 9. 발견 사항 요약
| 등급 | 항목 | 위치 | 구분 |
|---|---|---|---|
| 권고 | 왼쪽+오른쪽 동시 누름 뒤 놓으면 `cutDragReleased` 가 나가지 않아 막이 「끄는 중」 미리보기에 머문다(다음 동작에서 확정) — JOB-005 대비 회귀 | View3D.cpp:183·203 | 추정(코드) |
| 참고 | 끄는 중 `Esc`·`S` 로 ② 를 떠나도 같은 끌기가 홈에서 막을 계속 돌린다(`cutDrag_` 고정) | View3D.cpp:183·192 | 확인(코드) |
| 참고 | 진행 중 제스처를 취소할 때, 목표각이 제스처 시작각과 같으면 되돌리기 칸이 하나 쌓이지만 되돌려도 각이 그대로다 | MainWindow.cpp:918-921 | 확인(코드) |

차단 사유: 없음.

## 10. 사람 검토
1. `atAxis` 는 원문(1460)대로 `mod 180` 이다. 그래서 180° 에서 `Y 막`, −90° 에서 `X 막` 이 켜지고 그룹 이름에서 각도가 빠진다. 그런데 D5 단면(x≥0 반쪽)으로 보면 180° 와 0°, −90° 와 90° 는 서로 반대쪽 반쪽이다. 켜진 타일을 누르면 원본을 다시 계산하고(≈1.4 s) 단면이 반대쪽 반쪽으로 바뀐다. 원문 해석이 맞는지 확인해 주세요.
2. ② 에서 `N`(단면 추가) 또는 슬롯 목록으로 가로 슬롯을 고르면, 도판 머리는 `B–B′ 횡단면` 이 되고 ctx·상태줄은 ② 문구(⑥ 단면 보기)로 남는다. `onPage` 의 「종단면으로」 규칙은 ② 에 들어갈 때만 적용된다. 이대로 둘지 결정해 주세요.
3. 각도 키(`← → , . [ ]`)는 ② 밖(홈·측정·확정 뒤 ①)에서도 막을 돌린다. JOB-005 동작을 유지한 것인데, 「② 페이지 = 도구」 규칙과 맞는지 확인해 주세요.
4. 위 9의 권고(두 단추 동시 누름) — 고칠지 결정해 주세요(작은 수정 1-2줄).
5. 왼쪽 끌기 중에 `Esc`·`S` 로 ② 를 떠나도 같은 끌기가 홈에서 막을 계속 돌리는 동작(9의 「참고」 첫째 줄)을 받아들일지 확인해 주세요.
