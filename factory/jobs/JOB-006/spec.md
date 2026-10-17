# JOB-006 명세 — ② 막으로 자르기 004c: **② 페이지** (탭·잠김·리본 원문 라벨·ctx·상태줄·도판 머리·`--cut`)

작성: spec-writer, 2026-10-17(비평 반영판 — §11). 기준 커밋 `804f60d`(factory/JOB-006 = JOB-005 끝, worktree `D:/_dongarch_build/wt/.worktrees/JOB-006`, **JOB-003 → JOB-004 → JOB-005 승인 전제**).
아래 코드 경로·줄 번호는 모두 이 worktree 에서 직접 읽고 Grep 으로 확인한 값이다. 디자인 줄 번호는 정본 `D:\_dongarch_build\design\dongarch3d-design-v2.html`(이하 `H`)에서 Grep 으로 확인했다(design-excerpt.md 와 다르면 `H` 가 맞다).
이하 `W` = `D:/_dongarch_build/wt/.worktrees/JOB-006`, `B` = `D:\_dongarch_build\verify_job006\studio`(빌드 폴더), `L` = `D:\_dongarch_build\verify_job006\studio-logs`(verify.ps1 로그 폴더).
출발점: JOB-004 명세 §10-2(004c 씨앗), JOB-005 명세 §10, JOB-005 승인 보고서 §5-5(끄는 동안 옛 값 표시)·§5-6(대기 커서·두 번 계산)·§5-10(용어 혼재)·§5-11(ctx 폭).

## 0. 분할 판단

씨앗 범위 그대로 세면 **제품 ≈ 300줄(빈 줄·주석 제외 ≈ 247줄) + 테스트·빌드 ≈ 290줄**(§9)이다. 이 공장은 JOB-005 부터 300줄 지침을 빈 줄·주석 제외로 잰다 — 그 기준 안이라 **나누지 않는다.** 다만 JOB-005 는 예상 260 → 실측 343(빈 줄·주석 제외 292)으로 30 % 넘었으므로, builder 실측이 **빈 줄·주석 제외 300줄을 넘으면 그 자리에서 멈추고** 코디네이터에게 다음 두 조각을 제안한다. 각 조각은 자기 수용 기준만으로 참/거짓이 갈리게 나눴다(006a 의 기준은 006b 코드 없이 참이 된다).

| 조각 | 내용 | 수용 기준 |
|---|---|---|
| **006a** ② 페이지 뼈대·라벨·진입 | `CutPageText.h`, Ribbon API·`rows` 줄 높이 고정(§3.3), Theme QSS 1줄, ② 탭·잠김, 리본 3그룹·[−][+]·X/Y 맞춤(§3.7 — 대기 커서·진행 중 제스처 취소 포함), 옛 단추·용어 정리(§3.8), §3.5 「onPage」 1-4·6 과 5 의 `setHandTool("cut")`, `requirePose` 순서, `confirmPose` → ②, `setHandTool` 문구, 잠김(`setPageLocked`), 표시 갱신(§3.6-1~4·6), UI_RULES R1·R8(§3.10) | 1-12, 14, 16, 18-27, 29-30 |
| **006b** 입력 규칙·3D 끌기·ctx 폭·캡처 | §3.5 「onPage」 5 의 가로 슬롯 → 첫 종단면, §3.5 「입력 규칙」의 `S`·홈 「단면선」·`Esc`·`O`, §3.5 「3D 끌기」(오른쪽 단추 = 보는 방향·② 3D 툴팁), ctx 폭(§3.6-5), `--cut`(§3.9), 캡처 | 13, 15, 17, 28, 31-34 |

## 1. 목표

사용자 요청(job.json `request`, 코디네이터 릴레이):
> 사용자 08:03 핵심 요구(바닥 정하기 → 얇은 막으로 자르기 → 도면 편집), 09:39 "승인개발진행", 10:59 "전부 다되면 개발진행하라". JOB-004 명세 §10 후속 조각 2(004c) 씨앗, 화면 디자인 v2 phase=cut.

project-profile.md 「사용자 핵심 요구」: "**바닥 정하기**(X·Y·Z 이동·회전으로 바닥판에 올려 확정) → **얇은 막 절단**(3D 뷰에서 막을 움직이면 오른쪽 도판에 단면이 실시간 표시) → **도면 편집**으로 넘기기." 코디네이터 전달: "초보자도 쓸 수 있게 직관적이어야 한다."

**재진술(한 문장)**: 리본에 `② 막으로 자르기` 탭을 ① 바로 뒤에 두어(바닥 확정 전에는 숨기지 않고 잠김 표시·누르면 ① 로 안내) 「이 자세로 확정」하면 곧바로 그 탭으로 넘어가고, 그 탭에 있는 동안은 「✂ 막으로 자르기」 도구를 든 상태로 디자인 v2 원문 라벨의 리본(막 방향 X·Y 맞춤 / 회전판 각도 [−][+] / 단면 위치)·ctx·상태줄·도판 머리 `A–A′ 실시간 단면 — θ°`·3D 머리가 JOB-005 실시간 단면과 함께 실제 각도를 따라 바뀌며(3D 는 왼쪽 끌기 = 회전판, 오른쪽 끌기 = 보는 방향), 흩어져 있던 「정면 회전」「절단 위치 이동」「절단면」 단추·용어를 `막`·`회전판` 으로 정리한다 — 계산 엔진(ArtifactDoc·CutGesture·Plate)은 바꾸지 않는다.

## 2. 범위 밖

- **JOB-007**: `고정 · 넘기기` 그룹([지금 각도 고정 Enter][도면 편집으로] 주 단추), `③ 도면 편집` 탭, 기준선 레이어, 상태줄 오른쪽 `막 고정 전 / 12° 고정`, 상태줄 원문 꼬리 「지금 각도 고정」 → 「도면 편집으로」 복원(§10).
- **그 뒤**: Z 막(높이 막·↑ ↓ 1 mm — 이번엔 사용 불가 타일만), 각도 띠, 추천 3, 한 바퀴 훑기, `보기` 그룹(앞쪽 반 흐리게·도판 방향으로 보기 V·각도 띠), 보라 점(짝 표시), 회전판 빨간 손잡이 3D 그리기(→ ctx 원문 안내·원문 마우스 규칙 「빈 곳 끌기 = 보는 방향만」 복원), 중심선 고정 풀기(막 밀기·"보고서 단면 아님"), 실시간 확인 줄(닫힌 단면·두께·좌우 구경 차), 왼쪽 안내 패널(「막으로 자르기」 걸음·단면 목록), 10걸음 단계 띠, 놓을 때 원본 계산 비동기화, 도판·파일에 절단 각도 기록(JOB-005 §5-8), ctx 오른쪽 문구 줄이기·말줄임(사람 검토 7 결과에 따라).
- **새 Theme 색 토큰**(축 색 X 주황 `#c8641e` · Y 초록 `#3f8f4f` · Z 파랑 `#3f6fb5`, 짝 보라 `#6b4fc0` — `H` 440·824행): UI_RULES R2 변경이라 사용자 승인 대상 → 넣지 않는다(사람 검토 3).
- 계산 엔진·내보내기: `studio/src/ArtifactDoc.*`·`CutGesture.h`·`Plate.*`·`MeshIO.*`·`MeshRaster.*` 수정, 정면각 부호·X 막 90° 의 뜻 변경(JOB-005 사람 검토 1·2 그대로).
- 홈 리본의 다른 그룹, StartPage·ExportDialog, `.github/workflows/*`(CI 캡처에 `--cut` 추가는 §10), `factory/**` 기록, `CLAUDE.md`, `docs/ui/mockup/**`.
- `GigaMesh/core`·`GigaMesh/external`·`GigaMesh/gui/src/dongarch/karch/*` 수정.

## 3. 설계 결정 (builder 가 그대로 구현할 기본값)

### 3.1 원문 라벨과 이번 표기 (`H` 줄 = Grep 확인)

