/* DongArch3D / K-ArchProfile T5 right-half section + hatch (GPLv3+) */

#include "SectionComposer.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <locale>
#include <limits>

namespace DongArch {
namespace KArch {

static double dist2(const Point2& a, const Point2& b) {
    const double dx = a.x - b.x;
    const double dz = a.y - b.y;
    return dx * dx + dz * dz;
}

static double polyArea(const std::vector<Point2>& p) {
    double a = 0.0;
    const int n = static_cast<int>(p.size());
    for (int i = 0; i < n; ++i) {
        const Point2& u = p[static_cast<std::size_t>(i)];
        const Point2& v = p[static_cast<std::size_t>((i + 1) % n)];
        a += u.x * v.y - v.x * u.y;
    }
    return 0.5 * a;
}

static void ensureCcw(std::vector<Point2>& p) {
    if (polyArea(p) < 0.0) {
        std::reverse(p.begin(), p.end());
    }
}

static void simplify(std::vector<Point2>& p, double tol) {
    if (p.size() < 3) {
        return;
    }
    const double t2 = tol * tol;
    std::vector<Point2> o;
    o.push_back(p.front());
    for (std::size_t i = 1; i < p.size(); ++i) {
        if (dist2(o.back(), p[i]) > t2) {
            o.push_back(p[i]);
        }
    }
    if (o.size() >= 2 && dist2(o.front(), o.back()) <= t2) {
        o.pop_back();
    }
    p.swap(o);
}

static bool pointInPoly(const Point2& q, const std::vector<Point2>& p) {
    bool in = false;
    const int n = static_cast<int>(p.size());
    for (int i = 0, j = n - 1; i < n; j = i++) {
        const Point2& a = p[static_cast<std::size_t>(i)];
        const Point2& b = p[static_cast<std::size_t>(j)];
        const bool hit = ((a.y > q.y) != (b.y > q.y)) &&
                         (q.x < (b.x - a.x) * (q.y - a.y) / ((b.y - a.y) + 1e-30) + a.x);
        if (hit) {
            in = !in;
        }
    }
    return in;
}

static Point2 lerp2(const Point2& a, const Point2& b, double t) {
    return Point2{a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
}

static std::vector<std::vector<Point2>> clipXPositive(const std::vector<Point2>& chain) {
    std::vector<std::vector<Point2>> parts;
    std::vector<Point2> cur;
    auto flush = [&]() {
        if (cur.size() >= 2) {
            parts.push_back(cur);
        }
        cur.clear();
    };
    for (std::size_t i = 0; i + 1 < chain.size(); ++i) {
        Point2 a = chain[i];
        Point2 b = chain[i + 1];
        const bool aIn = a.x >= -1e-12;
        const bool bIn = b.x >= -1e-12;
        if (aIn && !cur.empty() && dist2(cur.back(), a) > 1e-20) {
            cur.push_back(a);
        } else if (aIn && cur.empty()) {
            cur.push_back(a);
        }
        if (aIn != bIn) {
            const double t = (0.0 - a.x) / ((b.x - a.x) + 1e-30);
            const Point2 x0 = lerp2(a, b, t);
            if (cur.empty() || dist2(cur.back(), x0) > 1e-20) {
                cur.push_back(x0);
            }
            if (!bIn) {
                flush();
            }
        }
        if (bIn && (cur.empty() || dist2(cur.back(), b) > 1e-20)) {
            cur.push_back(b);
        }
    }
    flush();
    return parts;
}

static std::vector<Point2> projectXz(const Polyline3& pl) {
    std::vector<Point2> s;
    s.reserve(pl.pts.size());
    for (const Point3& p : pl.pts) {
        s.push_back(Point2{p.x, p.z});
    }
    return s;
}

static void hatchPoly(const std::vector<Point2>& poly, const SectionParams& p,
                      std::vector<std::pair<Point2, Point2>>& out) {
    if (poly.size() < 3) {
        return;
    }
    const double ang = p.hatchAngleDeg * 3.14159265358979323846 / 180.0;
    const double nx = -std::sin(ang);
    const double nz = std::cos(ang);
    const double tx = std::cos(ang);
    const double tz = std::sin(ang);
    double cMin = 1e300;
    double cMax = -1e300;
    double tMin = 1e300;
    double tMax = -1e300;
    for (const Point2& q : poly) {
        const double c = q.x * nx + q.y * nz;
        const double t = q.x * tx + q.y * tz;
        cMin = std::min(cMin, c);
        cMax = std::max(cMax, c);
        tMin = std::min(tMin, t);
        tMax = std::max(tMax, t);
    }
    const double spanT = (tMax - tMin) + 4.0;
    for (double c = cMin - p.hatchSpacing; c <= cMax + p.hatchSpacing; c += p.hatchSpacing) {
        const Point2 origin{nx * c, nz * c};
        std::vector<double> hits;
        const int n = static_cast<int>(poly.size());
        for (int i = 0; i < n; ++i) {
            const Point2& a = poly[static_cast<std::size_t>(i)];
            const Point2& b = poly[static_cast<std::size_t>((i + 1) % n)];
            const double da = (a.x - origin.x) * nx + (a.y - origin.y) * nz;
            const double db = (b.x - origin.x) * nx + (b.y - origin.y) * nz;
            if (da * db > 0.0 && std::abs(da) > 1e-15 && std::abs(db) > 1e-15) {
                continue;
            }
            const double den = db - da;
            if (std::abs(den) < 1e-15) {
                continue;
            }
            const double u = -da / den;
            if (u < -1e-9 || u > 1.0 + 1e-9) {
                continue;
            }
            const Point2 h = lerp2(a, b, std::min(1.0, std::max(0.0, u)));
            hits.push_back((h.x - origin.x) * tx + (h.y - origin.y) * tz);
        }
        if (hits.size() < 2) {
            continue;
        }
        std::sort(hits.begin(), hits.end());
        hits.erase(std::unique(hits.begin(), hits.end(),
                               [](double a, double b) { return std::abs(a - b) < 1e-7; }),
                   hits.end());
        for (std::size_t i = 0; i + 1 < hits.size(); i += 2) {
            const double ta = hits[i];
            const double tb = hits[i + 1];
            if (tb - ta < p.minHatchLen) {
                continue;
            }
            Point2 a{origin.x + tx * ta, origin.y + tz * ta};
            Point2 b{origin.x + tx * tb, origin.y + tz * tb};
            const Point2 mid{(a.x + b.x) * 0.5, (a.y + b.y) * 0.5};
            if (!pointInPoly(mid, poly)) {
                continue;
            }
            if (std::abs(ta) > spanT && std::abs(tb) > spanT) {
                continue;
            }
            out.push_back({a, b});
        }
    }
}

SectionResult ComposeSection(const MeshData& alignedMesh, const SectionParams& p) {
    SectionResult r;
    const auto slices = Slice(alignedMesh, Plane{{0.0, 1.0, 0.0}, 0.0});
    std::vector<Point2> rightPts;
    for (const auto& pl : slices) {
        const auto xz = projectXz(pl);
        for (const auto& part : clipXPositive(xz)) {
            for (const Point2& q : part) {
                if (q.x > 0.0) {
                    rightPts.push_back(q);
                }
            }
        }
    }
    if (rightPts.size() < 4) {
        r.warning = "section not found";
        return r;
    }

    double zMin = rightPts[0].y;
    double zMax = zMin;
    for (const Point2& q : rightPts) {
        zMin = std::min(zMin, q.y);
        zMax = std::max(zMax, q.y);
    }
    const double zBin = 0.5;
    const int nBin = std::max(1, static_cast<int>(std::ceil((zMax - zMin) / zBin)));
    std::vector<double> minX(static_cast<std::size_t>(nBin), 1e300);
    std::vector<double> maxX(static_cast<std::size_t>(nBin), -1e300);
    std::vector<double> zc(static_cast<std::size_t>(nBin));
    for (int i = 0; i < nBin; ++i) {
        zc[static_cast<std::size_t>(i)] = zMin + (static_cast<double>(i) + 0.5) * zBin;
    }
    for (const Point2& q : rightPts) {
        int b = static_cast<int>((q.y - zMin) / zBin);
        if (b < 0) {
            b = 0;
        }
        if (b >= nBin) {
            b = nBin - 1;
        }
        minX[static_cast<std::size_t>(b)] = std::min(minX[static_cast<std::size_t>(b)], q.x);
        maxX[static_cast<std::size_t>(b)] = std::max(maxX[static_cast<std::size_t>(b)], q.x);
    }

    std::vector<Point2> outer;
    std::vector<Point2> inner;
    double thickMin = 1e300;
    double thickMax = 0.0;
    for (int i = 0; i < nBin; ++i) {
        if (maxX[static_cast<std::size_t>(i)] < minX[static_cast<std::size_t>(i)]) {
            continue;
        }
        double z = zc[static_cast<std::size_t>(i)];
        if (i == 0) {
            z = zMin;
        } else if (i == nBin - 1) {
            z = zMax;
        }
        outer.push_back(Point2{maxX[static_cast<std::size_t>(i)], z});
        inner.push_back(Point2{minX[static_cast<std::size_t>(i)], z});
        const double t = maxX[static_cast<std::size_t>(i)] - minX[static_cast<std::size_t>(i)];
        thickMin = std::min(thickMin, t);
        thickMax = std::max(thickMax, t);
    }
    if (outer.size() < 2) {
        r.warning = "section not found";
        return r;
    }

    std::vector<Point2> wall = outer;
    for (int i = static_cast<int>(inner.size()) - 1; i >= 0; --i) {
        wall.push_back(inner[static_cast<std::size_t>(i)]);
    }
    simplify(wall, p.simplifyTol);
    ensureCcw(wall);
    if (std::abs(polyArea(wall)) < 1e-6) {
        r.warning = "section not found";
        return r;
    }
    r.wallPolygons.push_back(std::move(wall));
    r.minWallThickness = thickMin;
    r.maxWallThickness = thickMax;
    r.interiorOutline = std::move(inner);
    hatchPoly(r.wallPolygons.front(), p, r.hatchLines);
    r.ok = true;
    return r;
}

bool WriteDrawingSvg(const std::string& path, const std::vector<Point2>& leftOutline,
                     const SectionResult& section) {
    std::ofstream out(path, std::ios::out | std::ios::binary);
    if (!out) {
        return false;
    }
    out.imbue(std::locale::classic());
    out << std::setprecision(6);
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"-80 -20 160 140\">\n";
    out << "<g transform=\"scale(1,-1) translate(0,-100)\">\n";
    if (!leftOutline.empty()) {
        out << "<polyline fill=\"none\" stroke=\"#111\" stroke-width=\"0.25\" points=\"";
        for (const Point2& q : leftOutline) {
            out << q.x << ',' << q.y << ' ';
        }
        out << "\"/>\n";
    }
    for (const auto& poly : section.wallPolygons) {
        out << "<polygon fill=\"#f4e6c3\" stroke=\"#111\" stroke-width=\"0.25\" points=\"";
        for (const Point2& q : poly) {
            out << q.x << ',' << q.y << ' ';
        }
        out << "\"/>\n";
    }
    for (const auto& h : section.hatchLines) {
        out << "<line x1=\"" << h.first.x << "\" y1=\"" << h.first.y << "\" x2=\"" << h.second.x
            << "\" y2=\"" << h.second.y << "\" stroke=\"#444\" stroke-width=\"0.15\"/>\n";
    }
    out << "</g></svg>\n";
    return static_cast<bool>(out);
}

} // namespace KArch
} // namespace DongArch
