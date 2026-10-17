결론: PASS

# JOB-003 검토: security (보안·안전)

- 검토자: security-reviewer (읽기 전용, 제품 코드·빌드 무실행)
- 대상: `D:/_dongarch_build/wt/.worktrees/JOB-003` `factory/JOB-003` HEAD `e9b655a` (커밋 `1ad49b4`·`e9b655a`), 기준 `58a33ae`
- 입력: `git diff 58a33ae..HEAD`(15파일 +821/−109, 전부 텍스트), spec.md(기준 6 → 6a/6b, decision.md), design-excerpt.md, verify.md(추기 포함), build-log.md
- 요약: 새 CLI 플래그 `--base` 는 불 값 하나만 바꾸고, 확정 전 상태에서 내보내기·저장 경로는 모두 막혀 있다. 새 파서·외부 실행·네트워크·비밀값·바이너리·새 의존성은 없다. 파일을 열 때 하는 일이 줄어 공격면은 오히려 작아졌다(자동 세우기를 열 때 하지 않음). 고쳐야 할 결함(blocking) 없음.

## 항목별 판정

| # | 항목 | 판정 | 심각도 | 근거 (file:line) |
|---|---|---|---|---|
| 1 | 새 CLI 플래그 `--base` 파싱 | PASS | low(관찰) | `studio/src/main.cpp:28` `args.contains("--base")` 로 bool 하나만 만들고 `:30-31` 에서 `openSample/openArtecBlindSample(baseStage)` 에 넘긴다. 경로·숫자 파싱, 파일 쓰기 없음. 관찰: `--base` 를 첫 인자로 잘못 주면 `:32` 에서 경로로 취급 → `MainWindow.cpp:425` `QMessageBox::warning` 모달 → `--shot` 타이머(`main.cpp:34-` 이후 등록)가 걸리기 전에 막혀 무인 캡처가 CI 시간 제한(15분)까지 멈춘다. `--max`·`--export` 등 기존 플래그도 똑같이 동작하던 기존 패턴이라 이번 변경의 새 결함은 아님 |
| 2 | 확정 전 상태에서의 내보내기·저장 | PASS | — | 리본 `export/svg/dxf` 를 포함한 10종이 `needPose` 가드(`MainWindow.cpp:708-710`) → `requirePose()`(`:592-597`). `Ctrl+P` 도 `onAction("export")`(`:816`)로 같은 가드를 거친다. 되돌리기/다시(`:222`, `:227`), 단면·슬롯 단축키(`:823-833`)도 막힘. `openExport`(`:677-679`)를 부르는 곳은 `onAction` 세 곳뿐. 도판 맞춤·그리기(`SheetView.cpp:45`, `:80-86`)와 3D 절단 계산(`View3D.cpp:38`)도 확정 전에는 건너뛴다 |
| 3 | 확정 전 상태가 저장 기록에 남기는 값 | PASS | — | 최근 목록(QSettings)에 확정 전에는 치수 대신 `치수 — (바닥 확정 전)` 을 쓴다(`MainWindow.cpp:440-449`). 같은 경로는 중복 없이 갈아끼운다(`StartPage.cpp:36-37`). 거짓 치수나 새 영구 설정은 쓰지 않음 |
| 4 | 메모리 안전(자세 API·빈 슬롯) | PASS | — | `resetPose` 는 정점 배열만 원본 사본으로 되돌리며(`ArtifactDoc.cpp:433`), 회전·자동 세우기는 정점 좌표만 바꾸고 면 구성은 그대로라(`:517-587`) 정점 수와 면 번호가 어긋나지 않는다. LOD 와 LOD 색은 짝으로 함께 복원. 확정 전 빈 슬롯 목록에서도 슬롯 접근은 모두 범위 검사를 거친다(`MainWindow.cpp:566`, `SheetView.cpp:236`, `View3D.cpp:41,122`, `Plate.cpp:34-36` → `:144,552`). 빈 메시는 `isLoaded()` 조기 반환으로 막고 `updateBbox` 도 빈 배열을 처리(`ArtifactDoc.cpp:63-67`). 새 스레드 없음(grep 0) |
| 5 | 경로·입력 처리 | PASS | — | 새 파서 없음. `loadFile` 은 기존 `readMeshAny` 를 그대로 쓰고 자세만 바꾸지 않는다. 테스트는 `argv[1]`(CTest 가 넘기는 빌드 폴더) 또는 cwd 에만 쓴다(`test_pose.cpp:74,194`). diff 의 추가 줄에 절대 경로(`D:\…`, `A:\1105`, `/media/kwon`)는 없다(grep 0) |
| 6 | 비정상·대형 입력(DoS) | PASS | low | 확정 전 바닥판 방안선 개수가 메시 폭에 비례한다(`View3D.cpp:24`, `:106-111`, 10 mm 당 1개). 멀리 떨어진 정점 하나(참조 안 되는 정점은 부유 노이즈 제거가 일어날 때만 정리됨)가 섞인 비정상 파일이면 다시 그릴 때마다 선 수백만 개 → 화면 멈춤. 기존 도판 방안 루프(`Plate.cpp:159-174`, 1 mm 당 1개)와 같은 종류이고 덜 심하므로 새 위험 등급은 아님. 원본 정점 사본 `rawVertices_`·`rawLod_` 로 메모리가 정점당 24 B 늘어나는 것은 명세 위험란에 적혀 있음 |
| 7 | 외부 실행·네트워크·감시 폴더 | PASS | — | diff 에 `QProcess`·`system(`·`ShellExecute`·`QDesktopServices`·`QNetwork`·URL·`QFile::remove` 없음(grep 0). Artec 감시 폴더 자동 열기는 문구만 바뀌었고(`MainWindow.cpp:414`) 열기 뒤 ① 단계에서 멈춘다. 자동 측정·자동 저장이 없어져 처리가 줄었다 |
| 8 | 라이선스·비밀값·바이너리 | PASS | — | `git diff --numstat` 15줄 모두 텍스트(바이너리 0). 새 라이브러리·외부 코드 조각 없음. triangle 미사용 유지. 토큰·키·Artec 자료 없음. 산출물 폴더는 gitignore(`.gitignore:81` `factory/jobs/*/artifacts/`). 새 파일에 라이선스 머리말이 없는 것은 `studio/` 전체 관례(SPDX 0개)와 같음 |
| 9 | CMake·CI 영향 | PASS | low(관찰) | 새 테스트 등록은 기존 패턴과 같음(`studio/CMakeLists.txt:91-93`). 출력 폴더 = 빌드 폴더, 파일 이름 `studio_pose_tilted.ply` 는 `studio_pipeline` 산출물(`studio_rt.stl`·`studio_plate.*`·`studio_blind.*`)과 겹치지 않아 CI 의 `ctest --parallel 4`(`.github/workflows/studio-ci.yml:65`)에서 경쟁 없음. 실행 8.91 초(verify.md), CI 제한 15분(`:33`). `/utf-8` 은 전역(`CMakeLists.txt:16`)이라 한국어 문자열이 든 새 테스트에도 적용되고, 빌드 경고는 0. CI 단계 이름 `12 Suites`·`11 K-Arch … + Pipeline`(`studio-ci.yml:31,63`)은 이제 13종인데 갱신되지 않음(표시만 다름, 이번 범위 밖) |
| 10 | 수치 보조 함수 | PASS | — | `cosSinDeg`(`ArtifactDoc.cpp:83-95`): 표 번호는 이중 `fmod` 로 [0,3] 범위에 들어가고, NaN 은 cos/sin 쪽으로 빠져 정수 변환 UB 가 없다. 부르는 곳은 내부(±90°·180°·−74° 테스트)뿐 |

