# DongArch3D - 프로젝트 컨텍스트 (Core)

**최종 업데이트**: 2025-11-08
**프로젝트 버전**: v3.1 (Arch3D Liner 기반)

---

## 📊 프로젝트 개요

### 기본 정보
- **프로젝트명**: DongArch3D (Dongguk Archaeological 3D Measurement System)
- **영문명**: Dongguk Archaeological 3D Measurement System
- **목표**: 고고학 실측 전문 프로그램 (3D → 2D 도면 생성)
- **기반**: GigaMesh v1.0+ (GPL v3)
- **참조 소프트웨어**: Arch3D Liner (Carrotphant)
- **개발 기간**: 70일 (59일 실작업 + 11일 버퍼)

### 핵심 원칙
> **"GigaMesh 강점 활용 + Arch3D Liner 워크플로우"**

1. ✅ **GigaMesh 기존 알고리즘 최대 활용** (70% 이미 구현됨!)
2. 🟢 **Arch3D Liner 워크플로우 구현** (3D → 2D 도면 생성)
3. 🟡 **한국어 UI/UX** (고고학자 친화적)

---

## 🎯 핵심 목표

### 완성된 워크플로우
```
3D 파일 열기 → Align (정렬) → Cutline (단면) → Outline (외곽)
→ Clip (절단) → Vis (X-Ray/D-Tak) → SVG 내보내기 → Adobe Illustrator
```

### 9가지 핵심 기능
1. ✅ **3D 메시 후처리** - GigaMesh gigamesh-clean 활용
2. ✅ **3D 뷰어 및 렌더링** - GigaMesh NPR 확장
3. 🟢 **Align (정렬)** - Rotation, ViewPoint, Ground Plane
4. 🟡 **Cutline (단면 라인 추출)** ← **핵심 기술** ⚠️
5. 🟡 **Outline (외곽 라인 추출)** ← **핵심 기술** ⚠️
6. 🟢 **Clip (3D 절단)** - GigaMesh splitMesh() 활용
7. 🟡 **Vis (시각화)** - X-Ray, D-Tak
8. ✅ **SVG 내보내기** - GigaMesh 기존 기능
9. 🟢 **Adobe Illustrator 연동** - 1:1 스케일 유지

---

## 🔧 GigaMesh 알고리즘 활용 현황

### ✅ 완전 구현됨 - 바로 사용 가능

| 기능 | GigaMesh 소스 | 활용 Phase |
|------|---------------|-----------|
| **3D 파일 I/O** | `core/mesh/MeshIO/*Reader.cpp` | Phase 0 |
| **Mesh-Plane Intersection** | `mesh.cpp:4041` - `calcIntersectionPolylineWithPlane()` | **Phase 2 (Cutline)** |
| **Plane 클래스** | `plane.cpp` | Phase 1, 2, 4 |
| **PolyLine 클래스** | `polyline.cpp:52` | **Phase 2 (Cutline)** |
| **Mesh Split** | `mesh.cpp:4017` - `splitMesh()` | **Phase 4 (Clip)** |
| **Geodesic Distance** | `edgegeodesic.cpp` | **Phase 5 (D-Tak)** |
| **Sobel Edge Detection** | `NPR_ApplySobel.frag` | Phase 3 (Outline 참고) |
| **3D 메시 후처리** | `gigamesh-clean.cpp` | Phase 0 |
| **SVG 내보내기** | `MeshWriter.cpp` | Phase 7 |

**결론**:
> **GigaMesh는 Arch3D Liner 핵심 알고리즘의 70%를 이미 구현!**
> - Mesh-Plane Intersection ✅
> - Mesh Split ✅
> - Geodesic Distance ✅
> - 개발 시간 **20일 단축** (22% 빠름) 🚀

### 🟡 새로 구현 필요 (30%)

| 기능 | 알고리즘 | 예상 시간 |
|------|----------|----------|
| **Line Simplification** | Douglas-Peucker | 3일 |
| **Curve Fitting** | Catmull-Rom Spline | 3일 |
| **CPU Silhouette Detection** | View-dependent edge detection | 3일 |

