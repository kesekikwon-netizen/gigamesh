# Phase 0: 기반 기능 강화

**기간**: 4일 (5일 → 4일, 1일 단축)
**우선순위**: 필수
**난이도**: ★★☆☆☆
**상태**: 🔴 대기

---

## 🎯 Phase 목표

이 Phase는 DongArch3D의 기반을 다지는 단계입니다:
1. ✅ **GigaMesh 메시 처리 기능을 GUI로 통합**
2. ✅ **3D 파일 드래그 앤 드롭 지원**
3. 🟢 **기본 Settings UI 구현**

**핵심 전략**: GigaMesh의 강력한 `gigamesh-clean` CLI를 GUI로 래핑

---

## 📋 Task 목록

### Task 0.1: GigaMesh 메시 처리 기능 통합 (2일)

**GigaMesh 활용**:
```cpp
// A:\1105\GigaMesh\cli\gigamesh-clean.cpp
bool cleanupGigaMeshData(
    bool borderErosion,
    bool keepLargestComponent,
    int maxVertices
);
```

**구현 내용**:
1. **CLI 래퍼 클래스 작성** (0.5일)
   - `DongArchMeshCleaner` 클래스
   - `gigamesh-clean` 프로세스 실행
   - stdout/stderr 캡처

2. **후처리 옵션 UI** (1일)
   - Border erosion on/off (QCheckBox)
   - Keep largest component (QCheckBox)
   - Max vertices 설정 (QSpinBox)
   - 적용 버튼

3. **진행 상황 표시** (0.5일)
   - QProgressDialog
   - 실시간 로그 출력

**예상 산출물**:
- `gui/src/DongArchMeshCleaner.h/cpp`
- `gui/src/dialogs/QGMDialogMeshCleanup.h/cpp`

---

### Task 0.2: 3D 파일 드래그 앤 드롭 (1일)

**구현 내용**:
1. **드래그 앤 드롭 이벤트 핸들러** (0.5일)
   ```cpp
   void QGMMainWindow::dragEnterEvent(QDragEnterEvent* event);
   void QGMMainWindow::dropEvent(QDropEvent* event);
   ```

2. **파일 형식 자동 인식** (0.3일)
   - `.ply`, `.obj`, `.stl` 확장자 검사
   - MIME type 검증

3. **자동 후처리 실행** (0.2일)
   - 파일 로드 후 자동으로 `gigamesh-clean` 실행
   - 사용자 확인 다이얼로그

**예상 산출물**:
- `QGMMainWindow.cpp` 수정 (드래그 앤 드롭 핸들러)

---

### Task 0.3: 기본 Settings UI (1일)

**구현 내용**:
1. **Model 3D 색상 프리셋** (0.5일)
   - 45개 색상 팔레트 (Arch3D Liner 참고)
   - QColorDialog 통합
   - 프리셋 저장/불러오기

2. **Canvas 3D 배경색** (0.3일)
   - 그라데이션 배경 (상단/하단 색상)
   - 단색 배경 옵션

3. **Ground Plane 색상** (0.2일)
   - 그리드 색상 설정
   - 투명도 조정

**예상 산출물**:
- `gui/src/dialogs/QGMDialogSettings.h/cpp` (신규 또는 수정)

---

## 🔧 GigaMesh 알고리즘 활용

### 완전 구현됨 - 바로 사용 가능

| 기능 | GigaMesh 소스 | 활용 방법 |
|------|---------------|----------|
| **메시 후처리** | `cli/gigamesh-clean.cpp` | CLI 프로세스 실행 |
| **PLY I/O** | `core/mesh/MeshIO/MeshIOPLY.cpp` | 자동 로드 |
| **OBJ I/O** | `core/mesh/MeshIO/MeshIOOBJ.cpp` | 자동 로드 |
| **STL I/O** | `core/mesh/MeshIO/MeshIOSTL.cpp` | 자동 로드 |

**단축 시간**: 2일 → **1일** (CLI 래핑만 필요, 50% 단축!)

---

## ⏱️ 타임라인

| Day | 작업 내용 | 산출물 |
|-----|----------|--------|
| 1 | CLI 래퍼 + 후처리 UI (전반부) | `DongArchMeshCleaner` 클래스 |
| 2 | 후처리 UI (후반부) + 진행 상황 표시 | `QGMDialogMeshCleanup` 다이얼로그 |
| 3 | 드래그 앤 드롭 구현 | `QGMMainWindow` 수정 |
| 4 | Settings UI 구현 | `QGMDialogSettings` 다이얼로그 |

---

## ✅ 성공 기준

### Task 0.1 성공 기준
- [ ] gigamesh-clean CLI를 GUI에서 실행 가능
- [ ] Border erosion, Keep largest component, Max vertices 옵션 동작
- [ ] 진행 상황 QProgressDialog 표시
- [ ] 후처리 완료 후 메시 자동 재로드

### Task 0.2 성공 기준
- [ ] PLY, OBJ, STL 파일 드래그 앤 드롭 동작
- [ ] 파일 로드 후 자동 후처리 실행
- [ ] 사용자 확인 다이얼로그 표시

### Task 0.3 성공 기준
- [ ] Model 3D 색상 변경 가능
- [ ] Canvas 3D 배경색 그라데이션 설정 가능
- [ ] Ground Plane 색상 및 투명도 조정 가능
- [ ] 설정 저장 및 불러오기 동작

---

## 📝 구현 노트

### 주의 사항
1. **gigamesh-clean 프로세스 실행 시**:
   - Windows: `gigamesh-clean.exe` 경로 확인
   - Linux: `gigamesh-clean` 바이너리 경로 확인
   - 상대 경로 vs 절대 경로 문제 해결

2. **드래그 앤 드롭 시**:
   - 여러 파일 동시 드롭 금지 (첫 번째 파일만 로드)
   - 파일 경로에 공백 및 한글 처리

3. **Settings UI**:
   - QSettings를 사용하여 설정 영구 저장
   - 기본값 제공 (첫 실행 시)

### GigaMesh 코드 참고
- **CLI 실행**: `cli/gigamesh-clean.cpp:main()` 함수 참고
- **메시 I/O**: `core/mesh/MeshIO/` 폴더 참고
- **색상 관리**: `gui/src/QGMMainWindow.cpp` 기존 색상 코드 참고

---

## 🔗 관련 문서

- **전체 개발 계획**: `A:\1105\.claude\DongArch3D_개발계획_v3_Arch3DLiner기반.md` (Phase 0 섹션)
- **설계 원칙**: `A:\1105\docs\dongarch3d\DESIGN_PRINCIPLES.md`
- **GigaMesh 원본 가이드**: `A:\1105\GigaMesh\CLAUDE.md`

---

## 🎉 Phase 완료 보고서

*Phase 완료 후 작성*

**완료 일자**: YYYY-MM-DD
**소요 시간**: X일 (예상 4일)

**주요 성과**:
- (작성 예정)

**어려웠던 점**:
- (작성 예정)

**다음 Phase 준비사항**:
- Phase 1 (Align) 시작 전 OpenGL 회전 변환 복습
- Ground Plane 렌더링 참고 자료 확인

---

**문서 버전**: 1.0.0
**최종 업데이트**: 2025-11-08
**작성자**: Claude Code