## 고치지 않아도 되는 제안
1. **내보내기 이중 방어**: 지금은 `onAction` 의 가드에만 의존한다. 앞으로 다른 곳에서 `openExport` 를 부를 때를 대비해 `MainWindow.cpp:678` 을 `if (!doc_.isLoaded() || !requirePose()) return;` 으로 바꾸면 확정 전 빈 도판(치수 0)을 내보낼 길이 원천 차단된다(한 줄, 명세 영향 파일 안).
2. **바닥판 방안선 상한**: `View3D.cpp:106` 의 `n` 을 상한(예: 400개 넘으면 테두리만 그리거나 간격을 100 mm 로)으로 제한하고 `std::isfinite(f.half)` 를 검사하면 비정상 파일에서도 화면이 멈추지 않는다.
3. **CLI 인자 순서 실수 방어**: `main.cpp:29-32` 에서 `args[1]` 이 `--` 로 시작하는 알 수 없는 플래그면 경로로 열지 않게 하면, `--base --sample …` 처럼 순서를 잘못 쓴 무인 캡처가 경고 창에 막혀 멈추지 않는다(기존 플래그 공통 문제).
4. **CI 후속(factory/CI 변경은 별도 작업)**: `studio-ci.yml` 캡처 단계에 `--sample --base --shot shot_base.png` 를 넣고, 단계 이름의 테스트 수 문구(12/11)를 13 으로 맞춘다.
5. 커밋 본문에 로컬 빌드 폴더 경로(`D:\_dongarch_build\verify_job003`)가 들어가 있다. 비밀값은 아니고 이전 JOB 관례와 같지만, 공개 저장소에 push 할 때는 상대 표현(빌드 폴더 `verify_job003`)으로 줄여도 된다.