| 자리 | 원문 (`H` 줄) | 이번 | 비고 |
|---|---|---|---|
| 탭 | `파일 · 홈 · ① 바닥 정하기 · ② 막으로 자르기 · ③ 도면 편집 · 내보내기` (1429 `rtabs`) | `파일 · 홈 · ① 바닥 정하기 · ② 막으로 자르기 · 측정·탁본 · 내보내기` | `③ 도면 편집` 은 JOB-007. `측정·탁본` 은 원문에 없지만 그대로 둔다(사람 검토 2 (차)) |
| 탭 잠김 | **원문 탭 줄에는 잠김 표시가 없다** — 바닥 미정에서 ② 를 누르면 바닥을 자동으로 놓고 ② 로 간다(1714). 잠김 규칙은 가이드 쪽: 큰 버튼 잠김 "투명도 42 % + 자물쇠 툴팁 · 잠김은 숨기지 않는다 · 툴팁에 잠긴 이유" (730, `.app .dis{opacity:.42}` 135), ③ 바닥 놓기·④ 수평·기준선 미완료 시 잠기는 것 = `막으로 자르기` (785-786) | 숨기지 않음. 탭 글자 `Faint`(투명도 대신 기존 토큰) + 툴팁 `🔒 바닥 확정 후 사용 — ① 바닥 정하기에서 「이 자세로 확정」 (Enter)`. 누르면 ① 로 돌아가 상태줄 `먼저 ① 바닥 정하기에서 「이 자세로 확정」 (Enter)`(기존 `requirePose` 문구) | 가이드 §8 표(785-786)를 탭에 적용한 **해석**. 자동으로 놓지 않는 것은 이 앱의 바닥 확정이 사람 몫이기 때문(JOB-003) — 사람 검토 2 (라) |
| ① 확정 | `이 자세로<br>확정` · `→ ② 막으로 자르기` (1457) | Enter/「이 자세로 확정」 뒤 **② 탭으로 이동**. 타일 툴팁 `이 자세로 확정 (Enter) → ② 막으로 자르기` | `--sample`(자동 확정)·「중심축 맞춤」 은 지금처럼 홈 |
| 그룹 `파일` | [열기 ▾] (1462) | 기존 `fileGroup`(열기 · 최근 ▾) | |
| 그룹 이름 | `막 방향 (누르면 바로 맞춤)`, 지금 각이 Y·X 에서 1° 넘게 벗어나면 `막 방향 · 지금 12° (누르면 맞춤)` (1460, 1463) | 같음(각 = 실제 `previewAngle()`) | 판정은 원문 `m = mod360(ang) % 180`, Y: `m ≤ 1 ‖ m ≥ 179`, X: `|m − 90| ≤ 1` (1460) |
| 막 방향 타일 | `X 막<br>옆에서` · `Y 막<br>정면에서` · `Z 막<br>높이로`(두 줄), 글자 아이콘 X 주황 · Y 초록 · Z 파랑, 지금 각이 맞는 축 켜짐 = 활성(어두운 `#2b2b2b` 바탕·흰 글자, 121), 누르면 X = 90°, Y = 0°, Z = 높이 막으로 바뀜 (1461, 1463, 1723-1725) | 순서 **X · Y · Z 그대로**. 글자 **한 줄** `X 막 옆에서` · `Y 막 정면에서` · `Z 막 높이로`. 글자 아이콘은 `Ink`(축 색 없음). 맞는 축 = R3 `On`(Oat). **Z 는 사용 불가**(`setEnabled(false)`, 글자·아이콘 `Faint`) + 툴팁 `🔒 Z 막 높이로 — 높이별 횡단면 막은 다음 판에서 엽니다 · 지금은 홈 「단면 추가 (횡단면)」 (N)` | **원문과 다름 4곳**(사람 검토 2 (마)-(아)): ① 한 줄 — 두 줄이면 `Tile::sizeHint` 높이가 48+4+2×16 = 84 px(Ribbon.cpp:25-29)로 지금 가장 높은 칸(① 3단 작은 단추 열 23×3+4×2 = 77 px, MainWindow.cpp:221-226)보다 커서, 모든 페이지 중 최대를 쓰는 QStackedWidget(Ribbon.cpp:260) 때문에 **모든 탭의 리본이 약 7 px 커진다**. 이 앱은 이미 원문 두 줄 타일 `이 자세로<br>확정`·`자동<br>세우기`(1457·1453)를 한 줄 `이 자세로 확정`·`자동 세우기`(MainWindow.cpp:230·219)로 썼다 ② 축 색 없음(R2) ③ 켜짐 = Oat — 이 앱에서 어두운 바탕은 R3 「손에 든 도구」 전용 ④ Z 는 원문에서 동작하나 이번엔 사용 불가, 모양은 원문 잠김(투명도 42 %+자물쇠)이 아닌 `Faint` 글자 + 툴팁 🔒. 작업 지시문은 [Y][X] 순서였으나 원문 X·Y·Z 를 따름 |
| 그룹 이름 | `막 움직이기 (회전판)` (1467) | 같음 | |
| 줄 1 | ☑ `중심선에 고정 (돌리기만)` (1464, 원문은 누르는 체크) | QLabel `☑ 중심선에 고정 (돌리기만)` — 표시만(누를 수 없음), 툴팁 `종단면은 중심선(회전 중심축)에 고정해 돌리기만 합니다 — 기고·구경은 중심축 기준 · 풀기(막 밀기)는 다음 판` | 풀기 범위 밖 |
| 줄 2 | `각도 [−] 12° [+] [← →] 1° · [Shift] 15°` (1466) | 같음: QLabel `각도` · 작은 단추 `−`(id `cutMinus`) · 각도 칸(모노, `CutAngle`) · 작은 단추 `+`(id `cutPlus`) · `kbdLabel("← →")` · `1° ·` · `kbdLabel("Shift")` · `15°` | R8 kbd 배지. `, . [ ]` 는 ← → 의 별칭(툴팁에 적음, §3.4) |
| 줄 3 | [한 바퀴 훑기] `끄는 동안 실시간` (1467) | `끄는 동안 실시간`(Muted)만 | 훑기 범위 밖 |
| 그룹 `보기` | (1468) | 없음 | 전부 범위 밖 |
| 그룹 `단면 위치` | [`외면 왼쪽·단면 오른쪽` / 반전 시 `단면 왼쪽·외면 오른쪽`] R · 초록 알약 `국내 보고서 표준`(`pill ok`) · `사용자 도판으로 확인` (1469) | 줄 1 = 기존 id `combined` 작은 단추(kbd `R`, 같은 id 라 홈 단추와 함께 바뀜), 줄 2 `국내 보고서 표준`(`Done` 글자, 알약 없음), 줄 3 `사용자 도판으로 확인`(`Muted`) | **원문과 다름 2곳**(사람 검토 2 (자)): 반전 글자는 JOB-004 의 `단면 왼쪽·외면 오른쪽 (반전)`(MainWindow.cpp:50-51 `sideText`, 홈 단추와 공용) 유지, 알약 대신 `Done` 글자 |
| 그룹 `고정 · 넘기기` | (1470) | 없음 | JOB-007. ② 페이지에는 `Action` 단추가 없다(R3 "하나만" 을 넘지 않음) |
| ctx 배지 | `✂ 막으로 자르기` (1664) | 같음 | |
| ctx 안내 | `빨간 손잡이로 회전판을 돌리면 막이 지나는 자리가 바뀌고 오른쪽 단면이 바로 바뀝니다 · [R] 좌/우` (1664) | `3D 를 좌우로 끌어 회전판을 돌리면 막이 지나는 자리가 바뀌고 오른쪽 단면이 바로 바뀝니다` | **원문과 다름**: 빨간 손잡이는 아직 그리지 않는다(범위 밖) — 지금은 단면 도구를 든 채 3D 를 왼쪽 단추로 끌면 어디서든 돈다(§3.5 「3D 끌기」). `R` 은 리본 ② `단면 위치` 단추의 kbd 배지에 있다 |
| ctx 오른쪽 | `막 12° · 끄는 중 3.5만 면 약 1 ms → 놓으면 25만 면 약 6 ms` (1667) | 놓은 상태 `막 12° · 끄는 동안 2.6만 면 미리보기 · 놓으면 원본 7.7만 면 다시 계산`, 미리보기 중 `막 12° · 끄는 중 2.6만 면 미리보기 — 음영·외곽·치수·두께는 놓은 뒤` (면 수 = 실제 LOD·원본) | **원문과 다름**: 원문 ms 는 이 앱 실측과 다르다(JOB-005 §5-6: 놓으면 7.7만 면 원본 계산 1.36 s) → 고정 숫자 대신 실제 면 수. 미리보기 문구는 JOB-005 §5-5("끄는 동안 음영·외곽·치수·두께는 옛 각도 값 — 이를 알리는 표시는 JOB-006", ArtifactDoc.h:99-100) 몫. 둘째 줄 구성은 `H` 1635 `끄는 동안 3.5만 면 미리보기 · 놓으면 원본으로 다시 계산` 에서 옴 |
| 상태줄 | `⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 「지금 각도 고정」 → 「도면 편집으로」` (1673) | `⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 다음: 「도면」 내보내기 (Ctrl+P)` | **원문과 다름**: 두 단추는 JOB-007 — 없는 단추를 가리키지 않게 지금 되는 다음 할 일(R8 "다음: …")로. JOB-007 에서 원문으로(§10) |
| 도판 머리 | `A–A′ 실시간 단면 — 12°` · `— 3D에서 막을 움직이는 동안 바로 다시 그림 · 외면 왼쪽 · 단면 오른쪽`(반전 시 `단면 왼쪽 · 외면 오른쪽`) (1631) | 같음. `A` = 지금 종단면 슬롯 이름 첫 글자, 각 = `doc.previewAngle()`. 미스캔 보정이면 기존 꼬리 ` · 내벽 미스캔 파선 보정(n mm)` 그대로 붙임 | 머리 오른쪽 [화면 1:1 · 게재 1/3][R 좌/우] 는 기존 `1:1`·□ 그대로 |
| 각도 표기 | 정수 **0~359°** (`mod360` 910, 단추 1726, 끌기 1742·1764, 키 1782 — 모두 `mod360`·`Math.round`) | **−180~180°, 0.1° 반올림**(정수면 정수: `12°`, 반 도: `37.5°`, 음수: `−20°`) | **원문과 다름**(사람 검토 2 (바)): 정면각은 `std::remainder(·, 360)` 로 −180~180(ArtifactDoc.cpp:610·621)이고, 부호가 "위에서 보아 반시계 +"(JOB-005)의 뜻을 갖는다. 3D 끌기는 0.5°/px 라 반 도가 생긴다(View3D.cpp:191) |
| 3D 머리 | `3D — 막으로 자르기` (1529) | ② 에서 `<b>3D — 막으로 자르기</b>` | [비스듬히][도판 방향 V][위] 는 기존 [정면][위] 그대로 |
| 3D 끌기 | `빨간 손잡이` = 회전판 돌리기 · `빈 곳 끌기` = 보는 방향만 (1506, 가이드 825) | ② 에서 **왼쪽 단추 끌기 = 회전판**, **오른쪽(·가운데) 단추 끌기 = 보는 방향**. ② 의 3D 그림 툴팁 `왼쪽 끌기 = 회전판 돌리기 · 오른쪽 끌기 = 보는 방향만` | **원문과 다름**(사람 검토 2 (카)·4): 손잡이를 그리지 않아 "손잡이 / 빈 곳"을 가를 수 없다. JOB-005 까지는 도구를 내려놓으면 끌어서 볼 수 있었는데, ② = 도구 든 상태(§3.5)라 그대로 두면 ② 에서는 끌어서 볼 수 없게 된다(View3D.cpp:186-196 — `cutTool_` 이면 y 성분 버림) |

### 3.2 표기 함수 `studio/src/CutPageText.h` (신규, 헤더만, Qt Core — `studio_core` 에 넣어 시험)

네임스페이스 `studio::cutpage`. 머리 주석에 "원문: 화면 디자인 v2 phase=cut (`rtabs`·`ribbonCut`·`ctxBand`·`statusBar`·`rightCut`·`midCut`), 원문과 다르게 정한 곳은 JOB-006 명세 §3.1·§8-2" 를 적는다. **② 페이지의 모든 새 화면 문자열은 이 헤더 한 곳에 둔다**(툴팁 제외 — MainWindow 에 둬도 됨. 단 `k3dTip` 은 여기).

- 상수: `kTabName = "② 막으로 자르기"`, `kLockTip = "🔒 바닥 확정 후 사용 — ① 바닥 정하기에서 「이 자세로 확정」 (Enter)"`, `kCtxBadge = "✂ 막으로 자르기"`, `kCtxHint`(§3.1 이번 안내), `kStatus`(§3.1 이번 상태줄), `k3dTitle = "3D — 막으로 자르기"`, `k3dTip = "왼쪽 끌기 = 회전판 돌리기 · 오른쪽 끌기 = 보는 방향만"`, 그룹 이름 `kGrpMove = "막 움직이기 (회전판)"`, `kGrpSide = "단면 위치"`, 타일 글자(한 줄) `"X 막 옆에서"`·`"Y 막 정면에서"`·`"Z 막 높이로"`, 줄 글자 `"☑ 중심선에 고정 (돌리기만)"`·`"각도"`·`"1° ·"`·`"15°"`·`"끄는 동안 실시간"`·`"국내 보고서 표준"`·`"사용자 도판으로 확인"`.
- `QString angleText(double deg)`: 0.1° 로 반올림(`std::round(deg*10)/10`), 0 이면 부호 없음, 정수면 정수, 아니면 소수 1자리, 음수는 `−`(U+2212), 끝에 `°`. 예: 0→`0°`, 12→`12°`, 37.5→`37.5°`, −20→`−20°`, −0.04→`0°`, 180→`180°`, 12.04→`12°`, 12.06→`12.1°`, −179.5→`−179.5°`.
- `QString plateTitle(QChar letter, double deg)` = `"%1–%1′ 실시간 단면 — %2"`(U+2013, U+2032, U+2014) ← `letter`, `angleText(deg)`.
- `QString plateSub(SectionSide s)` = `"— 3D에서 막을 움직이는 동안 바로 다시 그림 · "` + (`Right` ? `"외면 왼쪽 · 단면 오른쪽"` : `"단면 왼쪽 · 외면 오른쪽"`).
- `bool atAxis(double deg, CutAxis a)`: `m = fmod(fmod(deg,360)+360,360)`, `h = fmod(m,180)`; Y = `h ≤ 1 || h ≥ 179`, X = `|h − 90| ≤ 1` (원문 1460).
- `QString axisGroupName(double deg)`: Y·X 중 하나면 `"막 방향 (누르면 바로 맞춤)"`, 아니면 `"막 방향 · 지금 %1 (누르면 맞춤)"` ← `angleText(deg)`.
- `QString faceText(size_t n)`: n ≥ 10000 → `round(n/1000)/10` 를 정수면 정수·아니면 소수 1자리 + `"만 면"`(26180→`2.6만 면`, 250000→`25만 면`), 아니면 영어 로캘 천 단위 쉼표 + `" 면"`(9800→`9,800 면`).
- `QString ctxRight(double deg, size_t lodFaces, size_t fullFaces, bool previewing)`: §3.1 두 문형(`"막 %1 · 끄는 동안 %2 미리보기 · 놓으면 원본 %3 다시 계산"` / `"막 %1 · 끄는 중 %2 미리보기 — 음영·외곽·치수·두께는 놓은 뒤"`).

### 3.3 Ribbon (`studio/src/Ribbon.h/.cpp`, `Theme.h`)

