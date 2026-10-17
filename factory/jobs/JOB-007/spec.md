# JOB-007 명세 — ② 「고정 · 넘기기」 + ③ 도면 편집 페이지 + 기준선 레이어(중심선·구연선·바닥선) + 도판 정면 방위각 기록

작성: spec-writer, 2026-10-18. 기준 커밋 `688b7bd`(factory/JOB-007 = JOB-006 끝, worktree `D:/_dongarch_build/wt/.worktrees/JOB-007`, **JOB-006 승인 전제** — JOB-003·004·005 병합됨).
아래 코드 경로·줄 번호는 모두 이 worktree 에서 직접 읽고 Grep 으로 확인한 값이다. 디자인 줄 번호는 정본 `D:\_dongarch_build\design\dongarch3d-design-v2.html`(이하 `H`)에서 Grep 으로 확인했다.
이하 `W` = `D:/_dongarch_build/wt/.worktrees/JOB-007`, `B` = `D:\_dongarch_build\verify_job007\studio`(빌드 폴더), `L` = `D:\_dongarch_build\verify_job007\studio-logs`(verify.ps1 로그 폴더), `A6` = `D:/_dongarch_build/wt/factory/jobs/JOB-006/artifacts`.
출발점: JOB-006 명세 §2·§10(JOB-007 몫), JOB-006 승인 보고서 §5, JOB-005 승인 보고서 §5-8(파일에 정면각이 남지 않음), 백로그 후보 3·14·17.

## 0. 분할 판단

전체 범위(§3)를 세면 **제품 ≈ 260줄(빈 줄·주석 제외 ≈ 218줄) + 시험·빌드 ≈ 397줄**(§9)이다. 이 공장의 300줄 지침(JOB-005 부터 빈 줄·주석 제외로 잼) 안이라 **나누지 않는다.** 다만 JOB-005 는 예상보다 +32 %, JOB-006 은 +13 %(빈 줄·주석 제외) 넘었으므로 이번 실측은 ≈ 245-290줄로 **300줄에 근접할 수 있다** → builder 는 **007a 부분을 먼저 구현·커밋**한 뒤 007b 로 넘어가며 줄 수를 재고, **빈 줄·주석 제외 300줄을 넘으면 그 자리에서 멈추고** 코디네이터에게 아래 두 조각을 제안한다. 두 조각은 서로의 코드 없이 자기 수용 기준만으로 참/거짓이 갈리게 나눴다. 007a 는 JOB-006 이 만든 파일·함수(`CutPageText.h`·`cutpage::`)를 쓰지 않는다(§3.3-3 — 각도 표기는 Plate.cpp 안에서 만든다) → JOB-006 이 재작업돼도 기능상 그대로 서고, 충돌은 `CMakeLists.txt`·`SheetView.cpp`·`UI_RULES.md` 의 문맥(같은 파일을 JOB-006 도 고침)에서만 날 수 있다.

| 조각 | 내용 | 파일 | 수용 기준 |
|---|---|---|---|
| **007a** 기준선 레이어·정면 방위각 | §3.3 전부(도판·SVG·DXF·PDF·PNG 기준선·닫는 선 정리·「기고 H」 머리표 정리, 정면 방위각 글, 문구 §3.3-7), UI_RULES R5·R6 | Plate.h·.cpp, ExportDialog.cpp, SheetView.cpp, tests/test_reflines.cpp, CMakeLists.txt, UI_RULES.md | 1-11, 21(이 경우 `out of 17`), 22-24, 25·26·28·29 의 007a 문자열, 27, 30(그 파일들), 31, 32(도판·치수 띠 글 부분 (b)-(d)), 35 |
| **007b** ② 고정·넘기기 + ③ 도면 편집 | §3.2·§3.4-§3.9, UI_RULES R1·R3 | CutPageText.h, Ribbon.cpp, MainWindow.h·.cpp, View3D.h·.cpp, main.cpp, tests/test_cutpage.cpp, UI_RULES.md | 12-20, 21(이 경우 `out of 16`), 22-24, 25·26·28·29 의 007b 문자열, 27, 30(그 파일들), 32 (a), 33, 34 |

## 1. 목표

사용자 요청(job.json `request`, 코디네이터 릴레이):
> 사용자 08:03 핵심 요구(바닥 정하기 → 얇은 막으로 자르기 → 도면 편집으로 넘기기 → Illustrator 내보내기·자동 축척), 사용자 요구 '기준선 레이어'(백로그 후보 14), 09:39 "승인개발진행", 10:59 "전부 다되면 개발진행하라". 디자인 v2 phase=cut 의 '고정 · 넘기기' 그룹과 ③ 도면 편집 화면.

project-profile.md 「사용자 핵심 요구」: "기준선 레이어: 중심선, 구연부 수평선, 바닥선, 다면도 눈금, 복원 파선, 파단 표시. 단면은 윤곽선 두 줄 기본." 코디네이터 전달: "국내 발굴보고서 토기 도면 표준(단면·평면·문양)을 따를 것, 초보자도 직관적으로."

**재진술(한 문장)**: ② 리본에 원문 「고정 · 넘기기」 그룹([지금 각도 고정] + `Enter`, 빨강 주 단추 [도면 편집으로])과 상태줄 오른쪽 `막 고정 전 / 막 12° 고정` 을 두고, 측정·탁본 탭 자리에 `③ 도면 편집` 탭(누르면 지금 막 각도로 고정하고 넘어감 — 바닥 확정 전에는 ② 처럼 잠김)을 만들어 그 탭에서는 「✎ 도면 편집」 을 든 채 도판을 넓게(3D 330 px) 보며 「↩ 3D로 돌아가 다시 자르기」 로 언제든 ② 로 돌아가게 하고, 도판·SVG·DXF·PDF·PNG 에 3D 기준에서 내려온 **중심선(3D 중심축)·구연선(구연 평면)·바닥선(바닥판)** 을 디자인 가이드 「도면 선 규약」 굵기로 **각각 독립 레이어**로 그리며, 내보낸 종단면 도판에 「정면 방위각 θ° (위에서 보아 반시계)」 를 적는다 — 계산 엔진(ArtifactDoc·CutGesture·MeshIO·K-Arch)은 바꾸지 않는다.

## 2. 범위 밖

- **편집 도구**(점 끌어 고치기·복원선 긋기·파단 표시·침선/회전흔 파선·탁본 넣기), **1~5 도면 버전 저장**(지금의 1~5 슬롯 저장은 그대로), **10걸음 단계 띠**, **Z 막**, 각도 띠·추천, 회전판 빨간 손잡이(→ ctx 안내·3D 끌기 원문 복원), 놓을 때 원본 계산 비동기화(백로그 16), **축 색 토큰**(R2 승인 대상).
- **기준선 2차**(사용자 목록의 나머지): 경부선(외면 실선·내면 긴 파선), 결실 쪽 구연선 파선, 복원 파선·파단 표시 층, 다면도 기준 눈금(`refline.ticks`), ③ 리본 「기준선」 그룹(☑ 중심선 · ☑ 구연선·바닥선 · ☐ 기준 눈금, `H` 1476)과 내보내기 창 「기준선」 칸(`H` 541) — 이번엔 기준선을 「단면·외곽」 표시와 함께 켜고 끈다(§3.3-1).
- **출력 규약 정리**(백로그 3): 출력 단면을 K100 0.30 윤곽선·채움 없음으로(`H` 756·798 — 지금은 빨강 0.35 + 연한 빨강 채움), 디자인 14층 id 로 통일(`H` 547-564), 축척 막대 "0 5 10㎝"·"(1/3)" 캡션, 보정 사각형 100 mm.
- 구연 수평(T)/바닥 수평(B) 선택(가이드 ④, `H` 1454-1455) — 이번 구연 평면은 확정 자세에서 바닥판에 평행한 유물 최고점 높이의 면(§3.3-2).
- 단면 닫힘 확인(가이드 §8 ⑦ 승인 기록 "닫힘", `H` 789).
- ② 밖(홈 등)에서 `← → , . [ ]` 가 막을 도는 것·중심축 맞춤/예제 열기 확정 뒤 ② 로 가지 않는 것(백로그 21) — 이번엔 **③ 에서만** 막는다(§3.6-5).
- 계산 엔진·불러오기: `studio/src/ArtifactDoc.*`·`CutGesture.h`·`MeshIO.*`·`MeshRaster.*`, `GigaMesh/**`(K-Arch `DxfWriter` 포함), `Theme.h`(새 색 토큰 없음), StartPage, `.github/workflows/*`(CI 캡처 목록 — §10), `factory/**`(verify.ps1 의 `QT_QPA_FONTDIR` — 백로그 20), `CLAUDE.md`, `docs/ui/mockup/**`.

## 3. 설계 결정 (builder 가 그대로 구현할 기본값)

### 3.1 원문 라벨과 이번 표기 (`H` 줄 = Grep 확인)

| 자리 | 원문 (`H` 줄) | 이번 | 비고 |
|---|---|---|---|
| 탭 줄 | `파일 · 홈 · ① 바닥 정하기 · ② 막으로 자르기 · ③ 도면 편집 · 내보내기` (1429) | 같음 — **`측정·탁본` 탭을 없애고** 그 자리에 `③ 도면 편집` | `측정·탁본` 페이지의 두 그룹(선 다듬기·간략화, 도판 표시)은 홈에도 있고 ③ 에도 넣는다 → 기능 손실 없음(사람 검토 3) |
| ③ 탭 들어가기 | 탭·단추를 누르면 고정 전이면 **지금 각도로 고정하고** 넘어감 (1714 `if(a==='edit'&&state.angFixed==null) state.angFixed=curAng()`, render 1681 도 같음). 다만 가이드 §8 표는 ⑦ 각도 고정 미완료 시 「도면 편집 · 치수」 잠김 (789) — **원문 안에서 갈림** | 시안 동작(1714) — 누르면 지금 각으로 고정하고 넘어감. **바닥 확정 전에는** ② 와 같은 잠김(`Faint` + 툴팁 `kLockTip`, 누르면 ① 로 안내 — JOB-006) | 각도 고정은 화면에 보이는 지금 각을 받아들이는 일이고 주 단추가 「도면 편집으로」 라, 잠그면 초보자에게 빨강 주 단추가 먹지 않는 것처럼 보인다. 바닥 확정은 후보·수평을 고르는 사람 판단이라 JOB-006 처럼 잠금 유지(사람 검토 2) |
| ② 그룹 | `고정 · 넘기기`: `지금 각도<br>고정`(자물쇠, 고정되면 활성) · `도면<br>편집으로`(→, **빨강**) (1470) | 같은 그룹 이름, 타일 **한 줄** `지금 각도 고정`(아이콘 `lock`, 고정되면 R3 `On`) · `도면 편집으로`(아이콘 `next`, `ToolState::Primary`) | 한 줄 = JOB-006 과 같은 이유(두 줄이면 모든 탭 리본 +7 px). ② 페이지에는 지금 주 단추가 없다 → `도면 편집으로` 가 ② 의 유일한 Action(R3) |
| `Enter` | ② 에서 `Enter` = 고정 (1784, 가이드 736·826) | ② 에서 확정 뒤 `Enter` = 「지금 각도 고정」. ctx 줄에 `kbd Enter` + `지금 각도 고정`(② 에서만 보임) | R8 배지. 원문은 오른쪽 단추 안 `Enter` 배지(1635) — 이 앱엔 그 단추가 없어 ctx 에 둠 |
| 상태줄 왼쪽 ② | `⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 「지금 각도 고정」 → 「도면 편집으로」` (1673) | **원문 그대로 복원** (JOB-006 의 `다음: 「도면」 내보내기 (Ctrl+P)` 꼬리를 되돌림) | `CutPageText.h:18` `kStatus` 한 줄 |
| 상태줄 오른쪽 | `막 고정 전` / `막 12° 고정` (1673, ③ 은 `막 12° 고정` 1674) | 새 QLabel `StFix` — ②·③ 에서만 보임, 같은 문구(각 = `angleText`) | |
| ③ 리본 | 그룹 7개 「파일」「선 고치기」「기준선」「문양·탁본」「선 다듬기」「도판 보기」「내보내기」(`도면 (PDF)` 빨강) (1472-1481) | 그룹 **5개** 「파일」「**고정된 막**」「선 다듬기 · 간략화」「도판 표시 (겹침)」「내보내기」(`도면` = 기존 Primary) | 편집 도구·기준선·문양 그룹은 범위 밖. 「고정된 막」 은 원문 리본에 없는 그룹 — 원문은 `막 12° 고정` 과 「↩ 3D로 돌아가 다시 자르기」 를 3D 아래 줄(1537)·도판 도구줄(1655)에 둠 → 이 그룹은 원문 1537 의 순서대로 `막 12° 고정`(위) · [↩ 3D로 돌아가 다시 자르기](아래) 두 줄(사람 검토 5 (마)) |
| ③ ctx | `✎ 도면 편집` · `점을 끌어 고치고 복원선 · 파단 표시 · 탁본을 넣습니다 · 1~5 도면 버전 저장` · 오른쪽 `점 1,420 → 186 (0.5 mm 간략화) · 최대 편차 0.21 mm` (1665) | 배지 같음. 안내 = 원문 도판 안내 줄(1656)을 줄인 글 `단면이 기준선 위에 놓였습니다 · 중심선 = 3D 중심축 · 바닥선 = 바닥판 · 구연선 = 구연 평면` — 원문의 앞 `✓ `(초록)·끝 ` · 막 12° 고정` 은 빼고 두 span 을 ` · ` 로 이음(각은 ③ 리본·상태줄에 있음). 오른쪽 = 기존 `점 N → <b>M개</b> (0.5 mm 간략화)` | 편집 도구가 없으니 없는 도구를 가리키지 않는다. 최대 편차는 계산 없음(사람 검토 5 (다)) |
| ③ 상태줄 | `⑧ 치수 확인 — 「치수 띠에 반영」 → ⑨ 문양·복원 → ⑩ 「도면(PDF)」 저장` (1674) | `⑧ 치수 확인 — 치수 띠를 보고 다음: 「도면」 내보내기 (Ctrl+P)` | 「치수 띠에 반영」 단추·⑨ 가 없다(사람 검토 5 (라)) |
| ③ 3D | `3D — 고정된 막` · [비스듬히][도판 방향] · 폭 330 px (1535, 가이드 691 "편집 단계는 도판을 넓게") | 머리 `<b>3D — 고정된 막</b>`, 단추는 기존 [정면][위], **3D 보조창 폭 330 px 고정**(②·홈은 지금처럼 27 %·최소 240) | |
| ③ 도판 머리 | `A-A′ 정면 실측도` · `— 외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)` (1654) | 기존 비-② 머리 그대로(`A–A′ 정면 실측도` · `외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)`) | 원문은 `A-A′`(하이픈), 앱 슬롯 이름은 `A–A′`(en dash, `ArtifactDoc.cpp:717`) — 그대로(사람 검토 5 (다)) |
| 다시 자르기 | `↩ 3D로 돌아가 다시 자르기` (1537·1655) | ③ 리본 「고정된 막」 그룹의 작은 단추(id `backToCut`, kbd `S`) — 누르면 ②(고정은 막을 움직일 때까지 유지) | `S` 는 이미 「② 로 가기」(JOB-006) |
| 기준선 (도판) | 중심선 = 3D 중심축, 바닥선 = 바닥판, 구연선 = 구연 평면 (431·1656). 규약표(750-762): 중심선 가는 실선·위아래 연장·화면 회색·출력 0.10·3 mm, 구연선 전체 폭·먹 1 px·0.13, 바닥선 전체 폭·먹 1.4 px·0.25 | 같은 세 선. 출력 0.10 / 0.13 / 0.25 mm(종이 기준)·중심선 종이 3 mm 연장, 화면 1 / 1 / 1.4 px. 색은 셋 다 `OuterInk`(R2·R5 「중심축은 먹색 실선」 — 원문 화면 회색과 다름) | 폭: 구연선 = 구경(`H` 1649 「구경 (구연선 폭)」), 바닥선 = 저경 — 원문 출력(1853-1856)은 외면·단면 윤곽의 실제 끝점(사람 검토 6) |
| SVG 층 | `refline.center` · `refline.rim` · `refline.base` (555-557) | 최상위 `<g id>` 3개 **`refline_center` · `refline_rim` · `refline_base`**, `inkscape:label` `중심선`·`구연선`·`바닥선` | 기존 층 id 모양(`grid_dims_scale`·`outline_axis`)에 맞춰 점 대신 밑줄. 원문 표시명은 `중심선 (위아래 3 mm 연장)`·`구연선 · 경부선 (전체 폭)`·`바닥선 (= 바닥판)` — 경부선이 없고 폭이 구경이라 짧은 이름(사람 검토 5 (아)) |
| DXF 층 | (원문 규정 없음, 「DXF R2000」 541) | 기존 11층 그대로 — 중심선 `01_AXIS_CENTER`, 구연선 `07_PROJECTION_RIM`, 바닥선 `08_PROJECTION_BASE`(셋 다 ACI 7·CONTINUOUS) | 07·08 은 Studio DXF 에서 지금 빈 층. 입면도에서 구연 원·저부 원의 투영 = 구연선·바닥선. K-Arch 단독 출력기(test_dxf_units)는 같은 층에 평면 투영 원을 그린다(사람 검토 7) |
| 정면 방위각 | (원문 도판에 없음, JOB-005 §5-8 제안) | SVG·PDF·PNG 표제란 첫 줄 끝 ` · 정면 방위각 12° (위에서 보아 반시계)`(종단면만), DXF `10_ANNOTATION_TEXT` `FRONT AZIMUTH 12.0 DEG (CCW, TOP VIEW)` | 도판 라벨은 보고서 용어 「정면 방위각」(용어표 `H` 775) — JOB-006 기준 24 가 `절단 각도` 를 금지(사람 검토 8) |

