/* DongArch3D / K-ArchProfile T4 left silhouette (GPLv3+) */

#include "Silhouette.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace DongArch {
namespace KArch {

static double percentile(std::vector<double> v, double p) {
    if (v.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    std::sort(v.begin(), v.end());
    const double idx = p * static_cast<double>(v.size() - 1);
    const std::size_t lo = static_cast<std::size_t>(idx);
    const std::size_t hi = std::min(lo + 1, v.size() - 1);
    const double t = idx - static_cast<double>(lo);
    return v[lo] * (1.0 - t) + v[hi] * t;
}

static void fillGaps(std::vector<double>& y) {
    const int n = static_cast<int>(y.size());
    int i = 0;
    while (i < n) {
        if (std::isfinite(y[static_cast<std::size_t>(i)])) {
            ++i;
            continue;
        }
        int j = i;
        while (j < n && !std::isfinite(y[static_cast<std::size_t>(j)])) {
            ++j;
        }
        if (i == 0 && j < n) {
            for (int k = 0; k < j; ++k) {
                y[static_cast<std::size_t>(k)] = y[static_cast<std::size_t>(j)];
            }
        } else if (j == n && i > 0) {
            for (int k = i; k < n; ++k) {
                y[static_cast<std::size_t>(k)] = y[static_cast<std::size_t>(i - 1)];
            }
        } else if (i > 0 && j < n) {
            const double a = y[static_cast<std::size_t>(i - 1)];
            const double b = y[static_cast<std::size_t>(j)];
            const double den = static_cast<double>(j - i + 1);
            for (int k = i; k < j; ++k) {
                const double t = static_cast<double>(k - i + 1) / den;
                y[static_cast<std::size_t>(k)] = a + (b - a) * t;
            }
        }
        i = j;
    }
}

// Savitzky-Golay window 7, polynomial order 2 (norm 21).
static std::vector<double> savitzkyGolay7(const std::vector<double>& y) {
    const int n = static_cast<int>(y.size());
    std::vector<double> o = y;
    if (n < 7) {
        return o;
    }
    static const double c[7] = {-2.0, 3.0, 6.0, 7.0, 6.0, 3.0, -2.0};
    for (int i = 3; i < n - 3; ++i) {
        double s = 0.0;
        for (int k = 0; k < 7; ++k) {
            s += c[k] * y[static_cast<std::size_t>(i - 3 + k)];
        }
        o[static_cast<std::size_t>(i)] = s / 21.0;
    }
    return o;
}

static void finishProfile(SilhouetteResult& r, const std::vector<double>& z,
                          std::vector<double> val, bool asRadius) {
    fillGaps(val);
    val = savitzkyGolay7(val);
    if (val.size() >= 61) {
        std::vector<double> ma = val;
        for (std::size_t i = 30; i + 30 < val.size(); ++i) {
            double s = 0.0;
            for (int k = -30; k <= 30; ++k) {
                s += val[i + static_cast<std::size_t>(k)];
            }
            ma[i] = s / 61.0;
        }
        val = std::move(ma);
    }
    r.leftOutline.reserve(z.size());
    r.maxBodyRadius = -1.0;
    for (std::size_t i = 0; i < z.size(); ++i) {
        if (!std::isfinite(val[i])) {
            continue;
        }
        const double x = asRadius ? -val[i] : val[i];
        r.leftOutline.push_back(Point2{x, z[i]});
        const double rad = asRadius ? val[i] : std::abs(val[i]);
        if (rad > r.maxBodyRadius) {
            r.maxBodyRadius = rad;
            r.maxBodyZ = z[i];
        }
    }
    r.ok = r.leftOutline.size() >= 2;
}

static SilhouetteResult extractMaxRadius(const MeshData& mesh, double zBin) {
    SilhouetteResult r;
    if (mesh.vertices.empty() || zBin <= 0.0) {
        return r;
    }
    std::vector<double> allZ;
    allZ.reserve(mesh.vertices.size());
    for (const Point3& p : mesh.vertices) {
        allZ.push_back(p.z);
    }
    double zMin = percentile(allZ, 0.02);
    double zMax = percentile(allZ, 0.98);
    if (!(zMax > zMin)) {
        zMin = allZ.front();
        zMax = allZ.front();
        for (double z : allZ) {
            zMin = std::min(zMin, z);
            zMax = std::max(zMax, z);
        }
    }
    const int nBin = std::max(1, static_cast<int>(std::ceil((zMax - zMin) / zBin)));
    std::vector<std::vector<double>> bins(static_cast<std::size_t>(nBin));
    for (const Point3& p : mesh.vertices) {
        if (p.z < zMin || p.z > zMax) {
            continue;
        }
        int b = static_cast<int>((p.z - zMin) / zBin);
        if (b < 0) {
            b = 0;
        }
        if (b >= nBin) {
            b = nBin - 1;
        }
        bins[static_cast<std::size_t>(b)].push_back(std::hypot(p.x, p.y));
    }
    std::vector<double> z(static_cast<std::size_t>(nBin));
    std::vector<double> p98(static_cast<std::size_t>(nBin));
    for (int i = 0; i < nBin; ++i) {
        z[static_cast<std::size_t>(i)] = zMin + (static_cast<double>(i) + 0.5) * zBin;
        const auto& bin = bins[static_cast<std::size_t>(i)];
        p98[static_cast<std::size_t>(i)] =
            (bin.size() < 8) ? std::numeric_limits<double>::quiet_NaN()
                             : percentile(bin, 0.98);
    }
    for (int i = 0; i < nBin; ++i) {
        if (!std::isfinite(p98[static_cast<std::size_t>(i)])) {
            continue;
        }
        for (int j = 0; j < nBin; ++j) {
            if (std::abs(z[static_cast<std::size_t>(j)] - z[static_cast<std::size_t>(i)]) > 10.0) {
                continue;
            }
            if (std::isfinite(p98[static_cast<std::size_t>(j)]) &&
                p98[static_cast<std::size_t>(j)] > p98[static_cast<std::size_t>(i)] + 3.0) {
                p98[static_cast<std::size_t>(i)] = std::numeric_limits<double>::quiet_NaN();
                break;
            }
        }
    }
    finishProfile(r, z, p98, true);
    return r;
}

static void addTriZHits(const Point3& a, const Point3& b, const Point3& c, double z,
                        std::vector<double>& xs) {
    const Point3 p[3] = {a, b, c};
    for (int i = 0; i < 3; ++i) {
        const Point3& u = p[i];
        const Point3& v = p[(i + 1) % 3];
        if (std::abs(u.z - z) <= 1e-12) {
            xs.push_back(u.x);
        }
        const double dz = v.z - u.z;
        if (std::abs(dz) < 1e-15) {
            continue;
        }
        const double t = (z - u.z) / dz;
        if (t > 0.0 && t <= 1.0) {
            xs.push_back(u.x + t * (v.x - u.x));
        }
    }
}

static SilhouetteResult extractTrueOrtho(const MeshData& mesh, double zBin) {
    SilhouetteResult r;
    if (mesh.vertices.empty() || mesh.faces.empty() || zBin <= 0.0) {
        return r;
    }
    double zMin = mesh.vertices[0].z;
    double zMax = zMin;
    for (const Point3& p : mesh.vertices) {
        zMin = std::min(zMin, p.z);
        zMax = std::max(zMax, p.z);
    }
    const int nBin = std::max(1, static_cast<int>(std::ceil((zMax - zMin) / zBin)));
    std::vector<double> z(static_cast<std::size_t>(nBin));
    std::vector<double> minX(static_cast<std::size_t>(nBin),
                             std::numeric_limits<double>::quiet_NaN());
    for (int i = 0; i < nBin; ++i) {
        const double zi = zMin + (static_cast<double>(i) + 0.5) * zBin;
        z[static_cast<std::size_t>(i)] = zi;
        std::vector<double> xs;
        for (const auto& f : mesh.faces) {
            if (f[0] < 0 || f[1] < 0 || f[2] < 0) {
                continue;
            }
            addTriZHits(mesh.vertices[static_cast<std::size_t>(f[0])],
                        mesh.vertices[static_cast<std::size_t>(f[1])],
                        mesh.vertices[static_cast<std::size_t>(f[2])], zi, xs);
        }
        if (!xs.empty()) {
            minX[static_cast<std::size_t>(i)] = *std::min_element(xs.begin(), xs.end());
        }
    }
    finishProfile(r, z, minX, false);
    return r;
}

SilhouetteResult ExtractSilhouette(const MeshData& alignedMesh, SilhouetteMode mode,
                                   double zBin) {
    SilhouetteResult a = (mode == SilhouetteMode::TrueOrtho)
                             ? extractTrueOrtho(alignedMesh, zBin)
                             : extractMaxRadius(alignedMesh, zBin);
    if (!a.ok) {
        return a;
    }
    const SilhouetteResult b = (mode == SilhouetteMode::TrueOrtho)
                                   ? extractMaxRadius(alignedMesh, zBin)
                                   : extractTrueOrtho(alignedMesh, zBin);
    if (b.ok && a.maxBodyRadius > 1e-12) {
        const double rel = std::abs(a.maxBodyRadius - b.maxBodyRadius) / a.maxBodyRadius;
        if (rel >= 0.03) {
            a.warning = "ASYMMETRIC_GEOMETRY";
        }
    }
    return a;
}

} // namespace KArch
} // namespace DongArch