- `signals: void pageChanged(int page);` — `setCurrentPage`(Ribbon.cpp:300-307) 끝에서 늘 내보낸다(탭 누름 275 도 이 함수를 거침). `int currentPage() const`(= `stack_->currentIndex()`).
- `void setPageLocked(int page, bool locked, const QString& tip)`: 탭 단추에 속성 `locked`, 툴팁(`locked ? tip : ""`), 다시 polish. 누르는 것은 막지 않는다(안내는 MainWindow). `Theme.h` QSS(89-90 **다음** — 같은 특이도라 뒤에 와야 `on` 보다 이김)에 1줄 `#Tabs QPushButton[tab="true"][locked="true"] { color: %Faint%; }` — **새 색 토큰 없음**(기존 `Faint`).
- `RibbonGroup`(Ribbon.h:53-67): 그룹 이름 QLabel(Ribbon.cpp:156)을 멤버로 들고 `void setTitle(const QString&)`. `SmallButton* button(const QString& id, const QString& text, const QString& kbd = {})` — 만들고 `setObjectName(id)`·`registerWidget`·`triggered(id)` 연결까지(줄에는 넣지 않음, `column` 175-190 과 같은 방식). `void rows(const QList<QList<QWidget*>>& rows)` — 위젯 나열 줄 여러 개를 한 칸에 세로로(간격 4, 줄마다 오른쪽 stretch, `chipRows` 226-234 와 같은 모양). **줄마다 높이 23 px 고정**(`SmallButton` 높이, Ribbon.cpp:124) → 3줄 = 23×3 + 4×2 = 77 px = ① `바닥판에 올려놓기` 3단 열과 같다.
- **리본 높이 불변**: ② 페이지의 어떤 칸도 77 px 를 넘지 않게 한다(타일 한 줄 = 68 px, `rows` 3줄 = 77 px). 리본 `QStackedWidget` 은 모든 페이지 중 최대 높이를 쓰므로(Ribbon.cpp:260) 이것이 깨지면 홈·① 을 포함한 모든 탭이 커진다 — 기준 9 가 확인.
- `Tile`(Ribbon.cpp:36-112): `drawIcon` 에 종류 `"axX"·"axY"·"axZ"` = 글자 `X`·`Y`·`Z` 를 `theme::uiFont(16, true)`·색 `c` 로 가운데. `paintEvent`: `!isEnabled()` 면 `fg = Faint`·호버 바탕 없음(글자는 110 이 이미 `Faint`). 타일 글자는 한 줄이므로 `sizeHint` 는 그대로 쓴다.
- **끄는 동안 리본이 옆으로 밀리지 않게**: 각도 칸(`CutAngle`)은 모노 12 의 `"−179.5°"` 폭 고정·가운데 맞춤, `막 방향` 그룹 이름 칸은 가장 긴 문구(`axisGroupName(-179.5)`) 폭을 최소 폭으로 둔다(방법은 builder 재량).

### 3.4 ② 리본 페이지 (`MainWindow::buildRibbon`, 152-265)

- `addPage` 순서(194-198): `파일`·`홈`·`① 바닥 정하기`·**`kTabName`(멤버 `pCut_`)**·`측정·탁본`·`내보내기`.
- 그룹: `fileGroup(pCut_)` · `axisGroupName(0)` 그룹(멤버 `cutAxisGroup_` 로 보관) — 타일 `axX`·`axY`·`axZ`(글자 §3.2, 아이콘 `"axX"` …, 툴팁 X `X 막 옆에서 — 회전판을 90° 로 맞춰 옆에서 본 종단면(절단면 A–A′)` / Y `Y 막 정면에서 — 회전판을 0° 로 맞춰 정면에서 본 종단면(절단면 A–A′)` / Z §3.1), `axZ->setEnabled(false)` · `kGrpMove` 그룹 — `rows` 3줄(§3.1), 단추 툴팁 − `회전판(정면 방위각) −1° — 키 ← (Shift 15°) · , [ 도 같음`, + `회전판(정면 방위각) +1° — 위에서 보아 반시계(가까운 면이 오른쪽으로) · 키 → (Shift 15°) · . ] 도 같음` · `kGrpSide` 그룹 — `rows` 3줄: `button("combined", sideText(…), "R")` / QLabel `국내 보고서 표준`(`Done`) / QLabel `사용자 도판으로 확인`(`Muted`) — 색은 `theme::hex` 로.
- `combined` 툴팁 반복(241-242)과 상태 `On`(250)이 ② 단추에도 걸리게 ② 페이지를 그 앞에서 만든다.
- ① `poseConfirm` 툴팁(230)을 `이 자세로 확정 (Enter) → ② 막으로 자르기` 로.
- `connect(ribbon_, &Ribbon::pageChanged, this, &MainWindow::onPage)` 는 246 의 `setCurrentPage(pHome_)` **뒤**에.

### 3.5 페이지 = 도구 (② 탭에 있는 동안만 「✂ 막으로 자르기」를 든다 — R1 ctxBar "도구를 들었을 때만")

**「onPage」** — 새 `void MainWindow::onPage(int p)`:
1. `flushCut();`(탭 이동도 "다른 동작" — JOB-005 규칙).
2. `if (p == pCut_ && (!doc_.isLoaded() || !requirePose())) return;`(확정 전이면 → ① 로 이동·안내). `requirePose`(640-645)는 **페이지를 먼저 옮기고 안내를 나중에** 쓰도록 642·643 순서를 바꾼다.
3. `p != pCut_` 이고 `hand_ == "cut"` 이면 `setHandTool(QString())`.
4. `sheet_->setCutPage(p == pCut_)`, `view3d_->setCutPage(p == pCut_)`.
5. `p == pCut_` 이면: (006b) 지금 슬롯이 가로(Horizontal)면 첫 종단면 슬롯으로 `slot_` 만 바꾼다(**각도는 바꾸지 않음** — `selectSlot` 의 저장 각 복원을 거치지 않는다), (006a) `setHandTool("cut")`.
6. 확정된 문서면 `refreshAll(false)`(제목·상태줄·각도 칸·ctx 단축키 보임 갱신, 다시 계산 없음). 확정 전이면 부르지 않는다(②에서 넘어온 `requirePose` 안내를 지우지 않게).

**「입력 규칙」**:
- (006b) `onAction("cut")`(홈 「단면선」 타일·`S` 키, 836-837): `ribbon_->setCurrentPage(hand_ == "cut" ? pHome_ : pCut_)`. 타일 툴팁(169-170) `② 막으로 자르기 (S) — 막(절단면 A–A′)을 돌려 단면을 봅니다 · Artec 미스캔 내벽 파선 자동 보정(W)`.
- (006b) `Esc`(945): ② 에 있으면 `setCurrentPage(pHome_)`, 아니면 지금처럼 `setHandTool(QString())`.
- (006b) `onAction("outline")`: ② 에 있으면 먼저 `setCurrentPage(pHome_)` 뒤 지금처럼 토글.
- (006a) `confirmPose`(647-661): 657 `setCurrentPage(pHome_)` → `setCurrentPage(pCut_)`.
- (006a) 잠김: `refreshAll`(532-) **확정 전 분기(545-558)보다 앞에서** `ribbon_->setPageLocked(pCut_, doc_.isLoaded() && !doc_.poseConfirmed(), cutpage::kLockTip)`.
- (006a) `setHandTool`(664-678) `"cut"` 의 배지·안내(673-674) = `kCtxBadge`·`kCtxHint`, 안내 칸 툴팁 = 같은 문구.

**「3D 끌기」** (006b, `View3DCanvas`, View3D.cpp:180-205 / View3D.h:37):
- 누를 때 `cutDrag_ = cutTool_ && e->button() == Qt::LeftButton`(새 멤버). 끌 때 `cutDrag_` 면 지금처럼 `emit cutAngleDragged(d.x() * 0.5)`, 아니면 지금의 보는 방향 돌리기(yaw·pitch, 194-197). 놓을 때 `drag_ && cutDrag_` 였을 때만 `cutDragReleased`. → ② 에서 왼쪽 끌기 = 회전판, 오른쪽·가운데 끌기 = 보는 방향. 도구를 들지 않았을 때는 어느 단추든 지금처럼 보는 방향.
- `View3DPane::setCutPage(bool on)` 에서 `canvas_->setToolTip(on ? cutpage::k3dTip : QString())`.

### 3.6 표시 갱신

1. **상태줄**: `refreshAll` 끝(확정 뒤 분기, 563-568 다음)에 `if (ribbon_->currentPage() == pCut_) setHint(cutpage::kStatus);`.
2. **도판 머리**(`SheetPane`, SheetView.h:51-71 / SheetView.cpp:241-282): `void setCutPage(bool)` 멤버 깃발. `refresh` 에서 깃발이 켜져 있고 지금 슬롯이 `Vertical` 이면 `title_ = plateTitle(s.name.at(0), doc.previewAngle())`, `sub_ = plateSub(doc.sectionSide())`(+ 기존 미스캔 꼬리 251). 아니면 지금 그대로. `refreshPreview`(721-726)가 이미 `sheet_->refresh` 를 부르므로 끄는 동안 제목 각도도 따라간다.
3. **3D 머리**(`View3DPane`, View3D.h:42-57 / View3D.cpp:262-279): `void setCutPage(bool)` 깃발. 확정 뒤 + 깃발 → `<b>` + `k3dTitle` + `</b>`. 확정 뒤 기본 제목(222, 267)의 `절단면 · 중심축` → `막 · 중심축`.
4. **`updateCutInfo`**(571-577, `refreshAll`·`refreshPreview` 양쪽에서 불림): ② 에 있으면 `ctxRight_ = ctxRight(previewAngle(), lod 면 수(비면 원본), 원본 면 수, isPreviewing())`, 아니면 `점 %1 → <b>%2개</b> (%3 mm 간략화)`(옛 `절단 각도 … (정면 중심축)` 앞부분 삭제). 그리고 언제나 `CutAngle = angleText(previewAngle())`, `cutAxisGroup_->setTitle(axisGroupName(previewAngle()))`, 타일 `axX`/`axY` 상태 = `atAxis(…)` ? `On` : `Idle`.
5. **ctx 폭**(006b, JOB-005 §5-11): ctx 단축키 묶음(284-290)에서 `[ ]` · `절단면 1° 이동` 두 위젯을 뺀다(`, . [ ]` 키 자체는 그대로 동작 — ← → 의 별칭). **② 에서는 리본과 겹치는 두 묶음 `← →`·`막 1° · Shift 15°` 와 `R`·`좌/우 반전` 을 숨긴다**(리본 ② 의 `막 움직이기` 줄 2·`단면 위치` 단추 kbd 에 같은 배지가 있다 — R8 유지). 이 네 위젯을 멤버 목록(예: `ctxDupKeys_`)으로 따로 들고, `refreshAll` 의 보임 갱신(543)을 `!base && !(currentPage()==pCut_ && 겹치는 위젯)` 으로 바꾼다(홈에서 외곽선 도구를 들면 다시 보임). `ctxHint_->setMinimumWidth(1)`(긴 안내가 창 최소 폭을 밀지 않게; 잘리면 툴팁으로 전체를 본다). 1366 px 실제 확인은 사람 검토 7.
6. **시험용 objectName**: `ctxHint_` = `CtxHint`, `ctxRight_` = `CtxRight`, `stHint_` = `StHint`, 각도 칸 = `CutAngle`, SheetPane `title_`/`sub_` = `SheetTitle`/`SheetSub`, View3DPane `title_` = `View3DTitle`(기존 `CtxBar`·`ToolBadge`·`SlotList` 와 같은 방식).

### 3.7 각도 입력 — 모두 JOB-005 입력 합치기 규칙 안에서

