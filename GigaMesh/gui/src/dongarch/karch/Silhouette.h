/* DongArch3D / K-ArchProfile T4 left silhouette (GPLv3+) */

#ifndef DONGARCH_KARCH_SILHOUETTE_H
#define DONGARCH_KARCH_SILHOUETTE_H

#include "RimDetector.h"
#include "TaubinFit.h"

#include <string>
#include <vector>

namespace DongArch {
namespace KArch {

enum class SilhouetteMode {
    MaxRadius = 0,
    TrueOrtho
};

struct SilhouetteResult {
    std::vector<Point2> leftOutline; // drawing (x, z), x < 0, z ascending
    double maxBodyRadius = 0.0;
    double maxBodyZ = 0.0;
    std::string warning;
    bool ok = false;
};

// MaxRadius: cylindrical r P98 per z-bin, SG(7,2), left = (-r, z).
// TrueOrtho: min X of triangle ∩ z=const (XZ ortho), no Clipper2.
SilhouetteResult ExtractSilhouette(const MeshData& alignedMesh,
                                   SilhouetteMode mode = SilhouetteMode::MaxRadius,
                                   double zBin = 0.25);

} // namespace KArch
} // namespace DongArch

#endif
