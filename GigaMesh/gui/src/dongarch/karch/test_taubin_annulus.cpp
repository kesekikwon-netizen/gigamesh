/* Standalone T0 check: synthetic cylinder shell slice → outer radius. */

#include "AnnulusClassifier.h"

#include <cmath>
#include <cstdio>
#include <numbers>
#include <vector>

using DongArch::KArch::AnnulusResult;
using DongArch::KArch::ClassifyAndFitOuter;
using DongArch::KArch::FitCircleTaubin;
using DongArch::KArch::Point2;
using DongArch::KArch::Polyline2;

static Polyline2 makeCircle(double r, int n, bool closed) {
    Polyline2 loop;
    loop.pts.reserve(static_cast<std::size_t>(n) + (closed ? 1 : 0));
    for (int i = 0; i < n; ++i) {
        const double t = 2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(n);
        loop.pts.push_back(Point2{r * std::cos(t), r * std::sin(t)});
    }
    if (closed) {
        loop.pts.push_back(loop.pts.front());
    }
    return loop;
}

int main() {
    constexpr double rOuter = 50.0;
    constexpr double thickness = 5.0;
    constexpr double rInner = rOuter - thickness;
    constexpr int segs = 180;

    // z=50 horizontal slice of a cylinder shell: outer + inner rings.
    const std::vector<Polyline2> slice = {
        makeCircle(rOuter, segs, true),
        makeCircle(rInner, segs, true),
    };

    const AnnulusResult classified = ClassifyAndFitOuter(slice);
    if (!classified.ok) {
        std::fprintf(stderr, "FAIL: annulus classify/fit failed\n");
        return 1;
    }
    if (classified.closedCount != 2) {
        std::fprintf(stderr, "FAIL: closedCount=%d expected 2\n", classified.closedCount);
        return 1;
    }
    if (classified.outerIndex != 0) {
        std::fprintf(stderr, "FAIL: outerIndex=%d expected 0\n", classified.outerIndex);
        return 1;
    }

    const double err = std::fabs(classified.outerCircle.r - rOuter);
    std::printf("outer R=%.12f err=%.3e rmse=%.3e xc=%.3e yc=%.3e\n",
                classified.outerCircle.r, err, classified.outerCircle.rmse,
                classified.outerCircle.xc, classified.outerCircle.yc);

    // Noise-free analytic circle: TaubinSVD must recover R within 1e-6 mm.
    if (err >= 1e-6) {
        std::fprintf(stderr, "FAIL: outer radius error %.3e >= 1e-6\n", err);
        return 1;
    }

    const auto innerOnly = FitCircleTaubin(slice[1].pts);
    if (!innerOnly.ok || std::fabs(innerOnly.r - rInner) >= 1e-6) {
        std::fprintf(stderr, "FAIL: inner self-fit R=%.12f\n", innerOnly.r);
        return 1;
    }
    if (std::fabs(classified.outerCircle.r - innerOnly.r) < 1.0) {
        std::fprintf(stderr, "FAIL: outer fit collapsed toward annulus mean\n");
        return 1;
    }

    const std::vector<Polyline2> innerFirst = {
        makeCircle(rInner, segs, true),
        makeCircle(rOuter, segs, true),
    };
    const AnnulusResult swapped = ClassifyAndFitOuter(innerFirst);
    if (!swapped.ok || swapped.outerIndex != 1 ||
        std::fabs(swapped.outerCircle.r - rOuter) >= 1e-6) {
        std::fprintf(stderr, "FAIL: inner-first outerIndex=%d R=%.12f\n",
                     swapped.outerIndex, swapped.outerCircle.r);
        return 1;
    }

    std::puts("PASS");
    return 0;
}
