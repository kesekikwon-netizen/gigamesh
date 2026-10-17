/* DongArch3D / K-ArchProfile horizontal-slice annulus (plan 5.3) (GPLv3+) */

#ifndef DONGARCH_KARCH_ANNULUS_CLASSIFIER_H
#define DONGARCH_KARCH_ANNULUS_CLASSIFIER_H

#include "TaubinFit.h"

#include <vector>

namespace DongArch {
namespace KArch {

struct Polyline2 {
    std::vector<Point2> pts;
};

struct AnnulusResult {
    Circle2D outerCircle;
    int outerIndex = -1;
    int closedCount = 0;
    bool ok = false;
};

// Closed if |start-end| < closeEpsMm. Largest |signed_area| is outer.
// Circle fit is applied to the outer polyline only.
AnnulusResult ClassifyAndFitOuter(const std::vector<Polyline2>& loops,
                                  double closeEpsMm = 0.05);

} // namespace KArch
} // namespace DongArch

#endif
