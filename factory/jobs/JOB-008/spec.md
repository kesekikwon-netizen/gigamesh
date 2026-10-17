# JOB-008 명세 — 단면·외곽선을 기준선에 맞추기 (편차 제한 곡선 · 막과 수직인 실루엣 · 최대 편차 표시)

작성: spec-writer, 2026-10-18. **비평 반영판**(같은 날 — 반영 내용은 §11). 기준 커밋 `89846ba`(factory/JOB-008 = JOB-007 끝, worktree `D:/_dongarch_build/wt/.worktrees/JOB-008`, **JOB-006·JOB-007 승인 전제** — JOB-003·004·005 승인·병합 완료).
코드 줄 번호는 모두 이 worktree 에서 직접 읽고 Grep 으로 확인한 값이다. 디자인 줄 번호는 정본 `D:\_dongarch_build\design\dongarch3d-design-v2.html`(이하 `H`)에서 Grep 으로 확인했다.
**수치 출처**: "현재" 값 = 89846ba 그대로(JOB-007 산출 DXF `D:/_dongarch_build/verify_job007/studio/studio_reflines.dxf` 분석 + 모의 실험), "예상" 값 = §3 알고리즘을 그대로 돌린 모의 실험 값. 모의 실험은 세션 scratchpad 에서 89846ba 와 같은 소스로 빌드된 `D:\_dongarch_build\verify_job007\studio\studio_core.lib`·`karch.lib` 에 링크해 돌렸다(제품 코드·저장소 무변경, `/fp:fast /arch:AVX2` 같은 플래그). 비평 반영판의 예상 값은 모두 다시 돌린 값이다(scratchpad `exp8r/sim.cpp` — 반쪽 정사 실루엣·좁힌 곡선 조건·외곽선 허용치 간격/2 그대로).
약어: `W` = worktree, `B` = `D:\_dongarch_build\verify_job008\studio`(빌드 폴더), `L` = `D:\_dongarch_build\verify_job008\studio-logs`, `A7` = `D:/_dongarch_build/wt/factory/jobs/JOB-007/artifacts`, `A8` = `D:/_dongarch_build/wt/factory/jobs/JOB-008/artifacts`.
출발점: JOB-007 `review-correctness.md` §4-1(단면 넘침), `approval-report.md` §5-1, JOB-007 명세 §7(외곽선 틈)·§8-6·§10 첫 줄.

## 0. 분할 판단

제품 변경은 `ArtifactDoc.h/.cpp`(곡선 규칙·반쪽 정사 실루엣·최대 편차) + `MainWindow.cpp` 2줄 + 정본 1줄로 **≈ 146줄(빈 줄·주석 제외 ≈ 112줄)**, 시험·빌드 ≈ 420줄이다(§9). 제품 300줄 지침 안이라 **나누지 않는다.** 한 커밋으로 충분하다.
만약 실측이 300줄(제품, 빈 줄·주석 제외)을 넘으면 builder 는 멈추고 아래처럼 제안한다(넘을 가능성은 낮음):

| 조각 | 내용 | 파일 | 수용 기준 |
|---|---|---|---|
| **008a** 엔진 | §3.3 곡선 · §3.4 실루엣 · §3.5 `maxDeviationMm()` | ArtifactDoc.h·.cpp, tests/test_fit.cpp, CMakeLists.txt | 1-9, 11-16, 18-21, 23-24 |
| **008b** ③ 표시 | §3.5 상황 줄 글 · §3.6 정본 | MainWindow.cpp, tests/test_cutpage.cpp, UI_RULES.md | 10, 15·17 의 008b 부분, 22 |

## 1. 목표

사용자 요청(job.json `request`, 코디네이터 전달):
> 사용자 요구: 실측 도면(단면·외곽선)이 국내 보고서 표준대로 기준선(중심선·구연선·바닥선) 위에 정확히 놓여 Illustrator 로 1:1 축척 출력. JOB-007 검토·완전성 점검에서 발견: 기본 Catmull-Rom 곡선이 단면 끝을 부풀려 예제 단면이 구연선 위 +3.1 mm·바닥선 아래 −3.7 mm 로 넘치고, 외면 외곽선 끝이 구연선·바닥선에 약 3 mm 못 미침(K-Arch 실루엣 꼭짓점 z 2·98 백분위). 사용자 "전부 다되면 개발진행하라".

코디네이터 지시: "실측 도구이므로 실측값을 지어내면 안 된다", "실측 점에서 그려진 선이 떨어진 정도(최대 편차)를 수치로 판정하라". 프로필 규약: "추정값(내벽 추정 등)은 실측값과 구분 표기".

**재진술(한 문장)**: 그려지는 단면선·외곽선을 **원 선에서 「간격」의 절반 안**(단면·외곽선 같은 허용치)으로 묶어 — 곡선 매끄럽게는 그 안에 머물고 원 선의 높이 범위·축 쪽을 넘지 않는 구간에만 넣고 나머지(구연·바닥 모서리 등)는 실측 점을 잇는 간략 꺾은선으로 두며, 외곽선은 디자인 원문대로 **막과 수직인 실루엣의 외면 반쪽**(원본 메시에서 칸마다 x ≤ 0 교점의 최소 x, 평활·복사·보간 없음)으로 바꿔 — 단면이 구연선·바닥선·중심선을 넘지 않고, 외곽선이 중심선을 넘지 않으며 수평인 바닥·구연에서는 끝이 바닥선·구연선 높이(0.25 mm 안)까지 닿게 하고, 그 편차를 `최대 편차 0.23 mm` 처럼 ③ 상황 줄에 보이고 시험·독립 파이썬으로 판정한다 — 실측 점은 옮기거나 지어내지 않는다(바닥·구연이 기울어 생기는 끝 틈은 실측대로 남긴다).

## 2. 범위 밖

- **구연선 = 최고점**(백로그 23): 구연선 높이는 그대로 메시 최고점 H. 이번 변경과의 맞물림 — 단면 끝을 기준선에 끌어 붙이지 않으므로(§3.2) 손잡이·꼭지가 구연보다 높은 유물이면 단면 꼭대기와 구연선 사이 틈이 그대로 보인다(실측대로). 기준 1 의 「틈 ≤ 0.30 mm」 는 예제(완형, 꼭지 없음)의 성질로만 판정한다.
- **결실·기운 구연**: 막 각도에서 단면 꼭대기가 H 보다 낮으면 그 틈은 실측 그대로 남는다(끌어 붙이지 않음). 결실 쪽 구연선 파선은 기준선 2차.
- **기운 바닥·기운 구연의 외곽선 끝**: 바닥이나 구연이 축에 대해 기울어(손빚음·접지·정렬 잔차) 외면 쪽(x ≤ 0) 맨 아래·위가 바닥선·구연선에서 떨어져 있으면, 외곽선은 그 높이에서 끝나고(끝 틈) 그 사이 높이에서는 기운 바닥·구연 가장자리를 따라 거의 수평으로 그려진다 — 정면에서 본 외면 반쪽의 실제 윤곽이므로 **실측대로 남긴다**(끌어 붙이거나 메우지 않음). 모의 실험: 바닥 0.3° → 아래 끝 y 0.625(틈 0.625 mm), 0.5° → 0.875, 구연 0.5° → 위 틈 0.724, 1° → 1.326(§3.8 아래 표).
- **조각(편)**: 외곽선(반쪽 정사 실루엣)은 x ≤ 0 쪽 교점만 쓰므로, 한쪽만 남은 조각은 그쪽이 없는 높이에서 외곽선이 비거나 짧다 — MVP 는 완형.
- **DXF 층 이름**, **정면 방위각 문구**: 바꾸지 않는다.
- **출력 단면 채움·두 줄 윤곽**(백로그 3): 지금처럼 빨강 0.35 + 연한 채움. 맞물림 — 두 줄(외벽·내벽)로 나눌 때도 이번 `fitCurve` 를 **닫힌 고리에 먼저** 적용한 뒤 나눠야 구연·바닥 모서리의 간략 꺾은선이 유지된다(§10).
- **기준선 폭을 윤곽 끝점에 맞추기**(원문 `H` 1852-1857 은 바닥선·구연선을 외곽선 끝 x ~ 단면 끝 x 로 긋는다): 지금은 JOB-007 결정대로 구경·저경 폭. 이번 변경 뒤 외곽선 끝 x 와 기준선 끝의 차이를 시험이 인쇄만 한다(§8-5).
- **K-Arch 수정·치수용 실루엣 호출 제거**: K-Arch `Silhouette.cpp` 는 고치지 않고, 치수용 `ExtractSilhouette(fastMesh, MaxRadius, 0.25)`(ArtifactDoc.cpp:675 → `AnalyzeDimensions` 인자) 호출도 그대로 둔다 — 5대 치수 무변경. 그리는 외곽선만 Studio 가 원본 `mesh_` 에서 직접 계산한다(§3.4). 치수용 호출을 없애 확정·놓기 때 ≈ 1 s 줄이는 일은 후속(§10).
- **25만 면 초과 메시의 치수**: 치수는 지금처럼 `fastMesh`(25만 면 초과면 LOD)로 계산한다(프로필 위험 3). 이번 모의 실험의 34.6만 면 합성 예제(기울기 0)에서 LOD 치수가 저경 5.2 mm·평균 기벽 3.85 mm(실제 142·6.8)로 **틀리게** 나왔다 — 기준선(바닥선 폭 = 저경)에도 영향. 이번 범위 밖, 후속(§10)으로 따로 보고.
- **② 끄는 동안 외곽선 갱신**: 끄는 동안은 지금처럼 외곽선이 확정 각의 것으로 멈춰 있다(test_cut 이 "frozen" 으로 확인). 정사 실루엣은 각에 따라 달라지므로 비대칭 유물은 놓은 뒤 외곽선이 바뀐다(§7). (반쪽 정사 실루엣 자체는 원본에서도 ≈ 7 ms 라 끄는 동안 갱신할 길이 생겼지만 이번엔 하지 않는다 — §10.)
- **도판 마우스 판독(`SheetView.cpp`)**: 판독이 쓰는 `displayOutline()` 의 뜻이 바뀌지만(§7) 코드는 고치지 않는다.
- **리본 「선 다듬기」 의 `1,420 → 186점`·`편차 0.21 mm [겹쳐 보기]`·간격 「원본」 칩**(`H` 1478): 이번엔 ③ 상황 줄 오른쪽(`H` 1665)에만 최대 편차를 보인다.
- 계산 엔진 밖 파일: `studio/src/Plate.*`(그리는 쪽은 `display*()` 를 그대로 쓰므로 무변경), `SheetView.*`, `CutGesture.h`·`MeshIO.*`·`MeshRaster.*`·`Theme.h`, `GigaMesh/**`(K-Arch 포함), 기존 시험 `test_pipeline/test_pose/test_side/test_cut/test_reflines.cpp`, `factory/**`, `.github/**`, `CLAUDE.md`.

## 3. 설계 결정 (builder 가 그대로 구현할 기본값)

### 3.1 원인 (확인한 것)

**(가) 단면 넘침 = 닫힌 Catmull–Rom 의 모서리 넘침.** `sectionNatural`(ArtifactDoc.cpp:733-755)은 실측 고리(Y=0 절단, `sliceSectionY0` 850-1033)를 `simplifyDP(poly, 간격×0.5, closed)`(175-205)로 줄인 뒤 `smoothCatmull(p, true, 3)`(207-233, 균일 Catmull–Rom, 접선 = (다음 − 이전)/2)를 씌운다. 예제 0° 는 DP 점이 20개뿐이고 구연 윗면은 바깥 끝 c19 (71.009, 184.632) → 안 끝 c00 (64.158, 184.685) 한 변인데, 양옆 이웃이 29 mm 아래(c18 (76.658, 155.457), c01 (69.830, 155.428))라 c19 의 접선이 (c00 − c18)/2 = (−6.25, **+14.61**) 로 위를 향해 곡선이 187.917 까지 솟는다. 바닥 변 c08 (64.188, 0.127) → c09 (71.026, 0.237) 도 같은 이유로 −3.651 까지 내려간다. 실측 고리 y 범위는 [0.127, 184.685], 기준선은 0 / H = 184.821.

| 예제·각 | 현재 단면 y 범위 | 실측 고리 y 범위 | 현재 최대 편차(그린 꼭짓점 → 실측 선) |
|---|---|---|---|
| 예제 0° | [−3.651, 187.917] | [0.127, 184.685] | 3.798 mm |
| 예제 12° | [−3.692, 187.408] | [0.114, 184.690] | 3.875 |
| 예제 −20° | [−2.726, 187.334] | [0.151, 184.711] | 2.888 |
| 예제 105° | [−3.184, 187.366] | [0.129, 184.641] | 3.339 |
| 미스캔 0° (실선 `solids`) | [−0.176, 192.988] | [0.190, 190.148] | 2.864 |

같은 넘침이 미스캔 예제의 열린 선(`scannedSolidNatural` 757-767, `estimatedInnerNatural` 769-780 — 둘 다 `smoothCatmull(p, false, 3)`)에도 있다.

