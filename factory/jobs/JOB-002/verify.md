# JOB-002 검증 기록 (test-runner)

결론: **PASS**

- 검증 대상: `D:/_dongarch_build/wt/.worktrees/JOB-002`, 브랜치 `factory/JOB-002`, HEAD `32d5d8505636b056df9a85f03493036cdbc52747`, 기준 `d74ae33b1480fcbd64db9e8bec897033c07c441f`(HEAD 의 직전 커밋). 검증 시작 시 작업 트리 clean, 읽기만 했다.
- 실행 시각: 2026-10-17 15:55:46 ~ 15:59:32 (+0900), 소요 약 4분. verify.ps1 자체 48초(configure 5 / build 18 / ctest 15 / shots 11).

## 환경
- 컴파일러: MSVC 19.44.35228.0 (VS 2022 BuildTools, `C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe`), 생성기 Ninja, CMake 4.4.1 / ctest 4.4.1, `CMAKE_BUILD_TYPE=Release`
- Qt: 6.11.1 (`D:/OSGeo4W/apps/Qt6` = `CMAKE_PREFIX_PATH`). 테스트 실행 시 PATH 앞에 `Qt6/bin`, `QT_PLUGIN_PATH=Qt6\plugins`, `QT_QPA_PLATFORM=offscreen`(verify.ps1:49-51 과 동일)
- 빌드 폴더: `D:\_dongarch_build\verify_job002\studio` — 실행 전 `studio`·`studio-logs`·`argtest` 를 `rm -rf` 한 뒤 처음부터 configure 한 **깨끗한 빌드**. 로그 `D:\_dongarch_build\verify_job002\studio-logs\`
- 셸/OS: Git Bash + `powershell -NoProfile -ExecutionPolicy Bypass`(Windows PowerShell 5.1), Windows 11 Pro 10.0.22631

## 수용 기준별 결과
| 기준 | 명령 | 종료 코드 | 결과 | 근거(로그 줄·산출물 경로) |
|---|---|---|---|---|
| 1 정적 | `grep -nE "[A-Za-z]:[\\/]\|gigamesh" GigaMesh/gui/src/dongarch/karch/test_{section,dimensions,dxf}.cpp` (worktree) | **1** (0건) | 통과 | 출력 없음. 대조: 같은 패턴을 `git show d74ae33:…` 에 적용하면 4건(test_section.cpp:120, test_dimensions.cpp:95·123, test_dxf.cpp:35) → grep 패턴이 실제로 잡아내는지 확인됨 |
| 2 경로 규칙(코드 리뷰) | `git diff d74ae33..HEAD` + HEAD 파일 직접 확인 | — | 통과 | `int main(int argc, char** argv)` 3종(test_section.cpp:56, test_dimensions.cpp:77, test_dxf.cpp:47). `static std::string outPath(int argc, char** argv, const char* name)` 본문이 3파일에서 md5 동일(`57ffd7f3…`): `argc>1 && argv[1]!=nullptr` 이면 dir, 비었으면 cwd, 끝 `/`·`\` 보정. 파일명 5개 불변. **되읽기 변수 일치**: test_dimensions.cpp:103 `svgPath` 쓰기 → :108 `std::ifstream in(svgPath, …)`, :131 `svgShift` 쓰기 → :136 `std::ifstream in2(svgShift, …)`; test_dxf.cpp:58 `dxfPath` 쓰기 → :64 `slurp(dxfPath)`, :97 `fragPath` 쓰기 → :103 `slurp(fragPath)`; test_section.cpp:129 는 되읽기 없음. test_dimensions.cpp:116 printf `%s` 에 `svgPath.c_str()`. test_dxf.cpp 에 옛 `D:\gigamesh` probe·`std::ofstream`·`ios::app` 잔존 0건(grep exit 1) |
| 2(a) 동적 | ctest 뒤 빌드 폴더에서 `ls -l karch_t5_cyl.svg karch_t6_cyl.svg karch_t6_aligned.svg karch_t8_cyl.dxf karch_t8_frag.dxf` | 0 | 통과 | 5개 존재(7768 / 8533 / 10414 / 39167 / 39231 bytes, mtime 15:58:05), 빌드 폴더의 `karch_t*` 는 정확히 이 5개뿐 → `artifacts/08_ac2a_ls.log`, `artifacts/build-out/` |
| 2(b) 동적 | `mkdir -p ../argtest; ./test_section.exe ../argtest; ./test_dimensions.exe ../argtest; ./test_dxf.exe ../argtest` (cwd=`…\studio`) | 0 / 0 / 0 | 통과 | stdout `svg ../argtest/karch_t6_cyl.svg bytes=8533`, `dxf ../argtest/karch_t8_cyl.dxf bytes=39167 frag=39231`. `ls -l ../argtest` 5개(count=5). 빌드 폴더 산출물과 md5 5쌍 모두 동일 → `artifacts/09_ac2b_argtest.log`, `artifacts/argtest-out/` |
| 2 추가(지시) 존재하지 않는 폴더 | `./test_section.exe ../no_such_dir` / `./test_dimensions.exe ../no_such_dir` / `./test_dxf.exe ../no_such_dir` | 1 / 1 / 1 | 통과 — **실패를 보고**, 크래시 아님 | stderr `FAIL svg` / `FAIL write svg` / `FAIL write dxf` 뒤 정상 종료 코드 1(0xC0000005 류 크래시 코드 아님). 폴더를 만들지 않음(사후 `ls -d ../no_such_dir` 없음) → `artifacts/09_ac2b_argtest.log` |
| 2 추가 경계값 | `./test_section.exe ../argtest/`(끝 슬래시), `./test_section.exe ""`(빈 argv[1]) | 0 / 0 | 통과 | 둘 다 `PASS` — 끝 구분자 중복 보정, 빈 문자열→cwd 처리(`std::string::back()` 미호출) 확인 |
| 3 회귀 없음 | `powershell -NoProfile -ExecutionPolicy Bypass -File …\JOB-002\factory\scripts\verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-002 -BuildRoot D:\_dongarch_build\verify_job002` | **0 (PASS)** | 통과 | configure 0(5s) / build 0(18s) / ctest 0(15s) / shots 0(11s). `03_ctest.log:25` `100% tests passed out of 11`(studio_pipeline 12.09s 포함 총 14.93s) → `artifacts/03_ctest.log`, `artifacts/verify-summary-studio.md` |
| 3 재실행 | `cd …\verify_job002\studio && ctest -N` / `ctest --output-on-failure` / `ctest -R 'karch_test_(section\|dimensions\|dxf)' -V` | 0 / 0 / 0 | 통과 | `-N`: Total Tests **11**(`test_dxf_units` 없음 — 기준 d74ae33 트리의 `karch/test_*.cpp` 10개 + studio_pipeline, 명세대로). `--output-on-failure`: 11/11 Passed, 9.73s. `-V`: `Working Directory: D:/_dongarch_build/verify_job002/studio`, 3/3 Passed, stdout `svg karch_t6_cyl.svg bytes=8533`·`dxf karch_t8_cyl.dxf bytes=39167 frag=39231`(상대 경로) → `artifacts/05_ctest_N.log`, `06_ctest_output_on_failure.log`, `07_ctest_karch_V.log` |
| 4 메인 체크아웃 불변 | `ls -l --time-style=full-iso /d/gigamesh/karch_t*` 를 검증 전(15:55:46)·모든 실행 후(15:59:06) 저장, `diff snap_before.txt snap_after.txt` | **0** | 통과 | `D:\gigamesh` 루트에는 수정 전 코드가 남긴 5개(mtime 2026-10-17 15:14:02~03)가 있었고, 사후에도 크기·mtime 전부 동일, 새 파일 없음, diff 출력 없음 → `artifacts/snap_before.txt`, `snap_after.txt`. `D:\gigamesh` 는 이 `ls` 두 번 외에 접근하지 않음 |
| 5 변경 범위 | `git -C D:/_dongarch_build/wt/.worktrees/JOB-002 diff --name-only d74ae33..HEAD` | 0 | 통과 | 정확히 3개: `GigaMesh/gui/src/dongarch/karch/test_dimensions.cpp`, `…/test_dxf.cpp`, `…/test_section.cpp` |
| 6 크기 | `git diff --shortstat d74ae33..HEAD` | 0 | 통과 | `3 files changed, 31 insertions(+), 16 deletions(-)`(numstat: dimensions 12/4, dxf 8/10, section 11/2) → ≤40 / ≤25 |

- 경고 수: `grep -c "warning" studio-logs/02_build.log` = **0**(`grep -ci` 도 0). 세 테스트는 이번 깨끗한 빌드에서 컴파일·링크됨(`02_build.log:9,13,14` 컴파일, `:27,33,34` 링크).

## 산출물 (artifacts/, 커밋하지 않음 — `.gitignore:81 factory/jobs/*/artifacts/`)
- `01_configure.log`, `02_build.log`, `03_ctest.log`, `04_shots.log`(빈 파일, 단계 exit 0), `verify-summary-studio.md` — verify.ps1 산출
- `05_ctest_N.log`, `06_ctest_output_on_failure.log`, `07_ctest_karch_V.log` — 빌드 폴더로 `cd` 후 재실행한 ctest
- `08_ac2a_ls.log`, `09_ac2b_argtest.log` — AC-2 동적 판정(no_such_dir·끝 슬래시·빈 문자열 포함)
- `snap_before.txt`, `snap_after.txt` — AC-4 스냅샷
- `build-out/` 5개(ctest 가 빌드 폴더에 쓴 것), `argtest-out/` 5개(`<exe> ../argtest` 산출) — 쌍별 md5 동일
- `shot_home.png`, `shot_main.png`, `shot_export.png` — verify.ps1 shots 단계(지시 범위 밖, 참고용)

## 비고
- builder `build-log.md` 의 주장(verify PASS, 11/11, +31/−16, 경고 0, AC-2(b) 5개)과 전부 일치. builder 가 "검증자 확인 필요"로 남긴 AC-4 를 이번에 확인(diff exit 0).
- verify.ps1 의 `-Job JOB-002` 옵션은 쓰지 않았다: `verify.ps1:81` 이 `$Root\factory\jobs\…\artifacts`, 즉 **검증 대상 worktree 안**으로 복사하므로 읽기 전용 원칙에 어긋난다. 대신 `D:/_dongarch_build/wt/factory/jobs/JOB-002/artifacts/` 로 직접 복사했다.
- verify.ps1 콘솔에 `vswhere.exe … 인식되지 않습니다` 2줄(vcvars64.bat 잡음, 각 단계 exit 0, JOB-001 때와 동일)과 Git Bash 를 거친 한글 깨짐이 보이나, 로그 파일과 `verify-summary.md` 자체는 UTF-8 로 정상.
- 두 cwd(빌드 폴더 / `../argtest`)의 산출물이 바이트 단위로 같아(md5 동일) 출력 경로 변경이 내용에 영향을 주지 않음을 덤으로 확인.
- 제품 코드·테스트 코드·git 상태를 건드리지 않았다(worktree 는 읽기만, `D:/gigamesh` 는 `ls` 만). 빌드 폴더 `D:\_dongarch_build\verify_job002` 는 재현용으로 남겨 둠.
