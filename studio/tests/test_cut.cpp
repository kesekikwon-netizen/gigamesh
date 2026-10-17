// 헤드리스 검증: ② 막으로 자르기 004b 실시간 단면 (JOB-005 수용 기준 1-12)
// 끄는 동안 = LOD(≤ 3.5만 면) Y=0 단면 1장(previewFrontAngle), 놓으면 = 원본 전체(commitPreview → setFrontAngle).
// 정면각 + = +Z 오른손 법칙(위에서 보아 반시계), 0° = 확정 자세.
// 허용오차: 기하 같음 1e-9 mm(K-Arch test_synthetic·test_dimensions, JOB-004 와 같은 값), 끄는 동안 바뀌면 안 되는
// 값은 ==(다시 계산하지 않으므로), 미리보기 16 ms(60 Hz 한 프레임, CutlineController.cpp:23) — 환경 변수 CI 면 ×4 = 64 ms.
#include "ArtifactDoc.h"
#include "CutGesture.h"
#include "MeshRaster.h"
#include "Plate.h"
#include "Theme.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <vector>

using namespace studio;

static int fails = 0;
#define CHECK(c, ...)                         \
    do {                                      \
        if (!(c)) {                           \
            std::printf("FAIL %s: ", #c);     \
            std::printf(__VA_ARGS__);         \
            std::printf("\n");                \
            ++fails;                          \
        }                                     \
    } while (0)

using Pts = std::vector<KA::Point2>;
using Polys = std::vector<Pts>;
using Segs = std::vector<std::pair<KA::Point2, KA::Point2>>;
using Loader = std::function<void(ArtifactDoc&)>;

static constexpr double kTol = 1e-9;  // mm
static constexpr double kInf = std::numeric_limits<double>::infinity();

static std::vector<double> probeZ() {  // 두께 확인 높이 10, 20, …, 180 mm
    std::vector<double> z;
    for (int i = 1; i <= 18; ++i) z.push_back(10.0 * i);
    return z;
}

// ---- 비교 도우미 ------------------------------------------------------------
static bool identical(const Pts& a, const Pts& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].x != b[i].x || a[i].y != b[i].y) return false;
    return true;
}
static bool identical(const std::vector<KA::Point3>& a, const std::vector<KA::Point3>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].x != b[i].x || a[i].y != b[i].y || a[i].z != b[i].z) return false;
    return true;
}
// 점 수가 같으면 최대 좌표 차, 다르면 ∞
static double ptsDiff(const Pts& a, const Pts& b) {
    if (a.size() != b.size()) return kInf;
    double m = 0.0;
    for (size_t i = 0; i < a.size(); ++i) m = std::max({m, std::fabs(a[i].x - b[i].x), std::fabs(a[i].y - b[i].y)});
    return m;
}
// 고리 수·고리별 점 수가 같으면 최대 좌표 차, 다르면 ∞
static double polyDiff(const Polys& a, const Polys& b) {
    if (a.size() != b.size()) return kInf;
    double m = 0.0;
    for (size_t i = 0; i < a.size(); ++i) m = std::max(m, ptsDiff(a[i], b[i]));
    return m;
}
static double segDiff(const Segs& a, const Segs& b) {
    if (a.size() != b.size()) return kInf;
    double m = 0.0;
    for (size_t i = 0; i < a.size(); ++i)
        m = std::max({m, std::fabs(a[i].first.x - b[i].first.x), std::fabs(a[i].first.y - b[i].first.y),
                      std::fabs(a[i].second.x - b[i].second.x), std::fabs(a[i].second.y - b[i].second.y)});
    return m;
}
static double vertDiff(const KA::MeshData& a, const KA::MeshData& b) {
    if (a.vertices.size() != b.vertices.size()) return kInf;
    double m = 0.0;
    for (size_t i = 0; i < a.vertices.size(); ++i) {
        const auto &p = a.vertices[i], &q = b.vertices[i];
        m = std::max({m, std::fabs(p.x - q.x), std::fabs(p.y - q.y), std::fabs(p.z - q.z)});
    }
    return m;
}
static double dimsDiff(const Dims5& a, const Dims5& b) {
    return std::max({std::fabs(a.height - b.height), std::fabs(a.rim - b.rim), std::fabs(a.belly - b.belly),
                     std::fabs(a.bellyZ - b.bellyZ), std::fabs(a.base - b.base), std::fabs(a.meanWall - b.meanWall)});
}
static bool sameDims(const Dims5& a, const Dims5& b) {
    return a.height == b.height && a.rim == b.rim && a.belly == b.belly && a.bellyZ == b.bellyZ && a.base == b.base &&
           a.meanWall == b.meanWall && a.ok == b.ok;
}
static double maxX(const Polys& ps) {
    double m = -kInf;
    for (const auto& p : ps)
        for (const auto& q : p) m = std::max(m, q.x);
    return m;
}
static double minX(const Polys& ps) {
    double m = kInf;
    for (const auto& p : ps)
        for (const auto& q : p) m = std::min(m, q.x);
    return m;
}