**(나) 외곽선 틈·저부 오류 = K-Arch MaxRadius 실루엣.** `recompute`(674-675)가 `ExtractSilhouette(fastMesh, MaxRadius, 0.25)` 를 쓴다. MaxRadius 는 ① 꼭짓점 z 의 2·98 백분위 사이만 칸으로 나누고(Silhouette.cpp:118-119), ② 꼭짓점 8개 미만 칸을 버리고(149), ③ 높이 10 mm 안에 3 mm 넘게 바깥인 칸이 있으면 그 칸을 버린 뒤(152-166), ④ `fillGaps` 가 앞·뒤 빈 칸을 첫·마지막 값으로 **복사해 채운다**(37-44). 그래서
- 예제: 외곽선 끝이 y 3.362 / 181.612 — 바닥선·구연선에서 3.362 / 3.208 mm 모자람(JOB-007 `[J7-1]`).
- **미스캔 예제(새로 찾음)**: 저부가 급하게 벌어져(0.36 mm/mm) ③ 이 아래 칸들을 버리고 ④ 가 위쪽 값 −71.546 을 아래로 복사해, 외곽선이 y 3.4–25 mm 에서 **수직으로 −71.5 mm** 에 그려진다. 실제 외면(정사 실루엣, 평활 없음)은 y 3 mm 에서 −63.0, 6 mm 에서 −64.1, 15 mm 에서 −67.3 — **최대 7.8 mm 바깥에 지어낸 선**이다(그린 외곽선 ↔ 실제 실루엣 양방향 최대 거리 9.80 mm).

**(다) 치수는 실루엣을 쓰지 않는다.** `AnalyzeDimensions`(DimensionAnalyzer.cpp:129-229)는 구경·저경·기고·최대경을 정점에서 계산하고 실루엣은 `maxBodyZ` 대체값(208-209, `zCount == 0` 일 때만 — 최대 반경 정점이 늘 있으므로 안 쓰임)에만 쓴다. 이번엔 치수용 `sil_`(MaxRadius) 계산을 그대로 두고 그리는 외곽선만 따로 계산하므로 5대 치수·LastTest 의 H/rim/belly/base/wall 은 그대로다.

**(라) K-Arch `TrueOrtho` 도 쓸 수 없다(비평으로 확인).** TrueOrtho(Silhouette.cpp:191-224)는 칸마다 **메시 전체**(x > 0 쪽 포함)의 최소 x 를 쓰고(204-221) `finishProfile`(76-106: 앞뒤 빈 칸 복사 37-44, SG7, 61칸 이동평균 80-90 — 창이 양 끝 칸을 포함)을 거친다. 바닥·구연이 축에 대해 조금만 기울어도 맨 아래·위 칸에는 단면 쪽(x > 0) 면만 걸려 최소 x 가 양수가 된다 — 외곽선이 중심선을 건너고, 그 튄 칸이 이동평균으로 안쪽 7.5–15 mm 에 번진다. 모의 실험(예제 0° 메시의 아래 30 mm·위 30 mm 만 기울여 `loadMesh` + `confirmPose`): 바닥 0.3°(단면 쪽 낮음) → TrueOrtho 최대 x **+51.407**, 0.5° → +61.174, 구연 0.5°(단면 쪽 높음) → +43.433, 1° → +63.696. 같은 메시에서 아래 §3.4 의 반쪽 정사 실루엣은 최대 x −49.203 / −28.593 / −12.018 / −6.047(중심선을 넘지 않음). 25만 면 초과 메시는 실루엣을 LOD 에서 잡는데(ArtifactDoc.cpp:674), 34.6만 면 합성 예제(기울기 0)의 LOD TrueOrtho 는 끝이 (24.256, 184.732) — 역시 중심선을 넘었다(LOD 는 정점 군집 평균, MeshIO.cpp:979-1003).

### 3.2 고른 방법과 버린 방법

| 방법 | 판정 | 이유 |
|---|---|---|
| **편차 제한 곡선 `fitCurve`(고름)** — DP 구간마다 Catmull–Rom 점을 넣되, 그 점들과 그 구간의 실측 점이 서로 허용치(DP 와 같은 간격/2) 안이고, 곡선 점이 원 선의 **높이 범위** 안·**축 쪽**(원 선이 모두 x ≥ 0 이면 x ≥ 0, 모두 x ≤ 0 이면 x ≤ 0)일 때만. 아니면 그 구간은 DP 직선(= 실측 점 두 개를 잇는 간략 꺾은선) | ✓ | 새 매개변수 없이 기존 「간격」 칩이 곧 허용치. 구연·바닥만이 아니라 어깨 능선·굽 등 모든 모서리에 같은 규칙. 넘침이 수치로 막힌다(예제 0°: 단면 20구간 중 6구간 직선, 편차 0.231 ≤ 0.25). 높이·축 조건으로 단면은 실측 y 범위(⊂ [0, H]) 밖·중심선 너머로, 외곽선은 실루엣 y 범위 밖·중심선 너머로 못 나간다 |
| 곡선 조건을 원 선의 **바운딩 박스 전체**로(원 명세) | ✗(비평) | 균일 Catmull–Rom 은 x 극값(최대경·목) 꼭짓점 옆에서 원 선의 x 범위를 허용치 안으로 조금 넘는데 이것까지 막혀, 매끈한 배 부분이 곧아진다 — 예제 0° 단면 14번 구간(82.649, 87.724)–(82.517, 103.132)(최대경)이 직선이 됐다. 목적(기준선·중심선)에는 높이 범위와 축 쪽만 필요 |
| y 를 [0, H] 로 자르기(클리핑) | ✗ | 넘침을 감출 뿐 옆으로 부푼 3 mm 는 남고, 잘린 윗면은 H 높이의 지어낸 수평선이 된다 |
| 끝을 기준선에 끌어 붙이기(snap) | ✗ | 실측 점(구연 184.632–184.685)을 H(184.821)로 옮기는 것 = 실측값 변경 |
| 각도 문턱으로 모서리 나누기 / 구심(centripetal) Catmull | ✗ | 임의 문턱이 하나 더 생기고 편차 상한이 없다 |
| **외곽선 = 반쪽 정사 실루엣(고름)** — Studio 가 원본 `mesh_` 에서 직접: 0.25 mm 칸마다 그 칸에 걸치는 삼각형만 돌아 수평면 교점 중 **x ≤ 0 인 것의 최소 x**, 맞는 점이 없는 칸은 비움(복사·보간 없음), **평활 없음** | ✓ | 디자인 원문 「외곽선 = 외면 반쪽의 윤곽 · 3D에서 온 곳: 막과 수직인 실루엣」(`H` 755)과 같은 정의(정면각에서 본 외면 반쪽 윤곽 = 도판 음영 영상 왼쪽 절반의 가장자리). 수평 예제는 칸이 바닥~최고점 전체라 끝이 기준선 0.25 mm 안. 칸 버림·채움·평활이 없어 미스캔 저부 7.8 mm 오류·엔진 평활 차이가 함께 사라지고, 기운 바닥·구연에서도 중심선을 넘지 않는다(§3.1 (라)). 늘 원본에서 계산해 25만 면 초과 LOD 오차도 없다. 빠르다(예제 7.7만 면 ≈ 7 ms, 34.6만 면 ≈ 24 ms — 면마다 걸치는 칸만). K-Arch 무변경 |
| 외곽선 = K-Arch `SilhouetteMode::TrueOrtho` + 메시 z 범위 밖 채움 칸 제거(원 명세) | ✗(비평) | 칸마다 메시 전체 최소 x + `finishProfile` 평활 — 바닥·구연이 0.3–1° 만 기울어도 외곽선이 중심선을 건너(+43 … +64 mm) 평활로 번진다(§3.1 (라)). 25만 면 초과면 LOD 로 잡혀 역시 넘는다. 수평 합성 예제만으로는 드러나지 않음 |
| 외곽선 끝에 구경/2·저경/2 닻을 이어 붙이기 | ✗ | 3 mm 직선 보간(지어낸 선). 미스캔 예제는 −62 → −71.5 로 9.5 mm 꺾인 선이 되고 저부 7.8 mm 오류는 그대로 |
| 틈 띠만 정점 반경 98 백분위로 메우기 | ✗ | 미스캔 예제는 K-Arch 끝값 자체가 틀려(위 (나)) 메운 점과 7 mm 어긋난다 |
| K-Arch `Silhouette.cpp`(백분위·칸 버림·평활) 고치기 | ✗ | K-Arch 시험(`karch_test_silhouette` 등)·CLI 동작이 바뀐다. 반쪽 정사 실루엣(≈ 30줄)을 Studio 에 두면 엔진 무변경으로 같은 목적을 이룬다 |
| 외곽선 허용치 = 간격(원 명세) | ✗(비평) | 기본 간격 0.5 에서 외곽선이 원 선에서 0.43 mm 까지 떨어져(예제 0°, 반쪽 정사 실루엣 기준 모의) 1:1 출력 외곽선 굵기 0.30 mm 보다 크다. 간격/2 로 해도 점 수는 거의 같다(예제 0° 81 → 82점). 단면과 같은 허용치 하나라 문구도 단순 |

### 3.3 곡선 규칙 `fitCurve` (`ArtifactDoc.h` 252-256 · `ArtifactDoc.cpp` 175-233)

1. **`simplifyDP` 에 남긴 점의 인덱스 출력 추가**(선언 253-254, 정의 175-205): 마지막 매개변수 `std::vector<size_t> *kept = nullptr`(정의에는 기본값 없이). `kept` 가 있으면 반환하는 점들의 `pts` 인덱스를 차례로 넣는다 — 이른 반환(`pts.size() < 3 || tol <= 0`, 177-178)이면 `0 … n−1`, 닫힘이면 맨 끝 되감기 점(`p.push_back(pts.front())` 의 인덱스 n)은 `out.pop_back()`(202-203)과 함께 뺀다. **고르는 점은 지금과 똑같다**(perpDist·dpRec·닫힘 먼 점 분할 123-146·184-197 그대로).
2. **`smoothCatmull`(선언 255-256, 정의 207-233)을 지우고 `fitCurve` 로 바꾼다**(다른 사용처 없음 — `grep -rn smoothCatmull studio GigaMesh/gui/src/dongarch` 결과 ArtifactDoc 뿐 확인). 선언 위 주석: "원 선 raw → 그릴 선: DP(tol) 뒤, smooth 면 DP 구간마다 Catmull–Rom 점 sub−1 개를 넣되 (가) 모두 raw 의 높이(y) 범위 안이고 raw 가 축 한쪽(모두 x ≥ 0 또는 모두 x ≤ 0)이면 그쪽에 있고 (나) 그 점들과 이 구간의 원 점이 서로 tol 안일 때만 — 아니면 그 구간은 DP 직선(실측 점을 잇는 간략 꺾은선). 곡선 점은 기준선(높이 범위)·중심선 너머로 나가지 않는다."
   ```cpp
   // 익명 이름공간(ArtifactDoc.cpp 17-173 안): 점-선분 거리, 점-꺾은선 최단 거리(닫힘이면 끝→처음 변 포함)
   double segDist(const KA::Point2 &p, const KA::Point2 &a, const KA::Point2 &b) {
     const double dx = b.x - a.x, dy = b.y - a.y, L2 = dx * dx + dy * dy;
     const double t = L2 > 0 ? std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / L2, 0.0, 1.0) : 0.0;
     return std::hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy));
   }
   double lineDist(const KA::Point2 &p, const std::vector<KA::Point2> &l, bool closed);  // l 비면 0, 점 1개면 그 점까지

   std::vector<KA::Point2> fitCurve(const std::vector<KA::Point2> &raw, double tol, bool closed, bool smooth, int sub) {
     std::vector<size_t> kept;
     const auto c = simplifyDP(raw, tol, closed, &kept);
     const long m = long(c.size());
     if (!smooth || m < 3 || sub < 2) return c;               // 곡선 끔 = 지금과 같은 DP 결과
     // raw 의 x0..x1 · y0..y1 (최솟값·최댓값). 허용 x 범위: xLo = x0 >= 0 ? 0 : −∞, xHi = x1 <= 0 ? 0 : +∞
     //   (단면·미스캔 실선·추정 내벽은 x ≥ 0 → 곡선 점 x ≥ 0, 외곽선은 x ≤ 0 → 곡선 점 x ≤ 0. 옆 방향 극값(최대경·목)은 막지 않는다)
     // at(i): 닫힘이면 c[(i mod m)], 열림이면 c[clamp(i, 0, m−1)]  (smoothCatmull 212-216 과 같음)
     // 구간 i (닫힘 m 개, 열림 m−1 개): p0..p3 = at(i−1..i+2)
     //   chain = raw[kept[i] .. e] (e = i+1 < m ? kept[i+1] : raw.size() — 닫힘 마지막 구간은 처음으로 되감기, raw[k % n])
     //   q_k (k = 1..sub−1, t = k/sub) = 0.5·(2b + (−a+c)t + (2a−5b+4c−d)t² + (−a+3b−3c+d)t³)  (smoothCatmull 224-225 식 그대로)
     //   ok = 모든 q_k 가 y0 <= q.y <= y1 ∧ xLo <= q.x <= xHi ∧ lineDist(q_k, chain, 열림) ≤ tol
     //        ∧ 모든 chain 점이 lineDist(·, [p1, q_1 … q_{sub−1}, p2], 열림) ≤ tol
     //   out ← p1, ok 면 q_1 … q_{sub−1} 도
     // 열림이면 끝에 c.back()
   }
   ```
   곡선이 모두 통과하면 결과는 지금의 `smoothCatmull(simplifyDP(...))` 와 꼭짓점까지 같다(t = 0 점 = p1). 비교는 `<= tol`, 높이·축 범위는 `>=`·`<=`(경계 포함).
