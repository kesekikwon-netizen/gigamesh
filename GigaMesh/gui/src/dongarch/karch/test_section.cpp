/* Standalone T5: right wall 5×100, hatch inside, spacing 1.5. */

#include "SectionComposer.h"
#include "Silhouette.h"
#include "SyntheticMeshGen.h"

#include <cmath>
#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>

using DongArch::KArch::ComposeSection;
using DongArch::KArch::ExtractSilhouette;
using DongArch::KArch::Point2;
using DongArch::KArch::SectionParams;
using DongArch::KArch::SilhouetteMode;
using DongArch::KArch::WriteDrawingSvg;
using DongArch::KArch::Testing::MakeCylinderShell;
using DongArch::KArch::Testing::SyntheticParams;

static double polyArea(const std::vector<Point2>& p) {
    double a = 0.0;
    const int n = static_cast<int>(p.size());
    for (int i = 0; i < n; ++i) {
        const Point2& u = p[static_cast<std::size_t>(i)];
        const Point2& v = p[static_cast<std::size_t>((i + 1) % n)];
        a += u.x * v.y - v.x * u.y;
    }
    return 0.5 * std::abs(a);
}

static bool pointIn(const Point2& q, const std::vector<Point2>& p) {
    bool in = false;
    const int n = static_cast<int>(p.size());
    for (int i = 0, j = n - 1; i < n; j = i++) {
        const Point2& a = p[static_cast<std::size_t>(i)];
        const Point2& b = p[static_cast<std::size_t>(j)];
        const bool hit = ((a.y > q.y) != (b.y > q.y)) &&
                         (q.x < (b.x - a.x) * (q.y - a.y) / ((b.y - a.y) + 1e-30) + a.x);
        if (hit) {
            in = !in;
        }
    }
    return in;
}

static std::string outPath(int argc, char** argv, const char* name) {
    std::string dir = (argc > 1 && argv[1] != nullptr) ? argv[1] : "";
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') {
        dir += '/';
    }
    return dir + name; // argv[1] = output dir, else relative to CTest cwd (build folder)
}

int main(int argc, char** argv) {
    SyntheticParams sp;
    sp.radialSeg = 180;
    sp.heightSeg = 20;
    sp.capTop = true;
    sp.capBottom = true;
    const auto mesh = MakeCylinderShell(50.0, 100.0, 5.0, sp).toMesh();

    SectionParams p;
    const auto sec = ComposeSection(mesh, p);
    if (!sec.ok || sec.wallPolygons.empty()) {
        std::fprintf(stderr, "FAIL section ok=%d\n", static_cast<int>(sec.ok));
        return 1;
    }
    const double area = polyArea(sec.wallPolygons[0]);
    const double areaErr = std::abs(area - 500.0) / 500.0;
    std::printf("wall area=%.4f err=%.4f%% thick=%.4f hatch=%zu\n", area, areaErr * 100.0,
                sec.minWallThickness, sec.hatchLines.size());
    if (areaErr >= 0.005) {
        std::fprintf(stderr, "FAIL area\n");
        return 1;
    }
    if (std::abs(sec.minWallThickness - 5.0) >= 0.1) {
        std::fprintf(stderr, "FAIL thickness %.4f\n", sec.minWallThickness);
        return 1;
    }

    const auto& wall = sec.wallPolygons[0];
    int outside = 0;
    for (const auto& h : sec.hatchLines) {
        for (int k = 1; k <= 3; ++k) {
            const double t = static_cast<double>(k) / 4.0;
            const Point2 q{h.first.x + t * (h.second.x - h.first.x),
                           h.first.y + t * (h.second.y - h.first.y)};
            if (!pointIn(q, wall)) {
                ++outside;
            }
        }
    }
    if (outside > 0) {
        std::fprintf(stderr, "FAIL hatch outside %d\n", outside);
        return 1;
    }

    const double nx = -std::sin(45.0 * 3.14159265358979323846 / 180.0);
    const double nz = std::cos(45.0 * 3.14159265358979323846 / 180.0);
    std::vector<double> cs;
    for (const auto& h : sec.hatchLines) {
        const Point2 m{(h.first.x + h.second.x) * 0.5, (h.first.y + h.second.y) * 0.5};
        cs.push_back(m.x * nx + m.y * nz);
    }
    std::sort(cs.begin(), cs.end());
    double spErr = 0.0;
    int spN = 0;
    for (std::size_t i = 1; i < cs.size(); ++i) {
        const double d = cs[i] - cs[i - 1];
        if (d < 0.2) {
            continue;
        }
        const double k = std::round(d / 1.5);
        if (k < 1.0) {
            continue;
        }
        spErr = std::max(spErr, std::abs(d / k - 1.5));
        ++spN;
    }
    std::printf("hatch spacing err=%.5f n=%d\n", spErr, spN);
    if (spN < 3 || spErr >= 0.01) {
        std::fprintf(stderr, "FAIL spacing\n");
        return 1;
    }

    const auto sil = ExtractSilhouette(mesh, SilhouetteMode::MaxRadius, 0.25);
    if (!WriteDrawingSvg(outPath(argc, argv, "karch_t5_cyl.svg"), sil.leftOutline, sec)) {
        std::fprintf(stderr, "FAIL svg\n");
        return 1;
    }
    std::printf("PASS\n");
    return 0;
}
