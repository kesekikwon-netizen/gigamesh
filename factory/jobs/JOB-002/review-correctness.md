# JOB-002 검토: correctness (correctness-reviewer, 2026-10-17)

결론: **PASS** — 반례 없음. 변경은 산출 경로 문자열에만 국한되고, 경로 변형 11종 전부에서 산출 5종이 올바른 위치에 **바이트 동일**하게 생성되며, 실패 입력에서는 크래시 없이 비0 종료한다.

대상 HEAD 32d5d85(기준 d74ae33). 실행파일 `D:/_dongarch_build/verify_job002/studio/test_{section,dimensions,dxf}.exe`(HEAD 빌드임을 `grep -a gigamesh` 0건으로 확인). `D:/gigamesh` 미접근.

## 1. 경로 조합(실행)
헬퍼 3곳 동일(section:48-54, dimensions:69-75, dxf:34-40). 임시 폴더 `review_scratch2/`에서 변형별 실행: 인자 없음 / `""` / 절대(끝 구분자 없음·`/`·`\`) / 상대 / `.` / 공백 포함 / 혼합 구분자 / 한글 폴더 → 모두 종료 0/0/0, 5개 파일이 지정 위치에 생성, `studio/` 산출과 `cmp` 5/5 동일(11변형 전부). 루트에 엉뚱한 파일 없음. 되읽기 변수 일치(dimensions :103→:104→:108, :131→:132→:136; dxf :58→:59→:64, :97→:98→:103). 추정: `argv[1]="D:"`는 드라이브 루트에 씀(JOB-001 헬퍼와 동일, 명세 밖).

## 2. 검증 논리 불변(diff 전수)
경로 외 변경 줄: `#include <string>`, `main(int argc, char** argv)`×3, 헬퍼×3, `printf(... svgPath.c_str() ...)`(포맷 동일, 타입 보정만), 옛 probe 삭제. 허용 오차·단언·`FAIL …` 메시지·기대 문자열(section:57-117, dimensions:27-67,108-115,136-141, dxf:64-107)에 hunk 없음. 실행 출력 수치(`rim=100.000…`, `wall area=500.0000 err=0.0000%`, `bytes=8533`, `39167/39231`)가 검증자 로그와 동일.

## 3. 병렬 실행(`ctest --parallel 4`)
공통 cwd=빌드 폴더. 파일명 집합이 서로소: JOB-002 5종 / JOB-001 `karch_t8u_cyl.dxf` / `studio_pipeline` `studio_plate.*`·`studio_rt.stl`·`studio_blind.svg` / `test_mesh_loader` `karch_t1_*` / `test_synthetic` `karch_t0_*`. 경합 없음.

## 4. 실패 처리(실행)
없는 폴더(끝 `/`·`\` 포함)·기존 파일을 폴더로 → 세 exe 모두 exit 1 + `FAIL svg / FAIL write svg / FAIL write dxf`, 크래시·폴더 자동 생성·잔여 파일 없음(`DxfWriter.cpp:266`, `SectionComposer.cpp:285`, `DimensionAnalyzer.cpp:242` 의 false 반환 경로).

## 5. 단위·축
제품 코드 무변경, 경로 변형 전부 산출 바이트 동일 → mm 수치·Z-up·좌/우 배치(D5)에 영향 없음.

## 6. 기준 대비
`argtest/`↔`studio/` 5/5 동일. 기준 exe 산출과의 직접 cmp 는 불가(기준은 D:\gigamesh 에만 썼음)이나 `snap_before.txt` 의 기준 파일 크기(7768/10414/8533/39167/39231)가 현재와 바이트 수까지 일치 → 내용 동일(근거 강한 추정).

AC 교차: AC-1 exit 1, AC-2 표, AC-3 11/11, AC-4 스냅샷 동일(검증자), AC-5 3파일, AC-6 31/16.
