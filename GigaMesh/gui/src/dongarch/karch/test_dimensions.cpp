/* Standalone T6: 4 synthetics err < 0.2 mm, 0.1 mm labels, 144 deg fragment dashed. */

#include "DimensionAnalyzer.h"
#include "SectionComposer.h"
#include "Silhouette.h"
#include "SyntheticMeshGen.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>

using DongArch::KArch::AnalyzeDimensions;
using DongArch::KArch::ComposeSection;
using DongArch::KArch::Dimensions;
using DongArch::KArch::ExtractSilhouette;
using DongArch::KArch::RoundTenths;
using DongArch::KArch::SilhouetteMode;
using DongArch::KArch::WritePlateSvg;
using DongArch::KArch::Testing::MakeArcFragment;
using DongArch::KArch::Testing::MakeConeShell;
using DongArch::KArch::Testing::MakeCylinderShell;
using DongArch::KArch::Testing::MakeTorusProfileVessel;
using DongArch::KArch::Testing::SyntheticMesh;
using DongArch::KArch::Testing::SyntheticParams;

static bool checkSpecies(const char* name, const SyntheticMesh& syn, bool expectRimFragment) {
    const auto mesh = syn.toMesh();
    const auto sil = ExtractSilhouette(mesh, SilhouetteMode::MaxRadius, 0.25);
    const auto dim = AnalyzeDimensions(mesh, sil);
    const double rimGt = 2.0 * syn.gtRimRadius;
    const double baseGt = 2.0 * syn.gtBaseRadius;
    const double bodyGt = 2.0 * syn.gtBellyRadius;
    const double rimErr = std::abs(2.0 * dim.rimArc.radius - rimGt);
    const double baseErr = std::abs(2.0 * dim.baseArc.radius - baseGt);
    const double hErr = std::abs(dim.vesselHeight - syn.gtHeight);
    const double bodyErr = std::abs(dim.maxBodyDiameter - bodyGt);
    std::printf("%s ok=%d rim=%.3f(err=%.3f) base=%.3f(err=%.3f) H=%.3f(err=%.3f) "
                "Dmax=%.3f(err=%.3f) frag=%d dash=%d\n",
                name, static_cast<int>(dim.ok), dim.rimDiameter, rimErr, dim.baseDiameter, baseErr,
                dim.vesselHeight, hErr, dim.maxBodyDiameter, bodyErr,
                static_cast<int>(dim.rimIsFragment), static_cast<int>(dim.rimArc.isRestored));
    if (!dim.ok) {
        std::fprintf(stderr, "FAIL %s not ok (%s)\n", name, dim.warning.c_str());
        return false;
    }
    if (rimErr >= 0.2 || baseErr >= 0.2 || hErr >= 0.2 || bodyErr >= 0.2) {
        std::fprintf(stderr, "FAIL %s err>=0.2\n", name);
        return false;
    }
    if (std::abs(dim.rimDiameter - RoundTenths(rimGt)) > 1e-9 ||
        std::abs(dim.baseDiameter - RoundTenths(baseGt)) > 1e-9 ||
        std::abs(dim.vesselHeight - RoundTenths(syn.gtHeight)) > 1e-9 ||
        std::abs(dim.maxBodyDiameter - RoundTenths(bodyGt)) > 1e-9) {
        std::fprintf(stderr, "FAIL %s tenths\n", name);
        return false;
    }
    if (expectRimFragment != (dim.rimIsFragment && dim.rimArc.isRestored)) {
        std::fprintf(stderr, "FAIL %s fragment flag\n", name);
        return false;
    }
    if (std::abs(dim.rimArc.center.x) > 1e-9 || std::abs(dim.baseArc.center.x) > 1e-9) {
        std::fprintf(stderr, "FAIL %s projection not on axis\n", name);
        return false;
    }
    return true;
}