---

## 📅 개발 계획 (Phase 0-8, 70일)

### **Phase 0: 기반 기능 강화 (4일)**
- **Task 0.1**: gigamesh-clean GUI 통합 (2일)
- **Task 0.2**: 3D 파일 드래그 앤 드롭 (1일)
- **Task 0.3**: 기본 Settings UI (1일)
- **GigaMesh 활용**: gigamesh-clean CLI 래핑

### **Phase 1: Align (정렬) 시스템 (10일)**
- **Task 1.1**: Rotation UI (3일)
- **Task 1.2**: Ground Plane 표시 (2일)
- **Task 1.3**: ViewPoint 버튼 (2일)
- **Task 1.4**: Fit Ground Plane (자동 정렬) (3일)

### **Phase 2: Cutline (단면 라인 추출) (8일)** ⚠️ **핵심**
- **Task 2.1**: Mesh-Plane Intersection (2일) ✅ GigaMesh 활용
  - `mesh.cpp:4041` - `calcIntersectionPolylineWithPlane()`
  - Triangle-Plane intersection 계산
  - 연속된 3D 교차점 생성
- **Task 2.2**: 3D → 2D 투영 (1일)
- **Task 2.3**: Line Simplification (3일) 🟡 Douglas-Peucker 구현
- **Task 2.4**: Spline Curve Fitting (3일) 🟡 Catmull-Rom Spline 구현
- **Task 2.5**: Cutline UI (2일)
  - Top Cut / Front Cut / Right Cut 탭
  - Detail Lv, Curve Lv 조정
  - Save Slots (5개)

**개발 시간**: 15일 → **8일** (7일 단축, 47% 빠름!)

### **Phase 3: Outline (외곽 라인 추출) (8일)** ⚠️ **핵심**
- **Task 3.1**: Silhouette Edge Detection (5일) 🟡 CPU 버전 구현
- **Task 3.2**: 벡터화 (3일)
- **Task 3.3**: Outline UI (2일)

### **Phase 4: Clip (3D 절단) (2일)**
- **Task 4.1**: OpenGL Clipping Plane (2일) ✅ GigaMesh splitMesh() 활용
- **Task 4.2**: Free Clip UI (1일)
- **Task 4.3**: Recall Cutline/Outline (1일)

**개발 시간**: 5일 → **2일** (3일 단축, 60% 빠름!)

### **Phase 5: Vis (시각화) (7일)**
- **Task 5.1**: X-Ray 렌더링 (4일)
  - GLSL 셰이더 작성
  - NPR Hatching 셰이더 참고
- **Task 5.2**: D-Tak (디지털 탁본) (3일) ✅ EdgeGeodesic 활용
  - 곡률 기반 색상 매핑
  - Tiled Capture (4x4, 9x9)

**개발 시간**: 10일 → **7일** (3일 단축, 30% 빠름!)

### **Phase 6: MFE (Mini File Explorer) (5일)**
- **Task 6.1**: 파일 탐색기 UI (2일)
- **Task 6.2**: 3D Model Information (1일)
- **Task 6.3**: SVG Drag & Drop (2일)

### **Phase 7: Adobe Illustrator 연동 (5일)**
- **Task 7.1**: SVG Export 개선 (3일) ✅ GigaMesh SVG export 활용
- **Task 7.2**: Align 기능 (2일)

### **Phase 8: 최종 통합 및 테스트 (10일)**
- **Task 8.1**: 워크플로우 테스트 (5일)
- **Task 8.2**: 성능 최적화 (3일)
- **Task 8.3**: 사용자 매뉴얼 (2일)

### **버퍼 (11일)**
- 예비 일정 (위험 관리)

---

## 📊 개발 일정 요약

