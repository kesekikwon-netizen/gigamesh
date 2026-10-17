/* DongArch3D / K-ArchProfile rim PCA alignment (plan 5.1) (GPLv3+) */

#ifndef DONGARCH_KARCH_AXIS_PCA_H
#define DONGARCH_KARCH_AXIS_PCA_H

#include "RimDetector.h"

namespace DongArch {
namespace KArch {

struct Transform4 {
    // Column-major: m[c * 4 + r] = row r, col c. Eigen default.
    double m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    Point3 apply(const Point3& p) const;
};

struct PcaAlignment {
    Transform4 transform;
    Point3 rimNormal = {0, 0, 1};
    Point3 rimCentroid = {};
    double planarity = 0.0;
    bool ok = false;
    bool planarityWarn = false;
};

// meshCentroid is the whole-mesh barycenter used for normal sign (outward / rim-up).
PcaAlignment ComputePcaAlignment(const std::vector<Point3>& rimPoints,
                                 const Point3& meshCentroid);

Point3 MeshCentroid(const MeshData& mesh);

} // namespace KArch
} // namespace DongArch

#endif
