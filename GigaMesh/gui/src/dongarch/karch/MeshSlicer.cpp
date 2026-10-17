/* DongArch3D / K-ArchProfile MeshData plane slicer (plan 5.5) (GPLv3+) */

#include "MeshSlicer.h"

#include <cmath>
#include <utility>

namespace DongArch {
namespace KArch {
namespace {

struct Seg {
    Point3 a;
    Point3 b;
    bool used = false;
};

double dist2(const Point3& a, const Point3& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
}

double bboxDiag(const MeshData& mesh) {
    if (mesh.vertices.empty()) {
        return 1.0;
    }
    Point3 mn = mesh.vertices[0];
    Point3 mx = mesh.vertices[0];
    for (const Point3& p : mesh.vertices) {
        mn.x = (p.x < mn.x) ? p.x : mn.x;
        mn.y = (p.y < mn.y) ? p.y : mn.y;
        mn.z = (p.z < mn.z) ? p.z : mn.z;
        mx.x = (p.x > mx.x) ? p.x : mx.x;
        mx.y = (p.y > mx.y) ? p.y : mx.y;
        mx.z = (p.z > mx.z) ? p.z : mx.z;
    }
    return std::sqrt(dist2(mn, mx));
}

Point3 lerp(const Point3& a, const Point3& b, double t) {
    return Point3{a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z)};
}

void addHit(std::vector<Point3>& hits, const Point3& p, double eps2) {
    for (const Point3& h : hits) {
        if (dist2(h, p) <= eps2) {
            return;
        }
    }
    hits.push_back(p);
}

void edgeHits(const Point3& a, const Point3& b, double sa, double sb,
              std::vector<Point3>& hits, double onEps, double eps2) {
    if (std::fabs(sa - sb) < 1e-12) {
        return;
    }
    const bool aOn = std::fabs(sa) <= onEps;
    const bool bOn = std::fabs(sb) <= onEps;
    if (aOn && bOn) {
        return;
    }
    if (aOn) {
        addHit(hits, a, eps2);
        return;
    }
    if (bOn) {
        addHit(hits, b, eps2);
        return;
    }
    if (sa * sb < 0.0) {
        const double t = -sa / (sb - sa);
        addHit(hits, lerp(a, b, t), eps2);
    }
}

int findJoin(const std::vector<Seg>& segs, const Point3& p, double eps2, bool* atB) {
    for (int i = 0; i < static_cast<int>(segs.size()); ++i) {
        if (segs[static_cast<std::size_t>(i)].used) {
            continue;
        }
        if (dist2(segs[static_cast<std::size_t>(i)].a, p) <= eps2) {
            *atB = false;
            return i;
        }
        if (dist2(segs[static_cast<std::size_t>(i)].b, p) <= eps2) {
            *atB = true;
            return i;
        }
    }
    return -1;
}

} // namespace

double SignedDistance(const Plane& pl, const Point3& p) {
    return pl.n.x * p.x + pl.n.y * p.y + pl.n.z * p.z + pl.d;
}

std::vector<Polyline3> Slice(const MeshData& mesh, const Plane& pl) {
    const double diag = bboxDiag(mesh);
    const double weld = 1e-6 * (diag > 0.0 ? diag : 1.0);
    const double weld2 = weld * weld;
    const double onEps = 1e-12 * (diag > 0.0 ? diag : 1.0);

    std::vector<Seg> segs;
    for (const auto& f : mesh.faces) {
        if (f[0] < 0 || f[1] < 0 || f[2] < 0) {
            continue;
        }
        const int nv = static_cast<int>(mesh.vertices.size());
        if (f[0] >= nv || f[1] >= nv || f[2] >= nv) {
            continue;
        }
        const Point3& v0 = mesh.vertices[static_cast<std::size_t>(f[0])];
        const Point3& v1 = mesh.vertices[static_cast<std::size_t>(f[1])];
        const Point3& v2 = mesh.vertices[static_cast<std::size_t>(f[2])];
        const double s0 = SignedDistance(pl, v0);
        const double s1 = SignedDistance(pl, v1);
        const double s2 = SignedDistance(pl, v2);
        std::vector<Point3> hits;
        edgeHits(v0, v1, s0, s1, hits, onEps, weld2);
        edgeHits(v1, v2, s1, s2, hits, onEps, weld2);
        edgeHits(v2, v0, s2, s0, hits, onEps, weld2);
        if (hits.size() == 2 && dist2(hits[0], hits[1]) > weld2) {
            segs.push_back(Seg{hits[0], hits[1], false});
        }
    }

    std::vector<Polyline3> out;
    for (std::size_t i = 0; i < segs.size(); ++i) {
        if (segs[i].used) {
            continue;
        }
        segs[i].used = true;
        Polyline3 line;
        line.pts.push_back(segs[i].a);
        line.pts.push_back(segs[i].b);
        bool grew = true;
        while (grew) {
            grew = false;
            bool atB = false;
            int j = findJoin(segs, line.pts.back(), weld2, &atB);
            if (j >= 0) {
                segs[static_cast<std::size_t>(j)].used = true;
                line.pts.push_back(atB ? segs[static_cast<std::size_t>(j)].a
                                       : segs[static_cast<std::size_t>(j)].b);
                grew = true;
                continue;
            }
            j = findJoin(segs, line.pts.front(), weld2, &atB);
            if (j >= 0) {
                segs[static_cast<std::size_t>(j)].used = true;
                line.pts.insert(line.pts.begin(),
                                atB ? segs[static_cast<std::size_t>(j)].a
                                    : segs[static_cast<std::size_t>(j)].b);
                grew = true;
            }
        }
        if (line.pts.size() >= 2) {
            out.push_back(std::move(line));
        }
    }
    return out;
}

std::vector<std::vector<Polyline3>> SliceHorizontalBatch(const MeshData& mesh,
                                                         const std::vector<double>& zHeights) {
    std::vector<std::vector<Polyline3>> all;
    all.reserve(zHeights.size());
    for (double z : zHeights) {
        all.push_back(Slice(mesh, Plane{{0.0, 0.0, 1.0}, -z}));
    }
    return all;
}

} // namespace KArch
} // namespace DongArch
