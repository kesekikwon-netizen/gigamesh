# factory/project-profile.md — 에이전트가 가장 먼저 읽는 프로젝트 프로필

갱신: 2026-10-17. 근거는 프로젝트 라이브러리의 1단계 조사 보고서(`DongArch3D_1단계_조사보고서_및_준비계획*.md`).

## 한 줄 정의
Artec 등 3D 스캐너의 유물 메시(mm) → 정위치(바닥 수평·회전 중심축) → 단면선·외곽선·5대 치수 → 1:1 mm 실측 도판(SVG/DXF/PDF, Illustrator 레이어 분리). 초보자가 세 걸음(가져오기 → 정위치 → 도면)으로 끝내는 Windows 앱.

## 스택
- 앱: `studio/` — C++20, Qt 6 Core/Gui/Widgets(OpenGL 미사용, 소프트웨어 래스터), MSVC 14.4x, CMake ≥3.21, Ninja.
- 엔진: `GigaMesh/gui/src/dongarch/karch/` — Qt·GigaMesh 무의존 C++20 정적 라이브러리 `karch`(Taubin 원 피팅·구연부 검출·PCA/ODR 축·슬라이스·실루엣·치수·DXF R12). 테스트 10종 + Studio 파이프라인 테스트 1종 = CTest 11종.
- 참조: `GigaMesh/` — Qt 5.15, GPLv3. 코어는 업스트림(gitlab fcgl 23d8cf4) 대비 5파일 +76/−2줄만 변경. 새 기능 금지.

## 검증된 명령 (2026-10-17, 이 PC)
| 대상 | 명령 | 결과 |
|---|---|---|
| Studio | `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1` (= vcvars64 + `cmake -S studio -B D:\_dongarch_build\verify\studio -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:/OSGeo4W/apps/Qt6` → build → `cd` 후 `ctest --output-on-failure` → `--shot` 캡처) | 빌드 19초, 11/11 통과 |
| GigaMesh | `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Target gigamesh` (= `prepare-gigamesh.ps1` → VS2022 생성기, `/DWIN32 /D_WINDOWS /EHsc /utf-8` → build → ctest) | 빌드 ≈7분, 오류 0, `testdata/` 있으면 ctest 통과 |
함정: 빌드 폴더는 짧은 경로(FTK1011), `ctest --test-dir` 대신 `cd`, 소스는 git 체크아웃 안, PATH의 OSGeo4W python 대신 `C:\Users\kyi25\AppData\Local\Programs\Python\Python312\python.exe`.

## 규약
- 단위 mm(double), 수직축/회전축 +Z, 도면 좌표 (x, z), SVG는 y 반전·`width/height` mm·viewBox 1:1·`<g id>` 레이어·선 두께 mm, DXF R12 `$INSUNITS=4` 11레이어.
- 단위 자동 환산은 모호하면 사용자에게 질문. 추정값(내벽 추정 등)은 실측값과 구분 표기.
- UI 정본 `docs/ui/UI_RULES.md` R1~R8. 새 문자열 한국어 100 %, 색상은 `studio/src/Theme.h` 토큰.
- `GigaMesh/core`·`external` 수정 금지. 커밋 메시지 한국어 본문.

## 결정 기록
| ID | 결정 | 상태 |
|---|---|---|
| D1 | 새 앱 본체 = `feat/dongarch-studio`(Studio + K-Arch). GigaMesh는 참조·CLI | **기본값 적용(사용자 확인 대기)** 2026-10-17 |
| D2 | 저장소 전체 GPL-3.0-or-later, 대상 저장소 LICENSE 교체 | 기본값 적용(사용자 확인 대기) |
| D3 | 새 이력으로 시작(Qt SDK·문서 사본·독점 자료 제외), 옛 이력은 `younga1234/git11`·로컬 `master`에 보존 | 기본값 적용(사용자 확인 대기) |
| D4 | 옛 `.claude` 15-Task 설정은 `docs/legacy/`에 보관, factory로 대체 | 기본값 적용(사용자 확인 대기) |
| D5 | 토기 도판 좌/우 배치 | **확정(2026-10-17, 코디네이터 전달 — 사용자 기관 보고서 도판 6장 근거): 외면 왼쪽 · 단면 오른쪽**, R 키로 좌우 반전 지원. 단면은 윤곽선 두 줄(외면·내면) 기본. 조사 결과(한국 법령·고시엔 규정 없음, 일본 우 단면, 영국 좌, 미국 우)와도 충돌하지 않음. **현재 Studio·UI_RULES R5 는 반대(왼쪽 단면)** → 백로그 결함 6. 사용자 "승인개발진행"(09:39) 뒤 JOB-004(004a)로 구현 — **2026-10-17 14:04 UTC 승인·병합(200ec16)**. 정면각 + = 위에서 보아 반시계(JOB-005, 같은 승인) |
| 배포 모델 | 무료 공개 / 기관 내부 / 판매 | 미정 — triangle 비상업 조항 영향이 갈림 |