### 3.2 표기 상수 (`studio/src/CutPageText.h`)

- 머리 주석 1-3행을 "②·③ 페이지(막으로 자르기·도면 편집)의 화면 문자열·표기 함수 — 원문 phase=cut·edit, 다르게 정한 곳은 JOB-006 §3.1·JOB-007 §3.1" 로. `#include <optional>`.
- **18행 `kStatus`** → `⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 「지금 각도 고정」 → 「도면 편집으로」`.
- 새 상수: `kGrpFix = "고정 · 넘기기"`, `kTileFix = "지금 각도 고정"`, `kTileEdit = "도면 편집으로"`, `kEditTab = "③ 도면 편집"`, `kEditBadge = "✎ 도면 편집"`, `kEditHint = "단면이 기준선 위에 놓였습니다 · 중심선 = 3D 중심축 · 바닥선 = 바닥판 · 구연선 = 구연 평면"`, `kEditStatus = "⑧ 치수 확인 — 치수 띠를 보고 다음: 「도면」 내보내기 (Ctrl+P)"`, `k3dEditTitle = "3D — 고정된 막"`, `kGrpFixed = "고정된 막"`, `kBackToCut = "↩ 3D로 돌아가 다시 자르기"`. (기준선 설명 줄은 ③ ctx `kEditHint` 하나에만 둔다 — 리본에 같은 글을 두지 않음, JOB-006 `ctxDupKeys_` 와 같은 원칙.)
- `inline QString fixText(const std::optional<double>& fixed)`: 없으면 `막 고정 전`, 있으면 `"막 " + angleText(*fixed) + " 고정"`(예: `막 12° 고정`, `막 −20° 고정`, `막 37.5° 고정`).
- `inline QString slotMemo(double deg)`: `"막 " + angleText(deg)`(예: `막 12°`) — 1~5 슬롯 저장(`saveSlotAngle`, MainWindow.cpp:823)과 고정(§3.5)이 같은 메모를 쓴다(823 의 식을 이 함수로 바꿔도 됨, 결과 글자 같음).

### 3.3 기준선 엔진·정면 방위각 (`studio/src/Plate.h` · `Plate.cpp`)

1. **`Plate.h`**: `#include <optional>`. `PlateOptions`(14-23)에 `double paperPxPerMm = 0.0;` — 주석 "0 = 화면(기준선 1 · 1 · 1.4 px, 중심선 연장 도면 3 mm), > 0 = 용지(paintSheet 가 넣음: 종이 1 mm 의 장치 px — 기준선 굵기·연장을 종이 기준으로)". 그리고 선언:
   ```cpp
   struct RefLine { KA::Point2 a, b; };
   // 기준선 (디자인 v2 가이드 §6 · 도면 mm, x = R(대칭이라 좌/우 부호 무관), y = H)
   struct PlateRefLines { std::optional<RefLine> center, rim, base; };
   // center = 3D 중심축 (0,−ext)→(0,H+ext) · rim = 구연 평면 (−구경/2,H)→(+구경/2,H) · base = 바닥판 (−저경/2,0)→(+저경/2,0).
   // H = 메시 높이(바닥판 Z 0 → 최고점, 도판 height 와 같음). 미로드·확정 전 = 셋 다 없음, 구경·저경이 0 이면 그 선 없음.
   PlateRefLines plateRefLines(const ArtifactDoc& doc, double extMm);
   // 내보낸 종단면 도판의 정면 방위각: "정면 방위각 12° (위에서 보아 반시계)". 가로 슬롯·미로드·확정 전 = 빈 문자열.
   QString azimuthNote(const ArtifactDoc& doc, int slot);
   ```
   **화면·SVG·PDF·PNG 의 기준선은 외곽선과 함께 켜고 끈다**(`o.outline` / `r.opts.outline` — 화면 「단면·외곽」(L), 내보내기 「외곽선·기준선」 칸). 그래서 `test_side` 의 래스터 시험(외곽 끔)은 그대로 참이다(test_side.cpp:401-406). **DXF 는 예외** — `exportDxf` 는 지금도 `r.opts` 중 `hatch`(Plate.cpp:779)만 보고 외곽선·눈금을 늘 쓰므로, 기준선 3층도 칸과 무관하게 늘 쓴다(사람 검토 9).
2. **`plateRefLines`**(Plate.cpp 안, 익명 이름공간의 `height()` 30-32 를 씀): `!doc.isLoaded() || !doc.poseConfirmed()` → 빈 값. `H = height(doc)`, `d = doc.dims()`. `center = {(0,−ext),(0,H+ext)}`, `d.rim > 0` 이면 `rim = {(−d.rim/2,H),(d.rim/2,H)}`, `d.base > 0` 이면 `base = {(−d.base/2,0),(d.base/2,0)}`. 구연 평면 = 확정 자세에서 바닥판에 평행한, 유물 최고점 높이의 면(이 앱엔 구연 수평 선택이 아직 없다 — §2).
3. **`azimuthNote`**: 가로 슬롯(`isHorizontal`)·미로드·확정 전이면 `""`, 아니면 `"정면 방위각 " + 각 + " (위에서 보아 반시계)"`. 각 글자는 **Plate.cpp 익명 이름공간의 작은 함수**로 만든다 — `t = std::llround(deg * 10.0)`(정수 0.1° — `/fp:fast`(CMakeLists.txt:21)에서도 `−0` 이 생기지 않음), 화면 글 = `(t < 0 ? "−"(U+2212) : "") + |t|/10 + (|t|%10 ? "." + |t|%10 : "") + "°"`(결과는 `cutpage::angleText`(CutPageText.h:40-43)와 같은 글자: 12° · 37.5° · −20° · −0.04 → 0°), DXF 글 = `(t < 0 ? "-" : "") + |t|/10 + "." + |t|%10`(12.0 · −20.0 · 0.0). **Plate.cpp 는 `CutPageText.h` 를 include 하지 않는다**(도판 엔진이 ② 페이지 문자열 헤더에 기대지 않게 · 007a 가 JOB-006 파일 없이 서게 — 기준 28).
4. **화면·PDF·PNG (`Plate::paint`, 145-366)**: 상수 `kRefExtPaperMm = 3.0`, 굵기 `0.10 / 0.13 / 0.25`(종이 mm), 화면 `1.0 / 1.0 / 1.4`(px).
   - 방안 단계의 **도판 전체 폭 바닥선**(222-224 `// 바닥선 H 0` 과 그 `drawLine`)을 **지운다** — 바닥선은 기준선 하나뿐(10 mm 회색 주선과 `0 mm (바닥)` 숫자는 그대로).
   - 외곽선 단계의 **중심축 선**(283-284 `setPen(QPen(theme::OuterInk, 1))`·`drawLine(…(0, -4)…(0, H + 6))`)을 **지운다**.
   - 외곽선 단계의 **닫는 가로선**(290-292: 축 → 외곽선의 두 끝점까지 수평선)은 **그 높이의 기준선이 있으면 그리지 않는다** — 두 끝 중 **y 가 작은 끝**의 닫는 선은 바닥선이 있으면, **y 가 큰 끝**의 닫는 선은 구연선이 있으면 뺀다(끝점 순서에 기대지 않는다. 참고로 지금 `displayOutline()` 은 K-Arch `leftOutline` 순서 그대로라 z 오름차순 — Silhouette.cpp `finishProfile`(76) 이 빈(z 오름차순, 145-146) 순서대로 넣고 ArtifactDoc.cpp:795-800 의 간략화·곡선은 순서를 바꾸지 않음 → front = 아래, back = 위). 까닭: K-Arch 실루엣은 **꼭짓점 z 값의 2·98 백분위**(`percentile(allZ, 0.02/0.98)`, Silhouette.cpp:118-119) 사이만 쓰고, 꼭짓점 8개 미만인 빈·이웃보다 3 mm 넘게 안쪽인 빈도 버려서 외곽선 끝이 바닥·구연보다 안쪽이다 — 예제에서 아래 끝 y 3.36, 위 끝 y 181.61, H 184.82(JOB-006 빌드 `studio_plate.dxf` 실측). 틈의 크기는 높이 비율이 아니라 **꼭짓점 분포에 따라 스캔마다 다르다**(구연부 꼭짓점이 적은 실제 스캔이면 위 틈이 더 클 수 있음). 닫는 선을 두면 구연선·바닥선에서 3 mm 남짓 떨어진 **두 번째 가로선**이 생겨 구연선을 잘못 읽게 한다(사람 검토 6). 외곽선 점은 그대로다.
   - 음영(2, 264-277) 뒤·외곽선(3, 279) 앞에 새 단계 「2.5) 기준선」: `if (o.outline)` 일 때 `paper = o.paperPxPerMm > 0`, `ext = paper ? kRefExtPaperMm * o.paperPxPerMm / px1mm : kRefExtPaperMm`, `plateRefLines(doc, ext)` 의 있는 선만 `QPen(theme::OuterInk, paper ? std::max(1.0, w_mm * o.paperPxPerMm) : w_px, Qt::SolidLine, Qt::FlatCap)` 로 그린다. (가로 슬롯은 이 단계 전에 return — 232-262.) 이 단계에서 구한 `plateRefLines` 값(`refs`)을 3)·5) 단계도 쓴다.
   - **「기고 H」 머리표(5) 단계, 352-358) 정리** — `rimLine = o.outline && refs.rim` 이면: Caution 1 px 파선을 `E.left() + 2` → **`−d.rim/2 − 2`**(도면 mm, 구연선 바깥 끝 2 mm 앞)까지만 긋고(끝이 시작보다 왼쪽이면 긋지 않음), 라벨 상자 `기고 H … mm` 는 **구연선 바로 위**에 놓는다(`labelBox(p, h0 + QPointF(4, -2), …, Qt::AlignLeft | Qt::AlignBottom)` — 상자 아래 끝이 구연선 2 px 위). `rimLine` 이 아니면 지금 그대로(파선 → 축, 상자 가운데 맞춤). 까닭: 5) 는 2.5) 보다 나중에 그려져, 그대로 두면 구연선 외면 쪽 반이 갈색 파선에 덮여 기고 지시선과 구연선이 헷갈리고, 흰 라벨 상자(화면 ≈ 35 mm, A4 1:1 ≈ 25 mm 폭)가 구연선 외면 쪽 끝 — 외곽선 위 끝과 구연선 사이 틈(사람 검토 6) — 을 가린다. 2-3줄 변경.
   - `paintSheet`(440-492): `PlateOptions o = r.opts;`(458) 다음에 `o.paperPxPerMm = ppm;`. 표제란 첫 줄(470-474)에 `azimuthNote(doc, r.opts.slot)` 이 비어 있지 않으면 ` · ` + 그 글을 붙인다.
5. **SVG (`exportSvgLayered`, 536-737)**:
   - 방안 층의 `if (!horiz)` 먹색 바닥선(600-603)을 **지운다**.
   - 표제란 `축척 … · 단위 mm` 줄(612-613)에 `azimuthNote` 가 비어 있지 않으면 ` · ` + 글.
   - 음영 층(646-672) 뒤·외곽선 층(673) 앞에, `r.opts.outline && !horiz` 이면 `plateRefLines(doc, 3.0 / s)`(→ 종이 3 mm) 의 **있는 선마다 최상위 층 하나**: `layer("refline_center", "중심선")` · `layer("refline_rim", "구연선")` · `layer("refline_base", "바닥선")`, 층 안에는 `<line x1 y1 x2 y2 stroke="(theme::hex(theme::OuterInk))" stroke-width="0.1 / 0.13 / 0.25"/>` **하나만**(stroke·굵기는 그 `<line>` 에, dasharray 없음), 좌표는 기존 `X()`·`Y()`·`n3()`. 순서 중심선 → 구연선 → 바닥선.
   - 외곽선 층: `inkscape:label` `외곽선·중심선`(675) → `외곽선`(id `outline_axis` 는 시험 호환으로 유지), 그 위 주석 `// (3) 외곽선·중심선`(673)도 `// (3) 외곽선` 으로(기준 26 이 주석까지 본다), 중심선 `<line … stroke-dasharray="6 1 1 1"/>`(679-681) **지운다**, 닫는 가로선 두 `<line>`(685-692)은 화면과 같은 규칙(y 가 작은 끝 = 바닥선, 큰 끝 = 구연선이 있으면 뺌) — 예제에서는 외곽선 층에 `<polyline>` 만 남는다.
   - 새 색은 `theme::hex(...)` 로만(`#xxxxxx` 글자 추가 금지 — 기준 27). **hex 가 든 기존 줄은 다시 배치하지 않는다**: 표제란 문장(607-613)은 613 만 고치고 607-611 은 그대로, 외곽선 층 문장(677-681)은 679-681 을 지우고 문장 끝 `;` 만 678 끝으로 옮긴다(677 그대로). clang-format 등 형식 도구를 Plate.cpp 전체에 돌리지 않는다.
6. **DXF (`exportDxf`, 741-806)**: 층 정의 `01_AXIS_CENTER`(751) → `7, LT::Continuous`, `07_PROJECTION_RIM`·`08_PROJECTION_BASE`(757-758) → `7, LT::Continuous`(층 수·이름·순서 그대로 11개). 중심선(763) → `plateRefLines(doc, 3.0)` 의 center 를 `WriteLine("01_AXIS_CENTER", a, b)`(선 종류 덮어쓰기 없음 — 코드 6 이 안 써진다, DxfWriter.cpp:153). 구연선 → `07_PROJECTION_RIM`, 바닥선 → `08_PROJECTION_BASE` 에 `WriteLine` 하나씩. `02_EXTERIOR_OUTLINE` 의 닫는 선 두 `WriteLine`(767-768)은 화면과 같은 규칙(y 가 작은 끝 = 바닥선, 큰 끝 = 구연선이 있으면 뺌) — 예제에서는 02 층에 POLYLINE 만 남는다. 정면 방위각 → `WriteText("10_ANNOTATION_TEXT", {-maxR(doc), -12}, 2.5, "FRONT AZIMUTH " + DXF 각 글 + " DEG (CCW, TOP VIEW)")` — DXF 각 글은 §3.3-3 의 정수 0.1° 함수(`%.1f`·`+ 0.0` 을 쓰지 않음 — `/fp:fast` 가 `x + 0.0` 을 접으면 `-0.0` 이 나올 수 있다). 기준선 3층은 내보내기 칸과 무관하게 늘 쓴다(§3.3-1). DXF 는 1:1 모델 공간이라 연장 3 mm.
7. **화면 문자열**: `studio/src/SheetView.cpp:232` → `그리는 순서: 방안눈금 → 탁본음영 → 기준선(먹) → 외곽선(먹) → 단면선(빨강)` — 이 `order_` 라벨은 치수 띠(InfoStrip, 224-238) 오른쪽에 **늘 보이므로** 홈 화면 `shot_main` 에도 글이 길어진다(기준 32 (c)). `studio/src/ExportDialog.cpp:125` → `외곽선·기준선 Layer (먹색 · 외곽선 0.50 · 기준선 0.10~0.25 mm)`(hex 글자 없음), 255 → `• SVG 선택 시: Adobe Illustrator에서 열었을 때 단면선·외곽선·기준선(중심선·구연선·바닥선)·눈금이 각각 독립된 레이어로 분리되며 1:1 mm 치수가 정확히 유지됩니다.`, 257 → `• DXF 선택 시: R12 · mm 단위 · K-Arch 11 레이어(01_AXIS_CENTER … 11_SCALE_BAR — 기준선: 중심선 01 · 구연선 07 · 바닥선 08)로 CAD에서 1:1로 엽니다. 음영은 포함되지 않습니다.`
8. 이 묶음(1-7)은 007a 의 전부다.

