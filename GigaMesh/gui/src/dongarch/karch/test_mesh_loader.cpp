/* Standalone T1 check: Load PLY/OBJ → slice z=50 → 2 closed, outer R err < 0.01. */

#include "AnnulusClassifier.h"
#include "MeshLoader.h"
#include "MeshSlicer.h"
#include "SyntheticMeshGen.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using DongArch::KArch::ClassifyAndFitOuter;
using DongArch::KArch::LoadError;
using DongArch::KArch::LoadMesh;
using DongArch::KArch::LoadResult;
using DongArch::KArch::Plane;
using DongArch::KArch::Polyline2;
using DongArch::KArch::Slice;
using DongArch::KArch::WriteObjAscii;
using DongArch::KArch::Testing::MakeCylinderShell;
using DongArch::KArch::Testing::SyntheticParams;
using DongArch::KArch::Testing::WritePlyAscii;

static bool checkSlice50(const LoadResult& loaded, double expectR, const char* tag) {
    if (loaded.error != LoadError::Ok) {
        std::fprintf(stderr, "FAIL %s: load %d %s\n", tag,
                     static_cast<int>(loaded.error), loaded.message.c_str());
        return false;
    }
    const auto loops3 = Slice(loaded.mesh, Plane{{0, 0, 1}, -50.0 * (expectR / 50.0)});
    std::vector<Polyline2> loops2;
    loops2.reserve(loops3.size());
    for (const auto& pl : loops3) {
        Polyline2 l;
        l.pts.reserve(pl.pts.size());
        for (const auto& p : pl.pts) {
            l.pts.push_back({p.x, p.y});
        }
        loops2.push_back(std::move(l));
    }
    const auto ann = ClassifyAndFitOuter(loops2);
    if (!ann.ok || ann.closedCount != 2) {
        std::fprintf(stderr, "FAIL %s: closed=%d ok=%d loops3=%zu\n", tag, ann.closedCount,
                     static_cast<int>(ann.ok), loops3.size());
        return false;
    }
    const double err = std::abs(ann.outerCircle.r - expectR);
    std::printf("%s closed=2 R=%.9f err=%.3e boundary=%d\n", tag, ann.outerCircle.r, err,
                static_cast<int>(loaded.mesh.hasBoundary));
    if (err >= 0.01) {
        std::fprintf(stderr, "FAIL %s: radius err %.9f >= 0.01\n", tag, err);
        return false;
    }
    return true;
}

int main() {
    SyntheticParams p;
    p.radialSeg = 180;
    p.heightSeg = 20;
    p.capTop = false;
    p.capBottom = true;
    const auto cyl = MakeCylinderShell(50.0, 100.0, 5.0, p);

    const std::string ply = "karch_t1_cyl.ply";
    const std::string obj = "karch_t1_cyl.obj";
    if (!WritePlyAscii(cyl, ply) || !WriteObjAscii(cyl.toMesh(), obj)) {
        std::fprintf(stderr, "FAIL write ply/obj\n");
        return 1;
    }

    if (!checkSlice50(LoadMesh(ply), 50.0, "ply")) {
        return 1;
    }
    if (!checkSlice50(LoadMesh(obj), 50.0, "obj")) {
        return 1;
    }
    if (!checkSlice50(LoadMesh(ply, 2.0), 100.0, "ply*2")) {
        return 1;
    }

    {
        const LoadResult miss = LoadMesh("karch_t1_missing.ply");
        if (miss.error != LoadError::FileNotFound) {
            std::fprintf(stderr, "FAIL missing -> %d\n", static_cast<int>(miss.error));
            return 1;
        }
        std::ofstream empty("karch_t1_empty.ply", std::ios::binary);
        empty.close();
        const LoadResult emp = LoadMesh("karch_t1_empty.ply");
        if (emp.error != LoadError::ParseFailed && emp.error != LoadError::EmptyMesh) {
            std::fprintf(stderr, "FAIL empty -> %d\n", static_cast<int>(emp.error));
            return 1;
        }
        std::ofstream junk("karch_t1_junk.xyz", std::ios::binary);
        junk << "nope\n";
        junk.close();
        const LoadResult uns = LoadMesh("karch_t1_junk.xyz");
        if (uns.error != LoadError::UnsupportedFormat) {
            std::fprintf(stderr, "FAIL xyz -> %d\n", static_cast<int>(uns.error));
            return 1;
        }
    }

    std::remove(ply.c_str());
    std::remove(obj.c_str());
    std::remove("karch_t1_empty.ply");
    std::remove("karch_t1_junk.xyz");
    std::printf("PASS\n");
    return 0;
}