- `[−]`·`[+]`(id `cutMinus`·`cutPlus`): 지금 `rotate`·`cutMove` 자리(832-835)를 `beginCutInput(); cut_.click(doc_.frontAngle(), id == "cutMinus" ? -1.0 : 1.0, now); scheduleCut();` 로(**`cut_.click(` 호출은 한 곳**). `needPose`(796-797)에서 `rotate`·`cutMove` 를 빼고 `cutMinus`·`cutPlus`·`axX`·`axY` 추가.
- 775 의 먼저-확정 예외 id 를 `cutMinus`·`cutPlus`·`axX`·`axY` 넷으로.
- `X 막`·`Y 막`(id `axX`·`axY`): 절대각 맞춤이라 델타 합치기 대상이 아니다. 분기:
  ```cpp
  const double goal = id == "axX" ? 90.0 : 0.0;
  const bool pending = cut_.active();               // 끄는·누르는 중 막이 있으면
  if (!pending && std::fabs(std::remainder(goal - doc_.frontAngle(), 360.0)) <= 1e-9) return;  // 이미 그 각 — 계산 안 함
  if (pending) cancelCut(); else pushUndo();        // 진행 중이면 beginCutInput 이 이미 되돌리기 한 칸을 쌓았다
  QApplication::setOverrideCursor(Qt::WaitCursor);  // 원본 다시 계산 ≈ 1.4 s (확정·자동 세우기·중심축 맞춤과 같게)
  doc_.alignCut(id == "axX" ? CutAxis::X : CutAxis::Y);  // 미리보기도 함께 지운다(recompute → clearPreview)
  QApplication::restoreOverrideCursor();
  refreshAll(true);
  ```
  진행 중 제스처를 **확정하지 않고 취소**하므로 원본 계산은 한 번(JOB-005 §5-6 의 "두 번 계산 ≈ 2.8 s" 를 피함). 진행 중이었으면 이미 그 각이어도 다시 계산한다(미리보기를 지우는 공개 경로가 `alignCut` 뿐 — ArtifactDoc.cpp:607-644). `axZ` 는 사용 불가라 눌리지 않는다.
- 키·끌기는 JOB-005 그대로. `previewFrontAngle` 호출은 MainWindow 에서 계속 한 곳(709-712).

### 3.8 용어 정리 (`막` · `회전판` 으로 — 디자인 용어표 `H` 774-775: UI 쉬운 말 `막`·`회전판`, 보고서 라벨 `절단면 A–A′`·`정면 방위각`, 툴팁은 둘 다)

- 홈 `유물 정위치 · 중심축` 칸에서 `{"rotate", "정면 회전 ±1°", ", ."}`(165), `실측선 추출` 칸에서 `{"cutMove", "절단 위치 이동 (1°)", "[ ]"}`(173) **단추를 뺀다** — ② 의 [−][+]·← → 로 대신(`, . [ ]` 키는 남음). 둘 다 이름과 달리 +1° 만 하던 단추다.
- ctx `절단면 1° 이동`(284) 삭제(006b), `✂ 단면·외곽선 편집 중`·`3D 뷰를 좌우로 끌어 절단면 A–A′ 각도를…`(673-674) → §3.5, ctx 오른쪽 `절단 각도 … (정면 중심축)`(572) → §3.6-4.
- 슬롯 메모·안내 `절단 각도 %1°`(750, 753, 758-759) → `막 ` + `angleText(frontAngle())`(예: `막 12°`, 안내 `슬롯 1 에 막 12° 를 저장했습니다.`).
- 3D 기본 제목 `절단면 · 중심축`(View3D.cpp:222, 267) → `막 · 중심축`.
- 코드 주석은 고치지 않아도 된다(검사는 `QStringLiteral` 문자열만).

### 3.9 검수용 `--cut` (`studio/src/main.cpp`, 006b)

- 사용법 주석(24-28)에 `--cut — S 로 ② 막으로 자르기에 가서 → 를 12번(디자인 시안의 12°) 누른 뒤 찍는다. 예: DongArch3DStudio.exe --sample --cut --shot shot_cut.png` 추가.
- `--shot` 블록 **밖**(캡처 없이도 ② 12° 로 열리게)에서 `QTimer::singleShot(200, &w, …)`: `Key_S` KeyPress 1번, 그다음 `Key_Right` KeyPress·KeyRelease(자동 반복 아님) 12쌍을 `QApplication::sendEvent(&w, …)`. 실제 키 경로(합치기 → 300 ms 뒤 원본 확정)를 그대로 지난다. 캡처 타이머(1800 ms)는 확정 계산(예제 ≈ 1.4 s, UI 스레드) 뒤에 돈다.

### 3.10 정본 `docs/ui/UI_RULES.md` (사람 검토 1)

- R1 표 1행(20): 탭 줄을 `` `파일 · 홈 · ① 바닥 정하기 · ② 막으로 자르기 · 측정·탁본 · 내보내기` `` 로, 끝에 `. 바닥 확정 전 \`② 막으로 자르기\` 는 숨기지 않고 \`Faint\` 글자 + 툴팁 「바닥 확정 후 사용」(누르면 ① 로 안내)` 추가.
- R1 표 3행(22) 끝에 `· ② 막으로 자르기 탭 = 「✂ 막으로 자르기」를 든 상태(다른 탭·\`Esc\` 로 내려놓음)` 추가.
- R8(95): 예시 `` `[ ]` `` 를 `` `← →` `` 로 바꾸고 문장 끝에 `` `, . [ ]` 는 `← →` 의 별칭이라 배지를 따로 두지 않는다(툴팁에 적음). `` 추가 — 이번에 `[ ]` 배지가 화면에서 모두 사라지므로(§3.6-5·§3.8) 규칙의 예시가 거짓이 되지 않게.

### 3.11 빌드·시험 (`studio/CMakeLists.txt`)

- `studio_core`(55-61)에 `src/CutPageText.h`.
- 앱 원본 목록(72-79의 MainWindow·Ribbon·SheetView·View3D·StartPage·ExportDialog)을 변수 `STUDIO_GUI_SRC` 로 묶어 앱(`src/main.cpp ${STUDIO_GUI_SRC}`)과 새 시험이 함께 쓴다(두 번 컴파일 — 앱 링크 구조는 그대로). 102 다음: `add_executable(test_studio_cutpage tests/test_cutpage.cpp ${STUDIO_GUI_SRC})`, `AUTOMOC ON`, `target_link_libraries(test_studio_cutpage PRIVATE studio_core Qt6::Widgets)`, `add_test(NAME studio_cutpage COMMAND test_studio_cutpage ${CMAKE_CURRENT_BINARY_DIR})`, `set_tests_properties(studio_cutpage PROPERTIES RUN_SERIAL TRUE)`(원본 계산 ≈ 11번 + 화면 갱신 — CI `--parallel 4` 에서 다른 시험과 겹치지 않게, 101-102 의 `studio_cut` 과 같은 방식). (앱 쪽 PCH 81-85 는 그대로.)
- 시험 `main`: `qputenv("QT_QPA_PLATFORM","offscreen")` → `QApplication` → **`setOrganizationName("DongArch3DTest")`·`setApplicationName("DongArch3DTest")`**(사용자 QSettings·최근 목록·감시 폴더를 건드리지 않게) → `app.setFont(theme::uiFont(13))`·`app.setStyleSheet(theme::styleSheet())`(탭 글자색 판정에 필요). 위젯은 objectName·`Ribbon::tabsBar()` 의 `tab` 속성 단추·`w.findChild<View3DCanvas*>()`·리본 안 `QStackedWidget` 으로 찾는다.
- 도움 함수: `wait(cond)` = `QElapsedTimer` + `QCoreApplication::processEvents(QEventLoop::AllEvents, 10)` 를 조건이 참이 될 때까지(한 번에 최대 15 s — CI 느림 대비, 넘으면 FAIL). `settle()` = 조건 없이 100 ms 동안 같은 처리 — **위치(`mapTo`)·폭(`width()`)을 읽기 전에는 늘 부른다**(`setTitle`·`setText` 뒤 다시 배치는 `LayoutRequest` 로 미뤄진다). 마우스는 `QMouseEvent(type, QPointF 로컬, QPointF 전역(= canvas->mapToGlobal), button, buttons, Qt::NoModifier)`(Qt 6 비권장 아닌 생성자 — 빌드 경고 0)를 `QApplication::sendEvent(canvas, &ev)`.

## 4. 수용 기준 (각 항목은 명령 하나로 참/거짓)

허용오차 근거: 이번 JOB 은 계산을 바꾸지 않으므로 모든 화면 문자열은 **정확히 같음**(`==`, 앞부분 비교는 `startsWith`, 뒷부분은 `endsWith`), 각도 판정 경계 1°·0.1° 반올림은 원문(`H` 1460, 표기 정수 `12°`)에서, 기다림 상한 15 s 는 판정값이 아닌 실패 감지용(JOB-005 실측 원본 계산 1.36 s 의 10배+). **시간에 따라 결과가 갈리는 판정은 없다** — 미리보기("끄는 중") 확인은 왼쪽 단추를 누른 채 끌기(놓을 때까지 확정 마감이 없음, CutGesture.h:16-19)로 하고, 나머지 기다림은 상한만 있다.

### A. 새 CTest `studio_cutpage` (`studio/tests/test_cutpage.cpp`, 실행 파일 `test_studio_cutpage`, 오프스크린 `QApplication`)

**A-1 표기 함수 (`CutPageText.h`)**
1. `angleText`: 0→`0°`, 12→`12°`, 37.5→`37.5°`, −20→`−20°`(U+2212), −0.04→`0°`, 180→`180°`, 12.04→`12°`, 12.06→`12.1°`, −179.5→`−179.5°` 모두 `==`.
2. `plateTitle(QChar('A'), 12) == "A–A′ 실시간 단면 — 12°"`; `plateSub(SectionSide::Right) == "— 3D에서 막을 움직이는 동안 바로 다시 그림 · 외면 왼쪽 · 단면 오른쪽"`; `plateSub(SectionSide::Left) == "— 3D에서 막을 움직이는 동안 바로 다시 그림 · 단면 왼쪽 · 외면 오른쪽"`.
3. `atAxis(·, Y)` 참: 0, 1, −1, 179, 179.5, 180, −180 / 거짓: 1.5, 12, 90, −20. `atAxis(·, X)` 참: 90, 89, 91, −90, −89 / 거짓: 88.5, 92, 0, 105. `axisGroupName(0) == axisGroupName(90.5) == "막 방향 (누르면 바로 맞춤)"`, `axisGroupName(12) == "막 방향 · 지금 12° (누르면 맞춤)"`, `axisGroupName(-20) == "막 방향 · 지금 −20° (누르면 맞춤)"`.
4. `faceText`: 26180→`2.6만 면`, 77000→`7.7만 면`, 35000→`3.5만 면`, 250000→`25만 면`, 10000→`1만 면`, 9800→`9,800 면`. `ctxRight(12, 26180, 77000, false) == "막 12° · 끄는 동안 2.6만 면 미리보기 · 놓으면 원본 7.7만 면 다시 계산"`, `ctxRight(12, 26180, 77000, true) == "막 12° · 끄는 중 2.6만 면 미리보기 — 음영·외곽·치수·두께는 놓은 뒤"`.

**A-2 Ribbon 단독**
5. `Ribbon rb;` 페이지 `가`·`나` 추가, `pageChanged` 를 람다로 모음 → `rb.setCurrentPage(1)` 뒤 받은 값 `[1]`·`currentPage()==1`; 탭 단추 `가` 를 `click()` → 받은 값 끝 `0`·`currentPage()==0`.
6. `setPageLocked(1, true, "이유")` → 탭 `나` 의 속성 `locked==true`·`toolTip()=="이유"`; `setPageLocked(1, false, "이유")` → `locked==false`·`toolTip()` 비어 있음.
7. `g = rb.addGroup(0, "옛 이름")` → `g->setTitle("새 이름")` 뒤 `g` 안 `gname` QLabel 글자 `== "새 이름"`. `b = g->button("bt", "+")`, `g->rows({{b}})` → `b->click()` 에 `triggered("bt")` 1번; `rb.setState("bt", ToolState::On)` → `b` 속성 `state == "on"`.

