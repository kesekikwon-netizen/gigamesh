/* DongArch3D / K-ArchProfile rotation-axis ODR (plan 5.4) (GPLv3+) */

#ifndef DONGARCH_KARCH_AXIS_ODR_H
#define DONGARCH_KARCH_AXIS_ODR_H

#include "AnnulusClassifier.h"
#include "MeshSlicer.h"

namespace DongArch {
namespace KArch {

struct SliceCircle {
    Circle2D circle;
    double z = 0.0;
};

struct Axis3D {
    Point3 origin{};
    Point3 direction{0, 0, 1};
    double rmse = 0.0;
    int usedSlices = 0;
    bool ok = false;
    bool usedPcaFallback = false;
};

std::vector<SliceCircle> FitHorizontalOuters(const MeshData& mesh,
                                             const std::vector<double>& zHeights);

Axis3D OptimizeRotationAxis(const std::vector<SliceCircle>& slices);

} // namespace KArch
} // namespace DongArch

#endif
