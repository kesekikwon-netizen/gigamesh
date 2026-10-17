결론: PASS

# JOB-008 보안·안전 검토 (security-reviewer)

- 대상: `D:\_dongarch_build\wt\.worktrees\JOB-008`, 브랜치 `factory/JOB-008`, `git diff 89846ba..HEAD`(HEAD `a913448`, 커밋 1개). 바뀐 파일은 7개(UI_RULES.md +1, CMakeLists.txt +4, ArtifactDoc.cpp 211, ArtifactDoc.h 33, MainWindow.cpp 5, test_cutpage.cpp +9, test_fit.cpp +613)이고 모두 텍스트다. `git status --porcelain --ignored` 는 빈 출력이다.
- 방법: diff 를 직접 읽었다. 경계값을 따지려고 관련된 기존 코드도 읽었다: `simplifyDP`·`dpRec`·`perpDist`, `recompute`·`confirmPose`, 벽 두께 루프, K-Arch `computeBbox`·`FillMeshStats`·`extractMaxRadius`, `MeshIO::weldVertices`·`buildLodMesh`, `Plate.cpp`·`SheetView.cpp` 의 `displayOutline()` 사용처, `MainWindow::checkWatchFolder`·`onChip`. 시험 산출물이 어디에 놓였는지는 `find` 로 확인했다. build-log.md 는 근거로 쓰지 않았다. 판정은 verify.md 와 artifacts 의 실행 기록을 참고만 했다. 쓰기 명령과 재현 실행은 하지 않았다(읽기 전용 역할).
- 차단할 결함(high/medium)은 없다. 발견 사항은 low 1건과 참고 3건이다.

## 1. 역할 점검표

| # | 항목 | 결과 | 근거 |
|---|---|---|---|
| 1 | 입력 파서 | 파서 변경 없음 | MeshIO·MeshLoader·PLY/OBJ/STL 읽기 코드는 diff 에 없다. 새 코드는 읽어 들인 메시를 소비만 한다(`halfOrthoSilhouette`, ArtifactDoc.cpp:183-225). 면 인덱스 상한은 `weldVertices`(MeshIO.cpp:736-739)가 이미 걸러 준다. 새 함수는 음수만 다시 본다(:196-197). 같은 가정을 쓰는 기존 `FillMeshStats`(MeshLoader.cpp:154)가 먼저 돌므로 새로 노출되는 곳은 없다 |
| 2 | 경로·파일 | 문제 없음 | 추가된 줄을 `[A-Za-z]:[\\/]`·`/media/`·`/home/`·`/tmp`·`QSettings`·`QTemporary`·`remove(` 로 grep 하면 0건이다. 제품 코드에 새 파일 쓰기는 없다 |
| 3 | 외부 실행·네트워크 | 없음 | 추가된 줄에 `QProcess`·`system(`·`ShellExecute`·`QNetwork`·`http(s)://` 가 없다. 감시 폴더 자동 로드(MainWindow.cpp:504-526)는 바뀌지 않았다. 자동 로드는 확정 전 상태까지만 가므로(`openPath` → `loadFile`), 새 외곽선 계산은 사람이 ① 에서 확정해야 돈다 |
| 4 | 의존성·라이선스 | 문제 없음 | 새 라이브러리는 없다. 반쪽 정사 실루엣은 같은 저장소(GPL) K-Arch `addTriZHits` 의 교점 규칙을 Studio 에 다시 쓴 것이다. Artec SDK/Studio 자료, 5 MB 넘는 바이너리, 토큰·키는 없다(numstat 에 바이너리 줄 없음) |
| 5 | CI/스크립트 | 변경 없음 | `.github/**`·`factory/**` 는 diff 에 없다 |
| 6 | `/utf-8` | 적용됨 | studio/CMakeLists.txt:16 `add_compile_options(/utf-8 …)` 는 디렉터리 단위라 새 `test_studio_fit` 에도 걸린다. `02_build.log` 의 `warning`·`C4819` 는 0이다. 새 한글 문자열 `· 최대 편차 %4 mm` 는 `QStringLiteral` 이다(MainWindow.cpp:664). test_cutpage 정규식은 `QString::fromUtf8(R"(…)")` 이다. verify 의 `[J8-10] CtxRight '점 1,272 → <b>94개</b> (0.5 mm 간략화) · 최대 편차 0.21 mm'` 가 정규식에 맞았으므로 인코딩은 정상이다 |

## 2. 수치 경계: 무한 루프·범위 밖 접근