**A-3 화면 통합** (`MainWindow w; w.resize(1560, 920); w.show();` — 예제 메시, 아래 순서대로 한 실행 안에서. ② = 탭·페이지 번호 3, ① = 2, 홈 = 1)
8. **탭 순서·잠김**: `w.openSample(true)`(확정 전) 뒤 `tab` 속성 단추 글자가 차례로 정확히 `파일`·`홈`·`① 바닥 정하기`·`② 막으로 자르기`·`측정·탁본`·`내보내기`(6개), `currentPage()` = ① 의 번호, ② 탭 `locked==true`·툴팁에 `바닥 확정 후 사용` 포함·**`palette().color(QPalette::ButtonText) == theme::Faint`**. ② 탭 `click()` → `currentPage()` 는 여전히 ① 번호, `StHint` 에 `먼저 ① 바닥 정하기에서` 포함, `ToolBadge == "▣ 바닥 정하기"`, `CtxBar` 보임, ② 탭 글자색 여전히 `theme::Faint`.
9. **확정 → ②**: `Key_Return` KeyPress 를 `w` 에 보냄 → `currentPage()` = ② 번호, ② 탭 `locked==false`·툴팁 비어 있음·글자색 `== theme::Ink`(현재 탭), `CtxBar` 보임, `ToolBadge == "✂ 막으로 자르기"`, `CtxHint == kCtxHint` 이고 그 툴팁도 같음, `StHint == kStatus`, `SheetTitle == "A–A′ 실시간 단면 — 0°"`, `SheetSub` 가 `plateSub(Right)` 로 시작, `View3DTitle` 에 `3D — 막으로 자르기` 포함, `CutAngle == "0°"`, `막 방향` 으로 시작하는 `gname` 라벨 `== "막 방향 (누르면 바로 맞춤)"`, 타일 `axY` `toolState()==On`·`axX` `Idle`·`axZ` `isEnabled()==false` 이고 툴팁이 `🔒` 로 시작하고 `다음 판` 포함, `CtxRight` 가 `막 0° · 끄는 동안 ` 로 시작하고 ` 다시 계산` 으로 끝남, 타일 글자 `axX == "X 막 옆에서"`·`axY == "Y 막 정면에서"`·`axZ == "Z 막 높이로"`, 리본 안 `gname` 라벨에 `막 움직이기 (회전판)`·`단면 위치` 가 있고 리본 안 QLabel 글자에 `☑ 중심선에 고정 (돌리기만)`·`각도`·`끄는 동안 실시간`·`국내 보고서 표준`·`사용자 도판으로 확인` 이 각각 있음. **리본 높이 불변**: `axX`·`axY`·`axZ` 의 `sizeHint().height()` 가 홈 `cut` 타일의 `sizeHint().height()` 와 같고, 리본 안 `QStackedWidget` 의 `widget(3)->sizeHint().height() <= widget(2)->sizeHint().height()`. `settle()` 뒤 **고정 폭**: `막 방향` `gname` 라벨 `width() >= fontMetrics().horizontalAdvance(axisGroupName(-179.5))`, `CutAngle` `width() >= fontMetrics().horizontalAdvance("−179.5°")`. 인쇄(기록만): `[9] ribbon height=… px (JOB-005 계산값 107)`, `[9] ctxHint width=… / sizeHint=… px · ctx minimumSizeHint width=… px (offscreen 글꼴)`, `[9] 단면 위치 x=…`(= `단면 위치` `gname` 라벨 `mapTo(ribbon, QPoint()).x()`).
10. **X 막 (진행 중 제스처 취소 포함)**: `cutPlus` `click()` 바로 뒤(이벤트 처리 없이) `axX` `click()` → `SheetTitle == "A–A′ 실시간 단면 — 90°"`, `CutAngle == "90°"`, `axX` `On`·`axY` `Idle`, `StHint == kStatus`; `settle()` 를 5번(500 ms) 해도 `SheetTitle` 은 `— 90°`(취소된 [+] 가 300 ms 뒤 확정되지 않음). `Ctrl+Z` KeyPress → `SheetTitle == "A–A′ 실시간 단면 — 0°"`·`CtxRight` 가 `막 0° · 끄는 동안 ` 로 시작([+] 와 X 막이 되돌리기 한 칸 — 먼저 확정했다면 `— 1°`). 다시 `axX` `click()` → `SheetTitle == "A–A′ 실시간 단면 — 90°"`.
11. **[+] 두 번 = 합치기 한 번 / 끄는 중 표시**: `cutPlus` 를 이어서 두 번 `click()` → 바로 뒤(이벤트 처리 전) `SheetTitle` 은 아직 `— 90°`. `CtxRight` 가 `막 92° · 끄는 동안 ` 로 시작할 때까지 기다림 → `SheetTitle == "A–A′ 실시간 단면 — 92°"`, `CutAngle == "92°"`, 그룹 이름 `== "막 방향 · 지금 92° (누르면 맞춤)"`, `axX`·`axY` 모두 `Idle`. `Ctrl+Z` → `SheetTitle == "A–A′ 실시간 단면 — 90°"`(제스처 하나 = 되돌리기 한 칸). 이어서 `View3DCanvas` 에 `MouseButtonPress`(LeftButton, (100,100))·`MouseMove`(buttons LeftButton, (104,100)) **만** 보냄 → `CtxRight` 가 `막 92° · 끄는 중 ` 으로 시작하고 `— 음영·외곽·치수·두께는 놓은 뒤` 로 끝날 때까지 기다림, `SheetTitle == "A–A′ 실시간 단면 — 92°"`; `settle()` 5번 뒤에도 `CtxRight` 는 `막 92° · 끄는 중 ` 으로 시작(누르고 있는 동안 확정 없음). `MouseButtonRelease`(LeftButton, (104,100)) → `CtxRight` 가 `막 92° · 끄는 동안 ` 으로 시작할 때까지 기다림. `Ctrl+Z` → `— 90°`. **리본 고정**: `settle()` 뒤 `[11] 단면 위치 x=…` 인쇄, 9 의 값과 같음.
12. **[−]**: `cutMinus` `click()` → `CtxRight` 가 `막 89° · 끄는 동안 ` 으로 시작할 때까지 기다림 → 그룹 이름 `== "막 방향 (누르면 바로 맞춤)"`, `axX` `On`.
13. **키·오른쪽 끌기**: `Key_Right` KeyPress·KeyRelease(NoModifier) → `막 90° · 끄는 동안 ` 까지 기다림; `Key_Right` KeyPress·KeyRelease(ShiftModifier) → `막 105° · 끄는 동안 ` 까지 기다림 → `SheetTitle == "A–A′ 실시간 단면 — 105°"`, 그룹 이름 `== "막 방향 · 지금 105° (누르면 맞춤)"`, `settle()` 뒤 `[13] 단면 위치 x=…` 인쇄·9 의 값과 같음. `View3DCanvas` 툴팁 `== k3dTip`. `img0 = View3DCanvas grab().toImage()` → `MouseButtonPress`(**RightButton**, (100,100))·`MouseMove`(buttons RightButton, (140,120))·`MouseButtonRelease`(RightButton, (140,120)) → `settle()` 뒤 `CtxRight` 가 여전히 `막 105° · 끄는 동안 ` 으로 시작(끄는 중 아님), `SheetTitle` 은 `— 105°`, 새 `grab().toImage() != img0`(보는 방향이 돌았음).
14. **R 반전**: `Key_R` → objectName `combined` 인 QPushButton 이 정확히 2개(홈·②)이고 둘 다 글자 `단면 왼쪽·외면 오른쪽 (반전)`, `SheetSub` 가 `plateSub(Left)` 로 시작. `Key_R` 다시 → 둘 다 `외면 왼쪽·단면 오른쪽`, `SheetSub` 가 `plateSub(Right)` 로 시작.
15. **페이지 = 도구·ctx 겹침 숨김**: (②) `CtxBar` 안 글자 `막 1° · Shift 15°`·`좌/우 반전` 인 QLabel 이 둘 다 `isVisible()==false`. `Key_Escape` → `currentPage()` = 홈, `CtxBar` 숨김, `SheetTitle == "A–A′ 정면 실측도"`, `View3DTitle` 에 `막으로 자르기` 없음, `View3DCanvas` 툴팁 비어 있음, ② 탭 글자색 `== theme::Ink2`. `Key_S` → ② 번호, `CtxBar` 보임, `ToolBadge == "✂ 막으로 자르기"`. 탭 `측정·탁본` `click()` → `CtxBar` 숨김. `Key_S` → ②. `Key_O` → `currentPage()` = 홈, `ToolBadge == "◠ 외곽선 편집 중"`, `CtxBar` 보임, 위 두 QLabel 이 둘 다 `isVisible()==true`.
16. **옛 단추 없음**: `w.findChildren<QWidget*>("rotate")`·`("cutMove")` 모두 비어 있음.
17. **가로 슬롯에서 ② 로**: (15 끝, 홈) `SlotList` `setCurrentRow(1)`(가로 슬롯 `B–B′ 구연부 횡단면`) → `SheetTitle == "B–B′ 구연부 횡단면"`. `Key_S` → `SlotList` `currentRow()==0`, `SheetTitle == "A–A′ 실시간 단면 — 105°"`(각은 그대로).
18. `test_studio_cutpage` 는 실패가 없으면 마지막 줄 `PASS`, 종료 코드 0.

### B. 회귀

19. CTest 16종(기존 15 + `studio_cutpage`): `B` 에서 `ctest --output-on-failure` 출력에 `100% tests passed` 와 `out of 16` 이 같은 줄에 있고, `tests failed` 가 0 이 아닌 줄이 없다(이 PC·CI 는 `, 0 tests failed` 를 찍지 않음 — JOB-005 decision.md).
20. 계산 값 그대로(verify.ps1 직후 `B\Testing\Temporary\LastTest.log`): `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=85/1318 lodFaces=26180"` ≥ 1, `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"` ≥ 1, `grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"` ≥ 1 (기준: `D:/_dongarch_build/wt/factory/jobs/JOB-005/artifacts/LastTest_verify.log` — 세 줄 모두 있음).
21. verify.ps1 종료 코드 0, `B\verify-summary.md` 마지막 줄 `결과: PASS`, 빌드 경고 0: `grep -ci "warning" D:/_dongarch_build/verify_job006/studio-logs/02_build.log` = `0`.
22. 엔진·기존 시험 무변경: `git diff --name-only 804f60d -- studio/src/ArtifactDoc.h studio/src/ArtifactDoc.cpp studio/src/CutGesture.h studio/src/Plate.h studio/src/Plate.cpp studio/src/MeshIO.h studio/src/MeshIO.cpp studio/src/MeshRaster.h studio/src/MeshRaster.cpp studio/tests/test_pipeline.cpp studio/tests/test_pose.cpp studio/tests/test_side.cpp studio/tests/test_cut.cpp` → 출력 없음.

### C. 코드·정본 (Git Bash, `cd W`. **builder 가 커밋한 뒤** `git status --porcelain` 출력이 없는 상태에서 잰다 — 그래야 `git diff 804f60d`(작업 트리)와 `804f60d HEAD`(커밋)가 같은 것을 본다)