3. **쓰는 곳 4곳**(허용치는 모두 간격/2, `sub` 는 지금 값 그대로): `sectionNatural` 748-750 → `auto p = fitCurve(poly, simplifyTol_ * 0.5, true, smooth_, 3);`, `scannedSolidNatural` 760-762 → `fitCurve(line, simplifyTol_ * 0.5, false, smooth_, 3)`, `estimatedInnerNatural` 773-775 → 같은 모양, `displayOutline` 797-799 → `fitCurve(ortho_, simplifyTol_ * 0.5, false, smooth_, 4)`(**외곽선 허용치를 간격 → 간격/2 로**, 원 선은 §3.4 의 `ortho_`). 걸러내기(`p.size() >= 3`/`>= 2`)와 좌/우 부호(`signedX`, D5)는 그대로.
4. `ArtifactDoc.h` 150-152 주석에 한 줄: "곡선은 원 선에서 간격/2 안이고 원 선의 높이 범위·축 쪽을 넘지 않는 구간에만 — `fitCurve`".
5. **보장의 범위**: 곡선을 넣은 구간은 양방향 ≤ tol 을 확인한 것이다. 직선 구간은 DP 성질(실측 점 → 현을 품은 **직선** 거리 ≤ tol, `perpDist` 123-128)이라 실측 사슬이 현 끝을 넘어 되돌아가는(접히는) 선에서는 선분 거리가 tol 을 넘을 수 있다 — 미스캔 예제의 추정 내벽(외벽을 안쪽으로 민 추정선)에 그런 곳이 있으나 전체 선 기준 거리(§3.5 정의)는 0.202 ≤ 0.25 다. 모의 실험 25경우(§3.8 두 표) 모두 전체 선 기준 ≤ 간격/2. 예제 판정은 기준 2·4·6·23·24.

### 3.4 외곽선 = 막과 수직인 실루엣의 외면 반쪽 (`ArtifactDoc` — Studio 가 원본 `mesh_` 에서 직접)

1. **익명 이름공간 함수**(ArtifactDoc.cpp 17-173 안, `segDist`·`lineDist` 옆):
```cpp
// 막과 수직인 실루엣의 외면 반쪽(디자인 v2 「도면 선 규약」 외곽선 = 외면 반쪽의 윤곽): 높이 칸
// zk = z0 + (k + 0.5)·bin (k = 0, 1, …, zk <= z1 인 것 모두, z0·z1 = m.bboxMin.z·bboxMax.z) 마다 삼각형 ∩ 수평면
// 교점 중 x <= 0 인 것의 최소 x. 맞는 점이 없는 칸은 비운다 — 복사·보간·평활 없음(실측 그대로). 면마다 그 면의
// z 범위에 걸치는 칸만 돈다. 교점 규칙은 K-Arch addTriZHits(Silhouette.cpp:171-189)와 같다.
std::vector<KA::Point2> halfOrthoSilhouette(const KA::MeshData &m, double bin);
```
   - 칸 수 n: `zk <= z1` 인 마지막 k 까지(같은 식 `z0 + (double(k) + 0.5) * bin` 으로 세고 비교 — 바닥 0·H 184.821 이면 0.125 … 184.625, 739칸).
   - 면 f(세 꼭짓점 중 하나라도 음수 인덱스면 건너뜀)마다 lo/hi = 세 꼭짓점 z 의 최소·최대, `k0 = max(0, floor((lo − z0)/bin − 0.5) − 1)`, `k1 = min(n − 1, ceil((hi − z0)/bin − 0.5) + 1)`(±1 칸 여유 — 판정은 아래 교점 규칙이 한다), k0…k1 의 각 zk 에서 세 변 u→v: `|u.z − zk| <= 1e-12` 이고 `u.x <= 0` 이면 u.x 후보, `dz = v.z − u.z` 가 `|dz| >= 1e-15` 이고 `t = (zk − u.z)/dz` 가 `t > 0 && t <= 1` 이며 `x = u.x + t·(v.x − u.x) <= 0` 이면 x 후보. 칸별 최솟값(초기 +∞).
   - 출력: 최솟값이 있는 칸만 `{x, zk}` 를 k 순서(높이 오름차순)로.
2. **멤버·접근자**(ArtifactDoc.h): `std::vector<KA::Point2> ortho_;  // 그리는 외곽선의 원 선 = 막과 수직인 실루엣의 외면 반쪽(x ≤ 0, 원본 mesh_, 평활 없음)` 와 `const std::vector<KA::Point2> &orthoOutline() const { return ortho_; }`. `silhouette()`(h:145) 주석은 "치수 계산용 K-Arch 실루엣(MaxRadius) — 그리지 않는다(그리는 외곽선은 orthoOutline)" 로 바꾼다.
3. **recompute**(674-676) — 치수용 두 줄은 그대로 두고 한 줄 더:
```cpp
  const KA::MeshData &fastMesh = mesh_.faces.size() > 250000 ? lodMesh_ : mesh_;
  sil_ = KA::ExtractSilhouette(fastMesh, KA::SilhouetteMode::MaxRadius, 0.25);  // 치수용(그리지 않음)
  dims_ = KA::AnalyzeDimensions(fastMesh, sil_);
  // 그리는 외곽선 = 막과 수직인 실루엣의 외면 반쪽, 늘 원본에서(LOD 아님). MaxRadius(2·98 백분위 · 칸 버림 · 빈 칸
  // 복사 · 평활)는 끝이 기준선에 못 미치고 급한 저부에서 선을 지어냈고, K-Arch TrueOrtho 는 x > 0 쪽까지 보고 평활해
  // 기운 바닥·구연에서 중심선을 넘었다(JOB-008).
  ortho_ = halfOrthoSilhouette(mesh_, 0.25);
```
   확정 전 분기(654 `sil_ = {};` 옆)에 `ortho_.clear();`.
4. `rawPointCount`(1090-1095)의 `sil_.leftOutline.size()` → `ortho_.size()`. `displayOutline` 은 §3.3-3.
5. 예상: 예제 0° 739점(첫 칸 0.125, 마지막 184.625), 미스캔 761점, 칸별 값은 시험 쪽 전수 계산과 같다(모의 실험 `maxdiff=0`). 시간: 예제(7.7만 면) ≈ 7 ms, 34.6만 면 ≈ 24 ms(`ExtractSilhouette` 한 번 ≈ 1.08 s 에 비해 무시할 만함).
6. 좌/우: Right 는 x ≤ 0 그대로, Left(R) 는 `signedX` 로 거울상 = 뒤에서 본 모양(정사 실루엣을 뒤에서 보면 화면 오른쪽이 월드 −x 쪽 — D5·`renderShade` 의 반쪽 클립(Plate.cpp:324-336)과 맞음).

### 3.5 최대 편차 (`ArtifactDoc` · ③ 상황 줄)

- **정의**(h, `pointCount` 174 옆): `// 그린 선(간략화·곡선)이 원 선에서 떨어진 가장 큰 거리 (mm). 원 선 = 단면은 막이 자른 선(미스캔 보정이면 실측 실선과 추정 내벽선 — 추정선도 포함), 외곽선은 막과 수직인 실루엣의 외면 반쪽(orthoOutline). 꼭짓점 기준 양방향: 그린 꼭짓점 → 원 꺾은선, 원 꼭짓점 → 그린 꺾은선. 미로드·확정 전 0.` + `double maxDeviationMm() const;`
- **계산**(cpp, `pointCount` 1083 앞): `shown()`(미리보기면 미리보기 선) 기준으로 짝마다 `lineDist` 전체 선 최단 거리의 최댓값.
  - 미스캔 보정 경로(`!s.solids.empty() && !s.estInner.empty()`, `sectionNatural` 735 와 같은 조건): `s.solids[i]` ↔ `fitCurve(s.solids[i], 간격×0.5, false, smooth_, 3)`, `s.estInner[i]` ↔ 같은 식(열림). 아니면 `s.loops[i]` ↔ `fitCurve(…, 간격×0.5, true, smooth_, 3)`(닫힘) — 그린 결과가 3점 미만인 고리는 그리지 않으므로 뺀다.
  - 외곽선: `ortho_` ↔ `fitCurve(ortho_, 간격×0.5, false, smooth_, 4)`(열림). 미리보기 중에도 외곽선은 확정 각의 것(지금과 같음).
  - 허용치·`sub` 를 표시 함수와 한 곳에서 쓰도록 builder 가 작은 도움 함수로 묶어도 된다(기준 6 이 표시 결과와 같은지 본다 — 기준 15 의 개수 판정은 이를 막지 않는다).
- **읽기 접근자**(h, `orthoOutline()` 옆): `const SectionLines &sectionLines() const { return shown(); }  // 지금 보이는 단면의 원 선(시험·판독용)` — 시험이 원 선을 다시 자르지 않고 문서가 쓴 그 값을 쓴다(§3.7).
- **③ 상황 줄**(`MainWindow::updateCutInfo` 664-667, ② 가 아닌 페이지): `QStringLiteral("점 %1 → <b>%2개</b> (%3 mm 간략화) · 최대 편차 %4 mm")` … `.arg(doc_.maxDeviationMm(), 0, 'f', 2)` — 원문 `H` 1665 `점 1,420 → 186 (0.5 mm 간략화) · 최대 편차 0.21 mm` 의 꼬리. ② 는 지금처럼 `cutpage::ctxRight`(662). JOB-007 명세 §8-5 (다) 「오른쪽 최대 편차 없음」 을 이번에 채운다. 예상: 예제 0°(홈·`--sample`) `점 1,343 → <b>82개</b> (0.5 mm 간략화) · 최대 편차 0.23 mm`, 12°(③ 캡처) `점 1,222 → <b>82개</b> (0.5 mm 간략화) · 최대 편차 0.20 mm`.
- 비용: 한 번 ≈ 2 ms(모의 실험) — `updateCutInfo` 는 놓은 뒤·페이지 바꿈마다 한 번(② 끄는 중에는 ② 분기라 계산 안 함). 홈에서 ← → 로 막을 돌리는 경우(백로그 21)에는 미리보기마다 한 번 — 16 ms 프레임 안.

### 3.6 정본 `docs/ui/UI_RULES.md` (사람 검토 1)

R5 의 기준선 줄(74) 다음에 새 줄:
`- **선 다듬기**: 단면선은 막이 자른 그대로, 외곽선은 막과 수직인 실루엣의 외면 반쪽(지금 정면각에서 본 외면 윤곽 — 원본 메시에서 0.25 mm 높이마다, 평활 없음). 그린 선은 원 선에서 「간격」의 절반 안 — 「매끄럽게」 곡선도 그 안에서만 넣고, 넘는 구간(구연·바닥 모서리 등)은 실측 점을 잇는 간략 꺾은선으로 둔다. 기준선·중심선 밖으로 나가지 않으며 끌어 붙이지도 않는다(기운 바닥·구연의 끝 틈은 실측대로). 상황 줄 오른쪽에 `점 N → M개 (간격 mm 간략화) · 최대 편차 d mm`(추정 내벽선도 포함).`

### 3.7 시험·빌드

