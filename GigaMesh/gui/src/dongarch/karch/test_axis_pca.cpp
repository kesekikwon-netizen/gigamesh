/* Standalone T3 PCA check: upright and tilt 15 deg open cylinder. */

#include "AxisPca.h"

#include <cmath>
#include <cstdio>
#include <numbers>

using DongArch::KArch::ComputePcaAlignment;
using DongArch::KArch::DetectRimBoundary;
using DongArch::KArch::MeshCentroid;
using DongArch::KArch::MeshData;
using DongArch::KArch::Point3;
using DongArch::KArch::RimError;

static MeshData makeOpenCylinderShell(double rOuter, double height, double thickness,
                                      int radial, int heightSeg) {
    MeshData m;
    const double rInner = rOuter - thickness;
    const int rings = heightSeg + 1;
    auto addRing = [&](double r, double z) {
        for (int j = 0; j < radial; ++j) {
            const double t = 2.0 * std::numbers::pi * static_cast<double>(j) /
                             static_cast<double>(radial);
            m.vertices.push_back(Point3{r * std::cos(t), r * std::sin(t), z});
        }
    };
    for (int i = 0; i < rings; ++i) {
        addRing(rOuter, height * static_cast<double>(i) / static_cast<double>(heightSeg));
    }
    const int innerBase = rings * radial;
    for (int i = 0; i < rings; ++i) {
        addRing(rInner, height * static_cast<double>(i) / static_cast<double>(heightSeg));
    }
    auto quad = [&](int a, int b, int c, int d) {
        m.faces.push_back({a, b, c});
        m.faces.push_back({a, c, d});
    };
    for (int i = 0; i < heightSeg; ++i) {
        for (int j = 0; j < radial; ++j) {
            const int jn = (j + 1) % radial;
            quad(i * radial + j, i * radial + jn, (i + 1) * radial + jn, (i + 1) * radial + j);
            const int i0 = innerBase + i * radial + j;
            const int i1 = innerBase + i * radial + jn;
            const int i2 = innerBase + (i + 1) * radial + jn;
            const int i3 = innerBase + (i + 1) * radial + j;
            quad(i0, i3, i2, i1);
        }
    }
    for (int j = 0; j < radial; ++j) {
        const int jn = (j + 1) % radial;
        quad(j, innerBase + j, innerBase + jn, jn);
    }
    return m;
}

static void rotateX(MeshData& m, double deg) {
    const double th = deg * std::numbers::pi / 180.0;
    const double c = std::cos(th);
    const double s = std::sin(th);
    for (Point3& p : m.vertices) {
        const double y = c * p.y - s * p.z;
        const double z = s * p.y + c * p.z;
        p.y = y;
        p.z = z;
    }
}

static bool checkAligned(const std::vector<Point3>& rim,
                         const DongArch::KArch::PcaAlignment& pca,
                         const char* tag) {
    if (!pca.ok) {
        std::fprintf(stderr, "FAIL %s: pca not ok\n", tag);
        return false;
    }
    Point3 c{};
    double zMin = 1e300;
    double zMax = -1e300;
    for (const Point3& p : rim) {
        const Point3 q = pca.transform.apply(p);
        c.x += q.x;
        c.y += q.y;
        c.z += q.z;
        zMin = (q.z < zMin) ? q.z : zMin;
        zMax = (q.z > zMax) ? q.z : zMax;
    }
    const double n = static_cast<double>(rim.size());
    c.x /= n;
    c.y /= n;
    c.z /= n;
    const double zSpan = zMax - zMin;
    std::printf("%s planarity=%.4e zSpan=%.3e c=(%.3e,%.3e,%.3e) warn=%d\n",
                tag, pca.planarity, zSpan, c.x, c.y, c.z, static_cast<int>(pca.planarityWarn));
    if (std::fabs(c.x) > 1e-6 || std::fabs(c.y) > 1e-6 || std::fabs(c.z) > 1e-6) {
        std::fprintf(stderr, "FAIL %s: rim centroid not at origin\n", tag);
        return false;
    }
    if (zSpan > 1e-6) {
        std::fprintf(stderr, "FAIL %s: rim not planar in XY\n", tag);
        return false;
    }
    return true;
}

int main() {
    MeshData up = makeOpenCylinderShell(50.0, 100.0, 5.0, 180, 20);
    const auto rimUp = DetectRimBoundary(up);
    if (rimUp.error != RimError::Ok) {
        std::fprintf(stderr, "FAIL: upright rim\n");
        return 1;
    }
    const auto pcaUp = ComputePcaAlignment(rimUp.rimPoints, MeshCentroid(up));
    if (!checkAligned(rimUp.rimPoints, pcaUp, "upright")) {
        return 1;
    }
    if (pcaUp.rimNormal.z < 0.0) {
        std::fprintf(stderr, "FAIL: upright normal flipped\n");
        return 1;
    }

    MeshData tilt = makeOpenCylinderShell(50.0, 100.0, 5.0, 180, 20);
    rotateX(tilt, 15.0);
    const auto rimT = DetectRimBoundary(tilt);
    if (rimT.error != RimError::Ok) {
        std::fprintf(stderr, "FAIL: tilt rim\n");
        return 1;
    }
    const auto pcaT = ComputePcaAlignment(rimT.rimPoints, MeshCentroid(tilt));
    if (!checkAligned(rimT.rimPoints, pcaT, "tilt15")) {
        return 1;
    }
    const double gtY = -std::sin(15.0 * std::numbers::pi / 180.0);
    const double gtZ = std::cos(15.0 * std::numbers::pi / 180.0);
    const double dot = pcaT.rimNormal.y * gtY + pcaT.rimNormal.z * gtZ;
    const double ang = std::acos(std::max(-1.0, std::min(1.0, dot))) * 180.0 / std::numbers::pi;
    std::printf("tilt15 angleErrDeg=%.6f\n", ang);
    if (ang >= 0.3) {
        std::fprintf(stderr, "FAIL: axis angle %.6f >= 0.3 deg\n", ang);
        return 1;
    }

    std::puts("PASS");
    return 0;
}
