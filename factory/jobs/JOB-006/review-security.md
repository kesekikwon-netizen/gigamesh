결론: PASS

# JOB-006 검토: security (보안·안전)

검토 대상: `git -C D:\_dongarch_build\wt\.worktrees\JOB-006 diff 804f60d..HEAD` (커밋 `eee6dd8`, `688b7bd`, 14개 파일 +714/−57). diff 는 직접 읽었고, build-log.md 는 참고하지 않았다. 레지스트리는 읽기 전용 `reg query` 로만 확인했다.

## 요약

차단 사유는 없다. 입력 파서, 외부 실행, 네트워크, 의존성은 바뀌지 않았고 경로 하드코딩도 없다. 새 입력 경로(탭 전환·잠김, 오른쪽 단추 끌기, `--cut`)에서 크래시, 무한 재귀, 데이터 손상 경로는 찾지 못했다. 아래에 low 4건과 info 1건을 적는다.

## 항목별 판정

| # | 항목 | 판정 | 근거 (file:line) |
|---|---|---|---|
| 1 | 입력 파서 (PLY/OBJ/STL/DXF/SVG) | 해당 없음 | `MeshIO.*`·`ArtifactDoc.*`·`MeshRaster.*` 무변경(diff 파일 목록 14개 = 명세 허용 목록). 시험의 예제는 합성 메시(`ArtifactDoc.cpp:270-282`)라 파일을 읽지 않음 |
| 2 | 경로 하드코딩 | 통과 | diff 의 `+` 줄에 `[A-Za-z]:\`, `/media/`, `/home/`, `OSGeo4W`, `_dongarch_build`, `Users` 가 없음. 시험 인자는 `${CMAKE_CURRENT_BINARY_DIR}`(`studio/CMakeLists.txt:109`) |
| 3 | 저장소·출력 폴더 밖 쓰기 | low (아래 L1) | 시험이 `HKCU\Software\DongArch3DTest\DongArch3DTest\window\geometry` 를 남김. 사용자 키 `Dongguk\DongArch3D` 는 건드리지 않음 |
| 4 | 외부 실행·네트워크 | 통과 | diff 에 `QProcess`/`system(`/`ShellExecute`/`QDesktopServices`/`QNetwork`/`QUrl`/`http` 없음 |
| 5 | 감시 폴더 자동 로드 | 통과 | 시험은 조직 이름을 `DongArch3DTest` 로 둔다(`test_cutpage.cpp:377-378`). 그래서 `MainWindow.cpp:151-152` 가 사용자의 `artec/watchDir` 를 읽지 않고, 시험 중 사용자 감시 폴더의 파일이 자동으로 열릴 수 없다 |
| 6 | 의존성·라이선스·비밀값·대용량 | 통과 | 새 라이브러리 없음(Qt6 Widgets 기존). 가장 큰 파일은 `test_cutpage.cpp` 24 KB, 바이너리 없음. 토큰·키 문자열 없음. Artec SDK 헤더·DLL·자료 없음 |
| 7 | CI·스크립트 | 통과 (low 제안 L4) | `.github/**`·`factory/scripts` 무변경. `RUN_SERIAL TRUE` 있음(`CMakeLists.txt:110`) |
| 8 | `/utf-8` (한글·🔒·①② 문자열) | 통과 | `studio/CMakeLists.txt:16` 의 `add_compile_options(/utf-8 …)` 가 전역이라 새 `test_studio_cutpage` 와 `CutPageText.h` 에도 걸린다. 독립 검증 빌드 로그 `artifacts/02_build.log` 의 warning 0건 |

## 새 입력 경로 점검

### 탭 전환·잠김·재진입 (`onPage` 안에서 탭 되돌리기) — 통과
- `Ribbon::setCurrentPage` 는 `emit pageChanged(i)` 를 함수 **맨 끝**(`Ribbon.cpp:332`)에서 부른다. 받는 쪽이 안에서 다른 페이지로 옮겨도, 신호를 보낸 쪽이 나중에 상태를 덮어쓰지 않는다.
- 잠긴 ② 를 누르는 흐름은 `onPage(pCut_)` → `requirePose()` → `setCurrentPage(pBase_)` → 안쪽 `onPage(pBase_)` 다(`MainWindow.cpp:701-706, 725-740`).
  - 안쪽 호출은 `p != pCut_` 라 `requirePose` 에 다시 들어가지 않는다. 재귀 깊이는 최대 2이고 루프가 없다.
  - 안쪽 `flushCut()` 은 바깥에서 이미 확정했으므로 아무 일도 하지 않는다.
- `connect(pageChanged → onPage)` 는 `buildRibbon` 안에서 처음 `setCurrentPage(pHome_)`(`MainWindow.cpp:288`) 다음에 걸린다. 그래서 생성 중에 `onPage` 가 불리지 않는다. 이때 `cutTimer_` 는 아직 null(생성자 ≈124행 이후 생성)이지만, `cut_` 가 비활성이라 `flushCut` 이 타이머에 닿지 않는다.
- `openSample`·`openPath` 가 새로 일으키는 `onPage` → `refreshAll(false)` 는 `sheet_->canvas()->setDoc` 보다 먼저 돈다. 그래도 같은 `&doc_` 포인터이고 그리기는 이벤트 루프로 미뤄지므로 낡은 상태를 읽어 생기는 크래시는 없다. 앞서 `cancelCut()` 을 불러 이전 문서의 제스처가 새 문서에 적용되지도 않는다.
- `X/Y 막`(`axX`/`axY`)의 원본 계산(≈1.4 s)은 UI 스레드에서 동기로 돈다. `processEvents` 가 없어 계산 중 재진입은 없다.

### 오른쪽 단추 끌기 — low (L2)
### `--cut` 플래그 파싱 — low (L3)

## 발견 사항

### L1 (low) 통합 시험이 레지스트리에 키를 남긴다
- 근거: `studio/tests/test_cutpage.cpp:371` 의 `w.close()` 가 `MainWindow::closeEvent`(`studio/src/MainWindow.cpp:1097`)를 거쳐 `QSettings().setValue("window/geometry", …)` 를 부른다. 조직·앱 이름이 `DongArch3DTest`(`test_cutpage.cpp:377-378`)라서 Windows 기본 형식(레지스트리)의 `HKCU\Software\DongArch3DTest\DongArch3DTest\window\geometry` 에 기록된다. 이 PC 에서 `reg query HKCU\Software\DongArch3DTest /s` 로 키가 있음을 확인했다. 시험은 이 키를 지우지 않는다.
- 시나리오: 시험을 돌린 모든 PC·CI 러너에 `HKCU\Software\DongArch3DTest` 가 남는다. 출력 폴더 밖 쓰기이지만, 사용자 설정 `Dongguk\DongArch3D`(최근 목록·창 위치·감시 폴더)와는 분리돼 있어 사용자 데이터는 손상되지 않는다. 명세 §3.11 이 정한 방식 그대로다.
- 권고(후속): 시험 `main` 에서 `QSettings::setDefaultFormat(QSettings::IniFormat)` 와 `QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, argv[1])` 를 함께 쓴다(이미 `${CMAKE_CURRENT_BINARY_DIR}` 를 인자로 받는다). 그러면 설정이 빌드 폴더 안 INI 로만 남는다. 다른 방법은 끝에서 `QSettings().clear()` 를 부르는 것이다.

### L2 (low) `cutDrag_` 가 누를 때 고정돼 두 단추를 함께 누르거나 끄는 도중 도구를 내려놓으면 상태가 어긋난다
- 근거: `studio/src/View3D.cpp:183` 는 `mousePressEvent` 가 올 때마다(두 번째 단추를 눌러도) `cutDrag_` 를 다시 정한다. `View3D.cpp:203` 는 놓을 때 그 마지막 값으로만 `cutDragReleased` 를 낸다. `setCutTool(false)`(`View3D.h:17`)는 끄는 중인 `cutDrag_` 를 정리하지 않는다.
- 시나리오 1: ② 에서 왼쪽으로 끄는 중에 오른쪽 단추를 더 누르면 `cutDrag_=false` 가 된다. 이후 어느 단추를 놓아도 `cutDragReleased` 가 나오지 않는다. `CutGesture::drag` 는 확정 마감을 −1 로 두므로(`CutGesture.h:16-19`), 막 각도가 "끄는 중 … 놓은 뒤" 미리보기 상태로 다음 입력까지 남는다. 데이터 손실은 없다. 다른 동작이나 키, [−][+], 내보내기는 `flushCut`/`click` 으로 확정한다.
- 시나리오 2: 왼쪽으로 끄는 중에 `Esc`/`S` 로 홈으로 가면 도구를 내려놓았는데도 이동 이벤트가 계속 `cutAngleDragged` 를 내보낸다. 그래서 홈 페이지에서 막 각도가 바뀌고, 놓을 때 확정된다. JOB-005 까지는 `cutTool_` 을 매번 읽어 보는 방향 회전으로 바뀌었다.
- 권고: `mousePressEvent` 에서 `if (drag_) return;` 으로 첫 단추에서만 끌기 종류를 정한다. `mouseReleaseEvent` 는 `e->buttons() == Qt::NoButton` 일 때 끝낸다. `setCutTool(false)` 에서 `cutDrag_` 가 켜져 있으면 `cutDragReleased` 를 내고 끈다.

### L3 (low) `--cut` 를 첫 인자로 주면 파일 경로로 취급된다
- 근거: `studio/src/main.cpp:39` 는 `args[1]` 이 `--sample`/`--blind`/`--home`/`--shot…` 이 아니면 `w.openPath(args[1])` 를 부른다. `--cut` 판정(`main.cpp:41`)은 `args.contains` 라 위치와 상관없이 따로 걸린다.
- 시나리오: `DongArch3DStudio.exe --cut` 처럼 첫 인자로 주면 `openPath("--cut")` 이 실패해 모달 경고창이 뜬다. `--shot` 과 함께 쓰면 캡처 타이머가 경고창 뒤에 등록돼 창을 닫을 때까지 멈춘다. `--flip`/`--base`/`--max`/`--export` 도 같은 기존 패턴이다. 명세 사용법(`--sample --cut --shot …`)대로면 생기지 않는다. 공격 면은 없다(같은 이름의 상대 경로 파일을 여는 정도).
- 권고(후속): `else if (!args[1].startsWith("--")) w.openPath(args[1]);` 로 플래그를 경로로 취급하지 않게 한다.

### L4 (low, 제안) 화면 통합 시험에 시간 상한이 없다
- 근거: `studio/CMakeLists.txt:109-110`. CTest 기본 상한은 `10000000` s 다(`artifacts/ctest_studio_cutpage_V.log`). 시험 안 `wait()` 의 15 s 상한은 조건 대기에만 걸리고 모달 대화상자는 막지 못한다.
- 시나리오: 나중에 바뀐 코드가 시험 경로에서 `QMessageBox`/`QFileDialog` 를 띄우면 오프스크린에서 아무도 닫지 못해 CI 작업이 러너 제한 시간까지 멈춘다. 지금 시험은 `Ctrl+O`·`Ctrl+P` 같은 대화상자 키를 보내지 않는다(`test_cutpage.cpp` 키 목록 확인).
- 권고: `set_tests_properties(studio_cutpage PROPERTIES RUN_SERIAL TRUE TIMEOUT 120)`.

### I1 (info) 문서를 열지 않은 상태에서 ② 탭
- 근거: `MainWindow.cpp:727` 의 `!doc_.isLoaded()` 면 바로 돌아간다. 잠김은 `doc_.isLoaded() && !poseConfirmed()` 일 때만 걸린다(`MainWindow.cpp:591`).
- 시나리오: 문서가 없을 때 ② 탭을 누르면 ② 리본 페이지는 보이지만 동작은 없다. `onAction` 이 `!doc_.isLoaded()` 에서 끝나므로 null·범위 밖 접근은 없다. 안전 문제는 아니고 UI 검토 몫이다.

## 점검했으나 문제없음
- `CutPageText.h` 의 `QString::arg` 는 여러 인자를 한 번에 넘기는 형태(`plateTitle`·`ctxRight`·`saveSlotAngle`)라 `%` 가 들어간 인자도 안전하다. `angleText` 의 `llround` 는 NaN·거대값에서 정의되지 않지만, 정면각은 `std::remainder` 로 −180~180 에 묶여 닿지 않는다.
- `SheetPane::refresh` 의 `s.name.at(0)` 에 들어가는 슬롯 이름은 모두 내부에서 만든 상수다(`ArtifactDoc.cpp:719·726`, `MainWindow.cpp` saveSlotAngle). 외부 입력이 아니다.
- `onPage` 의 `slot_` 색인은 늘 0 이상이고, `slot_ < size` 를 확인한 뒤 접근한다.
- `--shot` 의 `w.resize(1845, 920)` 는 `closeEvent` 를 거치지 않아(`QApplication::exit`) 사용자의 `window/geometry` 를 덮어쓰지 않는다.
- 오른쪽 단추 끌기에서 문맥 메뉴는 뜨지 않는다. studio 에는 `QToolBar`/`QDockWidget`/`contextMenu` 가 없어 `QMainWindow::createPopupMenu` 가 null 이다.
- 시험은 파일을 쓰지 않는다(`grab()` 은 메모리 안).

## 미확인
- CI(GitHub Windows 러너)에서 `studio_cutpage` 가 실제로 걸리는 시간과 레지스트리 잔존 여부. 이 PC 의 ctest 결과로만 판단했다.
- L2 시나리오는 오프스크린에서 직접 재현하지 않았다. 코드와 `CutGesture.h` 의미를 읽어 추론한 것이다.