| Phase | 내용 | 기간 | GigaMesh 활용 | 누적 일수 |
|-------|------|------|---------------|----------|
| Phase 0 | 기반 기능 강화 | 4일 | gigamesh-clean | 4일 |
| Phase 1 | Align (정렬) | 10일 | - | 14일 |
| Phase 2 | **Cutline (단면)** ⚠️ | **8일** | ✅ Mesh-Plane Intersection | **22일** |
| Phase 3 | **Outline (외곽)** ⚠️ | **8일** | 🟡 Sobel shader 참고 | **30일** |
| Phase 4 | Clip (절단) | 2일 | ✅ splitMesh() | 32일 |
| Phase 5 | Vis (시각화) | 7일 | ✅ EdgeGeodesic + NPR | 39일 |
| Phase 6 | MFE (파일 탐색기) | 5일 | - | 44일 |
| Phase 7 | Illustrator 연동 | 5일 | SVG export | 49일 |
| Phase 8 | 최종 통합 및 테스트 | 10일 | - | 59일 |
| **버퍼** | 예비 일정 | 11일 | - | **70일** |

**총 개발 기간**: **70일 (약 2.3개월)**

**핵심 단축 요인**:
1. ✅ **Phase 2 Cutline**: 7일 단축
2. ✅ **Phase 4 Clip**: 3일 단축
3. ✅ **Phase 5 Vis**: 3일 단축

---

## ⚠️ 핵심 기술 도전 과제

### 1. Cutline (단면 라인 추출) - 가장 어려운 부분
**난이도**: ★★★★★

**GigaMesh 활용**:
```cpp
// A:\1105\GigaMesh\core\mesh\mesh.cpp:4041
bool Mesh::calcIntersectionPolylineWithPlane(
    const Vector3D& planeHNF,
    std::vector<Vector3D>* rIntersectionPoints
);
```

**추가 구현**:
- Douglas-Peucker 알고리즘 (Line simplification)
- Catmull-Rom Spline (Curve fitting)

### 2. Outline (외곽 라인 추출) - 두 번째 어려운 부분
**난이도**: ★★★★☆

**구현 방법**:
- Silhouette Edge Detection (CPU 버전)
- View-dependent edge detection
- GigaMesh Sobel shader 참고

### 3. X-Ray / D-Tak 렌더링
**난이도**: ★★★☆☆

**GigaMesh 활용**:
- NPR Hatching 셰이더 구조 참고
- EdgeGeodesic 곡률 계산 활용

---

## 🎯 현재 개발 상태

### 진행 상황
- **현재 Phase**: Phase 0 (계획 단계)
- **현재 Task**: 문서 구조 정비
- **전체 진행률**: 0% (70일 계획 시작 전)

### 완료된 작업
- [x] GigaMesh 소스코드 분석 완료
- [x] Arch3D Liner 워크플로우 분석 완료
- [x] 70일 개발 계획 수립 (v3.1)
- [x] GigaMesh 알고리즘 매핑 완료

### 다음 작업
- [ ] Phase 0 시작: gigamesh-clean GUI 통합
- [ ] 3D 파일 I/O 테스트
- [ ] 기본 Settings UI 구현

---

## 🚀 개발 우선순위

### **필수 (Must Have)** - Phase 0-4 (32일)
1. ✅ 3D 파일 I/O + 후처리
2. 🟢 Align (정렬)
3. 🟡 **Cutline (단면)** ← **최우선**
4. 🟡 **Outline (외곽)** ← **최우선**
5. 🟢 Clip (절단)
6. ✅ SVG Export

### **중요 (Should Have)** - Phase 5-6 (12일)
7. 🟡 X-Ray / D-Tak
8. 🟢 MFE 파일 탐색기

### **선택 (Nice to Have)** - Phase 7-8 (15일)
9. 🟢 Adobe Illustrator 연동
10. 🟢 최종 통합 및 테스트

---

## 📊 성공 기준

### **MVP (45일 목표)**
1. ✅ 3D 파일 열기 (PLY, OBJ, STL)
2. ✅ 메시 후처리
3. 🟢 Align (6방향 ViewPoint)
4. 🟡 **Cutline (Top/Front/Right 단면)**
5. 🟡 **Outline (6방향 외곽선)**
6. ✅ SVG 내보내기