// ---- 끄는 동안 그대로여야 하는 확정값 -----------------------------------------
struct Frozen {
    Dims5 d;
    Pts outline;
    std::vector<double> wall;
    std::vector<bool> est;
    double estWall = 0.0;
    std::vector<KA::Point3> verts;
    std::vector<SectionSlot> slotCopy;
};
static Frozen freeze(const ArtifactDoc& doc) {
    Frozen f;
    f.d = doc.dims();
    f.outline = doc.displayOutline();
    for (double z : probeZ()) {
        f.wall.push_back(doc.wallThicknessAt(z));
        f.est.push_back(doc.isWallEstimatedAt(z));
    }
    f.estWall = doc.estimatedWallMm();
    f.verts = doc.mesh().vertices;
    f.slotCopy = doc.slotList();
    return f;
}
// 다른 항목 이름을 돌려준다 (모두 같으면 빈 문자열)
static const char* frozenDiff(const Frozen& a, const Frozen& b) {
    if (!sameDims(a.d, b.d)) return "dims";
    if (!identical(a.outline, b.outline)) return "outline";
    if (a.wall != b.wall) return "wallThicknessAt";
    if (a.est != b.est) return "isWallEstimatedAt";
    if (a.estWall != b.estWall) return "estimatedWallMm";
    if (!identical(a.verts, b.verts)) return "mesh";
    if (a.slotCopy.size() != b.slotCopy.size()) return "slots.size";
    for (size_t i = 0; i < a.slotCopy.size(); ++i)
        if (a.slotCopy[i].angleDeg != b.slotCopy[i].angleDeg || a.slotCopy[i].z != b.slotCopy[i].z) return "slots";
    return "";
}

// ---- 상자 (Z 축 180° 회전에 대칭인 삼각분할 → 확정의 중심선 이동 0) ------------
static KA::MeshData boxMesh() {
    KA::MeshData m;
    m.vertices = {{-20, -10, 0}, {20, -10, 0}, {20, 10, 0}, {-20, 10, 0},
                  {-20, -10, 20}, {20, -10, 20}, {20, 10, 20}, {-20, 10, 20}};
    m.faces = {{0, 2, 1}, {0, 3, 2}, {4, 5, 6}, {4, 6, 7}, {0, 1, 5}, {0, 5, 4},
               {2, 3, 7}, {2, 7, 6}, {1, 2, 6}, {1, 6, 5}, {3, 0, 4}, {3, 4, 7}};
    return m;
}
static void loadBox(ArtifactDoc& d) {
    d.loadMesh(boxMesh(), {}, QStringLiteral("상자"));
    d.confirmPose();
}

