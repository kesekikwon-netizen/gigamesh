/* Standalone slicer + ODR check on open cylinder shells. */

#include "AxisOdr.h"
#include "AxisPca.h"

#include <cmath>
#include <cstdio>
#include <numbers>
#include <vector>

using DongArch::KArch::Axis3D;
using DongArch::KArch::ComputePcaAlignment;
using DongArch::KArch::DetectRimBoundary;
using DongArch::KArch::FitHorizontalOuters;
using DongArch::KArch::MeshCentroid;
using DongArch::KArch::MeshData;
using DongArch::KArch::OptimizeRotationAxis;
using DongArch::KArch::Point3;
using DongArch::KArch::RimError;
using DongArch::KArch::Slice;
using DongArch::KArch::SliceCircle;

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

static std::vector<double> zBand(const MeshData& m, int n) {
    double zMin = m.vertices[0].z;
    double zMax = m.vertices[0].z;
    for (const Point3& p : m.vertices) {
        zMin = (p.z < zMin) ? p.z : zMin;
        zMax = (p.z > zMax) ? p.z : zMax;
    }
    const double z0 = zMin + 0.10 * (zMax - zMin);
    const double z1 = zMin + 0.90 * (zMax - zMin);
    std::vector<double> zs;
    zs.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(n - 1);
        zs.push_back(z0 + t * (z1 - z0));
    }
    return zs;
}

static bool checkAxis(const Axis3D& ax, const char* tag) {
    if (!ax.ok) {
        std::fprintf(stderr, "FAIL %s: odr not ok fallback=%d used=%d\n",
                     tag, static_cast<int>(ax.usedPcaFallback), ax.usedSlices);
        return false;
    }
    const double ang = std::acos(std::max(-1.0, std::min(1.0, ax.direction.z))) *
                       180.0 / std::numbers::pi;
    std::printf("%s used=%d rmse=%.4e dir=(%.3e,%.3e,%.3e) angDeg=%.6f\n",
                tag, ax.usedSlices, ax.rmse, ax.direction.x, ax.direction.y,
                ax.direction.z, ang);
    if (ang >= 0.3) {
        std::fprintf(stderr, "FAIL %s: axis angle %.6f >= 0.3\n", tag, ang);
        return false;
    }
    return true;
}

int main() {
    MeshData up = makeOpenCylinderShell(50.0, 100.0, 5.0, 180, 20);
    const auto mid = Slice(up, DongArch::KArch::Plane{{0, 0, 1}, -50.0});
    const auto midFit = FitHorizontalOuters(up, {50.0});
    if (mid.size() < 2 || midFit.size() != 1 || !midFit[0].circle.ok ||
        std::fabs(midFit[0].circle.r - 50.0) >= 0.05) {
        std::fprintf(stderr, "FAIL: z=50 loops=%zu fits=%zu R=%.6f\n",
                     mid.size(), midFit.size(),
                     midFit.empty() ? -1.0 : midFit[0].circle.r);
        return 1;
    }
    std::printf("z50 loops=%zu outerR=%.9f\n", mid.size(), midFit[0].circle.r);

    const Axis3D axUp = OptimizeRotationAxis(FitHorizontalOuters(up, zBand(up, 40)));
    if (!checkAxis(axUp, "upright")) {
        return 1;
    }

    MeshData tilt = makeOpenCylinderShell(50.0, 100.0, 5.0, 180, 20);
    rotateX(tilt, 15.0);
    const auto rim = DetectRimBoundary(tilt);
    if (rim.error != RimError::Ok) {
        std::fprintf(stderr, "FAIL: tilt rim\n");
        return 1;
    }
    const auto pca = ComputePcaAlignment(rim.rimPoints, MeshCentroid(tilt));
    if (!pca.ok) {
        std::fprintf(stderr, "FAIL: tilt pca\n");
        return 1;
    }
    for (Point3& p : tilt.vertices) {
        p = pca.transform.apply(p);
    }
    const Axis3D axT = OptimizeRotationAxis(FitHorizontalOuters(tilt, zBand(tilt, 40)));
    if (!checkAxis(axT, "tilt15+pca")) {
        return 1;
    }

    std::puts("PASS");
    return 0;
}