### **Full (70일 목표)**
7. 🟢 Clip (3D 절단)
8. 🟡 X-Ray 렌더링
9. 🟡 D-Tak 렌더링
10. 🟢 MFE + Adobe Illustrator 연동

---

## 🔧 프로젝트 구조

### 주요 디렉토리
```
A:\1105\
├── .dongarch3d-progress.json          # 진행 상황 추적
├── .claude/
│   └── DongArch3D_개발계획_v3_Arch3DLiner기반.md  # 전체 개발 계획
├── docs/dongarch3d/
│   ├── CONTEXT_CORE.md                # 이 파일 (핵심 컨텍스트)
│   ├── DESIGN_PRINCIPLES.md           # 설계 원칙
│   └── implementation_reports/        # Phase별 구현 보고서
│       ├── Phase_0_Overview.md
│       ├── Phase_1_Overview.md
│       ├── Phase_2_Overview.md (Cutline) ⭐
│       └── ...
└── GigaMesh/
    ├── core/mesh/
    │   ├── mesh.cpp                   # Mesh-Plane Intersection
    │   ├── plane.cpp                  # Plane 클래스
    │   ├── polyline.cpp               # PolyLine 클래스
    │   └── edgegeodesic.cpp           # Geodesic Distance
    ├── cli/
    │   └── gigamesh-clean.cpp         # 메시 후처리 CLI
    └── gui/src/shaders/NPR/
        └── NPR_ApplySobel.frag        # Sobel Edge Detection
```

---

## 💡 핵심 전략

### GigaMesh 강점 활용
1. **Mesh-Plane Intersection** → Cutline 개발 시간 7일 단축
2. **splitMesh()** → Clip 개발 시간 3일 단축
3. **EdgeGeodesic** → D-Tak 개발 시간 3일 단축
4. **gigamesh-clean** → 메시 후처리 1일 단축

### 위험 관리
1. **Phase 2 (Cutline)** 프로토타입 먼저 개발
2. **Phase 3 (Outline)** 프로토타입 먼저 개발
3. 11일 버퍼로 예상치 못한 문제 대응

### 한국어 UI/UX
- 모든 새 기능은 100% 한국어
- 고고학자 친화적 용어 사용
- 직관적인 메뉴 구조

---

## 📝 참고 문서

### 필수 문서
- **전체 개발 계획**: `A:\1105\.claude\DongArch3D_개발계획_v3_Arch3DLiner기반.md`
- **설계 원칙**: `A:\1105\docs\dongarch3d\DESIGN_PRINCIPLES.md`
- **프로젝트 가이드**: `A:\1105\CLAUDE.md`

### 구현 보고서
- **Phase별 보고서**: `A:\1105\docs\dongarch3d\implementation_reports/`

---

## 🎯 다음 세션을 위한 메모

**즉시 시작 가능한 상태**:
- Phase 0 준비 완료
- GigaMesh 알고리즘 매핑 완료
- 70일 개발 계획 수립 완료

**빠른 시작**:
```bash
/mvp-status        # 현재 상태 확인
/mvp-init          # 컨텍스트 복원
```

**우선 작업**:
1. Phase 0 Task 0.1 시작 (gigamesh-clean GUI 통합)
2. Cutline/Outline 알고리즘 프로토타입 (위험 관리)

---

**문서 버전**: v3.1 (Arch3D Liner 기반)
**최종 업데이트**: 2025-11-08
**작성자**: Claude Code

**핵심 메시지**:
> **"GigaMesh는 Arch3D Liner 핵심 알고리즘의 70%를 이미 구현했습니다!"**
> - 개발 시간 **20일 단축** (90일 → 70일)
> - Mesh-Plane Intersection, Mesh Split, Geodesic Distance 완전 구현
> - 바퀴를 다시 발명하지 말고, GigaMesh 강력한 기반을 활용하자!
