/* Standalone T0 check: 3 shells, seed replay, ASCII PLY round-trip. */

#include "RimDetector.h"
#include "SyntheticMeshGen.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using DongArch::KArch::DetectRimBoundary;
using DongArch::KArch::MeshData;
using DongArch::KArch::Point3;
using DongArch::KArch::RimError;
using DongArch::KArch::Testing::MakeArcFragment;
using DongArch::KArch::Testing::MakeConeShell;
using DongArch::KArch::Testing::MakeCylinderShell;
using DongArch::KArch::Testing::MakeTorusProfileVessel;
using DongArch::KArch::Testing::ReadPlyAscii;
using DongArch::KArch::Testing::SyntheticMesh;
using DongArch::KArch::Testing::SyntheticParams;
using DongArch::KArch::Testing::WritePlyAscii;

static bool sameMesh(const SyntheticMesh& a, const SyntheticMesh& b) {
    if (a.vertices.size() != b.vertices.size() || a.faces.size() != b.faces.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.vertices.size(); ++i) {
        if (a.vertices[i].x != b.vertices[i].x || a.vertices[i].y != b.vertices[i].y ||
            a.vertices[i].z != b.vertices[i].z) {
            return false;
        }
    }
    return a.faces == b.faces;
}

static std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

static bool checkFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return static_cast<bool>(in);
}

static double distToAxis(const Point3& p, const Point3& o, const Point3& d) {
    const double n = std::hypot(d.x, std::hypot(d.y, d.z));
    const double nx = d.x / n;
    const double ny = d.y / n;
    const double nz = d.z / n;
    const double rx = p.x - o.x;
    const double ry = p.y - o.y;
    const double rz = p.z - o.z;
    const double t = rx * nx + ry * ny + rz * nz;
    return std::hypot(rx - t * nx, std::hypot(ry - t * ny, rz - t * nz));
}

static bool sameLoaded(const MeshData& a, const SyntheticMesh& b) {
    if (a.vertices.size() != b.vertices.size() || a.faces.size() != b.faces.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.vertices.size(); ++i) {
        if (std::abs(a.vertices[i].x - b.vertices[i].x) > 1e-12 ||
            std::abs(a.vertices[i].y - b.vertices[i].y) > 1e-12 ||
            std::abs(a.vertices[i].z - b.vertices[i].z) > 1e-12) {
            return false;
        }
    }
    return a.faces == b.faces;
}