// ---- 꺾은선 거리 (기준 10) ------------------------------------------------------
static double segDist(const KA::Point2& p, const KA::Point2& a, const KA::Point2& b) {
    const double dx = b.x - a.x, dy = b.y - a.y, L2 = dx * dx + dy * dy;
    const double t = L2 > 0 ? std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / L2, 0.0, 1.0) : 0.0;
    return std::hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy));
}
struct OneWay {
    double max = 0.0, sum = 0.0;
    size_t n = 0;
};
// from 의 각 점 → to 의 닫힌 꺾은선들 중 가장 가까운 선분
static OneWay oneWay(const Polys& from, const Polys& to) {
    OneWay r;
    for (const auto& poly : from)
        for (const auto& p : poly) {
            double best = kInf;
            for (const auto& q : to)
                for (size_t i = 0; i < q.size(); ++i) best = std::min(best, segDist(p, q[i], q[(i + 1) % q.size()]));
            r.max = std::max(r.max, best);
            r.sum += best;
            ++r.n;
        }
    return r;
}

static double ms(std::chrono::steady_clock::duration d) {
    return std::chrono::duration<double, std::milli>(d).count();
}
static double quantile(std::vector<double> v, double q) {  // q = 0.5 중앙값, 0.9 → ceil(0.9 n) 번째
    std::sort(v.begin(), v.end());
    if (v.empty()) return kInf;
    const size_t k = size_t(std::ceil(q * double(v.size())));
    return v[std::min(v.size() - 1, k > 0 ? k - 1 : 0)];
}
static double median(std::vector<double> v) {
    std::sort(v.begin(), v.end());
    return v.empty() ? kInf : v[v.size() / 2];
}

// ---- 1. 확정 전·미로드 무시 ----------------------------------------------------
static void test1() {
    auto poke = [](ArtifactDoc& d) {
        d.previewFrontAngle(30);
        d.commitPreview();
        d.alignCut(CutAxis::X);
        d.setFrontAngle(30);
    };
    {
        ArtifactDoc d;
        poke(d);
        CHECK(!d.isPreviewing() && d.frontAngle() == 0.0, "unloaded: previewing=%d front=%g", int(d.isPreviewing()),
              d.frontAngle());
    }
    ArtifactDoc d;
    d.loadSample(false);
    const auto mv = d.mesh().vertices;
    const auto lv = d.lodMesh().vertices;
    poke(d);
    CHECK(!d.isPreviewing(), "unconfirmed: previewing");
    CHECK(d.frontAngle() == 0.0 && d.previewAngle() == 0.0, "unconfirmed: front=%g preview=%g", d.frontAngle(),
          d.previewAngle());
    CHECK(d.displaySection().empty(), "unconfirmed: section not empty");
    CHECK(identical(mv, d.mesh().vertices) && identical(lv, d.lodMesh().vertices), "unconfirmed: mesh changed");
    std::printf("[1] unloaded/unconfirmed: ignored (front=%g preview=%g section=%zu)\n", d.frontAngle(), d.previewAngle(),
                d.displaySection().size());
}

