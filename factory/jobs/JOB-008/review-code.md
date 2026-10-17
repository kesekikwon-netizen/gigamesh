결론: PASS

# JOB-008 코드 품질 검토 (code-reviewer)

- 검토 대상: `git -C D:\_dongarch_build\wt\.worktrees\JOB-008 diff 89846ba..HEAD`(HEAD `a913448`, 커밋 1개, 파일 7개 +833/−43). 기준은 명세 `factory/jobs/JOB-008/spec.md` §0·§3·§4·§8·§11, 그리고 `CLAUDE.md`·`docs/ui/UI_RULES.md` 규칙이다.
- 방법: diff 를 처음부터 끝까지 직접 읽었다. 함께 읽은 것은 `ArtifactDoc.cpp/.h` 의 바뀐 함수 주변(simplifyDP·fitCurve·recompute·sectionNatural·maxDeviationMm), `test_fit.cpp` 전체(613줄), `test_cutpage.cpp` 의 바뀐 곳이다. build-log 의 주장은 diff 로 하나씩 확인했다. 빌드와 시험 실행은 하지 않았다(검토자 역할 밖). 실행 결과는 test-runner 의 `verify.md` 와 `artifacts/03_ctest.log`·`LastTest_verify.log` 로만 대조했다.
- 작업 트리는 깨끗하다(`git status --porcelain` 빈 출력). 바뀐 제품 코드는 빈 줄·주석을 빼고 +171/−41줄이다(검토자가 diff 로 직접 셈). 300줄 지침 안이므로 §0 에 따라 나누지 않는 것이 맞다.

## 항목별 판정