### 3.4 ② 「고정 · 넘기기」 와 ③ 리본 페이지 (`MainWindow::buildRibbon` 158-308, `Ribbon.cpp`)

- `Ribbon.cpp` `Tile::drawIcon`(36-91): 종류 `"lock"`(자물쇠: 둥근 몸통 + 고리, 원문 `i-lock`)·`"next"`(→ 화살표, 원문 `i-next`) 추가 — 기존처럼 2 px 둥근 선, 색 `c`.
- 페이지: 202 `pMeasure = addPage("측정·탁본")` → `pEdit_ = ribbon_->addPage(cutpage::kEditTab)`(멤버, 번호 4). 285-286 `simplifyGroup(pMeasure); showGroup(pMeasure);` 은 ③ 쪽으로 옮긴다.
- ② (273 「단면 위치」 그룹 다음): `auto* g = ribbon_->addGroup(pCut_, cutpage::kGrpFix);` `g->tile("fixAngle", cutpage::kTileFix, "lock", "지금 각도 고정 (Enter) — 지금 막 각도로 종단면 도면을 정합니다 · 막을 다시 움직이면 풀립니다");` `g->tile("toEdit", cutpage::kTileEdit, "next", "③ 도면 편집으로 — 고정 전이면 지금 각도로 고정하고 넘어갑니다 · 중심선 = 3D 중심축 · 바닥선 = 바닥판 · 구연선 = 구연 평면");`
- ③: `fileGroup(pEdit_)` → 「고정된 막」 그룹: `fixedLbl_ = textLabel(cutpage::fixText(std::nullopt), g)`(objectName `FixedAngle`, 먹색 — 원문 1537 `<b>막 12° 고정</b>`; 글이 바뀌어도 리본이 옆으로 밀리지 않게 `ensurePolished()` 뒤 `setMinimumWidth(fontMetrics().horizontalAdvance("막 −179.5° 고정"))`, JOB-006 각도 칸과 같은 방법), `back = g->button("backToCut", cutpage::kBackToCut, "S")`, 툴팁 `② 막으로 자르기로 돌아가 막을 다시 움직입니다 (S) — 막을 움직이면 고정이 풀립니다`, `g->rows({{fixedLbl_}, {back}})`(원문 1537 순서: 각 → 단추. 2줄, 리본 높이 불변) → `simplifyGroup(pEdit_)` → `showGroup(pEdit_)` → `exportGroup(pEdit_)`. ③ 리본 그룹은 차례로 「파일」「고정된 막」「선 다듬기 · 간략화」「도판 표시 (겹침)」「내보내기」 **5개**.
- 290-291 옆에 `ribbon_->setState("toEdit", ToolState::Primary);` — R3: ② 의 유일한 Action. ③ 의 Action 은 기존 `export`(도면) 하나.
- 폭: ② 는 타일 2개(≈ 220 px)가 늘어도 홈 페이지보다 좁고(JOB-006 `[9] 단면 위치 x=919`), ③ 은 홈보다 그룹이 적다 → 창 최소 폭은 홈이 정한다(기준 13·15 가 인쇄·확인).

### 3.5 고정 상태 (`MainWindow`)

- 멤버(MainWindow.h): `std::optional<double> fixedAngle_;`, `int pEdit_ = -1;`(115 행에 더함), `QLabel* stFix_ = nullptr;`, `QLabel* fixedLbl_ = nullptr;`, `QVector<QWidget*> ctxFixKeys_;`, 함수 `void fixAngle();`, `bool angleFixed() const;`, `int firstVerticalSlot() const;`(첫 종단면 번호, 없으면 −1 — `onPage` 732-736 의 찾기를 이 함수로 옮겨 함께 씀).
- `angleFixed()` = `fixedAngle_ && doc_.poseConfirmed() && !doc_.isPreviewing() && !cut_.active() && std::fabs(std::remainder(*fixedAngle_ - doc_.frontAngle(), 360.0)) <= 1e-9`.
- `fixAngle()`: `fixedAngle_ = doc_.frontAngle();` **고정할 슬롯 `k` = 지금 슬롯이 종단면이면 `slot_`, 아니면 `firstVerticalSlot()`**(= ③ 에 들어갈 때 `onPage` 가 고를 슬롯) — `k >= 0` 이면 `sl[k].angleDeg = *fixedAngle_`, `sl[k].note = cutpage::slotMemo(*fixedAngle_)`(`막 105°` — 1~5 슬롯 저장 메모와 같은 모양, **「고정」 은 메모에 저장하지 않는다**). 슬롯을 다시 골라도 고정 각으로 돌아온다(그러지 않으면 ③ 에서 B–B′ 를 봤다가 A–A′ 를 다시 고를 때 `selectSlot`(673-687)이 옛 슬롯 각으로 `setFrontAngle` 하고, `refreshAll` 이 그 각으로 조용히 다시 고정한다 — ② 에서 B–B′ 를 보며 `Enter` 로 고정한 경우도 `k` 가 첫 종단면이라 막힌다, 기준 18). 되돌리기에는 쌓지 않는다(ViewState 에 슬롯 없음).
- **슬롯 목록의 「고정」 표시는 살아 있는 상태로**(원문 ② 1508 `고정됨 🔒 12°`/`고정 전 · 움직이는 중`, ③ 1514 `막 12° 고정`): `refreshSlots`(640-671)의 메모 라벨(661)을 `fx ? cutpage::fixText(fixedAngle_) : s.note` 로 — `fx = s.kind == SlotKind::Vertical && angleFixed() && std::fabs(std::remainder(s.angleDeg - *fixedAngle_, 360.0)) <= 1e-9`. 그래서 고정 중에는 그 각의 종단면 슬롯이 `막 105° 고정`, 풀리면 저장된 메모 `막 105°` 로 돌아가고(StFix `막 고정 전` 과 어긋나지 않음), 고정 중에 1~5 슬롯 저장이 메모를 `막 105°` 로 덮어써도 표시는 `막 105° 고정` 그대로다.
- **`fixedAngle_.reset()` 하는 곳**(막 각도가 바뀌거나 문서·자세가 바뀌는 모든 길): `beginCutInput()`(806-808 — 끌기·키·[−][+] 모두. 여기서는 값이 있었을 때만 `refreshSlots()` 도 부른다 — 끄는 동안은 `refreshAll` 이 오지 않아 슬롯 목록에 `고정` 이 남기 때문, 제스처마다 한 번), `axX`/`axY` 분기(915-925), `restore()`(764-774 — 되돌리기·다시), `openPath`(503 — 불러오기 성공 뒤, 525 `refreshAll(true)` 전)·`openSample`(541)·`openArtecBlindSample`(556), `confirmPose()`(708, `setCurrentPage(pCut_)` 전), `onAction("axis")`(898-905). `beginCutInput` 밖의 리셋 지점은 모두 뒤에 `refreshAll` 이 온다(위 줄 번호에서 확인). 그 밖(슬롯 고르기로 각이 바뀜 등)은 `angleFixed()` 의 각 비교가 거른다.
- **③ 에서는 지금 각이 곧 고정 각**(원문 render 1681): `refreshAll`(580) **맨 처음**에 `if (ribbon_->currentPage() == pEdit_ && doc_.isLoaded() && doc_.poseConfirmed() && !angleFixed()) fixAngle();` — ③ 에 들어갈 때(자동 고정, 1714)와 ③ 에서 되돌리기 등으로 각이 바뀐 때 모두. `refreshSlots` 보다 앞이라 슬롯 메모가 같은 갱신에 보인다.

### 3.6 페이지 = 도구 (②: ✂ 막으로 자르기, ③: ✎ 도면 편집)

1. **`onPage`**(725-740) 다시 쓰기: `flushCut()` → `cut = p == pCut_`, `edit = p == pEdit_` → `if ((cut || edit) && (!doc_.isLoaded() || !requirePose())) return;` → `!cut && hand_ == "cut"` 또는 `!edit && hand_ == "edit"` 이면 `setHandTool(QString())` → `sheet_->setCutPage(cut)`, `view3d_->setCutPage(cut)`, `view3d_->setEditPage(edit)` → 3D 폭: `edit` 이면 `view3d_->setFixedWidth(330)`, 아니면 `setMinimumWidth(240)`·`setMaximumWidth(QWIDGETSIZE_MAX)` → `cut || edit` 이면 가로 슬롯을 보고 있었을 때 첫 종단면(`firstVerticalSlot()`)으로(지금 732-736 과 같은 동작, 각은 그대로) 뒤 `setHandTool(cut ? "cut" : "edit")` → `if (doc_.poseConfirmed()) refreshAll(false);`(여기서 §3.5 자동 고정).
2. **`setHandTool`**(742-756): `"edit"` 이면 배지 `kEditBadge`, 안내 `kEditHint`(툴팁 같음), `setCutTool(false)`. 리본 상태 `setState("edit", Hand)` 는 위젯이 없어 무해하다.
3. **`refreshAll`**: 591 다음에 `ribbon_->setPageLocked(pEdit_, base, cutpage::kLockTip);`, `onEdit = currentPage() == pEdit_`. 593 의 보임 규칙에 `&& !(onEdit && (w == ctxCutKeys_[0] || w == ctxCutKeys_[1]))`(③ 에서 `← →`·`막 1° · Shift 15°` 숨김 — ③ 에서는 돌지 않는다), `for (auto* w : ctxFixKeys_) w->setVisible(!base && onCut);`, `stFix_->setVisible(!base && (onCut || onEdit));` — 이 셋은 확정 전 분기(595) **앞**. 619 다음 `if (onEdit) setHint(cutpage::kEditStatus);`.
4. **`updateCutInfo`**(623-638, 확정·미리보기 양쪽에서 불림) 끝에 `const QString fx = cutpage::fixText(angleFixed() ? fixedAngle_ : std::nullopt); stFix_->setText(fx); fixedLbl_->setText(fx);`, `ribbon_->setState("fixAngle", angleFixed() ? ToolState::On : ToolState::Idle);`.
5. **입력**: `onAction` — `needPose`(875-876)에 `"fixAngle"`, `"toEdit"`, `"backToCut"` 추가(`cutInput`(853)은 그대로 → 셋 다 먼저 `flushCut`). 분기: `fixAngle` → `fixAngle(); refreshAll(false);` · `toEdit` → `ribbon_->setCurrentPage(pEdit_);` · `backToCut` → `ribbon_->setCurrentPage(pCut_);` · `outline`(929) 의 `== pCut_` → `== pCut_ || == pEdit_`. `keyPressEvent` — 확정 뒤 `switch`(1037-)에 `case Qt::Key_Return: case Qt::Key_Enter: if (ribbon_->currentPage() == pCut_) onAction("fixAngle"); break;`, 1028 의 `if (cutSign != 0)` **앞**에 `if (cutSign != 0 && ribbon_->currentPage() == pEdit_) return;`(③ 에서 `← → , . [ ]` 무시 — `keyReleaseEvent` 의 `keyUp` 은 비활성이면 무해, CutGesture.h:34-37), Esc(1038-1041)의 `== pCut_` → `== pCut_ || == pEdit_`(③ 에서도 홈으로 = 내려놓기). `S`(→ `onAction("cut")`, 927)는 ③ 에서 `hand_ == "edit"` 이라 ② 로 간다 = 다시 자르기.
6. ③ 에서 3D 왼쪽 끌기는 `cutTool_ == false` 라 보는 방향이다(View3D.cpp:180-205 그대로).

### 3.7 3D 보조창 (`View3D.h` 49·55, `View3D.cpp` 265-276)

`void setEditPage(bool on)` + 멤버 `editPage_`. `refresh` 제목: `base ? … : cutPage_ ? k3dTitle : editPage_ ? "<b>" + k3dEditTitle + "</b>" : 기본`. 캔버스 툴팁은 ② 만(그대로).

### 3.8 ctx·상태줄 위젯

- `buildCtxBar`(310-349): `ctxFixKeys_ = {kbdLabel("Enter", w), new QLabel(cutpage::kTileFix, w)}` — `ctxCutKeys_` 다음에 넣고 처음엔 숨김(②·확정 뒤에만 보임, §3.6-3).
- `buildStatusBar`(406-429): `stFix_ = new QLabel(w); stFix_->setObjectName("StFix");` `stDirty_` 다음에 `addWidget`, 처음엔 숨김.

### 3.9 검수용 `--edit` (`studio/src/main.cpp`)

- 사용법 주석(27-33)에 `--edit — 300 ms 에 「도면 편집으로」(toEdit)를 눌러 ③ 도면 편집에서 찍는다(--cut 과 함께면 12° 고정). 예: DongArch3DStudio.exe --sample --cut --edit --shot shot_edit.png`.
- `--cut` 블록(41-49) 다음, `--shot` 블록 **밖**: `if (args.contains("--edit")) QTimer::singleShot(300, &w, [&w] { if (auto* b = w.findChild<QAbstractButton*>("toEdit")) b->click(); });`(`#include <QAbstractButton>`) — 실제 단추 경로(먼저 확정 → 자동 고정 → ③). 확정 계산(≈ 1.4 s, UI 스레드)이 끝난 뒤 1800 ms 캡처 타이머가 돈다(JOB-006 과 같음).

### 3.10 정본 `docs/ui/UI_RULES.md` (사람 검토 1)

- R1 표 1행(20): 탭 줄 `` `파일 · 홈 · ① 바닥 정하기 · ② 막으로 자르기 · ③ 도면 편집 · 내보내기` ``, 잠김 문장을 `` 바닥 확정 전 `②`·`③` 은 숨기지 않고 `Faint` 글자 + 툴팁 「바닥 확정 후 사용」(누르면 ① 로 안내). `③` 은 누르면 지금 막 각도로 고정하고 넘어간다 `` 로.
- R1 표 3행(22) 끝에 `` · ③ 도면 편집 탭 = 「✎ 도면 편집」을 든 상태(막 각도 고정, ← → 로 돌지 않음) ``.
- R1 표 4행(23) 비고에 `— ③ 도면 편집에서는 3D 보조창 330 px(도판을 넓게)`.
- R3(59) 주 단추 목록에 `` ② 막으로 자르기의 `[도면 편집으로]`, `` 추가.
- R5(71) 그리는 순서: `` 방안 눈금 → 유물 음영(실사/깊이/D-Tak) → 기준선(중심선·구연선·바닥선, `OuterInk`) → 외곽선·능선(`OuterInk`) → 단면선(`ProfileRed`) ``. 73 다음에 새 줄 `` - **기준선**(디자인 v2 가이드 §6 · 사용자 도판): 중심선 = 3D 중심축(가는 실선, 구연 위·바닥 아래로 종이 3 mm 연장), 구연선 = 구연 평면(유물 최고점 높이, 폭 = 구경), 바닥선 = 바닥판(높이 0, 폭 = 저경). 출력 굵기 종이 기준 0.10 / 0.13 / 0.25 mm, 화면 1 / 1 / 1.4 px. 「단면·외곽」(L) 표시·내보내기 「외곽선·기준선」 칸과 함께 켜고 끈다. ``
- R6(82): `` SVG는 **레이어 분리**: `단면선` / `외곽선` / 기준선 3층 `중심선`·`구연선`·`바닥선`(id `refline_center`·`refline_rim`·`refline_base`, 층마다 독립) / `능선·문양` / `음영·탁본` / `방안 눈금·치수·축척` — 단위 `mm`, 1:1 실제 치수. ``
- R6(83) 끝에 `` 기준선은 중심선 `01_AXIS_CENTER` · 구연선 `07_PROJECTION_RIM` · 바닥선 `08_PROJECTION_BASE`(셋 다 실선). ``, 그리고 새 줄 `` - 종단면 도판에는 정면 방위각을 적는다 — SVG·PDF·PNG 표제란 「정면 방위각 12° (위에서 보아 반시계)」, DXF `FRONT AZIMUTH 12.0 DEG (CCW, TOP VIEW)`. ``

