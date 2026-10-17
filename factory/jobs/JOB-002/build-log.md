# JOB-002 구현 기록 (builder)

- 작성: 2026-10-17, builder. 작업 트리 `D:/_dongarch_build/wt/.worktrees/JOB-002`(브랜치 `factory/JOB-002`, 기준 d74ae33).
- 목표: K-Arch 테스트 3종이 산출 SVG/DXF 를 `argv[1]` 폴더, 없으면 cwd 상대 경로에만 쓰게 하여 `D:\gigamesh` 절대 경로(CI run 39010516345 실패 원인)를 제거한다. 제품 코드·검증 논리·허용 오차·파일명은 바꾸지 않았다.

## 바꾼 파일
| 파일 | 내용 | 이유 |
|---|---|---|
| `GigaMesh/gui/src/dongarch/karch/test_section.cpp` | `#include <string>` 추가. `main()`→`main(int argc, char** argv)`. 7줄짜리 `static std::string outPath(int argc, char** argv, const char* name)` 헬퍼 추가. `WriteDrawingSvg("D:\\gigamesh\\karch_t5_cyl.svg", …)` → `WriteDrawingSvg(outPath(argc, argv, "karch_t5_cyl.svg"), …)` | 러너에 `D:\gigamesh` 가 없어 `FAIL svg` 로 실패하던 지점. `WriteDrawingSvg(const std::string&)`(SectionComposer.h:37) 이라 `std::string` 직접 전달. 이 파일은 되읽기 없음 |
| `GigaMesh/gui/src/dongarch/karch/test_dimensions.cpp` | 같은 헬퍼 추가, `main` 시그니처 변경. `const char* svgPath = "D:\\gigamesh\\karch_t6_cyl.svg"` → `const std::string svgPath = outPath(argc, argv, "karch_t6_cyl.svg")`, `svgShift` 도 동일(`karch_t6_aligned.svg`). printf `%s` 인자 `svgPath` → `svgPath.c_str()` | `FAIL write svg` 실패 지점. `std::ifstream in(svgPath)`·`in2(svgShift)` 가 쓰기와 **같은 변수**를 그대로 쓰므로 쓰기·읽기 경로 일치(라벨·`y1="-100"` 검사가 빈 문자열을 읽는 거짓 실패 방지). `%s` 에 `std::string` 을 넘기면 UB 라 `.c_str()` |
| `GigaMesh/gui/src/dongarch/karch/test_dxf.cpp` | 옛 `outPath(const char*)`(`D:\gigamesh` 를 `std::ofstream(app)` 으로 probe 한 뒤 폴백, 9줄) 를 새 헬퍼(7줄)로 교체. `main` 시그니처 변경. `outPath("karch_t8_cyl.dxf")`/`outPath("karch_t8_frag.dxf")` 에 `argc, argv` 전달 | probe 가 개발 PC 의 메인 체크아웃 루트에 파일을 남기던 부작용 제거. `slurp(dxfPath)`·`slurp(fragPath)`·`dxfPath.c_str()` 는 이미 변수 기반이라 불변. `<fstream>` 은 `slurp` 이 계속 쓰므로 유지 |

