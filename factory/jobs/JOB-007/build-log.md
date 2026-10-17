# JOB-007 구현 기록 (builder)

작업 폴더 `D:\_dongarch_build\wt\.worktrees\JOB-007`(factory/JOB-007, 기준 688b7bd), 빌드 폴더 `D:\_dongarch_build\build_job007\studio`
(오케스트레이터 지시 경로 — 명세 §5 의 `verify_job007` 대신). 2026-10-18.

## 요약
- **나누지 않음**(명세 §0): 제품 코드(studio/src) 빈 줄·주석 제외 **+277 / −77**(전체 +323 / −84) ≤ 300.
  007a(기준선·방위각) +134 를 먼저 커밋(6542878)하고 007b(② 고정·넘기기·③ 도면 편집) +146 을 이어 커밋(89846ba).
- 시험·빌드: test_reflines.cpp 신규 453줄, test_cutpage.cpp +191, CMakeLists.txt +4, UI_RULES.md +16/−7.
- verify.ps1 -Clean: 빌드 경고 0, **ctest 100% tests passed out of 17**, shot_home 해시 JOB-006 과 같음, 파이썬 PY-SVG OK · PY-DXF OK.

## 바꾼 파일
| 파일 | 내용 | 이유 |
|---|---|---|
| studio/src/Plate.h | `PlateOptions::paperPxPerMm`, `RefLine`·`PlateRefLines`·`plateRefLines()`·`azimuthNote()` 선언 | §3.3-1 |
| studio/src/Plate.cpp | 익명 이름공간: 정수 0.1° 방위각 글(`azimuthText`·`azimuthDxf`), 닫는 가로선 고르기 `closingEnds`(y 작은 끝 = 바닥선·큰 끝 = 구연선이 있으면 뺌), 기준선 출력 규약 표 `kRefStyles`(층 id·이름·DXF 층·종이 mm·화면 px). `plateRefLines`·`azimuthNote`. 화면·PDF·PNG: 방안 먹 바닥선·외곽선 단계 중심축 지움, 「2.5) 기준선」, 닫는 선 규칙, 「기고 H」 파선 끝 −구경/2−2·라벨 구연선 위. `paintSheet` 종이 굵기·표제란 방위각. SVG: 방안 먹 바닥선·`6 1 1 1` 중심선 지움, `refline_center/rim/base` 층, 외곽선 층 이름 `외곽선`, 표제란 방위각. DXF: 01·07·08 = ACI 7·CONTINUOUS, 기준선 LINE 3개, 02 닫는 LINE 규칙, `FRONT AZIMUTH … DEG (CCW, TOP VIEW)`. `CutPageText.h` include 없음 | §3.3-1~6 |
| studio/src/ExportDialog.cpp | `외곽선·기준선 Layer (먹색 · 외곽선 0.50 · 기준선 0.10~0.25 mm)`, SVG·DXF 안내 글 | §3.3-7 |
| studio/src/SheetView.cpp | 「그리는 순서」 글에 `기준선(먹) →`. **최소 폭 `setMinimumWidth(0)` → `1`**(아래 「명세와 다르게 판단한 곳」 1) | §3.3-7 + 판단 |
| studio/src/CutPageText.h | 머리 주석 ②·③, `<optional>`, `kStatus` 원문 복원, `kGrpFix`·`kTileFix`·`kTileEdit`·`kEditTab`·`kEditBadge`·`kEditHint`·`kEditStatus`·`k3dEditTitle`·`kGrpFixed`·`kBackToCut`, `fixText()`·`slotMemo()` | §3.2 |
| studio/src/Ribbon.cpp | 타일 아이콘 `lock`(둥근 몸통 + 고리)·`next`(→) | §3.4 |
| studio/src/MainWindow.h | `pEdit_`·`fixedAngle_`·`ctxFixKeys_`·`fixedLbl_`·`stFix_`, `fixAngle()`·`angleFixed()`·`firstVerticalSlot()` | §3.5 |
| studio/src/MainWindow.cpp | 측정·탁본 탭 → ③ 도면 편집(파일 · 고정된 막 · 선 다듬기 · 도판 표시 · 내보내기), ② 「고정 · 넘기기」(fixAngle·toEdit Primary), ctx `Enter 지금 각도 고정`, 상태줄 `StFix`, 고정 풀림 지점(열기 3곳·확정·중심축 맞춤·X/Y 막·되돌리기·beginCutInput), ③ 자동 고정(refreshAll 처음), ③ 잠김, 슬롯 목록 살아 있는 「고정」 꼬리, onPage 다시 쓰기(③ 3D 330 px, 첫 종단면 `firstVerticalSlot`), setHandTool `edit`, onAction `fixAngle`·`toEdit`·`backToCut`, Enter(②)·③ 에서 ← → , . [ ] 무시·Esc(③ → 홈), saveSlotAngle 메모 = `slotMemo` | §3.4~3.8 |
| studio/src/View3D.h/.cpp | `setEditPage()`·`editPage_`, ③ 머리 `<b>3D — 고정된 막</b>` | §3.7 |
| studio/src/main.cpp | `--edit`(300 ms 에 toEdit 단추 click), 사용법 주석 | §3.9 |
| studio/tests/test_reflines.cpp | 신규 — 수용 기준 1-9, 꼬리표 `[J7-n]`, 파이썬 판정용 SVG 5·DXF 4 파일을 빌드 폴더에 남김 | §3.11 |
| studio/tests/test_cutpage.cpp | JOB-006 [8] 탭 글자 `③ 도면 편집`·[15] `tb[5]`(내보내기) 갱신, `[J7-12]`-`[J7-19]` 추가(`groupNames`·`primaryTiles`·`slotHas` 도움 함수) | §3.11 |
| studio/CMakeLists.txt | `test_studio_reflines` / `studio_reflines` 등록 (CTest 16 → 17) | §3.11 |
| docs/ui/UI_RULES.md | R1 표 1·3·4행, R3 주 단추 목록, R5 그리는 순서·기준선 줄, R6 SVG 층·DXF 기준선 층·정면 방위각 | §3.10 |

