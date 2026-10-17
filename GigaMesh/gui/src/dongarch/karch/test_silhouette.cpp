/* Standalone T4: MaxRadius x=-R, TrueOrtho includes left handle. */

#include "Silhouette.h"
#include "SyntheticMeshGen.h"

#include <cmath>
#include <cstdio>
#include <algorithm>
#include <vector>

using DongArch::KArch::ExtractSilhouette;
using DongArch::KArch::MeshData;
using DongArch::KArch::Point3;
using DongArch::KArch::SilhouetteMode;
using DongArch::KArch::Testing::MakeConeShell;
using DongArch::KArch::Testing::MakeCylinderShell;
using DongArch::KArch::Testing::SyntheticParams;

static void addLeftHandle(MeshData& m) {
    const int b = static_cast<int>(m.vertices.size());
    const Point3 c[8] = {
        {-80, -2, 45}, {-60, -2, 45}, {-60, 2, 45}, {-80, 2, 45},
        {-80, -2, 55}, {-60, -2, 55}, {-60, 2, 55}, {-80, 2, 55},
    };
    for (const Point3& p : c) {
        m.vertices.push_back(p);
    }
    const int f[][3] = {{0, 1, 2}, {0, 2, 3}, {4, 6, 5}, {4, 7, 6},
                        {0, 4, 5}, {0, 5, 1}, {3, 2, 6}, {3, 6, 7},
                        {0, 3, 7}, {0, 7, 4}, {1, 5, 6}, {1, 6, 2}};
    for (const auto& t : f) {
        m.faces.push_back({b + t[0], b + t[1], b + t[2]});
    }
}

static double maxDevNegR(const std::vector<DongArch::KArch::Point2>& poly, double r) {
    double d = 0.0;
    for (const auto& p : poly) {
        d = std::max(d, std::abs(p.x + r));
    }
    return d;
}

int main() {
    SyntheticParams p;
    p.radialSeg = 180;
    p.heightSeg = 20;
    p.capTop = true;
    p.capBottom = true;

    const auto cyl = ExtractSilhouette(MakeCylinderShell(50.0, 100.0, 5.0, p).toMesh(),
                                       SilhouetteMode::MaxRadius, 0.25);
    if (!cyl.ok || cyl.leftOutline.size() < 10) {
        std::fprintf(stderr, "FAIL cyl ok=%d n=%zu\n", static_cast<int>(cyl.ok),
                     cyl.leftOutline.size());
        return 1;
    }
    const double cylDev = maxDevNegR(cyl.leftOutline, 50.0);
    std::printf("cyl MaxRadius n=%zu maxDev=%.6f maxR=%.6f\n", cyl.leftOutline.size(), cylDev,
                cyl.maxBodyRadius);
    if (cylDev >= 0.05) {
        std::fprintf(stderr, "FAIL cyl x=-50 dev=%.6f\n", cylDev);
        return 1;
    }

    const auto coneMesh = MakeConeShell(50.0, 30.0, 100.0, 5.0, p).toMesh();
    const auto cone = ExtractSilhouette(coneMesh, SilhouetteMode::MaxRadius, 0.25);
    double coneDev = 0.0;
    for (const auto& q : cone.leftOutline) {
        const double expectR = 50.0 + (30.0 - 50.0) * (q.y / 100.0);
        coneDev = std::max(coneDev, std::abs(q.x + expectR));
    }
    std::printf("cone MaxRadius n=%zu taperDev=%.6f\n", cone.leftOutline.size(), coneDev);
    if (!cone.ok || coneDev >= 0.05) {
        std::fprintf(stderr, "FAIL cone taper dev=%.6f\n", coneDev);
        return 1;
    }

    MeshData handled = MakeCylinderShell(50.0, 100.0, 5.0, p).toMesh();
    addLeftHandle(handled);
    const auto mr = ExtractSilhouette(handled, SilhouetteMode::MaxRadius, 0.25);
    const auto to = ExtractSilhouette(handled, SilhouetteMode::TrueOrtho, 0.25);
    const double mrDev = maxDevNegR(mr.leftOutline, 50.0);
    double toMin = 0.0;
    for (const auto& q : to.leftOutline) {
        if (q.y >= 45.0 && q.y <= 55.0) {
            toMin = std::min(toMin, q.x);
        }
    }
    std::printf("handle MaxRdev=%.6f TrueOrtho minX@45-55=%.3f warn=%s\n", mrDev, toMin,
                to.warning.c_str());
    if (mrDev >= 0.05) {
        std::fprintf(stderr, "FAIL MaxRadius ate handle\n");
        return 1;
    }
    if (toMin > -65.0) {
        std::fprintf(stderr, "FAIL TrueOrtho missed handle minX=%.3f\n", toMin);
        return 1;
    }
    if (to.warning != "ASYMMETRIC_GEOMETRY") {
        std::fprintf(stderr, "FAIL expected ASYMMETRIC_GEOMETRY\n");
        return 1;
    }

    p.noiseSigma = 0.2;
    p.seed = 42;
    const auto noisy = ExtractSilhouette(MakeCylinderShell(50.0, 100.0, 5.0, p).toMesh(),
                                         SilhouetteMode::MaxRadius, 0.25);
    std::vector<double> xs;
    for (const auto& q : noisy.leftOutline) {
        if (q.y >= 20.0 && q.y <= 80.0) {
            xs.push_back(q.x);
        }
    }
    std::sort(xs.begin(), xs.end());
    const double med = xs[xs.size() / 2];
    double amp = 0.0;
    for (double x : xs) {
        amp = std::max(amp, std::abs(x - med));
    }
    amp *= 2.0;
    std::printf("noise amp=%.6f\n", amp);
    if (amp >= 0.1) {
        std::fprintf(stderr, "FAIL noise amplitude %.6f\n", amp);
        return 1;
    }

    std::printf("PASS\n");
    return 0;
}
