# JOB-002 명세 — K-Arch 테스트 3종의 `D:\gigamesh` 절대 경로 제거(CI 러너 실패 수정)

- 작성: 2026-10-17, spec-writer. 기준 코드: `factory/setup` 작업 트리(`D:/_dongarch_build/wt`), `job.json.base_sha` = `d74ae33b1480fcbd64db9e8bec897033c07c441f`. 줄 번호는 모두 이 트리에서 Grep 으로 확인.
- 실패 근거: run 39010516345 ctest 로그(`D:/_dongarch_build/gh_ctest_failed.log`) — `karch_test_section ***Failed` + `FAIL svg`, `karch_test_dimensions ***Failed` + `FAIL write svg`, `83% tests passed, 2 tests failed out of 12`. `karch_test_dxf` 는 Passed(러너에 `D:\gigamesh` 가 없어 `test_dxf.cpp:34-42` 의 cwd 폴백이 작동).

## 목표
요청 원문(`factory/jobs/JOB-002/job.json` → `request`):
> "…세 테스트가 argv[1] 폴더 또는 cwd 상대 경로에만 쓰도록 고쳐 러너와 개발 PC 모두에서 통과하게 한다. 제품 코드(DxfWriter 등)와 테스트의 검증 논리는 바꾸지 않는다."

재진술: `test_section.cpp`·`test_dimensions.cpp`·`test_dxf.cpp` 가 산출 파일(SVG/DXF)을 **`argv[1]` 디렉터리, 없으면 cwd 상대 경로**에만 쓰게 바꿔(JOB-001 `test_dxf_units.cpp:490-499` 와 같은 패턴) GitHub 러너(cwd = `studio/build-ci`)와 개발 PC 양쪽에서 통과시킨다.

## 범위 밖
- 테스트의 검증 논리·허용 오차·기대 문자열 변경(`test_section.cpp:57-117`, `test_dimensions.cpp:27-67,100-107,128-133`, `test_dxf.cpp:66-109`). 산출 파일명 변경.
- 제품 코드 `DxfWriter.*`, `SectionComposer.*`, `DimensionAnalyzer.*`, `Silhouette.*`, `GigaMesh/core`·`external`.
- 새 테스트 추가, `studio/CMakeLists.txt`(`add_test(NAME karch_${t} COMMAND ${t})` 는 인자 없이 실행 — `:104-108` — 그대로 둔다), CI yml 문구(`studio-ci.yml:60,63`), 공용 테스트 헤더 신설(§영향 파일 이유), D5 좌/우 배치.
- push·병합: 커밋은 `factory/JOB-002` 브랜치에만. push/Actions 실행은 사용자 지시(`job.json.request` 말미)에 따라 오케스트레이터가, 병합은 `/factory approve` 뒤(`CLAUDE.md` §4-4).