| # | 항목 | 판정 | 근거 (file:line) |
|---|---|---|---|
| 1 | 명세 범위 | **pass** | 바뀐 파일 7개가 명세 §6 허용 목록(기준 14)과 정확히 같다: `ArtifactDoc.h/.cpp`·`MainWindow.cpp`·`test_fit.cpp`(신규)·`test_cutpage.cpp`·`CMakeLists.txt`·`UI_RULES.md`. 제품 변경은 모두 §3 설계 결정과 1:1 로 대응한다. ① `simplifyDP` 에 `kept` 출력 추가(ArtifactDoc.cpp:254-297, 고르는 점은 그대로). ② `smoothCatmull` 을 지우고 `fitCurve` 를 넣음(:299-350). ③ `halfOrthoSilhouette`(:183-225). ④ recompute 에 `ortho_ = halfOrthoSilhouette(mesh_, 0.25)`(:798), 확정 전 분기에 `ortho_.clear()`(:772), 치수용 MaxRadius 두 줄은 그대로(:793-794). ⑤ 표시 4곳을 간격/2 로(:901·905). ⑥ `rawPointCount` 가 `ortho_.size()` 를 씀(:1230). ⑦ `maxDeviationMm`(:1206-1220). ⑧ ③ 상황 줄(MainWindow.cpp:664-668). ⑨ 정본 한 줄(UI_RULES.md:75). 범위 밖으로 적힌 것(Plate·SheetView·K-Arch·기존 시험 5개·치수용 호출)은 건드리지 않았다. 군더더기는 h:215 `lines_` 주석 정렬 공백이 바뀐 것 하나뿐이다(동작 무관) |
| 2 | 금지 구역 | **pass** | `git diff --name-only 89846ba HEAD -- GigaMesh` 의 출력이 없다. `GigaMesh/core`·`external`·K-Arch(`gui/src/dongarch/karch`)는 모두 무변경이다. 반쪽 정사 실루엣은 K-Arch `addTriZHits` 의 교점 규칙을 Studio 안(ArtifactDoc.cpp:183-225)에 다시 구현한 것으로, 규칙 4-1 이 말하는 「코어 밖 재구현」이다. karch 를 고치지 않았으므로 허용 범위 판단도 필요 없다 |
| 3 | 규약 (한국어·약어·Theme·mm/Z-up) | **pass** | 새 UI 문자열은 `· 최대 편차 %4 mm` 하나다(MainWindow.cpp:664). 원문 `H` 1665 의 글자와 같은 한국어이고 약어가 없다. 새 색은 없다(diff 의 `+` 줄에 `QColor`·`qRgb`·`#hex`·`Qt::색` 0건). 단위·좌표 주석은 일관된다: 「높이 칸 zk」, 「x ≤ 0」, 「원 선의 높이(y) 범위」, 「(mm)」(h:182-186·cpp:179-182). 부호 처리(D5·R)는 자연 좌표에서 맞춘 뒤 마지막에 `signedX` 를 곱하는 기존 방식 그대로다(cpp:921-924) |
| 4 | 구조 (중복·매직 넘버·길이·에러 처리) | **pass** | **중복**: 허용치와 sub 를 private `fitSectionLine`(sub 3)·`outlineNatural`(sub 4) 두 곳으로 묶었다(cpp:898-906). 표시 4곳과 `maxDeviationMm` 이 같은 함수를 쓰므로 표시 선과 편차 계산이 어긋날 수 없다(§3.5 가 허용). 시험 쪽 `segDist`·`lineDist`·`dev2`·`rawHalfOrtho` 가 제품 쪽과 겹치는 것은 명세 §3.7 이 「독립 계산」 을 요구해서 생긴 의도된 중복이다. **매직 넘버**: 「간격/2」 는 두 도움 함수의 `simplifyTol_ * 0.5` 에만 있다. 칸 0.25 는 기준 15 가 `halfOrthoSilhouette(mesh_, 0.25)` 글자를 grep 으로 요구하므로 리터럴이어야 한다. 1e-12·1e-15 는 K-Arch 와 같은 교점 판정 ε 이다(명세 §3.4-1 값 그대로). 이 작업에는 자르기·연장이 없다(명세 §3.2 가 클리핑·끌어 붙이기를 버림). 그래서 이름 붙일 자르기·연장 허용 오차도 없다. 시험 쪽은 `kBin`·`kPpm` 처럼 이름 있는 상수를 쓴다. **길이**: `fitCurve` 약 52줄, `halfOrthoSilhouette` 약 42줄, `maxDeviationMm` 15줄로 적당하다. **에러 처리**: 빈 메시·`bin <= 0`·음수 면 인덱스를 건너뛴다(cpp:185-195). 빈 원 선이면 `lineDist` 가 0 을 돌려준다(:160-161). 미로드·확정 전에는 `ortho_`·`lines_` 가 비어 편차가 0 이다(시험 [J8-6] 이 확인). `n = 0` 이면 `k1 = −1` 이라 빈 `best` 를 읽지 않는다. 닫힌 DP 는 늘 `keep.front()=1` 이라 `kept[0]=0` 이므로, 마지막 구간을 처음 점으로 되감는 `raw[k % n]` 이 올바르다 |
| 5 | 테스트 | **pass** | 새 시험 `studio_fit` 가 CTest 에 등록됐다(CMakeLists.txt:104-107). 출력 폴더는 `${CMAKE_CURRENT_BINARY_DIR}` = argv[1] 이고, 시험이 `D:\…` 같은 절대 경로에 쓰지 않는다(test_fit.cpp:446-447). 수용 기준과 시험이 1:1 인지는 아래 표에 있다. 기존 SVG·DXF·치수 시험은 고치지 않았고(기준 14), 독립 검증에서 18/18 통과했다(`03_ctest.log` `100% tests passed out of 18`). `karch_test_dxf`·`dxf_units`·`silhouette`·`dimensions` 와 `studio_pipeline`·`reflines` 도 통과했다. LastTest 에서 바뀐 값은 `pts=82/1343` 하나이고 치수·STL·ArtecBlind 줄은 그대로다 |
| 6 | 문서 | **pass** | 사용자에게 보이는 동작 두 가지(③ 상황 줄 「최대 편차」, 선 다듬기·외곽선 정의)가 정본 UI_RULES R5 「선 다듬기」 줄(UI_RULES.md:75)에 반영됐다. 문구는 명세 §3.6 과 글자까지 같다. studio 에는 README 가 없다. 저장소 밖 보조 문서 하나가 낡았는데, 아래 제안 5에 적었다(허용 목록 밖이라 이번에는 못 고침) |