| 경계 | 코드 경로 | 결과 |
|---|---|---|
| 빈 메시·면 없음 | `halfOrthoSilhouette` 조기 반환(cpp:185-186) → `ortho_` 빔 → `fitCurve(빈)` 은 DP 조기 반환 뒤 `m < 3` 이라 빈 결과 → `dev2` = 0 | 안전. 시험 `[J8-6] empty=0.0 unconfirmed=0.0` |
| 0 높이(z0 == z1) | 칸 수 n = 0 → `best` 빔. 면마다 `k1 = min(n−1, …) = −1` 이라 칸 루프를 돌지 않는다(cpp:200-202) → 빈 외곽선 | 안전. 소비자 `Plate.cpp:361·770·855`, `closingEnds`(Plate.cpp:61), `SheetView.cpp:62` 는 모두 `size() >= 2`(또는 i = 1 부터) 를 확인한다 |
| 점 1개(외곽선) | `ortho_` 1점 → DP 조기 반환 → 1점. `lineDist` 는 n = 1 이면 그 점까지의 거리(cpp:158-167, `n > 1` 가드) | 안전. 그리기·DXF·SVG 는 2점 미만이면 건너뛴다 |
| 단면 고리 0·1·2점 | `fitSectionLine` 조기 반환 → `sectionNatural`·`maxDeviationMm` 의 `size() >= 3`(닫힘)·`>= 2`(열림) 거름(cpp:1213-1219) | 안전 |
| `fitCurve` 인덱스 | `kept` 와 `c` 는 push·pop 을 늘 짝으로 한다(cpp:285-296). 조기 반환이면 `0…n−1` 이므로 `kept.size() == c.size()`. 닫힘 되감기 점(인덱스 n)은 늘 마지막 keep 이고 pop 으로 빠진다. 구간 끝 `e = kept[i+1]` 또는 n(cpp:323), `raw[k % n]` 에서 n ≥ 3(cpp:326). 모든 루프의 상한은 유한하다 | 범위 밖 접근 없음, 무한 루프 없음 |
| tol ≤ 0 / 축척 0 | 간격은 칩 고정값 {0.1, 0.3, 0.5, 1.0}(MainWindow.cpp:1051-1052)과 그 값을 담은 되돌리기 상태뿐이다. tol ≤ 0 이면 DP 가 조기 반환하고(전 점 유지) 곡선 판정 `≤ 0` 은 거의 거짓이라 직선이 된다. 실루엣 칸 크기는 상수 0.25 이고 `bin <= 0` 가드가 있다. 이번 diff 는 내보내기 축척 코드(Plate.*)를 바꾸지 않는다 | 안전 |
| NaN | 첫 정점이 NaN 이면 bbox 가 NaN 이 되고, 칸 수 루프 조건이 거짓이라 n = 0. NaN 이 든 면은 `long(floor(NaN))` 이 정의되지 않은 동작(UB)이다. MSVC 는 LONG_MIN 을 내므로 `k0 > k1` 이 되어 칸 루프를 건너뛴다. 클램프 덕에 범위 밖 접근은 없다. `fitCurve`·`dev2` 는 NaN 비교가 거짓이라 직선으로 떨어진다 | 실무상 안전(IEEE 비교 전제). 아래 [S-1]·[참고 1] |
| ±inf z | 칸 수 루프 `while (z0 + (n+0.5)·bin <= z1)`(cpp:189)가 끝나지 않는다 | 새 경로다. 다만 **기존 코드도 같은 입력에서 끝나지 않는다**(벽 두께 루프 cpp:814: `wallStep_ = max(1, h/160)` 이 inf 이거나 z 가 inf 에 머묾, 정적 판독). 따라서 새 위험 부류는 아니다 |
| 거대한 유한 z 범위 | 아래 [S-1] | **low** |
| 재귀 깊이 | `dpRec` 는 기존 코드이고 최악 O(n) 이다. 새로 넣는 원 선 `ortho_` 는 높이/0.25 개(예제 739)로 묶인다 | 새 위험 없음 |

### [S-1] low — 반쪽 정사 실루엣의 칸 수가 메시 z 범위에 비례하고 상한이 없다

