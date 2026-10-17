# JOB-001 구현 기록 (builder)

- 작성: 2026-10-17, builder. 작업 트리 `.worktrees/JOB-001`, 브랜치 `factory/JOB-001`, 기준 커밋 `e7e641b`.
- 명세: `factory/jobs/JOB-001/spec.md` (AC-1 ~ AC-8).

## 바꾼 파일
| 파일 | 내용 | 이유 |
|---|---|---|
| `GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp` | **신규**, 589줄. `WritePlateDxf()` 산출 DXF 를 `(그룹코드, 값)` 쌍 순서열로 파싱하는 소형 파서(`parsePairs` → `splitSections` → `records`)와 검사 함수 `checkDxf()` (AC-2 HEADER, AC-3 LAYER 표, AC-4 엔티티 참조 무결성, AC-5 mm 축척), `main()` 에서 합성 원통(r=50, h=100, t=5, radialSeg 180, heightSeg 20) → Silhouette/Section/Dimensions → DXF 작성 → 검사 → AC-7 변이 4종 거부 확인. 산출물 `karch_t8u_cyl.dxf` 는 `argv[1]` 폴더 또는 cwd 상대 경로. 기대 레이어 표(레이어 0 + 11개 이름·ACI·선종류)는 `constexpr` 배열 `kLayers` 로 고정(회귀 잠금) | 기존 `test_dxf.cpp` 는 부분 문자열만 보고 `$INSUNITS`-`70/4` 연결·LAYER 표의 색/선종류·엔티티 레이어 참조·수치를 검사하지 않음(명세 §1). 새 파일로 둔 이유는 명세 §5(기존 T8 회귀와 diff 분리) |
| `studio/CMakeLists.txt` | `KARCH_TESTS` 목록에 `test_dxf_units` 1줄 추가(`test_dxf` 다음). 13행 옵션 설명 "10 standalone" → "11" | 기존 `foreach` 가 `add_executable`·`karch` 링크·`add_test(NAME karch_test_dxf_units)` 를 자동 생성(명세 §5). 옵션 설명 갱신은 명세가 허용한 선택 항목 |

변경하지 않은 것: `DxfWriter.h/.cpp`, `test_dxf.cpp`, `studio/src/*`, `GigaMesh/core/**`, `GigaMesh/external/**`, `.github/workflows/*.yml` (AC-8: `git diff --name-only e7e641b..HEAD` = 정확히 위 2개 파일).

## 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-001 -BuildRoot D:\_dongarch_build\verify_job001 -NoShots` | 0 | configure 0 (5초) / build 0 (18초) / ctest 0 (10초), 요약 `결과: PASS`. 로그: `D:\_dongarch_build\verify_job001\studio-logs\01_configure.log`, `02_build.log`, `03_ctest.log` |
| `cd /d/_dongarch_build/verify_job001/studio && ctest -N` | 0 | `Test #10: karch_test_dxf_units`, **`Total Tests: 12`** (AC-1) |
| `ctest -R karch_test_dxf_units -V` | 0 | `Passed 0.12 sec`. stdout: 변이 1~4 각각 `rejected: FAIL AC-2.3 / AC-2.2 / AC-3c / AC-4.1 …`, 마지막 줄 `dxf karch_t8u_cyl.dxf bytes=39167 … 4/4 mutations rejected` (AC-2~5, AC-7) |
| `ctest --output-on-failure` (verify.ps1 내부, `03_ctest.log`) | 0 | **`100% tests passed out of 12`**, 기존 11종 회귀 없음 (AC-1) |
| `grep -nE "[A-Za-z]:[\\/]|gigamesh" GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp` | 1 (0건) | AC-6 정적 검사 통과 |
| `ls -l D:\_dongarch_build\verify_job001\studio\karch_t8u_cyl.dxf` / `ls .worktrees/JOB-001/karch_t8u_*.dxf \| wc -l` | 존재 (39,167 B) / 0 | AC-6: 산출물은 빌드 폴더(ctest 작업 폴더)에만 생성, 작업 트리 루트에는 없음 |
| `grep -c warning D:\_dongarch_build\verify_job001\studio-logs\02_build.log` | 0건 | `/W4 /permissive-` 기준 새 파일 포함 전체 빌드 경고 0 |
| `git -c user.name=kesekikwon-netizen -c user.email=kesekikwon@gmail.com commit -F <msg>` | 0 | 아래 커밋 SHA |
| `git diff --name-only e7e641b..HEAD` | 0 | `GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp`, `studio/CMakeLists.txt` 2개 (AC-8) |