static std::string outPath(int argc, char** argv, const char* name) {
    std::string dir = (argc > 1 && argv[1] != nullptr) ? argv[1] : "";
    if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') {
        dir += '/';
    }
    return dir + name; // argv[1] = output dir, else relative to CTest cwd (build folder)
}

int main(int argc, char** argv) {
    if (std::abs(RoundTenths(12.34) - 12.3) > 1e-12 || std::abs(RoundTenths(12.35) - 12.4) > 1e-12) {
        std::fprintf(stderr, "FAIL RoundTenths\n");
        return 1;
    }

    SyntheticParams sp;
    sp.radialSeg = 180;
    sp.heightSeg = 20;
    sp.capTop = true;
    sp.capBottom = true;

    const auto cyl = MakeCylinderShell(50.0, 100.0, 5.0, sp);
    const auto cone = MakeConeShell(50.0, 30.0, 100.0, 5.0, sp);
    const auto torus = MakeTorusProfileVessel(60.0, 40.0, 100.0, 5.0, sp);
    const auto frag = MakeArcFragment(cyl, 144.0);

    if (!checkSpecies("cyl", cyl, false) || !checkSpecies("cone", cone, false) ||
        !checkSpecies("torus", torus, false) || !checkSpecies("frag144", frag, true)) {
        return 1;
    }

    const auto mesh = cyl.toMesh();
    const auto sil = ExtractSilhouette(mesh, SilhouetteMode::MaxRadius, 0.25);
    const auto sec = ComposeSection(mesh);
    const auto dim = AnalyzeDimensions(mesh, sil);
    const std::string svgPath = outPath(argc, argv, "karch_t6_cyl.svg");
    if (!WritePlateSvg(svgPath, sil.leftOutline, sec, dim)) {
        std::fprintf(stderr, "FAIL write svg\n");
        return 1;
    }
    std::ifstream in(svgPath, std::ios::binary);
    std::string svg((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (svg.find("구연경") == std::string::npos || svg.find("저경") == std::string::npos ||
        svg.find("기고") == std::string::npos || svg.find("최대동체경") == std::string::npos ||
        svg.find("100.0") == std::string::npos) {
        std::fprintf(stderr, "FAIL svg labels\n");
        return 1;
    }
    std::printf("svg %s bytes=%zu\n", svgPath.c_str(), svg.size());

    auto shifted = cyl.toMesh();
    for (auto& v : shifted.vertices) {
        v.z -= 100.0;
    }
    const auto silShift = ExtractSilhouette(shifted, SilhouetteMode::MaxRadius, 0.25);
    const auto secShift = ComposeSection(shifted);
    const auto dimShift = AnalyzeDimensions(shifted, silShift);
    if (!dimShift.ok || std::abs(dimShift.zMin + 100.0) > 1e-9 || std::abs(dimShift.zMax) > 1e-9 ||
        std::abs(dimShift.vesselHeight - 100.0) > 1e-9) {
        std::fprintf(stderr, "FAIL aligned zMin/zMax H=%.3f z=[%.3f,%.3f]\n", dimShift.vesselHeight,
                     dimShift.zMin, dimShift.zMax);
        return 1;
    }
    const std::string svgShift = outPath(argc, argv, "karch_t6_aligned.svg");
    if (!WritePlateSvg(svgShift, silShift.leftOutline, secShift, dimShift)) {
        std::fprintf(stderr, "FAIL write aligned svg\n");
        return 1;
    }
    std::ifstream in2(svgShift, std::ios::binary);
    std::string svg2((std::istreambuf_iterator<char>(in2)), std::istreambuf_iterator<char>());
    if (svg2.find("y1=\"-100") == std::string::npos || svg2.find("y2=\"0") == std::string::npos) {
        std::fprintf(stderr, "FAIL aligned height line not on zMin..zMax\n");
        return 1;
    }
    std::printf("aligned svg z=[%.1f,%.1f]\n", dimShift.zMin, dimShift.zMax);
    return 0;
}