## 수용 기준 (각 항목은 명령 하나로 참/거짓이 갈림)
1. **정적**: `grep -nE "[A-Za-z]:[\/]|gigamesh" GigaMesh/gui/src/dongarch/karch/test_{section,dimensions,dxf}.cpp` 가 0건(종료 코드 1). 현재 4건(`test_section.cpp:120`, `test_dimensions.cpp:95,123`, `test_dxf.cpp:35`).
2. **경로 규칙**: 세 테스트 모두 `main(int argc, char** argv)` 이고, `argv[1]` 이 비어 있지 않으면 `argv[1]` + 구분자 + 파일명, 아니면 파일명만(cwd 상대)으로 쓴다. 파일명은 기존 그대로 `karch_t5_cyl.svg`, `karch_t6_cyl.svg`, `karch_t6_aligned.svg`, `karch_t8_cyl.dxf`, `karch_t8_frag.dxf`. 다시 읽는 코드(`test_dimensions.cpp:100,128` 의 `std::ifstream`, `test_dxf.cpp:66,105` 의 `slurp`)는 **쓸 때와 같은 경로 변수**를 쓴다(코드 리뷰). 동적 판정: (a) ctest 뒤 빌드 폴더 `D:\_dongarch_build\verify_job002\studio` 에 위 5개 파일이 있다. (b) 세 exe 를 `<exe> argtest` 로 직접 실행하면 종료 0 이고 `argtest\` 에 같은 5개가 생긴다.
3. **회귀 없음**: `verify.ps1` 결과 PASS, `03_ctest.log` 에 `100% tests passed`. 기준 SHA d74ae33 은 11종(`studio/CMakeLists.txt:90,93-103`; `test_dxf_units` 는 JOB-001 브랜치(4ed86af, 기반 e7e641b)에만 있고 d74ae33 트리에 없음 — `git ls-tree -r --name-only d74ae33 | grep karch/test_` 로 확인)이므로 **11/11**. JOB-001 병합 뒤 재검증이면 12/12.
4. **메인 체크아웃 불변(러너 환경 모사)**: 검증 전후 `ls -l --time-style=full-iso /d/gigamesh/karch_t* 2>/dev/null` 결과가 동일(`diff` 종료 0) — `D:\gigamesh` 루트에 `karch_t*` 가 새로 생기거나 mtime 이 바뀌지 않는다. 검증자는 `D:\gigamesh` 를 읽기만 한다(`CLAUDE.md` §4-8).
5. **변경 범위**: `git diff --name-only d74ae33..HEAD` = 정확히 `GigaMesh/gui/src/dongarch/karch/test_section.cpp`, `…/test_dimensions.cpp`, `…/test_dxf.cpp` 3개.
6. **크기**: `git diff --shortstat d74ae33..HEAD` 에서 insertions ≤ 40, deletions ≤ 25.

## 검증 명령
```bat
:: AC-4 사전 스냅샷(Git Bash): ls -l --time-style=full-iso /d/gigamesh/karch_t* 2>/dev/null > /d/_dongarch_build/verify_job002/gm_before.txt
powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-002 -BuildRoot D:\_dongarch_build\verify_job002
::   (빌드 폴더 = <BuildRoot>\studio, verify.ps1:27; 로그를 artifacts/ 로 복사하려면 -Job JOB-002 추가, verify.ps1:80-81)
cd /d D:\_dongarch_build\verify_job002\studio && ctest --output-on-failure                                        :: AC-3 (cd 후 실행, --test-dir 금지)
dir karch_t5_cyl.svg karch_t6_cyl.svg karch_t6_aligned.svg karch_t8_cyl.dxf karch_t8_frag.dxf                     :: AC-2(a) 5개
mkdir argtest 2>nul & test_section.exe argtest && test_dimensions.exe argtest && test_dxf.exe argtest && dir argtest  :: AC-2(b) 종료 0, 5개
```
```bash
cd /d/_dongarch_build/wt/.worktrees/JOB-002
grep -nE "[A-Za-z]:[\/]|gigamesh" GigaMesh/gui/src/dongarch/karch/test_{section,dimensions,dxf}.cpp; echo "exit=$?"   # AC-1: exit=1
ls -l --time-style=full-iso /d/gigamesh/karch_t* 2>/dev/null > /d/_dongarch_build/verify_job002/gm_after.txt; diff /d/_dongarch_build/verify_job002/gm_{before,after}.txt; echo "exit=$?"   # AC-4: exit=0
git diff --name-only d74ae33..HEAD; git diff --shortstat d74ae33..HEAD                                                 # AC-5, AC-6
```
- CI: push 가 허용되면 `studio-ci.yml:63-65`(`working-directory: studio/build-ci`, `ctest --output-on-failure --parallel 4`)가 그대로 재실행된다. 러너 cwd = 빌드 폴더이므로 상대 경로 쓰기가 성공해야 한다.

## 영향 파일
| 파일 | 변경 | 이유 |
|---|---|---|
| `GigaMesh/gui/src/dongarch/karch/test_section.cpp` | `#include <string>` 추가(현재 없음, `:7-10`), `static std::string outPath(int argc, char** argv, const char* name)` 헬퍼 ≈6줄 추가, `main()`→`main(int argc, char** argv)`(`:47`), `:120` → `WriteDrawingSvg(outPath(argc, argv, "karch_t5_cyl.svg"), …)` | `WriteDrawingSvg(const std::string& path, …)`(`SectionComposer.h:37`) 이라 `std::string` 직접 전달 가능 |
| `…/test_dimensions.cpp` | 헬퍼 추가, `main` 시그니처(`:69`), `:95`/`:123` 을 `const std::string svgPath/svgShift = outPath(argc, argv, "…")`, **`:108` `svgPath` → `svgPath.c_str()`**(printf `%s` 에 `std::string` 전달 금지) | `:100,128` `std::ifstream(svgPath/svgShift, …)` 는 `std::string` 을 그대로 받으므로 변수 재사용만으로 쓰기·읽기 경로 일치. `WritePlateSvg(const std::string&)`(`DimensionAnalyzer.h:43`) |
| `…/test_dxf.cpp` | `:34-42` `outPath(const char*)` 본문을 `outPath(int, char**, const char*)` 로 교체(`D:\gigamesh` probe·`std::ofstream app` 삭제), `main` 시그니처(`:49`), `:60`/`:99` 호출에 `argc, argv` 전달 | `:66,105` `slurp(dxfPath/fragPath)`·`:110` `dxfPath.c_str()` 는 이미 변수 기반이라 불변. `WritePlateDxf(const std::string&)`(`DxfWriter.h:53`) |