## 확인 필요 (미확인)
- **실제 멈춤 문턱**: 제안 2 의 화면 멈춤은 코드 읽기로만 판단했다. 큰 좌표를 가진 정점이 든 파일로 실행해 보지는 않았다(빌드·실행 금지 역할).
- **`std::lround` 의 NaN·무한대 결과(MSVC)**: 바운딩 박스는 첫 정점이 NaN 일 때만 NaN 이 된다(`std::min/max` 규칙). 이때 반복 횟수가 어떻게 되는지는 실행으로 확인하지 않았다.
- **GitHub CI(Qt 6.6.3)에서 `studio_pose`**: push 금지라 실행하지 못했다. 로컬 Qt 6.11.1 결과(13/13)만 근거다.
- **범위 밖 기존 코드 관찰(이번 diff 아님, 백로그 후보)**: `MeshIO.cpp` 에는 NaN·무한대 좌표를 거르는 검사가 없다(`isfinite` grep 0). `removeFloatingNoise` 의 두 번째·세 번째 면 루프(`MeshIO.cpp:781-783`, `:798-803`)는 첫 루프(`:773-778`)와 달리 면 번호 범위를 검사하지 않는다. 앞 단계 로더가 번호를 검증하는지는 확인하지 않았다.

## 사람 검토 항목
1. **홈 「중심축 맞춤」은 한 번 누르면 확정까지 한다**(`MainWindow.cpp:731-738`, 명세 2절). 사람이 누르는 동작이지만 「이 자세로 확정」을 따로 확인하지 않고 측정·도판까지 간다. 명세 의도(예전 동작 보존)대로 둘지 결정이 필요하다.
2. **Artec 감시 폴더 자동 열기가 ① 단계에서 멈춘다**(`MainWindow.cpp:414`). 예전에는 열자마자 도판까지 갱신했다. 처리가 줄어 안전 면에서는 낫지만 작업 흐름이 바뀐다.
3. **기울어진 채 수동 확정하면 구경 미측정**(decision.md, 기준 6a): 도판 치수 띠에 구경이 `—`/0 으로 나온다. 보안 결함은 아니지만, 결과물을 믿을 수 있는지와 관련된다. 기울어 보일 때 안내를 띄우는 일은 후속 조각으로 넘어간 상태다.
