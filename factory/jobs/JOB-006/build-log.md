# JOB-006 구현 기록 (builder)

작업 장소: `D:\_dongarch_build\wt\.worktrees\JOB-006` (브랜치 `factory/JOB-006`, 기준 `804f60d`). 명세 §3 설계 결정을 그대로 구현했고, 실측 제품 코드가 300줄 안이라 **나누지 않고 006a + 006b 전부** 한 커밋으로 넣었다(§0).

## 바꾼 파일
| 파일 | 내용 | 이유 |
|---|---|---|
| `studio/src/CutPageText.h` (신규, 79줄) | `studio::cutpage` — ② 원문 라벨 상수(`kTabName`·`kLockTip`·`kCtxBadge`·`kCtxHint`·`kStatus`·`k3dTitle`·`k3dTip`·그룹·타일·줄 글자)와 `angleText`(0.1° 반올림, −는 U+2212)·`plateTitle`·`plateSub`·`atAxis`(원문 1460 판정)·`axisGroupName`·`faceText`·`ctxRight`. 공용 도우미 `tenthsText` | §3.2 |
| `studio/src/Ribbon.h/.cpp` | `pageChanged(int)`(`setCurrentPage` 끝에서 늘 내보냄)·`currentPage()`·`setPageLocked()`; `RibbonGroup::setTitle`·`titleLabel`·`button`·`rows`(줄 높이 23 px 고정, 줄 안 위젯 `AlignVCenter`); `column` 은 `button` 을 써서 같은 동작; `Tile::drawIcon` 글자 아이콘 `axX/axY/axZ`(먹색), 사용 불가 타일은 `Faint`·호버 바탕 없음 | §3.3 |
| `studio/src/Theme.h` | QSS 1줄 `#Tabs QPushButton[tab="true"][locked="true"] { color: %Faint%; }` (89-90 다음 — `on` 보다 뒤). 새 토큰 없음 | §3.3 |
| `studio/src/MainWindow.h/.cpp` | `pCut_`·`cutAxisGroup_`·`cutAngle_`·`ctxDupKeys_`·`onPage`; ② 페이지(파일 · 막 방향 X·Y·Z · 막 움직이기 3줄 · 단면 위치 3줄); `combined` 툴팁·`On` 상태가 ② 단추에도 걸리게 그 앞에서 생성; `poseConfirm` 툴팁; 홈 `rotate`·`cutMove` 단추 삭제; ctx `[ ]` 묶음 삭제·`ctxHint_->setMinimumWidth(1)`·② 에서 `← →`·`R` 묶음 숨김; objectName `CtxHint`·`CtxRight`·`StHint`·`CutAngle`; `refreshAll` 잠김·ctx 보임·상태줄 `kStatus`; `updateCutInfo`(② = 실제 면 수 문형, 그 밖 = `점 … 간략화`, 늘 각도 칸·그룹 이름·X/Y 켜짐); `requirePose` 순서; `confirmPose` → ②; `setHandTool` 문구·툴팁; 슬롯 메모 `막 12°`; `onAction`(`cutMinus/cutPlus` = `cut_.click` 한 곳, `axX/axY` = 대기 커서·진행 중 제스처 취소, `cut` = ② 오가기, ② 에서 `outline` = 홈으로 간 뒤 토글); `Esc` ② → 홈 | §3.4-3.8 |
| `studio/src/SheetView.h/.cpp` | `setCutPage`, ② 종단면 머리 `plateTitle`/`plateSub`(+ 미스캔 꼬리 그대로), objectName `SheetTitle`·`SheetSub` | §3.6-2 |
| `studio/src/View3D.h/.cpp` | `cutDrag_`(단면 도구 + 왼쪽 단추만 회전판, 오른쪽·가운데 = 보는 방향), `View3DPane::setCutPage`(3D 툴팁 `k3dTip`)·② 머리 `3D — 막으로 자르기`, 기본 제목 `막 · 중심축`, objectName `View3DTitle` | §3.5 「3D 끌기」·§3.6-3 |
| `studio/src/main.cpp` | `--cut`(사용법 주석 + `--shot` 블록 밖 200 ms 에 `S` 1번 · `→` 누름/뗌 12쌍) | §3.9 |
| `studio/tests/test_cutpage.cpp` (신규, 386줄) | 수용 기준 1-18 (표기 함수 → Ribbon 단독 → 오프스크린 MainWindow 한 실행) | §4 A |
| `studio/CMakeLists.txt` | `studio_core` 에 `CutPageText.h`, `STUDIO_GUI_SRC` 로 앱·시험 공용, `test_studio_cutpage`·`studio_cutpage`·`RUN_SERIAL TRUE` | §3.11 |
| `docs/ui/UI_RULES.md` | R1 표 1행(탭 줄·잠김)·3행(② = ✂ 도구 든 상태), R8 예시 `[ ]` → `← →` + 별칭 문장 | §3.10 (사람 검토 1) |

