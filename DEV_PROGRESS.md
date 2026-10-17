# DongArch3D 개발 진행 기록

> UI 규칙: [docs/ui/UI_RULES.md](docs/ui/UI_RULES.md) · 기준 목업: [docs/ui/mockup/](docs/ui/mockup/)

## 결정 (2026-10-14)

- **목업 「조용한 유물 도판」을 UI 규칙으로 확정** → `docs/ui/UI_RULES.md` (R1~R8 + 검수 체크리스트), `CLAUDE.md`·`AGENTS.md` 최상단에 연결.
- **새 UI 셸 `studio/` (Qt6 + MSVC)**: 이 PC에는 GigaMesh GUI(Qt5)를 빌드할 도구가 없다
  (`Qt/5.15.2/msvc2019_64/bin` 에 moc/uic/rcc 없음, Qt5 미설치). OSGeo4W Qt6 로 바로 빌드되는 독립 셸을 만들고,
  계산은 Qt 무의존 K-Arch 엔진(`GigaMesh/gui/src/dongarch/karch/`)을 그대로 링크한다.
  GigaMesh 기존 Dock 기반 GUI(`DongArchToolPalette/PropertyPanel/MeasurementPanel`)는 손대지 않았다.
- K-Arch 엔진은 `D:\gigamesh` 작업 트리에만 있던(미커밋) 파일을 이 브랜치로 옮겨 추적한다.

## 완료

| # | 항목 | 규칙 | 검증 |
|---|---|---|---|
| 1 | 디자인 토큰·QSS (`Theme.h`) | R2·R3·R4 | 빌드 |
| 2 | 유물 문서: 로드 → 구연부 PCA → ODR 회전축 → R0/H0 → 단면·외곽·5대 치수, 둘레 평균 기벽 두께 | R1.3·R5 | `test_studio_pipeline` (합성 7° 기울어진 항아리: 기고 184.8 / 구경 142.0 / 최대복경 165.5 / 기벽 6.80 mm) |
| 3 | 바이너리 PLY · STL 읽기 + 정점 용접 (K-Arch 는 ASCII PLY/OBJ 만) | — | STL 왕복 동일 치수 |
| 4 | 소프트웨어 래스터 (실사 / 깊이 / 디지털 탁본 D-Tak) | R5 | 화면 캡처 |
| 5 | 실측 도판: 그리는 순서, 좌단면(빨강)·우외곽(먹)+음영, 1/10 mm 방안, 50 mm 숫자, 구경·기고 표기, 축척 막대, 기벽 두께 풍선 | R5·R1.3 | 화면 캡처 |
| 6 | 메인 창 고정 구조: 탭 34 · 리본 98 · ctxBar 36(도구 들 때만) · 슬롯 176 · 3D ≈27 % · 도판 · 상태줄 30, `Tab` 100 % | R1 | 화면 캡처 |
| 7 | 리본 3단계 상태(대기/켜짐/손에 든 도구) + 주 단추 1개, 간략화(DP)·곡선(Catmull) 칩 | R3 | 테스트(DP 점 수 감소) |
| 8 | 3D 보조창: 음영 메시, 절단면, 흰 테 빨강 단면선, A/A′, 단면선 도구로 끌어 각도 회전 | R1·R2 | 화면 캡처 |
| 9 | 내보내기 창: 용지 미리보기·표제란·맞춤 안내, 축척 칩, 레이어 체크, SVG(레이어 분리)/PDF/DXF(R12 11레이어)/PNG | R6 | 테스트(SVG 레이어 4종·순서, A4 1:1 맞춤/2:1 경고) |
| 10 | 시작 화면: 이어서 작업 카드, 최근 삼선표, 끌어놓기, 세 걸음 안내 | R7 | 화면 캡처 |
| 11 | **Artec 3D 표준 포맷 & 자동 전처리 (`MeshIO`)**: PLY(ASCII/Binary + 정점 RGB + `TextureFile` UV), 고속 OBJ(`from_chars`/`strtod` + `.mtl` `map_Kd` 및 동반 JPG/PNG UV 텍스처 샘플링), VRML 2.0(`.wrl`), STL 통합 로더 · 턴테이블/먼지 부유 조각(`< 2%`) 자동 제거 · `m/cm → mm` 자동 단위 환산 · 스캐너 프로파일(`Artec Space Spider` / `Artec Eva·Leo`) 자동 감지 | R1·R7 | `test_studio_pipeline` (미터 단위 STL + 부유 노이즈 2면 자동 제거 및 ×1000 환산 검증) |
| 12 | **광학 스캐너 음영(내벽 미스캔·홀필 마개) 자동 보정 (`ArtifactDoc`, `Plate`)**: 좁은 아가리 토기(호·병·항아리)에서 구연부 상부만 찍히고 동체부·저부 내벽이 미스캔된 경우 구연부 실측 두께를 기준으로 내벽을 자동 추정 복원하고, 실제 스캔 구간은 `#FF0000` 실선(`04_SECTION_WALL`)으로, 추정 내벽 구간은 `#FF0000` 파선(`06_INTERIOR_OUTLINE`, SVG `stroke-dasharray="2.0,1.3"`, DXF `Dashed`)으로 분리 표기 (`W` 키 토글, `-/+` 두께 미세조절) | R5·R6 | `test_studio_pipeline` (`loadArtecBlindSample` 검증) + `shot_blind.png` |
| 13 | **Artec Studio 실시간 감시 폴더 & 전역 Drag & Drop (`MainWindow`, `StartPage`)**: `QFileSystemWatcher` + 파일 쓰기 완료 대기(안정화 타이머)로 Artec Studio에서 내보내는 즉시 무클릭 자동 로드·정위치·1:1 도판 표시 | R7 | UI 및 감시 폴더 연동 |
| 14 | **초고해상도 Space Spider 60 fps 3D LOD & 고대비 D-Tak 탁본 (`MeshIO::buildLodMesh`, `MeshRaster`)**: 3D 회전 뷰는 공간 격자 클러스터링 LOD(`≤ 35,000면`, 정점 컬러 보존)로 즉시 회전하고, 2D `Y=0` 단면 절단과 고대비 곡률 디지털 탁본(`D-Tak`) 및 실사 텍스처(`Real`)는 원본 정밀도로 렌더링 | R1·R5 | `test_studio_pipeline` (`lodFaces=26180` 검증) |

## 빌드 · 테스트

```powershell
powershell -File studio/build.ps1 -Test -ShotDir <dir>  # 증분 빌드 + 헤드리스 테스트 + 화면 캡처 5종
powershell -File studio/build.ps1 -Run                  # 실행
studio/build-msvc/DongArch3DStudio.exe --shots <dir>    # shot_home / shot_main / shot_blind / shot_max / shot_export 일괄 생성
```

## 다음 (백로그)

- [ ] 석기·기와 **4면 전개 도판** (목업 ②) — 비대칭 유물: 정면·측면·배면·횡단면, 능선 추출
- [ ] 슬롯·정위치·간략화 설정을 프로젝트 파일로 저장 → "이어서 열기"가 슬롯까지 복원
- [ ] 단면선 수동 편집(점 끌기), 문양·능선 레이어 채우기
- [ ] 햇빛 고대비 모드 (R2 단일 라이트 테마의 변형)
