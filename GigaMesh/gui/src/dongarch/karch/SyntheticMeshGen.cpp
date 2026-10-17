/* DongArch3D / K-ArchProfile T0 synthetic verification meshes (GPLv3+) */

#include "SyntheticMeshGen.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <locale>
#include <numbers>
#include <random>
#include <sstream>

namespace DongArch {
namespace KArch {
namespace Testing {

MeshData SyntheticMesh::toMesh() const {
    return MeshData{vertices, faces};
}

static void rotateXPoint(Point3& p, double c, double s) {
    const double y = c * p.y - s * p.z;
    const double z = s * p.y + c * p.z;
    p.y = y;
    p.z = z;
}

static double norm3(const Point3& p) {
    return std::hypot(p.x, std::hypot(p.y, p.z));
}

static Point3 sub3(const Point3& a, const Point3& b) {
    return Point3{a.x - b.x, a.y - b.y, a.z - b.z};
}

static double dot3(const Point3& a, const Point3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Point3 cross3(const Point3& a, const Point3& b) {
    return Point3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static Point3 unit3(const Point3& p) {
    const double n = norm3(p);
    if (n < 1e-15) {
        return Point3{0.0, 0.0, 1.0};
    }
    return Point3{p.x / n, p.y / n, p.z / n};
}

// Angle in [0, 2π) around gt axis; +X is 0 when axis is +Z (matches atan2(y,x)).
static double axisAzimuth(const Point3& p, const Point3& origin, const Point3& dir) {
    const Point3 n = unit3(dir);
    Point3 ref{1.0, 0.0, 0.0};
    if (std::abs(dot3(ref, n)) > 0.9) {
        ref = Point3{0.0, 1.0, 0.0};
    }
    const Point3 u = unit3(sub3(ref, Point3{n.x * dot3(ref, n), n.y * dot3(ref, n), n.z * dot3(ref, n)}));
    const Point3 v = cross3(n, u);
    const Point3 r = sub3(p, origin);
    double a = std::atan2(dot3(r, v), dot3(r, u));
    if (a < 0.0) {
        a += 2.0 * std::numbers::pi;
    }
    return a;
}

static void applyParams(SyntheticMesh& m, const SyntheticParams& p) {
    if (p.noiseSigma > 0.0) {
        std::mt19937 rng(p.seed);
        std::normal_distribution<double> nd(0.0, p.noiseSigma);
        for (Point3& v : m.vertices) {
            v.x += nd(rng);
            v.y += nd(rng);
            v.z += nd(rng);
        }
    }

    const double th = p.tiltDeg * std::numbers::pi / 180.0;
    const double c = std::cos(th);
    const double s = std::sin(th);
    if (p.tiltDeg != 0.0) {
        for (Point3& v : m.vertices) {
            rotateXPoint(v, c, s);
        }
        rotateXPoint(m.gtAxisOrigin, c, s);
        rotateXPoint(m.gtAxisDir, c, s);
    }

    if (p.translation.x != 0.0 || p.translation.y != 0.0 || p.translation.z != 0.0) {
        for (Point3& v : m.vertices) {
            v.x += p.translation.x;
            v.y += p.translation.y;
            v.z += p.translation.z;
        }
        m.gtAxisOrigin.x += p.translation.x;
        m.gtAxisOrigin.y += p.translation.y;
        m.gtAxisOrigin.z += p.translation.z;
    }
}

static SyntheticMesh makeRevolvedShell(int radial, int heightSeg, double height,
                                       double thickness, bool capTop, bool capBottom,
                                       const std::function<double(double)>& outerR) {
    SyntheticMesh m;
    const int rings = heightSeg + 1;
    m.vertices.reserve(static_cast<std::size_t>(2 * rings * radial));

    auto addRing = [&](double r, double z) {
        for (int j = 0; j < radial; ++j) {
            const double t = 2.0 * std::numbers::pi * static_cast<double>(j) /
                             static_cast<double>(radial);
            m.vertices.push_back(Point3{r * std::cos(t), r * std::sin(t), z});
        }
    };

    for (int i = 0; i < rings; ++i) {
        const double z = height * static_cast<double>(i) / static_cast<double>(heightSeg);
        addRing(outerR(z), z);
    }
    const int innerBase = rings * radial;
    for (int i = 0; i < rings; ++i) {
        const double z = height * static_cast<double>(i) / static_cast<double>(heightSeg);
        const double ri = std::max(outerR(z) - thickness, 1e-6);
        addRing(ri, z);
    }

    auto quad = [&](int a, int b, int c, int d) {
        m.faces.push_back({a, b, c});
        m.faces.push_back({a, c, d});
    };

    for (int i = 0; i < heightSeg; ++i) {
        for (int j = 0; j < radial; ++j) {
            const int jn = (j + 1) % radial;
            const int o0 = i * radial + j;
            const int o1 = i * radial + jn;
            const int o2 = (i + 1) * radial + jn;
            const int o3 = (i + 1) * radial + j;
            quad(o0, o1, o2, o3);
            const int i0 = innerBase + i * radial + j;
            const int i1 = innerBase + i * radial + jn;
            const int i2 = innerBase + (i + 1) * radial + jn;
            const int i3 = innerBase + (i + 1) * radial + j;
            quad(i0, i3, i2, i1);
        }
    }

    if (capBottom) {
        for (int j = 0; j < radial; ++j) {
            const int jn = (j + 1) % radial;
            quad(j, innerBase + j, innerBase + jn, jn);
        }
    }
    if (capTop) {
        const int oTop = heightSeg * radial;
        const int iTop = innerBase + heightSeg * radial;
        for (int j = 0; j < radial; ++j) {
            const int jn = (j + 1) % radial;
            m.faces.push_back({oTop + j, oTop + jn, iTop + jn});
            m.faces.push_back({oTop + j, iTop + jn, iTop + j});
        }
    }
    return m;
}

SyntheticMesh MakeCylinderShell(double rOuter, double height, double thickness,
                                const SyntheticParams& p) {
    SyntheticMesh m = makeRevolvedShell(
        p.radialSeg, p.heightSeg, height, thickness, p.capTop, p.capBottom,
        [rOuter](double) { return rOuter; });
    m.gtRimRadius = rOuter;
    m.gtBaseRadius = rOuter;
    m.gtBellyRadius = rOuter;
    m.gtHeight = height;
    m.gtWallThick = thickness;
    m.gtAxisOrigin = Point3{0.0, 0.0, 0.0};
    m.gtAxisDir = Point3{0.0, 0.0, 1.0};
    applyParams(m, p);
    return m;
}

SyntheticMesh MakeConeShell(double rBottom, double rTop, double height,
                            double thickness, const SyntheticParams& p) {
    SyntheticMesh m = makeRevolvedShell(
        p.radialSeg, p.heightSeg, height, thickness, p.capTop, p.capBottom,
        [rBottom, rTop, height](double z) {
            return rBottom + (rTop - rBottom) * (z / height);
        });
    m.gtRimRadius = rTop;
    m.gtBaseRadius = rBottom;
    m.gtBellyRadius = std::max(rBottom, rTop);
    m.gtHeight = height;
    m.gtWallThick = thickness;
    m.gtAxisOrigin = Point3{0.0, 0.0, 0.0};
    m.gtAxisDir = Point3{0.0, 0.0, 1.0};
    applyParams(m, p);
    return m;
}

SyntheticMesh MakeTorusProfileVessel(double bellyRadius, double rimRadius,
                                     double height, double thickness,
                                     const SyntheticParams& p) {
    const double bulge = bellyRadius - rimRadius;
    SyntheticMesh m = makeRevolvedShell(
        p.radialSeg, p.heightSeg, height, thickness, p.capTop, p.capBottom,
        [rimRadius, bulge, height](double z) {
            const double u = z / height;
            const double ang = std::numbers::pi * (u - 0.5);
            return rimRadius + bulge * std::cos(ang);
        });
    m.gtRimRadius = rimRadius;
    m.gtBaseRadius = rimRadius;
    m.gtBellyRadius = bellyRadius;
    m.gtHeight = height;
    m.gtWallThick = thickness;
    m.gtAxisOrigin = Point3{0.0, 0.0, 0.0};
    m.gtAxisDir = Point3{0.0, 0.0, 1.0};
    applyParams(m, p);
    return m;
}

SyntheticMesh MakeArcFragment(const SyntheticMesh& full, double arcDeg) {
    SyntheticMesh out = full;
    out.vertices.clear();
    out.faces.clear();
    const double span = arcDeg * std::numbers::pi / 180.0;
    auto inArc = [&](const Point3& p) {
        return axisAzimuth(p, full.gtAxisOrigin, full.gtAxisDir) <= span + 1e-12;
    };

    std::vector<int> remap(full.vertices.size(), -1);
    auto take = [&](int i) {
        if (remap[static_cast<std::size_t>(i)] < 0) {
            remap[static_cast<std::size_t>(i)] = static_cast<int>(out.vertices.size());
            out.vertices.push_back(full.vertices[static_cast<std::size_t>(i)]);
        }
        return remap[static_cast<std::size_t>(i)];
    };

    for (const auto& f : full.faces) {
        const Point3& a = full.vertices[static_cast<std::size_t>(f[0])];
        const Point3& b = full.vertices[static_cast<std::size_t>(f[1])];
        const Point3& c = full.vertices[static_cast<std::size_t>(f[2])];
        if (inArc(a) && inArc(b) && inArc(c)) {
            out.faces.push_back({take(f[0]), take(f[1]), take(f[2])});
        }
    }
    return out;
}

bool WritePlyAscii(const SyntheticMesh& m, const std::string& path) {
    std::ofstream out(path, std::ios::out | std::ios::binary);
    if (!out) {
        return false;
    }
    out.imbue(std::locale::classic());
    out << "ply\n"
        << "format ascii 1.0\n"
        << "comment karch synthetic\n"
        << "element vertex " << m.vertices.size() << "\n"
        << "property double x\n"
        << "property double y\n"
        << "property double z\n"
        << "element face " << m.faces.size() << "\n"
        << "property list uchar int vertex_indices\n"
        << "end_header\n";
    out << std::setprecision(17);
    for (const Point3& v : m.vertices) {
        out << v.x << ' ' << v.y << ' ' << v.z << '\n';
    }
    for (const auto& f : m.faces) {
        out << "3 " << f[0] << ' ' << f[1] << ' ' << f[2] << '\n';
    }
    return static_cast<bool>(out);
}

bool ReadPlyAscii(const std::string& path, MeshData& dest) {
    dest.vertices.clear();
    dest.faces.clear();
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in) {
        return false;
    }
    in.imbue(std::locale::classic());
    std::string line;
    int nVert = -1;
    int nFace = -1;
    bool headerDone = false;
    bool ascii = false;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::istringstream ls(line);
        ls.imbue(std::locale::classic());
        std::string tok;
        ls >> tok;
        if (tok == "format") {
            std::string kind;
            ls >> kind;
            ascii = (kind == "ascii");
        } else if (tok == "element") {
            std::string kind;
            int n = 0;
            ls >> kind >> n;
            if (kind == "vertex") {
                nVert = n;
            } else if (kind == "face") {
                nFace = n;
            }
        } else if (tok == "end_header") {
            headerDone = true;
            break;
        }
    }
    if (!headerDone || !ascii || nVert < 0 || nFace < 0) {
        return false;
    }
    dest.vertices.reserve(static_cast<std::size_t>(nVert));
    dest.faces.reserve(static_cast<std::size_t>(nFace));
    for (int i = 0; i < nVert; ++i) {
        Point3 p{};
        if (!(in >> p.x >> p.y >> p.z)) {
            return false;
        }
        dest.vertices.push_back(p);
    }
    for (int i = 0; i < nFace; ++i) {
        int count = 0;
        std::array<int, 3> f{};
        if (!(in >> count >> f[0] >> f[1] >> f[2]) || count != 3) {
            return false;
        }
        if (f[0] < 0 || f[0] >= nVert || f[1] < 0 || f[1] >= nVert || f[2] < 0 ||
            f[2] >= nVert) {
            return false;
        }
        dest.faces.push_back(f);
    }
    return true;
}

} // namespace Testing
} // namespace KArch
} // namespace DongArch
