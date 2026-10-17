# JOB-001 검증 기록 (test-runner)

결론: **PASS**

- 검증 대상: 작업 트리 `D:/_dongarch_build/wt/.worktrees/JOB-001`, 브랜치 `factory/JOB-001`, HEAD `4ed86afc2c06fe08191331318bf4a63cb8d53df8`(기준 `e7e641b`). 검증 시작 시점 `git status --porcelain` 빈 출력(clean).
- 모든 명령은 test-runner 가 직접 실행했고 종료 코드를 그대로 적었다. builder 의 `build-log.md` 주장은 참고만 했으며 결과는 전부 일치했다.

## 환경
- 컴파일러: MSVC 19.44.35228.0 (VS 2022 BuildTools, `VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe`), Ninja(VS 내장), `CMAKE_BUILD_TYPE=Release`, 프로젝트 옵션 `/utf-8 /W4 /permissive- /EHsc`
- Qt: Qt6 `D:/OSGeo4W/apps/Qt6` (`Qt6_DIR=D:/OSGeo4W/apps/Qt6/lib/cmake/Qt6`), ctest 실행 시 `PATH+=Qt6\bin`, `QT_PLUGIN_PATH=Qt6\plugins`, `QT_QPA_PLATFORM=offscreen`
- 빌드 폴더: `D:\_dongarch_build\verify_job001\studio` — 검증 전 `rm -rf /d/_dongarch_build/verify_job001` 로 완전히 지운 뒤 처음부터 configure/build. 로그 `D:\_dongarch_build\verify_job001\studio-logs\`
- 실행 셸: Git Bash(Windows 11). `ctest` 는 빌드 폴더로 `cd` 한 뒤 실행, `--test-dir` 미사용.
- 시각: 2026-10-17 15:12:17 검증 시작 → verify.ps1 15:12:38~15:13:22(44초: configure 5s / build 19s / ctest 10s / shots 10s) → 개별 판정·DXF 교차확인·산출물 복사 완료 15:18. **총 소요 약 6분.**
- 경고 수: `grep -c "warning" D:/_dongarch_build/verify_job001/studio-logs/02_build.log` = **0** (grep 종료 코드 1 = 일치 없음).

## 1단계: verify.ps1 (clean, 처음부터)
명령:
```
rm -rf /d/_dongarch_build/verify_job001
powershell -NoProfile -ExecutionPolicy Bypass -File D:/_dongarch_build/wt/.worktrees/JOB-001/factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-001 -BuildRoot D:\_dongarch_build\verify_job001
```
종료 코드 **0**. `D:\_dongarch_build\verify_job001\studio\verify-summary.md` 내용:

| 단계 | 종료 코드 | 초 | 로그 |
|---|---|---|---|
| configure | 0 | 5 | D:\_dongarch_build\verify_job001\studio-logs\01_configure.log |
| build | 0 | 19 | D:\_dongarch_build\verify_job001\studio-logs\02_build.log |
| ctest | 0 | 10 | D:\_dongarch_build\verify_job001\studio-logs\03_ctest.log |
| shots | 0 | 10 | D:\_dongarch_build\verify_job001\studio-logs\04_shots.log |

소스: `D:\_dongarch_build\wt\.worktrees\JOB-001` / 빌드 폴더: `D:\_dongarch_build\verify_job001\studio` / **결과: PASS**

`02_build.log` 22행 `[22/56] Building CXX object CMakeFiles\test_dxf_units.dir\...\test_dxf_units.cpp.obj`, 39행 `[39/56] Linking CXX executable test_dxf_units.exe`. `03_ctest.log`: `10/12 Test #10: karch_test_dxf_units ............. Passed 0.16 sec`, `100% tests passed out of 12`.

