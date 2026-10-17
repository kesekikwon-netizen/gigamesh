/* DongArch3D / K-ArchProfile T6 dimensions (GPLv3+) */

#ifndef DONGARCH_KARCH_DIMENSION_ANALYZER_H
#define DONGARCH_KARCH_DIMENSION_ANALYZER_H

#include "SectionComposer.h"
#include "Silhouette.h"

#include <string>

namespace DongArch {
namespace KArch {

struct ArcSpec {
    Point2 center{}; // drawing: center.y = z
    double radius = 0.0;
    double startAngleDeg = 0.0;
    double endAngleDeg = 360.0;
    bool isRestored = false; // dashed restored arc
};

struct Dimensions {
    double rimDiameter = 0.0;
    double baseDiameter = 0.0;
    double vesselHeight = 0.0;
    double maxBodyDiameter = 0.0;
    double maxBodyZ = 0.0;
    double zMin = 0.0; // raw mesh Z, for plate placement
    double zMax = 0.0;
    ArcSpec rimArc;
    ArcSpec baseArc;
    bool rimIsFragment = false;
    bool baseIsFragment = false;
    bool ok = false;
    std::string warning;
};

double RoundTenths(double mm);

Dimensions AnalyzeDimensions(const MeshData& alignedMesh, const SilhouetteResult& silhouette);

// Plate with silhouette, section, hatch, projection circles, and 0.1 mm labels.
bool WritePlateSvg(const std::string& path, const std::vector<Point2>& leftOutline,
                   const SectionResult& section, const Dimensions& dims);

} // namespace KArch
} // namespace DongArch

#endif
