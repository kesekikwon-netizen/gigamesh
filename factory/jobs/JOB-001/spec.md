# JOB-001 명세 — DXF R12·`$INSUNITS=4`(mm)·11 레이어 자동 검증 테스트 보강

- 작성: 2026-10-17, spec-writer. 기준 코드: `factory/setup` 작업 트리(`D:/_dongarch_build/wt`), `job.json.base_sha` = `e7e641bd43716ccd57aad8246332d117d048a310`(AC-8 의 `<base_sha>`). 프로젝트 기준 SHA 는 `factory/project-profile.md` §기준 SHA 참조.
- 모든 줄 번호는 이 작업 트리에서 Grep 으로 확인한 값이다.

## 1. 목표

사용자 요청 원문(`factory/jobs/JOB-001/job.json` → `request`):

> "시연 작업: K-Arch DxfWriter 가 만드는 DXF 가 R12(AC1009)·$INSUNITS=4(mm)·11 레이어 규약을 지키는지 명세대로 검증하는 테스트를 보강한다. 도판 좌/우 배치(D5)와 무관한 작업으로 선정."

재진술: K-Arch `WritePlateDxf()`(`GigaMesh/gui/src/dongarch/karch/DxfWriter.cpp:263-333`)가 만든 DXF 를 **그룹코드/값 쌍 단위로 파싱**해, HEADER(`$ACADVER`=`AC1009`, `$INSUNITS`=4)·TABLES/LAYER(11 레이어의 이름·ACI 색·선종류)·ENTITIES(레이어 참조 무결성·mm 축척 수치)를 자동으로 검증하는 독립 실행 테스트 `test_dxf_units` 를 추가하고 CTest 에 `karch_test_dxf_units` 로 등록한다.

### 현재 상태(무엇이 이미 검사되고 무엇이 빠졌나)

기존 `GigaMesh/gui/src/dongarch/karch/test_dxf.cpp` 는 **부분 문자열 존재 여부**만 본다:

