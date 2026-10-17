# JOB-002 검토: security (security-reviewer, 2026-10-17)

결론: **PASS** (high/medium 없음, low 정보성만)

대상 `git diff d74ae33..HEAD`(32d5d85, 3파일 +31/−16). 동적 기준(AC-2/3/4)은 규칙상 미실행 → test-runner 결과(PASS)로 확정.

| 항목 | 판정 | 심각도 | 근거 / 시나리오 / 권고 |
|---|---|---|---|
| 1 경로 처리 | pass | low 정보 | 헬퍼 3곳 동일(section:48-54, dimensions:69-75, dxf:34-40). `argc>1 && argv[1]!=nullptr` → `std::string dir`, 빈 문자열은 `dir.empty()`로 `back()` 보호, 문자열 연결만(고정 버퍼 없음). 없는 폴더/권한 없음/파일 경로는 writer 가 false(`SectionComposer.cpp:285-288`, `DimensionAnalyzer.cpp:242-245`, `DxfWriter.cpp:63`) → `FAIL write …` 후 return 1, 크래시 아님. 되읽기는 쓰기 성공 뒤 같은 변수로. `..` 조작은 검증 없음(호출자가 신뢰 경계, CTest 는 인자 없음; 기존 코드는 무조건 저장소 밖 D:\gigamesh 에 썼으므로 개선). 비ASCII 출력 폴더는 미확인(기존과 동일) |
| 2 절대 경로·probe | pass | — | AC-1 grep 0건(exit 1). karch 전체에서 남은 `[A-Za-z]:` 매치는 URL 문자열 3건(경로 아님). `ios::app` probe 블록(기준 `test_dxf.cpp:34-42`, D:\gigamesh 터치 부작용) 통째로 삭제, karch 폴더 `ios::app` 0건 |
| 3 외부 실행·네트워크 | pass | — | 세 파일 0건. include 는 표준 헤더 + karch 뿐 |
| 4 라이선스·비밀값·바이너리 | pass | low 정보 | 1행 주석은 기준과 동일(변경 없음). 참고: `test_*.cpp` 10개 전부 `(GPLv3+)` 표기 없음(기존 관행, 범위 밖). 비밀값·바이너리 없음, `git diff --check` 깨끗, eol=lf |
| 5 CI·CMake 영향 | pass | — | 변경 3파일 M 뿐. `add_test(... COMMAND ${t})` 불변 → cwd 상대. CI `working-directory: studio/build-ci` → 산출물은 빌드 폴더(`**/build*/` 무시). 아티팩트 집합 불변. `/utf-8` 기설정 |
| 6 부작용·.gitignore | pass | low 정보 | 저장소 루트 실행 시 5개 파일명 모두 `/karch_t*.svg|dxf` 로 무시(`check-ignore` 확인), 추적 중인 karch_t* 없음. 저장소 안 `argtest/` 같은 하위 폴더에 쓰면 무시되지 않음(untracked 로 보임) → 후속: `/argtest/` 추가 또는 "빌드 폴더에서 실행" 문서화 |

권고(비차단): `/argtest/` .gitignore 또는 문서화; `test_*.cpp` 10개 GPLv3+ 표기 통일(후속 작업); POSIX 지원 시 `'\'` 구분자 취급을 `_WIN32` 로 한정.
미확인: 비ASCII 출력 폴더 동작. (AC-2/3/4 동적 결과는 test-runner 가 PASS 로 확인.)