## 수용 기준별 결과
| 기준 | 명령 | 종료 코드 | 결과 | 근거(로그 줄·산출물 경로) |
|---|---|---|---|---|
| AC-1 (등록) | `cd /d/_dongarch_build/verify_job001/studio && ctest -N` | 0 | 통과 | `Test #10: karch_test_dxf_units`, `Total Tests: 12` (`artifacts/step2_ctest_N.log`) |
| AC-1 (단독 통과) | `ctest -R karch_test_dxf_units --output-on-failure -V` | 0 | 통과 | `1/1 Test #10: karch_test_dxf_units ............. Passed 0.12 sec`, `100% tests passed out of 1` (`artifacts/step3_ctest_dxf_units_V.log`) |
| AC-1 (회귀 없음) | `ctest --output-on-failure` | 0 | 통과 | `100% tests passed out of 12`, 12종 모두 `Passed` (`artifacts/step4_ctest_all.log`; verify.ps1 의 `03_ctest.log` 도 동일) |
| AC-2 HEADER 구조 | 위 `ctest -R … -V` + DXF 직접 파싱(`grep -n -A2`, awk) | 0 | 통과 | 테스트 stdout `HEADER R12 mm … ok`. 파일 교차확인(`artifacts/step5_dxf_header_layers_circles.log`): 6~8행 `$ACADVER`/`1`/`AC1009`, 10~12행 `$INSUNITS`/`70`/`4`, 둘 다 HEADER 섹션 안(HEADER `0/ENDSEC` 14행), 순서 ACADVER(6) < INSUNITS(10) < ENDSEC(14); 섹션 순서 HEADER→TABLES(ENDSEC 210행)→ENTITIES(ENDSEC 5784행), 마지막 쌍 `0`/`EOF`(5785~5786행). CR 0개(LF 전용). 변이 1·2 거부: `mutation 1 ($INSUNITS 4 -> 1 inch) rejected: FAIL AC-2.3 $INSUNITS next pair is 70/1, expected 70/4 (mm)`, `mutation 2 ($ACADVER AC1009 -> AC1015) rejected: FAIL AC-2.2 $ACADVER next pair is 1/AC1015, expected 1/AC1009` |
| AC-3 LAYER 표 | 위 `ctest -R … -V` + awk 로 `0/LAYER` 레코드 추출 | 0 | 통과 | 테스트 stdout `12 layer records`. 파일 교차확인(`artifacts/step5_dxf_header_layers_circles.log` (d)/(d2)): LAYER 테이블 `70`=12(86행), `0/LAYER` 레코드 12개, 순서·값: `0`(7/CONTINUOUS), `01_AXIS_CENTER`(1/CENTER), `02_EXTERIOR_OUTLINE`(7), `03_EXTERIOR_FEATURES`(3), `04_SECTION_WALL`(4), `05_SECTION_HATCH`(8), `06_INTERIOR_OUTLINE`(5), `07_PROJECTION_RIM`(6), `08_PROJECTION_BASE`(6), `09_DIMENSION_MEASURE`(2), `10_ANNOTATION_TEXT`(7), `11_SCALE_BAR`(7) — 01 만 CENTER, 나머지 CONTINUOUS, `70` 플래그 전부 0, 중복 없음 → 명세 §3 AC-3 표와 일치. 변이 3 거부: `mutation 3 (01_AXIS_CENTER ACI 1 -> 2) rejected: FAIL AC-3c layer[1] 01_AXIS_CENTER ACI(62) 2, expected 1`. 테스트 소스 `test_dxf_units.cpp:75-87` `constexpr LayerExpect kLayers[12]` 가 같은 표 |
| AC-4 참조 무결성 | 위 `ctest -R … -V` + awk(ENTITIES 의 코드 8/6/0 집계) | 0 | 통과 | 테스트 stdout `entity refs … ok`. 파일 교차확인(`artifacts/step5_dxf_extra.log`): ENTITIES 레이어(8) 집합 = 01(5)·02(402)·04(44)·05(33)·06(23)·07(1)·08(1)·09(1)·10(4)·11(7) — 레이어 `0`·미정의 이름 없음; 선종류(6) 값 = `CENTER` 5건뿐(LTYPE 테이블 CONTINUOUS/CENTER/DASHED 안); 엔티티 타입 = CIRCLE 2, LINE 40, POLYLINE 3, SEQEND 3, SOLID 5, TEXT 5, VERTEX 463 (R12 8종 안, LWPOLYLINE/HATCH/MTEXT 없음); 필수 레이어 01/02/07/08/09/10/11 각각 ≥1. 변이 4 거부: `mutation 4 (first entity on 02 -> 99_UNKNOWN) rejected: FAIL AC-4.1 layer ref POLYLINE (line 305) on undefined layer 99_UNKNOWN` |
| AC-5 mm 축척 | 위 `ctest -R … -V` + awk(CIRCLE/LINE/VERTEX 수치) | 0 | 통과 | 테스트 stdout `mm scale ok`. 파일 교차확인(`step5_dxf_header_layers_circles.log` (e)(e2)(f), `step5_dxf_extra.log`): `07_PROJECTION_RIM` CIRCLE 정확히 1개 `10`=0.000000, `20`=165.000000, **`40`=50.000000**(\|r−50\|=0 ≤ 0.1); `08_PROJECTION_BASE` CIRCLE 1개 `20`=−65.000000, `40`=50.000000; `09_DIMENSION_MEASURE` LINE (−62,0)→(−62,100): \|21−20\|=100.000000, \|11−10\|=0; `02_EXTERIOR_OUTLINE` POLYLINE 정점 400개 max\|10\|=50.000000, `20`∈[0.125, 99.875], 범위 99.750 ≥ 99.0 (명세 추정치 99.75 와 일치). 파일: `artifacts/karch_t8u_cyl.dxf`(39,167 B, sha256 b925380f…) |
| AC-6 산출 경로 | `grep -nE "[A-Za-z]:[\\/]\|gigamesh" …/karch/test_dxf_units.cpp` / `find /d/_dongarch_build/verify_job001 -name karch_t8u_cyl.dxf` / `ls <worktree>/karch_t8u_*.dxf \| wc -l` / `git -C <worktree> status --porcelain` | grep 1(0건) / 0 / 0개 / 0(빈 출력) | 통과 | 정적 grep 일치 없음; DXF 는 `D:\_dongarch_build\verify_job001\studio\karch_t8u_cyl.dxf`(ctest 작업 폴더) 에만 생성(39,167 B, 15:14); 작업 트리 전체 `find` 0개, `git status --porcelain` 빈 출력, `--ignored` 포함 검색도 0건. 소스 `test_dxf_units.cpp:490-492` argv[1] 분기, `:519` 상대 이름 `karch_t8u_cyl.dxf` (`artifacts/step6_7_static_git.log`) |
| AC-7 음성 대조 | 위 `ctest -R … -V` | 0 | 통과 | stdout 4줄 `mutation 1 … rejected: FAIL AC-2.3`, `mutation 2 … FAIL AC-2.2`, `mutation 3 … FAIL AC-3c`, `mutation 4 … FAIL AC-4.1`, 요약 `dxf karch_t8u_cyl.dxf bytes=39167 … 4/4 mutations rejected`. 소스 `:549-553` 의 4개 치환 패턴이 명세 §3 AC-7 과 문자 단위로 동일, `:566-568` 에서 원문 패턴 부재 시 `FAIL mutation … pattern not present in original` 로 실패 처리, `:559-561` 변이 4 는 `\n2\nENTITIES\n` 이후에서만 치환 (`artifacts/step3_ctest_dxf_units_V.log`) |
| AC-8 변경 범위 | `git -C <worktree> diff --name-only e7e641b..HEAD` | 0 | 통과 | 정확히 2개: `GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp`(+589), `studio/CMakeLists.txt`(+2/−1: 13행 옵션 설명 "10"→"11", `KARCH_TESTS` 에 `test_dxf_units` 1줄). `GigaMesh/core`·`external`·`DxfWriter.*`·`test_dxf.cpp`·`studio/src/*` 변경 없음 (`artifacts/step6_7_static_git.log`) |

