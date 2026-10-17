/* DongArch3D / K-ArchProfile MeshData plane slicer (plan 5.5) (GPLv3+) */

#ifndef DONGARCH_KARCH_MESH_SLICER_H
#define DONGARCH_KARCH_MESH_SLICER_H

#include "RimDetector.h"

#include <vector>

namespace DongArch {
namespace KArch {

struct Plane {
    Point3 n = {0, 0, 1};
    double d = 0.0; // n·x + d = 0
};

struct Polyline3 {
    std::vector<Point3> pts;
};

double SignedDistance(const Plane& pl, const Point3& p);
std::vector<Polyline3> Slice(const MeshData& mesh, const Plane& pl);
std::vector<std::vector<Polyline3>> SliceHorizontalBatch(const MeshData& mesh,
                                                         const std::vector<double>& zHeights);

} // namespace KArch
} // namespace DongArch

#endif