크기(`git diff --shortstat 804f60d -- studio/src`): 제품 **+309 / −51**, 빈 줄·주석만인 줄을 뺀 추가 **274줄**(300 이하 → 분할 안 함, 명세 추정 ≈245). 테스트 +386, CMake +10/−2, 정본 3줄.

### 명세에서 판단이 필요했던 곳
1. **빌드 폴더**: 명세 §5 는 `D:\_dongarch_build\verify_job006` 이지만 오케스트레이터 지시대로 `-BuildRoot D:\_dongarch_build\build_job006` 을 썼다. 기준 19-21·34 의 `B`·`L` 은 `D:\_dongarch_build\build_job006\studio`·`…\studio-logs`.
2. **각도 칸 모노 글꼴**: 전역 QSS `QWidget { font-family: "Malgun Gothic"; font-size: 13px; }` 가 polish 때 `setFont()` 를 덮으므로(Qt `QStyleSheetStyle::updateStyleSheetFont` — 규칙에 있는 속성이 이김) `CutAngle` 은 위젯 스타일시트 `font-family: Consolas; font-size: 12px;` 로 모노 12 를 준다. 고정 폭(`"−179.5°"` + 6 px)과 막 방향 이름 칸 최소 폭(`axisGroupName(-179.5)`)은 `ensurePolished()` 뒤 실제 `fontMetrics()` 로 잰다 — 시험(기준 9)이 같은 `fontMetrics()` 로 판정하므로 글꼴이 바뀌어도 어긋나지 않는다.
3. **API 하나 추가**: 이름 칸 최소 폭을 주려고 `RibbonGroup::titleLabel()`(명세 §3.3 목록에 없음)을 더했다.
4. **`rows()` 줄 안 정렬**: 줄 높이는 23 px 고정, 위젯은 `AlignVCenter` 로 넣어 kbd 배지·글이 23 px 로 늘어나지 않게 했다(작은 단추는 원래 23 px).
5. **ctx 안내 툴팁**: `setHandTool` 뿐 아니라 ① 확정 전 분기에서도 `ctxHint_` 툴팁 = 안내 글로 둔다(도구를 들었다 파일을 새로 열면 옛 ✂ 툴팁이 남는 것을 막음 — 1줄).
6. **기준 34 `shot_home` 해시가 다르다 — 원인은 사용자 설정이 아니라 이번 코드**(아래 「캡처」 참조). 명세 §7 의 가정("StartPage·ExportDialog 를 안 바꾸니 바이트까지 같다")이 성립하지 않는다: 시작 화면과 작업 화면은 한 창(`QStackedWidget`)이라 창 최소 폭 = 모든 페이지의 최소 폭 최댓값이고, JOB-005 에서는 홈 리본 페이지가 가장 넓었다. §3.8(기준 16)이 요구한 홈 `정면 회전 ±1°`·`절단 위치 이동 (1°)` 단추 삭제로 홈 리본이 좁아져 `--shot` 창 폭이 1845 → 1813 px(shot_home)·1814 px(shot_main)로 줄었다. 판단: 명세 지시(단추 삭제)를 따르고 FAIL 로 남긴다(창 최소 폭이 줄어든 것은 1366 px 쪽으로는 이득).