## 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| `verify.ps1 -Root …\JOB-007 -BuildRoot D:\_dongarch_build\build_job007 -NoShots` (007a 첫 빌드) | 8 | 빌드 경고 0, 16/17 — `studio_cutpage` JOB-006 `[11]`·`[13]` 「리본이 옆으로 밀림 919 → 945」. 원인: 길어진 「그리는 순서」 글(오프스크린 □ 글꼴)이 창 최소 폭을 1791 → 1874 로 밀어 시험 중 창이 커짐(디버그 인쇄로 확인 후 되돌림) |
| `verify.ps1 …` (그리는 순서 최소 폭 1 시험판) | 0 | 17/17, shot_home 해시 같음, shot_main 3D 보조창 269 → 451 px |
| `verify.ps1 …` (그리는 순서 최소 폭 = 옛 글 폭, 007a 커밋본) | 0 | 17/17, shot_home 해시 같음, shot_main 배치 JOB-006 과 같음 → **6542878** |
| `bt.ps1`(vcvars + cmake --build + `ctest -R "studio_reflines\|studio_side\|studio_pipeline" -V`, RefStyle 정리 뒤) | 0 | 3/3 |
| `bt.ps1 -Regex studio_cutpage` (007b) | 0 | `[J7-13]`·`[J7-15]` window minimumSizeHint width=1791, JOB-006 단면 위치 x=919 그대로 |
| `verify.ps1 -Root …\JOB-007 -BuildRoot D:\_dongarch_build\build_job007 -Clean` (최종) | 0 | configure 0 · build 0(23 s) · ctest 0(130 s) · shots 0. `grep -ci warning 02_build.log` = 0, `03_ctest.log` 「100% tests passed out of 17」, `verify-summary.md` 마지막 줄 `결과: PASS` |
| LastTest.log 기준 22 세 줄 grep | 각 1 | `H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180` · `STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2` · `ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95` |
| 파이썬 기준 10 (`py_svg.py` = 명세 §5-3 그대로, B = build_job007\studio) | 0 | `PY-SVG OK` |
| 파이썬 기준 11 (`py_dxf.py`) | 0 | `PY-DXF OK` |
| `(Get-FileHash shot_home.png).Hash -eq (JOB-006 artifacts shot_home.png)` | — | `True` (기준 31) |
| 실제 글꼴 캡처(§5-4, `QT_QPA_FONTDIR=C:\Windows\Fonts`, 출력은 빌드 폴더 `shots\`) | 0 | shot_cut_font 434 828 B · shot_edit_font 407 040 B · shot_export_font 116 051 B, 오프스크린 shot_edit 340 437 B, shot_main 368 572 B (모두 1845×920) |
| 기준 24-30 Git Bash 점검(커밋 뒤 깨끗한 트리) | — | 24 출력 없음 · 25 MISSING 없음 · 26 출력 없음, `kEditStatus` 1 · 27 hex 19개·새 값 없음·Primary 추가는 toEdit 뿐·`"toEdit", ToolState::Primary` 1 · 28 4/3/0/1/2/4/1/1 · 29 1/0/1/1/1/1/1/1/1 · 30 출력 없음 |

시험 인쇄(사람 검토 6 자료):
```
[J7-1] sample H=184.8206 rim=142.00 base=142.00 outline ends=(-71.700,3.362)…(-71.697,181.612) gap low 3.362 high 3.208 mm
[J7-3] blind H=190.2556 rim=124.00 base=124.00 rim.a.x=-62.000 base line=1
[J7-13] window minimumSizeHint width=1791
[J7-15] window minimumSizeHint width=1791
[9] ctxHint width=174 / sizeHint=612 px · ctx minimumSizeHint width=1618 px (offscreen 글꼴)   ← JOB-006 372 / 1420 (② ctx 에 Enter 지금 각도 고정)
```

## 명세와 다르게 판단한 곳
1. **치수 띠 「그리는 순서」 글 최소 폭(SheetView.cpp)** — 명세 §3.3-7 은 글자만 바꾸라고 했지만, 오프스크린 □ 글꼴에서 새 글 폭이
   치수 띠·도판 최소 폭을 밀어 (a) 창 최소 폭 1874 px(> 1845, 기준 13·15·31) → JOB-006 `[11]`·`[13]` FAIL, (b) ③ 3D 330 px 이면 1906 px,
   (c) ② 3D 보조창이 최소 240 px 로 눌려 기준 17(② 3D > 330) 이 설 수 없었다. 기존 `order_->setMinimumWidth(0)` 은 Qt 에서
   「정하지 않음」이라 의도(글이 창 폭을 밀지 않음)가 동작하지 않던 것 → `1` 로(ctxHint 와 같은 방법). **대가: 오프스크린 shot_main 의
   3D 보조창이 JOB-006 의 269 px → 451 px(R1 「≈27 %」, 실제 글꼴 캡처와 같은 배치)로 넓어져 도판도 오른쪽으로 옮겨진다 — 기준 32 의
   「작업 공간·3D 같음」 과 다르다.** 「그리는 순서」 글은 1845 px 오프스크린에서 오른쪽 끝이 조금 잘릴 수 있다(실제 글꼴 캡처에서는 전부 보임).
   007a 커밋(6542878)은 중간 해법(최소 폭 = 옛 글 폭 → shot_main 배치 JOB-006 그대로)이었고, 007b 에서 위 이유로 `1` 로 바꿨다.
2. **DXF 정면 방위각**: `exportDxf` 는 슬롯과 무관하게 늘 종단면을 쓰므로 `FRONT AZIMUTH` 는 확정 뒤면 늘 쓴다(`azimuthNote` 의 가로 슬롯 조건을 쓰지 않음).
3. **SVG 기준선 굵기 표기** `0.100 / 0.130 / 0.250`(`n3`) — 값은 0.10 / 0.13 / 0.25 와 같다(기준 6·10 숫자 비교).
4. 기준선 세 줄의 층 id·이름·굵기·DXF 층을 `kRefStyles` 표 하나로 화면·SVG·DXF 가 함께 쓴다(명세는 단계별 서술 — 동작 같음).
5. ③ 3D 폭은 `setFixedWidth(330)` 대신 `setMinimumWidth(edit ? 330 : 240)`·`setMaximumWidth(edit ? 330 : QWIDGETSIZE_MAX)`(같은 효과).
6. `test_cutpage` 의 슬롯 목록 판정은 `itemWidget` 안 QLabel 만 본다(`slotHas`) — `clear()` 로 지운 옛 항목 위젯이 지연 삭제 전까지 남아 거짓 참이 되지 않게.

## 남은 경고
- 빌드 경고 0. ctest 실패 0.
- (손 확인) 실제 글꼴 캡처에서 ③ ctx 배지 `✎ 도면 편집` 의 `✎`(U+270E)가 □ 로 나온다(오프스크린 + `QT_QPA_FONTDIR` 의 글꼴 대체 한계로 보임 — `✂` 는 보임). 실제 Windows 화면에서 확인 필요(ui-reviewer).
- (손 확인) ② 리본에 그룹이 늘어 실제 글꼴 캡처에서 「막 움직이기 (회전판)」 그룹이 좁아지고 「단면 위치」 가 x≈827(JOB-006 ≈895)로 옮겨진다(그룹 안 줄이 늘어나는 성질 — 높이는 같음).
- ② ctx 에 `Enter 지금 각도 고정` 이 늘어 오프스크린에서 ctx 안내(ctxHint)가 더 잘린다(폭 372 → 174 px, 툴팁에 전체).

## 제안 (범위 밖, 하지 않음)
- 오프스크린 캡처 글꼴(백로그 20 `QT_QPA_FONTDIR`)을 verify.ps1 에 넣어 □ 글꼴 폭이 레이아웃을 왜곡하지 않게 — 이번 1번 판단처럼 오프스크린 전용 배치 차이가 줄어든다.
- 치수 띠 왼쪽 `dims_`(리치 텍스트)도 창 최소 폭을 크게 민다(오프스크린 ≈ 913 px) — 좁은 화면(1366 px)용으로 줄임 규칙 검토.
- ③ 배지 글리프(✎)가 안 보이는 글꼴 환경 대비(아이콘 그림 또는 글꼴 지정).
- `Ribbon` 그룹이 페이지 여유 폭을 나눠 가지는 성질 때문에 그룹을 더할 때마다 다른 그룹 폭이 바뀐다 — 그룹 폭 고정 여부 결정.

## 커밋
- `6542878` feat(studio): 기준선 레이어(중심선·구연선·바닥선)·정면 방위각 — 도판·SVG·DXF·PDF·PNG (JOB-007) — 007a
- `89846ba` feat(studio): ② 「고정 · 넘기기」·③ 도면 편집 페이지 — 지금 각도 고정(Enter)·도면 편집으로·다시 자르기 (JOB-007) — 007b

## 차단 사유 (있으면)
- 없음.

## 재작업 1 (2026-10-18) — 기준 32 · **차단, 코드 변경·새 커밋 없음**

지적(verify.md): 오프스크린 `shot_main` 이 JOB-006 과 (a)(b)(d) 밖에서도 다름 — 3D 보조창 271 → 451 px, 도판 +90 px·도판 머리/치수 띠 +180 px,
(c) 「그리는 순서」 라벨 453 → 273 px(□ 23칸, JOB-006 38칸). 원인은 위 「명세와 다르게 판단한 곳」 1(`order_->setMinimumWidth(0)` → `1`).

### 결론
기준 32 의 「3D 같음」 을 맞추면 지금 통과하는 기준 15·17 이 깨지고, (c) 「길어짐」 은 어느 배치에서도 오프스크린에 보일 수 없다.
페이지와 무관한 제품 코드로는 32·15·17 을 함께 세울 방법이 없어, 고치지 않고 멈춘다(builder 규칙 7). HEAD 는 89846ba 그대로다.

### 근거 1 — 배치 계산 (오프스크린 1845 px, 슬롯 칸 176 px 고정, 3D : 도판 = 27 : 73, 3D 최소 240)
- 오프스크린 글꼴은 모든 글자(빈칸 포함)를 같은 폭 □(≈ 12 px)로 그린다. 그래서 치수 띠 왼쪽 `dims_` 가 ≈ 912 px 다.
- JOB-006 의 3D 271 px 은 R1 「≈ 27 %」 가 아니다(27 % 면 451 px). `dims_` 912 + 옛 「그리는 순서」 453 + 여백이 도판 칸 최소 폭을
  1398 px 로 밀었고, 3D 는 남은 1845 − 176 − 1398 = 271 px 을 받았다. `setMinimumWidth(0)` 은 Qt 에서 「정하지 않음」이라 이 밀기를 막지 못했다.
- 도판 칸 최소 폭은 페이지와 무관하다. 따라서 홈 3D 271 px(기준 32) 이려면 도판 칸 최소 폭이 1398 px 이어야 하고,
  ③ 3D 330 px 에 창 최소 폭 ≤ 1845(기준 15) 이려면 1845 − 176 − 330 = 1339 px 이하여야 한다. 둘은 함께 설 수 없다.
- (c): 3D 271 px 이면 그리는 순서 칸은 ≈ 456 px(□ 38칸), 3D 451 px 이면 ≈ 273 px(□ 23칸)이다. 새 글 47칸(≈ 560 px)이 들어갈
  자리는 1845 px 오프스크린에 없다. 실제 글꼴 캡처 33·34 에서는 새 글 전체가 보인다(verify.md).

### 근거 2 — 실험 (작업 트리에서 고쳤다가 되돌림, 커밋 안 함)
| 그리는 순서 최소 폭 | shot_main(홈, 오프스크린) | test_cutpage |
|---|---|---|
| `1` (지금 89846ba) | 3D 451 px, 라벨 □ 23칸 → **32 FAIL** | `[J7-13]` 1791 · `[J7-15]` 1791 · `[J7-17]` 통과 → PASS |
| 옛 글 폭(polish 뒤 `minimumSizeHint`, 007a 6542878 방식) | JOB-006 과 다른 화소 6,524개. 행 11-19(탭 글자)·274-302(구연선·기고 H 머리표)·816-838(바닥선 근처)뿐이다. 3D·치수 띠는 0 → 배치는 32 충족. 다만 라벨은 □ 38칸으로 JOB-006 과 화소까지 같아 **(c) 「길어짐」 안 보임** | `[J7-15] 창 최소 폭 1856 > 1845` **FAIL**, `[J7-17] ② 3D 보조창 폭 330` **FAIL**(> 330 아님) → FAILED |

- 실험 명령: scratchpad `bt.ps1 -Regex studio_cutpage`(vcvars + `cmake --build D:\_dongarch_build\build_job007\studio` + `ctest -R studio_cutpage -V`).
  캡처는 `DongArch3DStudio.exe --sample --shot …`(오프스크린)로 찍어 PIL 로 JOB-006 `shot_main.png` 와 화소 비교했다.
- 되돌린 뒤 `git status --porcelain` 출력 없음. 같은 빌드 폴더를 HEAD 로 다시 빌드하고 `studio_cutpage` 를 돌려 PASS, `[J7-13]`·`[J7-15]` 1791 을 확인했다.
  (`build_job007\studio\Testing\Temporary\LastTest.log` 는 이 개별 실행이 덮어썼다. 검증 폴더 `verify_job007` 은 건드리지 않았다.)

### 하지 않은 우회 (R1 과 무관한 오프스크린 전용이라)
- 페이지마다 최소 폭을 달리하기(홈·① 에서만 라벨이 밀고 ②·③ 에서는 안 밀기). 기준 17 은 ② 에서도 3D > 330 px 을 요구해서
  ② 까지 빼야 하므로 결국 「홈·① 에서만 미는」 배치가 된다.
- 시험 창을 ≥ 1900 px 로 키워 `[J7-17]` 을 맞추기(명세 §3.11 「같은 실행·같은 창」). 이렇게 해도 기준 15(1856 > 1845)는 남는다.
- 라벨 글꼴을 폭에 맞춰 줄이기(실제 화면 1366 px 등에서 글이 읽기 힘들 만큼 작아진다).

### 코디네이터·ui-reviewer 결정 요청 (택1)
- **A (권장, 코드 그대로 89846ba)**: 명세 32 를 고친다. 「3D 같음」 → 「3D 보조창이 R1 27 %(오프스크린 451 px)로 넓어지고 도판·도판 머리·
  치수 띠가 그만큼 옮겨짐 — JOB-006 의 271 px 은 □ 글꼴 라벨이 밀어 생긴 배치」. (c) 는 기준 25(`기준선(먹)` 문자열)와 실제 글꼴 캡처 33·34 로 판정한다.
- **B (JOB-006 배치 유지, 007a 방식으로 되돌림)**: 명세 15 의 창 최소 폭 상한을 1856 이상으로 올리거나 오프스크린 판정에서 빼고,
  17 은 시험 창 폭 ≥ 1900 에서 판정하도록 고친다. 대가: 오프스크린 ③ `--shot` 창이 1856 px 로 커진다. R1 27 % 와 다른 오프스크린 배치도 남는다.
  (c) 는 여전히 보이지 않는다(38칸 그대로).
- **C**: verify.ps1 에 `QT_QPA_FONTDIR`(백로그 20 — `factory/**` 라 이 작업 범위 밖)를 넣고 JOB-006 기준 캡처를 다시 찍는다. 오프스크린 배치가 실제 글꼴 배치(A 와 같은 451 px)와 맞게 된다.