| 기존 검사 | 위치 | 한계 |
|---|---|---|
| 11 레이어 이름이 파일 어딘가에 있음 | `test_dxf.cpp:67-78` | LAYER 테이블에 있는지, 색(62)·선종류(6)가 맞는지, 중복·누락 개수는 미검사 |
| `AC1009`, `POLYLINE`, `VERTEX`, `SEQEND`, `CIRCLE`, 치수 문자열, `\n40\n50.000000\n` | `test_dxf.cpp:79-85` | 섹션 무관 부분 문자열. `AC1009` 가 HEADER 의 `$ACADVER` 값인지 보장 없음 |
| `$INSUNITS` 와 `\n70\n4\n` 가 각각 존재 | `test_dxf.cpp:90-93` | 두 토큰이 **연결되어 있는지** 미검사(`70\n4` 는 LAYER 플래그 등 다른 곳에서도 나올 수 있음) |
| `LWPOLYLINE`·한글·`100,0mm` 금지, 파편 ARC 의 `DASHED` | `test_dxf.cpp:86-89, 95-109` | 유지(이번 작업과 무관) |
| 산출 경로 | `test_dxf.cpp:34-42` | `D:\gigamesh\` 절대 경로를 먼저 시도하는 하드코딩 — 이번 테스트에서는 금지 |

## 2. 범위 밖 (이번에 하지 않는 것)

- **D5 도판 좌/우 배치**(`factory/project-profile.md` §결정 D5, `CLAUDE.md` §1 미결): 배치 코드를 바꾸지 않으며, 테스트도 좌표의 **부호**(외곽선이 x<0 쪽인지 등)를 규약으로 잠그지 않는다. 수치 검사는 절댓값·거리·반지름으로만 한다(AC-5).
- **레이어 의미(색·선종류) 통일**: 같은 11개 이름을 쓰면서 karch(`DxfWriter.cpp:269-279`)와 Studio(`studio/src/Plate.cpp:723-733`)의 ACI/선종류가 다르다(예: `01_AXIS_CENTER` ACI 1 vs 7, `04_SECTION_WALL` 4 vs 1, `06_INTERIOR_OUTLINE` 5/CONTINUOUS vs 1/DASHED, `07/08_PROJECTION_*` 6/CONTINUOUS vs 8/DASHED, `09_DIMENSION_MEASURE` 2 vs 3). 어느 쪽이 정본인지는 문서에 없음(미확인, `docs/ui/UI_RULES.md:82` 는 이름만 규정). 이번 테스트는 **karch `WritePlateDxf` 만** 대상으로 하고 `DxfWriter.cpp:269-279` 를 기대값으로 고정한다. 통일은 별도 작업.
- **Studio `exportDxf`(`studio/src/Plate.cpp:715-778`) 수정 및 그에 대한 테스트** — `studio_pipeline` 이 이미 `studio_plate.dxf` 를 쓰지만(`studio/tests/test_pipeline.cpp:97`) 내용 검증은 이번 범위 밖.
- **DXF 한글 TEXT / `$DWGCODEPAGE`**: R12 TEXT 는 ASCII 전용(`DxfWriter.h:52`). 변경 없음.
- **기존 `test_dxf.cpp` 수정**(하드코딩 경로 제거 포함) — 이유는 §5 참조. 별도 chore 후보.
- **`DxfWriter.cpp` 제품 코드 변경**: 새 테스트가 실패하면 builder 가 제품 코드를 고치지 말고 `needs-human` 으로 올린다(명세 오류인지 코드 버그인지는 사용자 판단).
- `GigaMesh/core`, `GigaMesh/external` 수정(절대 규칙), GigaMesh Qt5 GUI 쪽 변경.
- CI 워크플로 문구 갱신(`.github/workflows/studio-ci.yml:31,60,63` 의 "11 Suites"/"10 K-Arch Unit Tests")과 `studio/CMakeLists.txt:13` 옵션 설명의 "10" — 동작에 영향 없는 문구. CMake 옵션 설명 1줄은 **선택**(같은 파일을 어차피 고치므로), CI yml 은 건드리지 않는다.
- 외부 CAD(AutoCAD/LibreCAD)로 열어 보는 수동 확인, push·병합(사용자 승인 전 금지, `CLAUDE.md` §4-4).

## 3. 수용 기준 (각 항목은 명령 하나로 참/거짓이 갈린다)

공통 입력(기존 `test_dxf.cpp:50-61` 과 동일한 합성 메시·파이프라인):

```cpp
SyntheticParams sp; sp.radialSeg = 180; sp.heightSeg = 20; sp.capTop = true; sp.capBottom = true;
const auto cyl  = MakeCylinderShell(50.0, 100.0, 5.0, sp);   // SyntheticMeshGen.h:42, .cpp:170-183
const auto mesh = cyl.toMesh();
const auto sil  = ExtractSilhouette(mesh, SilhouetteMode::MaxRadius, 0.25);   // Silhouette.h:30
const auto sec  = ComposeSection(mesh);                                        // SectionComposer.h:34
const auto dim  = AnalyzeDimensions(mesh, sil);                                // DimensionAnalyzer.h:40
WritePlateDxf(path, sil.leftOutline, sec, dim);                                // DxfWriter.h:53
```

정답값(ground truth): 바깥 반지름 50 → **지름 100.0 mm**(`SyntheticMeshGen.cpp:170-176`), z ∈ [0, 100] mm(`SyntheticMeshGen.cpp:121-123`), 벽 두께 5 mm. 투영 원 중심은 rim `(0, zMax + r + 15) = (0, 165)`, base `(0, zMin − r − 15) = (0, −65)`(`DimensionAnalyzer.cpp:44-47, 149, 192`). 치수선은 `(−62, zMin)→(−62, zMax)`(`DxfWriter.cpp:306`), zMin/zMax 는 원시 메시 z(`DimensionAnalyzer.cpp:136-140, 215-216`). 작성기 좌표 대응: `Point2.x → 10`, `Point2.y(=도면 z) → 20`, `30 = 0`(`DxfWriter.cpp:156-158`), 실수는 소수 6자리(`DxfWriter.cpp:59`), 줄 끝 `\n`, 바이너리 모드(`DxfWriter.cpp:51-60, 63`).

아래 AC-2 ~ AC-5, AC-7 은 모두 `test_dxf_units` 실행 하나로 판정된다(실패 항목을 `FAIL <항목> <이유>` 로 stderr 에 찍고 종료 코드 ≠ 0). 판정 명령은 모두 빌드 폴더 `D:\_dongarch_build\verify\studio` 에서 실행한다(`cd` 후 ctest, `--test-dir` 금지 — `CLAUDE.md` §3).

### AC-1. CTest 등록·통과
- `ctest -N` 출력에 `karch_test_dxf_units` 가 있고 `Total Tests: 12`(기존 11 + 1)이다.
- `ctest -R karch_test_dxf_units --output-on-failure` 의 종료 코드가 0 이다.
- 전체 `ctest --output-on-failure` 가 12/12 통과(기존 테스트 회귀 없음).

### AC-2. HEADER 섹션 파싱(부분 문자열이 아니라 구조)
테스트는 파일을 `(그룹코드, 값)` 쌍의 순서열로 파싱한 뒤 다음을 **모두** 확인한다:
1. 첫 섹션은 `0/SECTION` + `2/HEADER` 이고, 섹션 순서는 `HEADER → TABLES → ENTITIES`, 마지막 쌍은 `0/EOF` 이다(`DxfWriter.cpp:81-82, 89-90, 144-145, 257-258`).
2. HEADER 섹션 **안에서** `9/$ACADVER` 바로 다음 쌍이 `1/AC1009` 이다(`DxfWriter.cpp:83-84`).
3. HEADER 섹션 **안에서** `9/$INSUNITS` 바로 다음 쌍이 `70/4` 이다(`DxfWriter.cpp:85-86`).
4. `$ACADVER` 의 위치 < `$INSUNITS` 의 위치 < HEADER 의 `0/ENDSEC` 위치(`DxfWriter.cpp:87`).

### AC-3. TABLES/LAYER 테이블 = 레이어 `0` + 정확히 11개, 이름·ACI·선종류 일치
`0/TABLE` + `2/LAYER` 아래의 `0/LAYER` 레코드를 모아, (a) 테이블 개수 값 `70` = 12 = 실제 레코드 수(`DxfWriter.cpp:128`), (b) 첫 레코드는 `0`(62=7, 6=CONTINUOUS, `DxfWriter.cpp:129-133`), (c) 이어지는 11개가 **이 순서·이 값**이며 중복 이름이 없고 각 레코드의 `70` 플래그는 0(`DxfWriter.cpp:134-140`)이다:

| # | 이름(2) | ACI(62) | 선종류(6) | 근거 |
|---|---|---|---|---|
| 1 | `01_AXIS_CENTER` | 1 | `CENTER` | `DxfWriter.cpp:269` |
| 2 | `02_EXTERIOR_OUTLINE` | 7 | `CONTINUOUS` | `:270` |
| 3 | `03_EXTERIOR_FEATURES` | 3 | `CONTINUOUS` | `:271` |
| 4 | `04_SECTION_WALL` | 4 | `CONTINUOUS` | `:272` |
| 5 | `05_SECTION_HATCH` | 8 | `CONTINUOUS` | `:273` |
| 6 | `06_INTERIOR_OUTLINE` | 5 | `CONTINUOUS` | `:274` |
| 7 | `07_PROJECTION_RIM` | 6 | `CONTINUOUS` | `:275` |
| 8 | `08_PROJECTION_BASE` | 6 | `CONTINUOUS` | `:276` |
| 9 | `09_DIMENSION_MEASURE` | 2 | `CONTINUOUS` | `:277` |
| 10 | `10_ANNOTATION_TEXT` | 7 | `CONTINUOUS` | `:278` |
| 11 | `11_SCALE_BAR` | 7 | `CONTINUOUS` | `:279` |

이 표는 테스트 소스에 상수 배열로 그대로 들어간다(의도된 회귀 잠금 — §6).

### AC-4. 엔티티 참조 무결성(미정의 레이어·선종류 사용 없음, R12 엔티티만)
ENTITIES 섹션의 모든 엔티티 레코드(`0/<TYPE>` 로 시작, `VERTEX`·`SEQEND` 포함 — `DxfWriter.cpp:177-184`)에 대해:
1. 그룹코드 `8` 값이 위 11개 이름 중 하나다(레이어 `0` 도 **불허** — `WritePlateDxf` 는 `0` 에 그리지 않는다).
2. 그룹코드 `6` 이 있으면 값이 `CONTINUOUS`/`CENTER`/`DASHED` 중 하나다(LTYPE 테이블 `DxfWriter.cpp:92-124`); LAYER 레코드의 `6` 도 같은 집합에 속한다.
3. 엔티티 타입 집합 ⊆ {`LINE`, `POLYLINE`, `VERTEX`, `SEQEND`, `CIRCLE`, `ARC`, `TEXT`, `SOLID`}(`DxfWriter.cpp:151, 169, 177, 183, 192, 209, 225, 236`). `LWPOLYLINE`·`HATCH`·`MTEXT` 등 R12 이후 타입이 하나라도 있으면 실패.
4. 레이어 `01`, `02`, `07`, `08`, `09`, `10`, `11` 에는 엔티티가 1개 이상 있다(`WritePlateDxf` 가 `dims.ok` 이고 두 원 반지름 > 0 이면 무조건 그리는 경로: `DxfWriter.cpp:283-292, 303-315, 321-331`). `03`(`WritePlateDxf` 가 쓰지 않음), `04`/`05`/`06`(SectionComposer 결과에 의존, `:293-301`)은 **존재를 요구하지 않는다**.

### AC-5. mm 축척 수치(합성 원통 지름 100 mm 가 DXF 에 반영)
단위 mm, 허용 오차 명시:
1. 레이어 `07_PROJECTION_RIM` 의 `CIRCLE` 이 정확히 1개이고 `40`(반지름) = **50.0 ± 0.1 mm**(지름 100 ± 0.2), `10` = 0 ± 0.1, `20` = 165.0 ± 0.5.
2. 레이어 `08_PROJECTION_BASE` 의 `CIRCLE` 이 정확히 1개이고 `40` = 50.0 ± 0.1, `20` = −65.0 ± 0.5.
3. 레이어 `09_DIMENSION_MEASURE` 의 `LINE` 에서 `|21 − 20|` = **100.0 ± 0.1 mm**(기고), `|11 − 10|` ≤ 0.001(수직선).
4. 레이어 `02_EXTERIOR_OUTLINE` 의 `POLYLINE` 정점(`VERTEX`)에 대해 `max|10|` ∈ [49.5, 50.5] mm, 모든 `20` ∈ [−0.5, 100.5], `20` 의 범위(max−min) ≥ 99.0 mm(부호를 보지 않으므로 D5 중립. z-bin 0.25 중심값이라 범위는 약 99.75 로 추정 — 미확인, `Silhouette.cpp:128, 146`).
단위가 cm(×0.1)나 m(×0.001)로 바뀌면 1~4 모두 실패한다.

### AC-6. 산출 경로 = CTest 작업 폴더(절대 경로 하드코딩 금지)
- 테스트는 DXF 를 `argv[1]` 이 주어지면 그 디렉터리에, 아니면 **상대 경로**(cwd)로 `karch_t8u_cyl.dxf` 에 쓴다(`studio/tests/test_pipeline.cpp:32` 와 같은 관례; 이름을 `karch_t*` 로 맞춰 `.gitignore:83-84` 가 덮게 한다).
- 판정: `grep -nE "[A-Za-z]:[\\\\/]|gigamesh" GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp` 가 아무것도 찾지 못한다(종료 코드 1). 그리고 ctest 실행 뒤 `D:\_dongarch_build\verify\studio\karch_t8u_cyl.dxf` 가 존재하고, 작업 트리 루트(`.worktrees/JOB-001/`)에는 `karch_t8u_*.dxf` 가 없다.

### AC-7. 음성 대조(파서가 실제로 위반을 잡는지 증명)
테스트는 슬러프한 DXF 문자열에 아래 변이를 **각각 하나씩** 적용한 사본 4개를 같은 검사 함수에 넣고, **4개 모두 실패로 판정**될 때만 통과한다(어느 하나라도 통과하면 `FAIL mutation <n>` 으로 종료 ≠ 0):
1. `"\n9\n$INSUNITS\n70\n4\n"` → `70\n1`(인치) — AC-2 위반.
2. `"\n1\nAC1009\n"` → `AC1015` — AC-2 위반.
3. `"\n2\n01_AXIS_CENTER\n70\n0\n62\n1\n"` → `62\n2` — AC-3 위반(ACI 변경).
4. ENTITIES 의 첫 `"\n8\n02_EXTERIOR_OUTLINE\n"` → `8\n99_UNKNOWN` — AC-4 위반(LAYER 테이블의 이름은 `2\n` 뒤라 치환되지 않음).
각 치환 전에 원문에 해당 패턴이 정확히 존재함을 먼저 확인한다(패턴이 없으면 그 자체가 실패).

### AC-8. 변경 범위
`git diff --name-only <base_sha>..HEAD`(worktree `.worktrees/JOB-001`, 브랜치 `factory/JOB-001`)의 결과가 정확히 다음 2개다:
`GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp`, `studio/CMakeLists.txt`. (`GigaMesh/core`, `GigaMesh/external`, `DxfWriter.*`, `test_dxf.cpp`, `studio/src/*` 변경 없음.)

## 4. 검증 명령

```powershell
# 표준 절차: configure → build → ctest(12) → 캡처, 로그·요약을 factory/jobs/JOB-001/artifacts/ 로 복사 (verify.ps1:44-58, 80-87)
powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Job JOB-001
# 기대: verify-summary-studio.md 의 결과 PASS, 03_ctest.log 에 "12 tests passed" 와 "karch_test_dxf_units ... Passed"
```

개별 판정(빌드 폴더로 `cd` 한 뒤):

```bat
cd /d D:\_dongarch_build\verify\studio
ctest -N                                                     :: AC-1: karch_test_dxf_units 있음, Total Tests: 12
ctest -R karch_test_dxf_units --output-on-failure            :: AC-1~5, AC-7: 종료 0
ctest --output-on-failure                                    :: AC-1: 12/12
dir karch_t8u_cyl.dxf                                        :: AC-6: 빌드 폴더에 생성됨
```

```bash
# AC-6 정적 검사 (작업 트리에서, Git Bash)
grep -nE "[A-Za-z]:[\\\\/]|gigamesh" GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp ; echo "exit=$?"   # exit=1 이어야 통과
ls karch_t8u_*.dxf 2>/dev/null | wc -l                                                                    # 0
# AC-8
git diff --name-only <base_sha>..HEAD
```

- 수동 실행 예: `test_dxf_units.exe D:\_dongarch_build\verify\studio` (인자 = 출력 폴더).
- CI: `.github/workflows/studio-ci.yml:5-8` 이 `factory/**` 브랜치의 `GigaMesh/gui/src/dongarch/karch/**`·`studio/**` 변경에 반응해 `ctest --output-on-failure --parallel 4`(`:63-65`)를 돌리므로, **push 가 허용된 뒤에는** 추가 설정 없이 새 테스트가 CI 에 포함된다. push 자체는 사용자 지시 전 금지.

## 5. 영향 파일

| 파일 | 변경 | 이유 |
|---|---|---|
| `GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp` | **신규**(약 230~280줄) | karch 테스트 관례: 10개 `test_*.cpp` 가 모두 karch 폴더에 있고 CMake 가 `${KARCH_DIR}/${t}.cpp` 로 찾는다(`studio/CMakeLists.txt:46, 104-108`). `karch` 만 링크(Qt 무의존, `:106`). `studio/tests/` 는 `QGuiApplication` 기반 파이프라인 테스트 자리(`test_pipeline.cpp:7, 31`)라 부적합 |
| `studio/CMakeLists.txt` | `KARCH_TESTS` 목록(`:93-103`)에 `test_dxf_units` **1줄 추가**(`test_dxf` 다음). 선택: `:13` 옵션 설명 "10 standalone" → "11" | `foreach`(`:104-108`)가 `add_executable`·`target_link_libraries(karch)`·`add_test(NAME karch_${t})` 를 자동 생성하므로 1줄로 등록 완료 |
| `GigaMesh/gui/src/dongarch/karch/test_dxf.cpp` | **불변** | (1) 이미 통과 중인 T8 회귀(파편 ARC `DASHED`, 금지 토큰)를 건드리면 리뷰 범위가 섞인다. (2) 구조 파싱은 새 파일에 두는 편이 "무엇이 추가됐나"가 diff 로 명확하다. (3) `:34-42` 의 `D:\gigamesh` 폴백 제거는 별도 chore 로 등록(backlog 후보) |
| `DxfWriter.h/.cpp`, `studio/src/Plate.cpp`, `GigaMesh/core/**`, `GigaMesh/external/**`, `.github/workflows/*.yml` | **불변** | 검증 테스트 보강만. 제품 코드 수정 금지(§2) |

### 테스트 파일 내부 구성 제안(구속력 없음, builder 참고)

1. `parsePairs(const std::string&) → std::vector<Pair{int code; std::string value; size_t line;}>` — `\n` 분리, `\r` 제거, 코드 줄 `std::stoi`(공백 트림), 홀수 줄 수면 실패.
2. `splitSections(pairs)` — `0/SECTION` + `2/<name>` … `0/ENDSEC` 구간 인덱스. `0/EOF` 확인.
3. `readLayerTable(tables)` — `0/TABLE`+`2/LAYER` 안의 `0/LAYER` 레코드(다음 코드 0 전까지의 쌍)를 `{name(2), flags(70), aci(62), lt(6)}` 로.
4. `readEntities(entities)` — `0/<TYPE>` 마다 `{type, layer(8), lt(6, 선택), 10/20/11/21/40 값}`.
5. `check(const std::string& dxf, std::string* why) → bool` — AC-2~AC-5 를 순서대로. 실수 파싱은 작성기와 대칭으로 `std::istringstream` + `imbue(std::locale::classic())`(`DxfWriter.cpp:15, 67`).
6. `main(argc, argv)` — 출력 폴더 결정(AC-6) → 메시 생성·`WritePlateDxf` → 슬러프 → `check` 통과 필수 → AC-7 변이 4종 `check` 실패 필수 → 요약 1줄 출력, 0 반환. 기대 레이어 표(AC-3)는 `constexpr`/`static const` 배열.

## 6. 위험·가정

- **의도된 회귀 잠금**: `DxfWriter.cpp:269-279` 의 ACI/선종류가 바뀌면 AC-3 이 깨진다. 이것이 목적이다. 바꿀 때는 테스트 표도 같이 고치고 이유를 커밋 본문에 쓴다. ACI 색의 문서 정본은 없음(미확인) — 코드가 유일한 기준이므로 테스트가 그 기준을 고정한다.
- **Studio 와의 불일치는 이번에 드러나기만 한다**: `Plate.cpp:723-733` 은 같은 이름에 다른 색/선종류를 쓴다(§2). 이 테스트는 karch 만 보므로 Studio DXF 는 여전히 미검증. 후속 작업에서 같은 파서를 `studio_pipeline` 산출물(`studio_plate.dxf`)에 재사용할 수 있게 `check()` 를 레이어 기대표를 인자로 받는 형태로 짜 두면 좋다(선택).
- **파서 단순화 가정**: `DxfWriter` 는 항상 `코드\n값\n` 쌍·LF·바이너리 모드로 쓴다(`DxfWriter.cpp:51-60, 63`). 파서는 일반 DXF 리더가 아니며 이 작성기 출력만 대상으로 한다. `\r` 은 방어적으로 제거.
- **AC-5 는 DXF 코드 밖(Silhouette/SectionComposer/DimensionAnalyzer)에도 의존**: 그쪽 알고리즘이 바뀌면 DXF 코드 변경 없이도 실패할 수 있다. 완화: 치수선(`09`)은 원시 zMin/zMax 를 그대로 쓰고(`DimensionAnalyzer.cpp:215-216`), 투영 원 반지름은 정확한 링 점의 Taubin 피팅이라 50.000000 으로 나옴(기존 `test_dxf.cpp:82` 가 그 리터럴을 이미 단언). 외곽선 항목(5-4)만 ±0.5 mm 로 느슨하게.
- **`04`/`05`/`06` 엔티티 존재 미확인**: 원통에서 `wallPolygons`·`hatchLines` 가 생길 것으로 추정하나 확인하지 않았으므로 AC-4-4 에서 요구하지 않는다. `03_EXTERIOR_FEATURES` 는 `WritePlateDxf` 가 쓰지 않는 레이어이므로 "모든 레이어에 엔티티가 있어야 한다"는 식의 기준을 넣으면 안 된다.
- **좌표계**: 내부 mm·Z-up 규약(`CLAUDE.md` §2)에서 도면 `(x, z)` 가 DXF `(10, 20)` 으로 간다. 테스트는 DXF 의 Y 가 유물 높이 방향이라는 전제로 `20` 을 높이로 읽는다. D5 와 무관하도록 x 는 절댓값으로만 본다.
- **작업 폴더**: ctest 기본 작업 폴더 = 빌드 폴더(`verify.ps1:52` 가 `cd /d $bld && ctest`, CI 는 `working-directory: studio/build-ci`, `studio-ci.yml:64`). 산출 DXF 는 빌드 폴더에 남는다(저장소 밖, `CLAUDE.md` §3). 수동으로 저장소 루트에서 실행해도 `.gitignore:83-84`(`/karch_t*.dxf`)가 덮는다.
- **문구 불일치(치명 아님)**: 테스트 수가 11→12 가 되어 `studio/CMakeLists.txt:13` 의 "10 standalone", `studio-ci.yml:31,60,63` 의 "11 Suites"/"10 K-Arch" 가 낡는다. CMake 옵션 설명만 선택적으로 고치고 CI 는 두되 `build-log.md` 에 메모.
- **MSVC `/W4 /permissive-`(`studio/CMakeLists.txt:16`)**: `size_t`↔`int` 변환·미사용 변수 경고가 쉽게 난다. 새 파일에서 새 경고 0 을 목표로 하되 수용 기준에는 넣지 않는다(기존 테스트들의 경고 상태 미확인).
- **라이선스/UI**: 테스트 전용 코드(GPLv3+ 헤더 주석 관례 `/* DongArch3D / K-ArchProfile ... (GPLv3+) */`, `test_dxf.cpp:1`), UI 문자열 없음 → UI_RULES 해당 없음(`review-ui.md` 는 `na` 예상).
- **작업 트리**: `.worktrees/JOB-001/` 이 이미 존재함(이 작성 시점에 `studio/CMakeLists.txt` 가 원본과 동일). builder 는 그 안에서만 수정하고 메인 체크아웃 `D:\gigamesh` 는 건드리지 않는다(`CLAUDE.md` §4-8).

## 7. 크기

예상 변경 줄 수: `test_dxf_units.cpp` 약 230~280줄(파서 ~90, 검사 ~110, 변이·main ~50) + `studio/CMakeLists.txt` 1~2줄 → **합계 ≤ 300줄**. 쪼갤 필요 없음. 300줄을 넘길 것 같으면 AC-5-4(외곽선 범위)와 `check()` 일반화(§6 선택 항목)를 먼저 뺀다.