// ---- 2·3. 끄는 동안 불변·미리보기는 따라감 / 놓기 == 새 문서 setFrontAngle ------------
static void testExample(const char* name, const Loader& load) {
    ArtifactDoc A, C;
    load(A);
    load(C);
    const Frozen f0 = freeze(A);
    for (double th : {5.0, 12.0, -20.0, 90.0, 137.0, 37.5}) {
        A.previewFrontAngle(th);
        C.setFrontAngle(th);
        CHECK(A.isPreviewing(), "%s θ=%g: not previewing", name, th);
        CHECK(A.previewAngle() == std::remainder(th, 360.0), "%s θ=%g: previewAngle %g", name, th, A.previewAngle());
        CHECK(A.frontAngle() == 0.0, "%s θ=%g: frontAngle %g", name, th, A.frontAngle());
        const char* changed = frozenDiff(f0, freeze(A));
        CHECK(changed[0] == '\0', "%s θ=%g: confirmed value changed during preview: %s", name, th, changed);
        const auto sec = A.displaySection();
        CHECK(!sec.empty(), "%s θ=%g: preview section empty", name, th);
        const double lod = vertDiff(A.lodMesh(), C.lodMesh());
        CHECK(lod <= kTol, "%s θ=%g: preview LOD vs setFrontAngle LOD %.3g", name, th, lod);
        size_t npts = 0;
        for (const auto& p : sec) npts += p.size();
        std::printf("[2] %s θ=%g: preview=%g front=%g frozen=%s loops=%zu pts=%zu lodFaces=%zu lodDiff=%.3g\n", name, th,
                    A.previewAngle(), A.frontAngle(), changed[0] ? changed : "same", sec.size(), npts,
                    A.lodMesh().faces.size(), lod);
    }
    // 3. 놓기 → 원본 전체, 새로 열어 setFrontAngle 한 번만 한 문서와 같음
    A.commitPreview();
    CHECK(!A.isPreviewing() && A.frontAngle() == 37.5, "%s: commit previewing=%d front=%g", name, int(A.isPreviewing()),
          A.frontAngle());
    ArtifactDoc B;
    load(B);
    B.setFrontAngle(37.5);
    const double dSec = polyDiff(A.displaySection(), B.displaySection());
    const double dSolid = polyDiff(A.displayScannedSolid(), B.displayScannedSolid());
    const double dEst = polyDiff(A.displayEstimatedInner(), B.displayEstimatedInner());
    const double dOut = ptsDiff(A.displayOutline(), B.displayOutline());
    const double dHatch = segDiff(A.displayHatch(), B.displayHatch());
    const double dDims = dimsDiff(A.dims(), B.dims());
    double dWall = 0.0;
    for (double z : probeZ()) dWall = std::max(dWall, std::fabs(A.wallThicknessAt(z) - B.wallThicknessAt(z)));
    const double dEstWall = std::fabs(A.estimatedWallMm() - B.estimatedWallMm());
    const double dMesh = vertDiff(A.mesh(), B.mesh()), dLod = vertDiff(A.lodMesh(), B.lodMesh());
    for (double v : {dSec, dSolid, dEst, dOut, dHatch, dDims, dWall, dEstWall, dMesh, dLod})
        CHECK(v <= kTol, "%s: commit vs fresh setFrontAngle diff %.3g", name, v);
    CHECK(A.hasEstimatedInner() == B.hasEstimatedInner() && A.rawPointCount() == B.rawPointCount() &&
              A.pointCount() == B.pointCount(),
          "%s: hasEst %d/%d raw %d/%d pts %d/%d", name, int(A.hasEstimatedInner()), int(B.hasEstimatedInner()),
          A.rawPointCount(), B.rawPointCount(), A.pointCount(), B.pointCount());
    std::printf("[c] %s commit(37.5) vs fresh: section=%.3g solid=%.3g est=%.3g outline=%.3g hatch=%.3g dims=%.3g "
                "wall=%.3g estWall=%.3g mesh=%.3g lod=%.3g hasEst=%d/%d raw=%d/%d pts=%d/%d\n",
                name, dSec, dSolid, dEst, dOut, dHatch, dDims, dWall, dEstWall, dMesh, dLod, int(A.hasEstimatedInner()),
                int(B.hasEstimatedInner()), A.rawPointCount(), B.rawPointCount(), A.pointCount(), B.pointCount());
}