헬퍼 본문(세 파일 동일, JOB-001 `test_dxf_units.cpp:490-499` 와 같은 규칙):
```cpp
static std::string outPath(int argc, char** argv, const char* name) {
    std::string dir = (argc > 1 && argv[1] != nullptr) ? argv[1] : "";
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') {
        dir += '/';
    }
    return dir + name; // argv[1] = output dir, else relative to CTest cwd (build folder)
}
```
- `argv[1]` 이 빈 문자열이면 `dir.empty()` 로 cwd 처리(`std::string::back()` 빈 문자열 호출 없음). 끝 `/`·`\` 는 보정.
- 공용 헤더로 빼지 않고 파일마다 복제: `studio/CMakeLists.txt:105` 가 `${t}.cpp` 한 파일만 컴파일하고, 헤더를 만들면 CMake·JOB-001 파일까지 diff 가 번진다(명세 §영향 파일).

## 실행한 명령과 종료 코드
| 명령 | 종료 코드 | 비고 |
|---|---|---|
| Python 3.12(`C:\Users\kyi25\AppData\Local\Programs\Python\Python312\python.exe -I`) 로 정확 문자열 치환 스크립트 실행(각 대상 문자열이 정확히 1회 존재함을 assert) | 0 | Git Bash heredoc 이 `\\` 를 반으로 줄여 1차 시도는 assert 에서 중단(파일 미변경), Write 도구로 스크립트를 다시 써서 성공. UTF-8·LF 유지 |
| `git diff --stat` / `--shortstat` (커밋 전) | 0 | 3 files, +31 / −16 |
| `grep -nE "[A-Za-z]:[\/]\|gigamesh" GigaMesh/gui/src/dongarch/karch/test_{section,dimensions,dxf}.cpp` | **1** (0건) | AC-1 |
| `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Root D:\_dongarch_build\wt\.worktrees\JOB-002 -BuildRoot D:\_dongarch_build\verify_job002 -NoShots` | 0 | configure 0(5s) / build 0(17s) / ctest 0(10s) → **PASS**. `03_ctest.log:25` `100% tests passed out of 11`. `vswhere.exe is not recognized` 2줄은 vcvars64.bat 가 내는 기존 잡음(결과 무관) |
| `cd /d/_dongarch_build/verify_job002/studio && ctest --output-on-failure` (PATH 에 Qt6 bin, `QT_QPA_PLATFORM=offscreen`) | 0 | **11/11 Passed**(studio_pipeline 7.81s, karch_test_section 0.07s, karch_test_dimensions 0.38s, karch_test_dxf 0.16s). AC-3 |
| `ls -l karch_t5_cyl.svg karch_t6_cyl.svg karch_t6_aligned.svg karch_t8_cyl.dxf karch_t8_frag.dxf` (빌드 폴더) | 0 | 5개 모두 존재(7768 / 8533 / 10414 / 39167 / 39231 bytes). AC-2(a) |
| `mkdir -p ../argtest && ./test_section.exe ../argtest && ./test_dimensions.exe ../argtest && ./test_dxf.exe ../argtest` | 0 | 세 exe 모두 종료 0, stdout 에 `svg ../argtest/karch_t6_cyl.svg`·`dxf ../argtest/karch_t8_cyl.dxf` 출력. `argtest/` 에 같은 5개(count=5). AC-2(b) |
| `grep -ci warning D:\_dongarch_build\verify_job002\studio-logs\02_build.log` | 0건 | 세 테스트 모두 이번 빌드에서 재컴파일됨(`02_build.log:12,13,15`). `build.ninja` FLAGS 에 `/W4` 확인 |
| `git add <3파일> && git -c user.name=kesekikwon-netizen -c user.email=kesekikwon@gmail.com commit -F <msg>` | 0 | 아래 커밋 절 |
| `git diff --name-only d74ae33..HEAD` | 0 | 정확히 3개(`test_dimensions.cpp`, `test_dxf.cpp`, `test_section.cpp`). AC-5 |
| `git diff --shortstat d74ae33..HEAD` | 0 | `3 files changed, 31 insertions(+), 16 deletions(-)` → ≤40 / ≤25. AC-6 |

### 수용 기준 자체 확인
| AC | 결과 | 근거 |
|---|---|---|
| 1 정적 | 통과 | grep 0건(종료 1) |
| 2 경로 규칙 | 통과 | `main(int argc, char** argv)` 3종, 헬퍼 규칙 동일, 파일명 불변, 되읽기 변수 동일(코드 리뷰), (a) 빌드 폴더 5개, (b) argtest 5개·종료 0 |
| 3 회귀 없음 | 통과 | verify PASS, 11/11 |
| 4 메인 체크아웃 불변 | **검증자 확인 필요** | builder 지시에 따라 `D:\gigamesh` 는 읽기도 하지 않았으므로 `ls` 스냅샷을 찍지 않았다. 코드에서 `D:\gigamesh` 참조가 모두 사라졌고 모든 실행 cwd 가 `D:\_dongarch_build\verify_job002` 아래였으므로 그 루트에 쓸 경로가 없다 |
| 5 변경 범위 | 통과 | 3파일 |
| 6 크기 | 통과 | +31 / −16 |

## 남은 경고
- 없음(`02_build.log` warning 0건, `/W4 /permissive-` 하에서).

## 제안 (범위 밖, 하지 않음)
- `studio-ci.yml:60,63` 의 "11 Suites" 문구는 JOB-001(`test_dxf_units`) 병합 뒤 12 가 되므로 그때 함께 갱신.
- 세 파일에 복제된 `outPath` 헬퍼와 JOB-001 의 것을 `karch/tests/TestPaths.h` 같은 공용 헤더로 합치려면 `studio/CMakeLists.txt` 의 `add_executable(${t} ${KARCH_DIR}/${t}.cpp)` 에 include 경로를 더해야 한다 — 별도 작업으로.
- `.gitignore:83-84`(`/karch_t*.svg`, `/karch_t*.dxf`) 는 저장소 루트에서 손으로 테스트를 돌릴 때만 의미가 있고, 이제 CTest 는 빌드 폴더에만 쓴다. 유지해도 무해.
- `vcvars64.bat` 가 내는 `vswhere.exe is not recognized` 잡음은 verify.ps1 로그 가독성을 위해 PATH 보강 또는 필터 고려.

## 커밋
- `factory/JOB-002` **32d5d8505636b056df9a85f03493036cdbc52747** — `test(karch): 테스트 산출 경로를 argv[1]/cwd 로 — D:\gigamesh 절대 경로 제거 (JOB-002)` (작성자 kesekikwon-netizen <kesekikwon@gmail.com>, 본문 한국어: CI 실패 원인·변경 요지·검증 결과, 끝 두 줄 `🤖 Generated with Claude Code` / `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`).
- push·병합하지 않음(`CLAUDE.md` §4-4). 작업 트리 clean.

## 차단 사유 (있으면)
- 없음.
