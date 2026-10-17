# JOB-001 검토: security (security-reviewer, 2026-10-17)

결론: **PASS** (high/medium 없음, low 3건 + 참고)

대상: `git diff e7e641b..HEAD` — 신규 `test_dxf_units.cpp`(589줄, 순수 ASCII 21,710 B, 모드 100644), `studio/CMakeLists.txt`(+2/−1). 변경 파일 정확히 2개(AC-8).

| 항목 | 판정 | 심각도 | 근거 / 시나리오 / 권고 |
|---|---|---|---|
| 1 파일 쓰기 경로 | pass | low 1 | 하드코딩 없음(`grep -nE "[A-Za-z]:[\/]|gigamesh"` 0건). `outPath()` L490-499: argv[1] 있으면 그 폴더, 없으면 cwd 상대; 빈 문자열 검사(L491). 비정상 폴더는 `DxfWriter::Open` 실패 → `FAIL write <path>` 후 exit 1(L524-527), 크래시 없음. [low] `std::ios::out|binary`(DxfWriter.cpp:64)라 같은 이름 파일을 묵음 덮어씀 — 수동 실행 때만 해당, 수용. 기본 경로 = 빌드 폴더(verify.ps1 `cd`, CI `working-directory`), `.gitignore:83-84` 가 루트 실행도 덮음. `ctest --parallel 4` 파일명 충돌 없음 |
| 2 파서 안전성 | pass | low 1 | 빈 입력(L529-532, 180-183), 홀수 줄(L180-183) 보고. 정수 `strtol`+끝 포인터(L128-141), **`stoi/stod` 미사용**. 실수 `istringstream`+`locale::classic()`(L144-158). 고정 버퍼는 `snprintf(buf,64)`(L118-122)뿐. 인덱스 경계 전부 선검사(L287/211/348/580). 무한 루프 없음, 메시지 20개 상한. [low] CIRCLE 분기(L430-432) `numOf` 반환값 무시 → 비수치면 r=0 으로 보고(판정은 맞음). 권고: 별도 메시지 |
| 3 정수/부동소수 | pass | low 1 | [low] `parseInt` `long→int` 범위 검사 없음(L139) — MSVC long=32bit 무손실, LP64 에서만 메시지 숫자 잘림. 권고: INT 범위 검사 1줄. size_t↔int 혼합 없음, `replace`(L573) 범위 안전 |
| 4 외부 실행·네트워크 | pass | — | system/QProcess/popen/ShellExecute/CreateProcess/socket/curl/http/getenv/remove 0건. karch 링크 라이브러리도 없음. Qt 무의존 |
| 5 라이선스·독점 자료 | pass | — | 머리말 `(GPLv3+)`(L1, 기존 karch 테스트 10개는 표기 없음 — 첫 준수 사례). artec/token/secret/1105/media/kwon 0건, 바이너리 없음, 신규 제3자 코드 없음 → THIRD_PARTY 갱신 불필요 |
| 6 CMake | pass | — | :13 설명 문자열만, :102 목록 1줄 → 기존 foreach 와 동일 규칙. install/CPack 없음, 플래그 `/utf-8 /W4 /permissive-` 유지. CI 는 karch/** 변경에 반응, `--parallel 4` 포함 |

미확인: 실제 빌드 경고 수(→ test-runner: 0), `D:\_dongarch_build\verify\studio` 의 DXF 는 다른 빌드 폴더(verify_job001)에서 생성됨, LP64 동작.
참고(범위 밖): 기존 `test_dxf.cpp:34-42`, `test_dimensions.cpp:95,123`, `test_section.cpp:120` 의 `D:\gigamesh\` 절대 경로 — 별도 chore 후보.
