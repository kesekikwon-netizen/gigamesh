/* DongArch3D / K-ArchProfile T0 synthetic verification meshes (GPLv3+) */

#ifndef DONGARCH_KARCH_SYNTHETIC_MESH_GEN_H
#define DONGARCH_KARCH_SYNTHETIC_MESH_GEN_H

#include "RimDetector.h"

#include <array>
#include <string>
#include <vector>

namespace DongArch {
namespace KArch {
namespace Testing {

struct SyntheticParams {
    double noiseSigma = 0.0; // mm, Gaussian vertex noise
    double tiltDeg = 0.0;    // rotate about +X after generation
    Point3 translation{};
    int radialSeg = 180;
    int heightSeg = 100;
    unsigned seed = 42;
    bool capTop = true;
    bool capBottom = true;
};

struct SyntheticMesh {
    std::vector<Point3> vertices;
    std::vector<std::array<int, 3>> faces;
    double gtRimRadius = 0.0;
    double gtBaseRadius = 0.0;
    double gtBellyRadius = 0.0;
    double gtHeight = 0.0;
    double gtWallThick = 0.0;
    Point3 gtAxisOrigin{};
    Point3 gtAxisDir{0.0, 0.0, 1.0};

    MeshData toMesh() const;
};

// Thick shells: outer + inner rings. Caps optional (open rim = capTop false).
SyntheticMesh MakeCylinderShell(double rOuter, double height, double thickness,
                                const SyntheticParams& p = {});
SyntheticMesh MakeConeShell(double rBottom, double rTop, double height,
                            double thickness, const SyntheticParams& p = {});
SyntheticMesh MakeTorusProfileVessel(double bellyRadius, double rimRadius,
                                     double height, double thickness,
                                     const SyntheticParams& p = {});
SyntheticMesh MakeArcFragment(const SyntheticMesh& full, double arcDeg);

bool WritePlyAscii(const SyntheticMesh& m, const std::string& path);
bool ReadPlyAscii(const std::string& path, MeshData& out);

} // namespace Testing
} // namespace KArch
} // namespace DongArch

#endif