### AC별 자체 확인
| AC | 결과 | 근거 |
|---|---|---|
| AC-1 | 통과 | `ctest -N` 12개·`karch_test_dxf_units` 등록, `-R` 종료 0, 전체 12/12 |
| AC-2 | 통과 | 섹션 순서·`0/EOF`·`$ACADVER`→`1/AC1009`·`$INSUNITS`→`70/4`·위치 순서 모두 검사, 원본 통과 |
| AC-3 | 통과 | `70`=12=레코드 12, 레이어 0(7/CONTINUOUS) + 11개 이름·ACI·선종류·`70`=0 순서 일치, 중복 없음 |
| AC-4 | 통과 | 엔티티 `8` ∈ 11개(레이어 0 불허), `6` ∈ {CONTINUOUS, CENTER, DASHED}, 타입 ⊆ R12 8종, 01/02/07/08/09/10/11 에 엔티티 존재 |
| AC-5 | 통과 | 07/08 CIRCLE 각 1개 r=50.000000, 중심 (0, 165)/(0, −65); 09 LINE 높이 100.0, 수직; 02 외곽선 max\|x\|·z 범위 허용치 안 |
| AC-6 | 통과 | 정적 grep 0건, 산출물 빌드 폴더에만 생성 |
| AC-7 | 통과 | 4종 변이 모두 원문에 패턴 존재 확인 후 치환, 4/4 `checkDxf` 실패로 판정 |
| AC-8 | 통과 | 변경 파일 정확히 2개 |

## 남은 경고
- 컴파일 경고: **0건** (`02_build.log` 에 `warning` 없음, MSVC `/W4 /permissive-`).
- verify.ps1 실행 시 콘솔에 `'vswhere.exe'은(는) 내부 또는 외부 명령… 아닙니다` 가 2회 출력됨(vcvars64.bat 환경 메시지). configure/build 종료 코드 0 이며 이번 변경과 무관(기존 환경 메시지).
- 파일 길이 589줄로 명세 §7 추정치(230~280줄, ≤300)를 넘음. 이유: karch 코드 관례대로 모든 `if` 에 중괄호·줄바꿈을 쓰고, 실패 메시지에 기대값·실측값·줄 번호를 모두 넣었기 때문. 명세 §7 의 "넘기면 AC-5-4 를 뺀다"는 적용하지 않았다 — AC-5-4 는 수용 기준(§3)에 있으므로 유지. 수용 기준에 줄 수 제한은 없음.

## 제안 (범위 밖, 하지 않음)
- `test_dxf.cpp:34-42` 의 `D:\gigamesh\` 절대 경로 폴백 제거 → 새 테스트와 같은 `argv[1]`/cwd 방식으로 통일(별도 chore, 명세 §5).
- `.github/workflows/studio-ci.yml:31,60,63` 의 "11 Suites"/"10 K-Arch Unit Tests" 문구가 12/11 로 낡음(동작 영향 없음, 명세 §2 에 따라 미수정).
- karch(`DxfWriter.cpp:269-279`)와 Studio(`studio/src/Plate.cpp:723-733`)의 같은 11개 레이어 이름에 대한 ACI/선종류 불일치는 이번 테스트가 karch 쪽만 고정함. 정본 결정 후 Studio `exportDxf` 산출물(`studio_plate.dxf`)에 같은 파서를 재사용하려면 `checkDxf()` 의 기대 레이어 표를 인자로 빼면 된다(명세 §6 선택 항목, 이번에는 미구현).
- `kLinetypes` 는 고정 집합이다. LTYPE 테이블을 실제로 읽어 "참조된 선종류가 LTYPE 에 정의돼 있는지"를 검사하면 더 엄밀해진다(현재 명세 AC-4-2 는 고정 집합으로 충분).

## 커밋
- `4ed86afc2c06fe08191331318bf4a63cb8d53df8` — `test(karch): DXF R12·$INSUNITS·11 레이어 구조 검증 테스트 추가 (JOB-001)` (브랜치 `factory/JOB-001`, 작성자 kesekikwon-netizen). push 하지 않음(사용자 승인 전 금지).

## 차단 사유 (있으면)
- 없음.