- `studio/CMakeLists.txt` 103 다음: `add_executable(test_studio_fit tests/test_fit.cpp)`, `target_link_libraries(test_studio_fit PRIVATE studio_core)`, `add_test(NAME studio_fit COMMAND test_studio_fit ${CMAKE_CURRENT_BINARY_DIR})` → CTest 17 → **18**(시간 판정 없음, RUN_SERIAL 불필요).
- **`studio/tests/test_fit.cpp`**(신규, test_reflines.cpp 처럼 `QGuiApplication` 오프스크린, 출력 폴더 = `argv[1]`, 마지막 줄 `PASS`/`FAILED`). 판정·인쇄 꼬리표 **`[J8-n]`**(n = 이 명세의 수용 기준 번호). 시험 쪽 도움 함수는 제품 코드를 쓰지 않고 시험 안에 둔다:
  - `segDist`·`lineDist`(§3.3 과 같은 정의), `dev2(raw, drawn, closed)` = max(그린 꼭짓점 → raw 꺾은선, raw 꼭짓점 → 그린 꺾은선).
  - `rawHalfOrtho(m)` = **시험 쪽 반쪽 정사 실루엣**(제품 함수를 부르지 않고 시험 안에 따로 쓴다): k = 0, 1, … 에 대해 `zk = m.bboxMin.z + (double(k) + 0.5)·0.25 ≤ m.bboxMax.z` 인 동안, 면의 세 변 u→v 에서 `|u.z − zk| ≤ 1e-12` 이고 `u.x ≤ 0` 이면 u.x, `dz = v.z − u.z` 가 `|dz| ≥ 1e-15` 이고 `t = (zk − u.z)/dz ∈ (0, 1]` 이며 `x = u.x + t·(v.x − u.x) ≤ 0` 이면 x 의 최솟값(K-Arch `addTriZHits` 171-189 와 같은 규칙 + x ≤ 0), 맞는 점이 없는 칸은 건너뜀. **면마다 걸치는 칸만** 도는 구현(예제 ≈ 7 ms, 34.6만 면 ≈ 25 ms). 이 도움 함수 자체는 예제 0° 에서 한 번 **전 칸 × 전 면** 전수 구현(≈ 0.8 s)과 같은지(점 수·좌표 `==`) 확인한다(기준 5).
  - 예제 목록: `loadSample()` 0°·12°·−20°·105°(`setFrontAngle`), `loadArtecBlindSample()` 0°·12°. 모두 Right·간격 0.5·곡선 켬(기본).
  - **기운 예제**(기준 23): `loadSample()`(0°) 의 `mesh()` 를 복사해 정점마다 `s = x`(앞쪽 높음만 `s = −y`), `z += tan(b°)·s·max(0, 1 − z/30)`(바닥 경우) 또는 `z += tan(r°)·s·max(0, (z − (H − 30))/30)`(구연 경우)(H = 복사본의 bboxMax.z, 아래 30 mm·위 30 mm 만 기울임 — 모의 실험·비평 탐침과 같은 식) → `ArtifactDoc t; t.loadMesh(std::move(m), {}, QStringLiteral("기운 예제")); t.confirmPose();`. 5경우: 바닥 b = −0.3 · −0.5(단면 쪽 x > 0 이 낮음), 구연 r = +0.5 · +1.0(단면 쪽 높음), 구연 r = +1.0 앞쪽(−y) 높음.
  - **25만 면 초과 예제**(기준 24): `KA::Testing::SyntheticParams sp; sp.radialSeg = 360; sp.heightSeg = 240; sp.capTop = false; sp.capBottom = true; sp.tiltDeg = 0; sp.translation = {0, 0, 0}; sp.noiseSigma = 0.05;` → `MakeTorusProfileVessel(82.6, 71.0, 184.5, 6.8, sp).toMesh()` + `FillMeshStats` → `loadMesh` + `confirmPose`(자동 세우기 없이 — 고해상도에서 autoAlign 이 실패하는 것을 비평이 관찰). 34.6만 면, 확정 ≈ 1.25 s.
  - 원 자료: `d.sectionLines()`(§3.5 — 문서가 그린 그 원 선), 단면 짝 = (s.loops[i] ↔ displaySection()[i], 닫힘) 또는 미스캔(s.solids[i] ↔ displayScannedSolid()[i], s.estInner[i] ↔ displayEstimatedInner()[i], 열림). 외곽선 짝 = (orthoOutline() ↔ displayOutline(), 열림).
  - 파일(기준 8·9): 예제 0° 와 미스캔 0° 에서 `ExportRequest R0{}`(A4 세로·1:1·opts 기본) + 제목 `단면 맞춤 시험` 으로 `studio_fit_s.svg`·`studio_fit_s.dxf`·`studio_fit_b.svg`·`studio_fit_b.dxf`, 그리고 원 자료 CSV `studio_fit_s_raw.csv`·`studio_fit_b_raw.csv` — 머리 `kind,idx,x,y`, 값 `%.9f`, kind = `sec`(s.loops), `solid`(s.solids), `est`(s.estInner), `ortho`(rawHalfOrtho, idx 0).
  - 래스터(기준 8): test_reflines.cpp 267-291 의 `drawSheet` 와 같은 방식(8 px/mm, `X0 = L.plateRectMm.left() − E.left()·s`, `Yb = L.plateRectMm.top() + E.bottom()·s`, 도면점 (x,y) → 화소 ((X0 + x)·8, (Yb − y)·8)). **세는 화소는 도판 칸 E(= `Plate::extentMm(d, 0)` = (−r, −8, 2r, H + 26), 도면 y ∈ [−8, H + 18]) 안만** — `paintSheet` 는 opts 와 상관없이 용지 아래 표제란(가로줄·「축척 1:1 …」 글·축척 막대·제목 상자, Plate.cpp:541-570 — A4 1:1 예제에서 도면 y ≈ −31 mm 아래)을 그리기 때문이다.
- **`studio/tests/test_cutpage.cpp`**: `#include <QRegularExpression>`. 437(`[J7-13]` 창 최소 폭 판정) 다음에 `CHECK(!ctxRight().contains(QStringLiteral("최대 편차")), "[J8-10] ② CtxRight …")`. 467(`[J7-15] CtxRight 가 「점 」 으로 시작`) 다음에 ③ 판정(기준 10). JOB-006·JOB-007 판정 줄은 고치지 않는다.

### 3.8 예상 값 (모의 실험, 간격 0.5·곡선 켬·Right)

| 경우 | 단면 y 범위(그림) · 틈 아래/위 | 단면 편차 | 단면 직선 구간 | 외곽선 끝 (x, y)…(x, y) · 원 점 n | 외곽선 편차(반쪽 정사 실루엣 기준) · 직선 구간 | `maxDeviationMm` | 점 수(그림/원) |
|---|---|---|---|---|---|---|---|
| 예제 0° | [0.127, 184.685] · 0.127/0.135 | 0.231 | 6/20 | (−70.627, 0.125)…(−71.033, 184.625) · 739 | 0.180 · 1/9 | 0.231 | 82/1343 |
| 예제 12° | [0.114, 184.690] · 0.114/0.130 | 0.202 | 6 | (−71.003, 0.125)…(−71.065, 184.625) · 739 | 0.191 · 0 | 0.202 | 82/1222 |
| 예제 −20° | [0.153, 184.645] · 0.153/0.175 | 0.187 | 6 | (−70.263, 0.125)…(−71.061, 184.625) · 739 | 0.155 · 1 | 0.187 | 82/1222 |
| 예제 105° | [0.129, 184.641] · 0.129/0.179 | 0.176 | 6 | (−70.969, 0.125)…(−71.058, 184.625) · 739 | 0.154 · 0 | 0.176 | 75/1222 |
| 미스캔 0° | [0.190, 190.148] · 0.190/0.107 | 0.202 | 13 | (−62.078, 0.125)…(−62.031, 190.125) · 761 | 0.173 · 0 | 0.202 | 113/1370 |
| 미스캔 12° | [0.091, 190.152] · 0.091/0.103 | 0.185 | 6 | (−62.010, 0.125)…(−62.053, 190.125) · 761 | 0.185 · 0 | 0.185 | 116/1243 |
| ③ 시험 경로 105°(기운 7° 예제를 들어온 자세로 확정, §4 기준 10) | [0.241, 184.295] · 0.241/16.168 | 0.211 | 7 | (−1.919, 5.375)…(−59.277, 200.375) · 781 | 0.184 · 2 | 0.211 | 94/1272 |
| 25만 면 초과(34.6만 면, 기준 24) | [0.155, 184.671] · 0.155/0.133 | 0.222 | 9 | (−71.002, 0.125)…(−71.027, 184.625) · 739 | 0.174 · 0 | 0.222 | 75/1955 |

예제 0° 간격 0.1 / 0.3 / 1.0: 단면 편차 0.049 / 0.149 / 0.458, 외곽선 편차 0.0497 / 0.143 / 0.433(모두 ≤ 간격/2), 점 수 493 / 84 / 67. 곡선 끔(간격 0.5): 단면 20점 + 외곽선 10점 = **30**(편차 0.231 / 0.232). 예제 0° 곡선 켬: 단면 48점(14구간 × 3 + 6구간 × 1 — 직선 6구간 = 구연 윗면 19 와 양옆 0·18, 바닥 변 8 과 양옆 7·9) + 외곽선 34점(8구간 × 4 + 1구간 × 1 + 끝 1 — 직선은 맨 아래 0번 (−70.627, 0.125)–(−71.126, 0.375)) = **82**, 원 점 = 단면 고리 604 + 반쪽 정사 실루엣 739 = **1343**. H: 예제 184.821, 미스캔 190.256, ③ 시험 경로 200.463, 34.6만 면 184.804.

**기운 예제**(기준 23, 예제 0° 메시를 §3.7 식으로 기울여 `loadMesh` + `confirmPose`, 간격 0.5·곡선 켬):

| 경우 | H | 외곽선 원 점 n · 끝 (x, y)…(x, y) | 외곽선 끝 틈 아래/위 | 외곽선 최대 x | 외곽선 편차 | 단면 y 범위 · 편차 | `maxDeviationMm` | (참고) K-Arch TrueOrtho 최대 x |
|---|---|---|---|---|---|---|---|---|
| 바닥 0.3°(단면 쪽 낮음) | 185.123 | 738 · (−49.203, 0.625)…(−71.042, 184.875) | 0.625 / 0.248 | −49.203 | 0.190 | [0.094, 184.988] · 0.224 | 0.224 | **+51.407** |
| 바닥 0.5° | 185.365 | 738 · (−28.593, 0.875)…(−71.042, 185.125) | 0.875 / 0.240 | −28.593 | 0.161 | [0.114, 185.230] · 0.224 | 0.224 | **+61.174** |
| 구연 0.5°(단면 쪽 높음) | 185.349 | 739 · (−70.630, 0.125)…(−12.018, 184.625) | 0.125 / 0.724 | −12.018 | 0.239 | [0.127, 185.248] · 0.231 | 0.239 | **+43.433** |
| 구연 1° | 185.951 | 739 · (−70.632, 0.125)…(−6.047, 184.625) | 0.125 / 1.326 | −6.047 | 0.180 | [0.127, 185.863] · 0.231 | 0.231 | **+63.696** |
| 구연 1°(앞쪽 −y 높음) | 185.981 | 744 · (−70.626, 0.125)…(−15.252, 185.875) | 0.125 / 0.106 | −15.252 | 0.201 | [0.127, 184.686] · 0.230 | 0.230 | −15.252 |

기운 경우 외곽선 끝 x 가 −49 … −6 인 것은 기운 바닥·구연의 가장자리(정면에서 본 외면 반쪽의 아래·위 윤곽)를 따라 그리기 때문이다(§2). 시험 쪽 전수 계산과 제품 후보 계산은 모든 경우 점 수·좌표가 같았다(`maxdiff=0`).

## 4. 수용 기준 (각 항목은 명령 하나로 참/거짓)

허용오차 근거: 같은 식으로 옮긴 도면 mm 는 1e-9, DXF 는 소수 6자리라 ±2e-6(거리 ±4e-6), SVG 는 소수 3자리라 ±0.002. 편차 상한은 「간격」 칩 값의 절반(단면·외곽선 같음) — 새 매개변수 없음. 「틈 ≤ 0.30 mm」(단면)·「≤ 0.25 mm」(외곽선 = 칸 크기)는 **수평인** 예제 실측 성질의 회귀 판정이다(끌어 붙임 없음 — 공식 허용오차가 아님, 프로필 「허용오차 공식 기준 없음」). 기운 예제(기준 23)는 틈을 판정하지 않고 외곽선 끝 = 원 실루엣의 첫·끝 칸인지, 중심선을 넘지 않는지 본다(§2 「기운 바닥·기운 구연」).

### A. 새 CTest `studio_fit` (`studio/tests/test_fit.cpp`)