23. 원문 라벨 있음: `for s in "② 막으로 자르기" "막 방향 (누르면 바로 맞춤)" "(누르면 맞춤)" "막 움직이기 (회전판)" "단면 위치" "X 막 옆에서" "Y 막 정면에서" "Z 막 높이로" "🔒 Z 막 높이로" "☑ 중심선에 고정 (돌리기만)" "끄는 동안 실시간" "국내 보고서 표준" "사용자 도판으로 확인" "✂ 막으로 자르기" "⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고" "실시간 단면 — " "— 3D에서 막을 움직이는 동안 바로 다시 그림 · " "3D — 막으로 자르기" "바닥 확정 후 사용" "→ ② 막으로 자르기" "음영·외곽·치수·두께는 놓은 뒤" ", [ 도 같음" ". ] 도 같음"; do grep -rqF "$s" studio/src || echo "MISSING $s"; done` → 출력 없음.
24. 옛 용어·단추 없음: `grep -rnE 'QStringLiteral\("[^"]*(정면 회전|절단 위치 이동|절단면 1°|절단 각도|단면·외곽선 편집 중|절단면 A–A′ 각도|절단면 · 중심축)' studio/src` → 출력 없음, `grep -cE '"rotate"|"cutMove"' studio/src/MainWindow.cpp` = `0`.
25. 페이지 신호·잠김·그룹 이름: `grep -c "void pageChanged(int" studio/src/Ribbon.h` = `1`, `grep -c "emit pageChanged" studio/src/Ribbon.cpp` ≥ `1`, `grep -c "setPageLocked" studio/src/MainWindow.cpp` ≥ `1`, `grep -c "pageChanged" studio/src/MainWindow.cpp` ≥ `1`, `grep -c "setTitle(" studio/src/MainWindow.cpp` ≥ `1`, `grep -c 'locked="true"' studio/src/Theme.h` = `1`.
26. 입력 합치기 불변·흐름: `grep -c "previewFrontAngle" studio/src/MainWindow.cpp` = `1`, `grep -c "setFrontAngle(doc_.frontAngle()" studio/src/MainWindow.cpp` = `0`, `grep -c "cut_.click(" studio/src/MainWindow.cpp` = `1`, `grep -c "alignCut(" studio/src/MainWindow.cpp` ≥ `1`, `grep -c "setOverrideCursor(Qt::WaitCursor)" studio/src/MainWindow.cpp` ≥ `7`(지금 6 + X/Y 맞춤), `sed -n '/^void MainWindow::confirmPose/,/^}/p' studio/src/MainWindow.cpp | grep -c "setCurrentPage(pCut_)"` = `1`.
27. R2·R3: `git diff 804f60d -- studio/src | grep -E '^\+.*(QColor\([0-9]|qRgba?\([0-9]|#[0-9A-Fa-f]{6}|Qt::(red|green|blue|yellow|white|black|gray|darkGray|lightGray)\b)'` → 출력 없음, `grep -rniE "c8641e|3f8f4f|3f6fb5|6b4fc0" studio/src` → 출력 없음, `git diff 804f60d -- studio/src | grep -c '^\+.*ToolState::Primary'` = `0`, `git diff 804f60d -- studio/src/Theme.h | grep -c '^+inline const QColor'` = `0`.
28. (006b 몫) ctx 폭·R8·`--cut`·3D 끌기: `grep -c "ctxHint_->setMinimumWidth(1)" studio/src/MainWindow.cpp` = `1`, `grep -c 'kbdLabel("\[ \]"' studio/src/MainWindow.cpp` = `0`, `grep -c 'kbdLabel("Shift"' studio/src/MainWindow.cpp` ≥ `2`(ctx ① ·리본 ②), `grep -c 'kbdLabel("← →"' studio/src/MainWindow.cpp` ≥ `2`(ctx·리본), `grep -c '"--cut"' studio/src/main.cpp` ≥ `1`, `grep -c "Qt::LeftButton" studio/src/View3D.cpp` ≥ `1`, `grep -rqF "오른쪽 끌기 = 보는 방향만" studio/src && echo OK` → `OK`.
29. 정본: `grep -c "① 바닥 정하기 · ② 막으로 자르기 · 측정·탁본 · 내보내기" docs/ui/UI_RULES.md` = `1`, `grep -c "바닥 확정 후 사용" docs/ui/UI_RULES.md` ≥ `1`, `grep -c "✂ 막으로 자르기" docs/ui/UI_RULES.md` ≥ `1`, `grep -cF '`Tab`, `← →`)' docs/ui/UI_RULES.md` = `1`, `grep -cF '`Tab`, `[ ]`)' docs/ui/UI_RULES.md` = `0`.
30. 변경 범위(허용 목록, 커밋 뒤): `git status --porcelain` → 출력 없음, 그리고 `git diff --name-only 804f60d HEAD | grep -vxF -e studio/src/CutPageText.h -e studio/src/Ribbon.h -e studio/src/Ribbon.cpp -e studio/src/Theme.h -e studio/src/MainWindow.h -e studio/src/MainWindow.cpp -e studio/src/SheetView.h -e studio/src/SheetView.cpp -e studio/src/View3D.h -e studio/src/View3D.cpp -e studio/src/main.cpp -e studio/tests/test_cutpage.cpp -e studio/CMakeLists.txt -e docs/ui/UI_RULES.md` → 출력 없음.

### D. 캡처 (31-33 은 ui-reviewer 예/아니오 — 오프스크린 한글 □ 는 알려진 위험 7, 배치만 본다. 34 는 명령 판정)

31. 검증 명령 3 의 `B\shots\shot_cut.png` 가 `> 5120` 바이트이고: 탭 6개 중 4번째가 현재(밑줄), 리본에 그룹 4개(파일 타일 2 · 막 방향 타일 3 — 글자 한 줄, 셋째 타일이 흐림 · 막 움직이기 세 줄, 둘째 줄에 `[−] 12° [+]` 와 kbd 배지 2개 · 단면 위치 단추 + `R` 과 두 줄 글), 리본 높이가 shot_main 과 같음, 그 아래 ctx 줄이 보이고 왼쪽 끝에 어두운 배지, 리본 그룹 이름·도판 머리·ctx 오른쪽에 숫자 `12°` 가 보임, 도판 빨간 단면이 중심축 오른쪽·먹 외곽이 왼쪽, 3D 의 굵은 절단선이 오른쪽 반.
32. 검증 명령 3 의 `B\shots\shot_base.png`(`--sample --base`)가 `> 5120` 바이트이고: 현재 탭은 3번째(① 바닥 정하기), 4번째 탭 글자가 다른 탭보다 옅음(`Faint` — 명령 판정은 기준 8), ctx 줄에 어두운 배지(▣ 바닥 정하기).
33. verify.ps1 의 `shot_main.png` 가 `> 5120` 바이트이고, JOB-005 산출물 `D:/_dongarch_build/wt/factory/jobs/JOB-005/artifacts/shot_main.png` 와 나란히 보아 다른 곳이 (a) 탭 줄에 탭 1개 추가 (b) 홈 리본 `유물 정위치 · 중심축`·`실측선 추출` 의 작은 단추 칸이 3개 → 2개(그에 따른 오른쪽 그룹들의 가로 위치 이동 포함) (c) 3D 머리 부제 글자 뿐이다 — 리본·작업 공간·상태줄의 세로 위치와 높이, 도판·치수 띠 값은 같다(`--sample` 은 홈에 머문다).
34. `shot_home.png`·`shot_export.png` 는 이번에 바꾸지 않는 StartPage·ExportDialog 화면이므로 JOB-005 와 바이트까지 같다: PowerShell `foreach ($n in 'shot_home.png','shot_export.png') { (Get-FileHash "D:\_dongarch_build\verify_job006\studio\shots\$n").Hash -eq (Get-FileHash "D:\_dongarch_build\wt\factory\jobs\JOB-005\artifacts\$n").Hash }` → `True` 두 줄(JOB-005 도 JOB-004 와 MD5 같았음 — JOB-005 승인 보고서 §2).

## 5. 검증 명령

1. 표준 검증(작업 폴더 `W`): `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-006 -BuildRoot D:\_dongarch_build\verify_job006 -Job JOB-006 -RecordRoot D:\_dongarch_build\wt` (로그 `L\01_configure.log … 04_shots.log`, 산출물 `D:/_dongarch_build/wt/factory/jobs/JOB-006/artifacts/`). 직후 수용 기준 19·20·21·34.
2. 개별 시험(PowerShell, `--test-dir` 금지 — 빌드 폴더로 이동): `Remove-Item Env:\CI -ErrorAction SilentlyContinue; $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job006\studio; ctest -R studio_cutpage -V` → 수용 기준 1-18(`[9]`·`[11]`·`[13]` 인쇄, 마지막 `PASS`). 개별 실행은 LastTest.log 를 덮어쓰므로 기준 20 은 1 직후 또는 전체 `ctest --output-on-failure` 재실행 뒤에 본다.
3. 캡처(verify.ps1 무변경, 2 와 같은 환경 변수 뒤): `$b='D:\_dongarch_build\verify_job006\studio'; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--cut','--shot',"$b\shots\shot_cut.png"; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--base','--shot',"$b\shots\shot_base.png"; (Get-Item "$b\shots\shot_cut.png").Length; (Get-Item "$b\shots\shot_base.png").Length; Copy-Item "$b\shots\shot_cut.png","$b\shots\shot_base.png" D:\_dongarch_build\wt\factory\jobs\JOB-006\artifacts\` → 기준 31·32. (Git Bash 에서는 명령 전체를 작은따옴표로 감싼 `powershell -NoProfile -Command '…'`.)
4. 수용 기준 22-30 은 builder 커밋 뒤 Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-006` 뒤 그대로.
5. 캡처 비교: `shot_main.png` 를 JOB-005 `artifacts/shot_main.png` 와 나란히(기준 33), `shot_cut.png` 를 `H` 의 ① 주 화면 ② 단계(기본 각 12°)와 나란히(기준 31), `shot_home`·`shot_export` 는 해시(기준 34 — 다르면 차이 원인을 decision.md 에 적는다).
6. verify.md 에 `[9]`(리본 높이·ctx 폭·x)·`[11]`·`[13]` 줄을 그대로 옮긴다(사람 검토 7 자료).

## 6. 영향 파일

| 파일 | 변경 | 이유 |
|---|---|---|
| `studio/src/CutPageText.h` | 신규 | ② 원문 라벨 상수·표기 함수(§3.2) — 문자열 시험 대상 |
| `studio/src/Ribbon.h` · `Ribbon.cpp` | 수정 | `pageChanged`·`currentPage`·`setPageLocked`(h:74-91, cpp:265-307), `RibbonGroup::setTitle/button/rows`(줄 높이 23 고정, h:53-67, cpp:152-234), Tile 글자 아이콘·사용 불가 모양(cpp:36-112) |
| `studio/src/Theme.h` | 수정 | 탭 잠김 QSS 1줄(89-90 다음, 기존 `Faint`) |
| `studio/src/MainWindow.h` · `MainWindow.cpp` | 수정 | `pCut_`(h:112)·`cutAxisGroup_`·각도 칸·`ctxDupKeys_`·`onPage`, ② 페이지(152-265), 옛 단추 제거(165·173), ctx(284-290, 278), objectName(278-280·367), `refreshAll` 잠김·ctx 보임·상태줄(532-569, 543), `updateCutInfo`(571-577), `requirePose` 순서(640-645), `confirmPose`(657), `setHandTool`(673-674), 슬롯 메모(750-759), `onAction`(775·796-797·832-837 — X/Y 맞춤 대기 커서·제스처 취소), `Esc`(945) |
| `studio/src/SheetView.h` · `SheetView.cpp` | 수정 | `setCutPage`(h:51-71), 도판 머리(cpp:199-202·241-252), objectName |
| `studio/src/View3D.h` · `View3D.cpp` | 수정 | `setCutPage`·3D 툴팁(h:42-57), 왼쪽/오른쪽 끌기 구분 `cutDrag_`(h:37, cpp:180-205), 3D 머리(cpp:222·262-267), objectName |
| `studio/src/main.cpp` | 수정 | `--cut`(24-28 주석, 블록 추가) |
| `studio/tests/test_cutpage.cpp` | 신규 | 수용 기준 1-18 |
| `studio/CMakeLists.txt` | 수정 | `CutPageText.h`(55-61), `STUDIO_GUI_SRC`(72-79), 102 다음 `test_studio_cutpage`·`studio_cutpage`·`RUN_SERIAL` |
| `docs/ui/UI_RULES.md` | 수정 | R1 표 1행(20)·3행(22), R8 예시(95) — 정본 변경(사람 검토 1) |

`GigaMesh/**`, `studio/src/ArtifactDoc.*`·`CutGesture.h`·`Plate.*`·`MeshIO.*`·`MeshRaster.*`·`StartPage.*`·`ExportDialog.*`, 기존 `studio/tests/*.cpp`, `CLAUDE.md`, `docs/ui/mockup/**`, `.github/**`, `factory/**` 는 수정하지 않는다(기준 30 의 허용 목록 = 위 표 14개 파일).

## 7. 위험·가정