// ---- 4·6·7. 상자 -----------------------------------------------------------------
static void testBox() {
    // 4. 같은 메시(LOD = 원본 사본)면 미리보기 단면 = 확정 단면
    for (double th : {12.0, 137.0}) {
        ArtifactDoc P, Q;
        loadBox(P);
        loadBox(Q);
        P.previewFrontAngle(th);
        Q.setFrontAngle(th);
        const auto a = P.displaySection(), b = Q.displaySection();
        const double d = polyDiff(a, b);
        CHECK(!a.empty() && d <= kTol, "box θ=%g: preview vs setFrontAngle section diff %.3g (loops %zu/%zu)", th, d,
              a.size(), b.size());
        std::printf("[4] box θ=%g: lodFaces=%zu fullFaces=%zu loops=%zu/%zu section diff=%.3g\n", th,
                    P.lodMesh().faces.size(), P.mesh().faces.size(), a.size(), b.size(), d);
    }
    // 6. 각도 부호: + = 위에서 보아 반시계 (x, y) → (−y, x)
    {
        ArtifactDoc d;
        loadBox(d);
        const auto v0 = d.mesh().vertices;
        d.setFrontAngle(90);
        const auto& v = d.mesh().vertices;
        double e = v.size() == v0.size() ? 0.0 : kInf;
        for (size_t i = 0; i < std::min(v.size(), v0.size()); ++i)
            e = std::max({e, std::fabs(v[i].x + v0[i].y), std::fabs(v[i].y - v0[i].x), std::fabs(v[i].z - v0[i].z)});
        CHECK(e <= kTol, "box +90°: not counter-clockwise, err %.3g", e);
        std::printf("[6] box +90°: (x,y,z) -> (-y,x,z) max err %.3g (front face y=-10 -> x=+10)\n", e);
    }
    // 7. 막 맞춤
    {
        ArtifactDoc d;
        loadBox(d);
        const double cx = 0.5 * (d.mesh().bboxMin.x + d.mesh().bboxMax.x), cy = 0.5 * (d.mesh().bboxMin.y + d.mesh().bboxMax.y);
        std::printf("[7] box confirmed bbox center x=%.3g y=%.3g\n", cx, cy);
        CHECK(std::fabs(cx) <= kTol && std::fabs(cy) <= kTol, "box center %.3g %.3g", cx, cy);
        d.setSmooth(false);
        d.alignCut(CutAxis::Y);
        const auto sy = d.displaySection();
        const double fy = d.frontAngle();
        CHECK(fy == 0.0, "alignCut(Y) front %g", fy);
        CHECK(!sy.empty() && std::fabs(maxX(sy) - 20.0) <= kTol && std::fabs(minX(sy)) <= kTol, "Y cut x[%.9g, %.9g]",
              minX(sy), maxX(sy));
        d.alignCut(CutAxis::X);
        const auto sx = d.displaySection();
        CHECK(d.frontAngle() == 90.0, "alignCut(X) front %g", d.frontAngle());
        CHECK(!sx.empty() && std::fabs(maxX(sx) - 10.0) <= kTol, "X cut max x %.9g", maxX(sx));
        std::printf("[7] alignCut Y: front=%g x[%.6f, %.6f] | X: front=%g max x=%.6f\n", fy, minX(sy), maxX(sy),
                    d.frontAngle(), maxX(sx));
        d.alignCut(CutAxis::Y);
        d.previewFrontAngle(90);
        const auto sp = d.displaySection();
        const double bx = d.mesh().bboxMax.x;
        CHECK(!sp.empty() && std::fabs(maxX(sp) - 10.0) <= kTol, "preview 90 max x %.9g", maxX(sp));
        CHECK(std::fabs(bx - 20.0) <= kTol, "preview 90 mesh bboxMax.x %.9g", bx);
        d.alignCut(CutAxis::Y);
        CHECK(!d.isPreviewing(), "alignCut must end preview");
        std::printf("[7] preview 90 max x=%.6f mesh.bboxMax.x=%.6f → alignCut(Y) previewing=%d\n", maxX(sp), bx,
                    int(d.isPreviewing()));
    }
}

// ---- 5. 확정값을 바꾸면 미리보기를 지움 ------------------------------------------------
static void test5() {
    ArtifactDoc d;
    d.loadSample();
    d.previewFrontAngle(30);
    CHECK(d.inferInnerWall(), "inferInnerWall should be on");
    d.setEstimatedWallMm(5.0);
    CHECK(!d.isPreviewing(), "setEstimatedWallMm must end preview");
    d.previewFrontAngle(30);
    d.setInferInnerWall(false);
    CHECK(!d.isPreviewing(), "setInferInnerWall must end preview");
    d.previewFrontAngle(30);
    d.setFrontAngle(10);
    CHECK(!d.isPreviewing() && d.frontAngle() == 10.0, "setFrontAngle must end preview (front %g)", d.frontAngle());
    d.previewFrontAngle(30);
    d.loadSample();
    CHECK(!d.isPreviewing(), "loadSample must end preview");
    d.previewFrontAngle(30);
    d.setSmooth(false);
    CHECK(d.isPreviewing(), "setSmooth must keep preview");
    d.setSimplifyTol(0.3);
    CHECK(d.isPreviewing(), "setSimplifyTol must keep preview");
    d.setSectionSide(SectionSide::Left);
    CHECK(d.isPreviewing(), "setSectionSide must keep preview");
    std::printf("[5] wall/infer/front/load end preview; smooth/simplify/side keep it (previewing=%d)\n",
                int(d.isPreviewing()));
}