1. **[J8-1] 단면이 기준선을 넘지 않음** — 예제 0°·12°·−20°·105°, 미스캔 0°·12° 각각: `H = mesh().bboxMax.z − mesh().bboxMin.z`, `|bboxMin.z| ≤ 1e-9`; 원 자료(§3.7) 단면 선들의 y 범위 [rLo, rHi] 가 `rLo ≥ −1e-9`·`rHi ≤ H + 1e-9`; `displaySection()` 의 모든 점이 `rLo − 1e-9 ≤ y ≤ rHi + 1e-9` 이고 `x ≥ −1e-9`; 그린 단면의 `min y ≤ 0.30` 이고 `H − max y ≤ 0.30`. 인쇄 `[J8-1] <경우> y=[lo, hi] raw=[rLo, rHi] H=… gap a/b` (예상 §3.8 둘째 열, 현재 코드는 0° 에서 [−3.651, 187.917] 로 거짓).
2. **[J8-2] 단면 편차 ≤ 간격/2** — 같은 6경우 + 예제 0° 의 간격 0.1·0.3·1.0(`setSimplifyTol`, 끝나면 0.5): 단면 짝(§3.7)마다 고리·선 수가 같고 `dev2 ≤ 간격/2 + 1e-9`. 인쇄 `[J8-2] <경우> tol=… dev=…`(예상 0.231 · 0.202 · 0.187 · 0.176 · 0.202 · 0.185, 간격별 0.049 · 0.149 · 0.458).
3. **[J8-3] 외곽선 끝이 바닥선·구연선 높이까지** — 같은 6경우: `displayOutline()` 의 모든 점이 `−1e-9 ≤ y ≤ H + 1e-9`·`x ≤ 1e-9`, `front().y` 가 최솟값이고 `back().y` 가 최댓값, `front().y ≤ 0.25`·`H − back().y ≤ 0.25`. 인쇄 `[J8-3] <경우> ends=(x,y)…(x,y) gap a/b · |x|−base/2 … · |x|−rim/2 …`(예상 §3.8 첫 표의 외곽선 끝 열, 현재 코드는 0° 에서 3.362/3.208 로 거짓).
4. **[J8-4] 외곽선 편차 ≤ 간격/2(반쪽 정사 실루엣 기준)** — 같은 6경우 + 예제 0° 간격 0.1·0.3·1.0: `orthoOutline()` 의 모든 점이 `x ≤ 0`·`mesh().bboxMin.z ≤ y ≤ mesh().bboxMax.z` 이고 y 가 순증가, 외곽선 짝 `dev2 ≤ 간격/2 + 1e-9`. 인쇄 `[J8-4] <경우> tol=… n=… dev=…`(예상 0.180 · 0.191 · 0.155 · 0.154 · 0.173 · 0.185, 간격별 0.0497 · 0.143 · 0.433 — 원 명세의 외곽선 허용치 = 간격이면 0° 에서 0.433 > 0.25 로 거짓).
5. **[J8-5] 외곽선 = 막과 수직인 실루엣의 외면 반쪽(시험 쪽 독립 계산)** — 예제 0°·12°·−20°, 미스캔 0°: `rawHalfOrtho(mesh())`(§3.7) 와 `orthoOutline()` 의 **점 수가 같고**(예제 739, 미스캔 761) 칸마다 `|x − x′| ≤ 1e-9`·`|y − y′| ≤ 1e-9`; 예제 0° 에서 `rawHalfOrtho` 가 전수 구현(전 칸 × 전 면)과 점 수·좌표 `==`. 인쇄 `[J8-5] <경우> n=… maxdiff=…`(예상 n 739/739/739/761, maxdiff 0 — 현재 코드는 `orthoOutline()` 이 없어 컴파일부터 거짓, 옛 MaxRadius 는 n 714).
6. **[J8-6] 최대 편차 함수** — 기준 2·4 의 각 상태에서 `|maxDeviationMm() − max(그 상태의 단면 dev2 들, 외곽선 dev2)| ≤ 1e-9` 이고 `maxDeviationMm() ≤ 간격/2 + 1e-9`; `ArtifactDoc{}.maxDeviationMm() == 0.0`, `loadSample(false)`(확정 전) 뒤 `== 0.0`. 인쇄(예상 예제 0° 0.231, 12° 0.202, −20° 0.187, 105° 0.176, 미스캔 0° 0.202, 12° 0.185, 간격 0.1·0.3·1.0 은 0.0497·0.149·0.458).
7. **[J8-7] 곡선 끔 = DP 그대로 · 점 수** — 예제 0°·간격 0.5: `setSmooth(false)` 뒤 `displaySection().size() == 1` 이고 `displaySection()[0] == simplifyDP(sectionLines().loops[0], 0.25, true)`(꼭짓점마다 `==`), `displayOutline() == simplifyDP(orthoOutline(), 0.25, false)`, `pointCount() == 30`; `setSmooth(true)` 뒤 `displaySection()[0].size() == 48`, `displayOutline().size() == 34`, `pointCount() == 82`, `rawPointCount() == 1343`.
8. **[J8-8] 출력 길(DXF·래스터)과 파일** — (a) `studio_fit_s.dxf`·`studio_fit_b.dxf`: `04_SECTION_WALL`·`06_INTERIOR_OUTLINE` POLYLINE 의 모든 VERTEX 가 `y08 − 2e-6 ≤ y ≤ y07 + 2e-6`(y07·y08 = 같은 파일의 `07_PROJECTION_RIM`·`08_PROJECTION_BASE` LINE 의 y)·`x ≥ −2e-6`, `02_EXTERIOR_OUTLINE` 의 모든 VERTEX 가 같은 띠 안·`x ≤ 2e-6`·`min y − y08 ≤ 0.25 + 2e-6`·`y07 − max y ≤ 0.25 + 2e-6`. (b) 래스터 = `paintSheet`(예제 0°, A4 세로 1:1, 8 px/mm), **세는 화소는 도판 칸 E = `Plate::extentMm(d, 0)` 안만**(도면 x ∈ [E.left(), E.right()], y ∈ [E.top(), E.bottom()] = [−8, H + 18] — 용지 아래 표제란 제외, §3.7): 단면만(`grid = shade = outline = labels = false`, `section = true`)일 때 `qRed − qGreen > 100` 화소가 도면 y ∈ (H + 0.5, E.bottom()] 인 행과 y ∈ [E.top(), −0.5) 인 행에 **0개**, y ∈ [H − 1, H] 행과 y ∈ [0, 1] 행에는 1개 이상; 외곽선만(`outline = true`, 나머지 끔)일 때 열 x ∈ [E.left(), −2](도면 mm, 중심선 제외)에서 `min(R,G,B) < 245` 화소가 y ∈ (H + 0.5, E.bottom()] · y ∈ [E.top(), −0.5) 행에 **0개**, y ∈ [0.3, 1.0] 행과 y ∈ [H − 1.0, H − 0.3] 행에는 1개 이상(외곽선이 바닥선·구연선 바로 앞까지 옴 — 현재 코드는 3.36 mm 에서 끝나 거짓). (c) §3.7 의 파일 6개(SVG 2 · DXF 2 · CSV 2)가 `B` 에 있다. 그리고 `test_studio_fit` 마지막 줄 `PASS`, 종료 코드 0.
23. **[J8-23] 기운 바닥·구연에서도 외곽선이 중심선을 넘지 않고 실측대로 끝남** — §3.7 의 기운 예제 5경우(바닥 0.3°·0.5° 단면 쪽 낮음, 구연 0.5°·1° 단면 쪽 높음, 구연 1° 앞쪽 높음) 각각: (a) `rawHalfOrtho(mesh())` 와 `orthoOutline()` 의 점 수가 같고 칸마다 `|x − x′|`·`|y − y′| ≤ 1e-9`; (b) `displayOutline()` 의 모든 점 `x ≤ 1e-9`, `front()`·`back()` 이 `orthoOutline().front()`·`back()` 과 좌표 `==`(끝점 = 원 실루엣의 첫·끝 칸), 모든 점 y 가 [front().y, back().y] 안; (c) 외곽선 짝 `dev2 ≤ 간격/2 + 1e-9`; (d) `displaySection()` 의 모든 점 `x ≥ −1e-9` 이고 원 선 y 범위 [rLo, rHi] ± 1e-9 안, 단면 짝 `dev2 ≤ 간격/2 + 1e-9`; (e) `maxDeviationMm() ≤ 간격/2 + 1e-9`. 인쇄 `[J8-23] <경우> H=… n=… ends=(x,y)…(x,y) gap a/b maxX=… olDev=… secDev=… maxDev=…`(예상 §3.8 기운 예제 표 — 원 명세의 K-Arch TrueOrtho 라면 최대 x +51.4 · +61.2 · +43.4 · +63.7 로 (b) 거짓).
24. **[J8-24] 25만 면 초과 메시도 외곽선은 원본에서** — §3.7 의 34.6만 면 예제: `mesh().faces.size() > 250000` 이고 `lodMesh().faces.size() < mesh().faces.size()`; `rawHalfOrtho(mesh())` 와 `orthoOutline()` 의 점 수·좌표가 같고(≤ 1e-9); `displayOutline()` 의 모든 점 `x ≤ 1e-9`, `front().y ≤ 0.25`·`H − back().y ≤ 0.25`(H = `mesh()` 기준); 외곽선 짝 `dev2 ≤ 간격/2 + 1e-9`; `maxDeviationMm() ≤ 간격/2 + 1e-9`. 인쇄 `[J8-24] faces=… lod=… n=… ends=(x,y)…(x,y) gap a/b olDev=… maxDev=… · dims base=… wall=… (LOD 치수 — 판정 아님)`(예상 faces 346320, lod 31103, n 739, ends (−71.002, 0.125)…(−71.027, 184.625), gap 0.125/0.179, olDev 0.174, maxDev 0.222, dims base 5.20 wall 3.85 — §2·§10 의 기존 LOD 치수 결함 기록. 원 명세처럼 LOD 에서 잡으면 끝 (24.256, 184.732) 로 x 판정 거짓).

### B. 독립 파이썬 판정 (§5-3, `B` 에 시험이 남긴 파일)

9. 판정 스크립트가 `PY-FIT OK` 를 찍고 종료 코드 0 — DXF 두 개: 단면(04·06)·외곽선(02) 꼭짓점이 같은 파일의 바닥선 y ~ 구연선 y 띠 안(±2e-6), 단면 x ≥ 0·외곽선 x ≤ 0, 단면 틈 ≤ 0.30·외곽선 틈 ≤ 0.25, **CSV 원 선과의 양방향 거리**: 단면 ≤ 0.25 + 4e-6(예제 `sec` ↔ 04 닫힌 선, 미스캔 `solid` ↔ 04·`est` ↔ 06), 외곽선 ↔ `ortho` ≤ 0.25 + 4e-6; SVG(예제): `section_profile` polygon·`outline_axis` polyline 의 y 가 `refline_rim` y ~ `refline_base` y 사이(±0.002), 외곽선 끝이 두 선에서 0.25 + 0.002 안. 인쇄 예상: `s section y [0.127, 184.685] H 184.821 dev 0.231 | outline y [0.125, 184.625] dev(ortho) 0.180`, `b section y [0.190, 190.148] H 190.256 dev 0.202 | outline y [0.125, 190.125] dev(ortho) 0.173`.

### C. `studio_cutpage` 확장 (실제 MainWindow, 오프스크린)

10. **[J8-10] ③ 상황 줄 최대 편차** — [J7-13](② 105°) 시점 `CtxRight` 에 `최대 편차` 가 **없다**; [J7-15](③ 105°) 시점 `CtxRight` 가 정규식 `^점 [0-9,]+ → <b>[0-9]+개</b> \(0\.5 mm 간략화\) · 최대 편차 (0\.[0-9]{2}) mm$` 에 맞고 잡힌 값 ≤ 0.25(= 간격/2, 소수 둘째 자리 반올림 값). 인쇄 `[J8-10] CtxRight '…'`(예상 `점 1,272 → <b>94개</b> (0.5 mm 간략화) · 최대 편차 0.21 mm` — 이 시험은 예제를 **들어온 자세(7° 기울기·이동 (12,−5,3), ArtifactDoc.cpp:276-277)로 열어 ① 에서 Enter 로 그대로 확정**하므로(test_cutpage.cpp:184 `openSample(true)` → MainWindow.cpp:570 `loadSample(false)`, test_cutpage.cpp:230 Enter → ArtifactDoc.cpp:488-501 회전 없이 중심만 원점) 자동 세운 예제의 §3.8 첫 줄들과 값이 다르다 — §3.8 「③ 시험 경로 105°」 줄. 판정은 형식과 ≤ 0.25 뿐). `test_studio_cutpage` 마지막 줄 `PASS`, 종료 코드 0(JOB-006·JOB-007 판정 모두 그대로 참).

### D. 회귀

11. CTest 18종(기존 17 + `studio_fit`): `B` 에서 `ctest --output-on-failure` 출력에 `100% tests passed` 와 `out of 18` 이 같은 줄에 있고, `tests failed` 가 0 이 아닌 줄이 없다.
12. **바뀌는 로그 값**(verify.ps1 직후 `B\Testing\Temporary\LastTest.log`): `grep -c "H=184.80 rim=142.00 belly=165.50 base=142.00 wall=6.80 pts=82/1343 lodFaces=26180"` ≥ 1 이고 `grep -c "pts=85/1318"` = 0 — **pts 만 85/1318 → 82/1343**(이유: 단면 구연·바닥 모서리 6구간이 곡선 대신 간략 꺾은선이라 60 → 48점, 외곽선이 반쪽 정사 실루엣·전 높이·평활 없음·허용치 간격/2 라 원 점 714 → 739·그린 점 25 → 34). 그대로인 값: `grep -c "STL(m->mm): verts=38720 H=184.80 rim=142.00 noiseRemoved=2"` ≥ 1, `grep -c "ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95"` ≥ 1. JOB-007 시험 인쇄의 바뀐 틈: `grep -cF "outline ends=(-70.627,0.125)…(-71.033,184.625) gap low 0.125 high 0.196 mm"` ≥ 1(예전 `gap low 3.362 high 3.208`).
13. verify.ps1 종료 코드 0, `B\verify-summary.md` 마지막 줄 `결과: PASS`, 빌드 경고 0: `grep -ci "warning" D:/_dongarch_build/verify_job008/studio-logs/02_build.log` = `0`.
14. 범위(Git Bash, `cd W`, **builder 커밋 뒤** `git status --porcelain` 출력 없음): `git diff --name-only 89846ba HEAD | grep -vxF -e studio/src/ArtifactDoc.h -e studio/src/ArtifactDoc.cpp -e studio/src/MainWindow.cpp -e studio/tests/test_fit.cpp -e studio/tests/test_cutpage.cpp -e studio/CMakeLists.txt -e docs/ui/UI_RULES.md` → 출력 없음, 그리고 `git diff --name-only 89846ba HEAD -- GigaMesh studio/src/Plate.h studio/src/Plate.cpp studio/tests/test_pipeline.cpp studio/tests/test_pose.cpp studio/tests/test_side.cpp studio/tests/test_cut.cpp studio/tests/test_reflines.cpp` → 출력 없음(K-Arch·도판 엔진·기존 시험 무변경).

### E. 코드·정본 (Git Bash, `cd W`, 커밋 뒤)

15. 흐름 코드: `grep -c "SilhouetteMode::TrueOrtho" studio/src/ArtifactDoc.cpp` = `0`, `grep -c "SilhouetteMode::MaxRadius" studio/src/ArtifactDoc.cpp` = `1`(치수용 그대로), `grep -c "halfOrthoSilhouette(mesh_, 0.25)" studio/src/ArtifactDoc.cpp` = `1`(외곽선은 원본에서 — `fastMesh`·`lodMesh_` 아님), `grep -c "sil_.leftOutline" studio/src/ArtifactDoc.cpp` = `0`(K-Arch 실루엣을 그리지 않음), `grep -rn "smoothCatmull" studio/src` → 출력 없음, `grep -c "fitCurve(" studio/src/ArtifactDoc.cpp` ≥ `2`(정의 + 사용 — 도움 함수로 묶는 것을 허용, 표시 결과는 기준 6·7 이 판정), `grep -c "maxDeviationMm" studio/src/ArtifactDoc.h` ≥ `1`, `grep -c "maxDeviationMm()" studio/src/MainWindow.cpp` = `1`, `grep -cF "· 최대 편차 %4 mm" studio/src/MainWindow.cpp` = `1`.
16. R2: `git diff 89846ba -- studio/src | grep -E '^\+.*(#[0-9A-Fa-f]{6}|QColor\(|qRgba?\(|Qt::(red|green|blue|yellow|white|black|gray)\b)'` → 출력 없음(새 색 없음).
17. 정본: `grep -c "막과 수직인 실루엣의 외면 반쪽" docs/ui/UI_RULES.md` ≥ `1`, `grep -c "실측 점을 잇는 간략 꺾은선" docs/ui/UI_RULES.md` ≥ `1`, `grep -c "최대 편차" docs/ui/UI_RULES.md` ≥ `1`.

