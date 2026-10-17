# Phase 2: Cutline (단면 라인 추출) ⚠️

**기간**: 8일 (15일 → 8일, 7일 단축!)
**우선순위**: **최우선** (핵심 기술)
**난이도**: ★★★★★ (가장 어려운 부분)
**상태**: 🔴 대기

---

## 🎯 Phase 목표

**DongArch3D의 가장 중요한 기능!**

Arch3D Liner의 핵심 기능인 **Cutline (단면 라인 추출)**을 구현합니다:
1. ✅ **Mesh-Plane Intersection** (GigaMesh 완전 구현됨!)
2. 🟡 **Line Simplification** (Douglas-Peucker 알고리즘)
3. 🟡 **Spline Curve Fitting** (Catmull-Rom Spline)
4. 🟢 **Cutline UI** (Top Cut / Front Cut / Right Cut)

**핵심 발견**:
> GigaMesh는 `calcIntersectionPolylineWithPlane()` 함수를 이미 완전히 구현했습니다!
> 개발 시간 **7일 단축** (47% 빠름) 🚀

---

## 📋 Task 목록

### Task 2.1: Mesh-Plane Intersection (2일) ✅ GigaMesh 활용

**GigaMesh 완전 구현됨**:
```cpp
// A:\1105\GigaMesh\core\mesh\mesh.cpp:4041
bool Mesh::calcIntersectionPolylineWithPlane(
    const Vector3D& planeHNF,                    // Hesse Normal Form
    std::vector<Vector3D>* rIntersectionPoints   // 출력: 3D 교차점
);

// A:\1105\GigaMesh\core\mesh\polyline.cpp:52
PolyLine::PolyLine( const Plane& rPlaneIntersecting );
```

**구현 내용** (2일):
1. **GUI 래퍼 함수 작성** (0.5일)
   ```cpp
   class DongArchCutlineManager {
   public:
       std::vector<Vector3D> computeTopCut(Mesh* mesh);
       std::vector<Vector3D> computeFrontCut(Mesh* mesh);
       std::vector<Vector3D> computeRightCut(Mesh* mesh);
   };
   ```

2. **평면 정의** (0.5일)
   ```cpp
   // Top Cut: XY 평면 (Z = 사용자 지정 높이)
   Plane topPlane(Vector3D(0, 0, 1), heightZ);

   // Front Cut: XZ 평면 (Y = 사용자 지정 깊이)
   Plane frontPlane(Vector3D(0, 1, 0), depthY);

   // Right Cut: YZ 평면 (X = 사용자 지정 위치)
   Plane rightPlane(Vector3D(1, 0, 0), positionX);
   ```

3. **교차점 계산 및 저장** (1일)
   - `calcIntersectionPolylineWithPlane()` 호출
   - Triangle-Plane intersection 계산 (GigaMesh 구현)
   - Edge 순서 정렬 (GigaMesh 구현)
   - 연속된 3D 교차점 목록 반환

**예상 산출물**:
- `gui/src/DongArchCutlineManager.h/cpp`

---

### Task 2.2: 3D → 2D 투영 (1일)

**GigaMesh Vector3D 변환 활용**:
```cpp
// 평면의 로컬 좌표계로 변환
Vector3D localX = plane.getPlaneX();
Vector3D localY = plane.getPlaneY();

for (auto& point3d : intersectionPoints) {
    double x2d = dot3(point3d, localX);
    double y2d = dot3(point3d, localY);
    points2d.push_back(Vector2D(x2d, y2d));
}
```

**구현 내용**:
1. **회전 행렬 기반 변환** (0.5일)
   - GigaMesh `Vector3D::dot3()` 활용
   - 평면 로컬 좌표계 계산

2. **2D 좌표 검증** (0.5일)
   - 2D 포인트 범위 확인
   - 이상치(outlier) 제거

**예상 산출물**:
- `DongArchCutlineManager::project3DTo2D()` 함수

---

### Task 2.3: Line Simplification (3일) 🟡 새로 구현