헬퍼는 파일마다 작게 복제한다(`test_dxf_units.cpp:490-499` 와 동일 패턴: `argc > 1 && argv[1] && argv[1][0]` 이면 `dir`(끝 `/`·`\` 보정) + name, 아니면 name). 공용 헤더를 만들면 CMake(`studio/CMakeLists.txt:105` 가 `${t}.cpp` 한 파일만 컴파일)와 JOB-001 파일까지 건드려 diff 가 커진다 — 범위 밖.

## 위험·가정
- **거짓 실패 위험**: `test_dimensions.cpp` 는 쓴 SVG 를 다시 열어 라벨(`구연경`·`저경`·`기고`·`최대동체경`·`100.0`, `:102-104`)과 `y1="-100"`·`y2="0"`(`:130`)을 검사한다. 쓰기와 읽기가 다른 경로를 쓰면 빈 문자열을 읽어 `FAIL svg labels` 로 끝난다 → AC-2 의 "같은 변수" 규칙.
- **cwd 가정**: ctest 는 `WORKING_DIRECTORY` 미지정 시 빌드 폴더에서 실행(`verify.ps1:52` 의 `cd /d $bld && ctest`, CI `studio-ci.yml:64`). 저장소 루트에서 손으로 돌리면 `.gitignore:83-84`(`/karch_t*.svg`, `/karch_t*.dxf`)가 덮는다.
- **병렬 실행**: CI 는 `--parallel 4`. 5개 파일명이 서로 다르고 JOB-001 의 `karch_t8u_cyl.dxf` 와도 겹치지 않아 경합 없음.
- `argv[1]` 이 빈 문자열이면 cwd 로 취급(`std::string::back()` 빈 문자열 호출 방지). 비ASCII 출력 폴더는 미검증(ANSI `std::ofstream`, 기존과 동일).
- 테스트 수 11 vs 12: 이 작업만으로는 11. CI 문구("11 Suites")·CMake 옵션 설명은 범위 밖.
- UI 문자열 없음 → `review-ui.md` 는 `na` 예상. 라이선스·좌표계(mm, Z-up) 영향 없음.

## 크기
예상 변경 줄 수: 추가 ≈ 30줄(헬퍼 3×6 + 시그니처 3 + 호출·선언 6 + include 1 + `.c_str()` 1), 삭제 ≈ 18줄(`test_dxf.cpp` 옛 `outPath` 9줄 포함) → **insertions ≤ 40 / deletions ≤ 25**(AC-6). 쪼갤 필요 없음.
