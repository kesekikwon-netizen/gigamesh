/* DongArch3D / K-ArchProfile horizontal-slice annulus (plan 5.3) (GPLv3+) */

#include "AnnulusClassifier.h"

#include <cmath>

namespace DongArch {
namespace KArch {
namespace {

double signedArea(const std::vector<Point2>& pts) {
    if (pts.size() < 3) {
        return 0.0;
    }
    double a = 0.0;
    for (std::size_t i = 0; i < pts.size(); ++i) {
        const Point2& p = pts[i];
        const Point2& q = pts[(i + 1) % pts.size()];
        a += p.x * q.y - q.x * p.y;
    }
    return 0.5 * a;
}

bool isClosed(const std::vector<Point2>& pts, double eps) {
    if (pts.size() < 3) {
        return false;
    }
    const double dx = pts.front().x - pts.back().x;
    const double dy = pts.front().y - pts.back().y;
    return std::sqrt(dx * dx + dy * dy) < eps;
}

} // namespace

AnnulusResult ClassifyAndFitOuter(const std::vector<Polyline2>& loops,
                                  double closeEpsMm) {
    AnnulusResult result;
    int best = -1;
    double bestAbs = -1.0;

    for (int i = 0; i < static_cast<int>(loops.size()); ++i) {
        const auto& pts = loops[static_cast<std::size_t>(i)].pts;
        if (!isClosed(pts, closeEpsMm)) {
            continue;
        }
        ++result.closedCount;
        const double absA = std::fabs(signedArea(pts));
        if (absA > bestAbs) {
            bestAbs = absA;
            best = i;
        }
    }

    if (best < 0) {
        return result;
    }

    result.outerIndex = best;
    std::vector<Point2> fitPts = loops[static_cast<std::size_t>(best)].pts;
    if (fitPts.size() >= 2 && isClosed(fitPts, closeEpsMm)) {
        fitPts.pop_back();
    }
    result.outerCircle = FitCircleTaubin(fitPts);
    result.ok = result.outerCircle.ok;
    return result;
}

} // namespace KArch
} // namespace DongArch