**Douglas-Peucker 알고리즘 구현**:
```cpp
class DongArchLineSimplifier {
public:
    std::vector<Vector2D> douglasPeucker(
        const std::vector<Vector2D>& points,
        double tolerance  // Detail Lv: 0.1mm, 0.5mm, 1mm
    );

private:
    double perpendicularDistance(
        const Vector2D& point,
        const Vector2D& lineStart,
        const Vector2D& lineEnd
    );
};
```

**구현 내용**:
1. **Douglas-Peucker 알고리즘** (2일)
   - 재귀적 line simplification
   - Perpendicular distance 계산
   - Tolerance 기반 점 제거

2. **Detail Lv 파라미터** (0.5일)
   - 0.1mm (초정밀) - tolerance = 0.1
   - 0.5mm (중간) - tolerance = 0.5
   - 1mm (빠름) - tolerance = 1.0
   - QSlider로 사용자 조정 가능

3. **성능 최적화** (0.5일)
   - 비재귀 버전 구현
   - 큰 메시 처리 최적화

**참고 자료**:
- Wikipedia: Douglas-Peucker algorithm
- C++ 구현 예제: https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm

**예상 산출물**:
- `gui/src/algorithms/DongArchLineSimplifier.h/cpp`

---

### Task 2.4: Spline Curve Fitting (3일) 🟡 새로 구현

**Catmull-Rom Spline 구현**:
```cpp
class DongArchSplineFitter {
public:
    std::vector<Vector2D> catmullRomSpline(
        const std::vector<Vector2D>& controlPoints,
        int segments  // Curve Lv: Low=10, Mid=20, High=40
    );

private:
    Vector2D catmullRomPoint(
        const Vector2D& p0, const Vector2D& p1,
        const Vector2D& p2, const Vector2D& p3,
        double t
    );
};
```

**구현 내용**:
1. **Catmull-Rom Spline 알고리즘** (2일)
   - Control points 사이 보간
   - Smooth curve 생성
   - Tension 파라미터 조정

2. **Curve Lv 파라미터** (0.5일)
   - Low (10 segments) - 빠름, 덜 부드러움
   - Mid (20 segments) - 균형
   - High (40 segments) - 느림, 매우 부드러움
   - QComboBox로 선택

3. **Polyline vs Spline 전환** (0.5일)
   - QCheckBox "Spline 활성화"
   - 실시간 미리보기

**참고 자료**:
- Catmull-Rom Spline 수식
- C++ 구현 예제

**예상 산출물**:
- `gui/src/algorithms/DongArchSplineFitter.h/cpp`

---

### Task 2.5: Cutline UI (2일)

**UI 구성**:
```
┌─────────────────────────────────────┐
│  Cutline (단면)                     │
├─────────────────────────────────────┤
│  [Top Cut] [Front Cut] [Right Cut]  │  ← QTabWidget
├─────────────────────────────────────┤
│  ┌───────────────────────────────┐  │
│  │   2D 미리보기                 │  │  ← QWidget + QPainter
│  │                               │  │
│  │   [단면 라인 표시]            │  │
│  └───────────────────────────────┘  │
├─────────────────────────────────────┤
│  Detail Lv:  [■■■■□□□□] 0.5mm  │  ← QSlider
│  Curve Lv:   [▼ Mid]                │  ← QComboBox
│  Line Width: [▼ 2px]                │  ← QSpinBox
│  [✓] Spline 활성화                  │  ← QCheckBox
├─────────────────────────────────────┤
│  Save Slots: [1][2][3][4][5]        │  ← QPushButton
│  [추출] [초기화]                     │  ← QPushButton
└─────────────────────────────────────┘
```

**구현 내용**:
1. **QTabWidget (Top/Front/Right)** (0.5일)
   - 3개 탭 구조
   - 각 탭에 독립적인 설정

2. **2D 미리보기 창** (1일)
   - QWidget + QPainter
   - 2D 라인 렌더링
   - 확대/축소 (QWheelEvent)
   - 패닝 (마우스 드래그)

