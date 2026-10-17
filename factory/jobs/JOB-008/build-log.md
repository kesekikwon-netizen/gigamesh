# JOB-008 구현 기록 (builder)

worktree `D:\_dongarch_build\wt\.worktrees\JOB-008`, 브랜치 `factory/JOB-008`, 기준 `89846ba`. 명세 비평 반영판(§3 설계 결정) 그대로 구현.
§0 분할 판단: 제품 코드 실측 **+171 / −41줄**(빈 줄·주석 제외, `git diff 89846ba` 기준 — ArtifactDoc.h +12/−4, ArtifactDoc.cpp +156/−35, MainWindow.cpp +3/−2) → 300줄 지침 안이라 **나누지 않음**(한 커밋). 명세 추정 112줄보다 많은 것은 줄 바꿈(80열) 때문이고 내용은 §3 그대로.

## 바꾼 파일
| 파일 | 내용 | 이유 |
|---|---|---|
| `studio/src/ArtifactDoc.h` | `simplifyDP` 에 `std::vector<size_t> *kept = nullptr` 출력, `smoothCatmull` 선언 → `fitCurve(raw, tol, closed, smooth, sub)`(§3.3-2 주석), `orthoOutline()`·`sectionLines()` 읽기 접근자, 멤버 `ortho_`, `maxDeviationMm()` 선언(정의 주석), `silhouette()` 주석 "치수 계산용 … 그리지 않는다", 화면용 선 주석 한 줄(§3.3-4), private 도움 함수 `fitSectionLine(raw, closed)`(sub 3)·`outlineNatural()`(sub 4) | §3.3·§3.4·§3.5 |
| `studio/src/ArtifactDoc.cpp` | 익명 이름공간 `segDist`·`lineDist`·`dev2`·`halfOrthoSilhouette`(§3.4-1 그대로: 면마다 걸치는 칸 k0…k1, x ≤ 0 교점 최소 x, 빈 칸 비움·평활 없음), `simplifyDP` 인덱스 출력(고르는 점 무변경, 닫힘 되감기 점 제외), `smoothCatmull` 삭제 → `fitCurve`(높이 범위·축 쪽 + 양방향 tol 조건, 실패 구간은 DP 직선), `recompute` 에 `ortho_ = halfOrthoSilhouette(mesh_, 0.25)`(치수용 MaxRadius 두 줄 그대로) · 확정 전 `ortho_.clear()`, 표시 4곳을 간격/2 의 `fitCurve` 로(외곽선 허용치 간격 → 간격/2, 원 선 `sil_.leftOutline` → `ortho_`), `rawPointCount` → `ortho_.size()`, `maxDeviationMm()`(미스캔 보정이면 실선·추정 내벽선 짝, 아니면 고리 짝(3점 미만 제외) + 외곽선 짝의 dev2 최댓값) | §3.1 (가)(나)·§3.3·§3.4·§3.5 |
| `studio/src/MainWindow.cpp` | `updateCutInfo` ②가 아닌 페이지: `점 %1 → <b>%2개</b> (%3 mm 간략화) · 최대 편차 %4 mm` + `.arg(doc_.maxDeviationMm(), 0, 'f', 2)` | §3.5 ③ 상황 줄 (원문 H 1665) |
| `docs/ui/UI_RULES.md` | R5 기준선 줄 다음에 「선 다듬기」 줄(§3.6 문장 그대로) | 정본 — 사람 검토 1 |
| `studio/tests/test_fit.cpp` (신규) | 수용 기준 [J8-1]~[J8-8]·[J8-23]·[J8-24]. 시험 쪽 독립 `segDist`·`lineDist`·`dev2`·`rawHalfOrtho`(면마다 칸)·`rawHalfOrthoAll`(전 칸 × 전 면). 6경우(예제 0°·12°·−20°·105°, 미스캔 0°·12°) + 간격 0.1·0.3·1.0, 곡선 끔/켬 점 수, DXF 띠·래스터(도판 칸 E 안만 셈), 기운 예제 5경우, 34.6만 면 예제. 파이썬용 `studio_fit_s/b.svg·.dxf·_raw.csv` 를 argv[1] 에 남김 | §3.7·§4 A |
| `studio/tests/test_cutpage.cpp` | `#include <QRegularExpression>`, [J7-13] 창 최소 폭 판정 뒤 ② `CtxRight` 에 「최대 편차」 없음, [J7-15] `CtxRight` 「점 」 판정 뒤 ③ 정규식·값 ≤ 0.25 판정·인쇄 | §4 기준 10 |
| `studio/CMakeLists.txt` | `test_studio_fit` / `add_test(NAME studio_fit …)` (reflines 다음) → CTest 18종 | §3.7 |