### F. 캡처 (18-20 명령 판정, 21-22 ui-reviewer 예/아니오 — 실제 글꼴 캡처는 §5-4)

18. 시작 화면 무변경: PowerShell `(Get-FileHash "D:\_dongarch_build\verify_job008\studio\shots\shot_home.png").Hash -eq (Get-FileHash "D:\_dongarch_build\wt\factory\jobs\JOB-007\artifacts\shot_home.png").Hash` → `True`(사용자 QSettings 최근 목록이 바뀌었으면 코드와 무관하게 다를 수 있다 — 다르면 FAIL 로 두고 원인을 decision.md 에).
19. verify 의 `shot_main.png`·`shot_export.png`(오프스크린) 각각 > 5120 바이트.
20. 실제 글꼴 화소 비교(§5-5)가 `SHOTS OK` — `A8` 의 `shot_main_font.png`·`shot_cut_font.png`·`shot_edit_font.png` 와 `A7` 의 같은 이름 파일(1845×920)의 화소 차이(채널 차 > 8)가 **도판 칸 안에만** 있고(홈·② x 628–1844 · y 201(②: 237)–889, ③ x 507–1844 · y 237–889) ③ 은 상황 줄 오른쪽(x 1300–1844 · y 141–176)도 허용, 그 밖(리본·3D·슬롯 목록·치수 띠·상태줄) 차이 **0**, 그리고 세 장 모두 도판 칸 안 차이 > 0.
21. `shot_main_font.png`(홈 0°)·`shot_cut_font.png`(② 12°): 빨강 단면의 꼭대기가 구연선 위로, 바닥이 바닥선 아래로 나가지 않고(JOB-007 캡처의 구연선 위 ≈ 10 px 솟음 없음), 왼쪽 외면 외곽선이 바닥선·구연선 왼쪽 끝 바로 앞까지 이어진다(JOB-007 의 3 mm ≈ 9 px 틈 없음). 구연 윗면·바닥 변은 곧은 간략 꺾은선으로 보인다.
22. `shot_edit_font.png`(③ 12°): 상황 줄 오른쪽 `점 1,222 → 82개 (0.5 mm 간략화) · 최대 편차 0.20 mm`(굵게 `82개`), 도판은 21 과 같은 모양, 나머지(리본 5그룹·`막 12° 고정`·3D 330 px·상태줄)는 JOB-007 과 같음.

## 5. 검증 명령

1. 표준 검증(작업 폴더 `D:\_dongarch_build\wt`): `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-008 -BuildRoot D:\_dongarch_build\verify_job008 -Job JOB-008 -RecordRoot D:\_dongarch_build\wt` (로그 `L\01_configure.log … 04_shots.log`, 산출물 `A8`). 직후 기준 11·12·13·18·19. 이 PC 의 CTest 는 실패가 없으면 `100% tests passed out of N` 만 찍는다.
2. 개별 시험(PowerShell, `--test-dir` 금지 — 빌드 폴더로 이동): `Remove-Item Env:\CI -ErrorAction SilentlyContinue; $env:PATH='D:\OSGeo4W\apps\Qt6\bin;'+$env:PATH; $env:QT_PLUGIN_PATH='D:\OSGeo4W\apps\Qt6\plugins'; $env:QT_QPA_PLATFORM='offscreen'; Set-Location D:\_dongarch_build\verify_job008\studio; ctest -R "studio_fit|studio_cutpage" -V` → 기준 1-8·10·23·24(`[J8-n]` 인쇄, 두 시험 마지막 `PASS`; 실패 줄은 `FAIL … [J8-n] …`). 개별 실행은 LastTest.log 를 덮어쓰므로 기준 12 는 1 직후 또는 전체 `ctest --output-on-failure` 재실행 뒤에 본다.
3. 독립 파이썬 판정(Git Bash, 2 또는 1 뒤 — 파일은 `B`). 0열에서 그대로 붙여 넣는다(스크립트에 역슬래시 없음 — Git Bash heredoc 이 역슬래시를 줄이므로) — 기준 9:
```bash
"/c/Users/kyi25/AppData/Local/Programs/Python/Python312/python.exe" -I - "D:/_dongarch_build/verify_job008/studio" <<'PY'
# -*- coding: utf-8 -*-
import csv, math, os, sys, xml.etree.ElementTree as ET
B = sys.argv[1]; P = lambda n: os.path.join(B, n)
def recs(p):
    L = open(p, encoding='latin-1').read().splitlines(); out = []
    for i in range(0, len(L) - 1, 2):
        c, v = int(L[i].strip()), L[i + 1].strip()
        if c == 0: out.append((v, []))
        elif out: out[-1][1].append((c, v))
    return out
def g1(r, c):
    v = [x for k, x in r[1] if k == c]; return v[0] if v else None
def dxf(n):
    polys, lines, cur = {}, {}, None
    for r in recs(P(n)):
        if r[0] == 'POLYLINE': cur = []; polys.setdefault(g1(r, 8), []).append(cur)
        elif r[0] == 'VERTEX' and cur is not None: cur.append((float(g1(r, 10)), float(g1(r, 20))))
        elif r[0] == 'SEQEND': cur = None
        elif r[0] == 'LINE': lines[g1(r, 8)] = [float(g1(r, c)) for c in (10, 20, 11, 21)]
    return polys, lines
def sd(p, a, b):
    dx, dy = b[0] - a[0], b[1] - a[1]; L2 = dx * dx + dy * dy
    t = 0.0 if L2 == 0 else max(0.0, min(1.0, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / L2))
    return math.hypot(p[0] - a[0] - t * dx, p[1] - a[1] - t * dy)
def ld(p, l, closed):
    n = len(l); return min([math.hypot(p[0] - l[0][0], p[1] - l[0][1])] + [sd(p, l[i], l[(i + 1) % n]) for i in range(n if closed else n - 1)])
def dev(raw, drawn, closed):
    return max(max(ld(p, raw, closed) for p in drawn), max(ld(p, drawn, closed) for p in raw))
def rawcsv(n):
    R = {}
    for row in csv.DictReader(open(P(n), encoding='utf-8')):
        R.setdefault(row['kind'], {}).setdefault(int(row['idx']), []).append((float(row['x']), float(row['y'])))
    return R
for tag in ('s', 'b'):
    polys, lines = dxf('studio_fit_' + tag + '.dxf'); R = rawcsv('studio_fit_' + tag + '_raw.csv')
    H, y0 = lines['07_PROJECTION_RIM'][1], lines['08_PROJECTION_BASE'][1]
    sec = polys.get('04_SECTION_WALL', []) + polys.get('06_INTERIOR_OUTLINE', [])
    ys = [q[1] for pl in sec for q in pl]; xs = [q[0] for pl in sec for q in pl]
    assert min(ys) >= y0 - 2e-6 and max(ys) <= H + 2e-6 and min(xs) >= -2e-6, (tag, 'section band', min(ys), max(ys), min(xs))
    assert min(ys) - y0 <= 0.30 and H - max(ys) <= 0.30, (tag, 'section gap', min(ys) - y0, H - max(ys))
    ol = polys['02_EXTERIOR_OUTLINE'][0]; oy = [q[1] for q in ol]
    assert min(oy) >= y0 - 2e-6 and max(oy) <= H + 2e-6 and max(q[0] for q in ol) <= 2e-6, (tag, 'outline band', min(oy), max(oy))
    assert min(oy) - y0 <= 0.25 + 2e-6 and H - max(oy) <= 0.25 + 2e-6, (tag, 'outline gap', min(oy) - y0, H - max(oy))
    if tag == 's': pairs = [(R['sec'][0], polys['04_SECTION_WALL'][0], True)]
    else: pairs = [(R['solid'][0], polys['04_SECTION_WALL'][0], False), (R['est'][0], polys['06_INTERIOR_OUTLINE'][0], False)]
    ds = max(dev(r, d, c) for r, d, c in pairs); do = dev(R['ortho'][0], ol, False)
    assert ds <= 0.25 + 4e-6 and do <= 0.25 + 4e-6, (tag, 'deviation', ds, do)
    print(tag, 'section y [%.3f, %.3f] H %.3f dev %.3f | outline y [%.3f, %.3f] dev(ortho) %.3f' % (min(ys), max(ys), H, ds, min(oy), max(oy), do))
S = '{http://www.w3.org/2000/svg}'
gs = {g.get('id'): g for g in ET.parse(P('studio_fit_s.svg')).getroot() if g.tag == S + 'g'}
ry = float(next(gs['refline_rim'].iter(S + 'line')).get('y1')); by = float(next(gs['refline_base'].iter(S + 'line')).get('y1'))
pts = lambda e: [tuple(map(float, q.split(','))) for q in e.get('points').split()]
sy = [q[1] for e in gs['section_profile'].iter(S + 'polygon') for q in pts(e)]
oy = [q[1] for e in gs['outline_axis'].iter(S + 'polyline') for q in pts(e)]
assert ry - 0.002 <= min(sy) and max(sy) <= by + 0.002 and ry - 0.002 <= min(oy) and max(oy) <= by + 0.002, ('svg band', ry, by, min(sy), max(sy), min(oy), max(oy))
assert min(oy) - ry <= 0.252 and by - max(oy) <= 0.252, ('svg outline gap', min(oy) - ry, by - max(oy))
print('svg rim y %.3f base y %.3f | section [%.3f, %.3f] outline [%.3f, %.3f]' % (ry, by, min(sy), max(sy), min(oy), max(oy)))
print('PY-FIT OK')
PY
```
4. 실제 글꼴 캡처(PowerShell 창에서만 — 작은따옴표 인자 때문, JOB-007 §5-4 와 같음): `$env:QT_QPA_FONTDIR='C:\Windows\Fonts'; $b='D:\_dongarch_build\verify_job008\studio'; $a='D:\_dongarch_build\wt\factory\jobs\JOB-008\artifacts'; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--shot',"$b\shots\shot_main_font.png"; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--cut','--shot',"$b\shots\shot_cut_font.png"; Start-Process -Wait -WorkingDirectory $b -FilePath "$b\DongArch3DStudio.exe" -ArgumentList '--sample','--cut','--edit','--shot',"$b\shots\shot_edit_font.png"; foreach ($n in 'shot_main_font.png','shot_cut_font.png','shot_edit_font.png') { (Get-Item "$b\shots\$n").Length; Copy-Item "$b\shots\$n" $a }` (2 와 같은 PATH·QT_PLUGIN_PATH·offscreen 환경, `--shot` 창 1845×920 고정) → 기준 21·22.
5. 화소 비교(Git Bash, 4 뒤) — 기준 20:
```bash
"/c/Users/kyi25/AppData/Local/Programs/Python/Python312/python.exe" -I - <<'PY'
from PIL import Image, ImageChops
A7 = 'D:/_dongarch_build/wt/factory/jobs/JOB-007/artifacts/'; A8 = 'D:/_dongarch_build/wt/factory/jobs/JOB-008/artifacts/'
cases = [('shot_main_font.png', [(628, 201, 1844, 889)]), ('shot_cut_font.png', [(628, 237, 1844, 889)]),
         ('shot_edit_font.png', [(507, 237, 1844, 889), (1300, 141, 1844, 176)])]
ok = True
for n, rects in cases:
    a = Image.open(A7 + n).convert('RGB'); b = Image.open(A8 + n).convert('RGB'); assert a.size == b.size == (1845, 920), (n, a.size, b.size)
    px = ImageChops.difference(a, b).convert('L').point(lambda v: 255 if v > 8 else 0).load()
    inside = outside = 0; out_box = None
    for y in range(920):
        for x in range(1845):
            if px[x, y]:
                if any(x0 <= x <= x1 and y0 <= y <= y1 for x0, y0, x1, y1 in rects): inside += 1
                else:
                    outside += 1; out_box = (x, y) if out_box is None else out_box
    print(n, 'inside', inside, 'outside', outside, 'first outside', out_box); ok = ok and outside == 0 and inside > 0
print('SHOTS OK' if ok else 'SHOTS FAIL')
PY
```
6. 사람 검토 자료(판정 아님): 5 와 같은 두 장에서 구연 모서리(홈 x 1400–1480 · y 270–310)와 바닥 모서리(x 1400–1480 · y 810–840)·외곽선 위·아래 끝(x 1010–1060 같은 y)을 4배로 잘라 붙인 비교 그림을 `A8/fit_corners_before_after.png` 로 남긴다(PIL `crop`·`resize(NEAREST)`).
7. 수용 기준 14-17 은 builder 커밋 뒤 Git Bash 에서 `cd D:/_dongarch_build/wt/.worktrees/JOB-008` 뒤 그대로.
8. verify.md 에 `[J8-1]`…`[J8-10]`·`[J8-23]`·`[J8-24]` 인쇄 줄, 파이썬 두 출력, LastTest 세 줄과 `[J7-1]` 줄, 캡처 크기를 그대로 옮긴다(§3.8 예상값과 다르면 다른 칸을 표시).

## 6. 영향 파일