3. **Line Property 컨트롤** (0.5일)
   - Detail Lv QSlider (0.1~1mm)
   - Curve Lv QComboBox (Low/Mid/High)
   - Line Width QSpinBox (1~10px)
   - Spline QCheckBox

4. **Save Slots** (JSON 저장) (0.5일)
   ```cpp
   struct CutlineData {
       std::vector<Vector2D> points;
       double detailLv;
       CurveLv curveLv;
       int lineWidth;
       bool splineEnabled;
   };
   // 5개 슬롯에 저장/불러오기
   ```

**예상 산출물**:
- `gui/src/dialogs/QGMDialogCutline.h/cpp`
- `gui/src/widgets/QGMCutlinePreview.h/cpp`

---

## 🔧 GigaMesh 알고리즘 활용

### 완전 구현됨 - 바로 사용 가능

| 기능 | GigaMesh 소스 | 단축 시간 |
|------|---------------|----------|
| **Mesh-Plane Intersection** | `mesh.cpp:4041` - `calcIntersectionPolylineWithPlane()` | **5일 → 2일** (3일 단축) |
| **Plane 클래스** | `plane.cpp` - 3점/점+법선/HNF 정의 | **2일 → 0.5일** (1.5일 단축) |
| **PolyLine 클래스** | `polyline.cpp:52` - Profile line 생성 | **1일 → 0.5일** (0.5일 단축) |
| **Vector3D 변환** | `vector3d.cpp` - dot3(), 회전 행렬 | **2일 → 1일** (1일 단축) |

**총 단축 시간**: 10일 → **4일** (6일 단축, 60%)

### 새로 구현 필요

| 기능 | 알고리즘 | 예상 시간 |
|------|----------|----------|
| **Line Simplification** | Douglas-Peucker | 3일 |
| **Curve Fitting** | Catmull-Rom Spline | 3일 |

**Phase 2 총 개발 시간**: **15일 → 8일** (7일 단축, 47% 빠름) 🚀

---

## ⏱️ 타임라인

| Day | 작업 내용 | 산출물 |
|-----|----------|--------|
| 1-2 | Mesh-Plane Intersection GUI 래퍼 | `DongArchCutlineManager` |
| 3 | 3D → 2D 투영 구현 | `project3DTo2D()` 함수 |
| 4-6 | Douglas-Peucker 알고리즘 | `DongArchLineSimplifier` |
| 7-9 | Catmull-Rom Spline 알고리즘 | `DongArchSplineFitter` |
| 10-11 | Cutline UI 구현 | `QGMDialogCutline` |

**Day 8 완료 목표**: 모든 기능 구현 및 테스트 완료

---

## ✅ 성공 기준

### Task 2.1 성공 기준
- [ ] `calcIntersectionPolylineWithPlane()` GUI에서 호출 가능
- [ ] Top/Front/Right 평면 정의 정확
- [ ] 연속된 3D 교차점 생성
- [ ] 빈 교차점 경우 처리

### Task 2.2 성공 기준
- [ ] 3D → 2D 투영 정확
- [ ] 2D 좌표 범위 검증
- [ ] 이상치 제거 동작

### Task 2.3 성공 기준
- [ ] Douglas-Peucker 알고리즘 동작
- [ ] Detail Lv 0.1mm, 0.5mm, 1mm 테스트 완료
- [ ] 큰 메시(10만+ vertices) 처리 가능

### Task 2.4 성공 기준
- [ ] Catmull-Rom Spline 생성
- [ ] Curve Lv Low/Mid/High 테스트 완료
- [ ] Polyline ↔ Spline 전환 동작

### Task 2.5 성공 기준
- [ ] Top/Front/Right 탭 전환 동작
- [ ] 2D 미리보기 렌더링
- [ ] 확대/축소/패닝 동작
- [ ] Save Slots 5개 저장/불러오기 동작

---

## 📝 구현 노트

### 핵심 도전 과제

