결론: PASS

# JOB-007 보안·안전 검토 (security-reviewer)

- 대상: `D:\_dongarch_build\wt\.worktrees\JOB-007`, 브랜치 `factory/JOB-007`, `git diff 688b7bd..HEAD`(HEAD `89846ba` ← `6542878`). 바뀐 파일은 15개이고 모두 텍스트다(`git diff --numstat` 에 바이너리 `-` 줄 없음). `git status --porcelain` 은 빈 출력이다.
- 방법: diff 를 직접 읽었다(Plate.h/.cpp, MainWindow.h/.cpp, main.cpp, CutPageText.h, ExportDialog.cpp, SheetView.cpp, Ribbon.cpp, View3D.h/.cpp, CMakeLists.txt, test_reflines.cpp, test_cutpage.cpp, UI_RULES.md). 관련된 기존 코드(K-Arch `DxfWriter.cpp`, `ExportDialog::request`, `CutGesture.h`, `ArtifactDoc::setFrontAngle`)도 함께 읽었다. 시험이 남긴 SVG·DXF 산출물은 `artifacts/` 에서 일부를 직접 확인했다. 쓰기 명령은 쓰지 않았다. build-log.md 의 주장은 근거로 쓰지 않았다.
- 차단할 결함(high/medium)은 없다. 아래 발견 사항은 모두 low 이거나 참고다.

## 1. 역할 점검표

| # | 항목 | 결과 | 근거 |
|---|---|---|---|
| 1 | 입력 파서 | 변경 없음 | `git diff --name-only 688b7bd -- studio/src/ArtifactDoc.* CutGesture.h MeshIO.* MeshRaster.* Theme.h … GigaMesh .github factory` → 출력 없음. PLY/OBJ/STL/DXF 읽기 경로는 이번 diff 에 없다 |
| 2 | 경로·파일 | 새 쓰기 경로 없음 | 제품 코드에서 파일을 여는 곳은 기존 `exportSvgLayered`(QFile `r.path`)·`exportDxf`(`r.path.toLocal8Bit()`)·PNG/PDF 뿐이고 바뀌지 않았다. 추가된 줄을 `[A-Z]:\\`·`[A-Z]:/`·`/media/` 로 grep 해도 걸리는 것이 없다 |
| 3 | 외부 실행·네트워크 | 없음 | 추가된 줄에 `QProcess`·`system(`·`ShellExecute`·`QNetwork`·`QDesktopServices`·`getenv` 가 없다. `http://` 한 곳은 test_reflines.cpp:88 의 inkscape XML 이름공간 URI 다(네트워크 아님) |
| 4 | 의존성·라이선스 | 문제 없음 | 새 라이브러리 없음. 타일 아이콘 `lock`·`next`(Ribbon.cpp)는 코드로 그린 선이다. Artec SDK/Studio 자료, 5 MB 넘는 바이너리, 토큰·키는 없다 |
| 5 | CI/스크립트 | 변경 없음 | `.github/**`·`factory/**` 는 diff 에 없다. 명세 §5-3 파이썬 판정은 아래 §2-D 에서 따로 봤다 |
| 6 | `/utf-8` | 적용됨 | studio/CMakeLists.txt:16 `add_compile_options(/utf-8 …)` 는 디렉터리 단위라 `test_studio_reflines` 에도 걸린다. 새 한글 narrow 리터럴(Plate.cpp `kRefStyles` 의 `"중심선"` 등)은 `QString::fromUtf8` 로 읽는다. 산출물 `studio_reflines_s1.svg` 에 `inkscape:label="중심선"`·`"구연선"`·`"바닥선"` 이 바르게 들어 있다 |

## 2. 요청한 초점별 확인

### A. 내보내기 경로 — SVG·DXF 새 층과 표제란 문자열에 사용자 입력이 섞이는가 → 섞이지 않는다

