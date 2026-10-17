# DongArch3D C++ 구현 참고서

**작성일**: 2025-11-10
**대상**: DongArch3D Phase 2-7 C++ 구현
**기반**: GigaMesh v1.0+ API

---

## 목차

1. [GigaMesh 핵심 API](#gigamesh-핵심-api)
2. [Phase 2: Cutline C++ 구현](#phase-2-cutline-c-구현)
3. [Phase 3: Outline C++ 구현](#phase-3-outline-c-구현)
4. [Phase 4: Clip C++ 구현](#phase-4-clip-c-구현)
5. [Phase 5: D-Tak/X-Ray C++ 구현](#phase-5-d-takx-ray-c-구현)
6. [Phase 6: MFE C++ 구현](#phase-6-mfe-c-구현)
7. [Phase 7: Illustrator C++ 구현](#phase-7-illustrator-c-구현)
8. [C++20 패턴 및 베스트 프랙티스](#c20-패턴-및-베스트-프랙티스)

---

## GigaMesh 핵심 API

### 1. Mesh::calcIntersectionPolylineWithPlane()

**위치**: `GigaMesh/core/mesh/mesh.cpp:4041`

**시그니처**:
```cpp
bool Mesh::calcIntersectionPolylineWithPlane(
    const Vector3D& planeHNF,
    std::vector<Vector3D>* rIntersectionPoints
);
```

**파라미터**:
- `planeHNF`: Hesse Normal Form 평면 (x, y, z, d)
- `rIntersectionPoints`: 교차점 결과 저장 (출력)

**반환값**:
- `true`: 교차점 발견
- `false`: 교차점 없음 또는 에러

**사용 예제**:
```cpp
#include "mesh/mesh.h"
#include "mesh/plane.h"

// Top view 평면 (Z=10 위치)
Vector3D planeHNF(0.0, 0.0, 1.0, -10.0);  // ax + by + cz + d = 0

std::vector<Vector3D> intersectionPoints;
bool success = mMesh->calcIntersectionPolylineWithPlane(planeHNF, &intersectionPoints);

if (success && !intersectionPoints.empty()) {
    std::cout << "Found " << intersectionPoints.size() << " intersection points\n";

    // 점들 사용
    for (const auto& point : intersectionPoints) {
        double x = point.getX();
        double y = point.getY();
        double z = point.getZ();
        // ...
    }
}
```

**중요 사항**:
- 평면은 Hesse Normal Form으로 정의: `n·x + d = 0`
- 반환되는 점들은 연속된 polyline 순서
- 여러 개의 분리된 라인이 있을 수 있음

---

### 2. Mesh::splitMesh()

**위치**: `GigaMesh/core/mesh/mesh.cpp:4149`

**시그니처**:
```cpp
bool Mesh::splitMesh(
    const std::function<bool(Face*)>& intersectTest,
    const std::function<double(VertexOfFace*)>& signedDistanceFunction,
    const std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>& getIntersectionVector,
    bool duplicateVertices,
    bool noRedraw,
    Vector3D rUniformOffset
);
```

**파라미터**:
- `intersectTest`: Face가 평면과 교차하는지 판정하는 함수
- `signedDistanceFunction`: Vertex의 평면까지 signed distance
- `getIntersectionVector`: 두 Vertex 사이 교차점 계산
- `duplicateVertices`: 정점 복제 여부
- `noRedraw`: 재렌더링 방지
- `rUniformOffset`: 오프셋

**사용 예제**:
```cpp
#include "mesh/mesh.h"
#include "mesh/plane.h"

// 평면 정의 (Z=0)
Vector3D planeNormal(0, 0, 1);
double planeD = 0.0;

// Face 교차 판정
auto intersectTest = [&](Face* face) -> bool {
    // Face의 3개 정점이 평면 양쪽에 있는지 확인
    int posCount = 0, negCount = 0;
    for (int i = 0; i < 3; i++) {
        Vertex* v = face->getVertexA(); // B, C도 확인
        double dist = planeNormal.dot(v->getPositionVector()) + planeD;
        if (dist > 0) posCount++;
        else negCount++;
    }
    return (posCount > 0 && negCount > 0);
};

// Signed distance 계산
auto distFun = [&](VertexOfFace* vof) -> double {
    Vector3D pos = vof->getPosition();
    return planeNormal.dot(pos) + planeD;
};

// 교차점 계산
auto getIntersectionVec = [&](VertexOfFace* v1, VertexOfFace* v2, Vector3D& result) {
    Vector3D p1 = v1->getPosition();
    Vector3D p2 = v2->getPosition();
    double d1 = distFun(v1);
    double d2 = distFun(v2);

    double t = d1 / (d1 - d2);
    result = p1 + (p2 - p1) * t;
};

// 메시 분할 실행
bool success = mMesh->splitMesh(intersectTest, distFun, getIntersectionVec, true, false, Vector3D(0,0,0));
```

**중요 사항**:
- 메시를 두 개로 분할 (원본 + 새 메시)
- 복잡한 함수형 프로그래밍 패턴 사용
- 람다 함수로 커스터마이징 가능

---

### 3. EdgeGeodesic (Geodesic Distance)

**위치**: `GigaMesh/core/mesh/edgegeodesic.cpp`

**헤더**: `GigaMesh/core/include/GigaMesh/mesh/edgegeodesic.h`

**주요 메서드**:
```cpp
class EdgeGeodesic {
public:
    double getGeoDistA();  // Vertex A로부터 측지선 거리
    double getGeoDistB();  // Vertex B로부터 측지선 거리

    // ...
};
```

**사용 예제**:
```cpp
#include "mesh/edgegeodesic.h"

// D-Tak (디지털 탁본) 계산
std::vector<double> computeDTak(Mesh* mesh, Vertex* sourceVertex) {
    std::vector<double> distances;

    // 모든 Edge에 대해 geodesic distance 계산
    for (Edge* edge : mesh->getEdges()) {
        EdgeGeodesic* geodesic = /* edge에서 가져오기 */;

        double dist = geodesic->getGeoDistA();
        distances.push_back(dist);
    }

    return distances;
}
```

**중요 사항**:
- CPU 기반 계산 (느림)
- GPU 버전 구현 필요 (Phase 5)
- Heat Method로 대체 가능

---

### 4. Plane 클래스

**위치**: `GigaMesh/core/mesh/plane.cpp`

**헤더**: `GigaMesh/core/include/GigaMesh/mesh/plane.h`

**사용 예제**:
```cpp
#include "mesh/plane.h"

// 방법 1: 3점으로 평면 정의
Vector3D p1(0, 0, 0);
Vector3D p2(1, 0, 0);
Vector3D p3(0, 1, 0);
Plane plane(p1, p2, p3);

// 방법 2: 법선과 점으로 정의
Vector3D normal(0, 0, 1);
Vector3D point(0, 0, 10);
Plane plane2(normal, point);

// HNF (Hesse Normal Form) 가져오기
Vector3D planeHNF = plane.getHNF();
```

**주요 메서드**:
- `getHNF()`: Hesse Normal Form 반환
- `distanceToPoint(Vector3D)`: 점까지 거리
- `getNormal()`: 법선 벡터

---

### 5. PolyLine 클래스

**위치**: `GigaMesh/core/mesh/polyline.cpp`

**헤더**: `GigaMesh/core/include/GigaMesh/mesh/polyline.h`

**사용 예제**:
```cpp
#include "mesh/polyline.h"

// 평면으로부터 PolyLine 생성
Plane plane(/* ... */);
PolyLine polyline(plane);

// PolyLine의 점들 가져오기
std::vector<Vector3D> points = polyline.getVertexPositions();
```

---

### 6. MeshWriter (SVG Export)

**위치**: `GigaMesh/core/mesh/MeshWriter.cpp`

**사용 예제**:
```cpp
#include "mesh/MeshWriter.h"

// SVG로 저장
MeshWriter writer;
bool success = writer.writeSVG(mesh, "output.svg");
```

---

## Phase 2: Cutline C++ 구현

### 전체 플로우

```cpp
// DongArchCutlineManager.cpp

class CutlineManager : public QObject {
    Q_OBJECT

private:
    Mesh* mMesh;

public:
    bool extractCutline(const Vector3D& planeHNF, CutlineResult& result) {
        // 1. GigaMesh API로 교차선 추출
        std::vector<Vector3D> rawPoints;
        bool success = mMesh->calcIntersectionPolylineWithPlane(planeHNF, &rawPoints);

        if (!success || rawPoints.empty()) {
            return false;
        }

        // 2. Douglas-Peucker 간략화
        std::vector<Vector3D> simplifiedPoints =
            douglasPeucker(rawPoints, tolerance);

        // 3. Catmull-Rom 스플라인 적용
        std::vector<Vector3D> smoothedPoints =
            catmullRomSpline(simplifiedPoints, segments);

        // 4. 결과 저장
        result.points = smoothedPoints;
        result.count = smoothedPoints.size();

        return true;
    }
};
```

### Douglas-Peucker 알고리즘 구현

**파일**: `GigaMesh/gui/src/dongarch/cutline/algorithms/DouglasPeucker.cpp`

```cpp
#include <vector>
#include <cmath>
#include <span>  // C++20

namespace DongArch::Cutline::Algorithms {

class DouglasPeucker {
public:
    // 3D 점 간략화
    std::vector<Vector3D> simplify(
        std::span<const Vector3D> points,
        double tolerance
    ) const {
        if (points.size() < 3) {
            return std::vector<Vector3D>(points.begin(), points.end());
        }

        std::vector<bool> keep(points.size(), false);
        keep[0] = true;  // 시작점
        keep[points.size()-1] = true;  // 끝점

        // 재귀적 간략화
        simplifyRecursive(points, 0, points.size()-1, tolerance, keep);

        // 유지할 점들만 필터링
        std::vector<Vector3D> result;
        for (size_t i = 0; i < points.size(); i++) {
            if (keep[i]) {
                result.push_back(points[i]);
            }
        }

        return result;
    }

private:
    void simplifyRecursive(
        std::span<const Vector3D> points,
        size_t start,
        size_t end,
        double tolerance,
        std::vector<bool>& keep
    ) const {
        if (end - start <= 1) {
            return;
        }

        // 시작-끝 직선에서 가장 먼 점 찾기
        double maxDist = 0.0;
        size_t maxIndex = start;

        for (size_t i = start + 1; i < end; i++) {
            double dist = perpendicularDistance(
                points[i],
                points[start],
                points[end]
            );

            if (dist > maxDist) {
                maxDist = dist;
                maxIndex = i;
            }
        }

        // tolerance보다 크면 분할
        if (maxDist > tolerance) {
            keep[maxIndex] = true;
            simplifyRecursive(points, start, maxIndex, tolerance, keep);
            simplifyRecursive(points, maxIndex, end, tolerance, keep);
        }
    }

    // 점에서 직선까지 수직 거리
    double perpendicularDistance(
        const Vector3D& point,
        const Vector3D& lineStart,
        const Vector3D& lineEnd
    ) const {
        Vector3D line = lineEnd - lineStart;
        Vector3D toPoint = point - lineStart;

        // Cross product magnitude = area of parallelogram
        Vector3D cross = line.cross(toPoint);
        double area = cross.length();
        double base = line.length();

        return area / base;  // height = area / base
    }
};

} // namespace
```

### Catmull-Rom Spline 구현

**파일**: `GigaMesh/gui/src/dongarch/cutline/algorithms/CatmullRomSpline.cpp`

```cpp
#include <vector>
#include <span>  // C++20

namespace DongArch::Cutline::Algorithms {

class CatmullRomSpline {
public:
    // Centripetal Catmull-Rom 스플라인
    std::vector<Vector3D> interpolate(
        std::span<const Vector3D> controlPoints,
        int segmentsPerCurve = 20
    ) const {
        if (controlPoints.size() < 4) {
            return std::vector<Vector3D>(controlPoints.begin(), controlPoints.end());
        }

        std::vector<Vector3D> result;
        result.reserve((controlPoints.size() - 3) * segmentsPerCurve);

        // 각 4개 제어점 세그먼트
        for (size_t i = 0; i < controlPoints.size() - 3; i++) {
            Vector3D p0 = controlPoints[i];
            Vector3D p1 = controlPoints[i+1];
            Vector3D p2 = controlPoints[i+2];
            Vector3D p3 = controlPoints[i+3];

            // Alpha = 0.5 (centripetal)
            double t0 = 0.0;
            double t1 = getT(t0, p0, p1, 0.5);
            double t2 = getT(t1, p1, p2, 0.5);
            double t3 = getT(t2, p2, p3, 0.5);

            // 보간
            for (int j = 0; j < segmentsPerCurve; j++) {
                double t = t1 + (t2 - t1) * (j / double(segmentsPerCurve));

                Vector3D A1 = interpolatePoint(p0, p1, t0, t1, t);
                Vector3D A2 = interpolatePoint(p1, p2, t1, t2, t);
                Vector3D A3 = interpolatePoint(p2, p3, t2, t3, t);

                Vector3D B1 = interpolatePoint(A1, A2, t0, t2, t);
                Vector3D B2 = interpolatePoint(A2, A3, t1, t3, t);

                Vector3D C = interpolatePoint(B1, B2, t1, t2, t);

                result.push_back(C);
            }
        }

        return result;
    }

private:
    double getT(double t, const Vector3D& p0, const Vector3D& p1, double alpha) const {
        Vector3D d = p1 - p0;
        double a = d.dot(d);  // |p1 - p0|^2
        double b = std::pow(a, alpha * 0.5);
        return b + t;
    }

    Vector3D interpolatePoint(
        const Vector3D& p0,
        const Vector3D& p1,
        double t0,
        double t1,
        double t
    ) const {
        if (t1 - t0 < 1e-6) {
            return p0;
        }

        double s = (t - t0) / (t1 - t0);
        return p0 * (1.0 - s) + p1 * s;
    }
};

} // namespace
```

### UI 연결

**파일**: `GigaMesh/gui/src/dongarch/cutline/DongArchCutlineDialog.cpp`

```cpp
#include "DongArchCutlineDialog.h"
#include "DongArchCutlineManager.h"
#include <QSlider>
#include <QPushButton>

class DongArchCutlineDialog : public QDialog {
    Q_OBJECT

private:
    CutlineManager* mManager;
    QSlider* mDetailSlider;
    QSlider* mCurveSlider;

public:
    DongArchCutlineDialog(Mesh* mesh, QWidget* parent = nullptr)
        : QDialog(parent)
        , mManager(new CutlineManager(mesh, this))
    {
        setupUI();
        connectSignals();
    }

private:
    void setupUI() {
        // Detail Level 슬라이더 (0.1mm ~ 1mm)
        mDetailSlider = new QSlider(Qt::Horizontal);
        mDetailSlider->setRange(1, 10);  // 0.1mm 단위
        mDetailSlider->setValue(5);  // 기본 0.5mm

        // Curve Level 슬라이더
        mCurveSlider = new QSlider(Qt::Horizontal);
        mCurveSlider->setRange(1, 3);  // Low, Mid, High
        mCurveSlider->setValue(2);  // 기본 Mid

        // Extract 버튼
        QPushButton* btnExtract = new QPushButton("Extract Cutline");
        connect(btnExtract, &QPushButton::clicked, this, &DongArchCutlineDialog::onExtract);
    }

private slots:
    void onExtract() {
        // Top view 평면 (Z=10)
        Vector3D planeHNF(0, 0, 1, -10.0);

        double tolerance = mDetailSlider->value() * 0.1;  // 0.1mm 단위
        int segments = mCurveSlider->value() * 10 + 10;  // 10, 20, 30

        CutlineResult result;
        bool success = mManager->extractCutline(planeHNF, tolerance, segments, result);

        if (success) {
            // 2D 프리뷰 업데이트
            updatePreview(result);
        }
    }
};
```

---

## Phase 3: Outline C++ 구현

### CPU 기반 Silhouette 감지

**파일**: `GigaMesh/gui/src/dongarch/outline/SilhouetteDetector.cpp`

```cpp
namespace DongArch::Outline {

class SilhouetteDetector {
private:
    Mesh* mMesh;

public:
    // 특정 방향에서 silhouette edge 추출
    std::vector<Edge*> detectSilhouette(const Vector3D& viewDirection) {
        std::vector<Edge*> silhouetteEdges;

        // 모든 Edge 검사
        for (Edge* edge : mMesh->getEdges()) {
            if (isSilhouetteEdge(edge, viewDirection)) {
                silhouetteEdges.push_back(edge);
            }
        }

        return silhouetteEdges;
    }

private:
    bool isSilhouetteEdge(Edge* edge, const Vector3D& viewDir) {
        // Edge의 인접한 두 Face 가져오기
        Face* face1 = edge->getFaceA();
        Face* face2 = edge->getFaceB();

        if (!face1 || !face2) {
            return false;  // Border edge
        }

        // 각 Face의 법선
        Vector3D n1 = face1->getNormal();
        Vector3D n2 = face2->getNormal();

        // 한쪽은 앞, 한쪽은 뒤 → silhouette
        double dot1 = n1.dot(viewDir);
        double dot2 = n2.dot(viewDir);

        return (dot1 * dot2 < 0.0);
    }
};

} // namespace
```

### GPU 기반 Silhouette (Geometry Shader)

**파일**: `GigaMesh/gui/src/shaders/silhouette.geom`

```glsl
#version 330 core

layout(triangles_adjacency) in;
layout(line_strip, max_vertices = 6) out;

uniform vec3 viewDirection;

in vec3 gNormal[];  // Face normals from vertex shader

void main() {
    // 각 edge 검사 (3개)
    for (int i = 0; i < 3; i++) {
        int i0 = i;
        int i1 = (i + 1) % 3;
        int i2 = i + 3;  // adjacent triangle

        vec3 n1 = gNormal[i0];  // Current face
        vec3 n2 = gNormal[i2];  // Adjacent face

        float dot1 = dot(n1, viewDirection);
        float dot2 = dot(n2, viewDirection);

        // Silhouette edge
        if (dot1 * dot2 < 0.0) {
            gl_Position = gl_in[i0].gl_Position;
            EmitVertex();

            gl_Position = gl_in[i1].gl_Position;
            EmitVertex();

            EndPrimitive();
        }
    }
}
```

**C++ 연결 코드**:

```cpp
// OutlineExtractor.cpp

class OutlineExtractor {
private:
    QOpenGLShaderProgram* mSilhouetteShader;

public:
    void initializeGL() {
        mSilhouetteShader = new QOpenGLShaderProgram();
        mSilhouetteShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/silhouette.vert");
        mSilhouetteShader->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/silhouette.geom");
        mSilhouetteShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/silhouette.frag");
        mSilhouetteShader->link();
    }

    void extractGPU(const Vector3D& viewDir) {
        mSilhouetteShader->bind();
        mSilhouetteShader->setUniformValue("viewDirection",
            QVector3D(viewDir.getX(), viewDir.getY(), viewDir.getZ()));

        // Render mesh with adjacency
        renderMeshWithAdjacency();

        mSilhouetteShader->release();
    }
};
```

---

## Phase 4: Clip C++ 구현

**파일**: `GigaMesh/gui/src/dongarch/clip/DongArchClipManager.cpp`

```cpp
namespace DongArch::Clip {

class ClipManager {
private:
    Mesh* mMesh;

public:
    bool clip(const Vector3D& planeNormal, double planeD) {
        // 람다 함수 정의
        auto intersectTest = [&](Face* face) -> bool {
            int posCount = 0, negCount = 0;
            for (int i = 0; i < 3; i++) {
                Vertex* v = face->getVertex(i);
                double dist = planeNormal.dot(v->getPositionVector()) + planeD;
                if (dist > 0) posCount++;
                else negCount++;
            }
            return (posCount > 0 && negCount > 0);
        };

        auto distFun = [&](VertexOfFace* vof) -> double {
            Vector3D pos = vof->getPosition();
            return planeNormal.dot(pos) + planeD;
        };

        auto getIntersectionVec = [&](VertexOfFace* v1, VertexOfFace* v2, Vector3D& result) {
            Vector3D p1 = v1->getPosition();
            Vector3D p2 = v2->getPosition();
            double d1 = distFun(v1);
            double d2 = distFun(v2);

            double t = d1 / (d1 - d2);
            result = p1 + (p2 - p1) * t;
        };

        // GigaMesh splitMesh 호출
        return mMesh->splitMesh(intersectTest, distFun, getIntersectionVec,
                                true, false, Vector3D(0,0,0));
    }
};

} // namespace
```

---

## Phase 5: D-Tak/X-Ray C++ 구현

### Heat Method (GPU 버전 TODO)

```cpp
// DTakRenderer.cpp

class DTakRenderer {
public:
    // CPU 버전 (EdgeGeodesic 사용)
    std::vector<double> computeCPU(Vertex* sourceVertex) {
        std::vector<double> distances;

        // EdgeGeodesic 사용
        // TODO: 구현

        return distances;
    }

    // GPU 버전 (TODO)
    std::vector<double> computeGPU(Vertex* sourceVertex) {
        // Fragment Shader로 Heat Diffusion
        // TODO: 구현
        return {};
    }
};
```

---

## Phase 6: MFE C++ 구현

```cpp
// MFEWidget.cpp

class MFEWidget : public QWidget {
    Q_OBJECT

private:
    QFileSystemModel* mFileModel;
    QTreeView* mTreeView;

public:
    MFEWidget(QWidget* parent = nullptr) : QWidget(parent) {
        mFileModel = new QFileSystemModel(this);
        mFileModel->setRootPath(QDir::currentPath());

        mTreeView = new QTreeView(this);
        mTreeView->setModel(mFileModel);
    }
};
```

---

## Phase 7: Illustrator C++ 구현

```cpp
// IllustratorBridge.cpp

class IllustratorBridge {
public:
    bool exportSVG(const std::vector<Vector2D>& cutline,
                   const std::vector<Vector2D>& outline,
                   const QString& filename) {
        // GigaMesh MeshWriter 사용
        MeshWriter writer;

        // SVG 생성
        // TODO: 레이어 분리

        return writer.writeSVG(/* ... */, filename.toStdString());
    }
};
```

---

## C++20 패턴 및 베스트 프랙티스

### 1. std::span 사용

```cpp
// Before
void processMesh(const float* vertices, size_t count);

// After (C++20)
void processMesh(std::span<const float> vertices);
```

### 2. Ranges 사용

```cpp
#include <ranges>

// Filter silhouette edges
auto silhouetteEdges = mesh.edges()
    | std::views::filter([&](const Edge& e) {
        return e.isSilhouette(viewDir);
    })
    | std::ranges::to<std::vector>();
```

### 3. Concepts

```cpp
template<std::ranges::range R>
requires std::same_as<std::ranges::range_value_t<R>, Vector3D>
void processPoints(const R& points);
```

### 4. Designated Initializers

```cpp
CutlineParameters params {
    .tolerance = 0.5,
    .segments = 20,
    .useSpline = true
};
```

---

**문서 버전**: 1.0.0
**최종 업데이트**: 2025-11-10
**다음 업데이트**: Phase 2 구현 시작 시