## 수용 기준 ↔ 시험 1:1 대조

| 기준 | 시험 위치 | 대조 |
|---|---|---|
| [J8-1] | test_fit.cpp:197-205(`checkBand`), :171-194(`sectionBand`) | 6경우 모두 있다. 판정 4개가 명세와 같다: `|bboxMin.z| ≤ 1e-9`, 원 선 범위 ⊂ [−1e-9, H+1e-9], 그린 점이 원 범위 ±1e-9 안이고 `x ≥ −1e-9`, 틈 ≤ 0.30. 미스캔 경우는 원 선을 solids+estInner 로 잡는다(명세 §3.7 과 같다) |
| [J8-2] | :229-235, :472-476 | 6경우와 간격 0.1·0.3·1.0 을 돌린 뒤 0.5 로 되돌린다. 짝 수가 같은지와 `dev2 ≤ tol/2 + 1e-9` 를 본다 |
| [J8-3] | :207-225 | 판정은 y 띠 ±1e-9, `x ≤ 1e-9`, front=최소·back=최대, 틈 ≤ 0.25 이다 |
| [J8-4] | :237-244 | 원 선이 `x ≤ 0`·z 범위 안·y 순증가인지, `dev2 ≤ tol/2 + 1e-9` 인지 본다 |
| [J8-5] | :253-258, :465-471, :539·543·554 | 예제 0°·12°·−20°와 미스캔 0°에서 제품 결과와 `rawHalfOrtho` 가 ≤ 1e-9 로 같은지 본다. 0° 에서는 전수 구현과 `==` 로도 비교한다. 시험 쪽 구현은 제품 함수를 부르지 않는다 |
| [J8-6] | :246-249, :451-458 | `|md − max(sd, od)| ≤ 1e-9` 와 `≤ tol/2` 를 본다. 빈 문서와 확정 전 문서에서는 `== 0.0` 이어야 한다 |
| [J8-7] | :479-497 | 곡선을 끄면 단면·외곽선이 `simplifyDP(…, 0.25)` 결과와 꼭짓점마다 `==` 이고 30점이다. 켜면 48·34·82점, 원 점은 1343 이다 |
| [J8-8] | :287-330(DXF), :339-371·512-533(래스터, 도판 칸 E 안만 셈), :606-608(파일 6개) | 명세 (a)(b)(c) 와 같다. 비평 반영 #2 의 「표제란 제외」 도 들어 있다 |
| [J8-9] | 명세 §5-3 파이썬(저장소 밖) | 시험이 CSV 를 쓴다(:374-391). 머리 `kind,idx,x,y`, 값 `%.9f`, ortho 는 시험 쪽 독립 계산 값이다 |
| [J8-10] | test_cutpage.cpp:439(② 에 「최대 편차」 없음), :470-476(③ 정규식·값 ≤ 0.25) | 위치가 명세와 같다([J7-13] 다음, [J7-15] 「점 」 다음). 정규식도 글자까지 같다. JOB-006·007 판정 줄은 지우거나 고치지 않았다(+9/−0) |
| [J8-23] | :394-440 | 기울이는 식(`s = x`, 앞쪽만 `−y`, 아래·위 30 mm, H = 복사본 bboxMax.z)과 5경우가 명세와 같다. (a)~(e) 를 모두 판정한다 |
| [J8-24] | :569-603 | 합성 매개변수가 §3.7 과 같다. 판정은 `faces > 250000`·`lod < faces`, 원본 실루엣과 같음, `x ≤ 1e-9`, 틈, 편차다. 틈은 bboxMin.z 를 빼서 잰다. 확정 뒤 Z=0 이므로 명세 식과 같은 판정이다(제안 7) |

## builder 가 명세에서 벗어난 곳 (build-log 「명세에서 벗어나 판단한 곳」 을 diff 로 판정)