새로 출력하는 문자열이 어디서 오는지 하나씩 확인했다.
- SVG 기준선 층 `<g id>`·`inkscape:label`: `kRefStyles` 상수 3쌍(Plate.cpp, `refline_center/중심선` 등)이다. `layer()` 람다(Plate.cpp:649)는 label 을 이스케이프하지 않지만 넘어오는 값이 모두 상수라 안전하다.
- `<line>` 의 stroke 는 `theme::hex(theme::OuterInk)`, 굵기는 `n3(0.10/0.13/0.25)`, 좌표는 `n3(X(..))`·`n3(Y(..))` 숫자다.
- 표제란 「정면 방위각 …」(SVG Plate.cpp:691-692, PDF/PNG `paintSheet`): `azimuthNote()` 가 상수와 `doc.frontAngle()`(double)로 만든다. 들어가는 글자는 숫자·`.`·`°`·`−`·한글뿐이라 XML 특수문자가 생길 수 없다. `paintSheet` 는 `QString::arg(a, b, c)` 다중 인자 판(한 번에 치환)이라 `%N` 이 다시 치환되는 일도 없다.
- `frontAngle` 의 출처: `ArtifactDoc::setFrontAngle` 이 `std::remainder(deg, 360)` 을 저장하고, 호출하는 곳은 MainWindow 의 막 입력·슬롯·되돌리기뿐이다. 파일이나 QSettings 에서 읽는 경로는 없다. 따라서 `std::llround`(Plate.cpp:45·51)에 NaN/Inf 가 들어가 `-LLONG_MIN` 오버플로가 생기는 경로가 없다.
- DXF: 새 TEXT 는 `"FRONT AZIMUTH " + azimuthDxf(..) + " DEG (CCW, TOP VIEW)"`(Plate.cpp:883-886)로 ASCII 고정문 + 숫자다. K-Arch `DxfWriter::WriteText`(GigaMesh/gui/src/dongarch/karch/DxfWriter.cpp:222-231)는 값을 이스케이프 없이 그대로 쓰지만 이 문자열에는 개행이 없다. R12 에 한글을 넣지 않고 영문을 쓴 것도 맞는 선택이다. 산출물 `studio_reflines_m20.dxf` 의 TEXT 레코드가 `1` / `FRONT AZIMUTH -20.0 DEG (CCW, TOP VIEW)` 인 것을 확인했다.
- 사용자 입력이 들어가는 길은 바뀌지 않았다. 도면명 `r.title`(ExportDialog.cpp:229 `name_->text()`)은 SVG 에서 `toHtmlEscaped()`(Plate.cpp:648·709)를 거치고, PDF 는 `setTitle`(583), PNG/PDF 표제란은 `drawText`(565)로 그린다. DXF 에는 쓰지 않는다. 저장 파일명은 `[\\/:*?"<>|]` 를 `_` 로 바꾼다(ExportDialog::save, 기존).

### B. 새 입력 경로 — Enter 고정, ③ 탭, `--edit`

- **Enter**(MainWindow.cpp:1121-1123): 확정 뒤 분기(1094 의 확정 전 분기 다음)에서 `currentPage() == pCut_` 일 때만 `onAction("fixAngle")` 을 부른다. 순서는 `flushCut` → `needPose`·`requirePose` → `fixAngle` → `refreshAll(false)` 다. 파일 I/O 나 외부 호출은 없다.
  - 인덱스 안전성도 확인했다. `slot_` 은 음수가 될 수 없다(412-413 이 `r >= 0` 으로 거름, 나머지 대입은 0 이거나 `size-1`). `fixAngle` 은 `k < 0` 이면 바로 돌아간다(693-694). `refreshSlots` 의 `*fixedAngle_` 역참조(720-721)는 앞의 `angleFixed()` 단락 평가가 막아 준다.
- **③ 탭**: 확정 전에는 `setPageLocked(pEdit_, base, kLockTip)` 로 잠기고 `onPage` 의 `requirePose()` 가 진입을 막는다. ③ 에서 막 회전 키는 1106 에서 무시된다. 뗌 이벤트는 `CutGesture::keyUp` 이 `active_` 일 때만 확정을 잡으므로 무해하다. 새 문서(`openPath`·`openSample`·`openArtecBlindSample`)·자세 확정·축 맞춤·되돌리기 경로에서 모두 `fixedAngle_.reset()` 을 부른다. 감시 폴더 자동 로드와 끌어 놓기도 `openPath` 를 거치므로 ③ 상태가 남지 않는다.
- **`--edit`**(main.cpp:53-55): `toEdit` 단추를 한 번 `click()` 할 뿐이다. 문서가 없으면 `onAction` 의 `!doc_.isLoaded()` 에서 끝난다. 실행 파일 인자로 `--edit` 만 주면 `args[1]` 이 `openPath("--edit")` 로 가서 실패하는데, 기존 `--cut` 과 같은 무해한 동작이다.

### C. 시험이 출력 폴더 밖에 쓰는가 → CTest 실행에서는 쓰지 않는다

- `test_studio_reflines` 는 `add_test(… ${CMAKE_CURRENT_BINARY_DIR})` 로 받은 `argv[1]`(test_reflines.cpp:315-316)에만 9개 파일을 쓴다(`studio_reflines_{s1,s3,L,h,12}.svg`, `studio_reflines{,_12,_m20,_m004}.dxf`). 이름은 다른 시험과 겹치지 않는다. 빌드 폴더 `D:\_dongarch_build\verify_job007\studio` 에 9개가 있고 worktree 는 깨끗하다.
- `test_studio_cutpage` 에 추가된 판정은 파일을 쓰지 않는다(내보내기 호출 없음). MainWindow 의 QSettings 쓰기(창 위치 등)는 기존 격리 키 `DongArch3DTest`(test_cutpage.cpp:562, 688b7bd 에도 있음)로 간다. 사용자 설정 `Dongguk/DongArch3D` 는 건드리지 않는다.

### D. 파이썬 판정 스크립트(명세 §5-3) 안전성 → 안전