int main() {
    SyntheticParams p;
    p.radialSeg = 180;
    p.heightSeg = 20;
    p.seed = 42;
    p.noiseSigma = 0.0;
    p.capTop = true;
    p.capBottom = true;

    const SyntheticMesh cylA = MakeCylinderShell(50.0, 100.0, 5.0, p);
    const SyntheticMesh cylB = MakeCylinderShell(50.0, 100.0, 5.0, p);
    if (!sameMesh(cylA, cylB)) {
        std::fprintf(stderr, "FAIL seed cylinder vertices differ\n");
        return 1;
    }
    if (cylA.gtRimRadius != 50.0 || cylA.gtHeight != 100.0 || cylA.gtWallThick != 5.0) {
        std::fprintf(stderr, "FAIL cylinder GT\n");
        return 1;
    }
    const int expectVert = 2 * (p.heightSeg + 1) * p.radialSeg;
    if (static_cast<int>(cylA.vertices.size()) != expectVert) {
        std::fprintf(stderr, "FAIL cylinder vert count %zu\n", cylA.vertices.size());
        return 1;
    }

    p.noiseSigma = 0.1;
    const SyntheticMesh noisyA = MakeCylinderShell(50.0, 100.0, 5.0, p);
    const SyntheticMesh noisyB = MakeCylinderShell(50.0, 100.0, 5.0, p);
    if (!sameMesh(noisyA, noisyB)) {
        std::fprintf(stderr, "FAIL seed+noise not identical\n");
        return 1;
    }
    p.noiseSigma = 0.0;

    const SyntheticMesh cone = MakeConeShell(50.0, 30.0, 100.0, 5.0, p);
    if (cone.gtRimRadius != 30.0 || cone.gtBaseRadius != 50.0) {
        std::fprintf(stderr, "FAIL cone GT\n");
        return 1;
    }

    const SyntheticMesh torus = MakeTorusProfileVessel(60.0, 40.0, 100.0, 5.0, p);
    if (torus.gtRimRadius != 40.0 || torus.gtBaseRadius != 40.0 ||
        torus.gtBellyRadius != 60.0) {
        std::fprintf(stderr, "FAIL torus GT\n");
        return 1;
    }
    double maxR = 0.0;
    for (const Point3& v : torus.vertices) {
        maxR = std::max(maxR, distToAxis(v, torus.gtAxisOrigin, torus.gtAxisDir));
    }
    if (std::abs(maxR - torus.gtBellyRadius) > 1e-9) {
        std::fprintf(stderr, "FAIL torus belly maxR=%.17g\n", maxR);
        return 1;
    }

    p.tiltDeg = 15.0;
    const SyntheticMesh tilt = MakeCylinderShell(50.0, 100.0, 5.0, p);
    const double ang = std::atan2(std::hypot(tilt.gtAxisDir.x, tilt.gtAxisDir.y),
                                  tilt.gtAxisDir.z) *
                       180.0 / 3.14159265358979323846;
    if (std::abs(ang - 15.0) > 1e-9) {
        std::fprintf(stderr, "FAIL tilt GT ang=%.17g\n", ang);
        return 1;
    }
    for (const Point3& v : tilt.vertices) {
        const double r = distToAxis(v, tilt.gtAxisOrigin, tilt.gtAxisDir);
        const bool outer = std::abs(r - 50.0) < 1e-9;
        const bool inner = std::abs(r - 45.0) < 1e-9;
        if (!outer && !inner) {
            std::fprintf(stderr, "FAIL tilt vertex-axis r=%.17g\n", r);
            return 1;
        }
    }
    double tiltTorusMax = 0.0;
    const SyntheticMesh tiltTorus = MakeTorusProfileVessel(60.0, 40.0, 100.0, 5.0, p);
    for (const Point3& v : tiltTorus.vertices) {
        tiltTorusMax =
            std::max(tiltTorusMax, distToAxis(v, tiltTorus.gtAxisOrigin, tiltTorus.gtAxisDir));
    }
    if (std::abs(tiltTorusMax - tiltTorus.gtBellyRadius) > 1e-9) {
        std::fprintf(stderr, "FAIL tilt torus belly=%.17g\n", tiltTorusMax);
        return 1;
    }
    p.tiltDeg = 0.0;

    SyntheticParams openP = p;
    openP.capTop = false;
    const SyntheticMesh open = MakeCylinderShell(50.0, 100.0, 5.0, openP);
    const auto rim = DetectRimBoundary(open.toMesh());
    if (rim.error != RimError::Ok || rim.loopCount != 2 ||
        static_cast<int>(rim.rimPoints.size()) != 180 ||
        std::abs(rim.meanZ - 100.0) > 1e-9) {
        std::fprintf(stderr, "FAIL open rim loops=%d n=%zu z=%.9g\n", rim.loopCount,
                     rim.rimPoints.size(), rim.meanZ);
        return 1;
    }

    const SyntheticMesh frag = MakeArcFragment(cylA, 90.0);
    if (frag.faces.empty() || frag.faces.size() >= cylA.faces.size() ||
        frag.vertices.size() >= cylA.vertices.size()) {
        std::fprintf(stderr, "FAIL fragment v=%zu/%zu f=%zu/%zu\n", frag.vertices.size(),
                     cylA.vertices.size(), frag.faces.size(), cylA.faces.size());
        return 1;
    }
    p.tiltDeg = 15.0;
    const SyntheticMesh tiltFrag = MakeArcFragment(MakeCylinderShell(50.0, 100.0, 5.0, p), 90.0);
    p.tiltDeg = 0.0;
    if (tiltFrag.vertices.size() >= cylA.vertices.size() || tiltFrag.faces.empty()) {
        std::fprintf(stderr, "FAIL tilt fragment still full\n");
        return 1;
    }

    const std::string p1 = "karch_t0_cyl.ply";
    const std::string p2 = "karch_t0_cyl_b.ply";
    const std::string pCone = "karch_t0_cone.ply";
    const std::string pTorus = "karch_t0_torus.ply";
    if (!WritePlyAscii(cylA, p1) || !WritePlyAscii(cylB, p2) ||
        !WritePlyAscii(cone, pCone) || !WritePlyAscii(torus, pTorus)) {
        std::fprintf(stderr, "FAIL WritePlyAscii\n");
        return 1;
    }
    if (slurp(p1) != slurp(p2)) {
        std::fprintf(stderr, "FAIL ply bytes differ for same seed\n");
        std::remove(p1.c_str());
        std::remove(p2.c_str());
        return 1;
    }
    MeshData loaded;
    if (!ReadPlyAscii(p1, loaded) || !sameLoaded(loaded, cylA)) {
        std::fprintf(stderr, "FAIL ply roundtrip coords/faces\n");
        return 1;
    }
    if (!checkFile(pCone) || !checkFile(pTorus)) {
        std::fprintf(stderr, "FAIL ply files missing\n");
        return 1;
    }
    std::remove(p1.c_str());
    std::remove(p2.c_str());
    std::remove(pCone.c_str());
    std::remove(pTorus.c_str());

    std::printf("cyl verts=%zu faces=%zu cone=%zu torus=%zu fragFaces=%zu\n",
                cylA.vertices.size(), cylA.faces.size(), cone.vertices.size(),
                torus.vertices.size(), frag.faces.size());
    std::printf("rim n=%zu meanZ=%.1f plyBytesMatch=1\n", rim.rimPoints.size(), rim.meanZ);
    std::printf("PASS\n");
    return 0;
}
