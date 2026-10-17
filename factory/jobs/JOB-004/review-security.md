# JOB-004 검토: security

결론: **PASS**

검토 대상: worktree `D:/_dongarch_build/wt/.worktrees/JOB-004`, `git diff e9b655a..HEAD`(커밋 `b033aef` 1개, 15파일 +602 −56), `spec.md`(JOB-004a 만 정의, 004b 실시간 단면·004c ② 페이지는 범위 밖), `design-excerpt.md`, `verify.md`, `build-log.md`. 빌드·실행은 하지 않았다(읽기 전용 검토).

요지: 이번 조각은 **그리기 부호(x × ±1)와 문구, 검수용 CLI 플래그 1개, CTest 1종**만 더한다. 새 입력 파서·새 파일 경로 처리·외부 프로세스·네트워크·의존성·비밀값은 없다. 타이머·디바운스(004b)는 아직 들어오지 않았고, 새 테스트에는 시간 측정 판정이 없다. 차단 사유 없음.

| # | 항목 | 심각도 | 판정 | 근거 (file:line) · 시나리오 |
|---|---|---|---|---|
| 1 | 새 CLI 플래그 `--flip` | low | PASS | `studio/src/main.cpp:38-41` — `--shot` 블록 안에서만 동작, 200 ms 뒤 자기 창에 `Qt::Key_R` 합성 키 1회. 경로·인자 값을 읽지 않는다. 타이머 문맥 객체가 `&w` 라 창이 먼저 사라지면 취소된다. `--flip`(200)·`--max`(300)·`--export`(400)·캡처(1800) 순서가 간격으로 고정되어 경쟁 없음. 확정 전(파일 경로 인자)이면 R 이 `requirePose()` 로 막혀 안내만 뜬다(`MainWindow.cpp:838-846`). |
| 2 | 연속 키 입력(R 길게 누름)·재진입 | low | PASS | `MainWindow.cpp:886` → `onAction("combined")`(`:759-766`)는 GUI 스레드에서 동기 처리하고 깃발(`side_`)만 바꾼다. `studio/src` 에 `processEvents`·스레드·`QtConcurrent` 없음(grep 0) → 중첩 이벤트 루프 재진입 없음. 음영 재계산은 `SheetView.cpp:38-41` `docChanged()` 가 dirty 표시만 하고 `paintEvent`(`:87-89`)에서 한 번 계산하므로 자동 반복 키가 쌓여도 그리기가 합쳐진다. 되돌리기 스택에 넣지 않음(`MainWindow.cpp:639-642` ViewState 에 side 없음) — 명세 §3.5 와 일치. |
| 3 | 끌기 중 파일 열기·닫기 | low | PASS(변경 없음) | `ArtifactDoc.h:86-95` 좌/우는 로드가 되돌리지 않는 값이라 재로드 중 불일치 상태를 만들지 않는다. `View3D.cpp:64-77` Left 의 yaw +180° 는 확정 뒤에만 적용되어, 끌기 중 감시 폴더 자동 로드(확정 전으로 돌아감)가 와도 확정 전 화면은 무변경. `View3D.cpp:149-157` 절단선 분할의 `runs.back()` 는 `cont` 가 참일 때만(직전 선분이 같은 벡터에 들어간 경우) 호출 → 빈 벡터 접근 없음, `pts.size()<2` 면 루프 생략. |
| 4 | 확정 전/후 내보내기 | — | PASS | `MainWindow.cpp:715-717` `needPose` 에 `combined` 추가, `export/svg/dxf` 는 기존대로 확정 필요. 내보내기 음영은 캐시가 아니라 매번 `Plate::renderShade(doc, …)` 로 새로 계산(`Plate.cpp:444-446`, `:632-633`)하고 현재 `sectionSide()` 를 읽어(`Plate.cpp:123`, `:637`) 화면과 다른 쪽으로 저장될 여지 없음. `ExportDialog` 는 모달(`MainWindow.cpp:687` `exec()`)이라 열려 있는 동안 R 이 메인 창에 닿지 않는다. |
| 5 | 경로·입력 | — | PASS | 새 사용자 경로 처리 없음. 시험 산출물은 CTest 인자 `${CMAKE_CURRENT_BINARY_DIR}`(`studio/CMakeLists.txt:96`) 아래 고정 이름 `studio_side_{sample,blind}_{Right,Left}.{svg,dxf}`(`test_side.cpp:314`, `:357`) — 기존 `studio_plate.*`·`studio_blind.*`(`test_pipeline.cpp:97-112,171`)와 겹치지 않아 CI `ctest --parallel 4` 에서도 파일 경쟁 없음. SVG/DXF 파서는 시험이 방금 쓴 파일만 읽는다. diff 에 절대 경로(`D:\`, `A:\1105`, `/media/kwon`) 0건. 음영 영상 크기는 기존 4000 px 상한(`Plate.cpp:119`) 그대로. |
| 6 | 외부 실행·네트워크 | — | PASS | diff 의 `+` 줄에 `QProcess`·`system(`·`ShellExecute`·`QDesktopServices`·`QNetwork`·`http(s)`·`curl` 0건(grep 종료 코드 1). |
| 7 | 라이선스·비밀값·대용량 | — | PASS | 새 의존성·외부 코드 조각 없음(`test_side.cpp` 는 Qt·표준 라이브러리만). Artec SDK/Studio 자료·바이너리·5 MB 초과 파일 없음(15파일 모두 텍스트). 토큰·키·비밀번호 문자열 0건. `GigaMesh/core`·`external`·`karch` 무변경. |
| 8 | 한글 문자열 `/utf-8` | — | PASS | `studio/CMakeLists.txt:15-16` `if(MSVC) add_compile_options(/utf-8 …)` 가 디렉터리 전체(새 `test_studio_side` 포함)에 적용. 새 한국어 문자열은 모두 `QStringLiteral`. |
| 9 | CMake·CI 영향 | low | PASS | `studio/CMakeLists.txt:94-96` 실행 파일·`add_test` 3줄뿐, 옵션·플래그 변경 없음. `.github/workflows/*` 무변경. 새 시험은 **시간 판정이 없다**(`test_side.cpp` 전체에 타이머·시간 비교 없음, verify.md "시간 측정 — 해당 없음") → CI 러너 속도 흔들림으로 실패할 경로 없음. 로컬 11.7 s, CI 잡 제한 15분(`studio-ci.yml:33`)·CTest 기본 시험 제한 1500 s 대비 여유 큼. 오프스크린을 시험 안에서 강제(`test_side.cpp:390`). 래스터 판정(기준 8)은 반대쪽 0 화소를 요구하지만 여유 5 px(선 두께 ≈1.4 px)·측정값 23,105/227,754 로 CI Qt 6.6.3 렌더 차이에도 견딜 수준. |
| 10 | 비밀 노출·파괴적 스크립트 | — | PASS | 워크플로·`factory/scripts/*.ps1` 무변경(`git diff --stat e9b655a..HEAD -- .github factory` 출력 없음). |
| 11 | 정본(CLAUDE.md·UI_RULES) 변경 | — | PASS(사람 승인 필요) | `CLAUDE.md:10`, `docs/ui/UI_RULES.md:7`, `:73`, `:84`, `:107` — 에이전트 작업 규칙 파일 수정이지만 내용은 D5 배치 문구뿐이며 권한·검증 절차를 완화하는 문장은 없다. CLAUDE.md §1 은 "바꾸려면 먼저 사용자 승인" 대상 → 사람 검토 항목. |

## 고치지 않아도 되는 제안
- **R 자동 반복 무시**: `MainWindow.cpp:886` 에서 `e->isAutoRepeat()` 면 무시하면, R 을 길게 누를 때 좌/우가 홀짝으로 깜빡이다 아무 쪽에서 멈추는 일이 없어진다(안전 문제 아님, 재진입 위험 없음).
- **`--flip` 을 첫 인자로 줄 때**: `main.cpp:31-35` 는 첫 인자가 `--sample/--blind/--home/--shot…` 이 아니면 파일 경로로 보고 `openPath("--flip")` 을 불러 오류 상자가 뜬다(캡처가 그 상자를 찍음). `--max`·`--export` 도 같은 기존 동작이라 이번 결함은 아님 — 나중에 `args[1].startsWith("--")` 면 경로로 보지 않게 하면 된다.
- **004b 시간 기준을 CI 에서 다루는 법(미리 적어 둠)**: 명세 §후속 1 의 "LOD 미리보기 51회 중앙값 ≤ 16 ms" 를 CTest 합격 조건으로 넣으면 `windows-latest` 공유 러너(vCPU 2~4, `ctest --parallel 4` 동시 실행)에서 흔들릴 수 있다. 권고: 그 시험에 `set_tests_properties(... PROPERTIES RUN_SERIAL TRUE)`, CI 에서는 측정·인쇄만 하고 판정은 환경 변수(예: `STUDIO_PERF_STRICT=1`, 이 PC 의 verify.ps1 에서만 켬)로 켜거나 CI 배수(예: ×4)를 둔다. 판정 기준과 측정 PC 를 명세에 함께 적는다.
- **CI 단계 이름**: `studio-ci.yml:31,60,63` 의 "12 Suites"·"11 K-Arch + Headless Studio Pipeline" 는 이제 14종과 다르다(이번 JOB 이 만든 차이 아님, 표시만).

## 확인 필요
- **(기존 동작, 이번 diff 밖) 모달 내보내기 중 감시 폴더 자동 로드**: `MainWindow.cpp:687` `dlg.exec()` 동안에도 이벤트 루프가 돌아 `watchTimer_`(`:70-75`) → `checkWatchFolder` → `openPath`(`:420`, `:426-`)가 `doc_` 를 다시 읽을 수 있다. `ExportDialog` 는 `const ArtifactDoc&` 를 쥐고 있어 그 뒤 저장하면 확정 전(또는 다른 유물) 문서로 내보낼 수 있다. JOB-004 가 만든 것도 악화시킨 것도 아니며(좌/우는 로드에 영향받지 않음), 미확인 — 004b(끌기 중 미리보기·"미리보기 중 내보내기 거부")를 명세할 때 함께 막는 것을 권한다(예: `activeModalWidget()` 이 있으면 감시 로드를 미룸).
- CI(Qt 6.6.3, AVX2·LTO)에서 `studio_side` 를 실제로 돌려 본 기록은 없다(이 PC Qt 6.11.1 에서만 통과). 사용한 API 는 6.6 에 있는 것들이라 컴파일 문제는 예상하지 않음 — 미확인.

## 사람 검토 항목
1. `CLAUDE.md:10` 과 `docs/ui/UI_RULES.md` R5(`:73`)·머리말 예외(`:7`)·R6(`:84`)·체크리스트(`:107`) 변경 — CLAUDE.md §1 "바꾸려면 먼저 사용자 승인" 대상. 내용은 D5 배치 문구뿐(권한·검증 완화 없음)임을 확인했으나 승인은 사용자 몫.
2. `R` 의 의미 변경(정면각 +180° → 좌/우 거울상)과 세션 유지·되돌리기 제외 — 이미 내보낸 옛 배치 SVG/DXF 는 그대로 남고 다시 내보내면 D5 배치가 됨(명세 §7). 사용자 흐름 변화로 기록.
