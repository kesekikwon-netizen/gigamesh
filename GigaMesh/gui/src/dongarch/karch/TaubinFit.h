/* DongArch3D / K-ArchProfile TaubinSVD circle fit (GPLv3+) */

#ifndef DONGARCH_KARCH_TAUBIN_FIT_H
#define DONGARCH_KARCH_TAUBIN_FIT_H

#include <cstddef>
#include <vector>

namespace DongArch {
namespace KArch {

struct Point2 {
    double x = 0.0;
    double y = 0.0;
};

struct Circle2D {
    double xc = 0.0;
    double yc = 0.0;
    double r = 0.0;
    double rmse = 0.0;
    int pointCount = 0;
    bool ok = false;
};

// Chernov TaubinSVD. V is the thin right-singular basis of the Mx3 design
// matrix (same as Eigen::JacobiSVD + ComputeThinV, V.col(2) = smallest).
// See https://libeigen.gitlab.io/eigen/docs-3.4/classEigen_1_1JacobiSVD.html
Circle2D FitCircleTaubin(const std::vector<Point2>& points);

} // namespace KArch
} // namespace DongArch

#endif