| 파일 | 변경 | 이유 |
|---|---|---|
| `studio/src/ArtifactDoc.h` | 수정 | `simplifyDP` 에 `kept` 출력(253-254), `smoothCatmull`(255-256) → `fitCurve` 선언, `maxDeviationMm()` 선언(174 옆), `orthoOutline()`·`sectionLines()` 읽기 접근자와 멤버 `ortho_`(219 옆), `silhouette()` 주석(145 — 치수용), 화면용 선 주석(150-152) |
| `studio/src/ArtifactDoc.cpp` | 수정 | 익명 이름공간 `segDist`·`lineDist`·`halfOrthoSilhouette`(17-173 안), `simplifyDP` 인덱스(175-205), `smoothCatmull` 207-233 → `fitCurve`, `recompute` 에 `ortho_ = halfOrthoSilhouette(mesh_, 0.25)`(676 뒤)·확정 전 분기 `ortho_.clear()`(654 옆), 표시 4곳(748-750·760-762·773-775·797-799), `rawPointCount`(1091), `maxDeviationMm`(1083 앞) |
| `studio/src/MainWindow.cpp` | 수정 | ③(② 아닌 페이지) 상황 줄 오른쪽 글에 `· 최대 편차 %4 mm`(664-667) |
| `studio/tests/test_fit.cpp` | 신규 | 수용 기준 1-8·23·24, 파이썬 판정 파일(SVG·DXF·CSV) |
| `studio/tests/test_cutpage.cpp` | 수정 | 기준 10(437 뒤 · 467 뒤), `#include <QRegularExpression>` |
| `studio/CMakeLists.txt` | 수정 | `test_studio_fit`·`studio_fit`(103 뒤) |
| `docs/ui/UI_RULES.md` | 수정 | R5 「선 다듬기」 줄(74 뒤) — 정본(사람 검토 1) |

`GigaMesh/**`(K-Arch `Silhouette.cpp`·`DimensionAnalyzer.cpp` 포함 — 치수용 기존 호출 그대로), `GigaMesh/core`·`external`, `studio/src/Plate.*`·`SheetView.*`(판독의 뜻만 바뀜 — §7)·`CutPageText.h`·`Theme.h`·`MeshIO.*`·`MeshRaster.*`·`CutGesture.h`, 기존 시험 5개, `factory/**`, `.github/**`, `CLAUDE.md` 는 수정하지 않는다(기준 14 의 허용 목록 = 위 표 7개 파일). 코어 수정이 필요한 곳은 없다 — 반쪽 정사 실루엣은 "코어 밖 재구현"(K-Arch `addTriZHits` 와 같은 교점 규칙을 Studio 에).

## 7. 위험·가정

- **좌표계**: mm·+Z 위·바닥판 Z = 0·확정 뒤 중심선 x = y = 0, 도판 단면 = 정면각 θ 의 Y = 0 면, D5 외면 왼쪽·단면 오른쪽(R = 거울상) — 모두 그대로. 곡선 규칙은 부호를 곱하기 전 자연 좌표에서 돌고(`signedX` 그대로), 높이·축 조건 때문에 단면은 실측 y 범위(⊂ [0, H]) 밖·x < 0(중심선 너머)으로, 외곽선은 실루엣 y 범위 밖·x > 0 으로 나가지 않는다. 반쪽 정사 실루엣의 원 점도 정의상 x ≤ 0. 기준선(JOB-007 `plateRefLines`)은 바꾸지 않는다.
- **실측 vs 추정**: 단면 = 막이 자른 고리(실측) → DP·곡선 모두 원 선에서 간격/2 안. 외곽선 = 메시 삼각형의 반쪽 정사 실루엣(실측, 평활 없음 — 「최대 편차」 가 그린 선 ↔ 이 실측 선 전체를 잰다). 미스캔 추정 내벽은 여전히 추정(파선, `06_INTERIOR_OUTLINE`) — 곡선 규칙은 추정선에서 허용치 안을 지킬 뿐 실측으로 바꾸지 않고, 「최대 편차」 에는 추정선의 편차도 들어간다(정의 주석·정본 문구에 명시).
- **끝 틈은 남는다(의도)**: 단면 끝은 막이 자른 그대로라 구연선·바닥선에서 예제 0.06–0.21 mm 떨어지고, 외곽선 끝은 칸 가운데라 수평 예제에서 바닥선 0.125 mm·구연선 ≤ 0.25 mm(예제 0.196) 떨어진다 — 끌어 붙이면 실측값 이동이라 하지 않는다. 1:1 출력 선 굵기(외곽선 0.30–0.50 · 단면 0.35 mm, 둥근 끝)보다 작아 닿아 보인다. **기운 바닥·구연**은 외면 쪽 맨 아래·위가 기준선에서 떨어진 만큼 틈이 남고(바닥 0.3° → 0.625 mm, 구연 1° → 1.326 mm), 그 사이는 기운 가장자리를 따라 거의 수평인 외곽선이 바닥선·구연선 바로 위(아래)에 겹쳐 그려진다(1:1 출력에서 두 줄처럼 보일 수 있음 — 사람 검토 4).
- **외곽선 정의가 바뀜**: MaxRadius(모든 방향 최대 반경의 회전 포락선, 평활) → 반쪽 정사 실루엣(지금 정면각에서 본 외면 반쪽 윤곽, 평활 없음). 대칭 예제에서는 칸별 0.1 mm 안팎 차이뿐이지만 비대칭 실제 스캔은 막 각도에 따라 외곽선이 달라지고(보이는 대로 — 도판 음영 영상 가장자리와 같아짐), ② 끄는 동안은 놓기 전 각의 외곽선이 남았다가 놓은 뒤(원본 다시 계산 ≈ 1.4 s, 그중 반쪽 정사 실루엣 ≈ 7 ms) 바뀐다. 최소 x 에 평활이 없어 튀어나온 정점(돌기·부유 잡음)이 그대로 외곽선에 나온다 — MaxRadius 의 98 백분위·평활이 하던 거름이 없다(실측대로이지만 잡음일 수 있음, 예제 잡음 σ 0.05 mm 는 간격/2 가 흡수). 실제 Artec 스캔 확인은 사람 검토 9.
- **도판 마우스 판독의 뜻이 바뀜**(`SheetView.cpp` 59-68·100-112, 코드 무변경): `outerRadiusAt` 이 `displayOutline()` 과 높이 h 의 첫 교점 |x| 를 「외경 2·rOut」 과 단면 쪽 빨간 점(s·rOut) 자리로 쓴다. 예전(MaxRadius)은 모든 방향 최대 반경, 이제는 지금 보는 방향 외면 반쪽의 폭이다 — 비대칭 유물이면 각도마다 「외경」 이 다르고 점이 단면 외벽 위에 놓이지 않을 수 있으며, 기운 바닥·구연 높이에서는 거의 수평인 구간과 먼저 만나 작은 값이 나올 수 있다. 사람 검토 3, 고치는 일은 후속(§10).
- **25만 면 초과**: 외곽선은 늘 원본에서라 LOD 끝 틈·평활 차이가 없다(기준 24). 치수는 지금처럼 LOD — 34.6만 면 합성 예제에서 저경 5.20·기벽 3.85 로 틀림(§2, 기준 24 는 인쇄만). 바닥선 폭이 저경이라 그 예제의 바닥선은 5.2 mm 로 그려진다.
- **바뀌는 값**: LastTest `pts=85/1318 → 82/1343`(기준 12), JOB-007 `[J7-1]` 인쇄의 틈 3.362/3.208 → 0.125/0.196. 치수(H·구경·최대경·저경·기벽)·`STL`·`ArtecBlind` 줄은 그대로(§3.1 (다)). 미스캔 예제 점 수 127/1344 → 113/1370(로그 판정 없음).
- **부동소수 재현**: 기준 7·12 의 점 수(82·48·34·30)는 §3.3·§3.4 알고리즘을 그대로(같은 식·`<=`·경계 포함) 구현해야 모의 실험과 같다. 다르면 builder 는 문턱을 손보지 말고 차이(어느 경우·몇 점)를 build-log 에 적고 멈춘다 — 오케스트레이터가 판단. 반쪽 정사 실루엣의 `x <= 0` 판정은 x ≈ 0 교점에서 제품·시험 번역 단위의 부동소수 차(≤ 1e-15)로 갈릴 수 있다 — 예제 25경우에서는 모두 같았다(`maxdiff=0`); 갈리면 차이를 적고 멈춘다.
- **보장 범위**(§3.3-5): 곡선 구간은 양방향 ≤ 허용치를 확인했지만 직선 구간은 DP 의 직선 거리 성질이라, 원 선이 현 끝을 넘어 접히는 곳(예: 추정 내벽)에서는 구간 기준 거리가 허용치를 넘을 수 있다 — 「최대 편차」 는 전체 선 기준이라 예제에서 모두 ≤ 간격/2(기준 2·4·6·23·24).
- **성능**: `fitCurve`(단면 + 외곽선) 한 번 ≈ 0.25 ms(지금 ≈ 0.03 ms), 도판 한 번 그리기에 표시 함수 5–8번 → ≤ 2 ms. ② 미리보기 판정(test_cut `[d]` 중앙값 16 ms 한도, 지금 0.72 ms)에 여유. `maxDeviationMm` ≈ 2 ms 는 상황 줄 갱신 때만. `recompute` 에 반쪽 정사 실루엣 ≈ 7 ms(34.6만 면 ≈ 24 ms) 더함 — `ExtractSilhouette`(≈ 1.08 s, 치수용)는 그대로. `studio_fit` 시험 자체는 예제 문서 여러 벌·기운 5벌·34.6만 면 1벌을 확정하므로 수십 초(시간 판정 없음).
- **창 최소 폭**: ③ 상황 줄 오른쪽 글에 `· 최대 편차 0.21 mm` 가 붙어도 그 글(≈ 42자)은 ② 의 `cutpage::ctxRight` 글(`막 105° · 끄는 동안 26,180면 미리보기 · 놓으면 원본 77,440면 다시 계산`, ≈ 50자, CutPageText.h:95-96)보다 짧고, ② 에서 ctx 줄 최소 폭은 1618 px(JOB-007 `ctest_reflines_cutpage_V.log` `[9]`, CtxHint 최소 폭 1 — MainWindow.cpp:343) < 창 최소 폭 1791(리본, `[J7-13]`·`[J7-15]`). `[J7-15]` 의 ≤ 1845 판정이 그대로 확인.
- **JOB-006·007 의존**: 89846ba 위에 쌓는다. JOB-007 이 재작업되면 다시 쌓아야 하고 충돌은 `MainWindow.cpp`(updateCutInfo 658-680 은 JOB-007 이 고친 곳 바로 옆)·`test_cutpage.cpp`·`UI_RULES.md` R5 에서 날 수 있다.
- **R2·R3·R8**: 새 색·새 주 단추 없음, 새 문자열은 `최대 편차` 하나(한국어, 원문 1665 글자 그대로). R4: 숫자는 기존 모노 라벨(`CtxRight`) 안.
- **라이선스**: 새 의존성 없음.

## 8. 사람 검토 항목

1. **[정본] UI_RULES R5 새 줄 「선 다듬기」**(§3.6): 단면 = 막이 자른 그대로, 외곽선 = 막과 수직인 실루엣의 외면 반쪽(평활 없음), 그린 선은 간격의 절반 안·넘는 구간은 실측 점을 잇는 간략 꺾은선·끌어 붙이지 않음, 상황 줄 최대 편차(추정선 포함).
2. **[곡선 모양] 「매끄럽게 (Catmull)」 이 모서리에서 곧아진다**: 예제 0° 단면 20구간 중 6구간(구연 윗면·바닥 변과 그 양옆 — 0·18·19, 7·8·9번)이 곡선 대신 간략 꺾은선, 외곽선 9구간 중 1구간(맨 아래). 간격 0.1 이면 단면 14·외곽선 13구간(허용치가 작을수록 곡선이 덜 들어감). 곡선 조건을 높이 범위·축 쪽으로 좁혀 매끈한 최대경·목은 곧아지지 않는다(바운딩 박스 조건이면 최대경 구간도 곧아졌다 — §3.2). 칩 이름을 그대로 둘지(예: 「매끄럽게 (편차 안에서)」).
3. **[외곽선 정의] MaxRadius → 막과 수직인 실루엣의 외면 반쪽**(디자인 원문 `H` 755 「외면 반쪽의 윤곽 · 막과 수직인 실루엣」): 원본 메시에서 칸마다 x ≤ 0 교점의 최소 x, 평활·복사·보간 없음. 비대칭 유물에서 각도마다 외곽선이 다르고, ② 끄는 동안은 놓기 전 외곽선. 미스캔 예제 저부의 지어낸 수직선(최대 7.8 mm)이 없어진다. 도판 마우스 판독 「외경 2·rOut」 이 「지금 방향 외면 반쪽 폭 × 2」 로 뜻이 바뀐다(§7) — 기존 도판과 모양·판독이 달라지는 것을 받아들일지.
4. **[끝 틈] 끌어 붙이지 않음**: 수평 예제는 단면 끝 0.06–0.21 mm, 외곽선 끝 0.125 / ≤ 0.25 mm 가 기준선과 떨어져 남는다(선 굵기 안). 기운 바닥·구연은 더 큰 틈(바닥 0.3° → 0.625, 구연 1° → 1.326 mm)과 기준선 바로 위를 따라가는 거의 수평인 외곽선이 남는다(§2·§3.8). 「정확히 놓인다」 를 이 정도로 받아들일지, 기운 경우는 ① 바닥 정하기에서 다시 세우라고 안내할지.
5. **[기준선 폭 vs 윤곽 끝]** 바닥선·구연선 폭 = 저경·구경(JOB-007). 새 외곽선 끝 x 는 그 높이의 실루엣이라 예제 바닥에서 0.37 mm(0°)·0.74 mm(−20°) 안쪽, 구연에서 0.03–0.07 mm 바깥 — 기준선 끝이 외곽선 끝을 조금 지나치거나 못 미친다. 원문(`H` 1852-1857)처럼 기준선을 윤곽 끝점까지로 바꿀지(후속).
6. **[최대 편차 표시]** ③(및 ② 가 아닌 페이지의 상황 줄) 오른쪽 `· 최대 편차 0.23 mm`(예제 0°). 정의 = 그린 선 ↔ 원 선(단면 = 막 절단 — 미스캔이면 추정 내벽선 포함, 외곽선 = 반쪽 정사 실루엣, 평활 없음)의 꼭짓점 기준 양방향 최대 거리 — 엔진 평활이 없어져 실측 대비 숫자다. 문구·자리·툴팁 필요 여부.
7. **[허용치 = 간격/2, 외곽선도]** 원 명세는 외곽선 허용치가 간격(기본 0.5 → 그린 외곽선이 원 선에서 0.43 mm 까지)이었다. 이번엔 단면과 같은 간격/2(기본 0.25 mm, 출력 외곽선 굵기 0.30 mm 안)로 바꿨다 — 예제 0° 점 수 81 → 82 로 거의 같다. 이 기본값이 맞는지, 아니면 「간격」 칩 기본값 자체를 바꿀지.
8. **[보장 범위]** 곡선 구간은 양방향 ≤ 허용치, 직선 구간은 DP 직선 거리 성질(§3.3-5) — 접히는 추정선에서는 구간 거리가 허용치를 넘을 수 있다(전체 선 기준은 지킴).
9. **[손 확인]** 실제 화면에서 구연·바닥 모서리 모양(§5-6 비교 그림), Illustrator·CAD 1:1, 실제 Artec 스캔(평활이 없으니 잡음·돌기가 외곽선에 그대로 나오는지, 비대칭 유물에서 각도별 외곽선, 손빚음 토기의 기운 바닥·구연 끝 모양).
10. **[25만 면 초과 치수]** 이번 모의 실험에서 34.6만 면 합성 예제의 LOD 치수가 저경 5.2 mm·기벽 3.85 mm 로 틀렸다(외곽선은 이번 변경으로 원본에서 계산해 정상). 실제 Artec 완형 스캔(보통 25만 면 초과)이 MVP 완료 게이트이므로 치수를 원본에서 계산하는 후속 작업을 먼저 할지(§10).