// ---- 8. 미리보기 중 내보내기 거부 -----------------------------------------------------
static void test8(const QString& out) {
    ArtifactDoc d;
    d.loadSample();
    d.previewFrontAngle(20);
    struct Kind {
        const char* ext;
        std::function<bool(const ExportRequest&, QString*)> run;
    };
    const std::vector<Kind> kinds = {
        {"svg", [&](const ExportRequest& r, QString* e) { return exportSvgLayered(d, r, e); }},
        {"pdf", [&](const ExportRequest& r, QString* e) { return exportPdf(d, r, e); }},
        {"png", [&](const ExportRequest& r, QString* e) { return exportPng(d, r, 150, e); }},
        {"dxf", [&](const ExportRequest& r, QString* e) { return exportDxf(d, r, e); }}};
    for (const auto& k : kinds) {
        ExportRequest r;
        r.path = out + QStringLiteral("/cut_refuse.") + QString::fromLatin1(k.ext);
        QFile::remove(r.path);
        QString err;
        const bool ok = k.run(r, &err);
        const bool exists = QFile::exists(r.path);
        CHECK(!ok && err.contains(QStringLiteral("놓은 뒤")) && !exists, "%s: ok=%d exists=%d err=%s", k.ext, int(ok),
              int(exists), err.toUtf8().constData());
        std::printf("[8] %s during preview: ok=%d file=%d err=%s\n", k.ext, int(ok), int(exists), err.toUtf8().constData());
    }
    d.commitPreview();
    ExportRequest r;
    r.path = out + QStringLiteral("/cut_refuse.svg");
    QString err;
    const bool ok = exportSvgLayered(d, r, &err);
    const qint64 size = QFileInfo(r.path).size();
    CHECK(ok && size > 0, "svg after commit ok=%d size=%lld err=%s", int(ok), size, err.toUtf8().constData());
    std::printf("[8] svg after commit: ok=%d size=%lld bytes\n", int(ok), size);
}