### 3.11 빌드·시험

- `studio/CMakeLists.txt`: 102 다음(또는 test_studio_side 옆)에 `add_executable(test_studio_reflines tests/test_reflines.cpp)`, `target_link_libraries(test_studio_reflines PRIVATE studio_core)`, `add_test(NAME studio_reflines COMMAND test_studio_reflines ${CMAKE_CURRENT_BINARY_DIR})`. (시간 판정이 없어 RUN_SERIAL 불필요.) CTest 16 → **17**.
- **`studio/tests/test_reflines.cpp`**(신규, test_side.cpp 처럼 `QGuiApplication` 오프스크린, 출력 폴더 = `argv[1]`): 순서 — 예제(`loadSample()`) 0° Right 에서 기준 1·2·5(0°·슬롯 1) → 내보내기: **기본 요청 `R0` = `ExportRequest{}`(A4 세로·1:1·`opts` 기본값 — grid·shade·outline·section·labels 켬, hatch 끔, slot 0) + 제목 `기준선 시험`** 으로 `studio_reflines_s1.svg`(`R0`)·`_s3.svg`(`R0` 에 scale 1/3)·`_L.svg`(`R0`, Left — 뒤 Right 로)·`_h.svg`(`R0` 에 `opts.slot = 1`)·`studio_reflines.dxf`(`R0`) → 기준 6·7·8(0°) → 래스터(기준 9) → `setFrontAngle(12)` → **`_12.svg`·`_12.dxf` 도 `R0` 그대로**(A4 세로 1:1 — 파이썬 기준 10 이 s = 1 로 판정) → 기준 5·7·8(12°) → `setFrontAngle(-20)` → 기준 5(−20°)·`studio_reflines_m20.dxf`(`R0`) → `setFrontAngle(-0.04)` → 기준 5(−0.04°)·`studio_reflines_m004.dxf`(`R0`) → 미스캔 예제(기준 3) → 빈 문서·확정 전(기준 4) → 마지막 줄 `PASS`/`FAILED`. SVG 는 `QXmlStreamReader` 나 정규식(test_side.cpp:104-128 방식) — builder 재량. **판정 꼬리표·인쇄는 `[J7-n]`**(n = 이 명세의 수용 기준 번호 — test_cutpage 의 JOB-006 꼬리표 `[1]`-`[17]` 과 섞이지 않게). 인쇄 줄: `[J7-1] sample H=… rim=… base=… outline ends=(x,y)…(x,y)`, `[J7-3] blind H=… rim=… base=…`, `[J7-6]`·`[J7-8]`·`[J7-9]` 판정 요약.
- **`studio/tests/test_cutpage.cpp`**(JOB-006 시험 — 바뀐 동작에 맞춰 고친다): [8] 기대 탭(184-185)의 `측정·탁본` → `③ 도면 편집` 과 ③ 잠김 확인 추가(기준 12), [15] 의 `tb[4]->click()` + `측정·탁본 → CtxBar 숨김`(351-352) → `tb[5]`(내보내기) 클릭(기준 20). [17] 끝(371 `w.close()` 앞)에 기준 13-19 를 차례로 덧붙인다(같은 실행·같은 창). JOB-006 의 나머지 판정([1]-[7]·[9]-[14]·[16]-[17])은 고치지 않고 그대로 참이어야 한다. **꼬리표**: 고치는 JOB-006 판정 줄은 `[8]`·`[15]` 를 그대로 두고, 새로 넣는 판정의 `CHECK`·`eq` 꼬리표와 인쇄는 모두 `[J7-12]` … `[J7-20]`(같은 출력에 JOB-006 의 `[13] 단면 위치 x=`(315)·`[15] …`(341-359)가 있어서). 그룹 이름 목록 판정용 작은 함수 `QStringList groupNames(QWidget* page)` = `page->findChildren<QLabel*>()` 중 `gname` 속성이 참인 것의 글자(찾은 순서 = 만든 순서 = 왼쪽→오른쪽 — `Ribbon::addGroup`(Ribbon.cpp:311-323)이 그룹을 차례로 붙이고 그룹 이름 라벨이 그룹의 첫 자식(155-162)).

## 4. 수용 기준 (각 항목은 명령 하나로 참/거짓)

허용오차 근거: 좌표는 계산을 바꾸지 않고 옮겨 그리는 값이라 도면 mm 는 **1e-9**(같은 식), SVG 는 `n3()` 소수 3자리 반올림 두 번 → **±0.002 mm**(길이 ±0.003), DXF 는 `setprecision(6)` → **±2e-6 mm**, 기고 H 는 `dims().height` 가 0.1 mm 반올림이라 **±0.05 mm**(종이 ×s, 판정 0.06·s). 래스터는 3×3 화소 창에서 비흰(min(R,G,B) < 245, test_side.cpp:197 과 같음) 유무. 화면 문자열은 정확히 같음. 기다림 상한 15 s 는 실패 감지용(판정값 아님).

### A. 새 CTest `studio_reflines` (`studio/tests/test_reflines.cpp`)

1. **기하(예제)**: `loadSample()` 뒤 `H = mesh().bboxMax.z − mesh().bboxMin.z`: `|bboxMin.z| ≤ 1e-9`, `|H − dims().height| ≤ 0.05`; `plateRefLines(d, 3.0)` 의 center·rim·base 가 모두 있고 center = (0,−3)→(0,H+3), rim = (−rim/2,H)→(rim/2,H), base = (−base/2,0)→(base/2,0)(rim·base = `dims().rim`·`dims().base`, 좌표마다 1e-9); `plateRefLines(d, 9.0).center` = (0,−9)→(0,H+9).
2. **좌/우**: `setSectionSide(Left)` 뒤 `plateRefLines(d, 3.0)` 의 여섯 점이 Right 와 `==` 로 같다.
3. **미스캔 예제**: `loadArtecBlindSample()` 뒤 center·rim 이 기준 1 과 같은 관계이고 `rim.a.x == −dims().rim/2`(1e-9, 기대 −62.0); base 는 `dims().base > 0` 이면 있고 기준 1 관계, 0 이면 없음. `[J7-3]` 인쇄.
4. **확정 전·빈 문서**: `ArtifactDoc{}` 와 `loadSample(false)` 둘 다 center·rim·base 가 모두 없고(`!has_value()`) `azimuthNote(d, 0) == ""`.
5. **정면 방위각 문구**: 예제 0°: `azimuthNote(d,0) == "정면 방위각 0° (위에서 보아 반시계)"`, `azimuthNote(d,1) == ""`(B–B′ 가로); `setFrontAngle(12)` 뒤 `== "정면 방위각 12° (위에서 보아 반시계)"`; `setFrontAngle(-20)` 뒤 `== "정면 방위각 −20° (위에서 보아 반시계)"`(U+2212); `setFrontAngle(-0.04)` 뒤 `== "정면 방위각 0° (위에서 보아 반시계)"`(`−0°` 아님).
6. **SVG 층**: `_s1`(s = 1)·`_s3`(s = 1/3)·`_L`(s = 1) 각각 — 최상위 `<g>` id 가 정확히 `[grid_dims_scale, shade_dtak, refline_center, refline_rim, refline_base, outline_axis, section_profile]`; `refline_*` 층마다 `<line` 정확히 1개, 레이블 `중심선`/`구연선`/`바닥선`, stroke `#141413`, stroke-width 0.10/0.13/0.25(1e-9), dasharray 없음; `L = layoutPaper(d, 0, A4Portrait, s)`, `E = Plate::extentMm(d, 0)`, `X0 = L.plateRectMm.left() − E.left()·s`, `Yb = L.plateRectMm.top() + E.bottom()·s` 로 center x1 = x2 = X0·{y1,y2} = {Yb+3, Yb−H·s−3}, rim y = Yb−H·s·{x} = {X0∓rim·s/2}, base y = Yb·{x} = {X0∓base·s/2} — 모두 ±0.002; `outline_axis` 층에 `<line` 이 하나도 없음(중심선은 옮겼고 닫는 가로선은 구연선·바닥선이 있어 뺌 — `<polyline>` 1개만); `grid_dims_scale` 층에 stroke `#141413` 이고 y1 = y2 = Yb 인 선 없음; 파일에 `6 1 1 1` 없음. `_h.svg` 최상위 id 정확히 `[grid_dims_scale, section_profile]`.
7. **SVG 정면 방위각**: `_s1.svg` 에 `정면 방위각 0° (위에서 보아 반시계)` 있음, `_12.svg` 에 `정면 방위각 12° (위에서 보아 반시계)` 있음, `_h.svg` 에 `정면 방위각` 없음.
8. **DXF**: `studio_reflines.dxf`(0°)·`_12.dxf`(12°) 각각 — LAYER 레코드가 정확히 12개·이름 순서 `0, 01_AXIS_CENTER … 11_SCALE_BAR`(지금과 같음); `01_AXIS_CENTER`·`07_PROJECTION_RIM`·`08_PROJECTION_BASE` 는 ACI 7·`CONTINUOUS`; 세 층에 LINE 이 정확히 1개씩·코드 6 없음; `02_EXTERIOR_OUTLINE` 에 LINE 없음(POLYLINE 만); 01 = (0,−3)→(0,H+3), 07 = (−rim/2,H)→(rim/2,H), 08 = (−base/2,0)→(base/2,0) ±2e-6; `10_ANNOTATION_TEXT` TEXT 중 값이 `FRONT AZIMUTH 0.0 DEG (CCW, TOP VIEW)`(0°)·`FRONT AZIMUTH 12.0 DEG (CCW, TOP VIEW)`(12°)인 것이 정확히 1개. 그리고 `_m20.dxf` 에 `FRONT AZIMUTH -20.0 DEG (CCW, TOP VIEW)`, `_m004.dxf`(−0.04°)에 `FRONT AZIMUTH 0.0 DEG (CCW, TOP VIEW)` 인 TEXT 가 정확히 1개씩이고, 네 DXF 모두 `FRONT AZIMUTH` 로 시작하는 TEXT 는 그 하나뿐이다(`FRONT AZIMUTH -0.0 …` 없음).
9. **래스터(= PDF·PNG 길)**: `paintSheet`(예제 0°·Right·A4 세로, opts `grid = shade = section = labels = false`, `outline = true`)를 8 px/mm QImage 에. 도면점 (x,y) → 화소 ((X0 + x·s)·8, (Yb − y·s)·8), `ink(x,y)` = 그 화소 둘레 3×3 에 비흰 화소가 있음. s = 1: `ink(rim/2−2, H) ∧ ¬ink(rim/2+3, H) ∧ ink(base/2−2, 0) ∧ ¬ink(base/2+3, 0) ∧ ink(0, H+1.5) ∧ ¬ink(0, H+5) ∧ ink(0, −1.5) ∧ ¬ink(0, −5)`. s = 1/3(같은 종이 mm): `ink(rim/2−6, H) ∧ ¬ink(rim/2+9, H) ∧ ink(0, H+4.5) ∧ ¬ink(0, H+15)`. **머리표(§3.3-4)**: 같은 장(s = 1)을 `labels = true` 로 한 장 더 — `caution(x,y)` = 그 화소 둘레 3×3 에 R − B > 40 인 화소가 있음(Caution `#7A5A00` 은 R − B = 122, 흰색에 반쯤 섞여도 ≈ 61, 먹·흰색·Edge 회색은 ≤ 12)일 때 `¬caution(−rim/2+3, H) ∧ ¬caution(−rim/4, H) ∧ ¬caution(−3, H) ∧ ink(−rim/4, H) ∧ caution(−rim/2−6, H)`(구연선 위에 갈색 파선·라벨 상자가 없고, 파선은 구연선 바깥에만 남음). 그리고 `test_studio_reflines` 마지막 줄 `PASS`, 종료 코드 0.

### B. 독립 파이썬 판정 (§5-3 명령, `B` 에 시험이 남긴 파일)

10. SVG 판정 스크립트가 `PY-SVG OK` 를 찍고 종료 코드 0 (`_s1`·`_s3`·`_L`·`_12`·`_h` — 층 순서, 층마다 선 하나·레이블·굵기·색, 중심선 위·아래 종이 3.000 ± 0.002 mm 연장, 구연선·바닥선 가운데 = 중심선 x, 바닥선 − 구연선 = 184.8·s ± 0.06·s, 구연선 길이 142.0·s ± 0.003, 바닥선 길이 142.0·s ± 0.003(예제), Left = Right 좌표, 외곽선 층 `<line>` 없음(polyline 1개)·방안 층 먹 바닥선 없음, `6 1 1 1` 없음, 정면 방위각 문구).
11. DXF 판정 스크립트가 `PY-DXF OK` 를 찍고 종료 코드 0 (`studio_reflines.dxf`·`_12.dxf` — 층 12개 이름 순서, 01·07·08 ACI 7·CONTINUOUS, 선 하나씩·코드 6 없음, 02 층 LINE 없음, 중심선 x 0·아래 끝 −3, H = 위 끝 − 3 = 184.8 ± 0.06, 구연선 y = H·좌우 대칭·길이 142.0(0°), 바닥선 y = 0·대칭·길이 142.0(0°), 방위각 TEXT 정확히 1개).

### C. `studio_cutpage` 확장 (② 고정·넘기기·③ 도면 편집 — 실제 MainWindow, 오프스크린, JOB-006 [8]-[17] 에 이어 한 실행)

새 판정의 꼬리표·인쇄는 `[J7-12]` … `[J7-20]`(§3.11 — JOB-006 의 `[13]`·`[15]` 와 구별). `groupNames(page)` = 그 페이지 안 그룹 이름 글자 목록(왼쪽→오른쪽, §3.11).