## 9. 크기

| 묶음 | 파일 | 예상 줄(추가+수정) | 빈 줄·주석 제외 |
|---|---|---|---|
| 곡선 규칙 | ArtifactDoc.cpp — `segDist`·`lineDist` ≈ 12, `simplifyDP` 인덱스 ≈ 6, `fitCurve` ≈ 40(− `smoothCatmull` 27 삭제) | ≈ 58 | ≈ 48 |
| 반쪽 정사 실루엣 | ArtifactDoc.cpp `halfOrthoSilhouette` ≈ 32, recompute·확정 전 분기·`rawPointCount` ≈ 8, ArtifactDoc.h(`ortho_`·`orthoOutline()`·주석) ≈ 6 | ≈ 46 | ≈ 34 |
| 표시 4곳 | ArtifactDoc.cpp | ≈ 8 | ≈ 8 |
| 최대 편차 | ArtifactDoc.cpp `maxDeviationMm` ≈ 20, ArtifactDoc.h(선언·`sectionLines()`) ≈ 12, MainWindow.cpp ≈ 2 | ≈ 34 | ≈ 22 |
| **제품 합계** | | **≈ 146** | **≈ 112** |
| 시험·빌드 | test_fit.cpp(신규 ≈ 405 — 기운 5경우·34.6만 면 ≈ 75 더함), test_cutpage.cpp(+≈ 10), CMakeLists.txt(+3) | ≈ 418 | — |
| 정본 | UI_RULES.md | 1 | — |

제품 300줄 지침 안이라 나누지 않는다(§0). 시험이 큰 까닭: 단면·외곽선 각각을 원 자료(시험 쪽 독립 반쪽 정사 실루엣 포함)와 거리로 판정하고, 기운 예제·25만 면 초과 예제를 만들고, DXF·래스터 두 출력 길과 파이썬용 CSV 를 함께 남기기 때문이다.

## 10. 후속 메모 (등록·명세는 코디네이터 몫)

- **25만 면 초과 치수를 원본에서**(§2·§8-10): 34.6만 면 합성 예제의 LOD 치수가 저경 5.2 mm·기벽 3.85 mm 로 틀렸다(프로필 위험 3이 실제 오류로 확인됨). 실제 Artec 완형 스캔이 완료 게이트라 우선순위 높음.
- **치수용 K-Arch 실루엣 호출 없애기**: `AnalyzeDimensions` 는 실루엣을 `maxBodyZ` 대체값에만 쓰므로(§3.1 (다)) `ExtractSilhouette`(≈ 1.08 s — 두 방식을 늘 다 계산) 대신 `ortho_` 로 만든 결과를 넘기면 확정·놓기 때 ≈ 1 s 줄어든다. 비대칭 경고(`ASYMMETRIC_GEOMETRY`)는 Studio 에서 안 쓰임 확인 필요.
- **도판 마우스 판독**(§7): 「외경」 을 단면 외벽(같은 높이의 `displaySection` 교점)이나 정점 반경에서 계산하도록.
- **외곽선 돌기·잡음 거름**: 평활 없는 반쪽 정사 실루엣에 칸 안 백분위 같은 거름을 둘지 — 실제 Artec 스캔 확인 뒤(§8-9).
- **기준선 폭 = 윤곽 끝점**(원문 1852-1857, §8-5) — 기준선 2차와 함께.
- **출력 단면 두 줄 윤곽·채움 없음**(백로그 3): 닫힌 고리에 `fitCurve` 를 먼저 적용한 뒤 외벽·내벽으로 나눌 것(구연·바닥의 간략 꺾은선 유지).
- 리본 「선 다듬기」 의 `N → M점`·`편차 d mm`·「겹쳐 보기」·간격 「원본」 칩(`H` 1478).
- ② 끄는 동안 외곽선 갱신(반쪽 정사 실루엣 자체는 원본에서도 ≈ 7 ms — 원본을 미리보기 각으로 돌리는 비용·LOD 로 할 때의 끝 오차는 따로 재야 함).
- 실제 Artec 완형 스캔으로 편차·틈·정사 실루엣 잡음 확인(프로필 「완료 게이트」).
- 구연선 = 최고점(백로그 23)·결실·기운 구연 — 단면 꼭대기와 구연선의 틈을 「실측대로」 보이는 지금 방식과 함께 정리.

## 11. 비평 반영 기록

비평 18건(must 3 · should 10 · nit 5, 정확성·코드 두 관점 — 겹치는 지적은 같은 수정으로 반영). 반영할 때 코드(`W`)를 다시 읽고, 숫자는 scratchpad `exp8r/sim.cpp` 로 다시 모의 실험했다(JOB-007 빌드 라이브러리 링크, 제품 코드 무변경).

| # | 지적 (심각도 · 관점 · 위치) | 반영 | 이유·확인 |
|---|---|---|---|
| 1 | must · 정확성 · §3.2·§3.4 K-Arch TrueOrtho 는 칸마다 메시 전체 최소 x + `finishProfile` 평활 → 기운 바닥·구연에서 외곽선이 중심선을 건넘 | 반영 | Silhouette.cpp:204-221·76-90·37-44 로 확인, 모의 실험에서 TrueOrtho 최대 x +51.4/+61.2/+43.4/+63.7 재현. 외곽선을 Studio 의 반쪽 정사 실루엣(원본 `mesh_`, x ≤ 0 교점, 빈 칸 비움, 평활 없음)으로 바꿈(§3.1 (라)·§3.2·§3.4), 기운 예제 5경우 기준 23 신설, 끝 틈은 「실측대로」(§2). K-Arch·치수 무변경 |
| 2 | must · 정확성 · 기준 8(b) 외곽선 래스터 행에 표제란이 들어감 | 반영 | Plate.cpp:476-489(표제란 위치)·541-570(opts 와 무관하게 그림) 확인, 계산(도면 y ≈ −31.1)도 맞음. 세는 화소를 도판 칸 E 안([−8, H + 18])으로 묶음(기준 8(b)·§3.7) |
| 3 | should · 정확성 · §3.5·§3.6 「최대 편차」 가 엔진 평활 차이를 못 잼 | 반영 | 1 의 수정으로 외곽선 원 선이 평활 없는 실측 실루엣이 되어 함께 해소. 정의·정본·§7·§8-6 문구 고침 |
| 4 | should · 정확성 · §3.4 25만 면 초과면 LOD 실루엣 | 반영 | ArtifactDoc.cpp:674 확인. 외곽선은 늘 `mesh_` 에서(§3.4), 34.6만 면 합성 예제(기울기 0, autoAlign 없이) 기준 24 신설 — 모의 끝 틈 0.125/0.179, LOD TrueOrtho 라면 끝 (24.256, 184.732). 이 과정에서 LOD **치수**가 저경 5.2·기벽 3.85 로 틀리는 것을 새로 찾아 §2·§7·§8-10·§10 에 기록(범위 밖) |
| 5 | should · 정확성 · §3.3-3 외곽선 허용치 = 간격이 느슨함 | 반영 | 외곽선도 간격/2(§3.3-3). 모의 결과 예제 0° 점 수 81 → 82 로 거의 같고 외곽선 편차 0.433 → 0.180. 사람 검토 7 로 기본값 확인 요청 |
| 6 | should · 정확성 · 기준 15 `fitCurve(` ≥ 6 이 도움 함수 허용과 충돌 | 반영 | 기준 15 를 ≥ 2(정의 + 사용)로, 표시 결과는 기준 6·7 이 판정 |
| 7 | nit · 정확성 · 시험이 단면 원 자료를 다시 자름(씨앗 경로면 어긋남) | 반영 | ArtifactDoc.cpp:706-708·845-847·964-981 확인. `sectionLines()` 읽기 접근자 추가(§3.5), 시험은 그것을 씀(§3.7·기준 7) |
| 8 | nit · 정확성 · 정의 주석이 추정 내벽을 빼먹고 「실측 꺾은선」 이 부정확 | 반영 | 정의에 「추정 내벽선도 포함」, 문구를 「실측 점을 잇는 간략 꺾은선」 으로(§3.3·§3.5·§3.6, 기준 17 grep 도 새 문구) |
| 9 | must · 코드 · 기준 8(b) 표제란(2 와 같은 지적) | 반영 | 2 와 같은 수정 |
| 10 | should · 코드 · 기준 10 예상값이 test_cutpage 경로(기운 7° 예제를 들어온 자세로 확정)와 다름 | 반영 | test_cutpage.cpp:184·MainWindow.cpp:570·ArtifactDoc.cpp:276-277·488-501 확인(Enter 는 test_cutpage.cpp:**230** — 비평의 227 은 조금 어긋남). 같은 경로로 다시 모의: `점 1,272 → 94개 … 0.21 mm`, §3.8 에 한 줄 추가, 판정은 형식과 ≤ 0.25 |
| 11 | should · 코드 · 기준 15 개수(6 과 같은 지적) | 반영 | 6 과 같은 수정 |
| 12 | should · 코드 · 25만 면 초과 LOD 끝 틈(4 와 같은 지적) | 반영 | 4 와 같은 수정(원본에서 계산 + 기준 24) |
| 13 | should · 코드 · 정본 문장이 평활·직선 구간 보장 범위를 무조건처럼 말함 | 반영 | 평활이 없어져 앞 절반은 해소, 직선 구간 보장 범위는 §3.3-5·§7·§8-8 에 그대로 둠 |
| 14 | should · 코드 · `SheetView::outerRadiusAt` 「외경」 판독의 뜻이 바뀜 | 반영(문서) | SheetView.cpp:59-68·100-112 확인. §2·§7·§8-3 에 적고 고치는 일은 §10 후속 — `SheetView.*` 는 이번 허용 파일 밖이고 판독 규칙은 따로 정할 일 |
| 15 | should · 코드 · `fitCurve` 바운딩 박스 조건이 x 극값까지 막음 | 반영 | 모의 실험으로 확인(바운딩 박스면 예제 0° 최대경 14번 구간이 직선, 직선 7구간 = §8-2 의 6 보다 하나 많던 까닭). 조건을 높이 범위 + 축 쪽으로 좁힘(§3.2·§3.3), §3.8·기준 7·12 값 다시 모의(직선 6구간 = 구연·바닥 변과 양옆) |
| 16 | nit · 코드 · §7 창 최소 폭 근거가 측정값과 다름 | 반영 | JOB-007 로그 `[9] ctx minimumSizeHint width=1618`·`[J7-13] 1791` 과 CutPageText.h:95-96 글 길이로 근거를 고침 |
| 17 | nit · 코드 · 기준 15 `smoothCatmull` grep 범위가 tests 포함 | 반영 | 범위를 `studio/src` 로 |
| 18 | nit · 코드 · §8-3 에 채움 칸 제거(`erase_if`) 규칙 누락 | 해당 없음 | 1 의 수정으로 K-Arch `fillGaps`·`erase_if` 경로가 없어져(빈 칸은 처음부터 비움) 적을 규칙이 사라짐 |