## 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| `powershell -NoProfile -ExecutionPolicy Bypass -File …\JOB-008\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-008 -BuildRoot D:\_dongarch_build\build_job008 -NoShots` (커밋 전) | 0 | configure 0 · build 0(25 s) · ctest 0(151 s) — 첫 실행에 18/18 통과 |
| 독립 파이썬 판정(명세 §5-3 스크립트, 인자 `D:/_dongarch_build/build_job008/studio`) | 0 | `PY-FIT OK` (출력 아래) |
| `git … commit` | 0 | `a913448` |
| 같은 verify.ps1 `-Clean`(캡처 포함, 커밋 뒤 — 깨끗한 빌드) | 0 | configure 0(4 s) · build 0(24 s, 76단계 전부) · ctest 0(151 s) · shots 0(9 s), `verify-summary.md` 마지막 `결과: PASS` |
| `grep -ci warning D:/_dongarch_build/build_job008/studio-logs/02_build.log` | — | **0** |
| `03_ctest.log` | — | `100% tests passed out of 18` (studio_fit 19.6 s) |
| 기준 12 (`build_job008\studio\Testing\Temporary\LastTest.log`) | — | `pts=82/1343` 줄 1 · `pts=85/1318` 0 · `STL(m->mm) … noiseRemoved=2` 1 · `ArtecBlind: H=190.30 rim=124.00 hasEst=1 estWall=6.95` 1 · `[J7-1] … gap low 0.125 high 0.196 mm` 1 |
| 기준 14 (`git diff --name-only 89846ba HEAD` 허용 목록 밖 · GigaMesh·Plate·기존 시험 5개) | — | 둘 다 출력 없음, 커밋 뒤 `git status --porcelain` 없음 |
| 기준 15 grep | — | TrueOrtho 0 · MaxRadius 1 · `halfOrthoSilhouette(mesh_, 0.25)` 1 · `sil_.leftOutline` 0 · `smoothCatmull`(studio/src) 없음 · `fitCurve(` 3 · h `maxDeviationMm` 1 · MainWindow `maxDeviationMm()` 1 · `· 최대 편차 %4 mm` 1 |
| 기준 16 (새 색 grep) | — | 출력 없음 |
| 기준 17 정본 grep | — | 1 · 1 · 1 |
| 기준 18 `shot_home.png` 해시 = JOB-007 | — | `True` |
| 실제 글꼴 캡처(§5-4 명령, 단 **내 빌드 폴더** `build_job008\studio\shots\` 에만 — A8 복사는 하지 않음) + §5-5 화소 비교(A8 대신 그 폴더) | — | main 431413 B · cut 434478 B · edit 406641 B, `inside 2593/2523/3971 · outside 0/0/0` → `SHOTS OK` |

### 시험 인쇄 (clean verify, LastTest.log — §3.8 예상값과 모두 같음)
```
[J8-6] empty=0.0 unconfirmed=0.0
[J8-1] sample 0° y=[0.127, 184.685] raw=[0.127, 184.685] H=184.821 gap 0.127/0.135
[J8-3] sample 0° ends=(-70.627,0.125)…(-71.033,184.625) gap 0.125/0.196 · |x|−base/2 -0.373 · |x|−rim/2 0.033
[J8-2] sample 0° tol=0.5 dev=0.231
[J8-4] sample 0° tol=0.5 n=739 dev=0.1801
[J8-6] sample 0° tol=0.5 maxDeviationMm=0.2309 = max(section 0.2309, outline 0.1801)
[J8-5] sample 0° n=739 maxdiff=0 · brute n=739 same=1
[J8-2] sample 0° tol=0.1 dev=0.049    [J8-4] … dev=0.0497   [J8-6] … 0.0497
[J8-2] sample 0° tol=0.3 dev=0.149    [J8-4] … dev=0.1426   [J8-6] … 0.1491
[J8-2] sample 0° tol=1.0 dev=0.458    [J8-4] … dev=0.4328   [J8-6] … 0.4583
[J8-7] smooth off: section 20 · outline 10 · points 30 | on: section 48 · outline 34 · points 82/1343
[J8-8] studio_fit_s.dxf dxf base y 0.000000 rim y 184.820570 · section 1 polyline y [0.126692, 184.685244] · outline y [0.125000, 184.625000] gap 0.125/0.196
[J8-8] raster E=[-90.7, 90.7]×[-8.0, 202.8] section above 0 below 0 top 193 bottom 196 · outline above 0 below 0 [0.3,1] 23 [H-1,H-0.3] 20
[J8-1] sample 12° y=[0.114, 184.690] … gap 0.114/0.130 · [J8-3] ends=(-71.003,0.125)…(-71.065,184.625) · [J8-2] dev=0.202 · [J8-4] dev=0.1914 · [J8-6] 0.2023 · [J8-5] n=739 maxdiff=0
[J8-1] sample −20° y=[0.153, 184.645] raw=[0.151, 184.711] gap 0.153/0.175 · [J8-3] ends=(-70.263,0.125)…(-71.061,184.625) · [J8-2] 0.187 · [J8-4] 0.1545 · [J8-6] 0.1868 · [J8-5] n=739 maxdiff=0
[J8-1] sample 105° y=[0.129, 184.641] gap 0.129/0.179 · [J8-3] ends=(-70.969,0.125)…(-71.058,184.625) · [J8-2] 0.176 · [J8-4] 0.1539 · [J8-6] 0.1757
[J8-1] blind 0° y=[0.190, 190.148] H=190.256 gap 0.190/0.107 · [J8-3] ends=(-62.078,0.125)…(-62.031,190.125) · [J8-2] 0.202 · [J8-4] n=761 0.1733 · [J8-6] 0.2022 · [J8-5] n=761 maxdiff=0
[J8-8] studio_fit_b.dxf dxf base y 0.000000 rim y 190.255610 · section 2 polyline y [0.189583, 190.148332] · outline y [0.125000, 190.125000] gap 0.125/0.131
[J8-1] blind 12° y=[0.091, 190.152] gap 0.091/0.103 · [J8-3] ends=(-62.010,0.125)…(-62.053,190.125) · [J8-2] 0.185 · [J8-4] 0.1845 · [J8-6] 0.1854
[J8-23] base 0.3° H=185.123 n=738 ends=(-49.203,0.625)…(-71.042,184.875) gap 0.625/0.248 maxX=-49.203 olDev=0.190 secDev=0.224 maxDev=0.224 (maxdiff 0)
[J8-23] base 0.5° H=185.365 n=738 ends=(-28.593,0.875)…(-71.042,185.125) gap 0.875/0.240 maxX=-28.593 olDev=0.161 secDev=0.224 maxDev=0.224 (maxdiff 0)
[J8-23] rim 0.5° H=185.349 n=739 ends=(-70.630,0.125)…(-12.018,184.625) gap 0.125/0.724 maxX=-12.018 olDev=0.239 secDev=0.231 maxDev=0.239 (maxdiff 0)
[J8-23] rim 1.0° H=185.951 n=739 ends=(-70.632,0.125)…(-6.047,184.625) gap 0.125/1.326 maxX=-6.047 olDev=0.180 secDev=0.231 maxDev=0.231 (maxdiff 0)
[J8-23] rim 1.0° front H=185.981 n=744 ends=(-70.626,0.125)…(-15.252,185.875) gap 0.125/0.106 maxX=-15.252 olDev=0.201 secDev=0.230 maxDev=0.230 (maxdiff 0)
[J8-24] faces=346320 lod=31103 n=739 ends=(-71.002,0.125)…(-71.027,184.625) gap 0.125/0.179 olDev=0.174 maxDev=0.222 maxdiff=0 · dims base=5.20 wall=3.85 (LOD 치수 — 판정 아님)
[J8-10] CtxRight '점 1,272 → <b>94개</b> (0.5 mm 간략화) · 최대 편차 0.21 mm'
```
### 파이썬 (§5-3)
```
s section y [0.127, 184.685] H 184.821 dev 0.231 | outline y [0.125, 184.625] dev(ortho) 0.180
b section y [0.190, 190.148] H 190.256 dev 0.202 | outline y [0.125, 190.125] dev(ortho) 0.173
svg rim y 47.090 base y 231.910 | section [47.225, 231.784] outline [47.285, 231.785]
PY-FIT OK
```
③ 실제 글꼴 캡처(edit) 상황 줄 오른쪽: `점 1,222 → 82개 (0.5 mm 간략화) · 최대 편차 0.20 mm`(굵게 82개). 홈 캡처 구연·바닥 모서리를 JOB-007 과 나란히 잘라 보니 단면 꼭대기가 구연선 위로·바닥이 바닥선 아래로 솟던 모양이 없어지고 구연 윗면·바닥 변이 곧은 꺾은선으로 그려짐(판정은 ui-reviewer 몫).

## 명세에서 벗어나 판단한 곳
1. **빌드 폴더** = 오케스트레이터 지시대로 `D:\_dongarch_build\build_job008`(명세 §5 의 `verify_job008` 아님). 오케스트레이터 표준 검증은 명세 경로로 다시 돌릴 것.
2. **도움 함수**: 표시 4곳과 `maxDeviationMm` 이 같은 허용치·sub 를 쓰도록 private `fitSectionLine(raw, closed)`(간격/2, sub 3)·`outlineNatural()`(`ortho_`, 간격/2, sub 4)로 묶음(§3.5 가 허용). 그래서 `fitCurve(` 는 정의 1 + 사용 2 = 3(기준 15 ≥ 2). `maxDeviationMm` 용 `dev2` 를 익명 이름공간에 하나 더 둠.
3. `simplifyDP(…, kept)` 는 시작에서 `kept->clear()` (명세에 없던 방어, 고르는 점·반환값 무변경).
4. 시험 인쇄 `<경우>` 이름은 영문 꼬리표(`sample 0°`, `blind 12°`, `base 0.3°`, `rim 1.0° front`, `hi`) — JOB-007 test_reflines 의 `sample` 표기와 맞춤.
5. 실제 글꼴 캡처(§5-4)·화소 비교(§5-5)·모서리 비교 그림(§5-6)은 **A8 에 복사하지 않았다**(builder 는 factory 기록 중 build-log 만 씀). 확인은 내 빌드 폴더 shots 로만 했으니 A8 산출물은 오케스트레이터가 만들 것.

## 남은 경고
- 빌드 경고 0. verify.ps1 실행 중 `'vswhere.exe' is not recognized` 줄이 두 번 찍히나(vcvars64 쪽 메시지) 종료 코드·빌드 로그와 무관 — 이전 작업과 같음.

## 제안 (범위 밖, 하지 않음)
- 명세 §10 의 후속(25만 면 초과 치수를 원본에서 — 이번 [J8-24] 에서도 LOD 치수 base 5.20·wall 3.85 로 틀림 재확인, 치수용 K-Arch 실루엣 호출 제거, 도판 마우스 판독 「외경」, 외곽선 돌기 거름, 기준선 폭 = 윤곽 끝점 등)을 그대로 지지.
- `maxDeviationMm()` 는 부를 때마다 `fitCurve` 를 다시 돈다(예제 ≈ 2 ms). 상황 줄 갱신 때만 불리니 지금은 충분하지만, 리본 「선 다듬기」 에 `편차 d mm` 를 더 붙일 때는 표시 선과 함께 캐시하는 편이 낫다.

## 커밋
- `a913448` feat(studio): 단면·외곽선을 기준선에 맞추기 — 편차 제한 곡선·막과 수직인 실루엣·최대 편차 (JOB-008)

## 차단 사유 (있으면)
- 없음.