- **좌표계·계산**: mm·+Z 위·바닥 Z=0·확정 뒤 중심선 x=y=0, 도판 단면 = 정면각 θ 의 Y=0 면 x ≥ 0 반쪽(D5 외면 왼쪽·단면 오른쪽, R = 거울상), 정면각 + = 위에서 보아 반시계 — 모두 JOB-004·005 그대로. 이번에는 엔진을 건드리지 않으므로 LastTest 세 줄(기준 20)이 그대로여야 한다. Y 막 = 0°, X 막 = +90°(JOB-005 사람 검토 2 — 이번에 단추가 화면에 생겨 실제로 확인할 수 있게 된다).
- **원문과 다르게 정한 곳 13곳**(§3.1, 사람 검토 2 (가)-(카); (타) 는 원문이 아니라 지시문과 다른 곳): ctx 안내 · ctx 오른쪽 · 상태줄 꼬리 · 탭 잠김(원문 탭 줄엔 없음) · 타일 한 줄 · 축 색 없음 · 맞는 축 켜짐 모양 · Z 막 사용 불가와 그 모양 · 반전 단추 `(반전)` 꼬리 · `국내 보고서 표준` 알약 없음 · 각도 표기 −180~180·0.1° · `측정·탁본` 탭 유지 · 3D 끌기 단추 규칙. 대부분 지금 없는 UI(빨간 손잡이·「지금 각도 고정」·Z 막)를 가리키지 않거나 R2·R3 를 지키려는 것.
- **페이지 = 도구 규칙의 동작 변화**: 확정(Enter) 뒤 홈이 아니라 ② 로 간다, `S`·홈 「단면선」 은 ② 로 가는 단추가 된다, `Esc`·다른 탭은 ② 를 떠나며 도구를 내려놓는다, 파일을 새로 열면(→ ①) 들고 있던 도구를 내려놓는다. `--sample`·「중심축 맞춤」(자동 확정)은 지금처럼 홈 — 그래서 verify 의 `shot_main` 은 ② 가 아니다(②는 `--cut`).
- **② 에서 3D 보기**: ② 는 늘 도구를 든 상태라 JOB-005 규칙 그대로면 3D 를 끌어서 볼 수 없다(View3D.cpp:190-193). → 오른쪽(·가운데) 단추 끌기 = 보는 방향(§3.5 「3D 끌기」). 오른쪽 끌기를 모르는 초보자를 위해 ② 의 3D 그림 툴팁에 적는다 — 원문 규칙(빈 곳 끌기 = 보는 방향)은 손잡이를 그리는 작업 때(§10).
- **페이지 전환 비용**: `onPage` 의 `refreshAll(false)` 가 3D 를 한 번 다시 그린다(≈ 15 ms, JOB-005 §5-7). 확정 경로는 `refreshAll` 이 두 번(전환·확정) 돈다 — 원본 다시 계산은 한 번. `openSample`·`openPath` 도 페이지를 바꾸므로 같은 15 ms 가 한 번 더 든다.
- **X/Y 맞춤 멈춤**: 맞춤은 원본 전체 계산(예제 ≈ 1.4 s, 25만 면 ≈ 4.4 s 추정, UI 스레드) — 대기 커서를 띄운다. 이미 그 각이면 계산하지 않는다. 끄는·누르는 중이면 그 제스처를 **취소**하고 맞춘다(먼저 확정하면 원본 계산이 두 번 ≈ 2.8 s). 비동기는 범위 밖.
- **리본 높이**: 막 방향 타일을 원문처럼 두 줄로 하면 모든 탭의 리본이 약 7 px 커져 작업 공간·도판 배율(fitView)이 바뀐다 → 한 줄(§3.1), `rows` 줄 높이 23 px 고정(§3.3), 기준 9 가 `sizeHint` 로 확인.
- **잠긴 탭은 누를 수 있다**(눌러서 ① 안내를 받게). 사용 불가 `Z 막` 타일의 툴팁은 Qt 가 비활성 위젯에도 띄운다고 가정 — **미확인**, 손 확인(사람 검토 8).
- **리본 흔들림**: 끄는 동안 그룹 이름(`막 방향 …`)·각도 칸 글자 폭이 바뀌면 오른쪽 그룹이 밀린다 → §3.3 고정 폭, 기준 9 가 폭을·11·13 이 x 위치를 확인(오프스크린 글꼴 기준, `settle()` 뒤).
- **ctx 폭**: `[ ]` 묶음을 빼고 ② 에서는 리본과 겹치는 `← →`·`R` 두 묶음을 숨기고 안내 칸 최소 폭을 1 로 두지만, 오프스크린은 한글 글꼴이 없어 실제 폭을 잴 수 없다(기준 9 는 기록만) — 1366 px 실제 확인은 사람 검토 7. 추정(Malgun 13 px)으로는 1366 px 에서 안내 글이 여전히 일부 잘린다 — 그때는 툴팁으로 보고, ctx 오른쪽 문구 줄이기는 사람 검토 7 결과에 따라 후속.
- **시험의 사용자 설정**: 통합 시험은 MainWindow 를 실제로 띄워 QSettings(최근 목록·창 위치·감시 폴더)를 읽고 쓴다 → 조직·앱 이름을 `DongArch3DTest` 로 바꿔 사용자 값과 분리(§3.11). 판정은 시간과 무관(§4 머리)하고 기다림은 상한만, `RUN_SERIAL` 로 CI(`studio-ci.yml:65` `--parallel 4`)에서도 혼자 돈다. 예제 원본 계산 ≈ 11번 → 한 번 실행 ≈ 25 s 예상.
- **캡처 해시(기준 34)**: `shot_home`(시작 화면 — 최근 목록)·`shot_export` 는 사용자 QSettings(최근 목록·`window/geometry`, main.cpp:19-20)에 기대므로, JOB-005 검증(10-17 22:09) 뒤 사용자가 다른 파일을 열었거나 창 위치를 바꿨으면 코드와 무관하게 다를 수 있다. 예제(`--sample`)는 경로가 없어 최근 목록에 남지 않는다(MainWindow.cpp:482). 다르면 FAIL 로 두고 원인을 decision.md 에 적는다.
- **두 번 컴파일**: GUI 원본 6개를 앱과 시험이 각각 컴파일한다(빌드 시간 소폭 증가, 앱 링크 구조 무변경). 경고 0 은 둘 다에 적용(시험의 `QMouseEvent` 생성자도 비권장 아닌 것).
- **`--cut` 시점**: 200 ms 에 키를 보내고 300 ms 뒤 원본 확정(≈ 1.4 s, UI 스레드) → 1800 ms 캡처 타이머는 확정이 끝난 뒤 돈다. 확정 시작이 1800 ms 를 넘을 일은 없다.
- **정본 변경**: UI_RULES R1(탭 줄·잠김·ctx)·R8(예시) — CLAUDE.md §1 "바꾸려면 먼저 사용자 승인" 대상, 사용자 "승인개발진행" 을 근거로 진행하고 승인 때 재확인(사람 검토 1). `Theme.h` 는 QSS 1줄만(새 토큰 없음, R2 무변경).
- **라이선스**: 새 의존성 없음(Qt6 Widgets 는 이미 앱이 씀).

## 8. 사람 검토 항목

1. **[정본] UI_RULES R1·R8 변경**: 탭 줄에 `② 막으로 자르기`(① 과 `측정·탁본` 사이), 바닥 확정 전 잠김 표시(숨기지 않음·`Faint`·툴팁 「바닥 확정 후 사용」·누르면 ① 안내), ② 탭 = 「✂ 막으로 자르기」를 든 상태, R8 예시 `[ ]` → `← →`(`, . [ ]` 는 별칭·배지 없음). `③ 도면 편집` 탭과 `측정·탁본` 의 관계는 JOB-007 에서.
2. **[문구·모양] 원문과 다르게 정한 곳** (원문 값 → 이번 값):
   - (가) ctx 안내: `빨간 손잡이로 회전판을 돌리면 … · [R] 좌/우`(1664) → `3D 를 좌우로 끌어 회전판을 돌리면 …`(손잡이를 그리는 작업 때 원문으로).
   - (나) ctx 오른쪽: `막 12° · 끄는 중 3.5만 면 약 1 ms → 놓으면 25만 면 약 6 ms`(1667) → 실제 면 수 + 끄는 중 `음영·외곽·치수·두께는 놓은 뒤`.
   - (다) 상태줄 꼬리: 「지금 각도 고정」 → 「도면 편집으로」(1673) → `다음: 「도면」 내보내기 (Ctrl+P)`(JOB-007 에서 원문으로).
   - (라) 탭 잠김: 원문 탭 줄에는 잠김이 없고 바닥 미정에서 ② 를 누르면 바닥을 자동으로 놓고 ② 로 간다(1714) → 이번엔 가이드 §8 표(785-786)를 탭에 적용해 잠그고 ① 로 안내. 모양도 원문 잠김(투명도 42 %+자물쇠, 730) 대신 `Faint` 글자 + 🔒 툴팁.
   - (마) 막 방향 타일 글자: 원문 두 줄 `X 막<br>옆에서`(1463) → 한 줄 `X 막 옆에서`(리본 높이 유지 — §3.1).
   - (바) 각도 표기: 원문 정수 **0~359°**(`mod360`, 910·1726·1782) → 이번 **−180~180°·0.1°**(정수면 `12°`, 끌기 반 도 `37.5°`, 음수 `−20°`). 예: 0° 에서 [−] 한 번 = 원문 `359°`, 이번 `−1°`.
   - (사) 축 글자 아이콘 색: 원문 X 주황·Y 초록·Z 파랑(1463) → 먹색(R2 — 3 의 결정 대기).
   - (아) 맞는 축 켜짐: 원문 활성 = 어두운 `#2b2b2b` 바탕·흰 글자(121) → R3 켜짐 `Oat`(어두운 바탕은 R3 「손에 든 도구」 전용). Z 막: 원문은 높이 막으로 동작(1725) → 이번엔 사용 불가 `Faint` + 🔒 툴팁.
   - (자) `단면 위치`: 반전 글자 원문 `단면 왼쪽·외면 오른쪽`(1469) → JOB-004 의 `단면 왼쪽·외면 오른쪽 (반전)`(홈 단추와 공용) 유지, 원문 초록 알약 `국내 보고서 표준` → `Done` 글자.
   - (차) 탭 줄: 원문 `… ② 막으로 자르기 · ③ 도면 편집 · 내보내기`(1429) → `… ② 막으로 자르기 · 측정·탁본 · 내보내기`.
   - (카) 3D 끌기: 원문 손잡이 = 회전판, 빈 곳 = 보는 방향(1506·825) → 왼쪽 끌기 = 회전판, 오른쪽 끌기 = 보는 방향(손잡이를 그리는 작업 때 원문으로).
   - (타) 막 방향 순서는 지시문 [Y][X] 가 아니라 원문 X · Y · Z.