// ---- 9. 시간 -------------------------------------------------------------------------
static void test9(const char* name, const Loader& load, bool judge) {
    using clk = std::chrono::steady_clock;
    const bool ci = qEnvironmentVariableIsSet("CI");
    const double limit = ci ? 64.0 : 16.0;
    ArtifactDoc d;
    load(d);
    std::vector<double> t;
    size_t sink = 0;
    for (int i = 0; i <= 51; ++i) {
        const auto t0 = clk::now();
        d.previewFrontAngle(3.7 * i);
        sink += d.displaySection().size();
        const auto t1 = clk::now();
        if (i > 0) t.push_back(ms(t1 - t0));  // i = 0 은 예열
    }
    const double med = median(t), p90 = quantile(t, 0.9), mx = *std::max_element(t.begin(), t.end());
    std::printf("[d] %s preview lodFaces=%zu fullFaces=%zu median=%.2f p90=%.2f max=%.2f ms limit=%.0f ms (%s)%s\n", name,
                d.lodMesh().faces.size(), d.mesh().faces.size(), med, p90, mx, limit, ci ? "CI" : "local",
                judge ? "" : " — 기록만");
    if (judge) CHECK(med <= limit, "%s preview median %.2f ms > %.0f ms", name, med, limit);
    // 3D 한 장 (보조창 크기 420×640, 미리보기 LOD, 실사)
    {
        const auto& m = d.lodMesh();
        RasterView v;
        v.yawDeg = -32.0;
        v.pitchDeg = 14.0;
        v.centerPx = QPointF(210.0, 320.0);
        const double r = std::max({std::fabs(m.bboxMin.x), std::fabs(m.bboxMax.x), std::fabs(m.bboxMin.y), std::fabs(m.bboxMax.y)});
        const double h = m.bboxMax.z - m.bboxMin.z;
        v.pxPerMm = 420.0 / std::max(1.0, std::sqrt(4 * r * r + h * h) * 1.12);
        v.centerZ = m.bboxMin.z + h / 2.0;
        const auto& lc = d.lodVertexColors();
        std::vector<double> rt;
        for (int i = 0; i < 11; ++i) {
            const auto t0 = clk::now();
            const QImage img = renderMesh(m, QSize(420, 640), v, ShadeMode::Real, theme::ModelClay, nullptr,
                                          lc.size() == m.vertices.size() ? &lc : nullptr);
            rt.push_back(ms(clk::now() - t0));
            sink += size_t(img.width());
        }
        std::printf("[d] %s 3D render lodFaces=%zu 420x640 median=%.2f ms (기록만)\n", name, m.faces.size(), median(rt));
    }
    // 원본 전체 (놓을 때)
    std::vector<double> ft;
    for (double th : {10.0, 20.0, 30.0}) {
        const auto t0 = clk::now();
        d.setFrontAngle(th);
        sink += d.displaySection().size();
        ft.push_back(ms(clk::now() - t0));
    }
    std::printf("[d] %s full fullFaces=%zu median=%.1f ms (n=3, 기록만)\n", name, d.mesh().faces.size(), median(ft));
    if (sink == 0) std::printf("(sink 0)\n");
}

// ---- 10. LOD ↔ 원본 거리 (기록만) -----------------------------------------------------
static void test10(const char* name, const Loader& load) {
    ArtifactDoc A, C;
    load(A);
    load(C);
    for (double th : {0.0, 12.0, 45.0, 90.0, 137.0}) {
        A.previewFrontAngle(th);
        C.setFrontAngle(th);
        const auto pa = A.displaySection(), pc = C.displaySection();
        const OneWay ab = oneWay(pa, pc), ba = oneWay(pc, pa);
        const double hd = std::max(ab.max, ba.max);
        const double mean = (ab.n + ba.n) ? (ab.sum + ba.sum) / double(ab.n + ba.n) : kInf;
        std::printf("lod-vs-full %s θ=%g hausdorff=%.3f mean=%.3f mm hasEst preview/full=%d/%d\n", name, th, hd, mean,
                    int(A.hasEstimatedInner()), int(C.hasEstimatedInner()));
        CHECK(!pa.empty() && !pc.empty() && std::isfinite(hd) && std::isfinite(mean), "%s θ=%g: lod-vs-full empty/inf",
              name, th);
    }
}