종합: AC-1 ~ AC-8 전부 통과, 실패·미확인 항목 없음 → **PASS**.

## 산출물 (artifacts/, 커밋하지 않음)
`D:/_dongarch_build/wt/factory/jobs/JOB-001/artifacts/`
- `01_configure.log`, `02_build.log`, `03_ctest.log`, `04_shots.log` — verify.ps1 단계별 로그(`04_shots.log` 는 빈 파일, 종료 0)
- `verify-summary-studio.md` — verify.ps1 요약(결과 PASS)
- `step2_ctest_N.log`, `step3_ctest_dxf_units_V.log`, `step4_ctest_all.log` — 2~4단계 ctest 직접 실행 출력
- `step5_dxf_header_layers_circles.log` — DXF 독립 파싱(HEADER `$ACADVER`/`$INSUNITS`, 섹션 위치, LAYER 테이블 12건, CIRCLE·치수 LINE 수치, EOF)
- `step5_dxf_extra.log` — DXF 독립 파싱(레이어별 엔티티 수, 선종류, 엔티티 타입 집합, LTYPE, 외곽선 정점 범위)
- `step6_7_static_git.log` — AC-6 정적 grep·작업 트리 청결·AC-8 diff
- `karch_t8u_cyl.dxf` — 테스트가 빌드 폴더에 생성한 DXF 원본 복사(39,167 B, 5,786줄, LF, sha256 동일 확인)
- `shot_home.png`, `shot_main.png`, `shot_export.png` — Studio 오프스크린 캡처 3장

## 비고
- **한글 □ (폰트 미탐색)**: 캡처 3장 모두 한글 UI 문자열이 □ 로 렌더링됨(`QT_QPA_PLATFORM=offscreen`, 한글 폰트 미탐색). 홈/메인(3D 뷰·도판 미리보기)/내보내기 창의 레이아웃 자체는 정상. 이번 작업(DXF 테스트)의 수용 기준과 무관하며 기존 환경 현상.
- verify.ps1 콘솔에 `'vswhere.exe'은(는) 내부 또는 외부 명령 … 아닙니다` 가 2회 출력됨(vcvars64.bat 환경 메시지). configure/build 종료 코드 0, 결과에 영향 없음(builder 기록과 동일).
- 지시에 따라 verify.ps1 을 `-Root/-BuildRoot` 로 실행(`-Job` 미사용)했으므로 artifacts 복사는 수동으로 수행함(위 목록). `-NoShots` 를 쓰지 않아 캡처도 생성됨.
- 새 테스트 파일은 589줄로 명세 §7 의 추정(≤300줄)을 넘지만 §7 은 수용 기준이 아님. builder 가 `build-log.md` 에 사유를 기록해 둠. 리뷰어 판단 사항.
- builder `build-log.md` 의 수치(12/12, 경고 0, DXF 39,167 B, 변이 4/4 거부, 변경 파일 2개)는 이번 clean 재검증 결과와 모두 일치.
- 명세 §2 의 karch/Studio 레이어 ACI·선종류 불일치는 범위 밖이며 이번 변경으로 달라진 것 없음(테스트는 karch `WritePlateDxf` 산출물만 대상).
- 제품 코드·테스트 코드·git 에 아무것도 쓰지 않았다(쓴 곳: 이 파일과 `artifacts/` 뿐).
