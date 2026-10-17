/* Standalone T8: R12 11 layers, 1:1 mm, ASCII TEXT, no LWPOLYLINE. */

#include "DimensionAnalyzer.h"
#include "DxfWriter.h"
#include "SectionComposer.h"
#include "Silhouette.h"
#include "SyntheticMeshGen.h"

#include <cstdio>
#include <fstream>
#include <string>

using DongArch::KArch::AnalyzeDimensions;
using DongArch::KArch::ComposeSection;
using DongArch::KArch::ExtractSilhouette;
using DongArch::KArch::SilhouetteMode;
using DongArch::KArch::WritePlateDxf;
using DongArch::KArch::Testing::MakeArcFragment;
using DongArch::KArch::Testing::MakeCylinderShell;
using DongArch::KArch::Testing::SyntheticParams;

static bool has(const std::string& s, const char* n) {
    return s.find(n) != std::string::npos;
}

static bool hasEntityDashed(const std::string& s) {
    const auto ent = s.find("\nENTITIES\n");
    if (ent == std::string::npos) {
        return false;
    }
    return s.find("\n6\nDASHED\n", ent) != std::string::npos;
}

static std::string outPath(const char* name) {
    const std::string preferred = std::string("D:\\gigamesh\\") + name;
    std::ofstream probe(preferred.c_str(), std::ios::out | std::ios::app);
    if (probe) {
        probe.close();
        return preferred;
    }
    return name;
}

static std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

int main() {
    SyntheticParams sp;
    sp.radialSeg = 180;
    sp.heightSeg = 20;
    sp.capTop = true;
    sp.capBottom = true;
    const auto cyl = MakeCylinderShell(50.0, 100.0, 5.0, sp);
    const auto mesh = cyl.toMesh();
    const auto sil = ExtractSilhouette(mesh, SilhouetteMode::MaxRadius, 0.25);
    const auto sec = ComposeSection(mesh);
    const auto dim = AnalyzeDimensions(mesh, sil);
    const std::string dxfPath = outPath("karch_t8_cyl.dxf");
    if (!dim.ok || !WritePlateDxf(dxfPath, sil.leftOutline, sec, dim)) {
        std::fprintf(stderr, "FAIL write dxf\n");
        return 1;
    }

    const std::string dxf = slurp(dxfPath);
    const char* layers[] = {
        "01_AXIS_CENTER",       "02_EXTERIOR_OUTLINE",  "03_EXTERIOR_FEATURES",
        "04_SECTION_WALL",      "05_SECTION_HATCH",     "06_INTERIOR_OUTLINE",
        "07_PROJECTION_RIM",    "08_PROJECTION_BASE",   "09_DIMENSION_MEASURE",
        "10_ANNOTATION_TEXT",   "11_SCALE_BAR",
    };
    for (const char* L : layers) {
        if (!has(dxf, L)) {
            std::fprintf(stderr, "FAIL missing layer %s\n", L);
            return 1;
        }
    }
    if (!has(dxf, "AC1009") || !has(dxf, "POLYLINE") || !has(dxf, "VERTEX") || !has(dxf, "SEQEND") ||
        !has(dxf, "CIRCLE") || !has(dxf, "RIM D. 100.0mm") || !has(dxf, "BASE D. 100.0mm") ||
        !has(dxf, "H 100.0mm") || !has(dxf, "BODY D. 100.0mm") || !has(dxf, "SCALE 1:1") ||
        !has(dxf, "\n40\n50.000000\n")) {
        std::fprintf(stderr, "FAIL required R12 tokens\n");
        return 1;
    }
    if (has(dxf, "LWPOLYLINE") || has(dxf, "구연") || has(dxf, "100,0mm") || hasEntityDashed(dxf)) {
        std::fprintf(stderr, "FAIL forbidden token or dashed full ring\n");
        return 1;
    }
    if (dxf.find("$INSUNITS") == std::string::npos || dxf.find("\n70\n4\n") == std::string::npos) {
        std::fprintf(stderr, "FAIL INSUNITS mm\n");
        return 1;
    }

    const auto fragMesh = MakeArcFragment(cyl, 144.0).toMesh();
    const auto silF = ExtractSilhouette(fragMesh, SilhouetteMode::MaxRadius, 0.25);
    const auto secF = ComposeSection(fragMesh);
    const auto dimF = AnalyzeDimensions(fragMesh, silF);
    const std::string fragPath = outPath("karch_t8_frag.dxf");
    if (!dimF.ok || !dimF.rimIsFragment || !WritePlateDxf(fragPath, silF.leftOutline, secF, dimF)) {
        std::fprintf(stderr, "FAIL frag dxf ok=%d frag=%d\n", static_cast<int>(dimF.ok),
                     static_cast<int>(dimF.rimIsFragment));
        return 1;
    }
    const std::string dxfF = slurp(fragPath);
    if (!hasEntityDashed(dxfF) || !has(dxfF, "ARC")) {
        std::fprintf(stderr, "FAIL restored fragment not entity-dashed ARC\n");
        return 1;
    }
    std::printf("dxf %s bytes=%zu frag=%zu\n", dxfPath.c_str(), dxf.size(), dxfF.size());
    return 0;
}
