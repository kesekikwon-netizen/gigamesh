# DongArch3D — 고고학 유물 3D 스캔 → 2D 실측 도면

Artec 등 3D 스캐너로 얻은 토기·석기 유물 메시(PLY/OBJ/STL/WRL)를 읽어, 정위치(바닥 수평·회전 중심축)를 잡고, 단면선·외곽선·5대 치수를 뽑아 **1:1 mm 실측 도판**(SVG/DXF/PDF)으로 내보내는 Windows 데스크톱 프로그램입니다. 보고서 도판 작업은 Adobe Illustrator에서 마감하는 것을 전제로 레이어를 분리해 내보냅니다.

- 라이선스: **GPL-3.0-or-later** ([LICENSE](LICENSE), 제3자 고지 [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md)). 기반 프레임워크 GigaMesh(GPLv3, gitlab.com/fcgl/GigaMesh)를 포함하므로 저장소 전체가 GPLv3 조건을 따릅니다.
- 개발 규칙: [CLAUDE.md](CLAUDE.md) · UI 정본 [docs/ui/UI_RULES.md](docs/ui/UI_RULES.md) · 작업 공장 [factory/](factory/)

## 구성

| 폴더 | 내용 | 상태 |
|---|---|---|
| `studio/` | 새 앱 **DongArch3D Studio** (Qt 6 Widgets, 소프트웨어 래스터). 「조용한 유물 도판」 UI 규칙 R1~R8 구현 | 빌드·테스트 11종 통과 (Windows) |
| `GigaMesh/gui/src/dongarch/karch/` | **K-Arch 엔진** — Qt·GigaMesh 무의존 C++20 정적 라이브러리(축 피팅·구연부 검출·슬라이스·실루엣·치수·DXF R12) | Studio가 링크, 단위 테스트 10종 |
| `GigaMesh/` | GigaMesh 프레임워크(Qt 5, GPLv3) + 이전 세대 DongArch 확장(`gui/src/dongarch/*`). 알고리즘 참조·CLI 도구용 | 빌드 가능, 코어는 업스트림 대비 5파일 +76줄만 변경 |
| `factory/` | AI 작업 공장: 작업 명세→구현→검증→검토→사용자 승인 기록 | 운영 중 |
| `.claude/` | Claude Code 설정: `/factory` 스킬, 역할별 서브에이전트 | — |
| `docs/ui/` | UI 규칙 정본·목업 | — |
| `docs/legacy/` | 2025-11 리눅스 시절 설정·15-Task 추적 파일(보관용, 사용 안 함) | 동결 |

## 빌드와 테스트 (Windows 11, 이 명령들은 실제로 실행해 확인한 것입니다)

필요: Visual Studio 2022 Build Tools(MSVC 14.4x), CMake ≥ 3.21, Ninja, Qt 6(Core/Gui/Widgets) — 예: `D:\OSGeo4W\apps\Qt6` 또는 `C:\Qt\6.x\msvc2019_64`.

```powershell
# Studio (권장 경로) — factory/scripts/verify.ps1 이 같은 일을 합니다
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" && cmake -S studio -B D:\b\studio -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=D:/OSGeo4W/apps/Qt6 && cmake --build D:\b\studio --parallel'
cd D:\b\studio ; $env:PATH = "D:\OSGeo4W\apps\Qt6\bin;$env:PATH" ; ctest --output-on-failure
.\DongArch3DStudio.exe            # 실행
.\DongArch3DStudio.exe --sample --shot main.png   # 합성 항아리로 화면 캡처
```

주의: 빌드 폴더는 **짧은 경로**를 쓰세요(긴 경로에서는 MSBuild `FTK1011`로 configure가 실패합니다). `ctest`는 빌드 폴더로 이동한 뒤 실행하세요.

GigaMesh(Qt 5) 쪽은 선택 사항입니다. 깨끗한 클론에는 gitignore된 생성 파일 17개가 없으므로 먼저 `factory/scripts/prepare-gigamesh.ps1`을 실행한 뒤 `factory/scripts/verify.ps1 -Target gigamesh`로 빌드합니다(약 7분).

## 입력 파일 준비 (Artec Studio 기준)

- 메시 내보내기: **PLY(binary, 정점색)** 또는 **OBJ + MTL + JPG/PNG**, STL(기하만), WRL 가능.
- 단위: Artec Studio는 모든 연산이 **mm**입니다. 설정 › 단위에서 내보내기 단위가 mm인지 확인하세요(cm/m로 내보낸 파일은 가져올 때 단위를 묻습니다).
- 좌표계: 내보내기의 'Move model to origin' 여부와 무관하게 **정위치는 앱에서 다시 잡습니다**(바닥 수평 0 mm, 회전 중심축 R 0 mm).
- Artec SDK·Studio 바이너리는 평가용 라이선스이므로 이 저장소에 넣지 않습니다.

## 작업 방식 (`/factory`)

Claude Code에서 `/factory <개발 요청>`으로 작업을 등록하면 명세 → 구현(별도 브랜치·worktree) → 검증(빌드·ctest) → 4종 검토(코드·정확성·보안·UI)까지 자동으로 진행되고, **모든 작업은 사용자가 `/factory approve <ID>`로 승인하기 전까지 병합되지 않습니다.** 자세한 사용법은 [.claude/skills/factory/SKILL.md](.claude/skills/factory/SKILL.md), 작업 기록 형식은 [factory/jobs/README.md](factory/jobs/README.md)를 보세요.

## 이력에 대하여

이 저장소의 이력은 2026-10-17에 새로 시작했습니다(`feat/dongarch-studio` bd53453 기반, 커밋돼 있던 Qt SDK 198 MB·문서 사본·독점 자료 제외). 그 이전 이력(2025-11 리눅스 시절 1커밋)은 `https://github.com/younga1234/git11`과 개발 PC의 로컬 `master`에 그대로 남아 있습니다.