- 근거: `studio/src/ArtifactDoc.cpp:188-192`. 칸 수 n 을 `while (z0 + (double(n) + 0.5) * bin <= z1) ++n;` 로 센다(`long`, MSVC 에서 32비트). 이어서 `std::vector<double> best(size_t(n), kInf)` 를 잡는다. 면마다 `k0`·`k1` 도 `long(std::floor/ceil(…))` 로 바꾼다(:200-201).
- 오동작 시나리오: 깨진 바이너리 PLY/STL 정점 하나가 본체 면에 붙어 |z| ≈ 1e12 같은 쓰레기 값을 가진 채 남는다. MeshIO 에는 유한성 검사가 없다(`grep isfinite studio/src/MeshIO.cpp` 0건). 사용자가 ① 에서 확정하면 n 이 2³¹ 을 넘겨 부호 있는 정수 넘침(UB)이 나고 루프가 끝나지 않아 창이 멈춘다. 범위가 1e8–5.4e8 mm 이면 루프는 끝나지만 수 GB 를 할당하다 `bad_alloc` 이 나고, 이 예외가 Qt 이벤트 루프 밖으로 나가 앱이 종료된다. 예전 MaxRadius 는 2·98 백분위를 써서 이 입력을 견뎠고, 벽 두께 루프는 161번에 끝난다. 그래서 「거대한 유한 값」 에서의 정지·종료는 이번에 새로 생겼다.
- 등급을 low 로 둔 까닭: 로컬 데스크톱 앱이고 사람이 확정해야 실행된다. 손상되었거나 조작된 파일이 필요하고, 영향은 서비스 거부(멈춤·종료)뿐이다. ±inf 는 기존 코드에서도 멈춘다. 예제·Artec 정상 출력은 해당하지 않는다.
- 권고(후속, 두 줄 수준):
  - 함수 머리에 `if (!std::isfinite(z0) || !std::isfinite(z1) || !(bin > 0) || (z1 - z0) / bin > 4e6) return out;` 를 둔다(예: 1 km 상한).
  - n 은 닫힌 식과 `long long`/`size_t` 로 셈하되 경계는 지금 식(`<=`)으로 한 번 보정한다.
  - 면 루프에서 `lo`·`hi` 가 유한하지 않으면 건너뛴다(:198 뒤).
  - 근본 대책은 파서 쪽이다. 읽기 단계에서 유한하지 않거나 터무니없는 좌표를 거르거나 경고한다(MeshIO, 이번 범위 밖, 기존 결함).
  - 시험 쪽 `rawHalfOrtho`(test_fit.cpp:100-101)는 같은 구조지만 알려진 합성 메시에만 쓰므로 그대로 둬도 된다.

## 3. 시험이 출력 폴더 밖에 쓰는가 → 쓰지 않는다

- test_fit.cpp:446-447: 출력 폴더는 `argv[1]` 이고, 인자가 없을 때만 현재 폴더다. 파일 경로는 모두 `out + '/' + 고정 이름` 이다(SVG·DXF 504-507, CSV 389-390). CMake 는 `${CMAKE_CURRENT_BINARY_DIR}` 를 넘긴다(studio/CMakeLists.txt `add_test(NAME studio_fit …)`).
- 실제 위치: `find D:/_dongarch_build -maxdepth 4 -name "studio_fit_*"` 결과는 `build_job008/studio/`·`verify_job008/studio/` 의 6개씩뿐이다. artifacts 의 사본은 test-runner 가 복사한 것이다. worktree 는 `--ignored` 까지 깨끗하다.
- 시험은 `QSettings`·환경 쓰기를 하지 않는다. 프로세스 안에서 `qputenv("QT_QPA_PLATFORM")` 만 쓴다. 파일 삭제나 덮어쓰기 외 파괴적 동작도 없다. CSV 는 `snprintf(buf, sizeof buf, …)` 라 긴 값은 잘릴 뿐 넘치지 않는다(:380). test_cutpage 는 읽기 판정 2개만 더했다(:439, :470-476).

## 4. 경로 하드코딩 → 없음

추가된 줄에 절대 경로가 없다(제품·시험·CMake·정본 모두). 시험 경로는 인자로만 받는다. 명세 §5 의 검증 명령에 있는 `D:\_dongarch_build\…` 경로는 factory 기록이고 저장소 코드가 아니다.

## 5. 참고(판정 아님)

- [참고 1] Release 플래그 `/fp:fast`(studio/CMakeLists.txt:21)와 GCC 의 `-ffast-math`(:29) 아래에서는 NaN·inf 비교가 IEEE 대로라는 보장이 없다. 위 「NaN 이면 n = 0」 은 IEEE 비교를 전제로 한 판단이다. **미확인**(재현하지 않음). 수치 재현성은 정확성 검토 몫이다.
- [참고 2] `long(std::floor(NaN))`(cpp:200-201)은 형식상 UB 다. 실제 MSVC 결과에서는 범위 밖 접근이 없다. [S-1] 권고(유한하지 않은 면 건너뛰기)로 함께 없어진다.
- [참고 3] `maxDeviationMm()` 은 원 선 × 그린 선 O(N·M) 이고 UI 스레드에서 상황 줄을 갱신할 때마다 돈다(MainWindow.cpp:668). 예제는 ≈ 2 ms 다. 25만 면 넘는 실제 스캔(단면 수천 점, 간격 0.1)에서는 수십 ms 일 수 있다. 성능 항목이라 정확성·코드 검토에 넘긴다(보안 결함 아님).

## 6. 미확인

- [S-1] 의 정지·종료를 실제 손상 파일로 재현하지 않았다(읽기 전용 역할). 정적 판독만 했다.
- `/fp:fast` 아래 NaN 비교의 실제 코드 생성은 보지 않았다.