1. **Mesh-Plane Intersection**:
   - ✅ GigaMesh 구현 완료 - 래핑만 필요!
   - 주의: Triangle이 평면과 정확히 일치하는 경우 처리
   - 주의: 교차점이 없는 경우 처리

2. **Douglas-Peucker 알고리즘**:
   - 재귀 깊이 제한 (큰 메시)
   - Tolerance 값에 따른 성능 차이 큼
   - **참고**: OpenCV `approxPolyDP()` 구현 참고 가능

3. **Catmull-Rom Spline**:
   - Control points 양 끝 처리 (phantom points)
   - Tension 파라미터 조정 (0.5 기본값)
   - Overshoot 방지

4. **2D 미리보기**:
   - QPainter 성능 최적화
   - 큰 polyline 렌더링 시 프레임 드롭 방지
   - Anti-aliasing 설정

### GigaMesh 코드 참고

**필수 읽기**:
```
A:\1105\GigaMesh\core\mesh\mesh.cpp:4041-4120
  → calcIntersectionPolylineWithPlane() 구현

A:\1105\GigaMesh\core\mesh\plane.cpp:52-150
  → Plane 클래스 생성자 및 메서드

A:\1105\GigaMesh\core\mesh\polyline.cpp:52-200
  → PolyLine 클래스 구현
```

### 성능 최적화 팁

1. **Mesh-Plane Intersection**:
   - Octree spatial index 활용 (GigaMesh 제공)
   - Triangle 수가 많으면 멀티스레딩

2. **Douglas-Peucker**:
   - 비재귀 버전 (스택 기반)
   - SIMD 최적화 (AVX2)

3. **UI 렌더링**:
   - VBO 사용 (OpenGL)
   - LOD (Level of Detail) 적용

---

## 🚨 위험 관리

### 높은 위험 요소

1. **Douglas-Peucker 알고리즘 복잡도**: ⚠️ 높음
   - 완화: 프로토타입 먼저 작성 (단순 버전)
   - 완화: 기존 라이브러리 참고 (OpenCV)

2. **Catmull-Rom Spline 수학**: ⚠️ 중간
   - 완화: 수식 검증 (Wikipedia)
   - 완화: 시각적 테스트 (곡선 모양 확인)

3. **2D 미리보기 성능**: ⚠️ 중간
   - 완화: QPainter 최적화
   - 완화: VBO 렌더링 (OpenGL)

### 대응 계획

- **프로토타입 우선**: Task 2.3, 2.4 프로토타입 먼저 작성
- **알고리즘 검증**: 간단한 테스트 케이스로 검증
- **버퍼 활용**: 11일 버퍼 중 3일 Phase 2에 할당 가능

---

## 🔗 관련 문서

- **전체 개발 계획**: `A:\1105\.claude\DongArch3D_개발계획_v3_Arch3DLiner기반.md` (Phase 2 섹션)
- **GigaMesh mesh.cpp**: `A:\1105\GigaMesh\core\mesh\mesh.cpp:4041`
- **GigaMesh plane.cpp**: `A:\1105\GigaMesh\core\mesh\plane.cpp`
- **GigaMesh polyline.cpp**: `A:\1105\GigaMesh\core\mesh\polyline.cpp`

---

## 🎉 Phase 완료 보고서

*Phase 완료 후 작성*

**완료 일자**: YYYY-MM-DD
**소요 시간**: X일 (예상 8일)

**주요 성과**:
- (작성 예정)

**어려웠던 점**:
- (작성 예정)

**다음 Phase 준비사항**:
- Phase 3 (Outline) 시작 전 Silhouette Edge Detection 알고리즘 복습

---

**문서 버전**: 1.0.0
**최종 업데이트**: 2025-11-08
**작성자**: Claude Code

**핵심 메시지**:
> **Phase 2는 DongArch3D의 가장 중요한 Phase입니다!**
> GigaMesh 덕분에 7일 단축 (15일 → 8일)
> Cutline 없이는 Arch3D Liner 워크플로우 불가능!
