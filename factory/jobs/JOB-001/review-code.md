# JOB-001 검토: code (code-reviewer, 2026-10-17)

결론: **PASS**

대상 `factory/JOB-001` @ 4ed86af, 기준 e7e641b. 줄 번호는 `GigaMesh/gui/src/dongarch/karch/test_dxf_units.cpp`(신규)와 `DxfWriter.cpp`(대조용)에서 직접 확인. 빌드·ctest 는 실행하지 않음(test-runner 몫).

| # | 항목 | 판정 | 근거 |
|---|---|---|---|
| 1 | 명세 범위 | pass | 변경 파일 정확히 2개(AC-8). `studio/CMakeLists.txt:13` 설명 "10→11"은 명세 §2·§5 가 허용한 선택 1줄. `DxfWriter.*`·`test_dxf.cpp`·`studio/src/*` 무변경. 파일 머리 주석 L2-8 이 AC 번호로 색인. 좌표 부호 미잠금(L457-487) → D5 중립 |
| 2 | 금지 구역 | pass | `GigaMesh/core/`, `external/` 변경 없음 |
| 3 | 규약 | pass | UI 문자열·색상 없음. GPLv3+ 머리 주석 L1(`test_dxf.cpp:1`·`DxfWriter.cpp:1` 관례). 영문 주석은 karch 폴더 관례, 커밋은 한국어 본문·`test(karch): …`·트레일러 2줄(CLAUDE.md §6). mm 단위 명시(L6-7, 442, 460, 466). 실수 파싱 `std::locale::classic()`(L146 ↔ `DxfWriter.cpp:67`) |
| 4 | 구조 | pass(주의 2건) | 파서(`parsePairs` L165 / `splitSections` L197 / `records` L234) 재사용, 중복 없음. 에러 처리 양호(홀수 줄 L180-183, EOF 뒤 잔여 L201-204, ENDSEC 누락 L224-227, `dim.ok` 경고 L520-523, 경로 포함 메시지 L524-531). 주의: (a) `checkDxf` L259-488 230줄 단일 함수 — 명세 §5 의 분해 제안 미적용. (b) 허용오차 리터럴이 조건식과 메시지에 이중 기재(L439/442, 459/461, 465/466, 483/485), `12`/`11` 반복(L75·343·348·379). FAIL 사유 아님 |
| 5 | 테스트 | pass | `studio/CMakeLists.txt:102` 등록 → `karch_test_dxf_units`. AC-2 L285-313, AC-3 L315-371, AC-4 L373-401, AC-5 L403-487, AC-7 L541-583(4변이 `rc` 누적, L577). 절대 경로 grep 0건, 기본 산출 경로 상대 `karch_t8u_cyl.dxf`(L498) + `.gitignore:84`. 작성기 대조: HEADER 순서 `DxfWriter.cpp:81-87` = 기대 L287-301; LAYER 필드 순서 `2,70,62,6`(:129-140) = 변이 3 패턴(L551); VERTEX·SEQEND 에도 `8`(:178,184) 있어 AC-4.1 전수 검사 성립; 02 첫 엔티티 POLYLINE(:292)이라 변이 4 가 ENTITIES 를 침(L555-560) |
| 6 | 문서 | pass | 사용자 가시 동작 변화 없음. `studio-ci.yml` "11 Suites/10 K-Arch" 문구는 명세 §2 범위 밖, `build-log.md:46` 에 메모됨 |

## 파일 길이(명세 §7 ≤300줄 vs 실제 589줄)
구성: 머리 36 / 자료구조·상수표 65(L38-103) / Verdict 20 / 숫자 파서 45(L118-162) / DXF 파서 83(L165-247) / `checkDxf` 230 / 경로·slurp 16 / main 82(변이 40).
- 정당: 실패를 최대 20건 누적해 AC·줄·실제값·기대값을 찍는 `Verdict` 설계(변이별 거부 AC 가 stdout 에 남음 L579), karch 중괄호 관례, AC-5-4 유지(AC 를 빼지 않은 것이 옳음).
- 비정당: `checkDxf` 230줄은 §5 제안대로 나눴다면 함수당 60-80줄. 초과의 본질은 "분해하지 않아서". 길이는 AC 가 아니라 PASS 유지. rework 가 열리면 제안 1 부터.

## 고치지 않아도 되는 제안(우선순위순)
1. `checkDxf` 를 `checkHeader / checkLayerTable / checkEntityRefs / checkMmScale` 4개로 분해.
2. 허용오차 상수화(`kRadiusMm, kRadiusTolMm, kRimYMm, kCenterTolMm, kHeightMm, kHeightTolMm, kVerticalTolMm`, `kMaxReported`) + 메시지 자동 생성.
3. `kLayerCount = sizeof(kLayers)/sizeof(kLayers[0])` 로 `12` 반복 제거.
4. 도달 불가 분기 2곳(`duplicate layer name` L361, LTYPE 집합 검사 L369) 제거 또는 사전 패스로 이동.
5. `splitSections` 빈 `why`(L205) → `"no SECTION before 0/EOF"`.
6. CIRCLE 수치 파싱 실패 묵살(L430-432) → "non-numeric 10/20/40" 분기.
7. AC-2.4 항진 조건(L311) 주석 또는 제거.
8. `std::size_t` + `<cstddef>` 명시(L39 등).
9. `slurp` 중복(`test_dxf.cpp:44-47`) → 후속 chore 때 `karch_test_util.h` 로.

## 확인 필요
- `std::istreambuf_iterator` 의 `<iterator>` 미포함(L503) — MSVC 에선 전이 포함으로 빌드되나 표준 보장 아님.
- `fails.size() < 20`(L103) `size_t` vs 리터럴 — `/W4` 경고 여부는 test-runner 로그로(결과: 경고 0).
- AC-5 정답값(y=165/−65, 범위 ≥99)은 명세 근거를 그대로 옮긴 것 — 통과 여부는 ctest 결과(test-runner: 12/12 PASS).