// ---- 11. 입력 합치기 CutGesture ----------------------------------------------------------
static void test11() {
    using Due = CutGesture::Due;
    {  // a·b. 끌기 합치기(throttle) → 놓기 즉시
        CutGesture g;
        std::vector<long long> pv;
        int commits = 0;
        for (long long t = 0; t < 1000; ++t) {
            g.drag(t == 0 ? 10.0 : 999.0, 0.5, t);  // 활성 중 base 는 무시된다
            const Due r = g.poll(t);
            if (r == Due::Preview) pv.push_back(t);
            else if (r == Due::Commit) ++commits;
        }
        bool gapsOk = !pv.empty();
        for (size_t i = 1; i < pv.size(); ++i) gapsOk = gapsOk && pv[i] - pv[i - 1] >= 16 && pv[i] - pv[i - 1] <= 17;
        CHECK(!pv.empty() && pv.front() == 16, "a: first preview at %lld", pv.empty() ? -1LL : pv.front());
        CHECK(gapsOk, "a: preview gaps not 16-17 ms");
        CHECK(pv.size() >= 55 && pv.size() <= 63, "a: %zu previews", pv.size());
        CHECK(commits == 0, "a: %d commits while dragging", commits);
        std::printf("[11a] drag 1000 ms: previews=%zu first=%lld commits=%d\n", pv.size(), pv.empty() ? -1LL : pv.front(),
                    commits);
        g.release(1000);
        CHECK(g.poll(1000) == Due::Commit, "b: release must commit now");
        CHECK(g.target() == 510.0 && !g.active(), "b: target %g active %d", g.target(), int(g.active()));
        CHECK(g.poll(5000) == Due::None, "b: nothing after commit");
        std::printf("[11b] release(1000): commit target=%g active=%d\n", g.target(), int(g.active()));
    }
    {  // c. 키 뗀 뒤 300 ms
        CutGesture g;
        int pv = 0, cm = 0;
        for (long long t = 0; t < 540; ++t) {
            if (t == 0) g.keyDown(0.0, 1.0, t);
            if (t == 40) g.keyUp(t);
            if (t == 200) g.keyDown(0.0, 1.0, t);
            if (t == 240) g.keyUp(t);
            const Due r = g.poll(t);
            pv += r == Due::Preview;
            cm += r == Due::Commit;
        }
        CHECK(cm == 0 && pv >= 1, "c: commits=%d previews=%d before 540", cm, pv);
        CHECK(g.poll(540) == Due::Commit && g.target() == 2.0, "c: commit at 540 target %g", g.target());
        std::printf("[11c] keys 0/40/200/240: previews=%d commits(<540)=%d → commit@540 target=%g\n", pv, cm, g.target());
    }
    {  // d. 누르고 있는 동안 보류
        CutGesture g;
        int downs = 0, cm = 0;
        for (long long t = 0; t <= 3000; ++t) {
            if (t == 0 || (t >= 500 && (t - 500) % 33 == 0)) {
                g.keyDown(0.0, 1.0, t);
                ++downs;
            }
            cm += g.poll(t) == Due::Commit;
        }
        CHECK(cm == 0, "d: %d commits while held", cm);
        g.keyUp(3000);
        CHECK(g.poll(3299) != Due::Commit, "d: commit before 3300");
        CHECK(g.poll(3300) == Due::Commit, "d: no commit at 3300");
        CHECK(g.target() == double(downs), "d: target %g != keyDowns %d", g.target(), downs);
        std::printf("[11d] held 3000 ms: keyDowns=%d commits=%d → keyUp(3000) commit@3300 target=%g\n", downs, cm, g.target());
    }
    {  // e. 단추
        CutGesture g;
        for (long long t : {0LL, 100LL, 200LL}) g.click(5.0, 1.0, t);
        CHECK(g.poll(499) != Due::Commit, "e: commit before 500");
        CHECK(g.poll(500) == Due::Commit && g.target() == 8.0, "e: commit@500 target %g", g.target());
        std::printf("[11e] clicks 0/100/200: commit@500 target=%g\n", g.target());
    }
    {  // f. 확정이 먼저 · 취소
        CutGesture g;
        g.click(0.0, 1.0, 0);
        CHECK(g.poll(400) == Due::Commit, "f: commit first");
        CHECK(g.poll(401) == Due::None, "f: nothing after commit");
        CutGesture h;
        h.drag(0.0, 1.0, 0);
        h.cancel();
        CHECK(!h.active() && h.poll(1000) == Due::None && h.nextDueMs() == -1, "f: cancel");
        std::printf("[11f] commit before preview; cancel → active=%d next=%lld\n", int(h.active()), h.nextDueMs());
    }
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    const QString out = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath();
    const Loader sample = [](ArtifactDoc& d) { d.loadSample(); };
    const Loader blind = [](ArtifactDoc& d) { d.loadArtecBlindSample(); };

    test1();
    testExample("sample", sample);
    testExample("blind", blind);
    testBox();
    test5();
    test8(out);
    test9("sample", sample, true);
    test9("blind", blind, false);
    test10("sample", sample);
    test10("blind", blind);
    test11();

    std::puts(fails ? "FAILED" : "PASS");
    return fails ? 1 : 0;
}