3. **[색 — R2 변경, 승인 필요]** 축 색 X 주황 `#c8641e` · Y 초록 `#3f8f4f` · Z 파랑 `#3f6fb5`, 짝 보라 `#6b4fc0` 을 Theme 토큰으로 넣을지. 이번에는 X·Y·Z 글자를 먹색으로 그렸다.
4. **[동작] 페이지 = 도구**: 확정하면 ② 로 바로 간다, `S`·홈 「단면선」 = ② 로 가기, `Esc`·다른 탭 = 내려놓기, ② 에서 `O` = 홈으로 가서 외곽선 도구, ② 에 들어가면 가로 슬롯을 보고 있었어도 A–A′ 종단면으로(각은 그대로). **② 에서 3D 왼쪽 끌기 = 회전판뿐, 보는 방향은 오른쪽 끌기 또는 [정면][위] 단추** — 오른쪽 끌기를 초보자가 찾을 수 있는지(툴팁만 있음).
5. **[동작] 홈의 「정면 회전 ±1°」·「절단 위치 이동 (1°)」 단추를 뺐다**: 둘 다 +1° 만 하던 단추다. `, . [ ]` 키는 그대로 동작하나 배지는 `← →` 만(② [−][+] 툴팁에 별칭을 적음). 슬롯 메모 `절단 각도 12.0°` → `막 12°`.
6. **[동작] X/Y 맞춤은 바로 원본 계산**: 절대각이라 델타 합치기를 거치지 않는다(이미 그 각이면 계산 안 함). 예제에서 누를 때 약 1.4 s 대기 커서. 끄는·누르는 중에 누르면 그 움직임은 **취소**되고 맞춘 각이 남는다(되돌리기 한 번 = 움직이기 전 각). X 막 = +90°(정면에서 보아 왼쪽 옆이 보는 사람 쪽 — JOB-005 사람 검토 2)를 이제 화면에서 확인해 주세요.
7. **[화면] 1366 px**: ctx 줄(배지·안내·단축키 4묶음·오른쪽 문구 — ② 에서는 `← →`·`R` 묶음을 숨김)과 상태줄이 1366 px 창에서 넘치지 않는지, 안내 글이 얼마나 잘리는지. 기준 9 의 오프스크린 폭은 참고값. 잘리면 ctx 오른쪽 문구 줄이기·말줄임을 후속으로(§10).
8. **손 확인(오프스크린 불가)**: ② 탭 잠김 툴팁·`Z 막` 툴팁이 뜨는지, 끄는 동안 리본·도판 머리 각도가 따라 바뀌고 리본이 옆으로 흔들리지 않는지, ← → 가 슬롯 목록 등에 먹히지 않는지, 한글 라벨이 잘리지 않는지(Malgun Gothic), `Shift`+→ 15°, `Ctrl+Z`, 오른쪽 끌기 감각, X/Y 맞춤 대기 커서.
9. 「국내 보고서 표준」 글은 원문대로 R 반전 상태에서도 그대로 보인다(반전 상태는 국내 표준이 아님 — 원문 유지가 맞는지).

## 9. 크기

| 묶음 | 파일 | 예상 줄(추가+수정) | 빈 줄·주석 제외 |
|---|---|---|---|
| 표기 함수 | CutPageText.h(신규) | ≈ 76 | ≈ 61 |
| 리본 | Ribbon.h/.cpp, Theme.h | ≈ 51 | ≈ 43 |
| 화면 연결 | MainWindow.h/.cpp | ≈ 134 | ≈ 109 |
| 도판·3D 머리·3D 끌기 | SheetView.h/.cpp, View3D.h/.cpp | ≈ 26 | ≈ 23 |
| 캡처 | main.cpp | ≈ 13 | ≈ 9 |
| **제품 합계** | | **≈ 300** | **≈ 245** |
| 테스트·빌드 | test_cutpage.cpp(신규), CMakeLists.txt | ≈ 290 | — |
| 정본 | UI_RULES.md | 3 | — |

빈 줄·주석 제외 300줄 지침 안이라 나누지 않는다. 넘으면 §0 의 006a/006b 로 나눈다. 테스트가 큰 것은 화면 통합(기준 8-17)을 실제 MainWindow 로 한 실행 안에서 확인하기 때문이며, 한글 라벨을 캡처로 판정할 수 없는(□) 환경에서 라벨·흐름을 참/거짓으로 가르는 유일한 방법이라 줄이지 않는다.

## 10. 후속 메모 (등록·명세는 코디네이터 몫)

- **JOB-007**: `고정 · 넘기기` 그룹(`지금 각도<br>고정` 타일 + `Enter`, `도면<br>편집으로` 빨강 주 단추 = ② 페이지의 `Action` 하나, `H` 1470·1635), 상태줄 꼬리를 원문 `「지금 각도 고정」 → 「도면 편집으로」`(`H` 1673)로 되돌림, 상태줄 오른쪽 `막 고정 전 / 12° 고정`, `③ 도면 편집` 탭(UI_RULES R1 탭 줄 다시 — `측정·탁본` 처리 결정 포함), 기준선 레이어. `CutPageText.h` 의 `kStatus` 한 줄만 바꾸면 된다.
- 회전판 빨간 손잡이를 3D 에 그리는 작업 때 ctx 안내를 원문 `빨간 손잡이로 회전판을 돌리면 …`(`H` 1664)으로, 3D 끌기를 원문 규칙(손잡이 = 회전판, 빈 곳 끌기 = 보는 방향만, `H` 1506)으로 — 오른쪽 끌기를 남길지 함께 정한다.
- Z 막 작업 때 `axZ` 잠김을 풀고 ctx·도판 머리 Z 문형(`C–C′ 실시간 횡단면 — 높이 …`, `파란 손잡이를 위아래로 끌면 …`, `H` 1631·1664)을 `CutPageText.h` 에 더한다.
- 사람 검토 3 승인 시 Theme 축 색 토큰 + UI_RULES R2 갱신 → `axX/axY/axZ` 글자 색.
- CI(`.github/workflows/studio-ci.yml:72-76`) 캡처 목록에 `--sample --cut --shot shot_cut.png` 추가(워크플로 변경 — 별도 지시가 있을 때).
- ctx 안내 칸 말줄임(…) 표시, ctx 오른쪽 문구 줄이기(원문 1667 처럼 한 줄 짧게), 1366 px 레이아웃 정리(사람 검토 7 결과에 따라).

## 11. 비평 반영 기록

| # | 지적(관점·등급) | 반영 | 이유·어디에 |
|---|---|---|---|
| 1 | 2줄 막 방향 타일 → 리본 +7 px, 기준 33 거짓 (design·must) | 반영 | 원문을 두 번 확인: 원문 리본은 높이 104 px 고정(`H` 114)이라 두 줄이 들어가지만 이 앱은 칸 높이가 모든 페이지 최대값이다(Ribbon.cpp:25-29·260 확인). 안 (2)「리본 높이 불변」을 택하되 줄 간격을 줄이는 대신 **한 줄 글자**로 — 기존 타일도 원문 두 줄을 한 줄로 썼다(MainWindow.cpp:219·230). §3.1·§3.3·기준 9(리본 높이 판정)·31·33·사람 검토 2 (마) |
| 2 | 미리보기 문구에 외곽 빠짐 (design·should) | 반영 | ArtifactDoc.h:99-100·MainWindow.cpp:711 확인. `음영·외곽·치수·두께는 놓은 뒤` — §3.1·§3.2·기준 4·11·23·사람 검토 2 (나) |
| 3 | 원문과 다른데 사람 검토에 없는 것 (a)-(e) (design·should) | 반영 | `H` 121·135·730·1469·1714·910·1726·1782 확인. (a)→(아) (b)→(자) (c)→(아)+Z 툴팁에 🔒 추가 (d)→(라)·§3.1 탭 잠김 행 (e)→(바) 원문 0~359 명시. 더 찾은 것: 알약 없음(자)·`측정·탁본` 유지(차)·3D 끌기(카). §7 의 "3곳" → 13곳 |
| 4 | ② 에서 3D 보는 방향을 끌어서 못 돌림 (design·should) | 반영(방법 일부 다름) | View3D.cpp:186-196 확인. 세로 끌기 → pitch 는 비스듬히 끌 때 회전판과 보기가 함께 움직여 택하지 않고 **오른쪽 단추 끌기 = 보는 방향**으로(§3.5 「3D 끌기」), 3D 툴팁 `k3dTip`, 기준 13·28, §7·사람 검토 4, 원문 규칙 복원은 §10 |
| 5 | 분할표 기준 배정이 구현 배정과 어긋남 (design·should) | 반영 | onPage(1-4·6, 5 의 도구 들기)·`confirmPose` → ② 를 006a 로, 「§3.5 의 4-6」을 절 이름(「onPage」「입력 규칙」「3D 끌기」)과 (006a)/(006b) 표시로 바꿈. `kbdLabel("[ ]")`·`--cut` 검사를 24·26 에서 28 로 옮김 — §0 |
| 6 | X/Y 맞춤 대기 커서 없음·이중 계산 (design·should) | 반영 | MainWindow.cpp:653·812·820 확인. 대기 커서 + 진행 중이면 `cancelCut()` 뒤 맞춤(되돌리기 칸은 `beginCutInput` 이 이미 쌓아 `pushUndo` 생략) — §3.7, 기준 10(취소 판정: Ctrl+Z → 0°)·26(WaitCursor ≥ 7), 사람 검토 6 |
| 7 | `[ ]` 배지가 사라져 R8 예시와 어긋남 (design·should) | 반영(두 안 모두) | UI_RULES.md:95 확인. R8 예시 `[ ]` → `← →` + 별칭 문장(§3.10, 기준 29), [−][+] 툴팁에 `, [`·`. ]` 병기(§3.4, 기준 23) |
| 8 | 2줄 타일 → 모든 페이지 리본 커짐, 기준 9 가 2줄을 요구 (code·must) | 반영 | 1 과 같은 지적. 안 (나) 쪽이나 둘째 줄을 툴팁으로 숨기지 않고 **한 줄에 두 낱말을 다 보임**(`X 막 옆에서`) — 초보자에게 "옆에서/정면에서"가 보이는 편이 낫다. 기준 9·23 기대 문자열 변경, 사람 검토 2 (마) |
| 9 | 기준 11 '끄는 중을 1번 이상 봄'이 300 ms 마감에 기댐 (code·should) | 반영(두 안 모두) | CutGesture.h:48-59(확정 먼저) 확인. 끄는 중 확인을 **왼쪽 단추 누른 채 끌기**(놓을 때까지 확정 없음)로 바꾸고, `RUN_SERIAL TRUE` 도 넣음 — 기준 11, §3.11, §4 머리·§7 문구 |
| 10 | 레이아웃 갱신 전 x 를 읽어 의미 없는 통과 (code·should) | 반영(두 안 모두) | `settle()`(조건 없는 100 ms 처리)을 위치·폭 읽기 전 의무로, 기준 9 에 그룹 이름 라벨·각도 칸 폭 ≥ 가장 긴 문구 폭 판정 추가 — §3.11, 기준 9·11·13 |
| 11 | 탭 잠김 색이 캡처로만 판정됨 (code·should) | 반영 | Theme.h:89-90 확인. 기준 8 에 `palette().color(QPalette::ButtonText) == theme::Faint`, 9 에 `Ink`(현재 탭), 15 에 `Ink2`. 잠김 QSS 는 89-90 다음(같은 특이도라 뒤가 이김)으로 명시 — §3.3 |
| 12 | shot_home·shot_export 는 해시 비교 가능 (code·should) | 반영 | JOB-005 artifacts 에 두 파일 있음 확인(13246·70269 바이트, JOB-004 와 같은 크기). 기준 34(해시) 신설, 33 은 shot_main 만. 사용자 QSettings 에 따른 위험은 §7 |
| 13 | 기준 30 이 사람 대조·점 셋 diff (code·should) | 반영 | 허용 목록 `grep -vxF` 명령 + `git status --porcelain` 빈 출력 + `804f60d HEAD`, C 묶음 전체를 "커밋 뒤에 잰다"로 — 기준 30·C 머리·§5-4 |
| 14 | 분할표 기준 배정 (code·should) | 반영(배정 일부 다름) | 5 와 같은 지적. 제안 배정과 달리 **11·12 는 006a** — [−][+]·왼쪽 끌기(JOB-005 동작)만 쓰므로 006b 코드 없이 참이다. 13 은 오른쪽 끌기 때문에 006b. 006a = 1-12·14·16·18-27·29-30, 006b = 13·15·17·28·31-34 — §0 |
| 15 | ctx 에서 초보자 안내가 먼저 잘림 (code·should) | 반영(첫째 안만) | MainWindow.cpp:284-290 확인. ② 에서 리본과 겹치는 `← →`·`R` 두 묶음을 숨김(§3.6-5, 기준 15), `[9]` 에 ctxHint 실제 폭/sizeHint 인쇄. 둘째 안(ctx 오른쪽 먼저 줄이기)은 반영 안 함 — 원문 ctx 오른쪽 문구(1667)를 유지하는 것이 이번 목표이고, 1366 px 실측(사람 검토 7) 전에 문구를 바꾸면 근거가 없다 → §10 후속 |
