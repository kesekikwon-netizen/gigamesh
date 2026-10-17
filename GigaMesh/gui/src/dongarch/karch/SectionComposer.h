/* DongArch3D / K-ArchProfile T5 right-half section + hatch (GPLv3+) */

#ifndef DONGARCH_KARCH_SECTION_COMPOSER_H
#define DONGARCH_KARCH_SECTION_COMPOSER_H

#include "MeshSlicer.h"
#include "TaubinFit.h"

#include <string>
#include <utility>
#include <vector>

namespace DongArch {
namespace KArch {

struct SectionParams {
    double hatchSpacing = 1.5;
    double hatchAngleDeg = 45.0;
    double simplifyTol = 0.02;
    double minHatchLen = 0.3;
};

struct SectionResult {
    std::vector<std::vector<Point2>> wallPolygons; // CCW, drawing (x,z)
    std::vector<std::pair<Point2, Point2>> hatchLines;
    std::vector<Point2> interiorOutline;
    double minWallThickness = 0.0;
    double maxWallThickness = 0.0;
    std::string warning;
    bool ok = false;
};

// Y=0 cut, keep X>0, close wall loops, 45° hatch clipped to the wall.
SectionResult ComposeSection(const MeshData& alignedMesh, const SectionParams& p = {});

// SVG plate preview (left outline + right section). z up.
bool WriteDrawingSvg(const std::string& path, const std::vector<Point2>& leftOutline,
                     const SectionResult& section);

} // namespace KArch
} // namespace DongArch

#endif