12. **탭·잠김(JOB-006 [8] 갱신)**: 확정 전 탭 글자가 차례로 정확히 `파일`·`홈`·`① 바닥 정하기`·`② 막으로 자르기`·`③ 도면 편집`·`내보내기`; ③ 탭(번호 4) `locked == true`·툴팁에 `바닥 확정 후 사용`·글자색 `theme::Faint`; ③ 탭 `click()` → 현재 페이지 = ①(2), `StHint` 에 `먼저 ① 바닥 정하기에서`. 확정(Enter, [9]) 뒤 ③ 탭 `locked == false`·툴팁 비어 있음. (탭 글자 판정은 JOB-006 `[8]` 줄을 고치고, ③ 잠김·풀림 판정은 `[J7-12]`.)
13. **② 고정 · 넘기기**([17] 끝 = ②·105°·고정 전): `fixText(std::nullopt) == "막 고정 전"`, `fixText(12.0) == "막 12° 고정"`, `fixText(-20.0) == "막 −20° 고정"`, `slotMemo(12.0) == "막 12°"`, `kStatus == "⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 「지금 각도 고정」 → 「도면 편집으로」"`; ② 페이지(리본 `QStackedWidget` 의 `widget(3)`)의 `groupNames` 가 **정확히 5개이고 마지막이 `고정 · 넘기기`**; Tile `fixAngle` 글자 `지금 각도 고정`·`Idle`, Tile `toEdit` 글자 `도면 편집으로`·`Primary`, 두 타일 `sizeHint().height()` = 홈 `cut` 타일과 같음; `widget(3)` 안 Tile 중 `Primary` 는 정확히 1개; `widget(3)->sizeHint().height() <= widget(2)->sizeHint().height()`; `StFix` 보임·`== "막 고정 전"`; `StHint == kStatus`; CtxBar 안 QLabel `지금 각도 고정` 보임. 인쇄 `[J7-13] window minimumSizeHint width=…` 이고 그 값 ≤ 1845.
14. **고정·풀림**: `Key_Return` → `StFix == "막 105° 고정"`, `fixAngle` `On`, SlotList 안에 글자 `막 105° 고정` 인 QLabel 있음; `tap(→)` → `CtxRight` 가 `막 106° · 끄는 동안 ` 으로 시작할 때까지 기다림(= 놓은 뒤 원본 계산·`refreshAll` 끝) → `StFix == "막 고정 전"`·`fixAngle` `Idle`, SlotList 안에 글자 `막 105° 고정` 인 QLabel **없음**·글자 `막 105°` 인 QLabel 있음(A–A′ 에 저장된 고정 각 — 고정 표시만 풀림); `Ctrl+Z` → `SheetTitle == "A–A′ 실시간 단면 — 105°"`·`StFix == "막 고정 전"`(되돌리기도 각 변화); `fixAngle` 타일 `click()` → `StFix == "막 105° 고정"`.
15. **③ 화면**: `toEdit` 타일 `click()` → `currentPage() == 4`, `ToolBadge == "✎ 도면 편집"`, `CtxHint == kEditHint` 이고 툴팁도 같음, CtxBar 보임, `StHint == kEditStatus`, `StFix == "막 105° 고정"`, `View3DTitle` 에 `3D — 고정된 막` 포함, `SheetTitle == "A–A′ 정면 실측도"`, `CtxRight` 가 `점 ` 으로 시작, CtxBar 안 QLabel `막 1° · Shift 15°`·`지금 각도 고정` 은 숨김이고 `좌/우 반전` 은 보임, `View3DCanvas` 툴팁 비어 있음, `settle()` 뒤 `View3DPane` `width() == 330`; ③ 페이지 `groupNames(stack->widget(4))` **== [`파일`, `고정된 막`, `선 다듬기 · 간략화`, `도판 표시 (겹침)`, `내보내기`]**(5개·이 순서 — 리본 전체가 아니라 ③ 페이지 안에서), `widget(4)` 안 `FixedAngle` 라벨 `== "막 105° 고정"`, `widget(4)` 안 `backToCut` 단추 글자 `↩ 3D로 돌아가 다시 자르기`; `widget(4)` 안 Tile 중 `Primary` 정확히 1개이고 그 objectName `export`; `widget(4)->sizeHint().height() <= widget(2)->sizeHint().height()`; 인쇄 `[J7-15] window minimumSizeHint width=…` ≤ 1845.
16. **③ 에서는 막이 돌지 않음**: ③ 에서 `tap(→)`·`tap(.)`·`tap(])` → `settle()` 5번 → `StFix == "막 105° 고정"`, `CtxRight` 가 `점 ` 으로 시작; `img0 = View3DCanvas grab()` → 왼쪽 단추 press (100,100)·move (140,120)·release → `settle()` 뒤 `grab() != img0`(보는 방향) 이고 `StFix == "막 105° 고정"`.
17. **다시 자르기**: `backToCut` `click()` → `currentPage() == 3`, `ToolBadge == "✂ 막으로 자르기"`, `SheetTitle == "A–A′ 실시간 단면 — 105°"`, `StFix == "막 105° 고정"`, `settle()` 뒤 `View3DPane` `width() > 330`. ③ 탭 `click()` → 4. `Key_Escape` → `currentPage() == 1`, CtxBar 숨김, `StFix` 숨김.
18. **들어가면 자동 고정·슬롯이 기억**: `Key_S` → 3; `tap(←)` → `CtxRight` 가 `막 104° · 끄는 동안 ` 으로 시작할 때까지 → `StFix == "막 고정 전"`; ③ 탭 `click()` → `StFix == "막 104° 고정"`, SlotList 안 QLabel `막 104° 고정` 있음; `SlotList` `setCurrentRow(1)` → `SheetTitle == "B–B′ 구연부 횡단면"`; `setCurrentRow(0)` → `SheetTitle == "A–A′ 정면 실측도"`·`StFix == "막 104° 고정"`; `Key_S` → `SheetTitle == "A–A′ 실시간 단면 — 104°"`. **② 에서 가로 슬롯을 보며 고정**(§3.5 의 `k` = 첫 종단면): `tap(←)` → `CtxRight` 가 `막 103° · 끄는 동안 ` 으로 시작할 때까지 → `SlotList` `setCurrentRow(1)` → `SheetTitle == "B–B′ 구연부 횡단면"` → `Key_Return` → `StFix == "막 103° 고정"` → `toEdit` 타일 `click()` → `currentPage() == 4`·`SheetTitle == "A–A′ 정면 실측도"` → `setCurrentRow(1)` → `setCurrentRow(0)` → `StFix == "막 103° 고정"`(104° 로 되돌아가 다시 고정되지 않음)·SlotList 안 QLabel `막 103° 고정` 있음 → `Key_S` → `SheetTitle == "A–A′ 실시간 단면 — 103°"`.
19. **새 문서 = 고정 풀림**: `w.openSample(false)` → `Key_S` → `Key_Return` → `StFix == "막 0° 고정"`; 다시 `w.openSample(false)` → `Key_S` → `StFix == "막 고정 전"`.
20. **JOB-006 [15] 갱신·끝**: [15] 의 탭 클릭은 `내보내기`(번호 5)로 → CtxBar 숨김. `test_studio_cutpage` 마지막 줄 `PASS`, 종료 코드 0.

### D. 회귀

21. CTest 17종(기존 16 + `studio_reflines`): `B` 에서 `ctest --output-on-failure` 출력에 `100% tests passed` 와 `out of 17` 이 같은 줄에 있고, `tests failed` 가 0 이 아닌 줄이 없다.
22. 계산 값 그대로(verify.ps1 직후 `B\Testing\Temporary\LastTest.log`): `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180"` ≥ 1, `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"` ≥ 1, `grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"` ≥ 1 (기준 `A6/LastTest_verify.log`).
23. verify.ps1 종료 코드 0, `B\verify-summary.md` 마지막 줄 `결과: PASS`, 빌드 경고 0: `grep -ci "warning" D:/_dongarch_build/verify_job007/studio-logs/02_build.log` = `0`.
24. 엔진·기존 시험 무변경: `git diff --name-only 688b7bd -- studio/src/ArtifactDoc.h studio/src/ArtifactDoc.cpp studio/src/CutGesture.h studio/src/MeshIO.h studio/src/MeshIO.cpp studio/src/MeshRaster.h studio/src/MeshRaster.cpp studio/src/Theme.h studio/tests/test_pipeline.cpp studio/tests/test_pose.cpp studio/tests/test_side.cpp studio/tests/test_cut.cpp GigaMesh` → 출력 없음.

### E. 코드·정본 (Git Bash, `cd W`. **builder 가 커밋한 뒤** `git status --porcelain` 출력이 없는 상태에서)

25. 새 문자열 있음: `for s in "③ 도면 편집" "고정 · 넘기기" "지금 각도 고정" "도면 편집으로" "✎ 도면 편집" "단면이 기준선 위에 놓였습니다" "⑧ 치수 확인 — 치수 띠를 보고" "3D — 고정된 막" "↩ 3D로 돌아가 다시 자르기" "고정된 막" "막 고정 전" "「지금 각도 고정」 → 「도면 편집으로」" "정면 방위각 " " (위에서 보아 반시계)" "FRONT AZIMUTH" "refline_center" "refline_rim" "refline_base" "외곽선·기준선" "기준선(먹)" "기준선: 중심선 01"; do grep -rqF "$s" studio/src || echo "MISSING $s"; done` → 출력 없음.
26. 옛 것 없음: `grep -rnF "측정·탁본" studio/src` → 출력 없음, `grep -rnF "외곽선·중심선" studio/src` → 출력 없음, `grep -rnF "6 1 1 1" studio/src` → 출력 없음, `grep -cF '다음: 「도면」 내보내기 (Ctrl+P)"' studio/src/CutPageText.h` = `1`(③ 상태줄 하나만 — ② `kStatus` 는 원문으로 돌아감).
27. R2·R3: `git diff 688b7bd -- studio/src | grep -E '^\+.*(QColor\([0-9]|qRgba?\([0-9]|Qt::(red|green|blue|yellow|white|black|gray|darkGray|lightGray)\b)'` → 출력 없음; hex 는 줄 단위가 아니라 **개수·값**으로(기존 줄을 다시 배치해도 거짓 FAIL 이 나지 않게): `grep -rhoE '#[0-9A-Fa-f]{6}' studio/src | wc -l` ≤ `19`(688b7bd 의 21 − 지운 방안 먹 바닥선 Plate.cpp:603 의 1 − 외곽선 칸 글 ExportDialog.cpp:125 `먹색 #141413` 의 1(§3.3-7 새 글은 hex 없음), 기준값은 `git grep -h -o -E '#[0-9A-Fa-f]{6}' 688b7bd -- studio/src | wc -l` = 21) 이고 `comm -13 <(git grep -h -o -E '#[0-9A-Fa-f]{6}' 688b7bd -- studio/src | sort -u) <(grep -rhoE '#[0-9A-Fa-f]{6}' studio/src | sort -u)` → 출력 없음(새 hex 값 없음), `grep -rniE "c8641e|3f8f4f|3f6fb5|6b4fc0" studio/src` → 출력 없음, `git diff 688b7bd -- studio/src | grep '^\+.*ToolState::Primary' | grep -vc toEdit` = `0`, `grep -c '"toEdit", ToolState::Primary' studio/src/MainWindow.cpp` = `1`.
28. 흐름 코드: `grep -c "plateRefLines(" studio/src/Plate.cpp` ≥ `4`(정의·화면·SVG·DXF), `grep -c "azimuthNote(" studio/src/Plate.cpp` ≥ `3`(정의·SVG·paintSheet), `grep -c "CutPageText.h" studio/src/Plate.cpp` = `0`(도판 엔진이 ② 페이지 문자열 헤더에 기대지 않음 — §3.3-3), `grep -c "paperPxPerMm" studio/src/Plate.h` ≥ `1`, `grep -c "slotMemo(" studio/src/MainWindow.cpp` ≥ `1`, `grep -c "firstVerticalSlot()" studio/src/MainWindow.cpp` ≥ `2`(onPage·fixAngle 에서 부름), `grep -c '"--edit"' studio/src/main.cpp` ≥ `1`, `grep -c "setEditPage" studio/src/View3D.h` = `1`.
29. 정본: `grep -c "① 바닥 정하기 · ② 막으로 자르기 · ③ 도면 편집 · 내보내기" docs/ui/UI_RULES.md` = `1`, `grep -c "측정·탁본" docs/ui/UI_RULES.md` = `0`, `grep -c "✎ 도면 편집" docs/ui/UI_RULES.md` ≥ `1`, `grep -cF '`[도면 편집으로]`' docs/ui/UI_RULES.md` ≥ `1`, `grep -c "330 px" docs/ui/UI_RULES.md` ≥ `1`, `grep -c "기준선(중심선·구연선·바닥선" docs/ui/UI_RULES.md` ≥ `1`, `grep -c "refline_center" docs/ui/UI_RULES.md` ≥ `1`, `grep -c "08_PROJECTION_BASE" docs/ui/UI_RULES.md` ≥ `1`, `grep -c "정면 방위각" docs/ui/UI_RULES.md` ≥ `1`.
30. 변경 범위(허용 목록): `git status --porcelain` → 출력 없음, 그리고 `git diff --name-only 688b7bd HEAD | grep -vxF -e studio/src/CutPageText.h -e studio/src/Plate.h -e studio/src/Plate.cpp -e studio/src/ExportDialog.cpp -e studio/src/SheetView.cpp -e studio/src/Ribbon.cpp -e studio/src/MainWindow.h -e studio/src/MainWindow.cpp -e studio/src/View3D.h -e studio/src/View3D.cpp -e studio/src/main.cpp -e studio/tests/test_reflines.cpp -e studio/tests/test_cutpage.cpp -e studio/CMakeLists.txt -e docs/ui/UI_RULES.md` → 출력 없음.

### F. 캡처 (31 은 명령 판정, 32-35 는 ui-reviewer 예/아니오 — 실제 글꼴 캡처는 §5-4)

31. 시작 화면은 바꾸지 않으므로 `shot_home.png` 가 JOB-006 과 바이트까지 같다: PowerShell `(Get-FileHash "D:\_dongarch_build\verify_job007\studio\shots\shot_home.png").Hash -eq (Get-FileHash "D:\_dongarch_build\wt\factory\jobs\JOB-006\artifacts\shot_home.png").Hash` → `True` (사용자 QSettings 최근 목록이 바뀌었으면 코드와 무관하게 다를 수 있다 — 다르면 FAIL 로 두고 원인을 decision.md 에, JOB-006 §7 과 같음).
32. verify 의 `shot_main.png`(오프스크린) > 5120 바이트이고 `A6/shot_main.png` 와 나란히 보아 다른 곳이 (a) 5번째 탭 글자(`측정·탁본` → `③ 도면 편집`) (b) 도판의 기준선 — 구연선이 유물 위 끝에서 외면·단면 양쪽 반을 가로지름, 바닥선이 저경 폭만(예전 도판 전체 폭 먹선 없음), 중심선이 바닥 아래·구연 위로 짧게만 나옴, 외면 쪽 외곽선 위·아래 끝의 닫는 가로선이 없어져 외곽선 끝과 구연선·바닥선 사이에 3 mm 남짓 틈(알려진 실루엣 끝 — 꼭짓점 z 2·98 백분위, 사람 검토 6) (c) 치수 띠 오른쪽 `그리는 순서` 글이 `… 탁본음영 → 기준선(먹) → 외곽선(먹) …` 으로 길어짐(오프스크린 □ 글꼴이라도 글자 수가 늘어 띠 오른쪽 모양이 바뀜, §3.3-7) (d) 「기고 H 184.8 mm」 머리표 — 라벨 상자가 구연선 바로 위로 올라가고 갈색 파선은 도판 왼쪽 끝에서 구연선 바깥 끝 2 mm 앞까지만(구연선 위에 파선·상자 없음, §3.3-4) — 뿐이다(리본·작업 공간·상태줄 위치·높이, 치수 띠 **왼쪽 `dims_` 의 5대 치수 값**, 3D 같음).
33. `shot_cut_font.png`(② 12°, 실제 글꼴): 리본 그룹 5개 중 마지막 「고정 · 넘기기」 에 타일 2개(`지금 각도 고정` 대기, `도면 편집으로` 빨강 — 화면에서 빨강 타일은 이것 하나), 리본 높이가 `A6/shot_cut.png` 와 같음, ctx 줄에 `Enter` 배지 + `지금 각도 고정`, 상태줄 왼쪽 `⑥ 단면 보기 — … 「지금 각도 고정」 → 「도면 편집으로」`·오른쪽 끝 `막 고정 전`, 도판에 기준선 셋과 머리표(32 (b)·(d) 와 같은 모양), 치수 띠 오른쪽 `그리는 순서: … → 기준선(먹) → …`, 머리 `A–A′ 실시간 단면 — 12°`.
34. `shot_edit_font.png`(③ 12°, 실제 글꼴): 현재 탭 5번째 `③ 도면 편집`, 리본 그룹이 차례로 「파일」「고정된 막」「선 다듬기 · 간략화」「도판 표시 (겹침)」「내보내기」 **5개**이고 빨강은 `도면` 하나, 「고정된 막」 그룹에 `막 12° 고정`(위)과 [↩ 3D로 돌아가 다시 자르기](아래), ctx 왼쪽 어두운 배지 `✎ 도면 편집` + `단면이 기준선 위에 놓였습니다 · …`, 3D 보조창이 33 보다 좁고(≈ 330 px) 머리 `3D — 고정된 막`, 도판 머리 `A–A′ 정면 실측도`, 치수 띠 오른쪽 `그리는 순서: … → 기준선(먹) → …`, 상태줄 `⑧ 치수 확인 — …`·오른쪽 `막 12° 고정`, 슬롯 목록 A–A′ 메모 `막 12° 고정`, 도판 단면이 33 과 같은 각(12°) 모양.
35. `shot_export_font.png`(`--sample --export`, 실제 글꼴): 오른쪽 칸 글 `외곽선·기준선 Layer (먹색 · 외곽선 0.50 · 기준선 0.10~0.25 mm)`, 미리보기 도판에 기준선 셋, 미리보기 표제란 첫 줄 끝 `정면 방위각 0° (위에서 보아 반시계)`(작아서 안 읽히면 "판독 불가"로 적고 기준 7 로 갈음).