- 절대 경로 인터프리터를 `-I`(= `-E -P -s`)로 실행하고 스크립트는 stdin 으로 넘긴다. 따라서 현재 폴더나 스크립트 폴더의 `json.py` 류 모듈 가로채기, `PYTHON*` 환경 변수(`PYTHONOPTIMIZE` 포함)가 막힌다. heredoc 구분자를 `<<'PY'` 로 따옴표 쳐서 셸 확장도 없다.
- 표준 라이브러리(`os`·`sys`·`xml.etree.ElementTree`)만 쓰고, 읽기만 한다. 쓰기·`subprocess`·네트워크·`eval`/`exec` 는 없다. `ET.parse` 는 시험이 방금 만든 SVG(최대 ≈ 1.5 MB)를 읽는 것이라 신뢰할 수 있는 입력이다. Python 3.12 동봉 expat 에는 엔티티 폭주 방어도 있다. DXF 는 `latin-1` 로 읽어 디코드가 실패할 수 없고, 형식이 깨진 파일이면 `int()` 예외로 종료 코드가 0 이 아니게 된다(판정이 실패 쪽으로 닫힘).

## 3. 발견 사항 (모두 low, 차단 아님)

| # | 심각도 | 위치 | 내용 | 악용·오동작 시나리오 | 권고 |
|---|---|---|---|---|---|
| L1 | low | studio/src/MainWindow.cpp:1121-1123 (·1094) | ② 의 Enter 가 자동 반복(`isAutoRepeat`)을 거르지 않는다 | ① 에서 Enter 를 0.5 s 넘게 누르고 있으면, 확정 계산(UI 스레드 ≈ 1.4 s)이 도는 동안 쌓인 반복 Enter 가 ② 로 넘어온다. 그러면 0° 로 바로 「지금 각도 고정」 되고 A–A′ 슬롯 메모 `보고서 주 도판 (1:1)` 이 `막 0°` 로 바뀐다. 데이터 손상은 아니고 화면 상태와 메모만 바뀌며, 막을 움직이면 고정이 풀린다 | Return/Enter 분기에 `if (e->isAutoRepeat()) break;` 를 넣는다. 사람이 판단할 일이다 |
| L2 | low | studio/src/Plate.cpp:345 · :755 (원인 ExportDialog.cpp:189·231, 기존 코드) | 새 코드가 축척으로 나눈다(`3.0 / s`, `… / px1mm`). 그런데 축척은 QSettings `export/scale` 에서 검증 없이 복원된다 | 레지스트리 값이 깨졌거나 누가 고쳐 0 이 되면 중심선 끝이 ±inf 가 된다. 그러면 SVG 에 `y1="inf"` 같은 잘못된 속성이 써지고 QPainter 가 무한 좌표로 선을 그린다(Qt 의 처리 방식은 미확인). 로컬에서 레지스트리를 고쳐야 생기는 일이라 영향은 작다 | 복원한 축척을 칩 값 5개 중 하나로 고정하거나 `s > 0 && isfinite(s)` 를 확인한다. 후속 작업으로 해도 된다 |
| L3 | 참고 | GigaMesh/gui/src/dongarch/karch/DxfWriter.cpp:222-231 · Plate.cpp:649 | DXF `WriteText` 와 SVG `layer()` label 은 이스케이프하지 않는다 | 지금은 상수와 숫자만 넘어와서 안전하다. 나중에 도면명·메모 같은 사용자 문자열을 DXF TEXT 나 층 label 에 넣으면 개행으로 DXF 레코드가 깨지고, `"`·`<` 로 SVG 속성이 깨질 수 있다 | 사용자 문자열을 넣는 작업에서 ASCII 로 거르고 개행을 지운다(DXF), `toHtmlEscaped()` 를 쓴다(SVG) |
| L4 | 참고 | studio/tests/test_reflines.cpp:315 | 인자가 없으면 `QDir::currentPath()` 에 쓴다(test_side 와 같은 방식) | 개발자가 worktree 루트에서 시험 exe 를 손으로 실행하면 SVG·DXF 9개(≈ 6 MB)가 worktree 에 남는다. CTest 경로에서는 일어나지 않는다 | 인자가 없으면 `QTemporaryDir` 을 쓰거나 실패로 끝내기를 검토한다 |
| L5 | 참고 | 명세 §5-3 판정 블록 | 판정이 `assert` 에만 기댄다 | `python -O` 나 `PYTHONOPTIMIZE` 로 실행하면 assert 가 모두 빠져, 실제로는 실패여도 `PY-SVG OK`·`PY-DXF OK` 를 찍는다. 명세대로 `-I` 로 돌리면 환경 변수가 무시되므로 지금은 해당되지 않는다 | 판정 스크립트를 파일로 옮길 때 명시적 비교와 `sys.exit(1)` 로 바꾼다 |

## 4. 미확인

- 무한·NaN 좌표에 대한 Qt raster/PDF 엔진의 동작(L2): 실행해 보지 않았다.
- 새 층 3개와 DXF 01/07/08 이 Illustrator·AutoCAD 에서 실제로 열리는지(명세 사람 검토 11, 백로그 5·12).
- test-runner 가 캡처 분석에 쓴 세션 scratchpad 의 PIL·numpy 스크립트: 저장소 밖에 있어 읽지 않았다.
- `factory/scripts/verify.ps1`(`-Clean` 의 삭제 경로): 이번 diff 에서 바뀌지 않아 다시 보지 않았다.
