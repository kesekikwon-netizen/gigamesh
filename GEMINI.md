# C++ Core Rules
1. 모던 C++(C++17/20) 표준을 준수하고 RAII 원칙을 철저히 적용한다.
2. Raw 포인터(new/delete) 직접 사용을 지양하고 스마트 포인터 및 표준 컨테이너를 우선 사용한다.
3. 빌드는 CMake 기반으로 구성하며 컴파일러 경고(-Wall -Wextra)가 없도록 작성한다.
4. 기존 주석과 문서를 보존하고 불필요한 외부 의존성 추가를 자제한다.

# DongArch3D Architecture & Scope Rules
1. **이원화 구조 준수 (`studio/` + `karch/`)**:
   - UI 및 도판/입출력 셸은 `studio/` (Qt6 + MSVC + Ninja, `studio_core` 및 `DongArch3DStudio`)에서 개발한다.
   - 고고학 기하 연산(구연부 PCA, ODR 회전축, 단면 절단, 실루엣, 5대 치수, DXF R12)은 Qt 무의존 엔진인 `GigaMesh/gui/src/dongarch/karch/` (`DongArch::KArch`)를 사용한다.
   - 레거시 Qt5 GigaMesh Dock UI(`DongArchToolPalette`, `PropertyPanel`, `MeasurementPanel`)는 수정하지 않는다.
2. **UI 절대 규칙 (「조용한 유물 도판」 — `docs/ui/UI_RULES.md`)**:
   - **R0 정본 디자인 단일 원본**: 모든 화면·컴포넌트·도판 배치는 반드시 `docs/ui/UI_RULES.md`, `docs/ui/mockup/dongarch3d_mockup.html`, `docs/ui/mockup/dongarch3d_design_spec.md`, `docs/ui/mockup/mockup_01..05_*.png` 정본 디자인만 따른다. 정본에 없는 임의의 UI 디자인을 창작하거나 폐기된 구형 시안(`docs/dongarch3d/mockups/` 등)을 참조·복원하는 것을 엄격히 금지한다.
   - **R1 고정 레이아웃**: 위에서 아래로 `탭 줄(34px) → 타일 리본(98px) → 지금 도구 줄 ctxBar(36px, 도구 파지 시에만 표시) → 작업 공간(슬롯 176px · 3D 보조창 ≈27% · 실측 도판 ≥73%) → 상태줄(30px)` 순서를 엄수한다. 추가 Dock/플로팅 패널/다크 테마를 금지하며 `Tab` 키로 도판 100% 최대화를 지원한다.
   - **R2 색상 토큰**: 모든 색상은 반드시 `studio/src/Theme.h` 토큰만 참조한다. 단면선은 `#FF0000`(`ProfileRed`), 외곽선·중심축은 `#141413`(`OuterInk`)이며, 주 단추(`Action`, `#B5573A`)는 한 화면에 단 하나만 배치한다.
   - **R5 도판 그리기 순서**: `방안 눈금(1/10/50mm) → 유물 음영(실사/깊이/D-Tak) → 외곽선·능선(OuterInk) → 단면선(ProfileRed)` 순서를 지키며 화면·미리보기·내보내기 모두 `Plate::paint` 단일 파이프라인을 공유한다. Artec 내벽 미스캔 추정 구간은 `#FF0000` 파선(`06_INTERIOR_OUTLINE`)으로 분리 표기한다.
   - **R6 내보내기**: 모든 도면 좌표는 실제 `mm` 단위를 유지하며 SVG(5개 레이어 분리), DXF(R12, K-Arch 11 레이어 `01_AXIS_CENTER`~`11_SCALE_BAR`), PDF, PNG를 지원한다.
3. **빌드·커밋·CI 최적화 불변식**:
   - 컴파일러 경고는 MSVC `/W4`(GCC/Clang `-Wall -Wextra -Wpedantic`) 기준 **0건**을 유지하며, Release 빌드 최적화(`AVX2`, `LTO/IPO`, `PCH`)와 타깃별 `AUTOMOC`(`studio_core`, `DongArch3DStudio`만 활성화) 구성을 유지한다.
   - 코드 변경 및 Git 커밋 전에는 반드시 `powershell -File studio/build.ps1 -Test`를 실행하여 11종 CTest(`studio_pipeline` + K-Arch 단위 테스트 10종) 100% 통과를 확인한다.

