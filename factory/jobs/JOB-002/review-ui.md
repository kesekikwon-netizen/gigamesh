# JOB-002 검토: ui (ui-reviewer, 2026-10-17)

결론: **해당 없음(PASS)**

범위 `d74ae33..HEAD` = 32d5d85, 3파일 +31/−16. 변경은 산출 파일 경로 처리뿐(헬퍼 3곳, `main(argc, argv)`, `D:\gigamesh` 리터럴 4건·probe 삭제, `.c_str()`). 제품 코드(`Theme.h`, `studio/src/*`, `DxfWriter.*`, `SectionComposer.*`, `DimensionAnalyzer.*`, `Silhouette.*`, core/external) 무변경.

| 규칙 | 판정 | 근거 |
|---|---|---|
| R1 화면 구조 | 해당 없음 | UI 코드 변경 0. 캡처 `shot_main.png` 구조 유지 |
| R2 색상 토큰 | 해당 없음 | diff 에 hex/QColor 0건. 캡처상 `Action` 색은 화면당 1곳 |
| R3 도구 3단계 | 해당 없음 | 미변경 |
| R4 글꼴 | 해당 없음 | 미변경 |
| R5 도판·D5 좌/우 | 해당 없음(**D5 중립 유지**) | 작성기 호출 인자는 첫 인자(경로)만 변경: `test_section.cpp:129`, `test_dimensions.cpp:104/132`, `test_dxf.cpp:59/98`. 입력 메시·검증 논리 변경 없음 |
| R6 내보내기 결과 | 해당 없음(내용 동일) | `artifacts/build-out/` ↔ `argtest-out/` 5쌍 **md5 전부 동일**. DXF `$ACADVER AC1009`, `$INSUNITS 4`, 11 레이어 순서 그대로. SVG 라벨 `기고·최대동체경·구연경·저경` 한국어 유지 |
| R7 홈 화면 | 해당 없음 | 미변경 |
| R8 문구 / §4-2 | 해당 없음 | 사용자 가시 문자열 변경 0. 바뀐 리터럴은 경로뿐, `fprintf/printf` 포맷 문자열은 기준↔HEAD 동일 |
| 오프스크린 캡처 | 기록 | 한글 □(폰트 문제), UI 코드가 기준과 같아 참고용 |

## 고치지 않아도 되는 제안(범위 밖, 별도 작업 후보)
- **기존 사항**: K-Arch 테스트 작성기(`WriteDrawingSvg`/`WritePlateSvg`)의 SVG 는 `#111/#444/#1a5/#c00/#f4e6c3` 하드코딩 색, `width/height` mm 단위 없음, `<g id>` 레이어 없음, `font-family="sans-serif"` → R2·R6·R4 와 다름. JOB-002 가 만든 차이가 아님(md5 동일). 이 작성기가 제품 경로인지 테스트 전용인지(조사 결과: Studio 는 `Plate.cpp` 로 재구현, karch 작성기는 사실상 테스트 전용)에 따라 별도 JOB 으로 정리 권고.