## 5. 검증 명령

1. 표준 검증(작업 폴더 `D:\_dongarch_build\wt`): `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-007 -BuildRoot D:\_dongarch_build\verify_job007 -Job JOB-007 -RecordRoot D:\_dongarch_build\wt` (로그 `L\01_configure.log … 04_shots.log`, 산출물 `D:/_dongarch_build/wt/factory/jobs/JOB-007/artifacts/`). 직후 기준 21·22·23·31·32. 이 PC 의 CTest 는 실패가 없으면 `100% tests passed out of N` 만 찍는다.
2. 개별 시험(PowerShell, `--test-dir` 금지 — 빌드 폴더로 이동): `Remove-Item Env:\CI -ErrorAction SilentlyContinue; $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job007\studio; ctest -R "studio_reflines|studio_cutpage" -V` → 기준 1-9·12-20(`[J7-1]`·`[J7-3]`·`[J7-13]`·`[J7-15]` 인쇄, 두 시험 마지막 `PASS`; 실패 줄은 `FAIL … [J7-n] …` 로 어느 기준인지 바로 보이고 JOB-006 판정 실패는 `[n]`). 개별 실행은 LastTest.log 를 덮어쓰므로 기준 22 는 1 직후 또는 전체 `ctest --output-on-failure` 재실행 뒤에 본다.
3. 독립 파이썬 판정(Git Bash, 2 또는 1 뒤 — 파일은 `B` 에 있다). 아래 두 블록은 0열(들여쓰기 없음)에서 그대로 붙여 넣는다 — 기준 10:
```bash
"/c/Users/kyi25/AppData/Local/Programs/Python/Python312/python.exe" -I - "D:/_dongarch_build/verify_job007/studio" <<'PY'
# -*- coding: utf-8 -*-
import os, sys, xml.etree.ElementTree as ET
B = sys.argv[1]; S = '{http://www.w3.org/2000/svg}'; LBL = '{http://www.inkscape.org/namespaces/inkscape}label'
ORDER = ['grid_dims_scale', 'shade_dtak', 'refline_center', 'refline_rim', 'refline_base', 'outline_axis', 'section_profile']
P = lambda n: os.path.join(B, n); near = lambda a, b, t: abs(a - b) <= t; f = lambda e, k: float(e.get(k))
top = lambda p: [g for g in ET.parse(p).getroot() if g.tag == S + 'g']
txt = lambda n: open(P(n), encoding='utf-8').read()
def svg(n, s, H, rim, base):
    gs = top(P(n)); ids = [g.get('id') for g in gs]; assert ids == ORDER, (n, ids); g = dict(zip(ids, gs)); L = {}
    for k, lab, w in (('center', '중심선', 0.10), ('rim', '구연선', 0.13), ('base', '바닥선', 0.25)):
        kids = list(g['refline_' + k]); assert g['refline_' + k].get(LBL) == lab and len(kids) == 1 and kids[0].tag == S + 'line', (n, k)
        e = kids[0]; assert e.get('stroke', '').upper() == '#141413' and near(f(e, 'stroke-width'), w, 1e-9) and e.get('stroke-dasharray') is None, (n, k, e.attrib)
        L[k] = [f(e, a) for a in ('x1', 'y1', 'x2', 'y2')]
    cx1, cy1, cx2, cy2 = L['center']; rx1, ry1, rx2, ry2 = L['rim']; bx1, by1, bx2, by2 = L['base']; ct, cb = min(cy1, cy2), max(cy1, cy2)
    assert near(cx1, cx2, 1e-3) and near(ry1, ry2, 1e-3) and near(by1, by2, 1e-3), (n, L)
    assert near((rx1 + rx2) / 2, cx1, 0.002) and near((bx1 + bx2) / 2, cx1, 0.002), (n, L)
    assert near(ry1 - ct, 3.0, 0.002) and near(cb - by1, 3.0, 0.002), (n, ry1 - ct, cb - by1)
    assert near(by1 - ry1, H * s, 0.06 * s), (n, by1 - ry1)
    if rim: assert near(abs(rx2 - rx1), rim * s, 0.003), (n, abs(rx2 - rx1))
    if base: assert near(abs(bx2 - bx1), base * s, 0.003), (n, abs(bx2 - bx1))
    assert len(list(g['outline_axis'].iter(S + 'line'))) == 0 and len(list(g['outline_axis'].iter(S + 'polyline'))) == 1, (n, 'outline_axis 에 line 이 남음')
    for e in g['grid_dims_scale'].iter(S + 'line'):
        assert not (e.get('stroke', '').upper() == '#141413' and near(f(e, 'y1'), by1, 0.002) and near(f(e, 'y2'), by1, 0.002)), (n, '방안 먹 바닥선', e.attrib)
    return L
a = svg('studio_reflines_s1.svg', 1.0, 184.8, 142.0, 142.0)
svg('studio_reflines_s3.svg', 1 / 3, 184.8, 142.0, 142.0)
l = svg('studio_reflines_L.svg', 1.0, 184.8, 142.0, 142.0)
assert all(near(x, y, 1e-3) for k in a for x, y in zip(a[k], l[k])), ('Left != Right', a, l)
svg('studio_reflines_12.svg', 1.0, 184.8, None, None)
assert [g.get('id') for g in top(P('studio_reflines_h.svg'))] == ['grid_dims_scale', 'section_profile']
for n in ('studio_reflines_s1.svg', 'studio_reflines_s3.svg', 'studio_reflines_L.svg', 'studio_reflines_12.svg', 'studio_reflines_h.svg'):
    assert '6 1 1 1' not in txt(n), n
assert '정면 방위각 0° (위에서 보아 반시계)' in txt('studio_reflines_s1.svg')
assert '정면 방위각 12° (위에서 보아 반시계)' in txt('studio_reflines_12.svg')
assert '정면 방위각' not in txt('studio_reflines_h.svg')
print('PY-SVG OK')
PY
```
기준 11:
```bash
"/c/Users/kyi25/AppData/Local/Programs/Python/Python312/python.exe" -I - "D:/_dongarch_build/verify_job007/studio" <<'PY'
import os, sys
B = sys.argv[1]; near = lambda a, b, t: abs(a - b) <= t
NAMES = ['0', '01_AXIS_CENTER', '02_EXTERIOR_OUTLINE', '03_EXTERIOR_FEATURES', '04_SECTION_WALL', '05_SECTION_HATCH', '06_INTERIOR_OUTLINE', '07_PROJECTION_RIM', '08_PROJECTION_BASE', '09_DIMENSION_MEASURE', '10_ANNOTATION_TEXT', '11_SCALE_BAR']
def recs(p):
    L = open(p, encoding='latin-1').read().split('\n'); out = []
    for i in range(0, len(L) - 1, 2):
        c, v = int(L[i].strip()), L[i + 1].strip()
        if c == 0: out.append((v, []))
        elif out: out[-1][1].append((c, v))
    return out
def g1(r, c):
    v = [x for k, x in r[1] if k == c]; return v[0] if v else None
def dxf(n, H, rim, base, az):
    R = recs(os.path.join(B, n)); lay = [r for r in R if r[0] == 'LAYER']
    assert [g1(r, 2) for r in lay] == NAMES, (n, [g1(r, 2) for r in lay])
    for r in lay:
        if g1(r, 2) in ('01_AXIS_CENTER', '07_PROJECTION_RIM', '08_PROJECTION_BASE'): assert g1(r, 62) == '7' and g1(r, 6) == 'CONTINUOUS', (n, r)
    ln = lambda nm: [r for r in R if r[0] == 'LINE' and g1(r, 8) == nm]; xy = lambda r: [float(g1(r, c)) for c in (10, 20, 11, 21)]
    c, m, b = ln('01_AXIS_CENTER'), ln('07_PROJECTION_RIM'), ln('08_PROJECTION_BASE')
    assert len(c) == len(m) == len(b) == 1 and all(g1(r, 6) is None for r in c + m + b), (n, len(c), len(m), len(b))
    assert len(ln('02_EXTERIOR_OUTLINE')) == 0, (n, '02 층에 닫는 LINE 이 남음')
    x1, y1, x2, y2 = xy(c[0]); assert near(x1, 0, 2e-6) and near(x2, 0, 2e-6) and near(min(y1, y2), -3, 2e-6), (n, xy(c[0]))
    h = max(y1, y2) - 3; assert near(h, H, 0.06), (n, h)
    x1, y1, x2, y2 = xy(m[0]); assert near(y1, h, 2e-6) and near(y2, h, 2e-6) and near(x1 + x2, 0, 4e-6) and (rim is None or near(abs(x2 - x1), rim, 4e-6)), (n, xy(m[0]))
    x1, y1, x2, y2 = xy(b[0]); assert near(y1, 0, 2e-6) and near(y2, 0, 2e-6) and near(x1 + x2, 0, 4e-6) and (base is None or near(abs(x2 - x1), base, 4e-6)), (n, xy(b[0]))
    t = [g1(r, 1) for r in R if r[0] == 'TEXT' and g1(r, 8) == '10_ANNOTATION_TEXT']; assert t.count(az) == 1, (n, t)
dxf('studio_reflines.dxf', 184.8, 142.0, 142.0, 'FRONT AZIMUTH 0.0 DEG (CCW, TOP VIEW)')
dxf('studio_reflines_12.dxf', 184.8, None, None, 'FRONT AZIMUTH 12.0 DEG (CCW, TOP VIEW)')
print('PY-DXF OK')
PY
```
4. 실제 글꼴 캡처(PowerShell, 2 와 같은 환경 변수 + 글꼴 폴더 — 오케스트레이터 확인, verify.ps1 에는 아직 없음): `$env:QT_QPA_FONTDIR='C:\Windows\Fonts'; $b='D:\_dongarch_build\verify_job007\studio'; $a='D:\_dongarch_build\wt\factory\jobs\JOB-007\artifacts'; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--cut','--shot',"$b\shots\shot_cut_font.png"; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--cut','--edit','--shot',"$b\shots\shot_edit_font.png"; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--export','--shot',"$b\shots\shot_export_font.png"; foreach ($n in 'shot_cut_font.png','shot_edit_font.png','shot_export_font.png') { (Get-Item "$b\shots\$n").Length; Copy-Item "$b\shots\$n" $a }` → 기준 33-35 (`--shot` 은 창 1845×920 고정 — JOB-006 재작업 1). **PowerShell 창에서만 실행한다**(명령 안에 작은따옴표 인자 `'--sample'` 등과 `$b` 변수가 있어 Git Bash 에서 작은따옴표로 감싸면 따옴표가 깨진다 — Git Bash 대안은 두지 않음).
5. 수용 기준 24-30 은 builder 커밋 뒤 Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-007` 뒤 그대로.
6. verify.md 에 `[J7-1]`(외곽선 끝점 — 사람 검토 6 자료)·`[J7-3]`·`[J7-13]`·`[J7-15]`(창 최소 폭) 인쇄 줄과 파이썬 두 출력, 캡처 크기를 그대로 옮긴다(JOB-006 의 `[9]`·`[11]`·`[13] 단면 위치 x=` 줄과 섞지 않는다).

## 6. 영향 파일

| 파일 | 변경 | 이유 |
|---|---|---|
| `studio/src/Plate.h` | 수정 | `PlateOptions::paperPxPerMm`(14-23), `RefLine`·`PlateRefLines`·`plateRefLines`·`azimuthNote` 선언(§3.3-1) |
| `studio/src/Plate.cpp` | 수정 | `plateRefLines`·`azimuthNote`·정수 0.1° 각 표기 함수(익명 이름공간, `CutPageText.h` include 없음), 화면 기준선 단계(옛 바닥선 222-224·중심선 283-284 지움, 닫는 선 290-292), 「기고 H」 머리표 파선 끝·라벨 위치(352-358), `paintSheet` 용지 굵기·표제란(458·470-474), SVG 기준선 3층·옛 바닥선 600-603·중심선 679-681 지움·외곽선 층 이름 675·표제란 612-613, DXF 01/07/08 정의·선·방위각(751-799) |
| `studio/src/ExportDialog.cpp` | 수정 | 외곽선 칸 글(125), SVG·DXF 안내(255·257) |
| `studio/src/SheetView.cpp` | 수정 | 그리는 순서 글(232) |
| `studio/src/CutPageText.h` | 수정 | `kStatus` 원문 복원(18), ③·고정 문구·`fixText`·`slotMemo`(§3.2) |
| `studio/src/Ribbon.cpp` | 수정 | 타일 아이콘 `lock`·`next`(drawIcon 36-91) |
| `studio/src/MainWindow.h` · `MainWindow.cpp` | 수정 | `pEdit_`(h:115)·`fixedAngle_`·`stFix_`·`fixedLbl_`·`ctxFixKeys_`·`fixAngle`·`angleFixed`·`firstVerticalSlot`, ②·③ 리본(158-308, 202·285-286 의 측정·탁본 대체), ctx(310-349), 상태줄(406-429), 열기 3곳(503·541·556), `refreshAll`(580-621), `updateCutInfo`(623-638), `refreshSlots` 메모 표시(661), `confirmPose`(708-722), `onPage`(725-740), `setHandTool`(742-756), `restore`(764-774), `beginCutInput`(806-808), `onAction`(852-965), `keyPressEvent`(999-1076) |
| `studio/src/View3D.h` · `View3D.cpp` | 수정 | `setEditPage`·③ 3D 머리(h:49·55, cpp:265-276) |
| `studio/src/main.cpp` | 수정 | `--edit`(주석 27-33, 블록) |
| `studio/tests/test_reflines.cpp` | 신규 | 수용 기준 1-9, 파이썬 판정 파일 생성 |
| `studio/tests/test_cutpage.cpp` | 수정 | JOB-006 [8]·[15] 갱신(184-185·351-352), 기준 12-20 추가(371 앞) |
| `studio/CMakeLists.txt` | 수정 | `test_studio_reflines`·`studio_reflines`(91-110 근처) |
| `docs/ui/UI_RULES.md` | 수정 | R1 표 20·22·23, R3 59, R5 71·73 뒤, R6 82·83 — 정본(사람 검토 1) |

`GigaMesh/**`(K-Arch `DxfWriter` 포함), `studio/src/ArtifactDoc.*`·`CutGesture.h`·`MeshIO.*`·`MeshRaster.*`·`Theme.h`·`Ribbon.h`·`SheetView.h`·`StartPage.*`·`ExportDialog.h`, 기존 `test_pipeline/test_pose/test_side/test_cut.cpp`, `CLAUDE.md`, `docs/ui/mockup/**`, `.github/**`, `factory/**` 는 수정하지 않는다(기준 30 의 허용 목록 = 위 표 15개 파일). 코어 수정이 필요한 곳은 없다.

## 7. 위험·가정

- **좌표계**: mm·+Z 위·바닥판 Z = 0·확정 뒤 중심선 x = y = 0, 도판 단면 = 정면각 θ 의 Y = 0 면(D5 외면 왼쪽·단면 오른쪽, R = 거울상), 정면각 + = 위에서 보아 반시계 — 모두 그대로. 기준선은 도면 좌표(x = R, y = H)에서 좌우 대칭이라 D5·R 부호와 무관하다(기준 2). 도판 중심선 = 3D 중심축(x = 0), 바닥선 = 바닥판(y = 0), 구연선 = 구연 평면(y = H). 엔진을 건드리지 않으므로 LastTest 세 줄(기준 22)이 그대로여야 한다.
- **구연 평면 가정**: 이 앱엔 구연 수평(T)/바닥 수평(B) 선택이 없어 구연 평면 = 확정 자세에서 바닥판에 평행한 유물 최고점 높이의 면이다. 구연이 기울었거나 결실이 있으면 실제 구연 끝과 다르다(결실 쪽 파선도 없음 — §2). 구연선 폭 = 구경(Taubin 원 지름, 0.1 mm 반올림)이라 외곽선 끝점과 조금 어긋날 수 있다 — 시험이 예제의 외곽선 끝점을 인쇄한다(사람 검토 6).
- **바닥선 모양 변화**: 지금 화면·SVG 의 도판 전체 폭 먹선(바닥판 표시)이 없어지고 저경 폭 기준선이 된다. 외곽선을 끄면(L) 기준선도 꺼져 바닥 먹선이 없다(10 mm 회색 주선은 남음).
- **외곽선이 기준선에 닿지 않음(기존 엔진 한계)**: K-Arch 실루엣이 꼭짓점 z 값의 2·98 백분위 사이만 써서(`percentile(allZ, 0.02/0.98)`, Silhouette.cpp:118-119 — 높이 비율이 아님) 외면 외곽선이 예제에서 바닥보다 3.36 mm 위·구연보다 3.21 mm 아래에서 끝난다. 틈은 꼭짓점 분포에 따라 스캔마다 다르다(구연부 꼭짓점이 적으면 위 틈이 더 커질 수 있음). 지금까지는 그 높이의 닫는 가로선이 구연선·바닥선처럼 보였다. 기준선이 들어오면 그 닫는 선을 빼고(§3.3-4~6) 외곽선 끝과 기준선 사이 틈이 보이게 둔다 — 없는 선을 지어내지 않기 위함(추정값과 실측값 구분 규칙). 실루엣을 끝까지 쓰는 것은 엔진 작업이라 범위 밖(§10).
- **용지 기준 굵기**: PDF·PNG 는 종이 기준 0.10 / 0.13 / 0.25 mm 로 축척과 무관(디자인 「선 굵기 종이 기준 고정」 `H` 543) — 하지만 기존 단면·외곽선은 지금처럼 도면 축척을 따른다(혼재, 백로그 3). 내보내기 미리보기(≈ 2.5 px/mm)에서는 1 px 아래로 내리지 않아 실제보다 굵게 보인다.
- **작은 축척의 기준선·머리 치수(알려진 모양)**: 중심선 연장은 종이 3 mm 라 도면으로는 1:3 에서 9 mm·1:4 에서 12 mm 다 → 위쪽은 머리 치수선(y = H + 8, Plate.cpp:343·SVG 640-642)을 가로지르고(제도에서 치수선이 중심선을 지나는 것은 흔함), 아래쪽은 도판 틀 아래 끝(`E.top() = −8`, Plate.cpp:120)을 종이 0.33 mm(1:3)·1 mm(1:4) 벗어난다. PDF·PNG 의 머리 치수 라벨 상자(`구경 ⌀`·`기고 H`)는 종이 크기 글자(2.6 mm)라 1:2 이하에서 도면보다 상대적으로 커져, 1:3 에서는 `구경 ⌀` 상자가 중심선 위 연장 대부분을 가리고 위로 올린 `기고 H` 상자가 머리 치수선 왼쪽을 덮을 수 있다. 출력 치수 표기(디자인: 치수 층 출력 끔, `H` 562 · 「치수 (화면 전용, 도면 편집 단계)」 1064)는 백로그 3 몫이라 이번엔 고치지 않는다 — 사람 검토 6.
- **DXF 층 재사용**: Studio DXF 의 빈 층 07·08 에 구연선·바닥선을 넣고 01·07·08 을 실선·ACI 7 로 바꾼다. 층 수·이름·순서(11)는 그대로라 CLAUDE.md §2·R6 「11 레이어」 와 맞는다. K-Arch 단독 출력기(`GigaMesh/.../DxfWriter.cpp:269-304`, test_dxf_units)는 같은 층에 평면 투영 원을 그린다 — 의미가 갈린다(사람 검토 7). KS 제도는 중심선을 가는 일점쇄선으로 그리지만 사용자 도판(디자인 가이드 §6)은 가는 실선이다.
- **③ 고정 모델**: 고정은 화면 상태(`fixedAngle_`)와 고정한 종단면 슬롯의 각·메모(`막 105°`)로만 남고 파일·되돌리기에 쌓이지 않는다. 슬롯 목록의 `고정` 꼬리는 저장값이 아니라 `refreshSlots` 가 매번 `angleFixed()` 로 붙인다(§3.5) — 고정이 풀리면 메모는 `막 105°` 로 돌아간다. ③ 에서는 막을 돌릴 수 없고(키 무시, 3D 왼쪽 끌기 = 보는 방향), 되돌리기·슬롯 고르기로 각이 바뀌면 그 각으로 다시 고정한다(원문 render 1681). 홈 등 ② 밖에서는 여전히 `← →` 가 막을 돈다(백로그 21) — 그때는 고정이 풀린다.
- **JOB-006 의존**: JOB-006 이 승인 대기다. 이 작업은 JOB-006 시험(test_cutpage.cpp [8]·[15])의 기대값을 바꾸고 UI_RULES R1 을 다시 고친다 — JOB-006 이 재작업되면 이 브랜치를 그 위로 다시 쌓아야 하고 충돌은 test_cutpage.cpp·MainWindow.cpp·UI_RULES.md 에 날 수 있다. 007a(§0)는 JOB-006 이 만든 파일·함수(`CutPageText.h`·`cutpage::angleText`)를 쓰지 않는다(정면 방위각 각도 글은 Plate.cpp 안의 정수 0.1° 함수 — 기준 28 이 `CutPageText.h` include 없음을 확인). 007b 는 `CutPageText.h` 에 기댄다.
- **창 최소 폭**: ② 에 타일 2개, ctx 에 `Enter 지금 각도 고정`, 상태줄에 `StFix` 가 늘지만 홈 리본이 가장 넓다고 본다(§3.4) — 기준 13·15 가 오프스크린 값으로 확인(≤ 1845, 넘으면 `--shot` 창이 커져 기준 31 해시가 깨진다). 실제 글꼴 폭은 캡처 33-34 로 본다.
- **시험 시간**: `studio_cutpage` 는 원본 계산이 ≈ 7번 늘어 ≈ +14 s(RUN_SERIAL 그대로), `studio_reflines` 는 예제·미스캔 로드와 다시 계산 3번·내보내기 9번·래스터 3장으로 ≈ 17 s 예상.
- **캡처 해시**: `shot_home` 은 같아야 하고(기준 31), `shot_main`·`shot_export` 는 도판 기준선·탭 글자·내보내기 글 때문에 일부러 다르다(기준 32·35). `shot_export` 는 사용자 QSettings(`export/format`·`export/scale`)에 따라 안내 글이 달라진다.
- **Illustrator**: SVG 층 3개가 Illustrator 에서 따로 보이는지는 미확인(백로그 5·12, 알려진 위험 6). 층 id 는 점 없이 밑줄로 해 위험을 줄였다.
- **R2·R3**: 새 색 토큰 없음(기준선 = `OuterInk`, `theme::hex` 로만). 주 단추는 ② `도면 편집으로`, ③ `도면`(기존) — 화면당 하나(기준 13·15·27).
- **라이선스**: 새 의존성 없음.

## 8. 사람 검토 항목

1. **[정본] UI_RULES 변경**: R1(탭 줄에 `③ 도면 편집`·`측정·탁본` 없앰·②③ 잠김 문장, ③ = 「✎ 도면 편집」 을 든 상태, ③ 에서 3D 330 px), R3(② 주 단추 `[도면 편집으로]`), R5(그리는 순서에 기준선, 기준선 규약 줄), R6(SVG 기준선 3층 id, DXF 기준선 = 01·07·08, 정면 방위각). 목업 파일(`docs/ui/mockup/`)은 그대로다. JOB-006 의 R1 변경도 아직 승인 전이다.
2. **[흐름] ③ 탭 = 누르면 지금 각도로 고정하고 넘어감**(시안 1714·1681). 원문 가이드 §8 표(789)는 ⑦ 각도 고정 전 「도면 편집」 을 잠근다 — 원문 안에서 갈려 시안 동작을 골랐다(잠그면 빨강 주 단추가 먹지 않는 것처럼 보임). 바닥 확정 전에는 ② 와 같이 잠김(원문 탭 줄엔 잠김 없음 — JOB-006 방식).
3. **[탭] `측정·탁본` 탭을 없앴다** — 그 두 그룹(선 다듬기·간략화, 도판 표시)은 홈과 ③ 에 있다. 탭 줄이 원문과 같아졌다.
4. **[고정] 고정하면 종단면 슬롯 하나(지금 슬롯이 종단면이면 그것, 가로 슬롯을 보고 있었으면 첫 종단면 A–A′)의 각도·메모가 바뀐다** — 메모는 1~5 슬롯 저장과 같은 `막 12°`(원래 메모 `보고서 주 도판 (1:1)` 은 덮어씀), 고정 중에는 슬롯 목록에 `막 12° 고정` 으로 보이고(원문 ② 1508·③ 1514 처럼 살아 있는 표시) 막을 움직여 풀리면 `막 12°` 로 돌아간다. 슬롯을 다시 골라도 그 각으로 돌아온다. 고정은 되돌리기에 쌓지 않고 파일에도 남지 않는다.
5. **[문구·모양] 원문과 다르게 정한 곳**: (가) 타일 두 줄 → 한 줄 `지금 각도 고정`·`도면 편집으로`(리본 높이) (나) 고정되면 타일 `On`(Oat) — 원문 활성(어두운 바탕) (다) ③ ctx 안내 = 원문 도판 안내 줄(1656)을 줄인 글 — 원문 ctx(1665)의 편집 도구 안내 대신, 원문 1656 의 앞 `✓ `(초록)와 끝 ` · 막 12° 고정` 은 빼고(각은 ③ 리본 「고정된 막」·상태줄에) 두 span 을 ` · ` 로 이음, 오른쪽 최대 편차 없음. ③ 에서 슬롯 목록으로 가로 슬롯(B–B′)을 보면 기준선이 그려지지 않고(가로 슬롯은 기준선 단계 전에 return, Plate.cpp:232-262), 중심축 미정렬 문서(`!aligned()`)에서는 x = 0 이 바운딩 박스 중심(ArtifactDoc.cpp:584-587)이라 「중심선 = 3D 중심축」 이 엄밀하지 않은데도 안내는 같다 — 이번엔 그대로 둔다(상태줄이 미정렬을 따로 알림). 도판 머리는 원문 `A-A′`(하이픈, 1654) 대신 앱 슬롯 이름 `A–A′`(en dash) (라) ③ 상태줄 `⑧ 치수 확인 — 치수 띠를 보고 다음: 「도면」 내보내기 (Ctrl+P)`(원문 「치수 띠에 반영」·⑨ 없음) (마) `막 12° 고정` 과 「↩ 3D로 돌아가 다시 자르기」 를 ③ 리본 새 그룹 「고정된 막」 에(원문: 3D 아래 줄 1537·도판 도구줄 1655) — 기준선 설명은 ctx 하나에만 (바) ③ 리본 그룹 원문 7개 → 5개(「선 고치기」「기준선」「문양·탁본」 없음, 「고정된 막」 추가) (사) ③ 3D 머리 단추 [정면][위] 그대로(원문 [비스듬히][도판 방향]) (아) SVG 층 id `refline_center` 등(원문 `refline.center`), 층 표시명 `중심선`·`구연선`·`바닥선`(원문 555-557 `중심선 (위아래 3 mm 연장)`·`구연선 · 경부선 (전체 폭)`·`바닥선 (= 바닥판)` — 경부선이 없고 폭이 구경이라) (자) 화면 중심선 먹색(원문 회색 — R2·R5 따름) (차) ctx 에 `Enter 지금 각도 고정`(원문은 오른쪽 단추 안 배지).
6. **[기준선 규약]** 구연선 = 유물 최고점 높이·폭 = 구경, 바닥선 폭 = 저경(원문 출력 1853-1856 은 외면·단면 윤곽의 실제 끝점), 중심선 실선 0.10 + 종이 3 mm, 굵기 0.13 / 0.25. 도판 전체 폭 바닥 먹선이 없어진다. **외면 외곽선 끝이 기준선에 닿지 않는다**(예제 아래 3.36 mm·위 3.21 mm 틈 — K-Arch 실루엣이 꼭짓점 z 의 2·98 백분위 사이만 씀, 틈은 꼭짓점 분포에 따라 스캔마다 다름) — 예전의 닫는 가로선(두 번째 구연선처럼 보이던 선)은 뺐다. 시험 `[J7-1]` 이 외곽선 끝점과 구경/2·H 의 차이를 인쇄한다 — 국내 보고서 관행에 맞는지, 틈을 엔진에서 메울지(§10), 결실·기운 구연 처리(§2)를 언제 할지. **「기고 H」 머리표**: 구연선이 있으면 갈색 파선은 구연선 바깥 끝 2 mm 앞까지만, 라벨 상자는 구연선 바로 위(§3.3-4) — 예전처럼 축까지 그으면 구연선 외면 쪽 반을 덮는다. **작은 축척(알려진 모양, §7)**: 1:3·1:4 에서 중심선 연장(종이 3 mm = 도면 9·12 mm)이 머리 치수선(H + 8)을 가로지르고 아래로 도판 틀을 종이 0.33·1 mm 벗어나며, PDF·PNG 의 종이 크기 라벨 상자(`구경 ⌀`·`기고 H`)가 1:3 에서 중심선 위 연장·머리 치수선 일부를 가린다 — 출력 치수 표기 정리(백로그 3)에서 함께 볼지.
7. **[DXF 층]** 새 층 대신 기존 `01_AXIS_CENTER`·`07_PROJECTION_RIM`·`08_PROJECTION_BASE`(입면 투영 = 구연선·바닥선, 11층 유지) — K-Arch 단독 출력기는 같은 층에 평면 투영 원을 그린다. 새 이름(`12_REFLINE_RIM`·`13_REFLINE_BASE` 등, CLAUDE.md §2·R6 「11 레이어」 변경)이 낫다면 후속. 01 의 선 종류를 일점쇄선(CENTER) → 실선으로 바꿨다(KS 제도 관행과 다름, 사용자 도판 따름).
8. **[정면 방위각]** 표제란 첫 줄 끝 `정면 방위각 12° (위에서 보아 반시계)`(종단면만), DXF 영문 `FRONT AZIMUTH 12.0 DEG (CCW, TOP VIEW)`. SVG 는 방안 층을 끄면 표제란과 함께 빠진다. 보고서 용어 「정면 방위각」·자리가 맞는지.
9. **[켜고 끄기]** 화면·SVG·PDF·PNG 의 기준선은 「단면·외곽」(L)·내보내기 「외곽선·기준선」 칸과 함께 켜고 끈다. **DXF 는 지금도 외곽선·눈금 칸을 보지 않고 늘 쓰므로**(Plate.cpp:741-806 — `r.opts` 는 `hatch` 만) 기준선 3층도 칸과 무관하게 늘 나간다 — 칸을 끄면 DXF 에도 빠져야 하는지. 원문의 별도 「기준선」 칸(541)·③ 기준선 그룹(1476)은 후속.
10. **[출력 단면]** 단면은 지금처럼 빨강 0.35 mm + 연한 빨강 채움(SVG)으로 나간다 — 디자인 출력 규약(K100 윤곽선 0.30·채움 없음, 사용자 「단면은 윤곽선 두 줄 기본」)과 다르다 → 후속(백로그 3).
11. **[손 확인]** 실제 화면: ② 「고정 · 넘기기」 타일 글자 잘림, ③ 3D 330 px 에서 3D 머리 단추·정보 줄이 잘리지 않는지, `Enter` 고정 체감, ③ 에서 ← → 가 먹지 않는지, 1366 px 화면(JOB-006 §5-7), Illustrator 에서 기준선 층 3개(백로그 5·12).

## 9. 크기

| 묶음 | 파일 | 예상 줄(추가+수정) | 빈 줄·주석 제외 |
|---|---|---|---|
| 기준선·방위각 엔진 | Plate.h/.cpp(머리표 정리 +3, 각 표기 함수 +6 포함) | ≈ 110 | ≈ 90 |
| 문구 | ExportDialog.cpp, SheetView.cpp | 4 | 4 |
| 표기 상수 | CutPageText.h | ≈ 20 | ≈ 16 |
| 아이콘 | Ribbon.cpp | ≈ 6 | ≈ 6 |
| 화면 연결 | MainWindow.h/.cpp — onPage 다시 쓰기 ≈ 20, 고정 상태(angleFixed·fixAngle·firstVerticalSlot·리셋 8곳·슬롯 표시) ≈ 28, ②·③ 리본 ≈ 15, ctx·상태줄 ≈ 6, refreshAll·updateCutInfo ≈ 10, onAction·keyPressEvent ≈ 14, 헤더 ≈ 10 | ≈ 105 | ≈ 90 |
| 3D 머리 | View3D.h/.cpp | ≈ 7 | ≈ 6 |
| 캡처 | main.cpp | ≈ 8 | ≈ 6 |
| **제품 합계** | | **≈ 260** | **≈ 218** |
| 시험·빌드 | test_reflines.cpp(신규 ≈ 250), test_cutpage.cpp(+≈ 140, 수정 4), CMakeLists.txt(+3) | ≈ 397 | — |
| 정본 | UI_RULES.md | ≈ 9 | — |

빈 줄·주석 제외 300줄 지침 안이라 나누지 않는다(§0). 다만 JOB-005(+32 %)·JOB-006(+13 %)처럼 넘기면 ≈ 245-290줄로 **300줄에 근접할 수 있다** → builder 는 007a(Plate·문구·test_reflines)를 먼저 커밋하고 007b 로 넘어가며 잰다. 넘으면 §0 의 007a/007b 로 나눈다. 시험이 큰 것은 SVG·DXF·래스터 세 출력 길을 모두 좌표로 판정하고(기준 1-9), 오프스크린에서 한글이 □ 인 캡처 대신 ③ 흐름을 실제 창으로 참/거짓을 가르기 때문이다(기준 12-20).

## 10. 후속 메모 (등록·명세는 코디네이터 몫)

- **외곽선을 기준선까지**: K-Arch 실루엣의 꼭짓점 z 2·98 백분위 자르기(Silhouette.cpp:118-119)와 빈 버림(꼭짓점 8개 미만·이웃보다 3 mm 안쪽)을 구연·바닥 끝까지 쓰게(엔진·LastTest 기준값 `pts=85/1318` 이 바뀜 — 별도 작업, 사람 검토 6 결과에 따라. 틈이 스캔마다 달라 실제 스캔으로 확인 필요).
- **출력 머리 치수 정리**(백로그 3 과 함께): 종이 크기 라벨 상자가 1:2 이하에서 기준선·치수선을 가리는 것(§7·사람 검토 6) — 디자인처럼 치수를 화면 전용으로 하거나 축척에 맞춰 배치.
- **기준선 2차**: 경부선(외면 실선·내면 긴 파선, `H` 754), 결실 쪽 구연선 파선(사람이 범위 확정), 복원선·파단 표시 층(`reconstruction`·`break`, `H` 552·561), 다면도 기준 눈금(`refline.ticks` 3 mm, `H` 558·761), ③ 리본 「기준선」 그룹(☑ 중심선 · ☑ 구연선·바닥선 · ☐ 기준 눈금, `H` 1476)·내보내기 「기준선」 칸 → `PlateOptions` 에 기준선 켜기를 외곽선과 따로.
- **출력 규약 정리(백로그 3)**: 단면 K100 0.30 윤곽선·채움 없음 기본(검정·빗금 선택), 디자인 14층 id 로 SVG 층 통일, DXF 기준선 층 이름 결정(사람 검토 7), 축척 막대 "0 5 10㎝"·캡션 "(1/3)", 보정 사각형 100 mm, 기존 단면·외곽선 굵기도 종이 기준으로.
- 구연 수평(T)/바닥 수평(B)(가이드 ④) — 들어오면 `plateRefLines` 의 구연선 높이·기울기를 그 구연 평면에서.
- ③ 편집 도구(점 고치기·복원선·파단 표시·탁본 넣기), 1~5 도면 버전, 10걸음 단계 띠(⑦ 각도 고정 ✓ 표시), 단면 닫힘 확인(⑦ 승인 기록).
- 회전판 빨간 손잡이 작업 때 ctx·3D 끌기 원문 복원(JOB-006 §10).
- ② 밖의 `← → , . [ ]`·확정 경로 차이(백로그 21), 창 포커스 잃으면 확정(백로그 19).
- CI(`.github/workflows/studio-ci.yml`) 캡처 목록에 `--sample --cut`·`--sample --cut --edit` 추가, verify.ps1 에 `QT_QPA_FONTDIR`(백로그 20) — 바꾸는 작업에서 기준 캡처 해시를 새로 만든다.

## 11. 비평 반영 기록

비평 26건(design 렌즈 D1-D14, code 렌즈 C1-C12)을 디자인 원문(`H`)과 worktree 코드로 다시 확인한 뒤 반영했다. 바뀐 수용 기준: 3·5·8·9·12·13·14·15·18·27·28·32·33·34(14개). 반영하지 않은 지적은 없다(D11·D14 는 지적이 제안한 대로 「알려진 모양·사람 검토 기록」으로 반영 — 동작 변경 없음).

| # | 지적 (심각도 · 위치) | 반영 | 이유·확인 |
|---|---|---|---|
| D1 | must · 기준 32 — `order_` 글 변경이 shot_main 에도 보임 | 반영 — 32 에 (c) 「그리는 순서」 글 길어짐 추가, 「치수 띠 값」 은 왼쪽 `dims_` 5대 치수 값만으로 한정, 33·34 에도 그 글 명시, §3.3-7 에 주석 | SheetView.cpp:224-238 확인 — `order_` 는 숨기는 곳이 없음(Grep `order_` 5곳) |
| D2 | should · §3.5 — 풀린 뒤에도 슬롯 메모 `막 105° 고정` 이 남음, 1~5 저장이 덮어씀 | 반영 — 메모는 `slotMemo` = `막 105°` 로 저장, `고정` 꼬리는 `refreshSlots` 가 `angleFixed()` 로 매번 붙임, `beginCutInput` 에서 풀릴 때 `refreshSlots()`, 기준 14 에 풀림 뒤 `막 105° 고정` 없음·`막 105°` 있음 | 원문 ② 1508(`고정됨 🔒` / `고정 전 · 움직이는 중`)·③ 1514 가 살아 있는 상태 표시. 끄는 동안은 `refreshAll` 이 안 와서(refreshPreview 799-804) beginCutInput 갱신 필요 |
| D3 | should · §3.5 fixAngle — 가로 슬롯을 보며 고정하면 A–A′ 각이 옛 값 | 반영 — 고정 슬롯 `k` = 종단면이면 `slot_`, 아니면 `firstVerticalSlot()`(onPage 와 같은 찾기), 기준 18 에 ② B–B′ 보며 Enter → ③ → B–B′ → A–A′ 경로 판정 | MainWindow.cpp:678-680 `selectSlot` 이 슬롯 각으로 `setFrontAngle` 함을 확인 |
| D4 | should · §3.3-4 — 「기고 H」 Caution 파선이 구연선 외면 쪽 반을 덮음 | 반영 — 구연선이 있으면 파선을 `−rim/2 − 2` 까지, 라벨 상자는 구연선 위(AlignBottom), 기준 9 에 labels 래스터(caution·ink 판정), 32 (d)·33 | Plate.cpp:352-358 확인. 추가로 **흰 라벨 상자**(화면 ≈ 35 mm, 1:1 PDF ≈ 25 mm)도 구연선 끝·외곽선 틈을 가림을 확인해 함께 옮김 |
| D5 | should · §3.11 — 새 판정 `[13]`·`[15]` 가 JOB-006 꼬리표와 섞임 | 반영 — 새 판정 꼬리표·인쇄 모두 `[J7-n]`(test_reflines 포함), 기준 3·13·15·§5-2·§5-6 | test_cutpage.cpp:315 `[13] 단면 위치 x=`, 341-359 `[15]` 확인 |
| D6 | should · 기준 15 — 그룹 판정이 리본 전체를 찾음 | 반영 — `groupNames(stack->widget(4))` == 5개 순서 판정 | test_cutpage.cpp:80-84 `groupName` 이 `rb->findChildren` 전체, 홈(217-219)·내보내기(287)에 같은 그룹 확인. 순서 = 만든 순서(Ribbon.cpp:155-162·311-323) |
| D7 | should · §3.11 — `_12.svg` 용지·축척 미명시 | 반영 — 기본 요청 `R0`(A4 세로 1:1, 제목 `기준선 시험`)를 정의하고 `_12.svg`·`_12.dxf` 도 `R0` | 파이썬 기준 10 이 `svg('studio_reflines_12.svg', 1.0, …)` 로 s = 1 가정 |
| D8 | nit · §7·사람 검토 6·§10 — 「높이 2–98 %」 부정확 | 반영 — 「꼭짓점 z 의 2·98 백분위(+ 빈 버림), 틈은 스캔마다 다름」 으로 §3.3-4·§7·사람 검토 6·§10·기준 32 | Silhouette.cpp:118-119 `percentile(allZ, 0.02/0.98)`, 빈 8개 미만(149)·이웃보다 3 mm 넘게 안쪽(161) 버림 확인 |
| D9 | nit · kEditHint·SVG 층 레이블·도판 머리 — 원문과 글자 차이 3곳 | 반영 — §3.1 표·사람 검토 5 (다)·(아)에 `✓ `·` · 막 12° 고정` 생략, 원문 층 표시명, `A-A′` vs `A–A′` 기록(글자는 그대로) | `H` 1656·555-557·1654 확인. 원문 층 표시명 「구연선 · 경부선 (전체 폭)」 은 이번 선(경부선 없음·폭 = 구경)과 맞지 않아 짧은 이름 유지 |
| D10 | nit · §3.3-4~6 — 닫는 선 front/back 근거 없음 | 반영 — 「y 가 작은 끝 = 바닥선, 큰 끝 = 구연선」 규칙으로 바꾸고 z 오름차순 근거 병기 | Silhouette.cpp:145-146 빈 z 오름차순(i 증가), 76 `finishProfile` 이 i 순서로 넣음, ArtifactDoc.cpp:795-800 순서 보존 확인 |
| D11 | nit · §3.3-4·5 — 작은 축척에서 중심선 연장이 치수선을 가로지르고 틀을 벗어남 | 반영(기록만) — §7 새 항목·사람 검토 6·§10 에 알려진 모양으로, 코드 변경 없음 | Plate.cpp:120 `E.top() = −8`, 343 `yDim = H + 8` 확인. 치수선이 중심선을 지나는 건 제도에서 흔하고, 종이 크기 라벨 상자가 1:3 에서 가리는 문제는 출력 치수 정리(백로그 3, 원문 `H` 562·1064 치수 = 화면 전용) 몫 |
| D12 | nit · §3.1·기준 34 — 그룹 목록이 6개처럼 읽힘 | 반영 — 그룹마다 「」로 묶고 「5개」 명시(§3.1·§3.4·기준 34·사람 검토 5 (바)) | MainWindow.cpp:181 그룹 이름 `선 다듬기 · 간략화` 확인 |
| D13 | nit · §5-4 — Git Bash 따옴표 안내가 깨짐 | 반영 — Git Bash 대안 삭제, PowerShell 에서만 실행 | 명령 안 `'--sample'` 등 작은따옴표 인자 확인 |
| D14 | nit · §3.6-2 — 가로 슬롯·미정렬에서도 같은 ③ 안내 | 반영(기록만) — 사람 검토 5 (다)에 적고 동작은 그대로 | Plate.cpp:232-262 가로 슬롯 조기 return, ArtifactDoc.cpp:584-587 미정렬 시 bbox 중심 확인. 미정렬은 상태줄(615)이 따로 알림 |
| C1 | must · 기준 32 — D1 과 같음 | 반영(D1) | 같은 근거 |
| C2 | should · §3.5 — 메모를 saveSlotAngle 처럼 `막 105°` 로 | 반영(D2 와 합침) — 저장 메모는 C2 안(`막 105°`, `slotMemo` 로 823 과 같은 글), 표시는 D2 안(살아 있는 `고정` 꼬리) | 원문이 슬롯 목록에 고정 상태를 보이므로(1508·1514) 꼬리 자체는 남기되 저장하지 않음 |
| C3 | should · 기준 15(·13) — 리본 전체 검색 | 반영(D6) — 13 에도 `widget(3)` 그룹 5개·마지막 「고정 · 넘기기」 판정 추가 | 33 의 「마지막」 을 코드로 판정 |
| C4 | should · §3.3-4 — 머리표 파선이 구연선을 덮음 | 반영(D4) — 파선 `E.left()+2 → −rim/2 − 2`, 기준 9 에 `caution(−rim/2−6, H)` 유·구연선 위 무 | ExportDialog 기본 `labels` 켜짐은 PlateOptions 기본값(Plate.h:20)으로 확인 |
| C5 | nit · §0·§7 — 007a 가 JOB-006 의 `CutPageText.h` 에 기댐(계층 역전) | 반영 — Plate.cpp 안에 정수 0.1° 각 표기 함수, `CutPageText.h` include 금지(기준 28), §0·§7 문장 고침 | `git log --follow CutPageText.h` = eee6dd8(JOB-006) 하나, 머리 주석 1-3행 「② 페이지 화면 문자열」 확인 |
| C6 | nit · §3.3-6 — `/fp:fast` 에서 `+ 0.0` 이 접혀 `-0.0` | 반영 — DXF 각도 `llround` 정수 0.1° 로 부호·소수 직접 조립, 기준 5·8 에 −0.04°·−20° 판정(`_m004.dxf`·`_m20.dxf`) | CMakeLists.txt:21 `/fp:fast` 확인 |
| C7 | nit · §9 — MainWindow 85줄은 낮음 | 반영 — 항목별로 다시 세어 ≈ 105 / ≈ 90, 제품 ≈ 260 / ≈ 218, 「300 근접 — 007a 먼저 커밋」 지침(§0·§9) | 이번 반영(슬롯 표시·firstVerticalSlot·fixedLbl_)으로 더 늘어남 |
| C8 | nit · 기준 27 — 재배치 시 hex grep 거짓 FAIL | 반영(둘 다) — 기준 27 hex 를 개수(≤ 19)·새 값 없음(`comm`)으로, §3.3-5 에 hex 든 줄(607-611·677) 재배치·형식 도구 금지 | Plate.cpp:607-613·677-681 확인, 688b7bd 의 hex 21개(`git grep -h -o`) 실측 |
| C9 | nit · §3.4 — 「고정된 막」 그룹 두 줄이 ctx 안내와 중복, 각도 없음 | 반영 — 그룹 = `FixedAngle` 라벨(`막 θ° 고정`, updateCutInfo 갱신) + [↩ …] 단추, `kFixedRow2/3` 삭제, 기준 15·34 | MainWindow.cpp:337 `ctxDupKeys_` 원칙, 원문 1537 `<b>막 12° 고정</b>` 먼저 확인 |
| C10 | nit · 기준 34 — D12 와 같음 | 반영(D12) | 같은 근거 |
| C11 | nit · §3.3-1·-6 — DXF 는 외곽선 칸을 안 봄 | 반영 — §3.3-1·§3.3-6 에 「DXF 는 늘 기준선 3층」 명시, 사람 검토 9 에 질문 | Plate.cpp:741-806 에서 `r.opts` 는 `hatch`(779)만 확인 |
| C12 | nit · §3.3-4·§7·§10 — D8 과 같음 | 반영(D8) | 같은 근거 |