## 사용자 핵심 요구 (2026-10-17, 코디네이터 전달)
- 작업 흐름: **바닥 정하기**(X·Y·Z 이동·회전으로 바닥판에 올려 확정) → **얇은 막 절단**(3D 뷰에서 막을 움직이면 오른쪽 도판에 단면이 실시간 표시) → **도면 편집**으로 넘기기. 디자인 v2 주 화면이 이 순서.
- 기준선 레이어: 중심선, 구연부 수평선, 바닥선, 다면도 눈금, 복원 파선, 파단 표시. 단면은 윤곽선 두 줄 기본.
- 화면 디자인 v2: https://claude.ai/artifact/C4b5eZFrQmF2NPaJZCmJQ8 (원본 `gigamesh-docs/design/dongarch3d-design.html`), 스캔 단면 자르기 설명서(기존 제품 비교): https://claude.ai/artifact/J7TtJ1fMbgSAYUPPVdT2yf. 저장소 반입은 백로그 8.

## 기준 SHA
- 새 이력 기반: `feat/dongarch-studio` **bd534539b41393d402e4f00eaa8da216d2cc96a1** (2026-10-17 10:20). 이후 그 브랜치의 커밋은 수동 검토 후 체리픽.
- 옛 이력: `master` d9f5543 (= `younga1234/git11` master).
- factory/setup 병합 이력: JOB-001 → a244ce4, JOB-002 → d651215 (2026-10-17, 사용자 승인 09:38 UTC). JOB-003 → eb95b13, JOB-004 → 200ec16, JOB-005 → 30af4e6 (2026-10-17, 사용자 "/goal 전부 승인한다" 14:04 UTC, 병합마다 깨끗한 빌드 재검증 13/14/15 통과, 병합본 Actions run 39039646665 성공 15/15). GitHub `main` 은 아직 LICENSE 1커밋 그대로 — main 병합은 별도 지시가 있을 때만.

## 알려진 위험 (작업 명세 때 참고)
1. 파편(유물편) 미지원: 열린 호 버림, 파손 하단에서 허위 저경 가능(`DimensionAnalyzer.cpp:153-193`).
2. 단위 자동 환산 경계 거침(`studio/src/MeshIO.cpp:1257-1273`): 소형 유물 ×10 오동작, 18 cm 이상 cm 파일 미환산.
3. 25만 면 초과 시 치수를 3.5만 면 LOD로 계산(`ArtifactDoc.cpp:381,401,760`).
4. 기존 GigaMesh 단면 함수 `calcIntersectionPolylineWithPlane`은 비정렬·범위 미검사 → 재사용 금지(`core/mesh/mesh.cpp:4035-4100`).
5. SVG 레이어 수(3/4/5/6)·축척 칩·선 굵기 단위·디지털 탁본 정의가 문서마다 다름 → 명세에서 하나를 명시.
6. Illustrator 실제 가져오기 미검증(Inkscape 레이어 네임스페이스, base64 PNG 음영).
7. 오프스크린 캡처에서 한글 폰트 미탐색(□).
8. `triangle` 비상업 라이선스(GigaMesh 기반 실행파일), Google Analytics 전송 코드(`QGMMainWindow.cpp:574-612`).

## 조사 문서 (2026-10-17, 별도 조사 스레드)
- 공유 폴더 `/mnt/project-files/gigamesh-docs/`: `plan-설계초안.md`, `research-실측표준.md`, `research-자동화범위.md`, `research-도구비교.md`. 저장소 `research/`·`plan/`로 반입하는 작업은 백로그 후보(코드 무변경, push 는 사용자 문장 필요).
- 요지: 허용오차의 공식 기준은 없음 → 검증 기준은 "제안 후 사용자 확인" 절차 유지. 단위 정책은 "추정 후 사용자 확정, 확정 전 잠금". 1차 MVP 는 완형 토기만(완료 게이트: 사용자 Artec 완형 스캔 1점 통과). Illustrator 가 mm SVG/PDF 를 1:1 로 여는지 사용자 PC 에서 100 mm 보정 사각형으로 실측.
- 라이선스: 조사 문서는 Studio+K-Arch 분리 저장소·독자 라이선스(②)를 권했으나 사용자는 저장소 전체 GPL-3.0-or-later(①)를 택했고 그대로 push 됨. ②로 바꾸려면 main 병합 전인 지금이 비용이 가장 적다(결정 기록용 메모, 변경 지시 아님).

## 외부 자료 위치 (읽기 전용, 커밋 금지)
- Artec Studio 20 매뉴얼: `C:\Program Files\Artec\Artec Studio 20 Trial\Documentation\Artec_Studio_Manual_EN.pdf`. SDK 헤더: `C:\Program Files\Artec\Artec 3D Scanning SDK\include`.
- 업스트림 GigaMesh 클론(testdata 26개): `C:\Users\kyi25\OneDrive\Dokumen\GigaMesh`.
- 바이너리 분석 원장: `C:\Users\kyi25\OneDrive\바탕 화면\gegamesh1\*\LEDGER.md` (공개 금지).
- 디자인 원형(Strata): `D:\desca`.