| # | 내용 | 판정 |
|---|---|---|
| 1 | 빌드 폴더 `build_job008` | 제품·시험 코드와 관계없다. test-runner 가 명세 경로 `verify_job008` 에서 `-Clean` 으로 다시 검증해 PASS 다 |
| 2 | 도움 함수 `fitSectionLine`·`outlineNatural`, 제품 쪽 `dev2` | 명세 §3.5 가 허용한다. 기준 15 `fitCurve(` 는 3 이라 ≥ 2 를 만족한다. 표시와 편차가 같은 함수를 지나므로 품질은 오히려 좋아졌다 |
| 3 | `simplifyDP` 시작에서 `kept->clear()` | 방어 코드다. 고르는 점과 반환값은 그대로다. [J8-7] 의 `sameLine(…, simplifyDP(…))` 가 이를 확인한다 |
| 4 | 시험 인쇄의 경우 이름이 영문(`sample`·`blind`·`base`·`rim`·`hi`) | UI 문자열이 아니고 시험 로그다. JOB-007 test_reflines 표기와 같아 문제없다 |
| 5 | A8 산출물을 builder 가 복사하지 않음 | 기록 규칙(builder 는 build-log 만 씀)에 맞는다. test-runner 가 만들었다 |

## 고치지 않아도 되는 제안

1. **「간격/2」 한 곳에 두기**: `simplifyTol_ * 0.5` 가 ArtifactDoc.cpp:901·905 두 곳에 있다. `drawTolMm()` 같은 접근자나 `kDrawTolFrac` 상수 하나로 모으면 「허용치 = 간격/2」 가 한 곳에서 정해진다. 리본에 `편차 d mm` 를 붙일 때(명세 §10) 쓰기도 쉬워진다.
2. **교점 ε 에 이름 붙이기**: `1e-12`·`1e-15`(cpp:207·210)를 익명 이름공간 `constexpr`(예: `kOnPlaneEps`·`kFlatEdgeEps`)로 두면 K-Arch 와 같은 값이라는 뜻이 드러난다. 칸 크기 0.25 는 기준 15 grep 때문에 호출 자리에서는 리터럴로 두어야 한다.
3. **낡은 주석**: ArtifactDoc.h:206 `자연 좌표(K-Arch: 단면 x ≥ 0, 외곽 x < 0)` 는 이제 맞지 않는다. 외곽선은 Studio 의 반쪽 정사 실루엣이고 범위도 x ≤ 0 이다. 다음 작업 때 고치면 된다.
4. **공백 군더더기**: h:215 `SectionLines lines_;      //` 의 주석 정렬이 이웃 줄과 어긋났다(동작 무관).
5. **저장소 보조 문서**: `.agents/skills/dongarch3d-studio/SKILL.md:41` 이 지워진 `smoothCatmull` 을 아직 가리킨다. 이번 허용 파일 밖이라 후속 문서 작업으로 고치는 편이 좋다.
6. **`maxDeviationMm()` 캐시**: 부를 때마다 `fitCurve` 를 다시 돈다(약 2 ms). 지금은 상황 줄 갱신 때만 불려 충분하다. 리본 「선 다듬기」 에 편차를 더 보이게 되면 표시 선과 함께 캐시하는 편이 낫다(build-log 제안과 같음).
7. **[J8-24] 틈 판정 식**: test_fit.cpp:595 는 `front().y − bboxMin.z` 로 잰다. 명세는 `front().y ≤ 0.25` 다. 확정 뒤 Z=0 이라 결과는 같지만, 이 경우에도 `|bboxMin.z| ≤ 1e-9` 를 함께 판정하면 명세 식과 완전히 같아진다.
8. **미스캔 분기 조건**: `maxDeviationMm` 의 미스캔 조건(cpp:1209)은 `sectionNatural` 의 바깥 조건(:857)과 같다. 다만 안쪽 되돌림(그린 실선·추정선이 비면 고리로 돌아가는 경로, :860)은 따라 하지 않는다. DP 가 양 끝점을 늘 남기므로 원 선이 2점 이상이면 이 경로에 갈 수 없다. 그래서 지금은 무해하다.

## 확인 필요 (확신 없는 지적)
- 없음. 코드 동작에 대한 의문은 diff 와 시험(독립 계산 `maxdiff=0`, 1:1 판정)으로 모두 풀렸다.
