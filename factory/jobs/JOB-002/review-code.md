# JOB-002 검토: code (code-reviewer, 2026-10-17)

결론: **PASS**

대상 `factory/JOB-002` 커밋 32d5d85(부모 d74ae33), 작업 트리 clean. 줄 번호는 변경 후 기준. 빌드·테스트 미실행(test-runner 몫), `verify.md`·다른 검토 파일은 읽지 않음.

| # | 항목 | 판정 | 근거 |
|---|---|---|---|
| 1 | 명세 범위 | pass | 변경 파일 정확히 3개(AC-5), +31/−16(AC-6). hunk 는 `#include <string>`(section:10), `outPath` 헬퍼, `main(int argc, char** argv)`, 경로 인자 치환, `.c_str()`(dimensions:116) 뿐. 검증 논리·허용 오차·기대 문자열·파일명은 전부 무변경(section:74 `0.005`, :78 `0.1`, :112 `0.2`, :123 `0.01`; dimensions:47, :51-54, :78, :110-112, :138; dxf:65-91, :104) |
| 2 | 금지 구역 | pass | core/external 변경 0 |
| 3 | 규약 | pass | UI 문자열·색상 없음, 커밋 제목 `test(karch): …(JOB-002)` 형식·한국어 본문·서명 2줄. 정적 grep `[A-Za-z]:[\/]|gigamesh` 0건(exit 1) |
| 4 | 구조 | pass | 헬퍼 7줄, 세 파일 md5 동일(section:48-54 = dimensions:69-75 = dxf:34-40). 중복은 명세가 허용(`CMakeLists.txt:105` 단일 파일 컴파일). 옛 `D:\gigamesh` probe·`ios::app` 삭제, `<fstream>`은 `slurp`에 여전히 필요 |
| 5 | 테스트 | pass | CTest 등록 불변. 절대 경로 쓰기 제거. 되읽기 변수 일치(dimensions `svgPath` :103→:108, `svgShift` :131→:136; dxf `dxfPath` :58→:64, `fragPath` :97→:103). 실행 판정(AC-2(a)(b)·3·4)은 test-runner 로 확정 |
| 6 | 문서 | pass(해당 없음) | 사용자 가시 동작 변화 없음, `karch_t*` 언급은 `.gitignore:83-84` 뿐 |

집중 점검: 끝 구분자(`/`·`\`) 처리 정상, 빈 `argv[1]` 안전(`dir.empty()` 가 `back()` 보호), JOB-001 헬퍼와 텍스트는 다르나 의미 동일(4경우 결과 일치), 호출 대상 3함수 모두 `const std::string&`.

## 고치지 않아도 되는 제안(범위 밖)
1. JOB-001·JOB-002 병합 뒤 `outPath` 4벌 → `karch_test_common.h` + `CMakeLists.txt:105` 수정(별도 JOB).
2. 쓰기 실패 메시지에 경로 포함(dimensions:105, section:130, dxf:60) — 기대 문자열 변경이라 범위 밖.
3. `.gitignore:83-84` 유지 무해. 4. 영문 주석은 기존 관례.

## 확인 필요
- AC-2(a)(b)·AC-3·AC-4 실행 판정 → test-runner verify.md 로 확정(결과: 전부 통과).
- `argv[1]` 분기는 CTest 가 인자 없이 실행하므로 수동 실행으로만 확인됨 — 후속 JOB 에서 `add_test(... ${t} <out dir>)` 로 흡수 가능.
- CI 라벨 "11 Suites"(`studio-ci.yml:31`) 는 JOB-001 병합 후 12 로 불일치 — 기억만.