## 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| `powershell -NoProfile -ExecutionPolicy Bypass -File …\JOB-006\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-006 -BuildRoot D:\_dongarch_build\build_job006 -NoShots` | 0 | 첫 빌드부터 경고 0, ctest 16/16 |
| 같은 명령 `-Clean` (shots 포함, 커밋 전) | 0 | |
| `test_studio_cutpage.exe` 직접 실행(임시 측정 출력 넣어 창·페이지 최소 폭 확인 후 출력 제거) | 0 | 기준 34 원인 조사용. 시험 환경(사용자 설정 없음): 홈 리본 페이지 최소 폭 1789, ② 페이지 1130, 시작 화면 1791 px |
| `D:\_dongarch_build\verify_job005\studio\DongArch3DStudio.exe --shot …\job005_home_now.png` (JOB-005 실행 파일을 지금 다시) | 0 | 13246 바이트, **JOB-005 산출물 shot_home 과 해시 같음** → 환경(QSettings)은 그대로이고 차이는 이번 코드 |
| `git -C … commit -F …` | 0 | `eee6dd8` |
| verify.ps1 `-Clean` (커밋 뒤 최종, shots 포함) | 0 | 깨끗한 빌드 22 s, **경고 0**(`grep -ci warning studio-logs\02_build.log` = 0), ctest **100% tests passed out of 16**(110 s, studio_cutpage 15.3 s), shots 3장, `verify-summary.md` 마지막 줄 `결과: PASS` |
| 검증 명령 3 (`--sample --cut --shot shot_cut.png`, `--sample --base --shot shot_base.png`) | 0 | shot_cut 321,062 B, shot_base 68,502 B (`D:\_dongarch_build\build_job006\studio\shots\`). artifacts 복사는 하지 않았다(factory 기록은 build-log 만 쓰기 — 오케스트레이터 몫) |
| 기준 22-30 (Git Bash, 커밋 뒤 `git status --porcelain` 빈 상태) | — | 아래 표 |

### 수용 기준 결과
| 기준 | 결과 | 근거 |
|---|---|---|
| 1-18 | 통과 | `studio_cutpage` 마지막 줄 `PASS`, 종료 0 |
| 19 | 통과 | `100% tests passed out of 16`, `tests failed` 줄 없음 |
| 20 | 통과 | LastTest.log 세 줄 각 1 |
| 21 | 통과 | verify 종료 0, `결과: PASS`, warning 0 |
| 22 | 통과 | 엔진·기존 시험 diff 없음 |
| 23 | 통과 | MISSING 없음 |
| 24 | 통과 | 옛 용어 grep 빈 출력, `"rotate"|"cutMove"` = 0 |
| 25 | 통과 | 1 · 1 · 1 · 1 · 1 · 1 |
| 26 | 통과 | previewFrontAngle 1 · setFrontAngle(doc_.frontAngle() 0 · cut_.click( 1 · alignCut( 1 · WaitCursor 7 · confirmPose 안 setCurrentPage(pCut_) 1 |
| 27 | 통과 | 색 하드코딩 추가 없음, 축 색 없음, `+…ToolState::Primary` 0, 새 QColor 토큰 0 |
| 28 | 통과 | setMinimumWidth(1) 1 · `kbdLabel("[ ]"` 0 · `kbdLabel("Shift"` 2 · `kbdLabel("← →"` 2 · `"--cut"` 1 · LeftButton 1 · OK |
| 29 | 통과 | 1 · 1 · 1 · 1 · 0 |
| 30 | 통과 | `git status --porcelain` 빈 출력, 허용 목록 밖 파일 없음 |
| 31-33 | ui-reviewer 판정 대상 | 아래 「캡처」 |
| 34 | **shot_home FAIL / shot_export 통과** | shot_home 13,117 B(JOB-005 13,246 B, 창 폭 1813 vs 1845), shot_export 해시 같음 — 원인은 판단 6 |

### 화면 시험 인쇄 (사람 검토 7 자료 — verify.md 로 옮길 줄)
```
[9] ribbon height=107 px (JOB-005 계산값 107)
[9] ctxHint width=372 / sizeHint=612 px · ctx minimumSizeHint width=1420 px (offscreen 글꼴)
[9] 단면 위치 x=919
[11] 단면 위치 x=919
[13] 단면 위치 x=919
```

### 캡처 (오프스크린 — 글자는 모두 □, 배치만)
- `shot_cut.png`: 4번째 탭 밑줄(현재), 리본 그룹 4개(파일 타일 2 · 막 방향 타일 3 — 셋째 흐림 · 막 움직이기 3줄, 둘째 줄 `[−] θ [+]` + kbd 2개 · 단면 위치 단추+R 과 두 줄 글 — 둘째 줄 초록 `Done`), 리본 높이 shot_main 과 같음(탭 34 + 리본 107), ctx 줄 왼쪽 어두운 배지(② 에서 kbd 묶음 4개 — `← →`·`R` 숨김), 도판 빨간 단면 중심축 오른쪽·먹 외곽 왼쪽, 3D 굵은 절단선 오른쪽 반. 막 방향 그룹 이름이 긴 문형(`지금 12°`)으로 보임. X·Y·Z 글자 아이콘도 오프스크린에서는 □.
- `shot_base.png`: 3번째 탭 현재, 4번째 탭 글자 옅음, ctx 어두운 배지.
- `shot_main.png`(325,851 B): JOB-005(333,540 B)와 비교하면 (a) 탭 1개 추가 (b) 홈 리본 작은 단추 칸 3 → 2 와 그에 따른 오른쪽 그룹 이동 (c) 3D 머리 부제 — 그리고 **(d) 창 폭 1845 → 1814 px**(판단 6) 때문에 3D 보조창·도판의 가로 위치/폭이 함께 줄었다. 세로 위치·높이(창 920)와 치수 띠 값은 같다. 기준 33 의 "다른 곳은 (a)-(c) 뿐" 은 (d) 때문에 그대로는 성립하지 않는다.

## 남은 경고
- 빌드 경고 0. configure·build 로그 앞에 `'vswhere.exe' is not recognized …` 가 찍히나 vcvars64 환경 메시지이고 종료 코드·빌드와 무관(JOB-005 와 같음).

## 제안 (범위 밖, 하지 않음)
1. **기준 34·33 기대값 갱신**: shot_home·shot_main 은 창 최소 폭(= 홈 리본 페이지 폭)에 묶여 있어 홈 리본을 바꾸는 작업마다 달라진다. 비교는 "창 폭이 같을 때만 해시" 로 하거나, `--shot` 에서 창 크기를 고정(`resize` 후 최소 폭 해제)하는 방안을 다음 명세에서 검토.
2. **ctx 최소 폭**: 오프스크린 기준 ctx 줄 최소 폭이 아직 1420 px(> 1366) — 안내 칸 외의 고정 글(kbd 묶음·오른쪽 문구)이 민다. 사람 검토 7 결과에 따라 ctx 오른쪽 문구 줄이기·말줄임(§10).
3. **`setFont()` 가 전역 QSS 글꼴에 덮이는 문제(추정, 미확인)**: `ctxRight_`·`stCursor_` 의 `monoFont`, `SheetPane` 제목의 `serifFont` 등은 같은 원리로 Malgun 으로 바뀌었을 가능성이 크다(이번 각도 칸은 위젯 스타일시트로 피함). R4(수치 모노·제목 명조) 확인 작업 제안.
4. CI(`studio-ci.yml`) 캡처에 `--sample --cut --shot shot_cut.png` 추가(§10, 워크플로 변경이라 별도 지시 때).

## 커밋
- `eee6dd827ce3a04e1ecd0cedf90c27bf98de2708` — `feat(studio): ② 막으로 자르기 페이지 — 탭·잠김·리본 원문 라벨·ctx·상태줄·도판 머리·--cut (JOB-006)` (worktree `factory/JOB-006`, push 안 함)

## 차단 사유 (있으면)
- 없음.

---

## 재작업 1 (독립 검증 FAIL — 기준 34·33)

지적: 기준 34 `shot_home.png` 해시가 JOB-005 와 다름(13,117 B 1813×920 vs 13,246 B 1845×920). 기준 33(관찰) `shot_main` 이 JOB-005 와 (a)-(c) 말고도 창 폭 1845 → 1814 px 가 달라 3D 보조창이 271 → 240 px 로 줄고 도판이 31 px 옮겨짐. 추가 발견(추정): `shot_cut` 의 ctx 오른쪽이 44칸이라 확정 전(끄는 중)에 찍혔을 수 있음.

### 원인
1. **창 폭(34·33)**: 창 최소 폭 = 모든 페이지 최소 폭의 최댓값이고, 그 최댓값은 홈 리본 페이지다. 기준 16 대로 홈 단추 2개(`정면 회전 ±1°`·`절단 위치 이동 (1°)`)를 빼자 홈 리본이 좁아져 `--shot` 창 폭이 줄었다. 캡처 창 폭은 지금까지도 늘 이 최소 폭을 따라왔다(JOB-001~003 1854, JOB-004·005 1845, JOB-004 `shot_flip` 1871 — artifacts PNG 머리로 확인). 시작 화면 코드는 그대로이므로, 창 크기만 같으면 바이트까지 같아야 한다.
2. **ctx 오른쪽 44칸(추가 발견)**: 확정 전에 찍힌 것이 **아니다**. 임시 계측(캡처 직전 `CtxRight` 글·폭을 파일로 적는 코드, 커밋 전 삭제)으로 확인: 찍을 때 글은 놓은 뒤 문형 `막 12° · 끄는 동안 2.6만 면 미리보기 · 놓으면 원본 7.7만 면 다시 계산`(47자)인데 폭은 `w=528`(끄는 중 문형 44자 폭), `sizeHint=564`. 밀린 `LayoutRequest` 를 처리하면 `w=564`. 즉 1800 ms 캡처 타이머가 원본 확정 계산(UI 스레드 ≈ 1.4 s) 동안 밀렸다가, 계산 직후 글자만 바뀌고 다시 배치는 되기 전에 돌았다 — 명세 §3.9 의 "캡처는 확정 뒤" 가정은 맞고, 배치 시점만 어긋났다.

### 바꾼 파일
| 파일 | 내용 | 이유 |
|---|---|---|
| `studio/src/main.cpp` | ① `--shot` 이면 `show()` 전에 `w.resize(1845, 920)`(JOB-004·005 캡처 크기, 최소 폭이 더 크면 Qt 가 그 폭). 이를 위해 `args` 를 `show()` 앞으로 옮김 ② 캡처 람다에서 `grab()` 직전 `QApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest)` | ① 기준 34·33: 리본 단추 수와 무관하게 이전 캡처와 같은 크기로 찍어 비교 가능하게. 실제 앱 창(사용자 저장 위치·크기)·창 최소 폭은 바꾸지 않음(최소 폭 1845 → 1813 은 1366 px 쪽으로 이득이라 유지) ② 추가 발견: 확정 직후 다시 배치를 마친 화면을 찍게 |

크기: 이번 커밋 `main.cpp` +6/−1(빈 줄·주석 제외 추가 3). JOB 전체(`git diff --shortstat 804f60d HEAD -- studio/src`) 제품 **+315/−52, 빈 줄·주석 제외 추가 277줄**(300 이하 — 분할 없음).

검토한 다른 방법(택하지 않음): (가) 홈 리본 칸 폭을 예전 값으로 붙잡기 — 없는 단추 자리를 남기는 꼼수이고 기준 33 (b) "오른쪽 그룹 이동" 과도 어긋남. (나) 창·리본 최소 폭을 1845 로 고정 — 실제 앱을 1366 px 에서 더 나쁘게 만듦. (다) verify.ps1 에 크기 인자 추가 — `factory/**` 는 수정 범위 밖.

### 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| verify.ps1 `-Root …\JOB-006 -BuildRoot D:\_dongarch_build\build_job006`(① 만 넣고, shots 포함) | 0 | shot_home 해시 = JOB-005 확인 |
| `cmake --build …\build_job006\studio --target DongArch3DStudio` + `--sample --cut --shot` ×2 (임시 계측 코드 넣은 상태) | 0 | 위 원인 2 의 `w=528 hint=564` / `after LayoutRequest w=564`. 계측 코드는 삭제 |
| verify.ps1 같은 인자 **`-Clean`**(②까지 넣은 최종 내용, 커밋 직전과 같은 diff) | 0 | configure 4 s · build 22 s · ctest 110 s · shots 9 s, `결과: PASS` |
| `grep -ci warning …\build_job006\studio-logs\02_build.log` | — | **0** |
| `03_ctest.log` | — | `100% tests passed out of 16` |
| LastTest.log 세 줄(기준 20) | — | `1`·`1`·`1` |
| `studio_cutpage` 인쇄 | — | `[9] ribbon height=107 px` · `[9] ctxHint width=372 / sizeHint=612 px · ctx minimumSizeHint width=1420 px` · `[9]`·`[11]`·`[13] 단면 위치 x=919` · `PASS` (재작업 전과 같음) |
| 검증 명령 3(`--sample --cut --shot`, `--sample --base --shot`, 빌드 폴더 `build_job006`) | 0 · 0 | 4,075 ms · 3,034 ms |
| 기준 34 해시 비교(`build_job006\studio\shots` vs JOB-005 artifacts) | — | `shot_home.png: True`, `shot_export.png: True` |
| `git -C … commit -F …` | 0 | `688b7bd` |
| 기준 22-30(Git Bash, 커밋 뒤 `git status --porcelain` 빈 상태) | — | 모두 재작업 전과 같은 값으로 통과: 22 출력 없음 · 23 MISSING 없음 · 24 출력 없음/`0` · 25 `1·1·1·1·1·1` · 26 `1·0·1·1·7·1` · 27 출력 없음·출력 없음·`0`·`0` · 28 `1·0·2·2·1·1·OK` · 29 `1·1·1·1·0` · 30 출력 없음 |

### 캡처 결과 (`D:\_dongarch_build\build_job006\studio\shots\`)
| 파일 | 바이트 · 크기 | 비고 |
|---|---|---|
| shot_home.png | 13,246 B · 1845×920 | **JOB-005 와 해시 같음** (기준 34) |
| shot_export.png | 70,269 B · 1205×680 | JOB-005 와 해시 같음 (기준 34) |
| shot_main.png | 332,770 B · 1845×920 | JOB-005 와 화소 비교(PIL `ImageChops.difference`): 다른 화소는 **y 11-19, x 324-548 (504 화소 — (a) 탭 1개 추가)** 와 **y 40-134, x 243-1822 (36,395 화소 — (b) 홈 리본 작은 단추 칸 3 → 2 와 오른쪽 그룹 이동)** 두 띠뿐. y 135-919(리본 아래 경계·작업 공간·3D 보조창·도판·치수 띠·상태줄)는 **0 화소**. (c) 3D 머리 부제(`절단면 · 중심축` → `막 · 중심축`)는 오프스크린에서 글자가 모두 □ 이고 두 장 모두 3D 머리 칸에서 같은 자리까지만 보여 잘리므로 화소 차이가 없다 |
| shot_cut.png | 327,857 B · 1845×920 | ctx 오른쪽 칸 수 **47**(x 1269-1828, 12 px 간격) = 놓은 뒤 문형 글자 수. 배치는 재작업 전과 같음(탭 4번째 현재, 리본 그룹 4개, Z 타일 흐림, ctx 어두운 배지, 도판 단면 오른쪽) |
| shot_base.png | 70,058 B · 1845×920 | 배치 재작업 전과 같음 |

### 명세에서 판단한 곳 (재작업 1)
1. 명세 §3.9 는 `main.cpp` 에 `--cut` 만 적었으나, 기준 33·34 를 지키려고 `--shot` 창 크기 고정과 찍기 전 배치 처리를 더했다(둘 다 `--shot` 경로에만 작용, 실제 앱 화면 동작 무변경). 상수 1845×920 은 "JOB-004·005 캡처 크기" 라는 뜻이다 — 앞으로 기준 캡처 크기를 바꾸려면 이 한 줄만 바꾸고 기준 캡처를 다시 찍는다.
2. CI(`studio-ci.yml`) 캡처도 이제 1845×920 으로 찍힌다(저장된 창 위치가 없는 CI 에서도 같은 크기).
3. `shot_cut`·`shot_base` 는 해시 기준이 없어 폭이 1814/1813 → 1845 로 바뀐 것은 판정에 영향이 없다.

### 제안 (범위 밖, 하지 않음)
1. 재작업 전 제안 1(캡처 크기 고정)은 이번에 `--shot` 고정으로 반영됨. 창 **최소 폭** 자체(오프스크린 기준 1813 px, 홈 리본이 결정)가 1366 px 노트북보다 넓은 문제는 그대로 — 리본 그룹 접기·가로 스크롤 등은 별도 작업.

### 커밋 (재작업 1)
- `688b7bd` — `fix(studio): 검수 캡처 창 크기 1845×920 고정·찍기 전 다시 배치 (JOB-006 재작업 1)` (앞 커밋 `eee6dd8` 은 고치지 않음, push 안 함)

### 차단 사유 (재작업 1)
- 없음.
