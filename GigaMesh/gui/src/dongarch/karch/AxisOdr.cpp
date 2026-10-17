/* DongArch3D / K-ArchProfile rotation-axis ODR (plan 5.4) (GPLv3+) */

#include "AxisOdr.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace DongArch {
namespace KArch {
namespace {

void jacobiEigen3(double A[3][3], double eval[3], double V[3][3]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            V[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }
    for (int iter = 0; iter < 64; ++iter) {
        int p = 0;
        int q = 1;
        double maxOff = std::fabs(A[0][1]);
        if (std::fabs(A[0][2]) > maxOff) {
            maxOff = std::fabs(A[0][2]);
            p = 0;
            q = 2;
        }
        if (std::fabs(A[1][2]) > maxOff) {
            maxOff = std::fabs(A[1][2]);
            p = 1;
            q = 2;
        }
        if (maxOff < 1e-18) {
            break;
        }
        const double app = A[p][p];
        const double aqq = A[q][q];
        const double apq = A[p][q];
        const double tau = (aqq - app) / (2.0 * apq);
        const double t = (tau >= 0.0)
            ? 1.0 / (tau + std::sqrt(1.0 + tau * tau))
            : -1.0 / (-tau + std::sqrt(1.0 + tau * tau));
        const double c = 1.0 / std::sqrt(1.0 + t * t);
        const double s = t * c;
        A[p][p] = app - t * apq;
        A[q][q] = aqq + t * apq;
        A[p][q] = A[q][p] = 0.0;
        for (int r = 0; r < 3; ++r) {
            if (r == p || r == q) {
                continue;
            }
            const double arp = A[r][p];
            const double arq = A[r][q];
            A[r][p] = A[p][r] = c * arp - s * arq;
            A[r][q] = A[q][r] = s * arp + c * arq;
        }
        for (int r = 0; r < 3; ++r) {
            const double vip = V[r][p];
            const double viq = V[r][q];
            V[r][p] = c * vip - s * viq;
            V[r][q] = s * vip + c * viq;
        }
    }
    eval[0] = A[0][0];
    eval[1] = A[1][1];
    eval[2] = A[2][2];
}

double medianOf(std::vector<double> v) {
    if (v.empty()) {
        return 0.0;
    }
    const std::size_t n = v.size();
    std::nth_element(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(n / 2), v.end());
    return v[n / 2];
}

double madOf(const std::vector<double>& v, double med) {
    std::vector<double> d;
    d.reserve(v.size());
    for (double x : v) {
        d.push_back(std::fabs(x - med));
    }
    return medianOf(std::move(d));
}

} // namespace

std::vector<SliceCircle> FitHorizontalOuters(const MeshData& mesh,
                                             const std::vector<double>& zHeights) {
    std::vector<SliceCircle> out;
    const auto batches = SliceHorizontalBatch(mesh, zHeights);
    for (std::size_t i = 0; i < batches.size(); ++i) {
        std::vector<Polyline2> loops;
        loops.reserve(batches[i].size());
        for (const Polyline3& L : batches[i]) {
            Polyline2 p2;
            p2.pts.reserve(L.pts.size());
            for (const Point3& p : L.pts) {
                p2.pts.push_back(Point2{p.x, p.y});
            }
            loops.push_back(std::move(p2));
        }
        const AnnulusResult fit = ClassifyAndFitOuter(loops);
        if (!fit.ok) {
            continue;
        }
        out.push_back(SliceCircle{fit.outerCircle, zHeights[i]});
    }
    return out;
}

Axis3D OptimizeRotationAxis(const std::vector<SliceCircle>& slices) {
    Axis3D axis;
    std::vector<double> rmses;
    std::vector<double> radii;
    rmses.reserve(slices.size());
    radii.reserve(slices.size());
    for (const SliceCircle& s : slices) {
        if (!s.circle.ok) {
            continue;
        }
        rmses.push_back(s.circle.rmse);
        radii.push_back(s.circle.r);
    }
    const double medR = medianOf(rmses);
    const double medRad = medianOf(radii);
    const double madR = madOf(rmses, medR);
    const double madRad = madOf(radii, medRad);

    std::vector<SliceCircle> keep;
    for (const SliceCircle& s : slices) {
        if (!s.circle.ok) {
            continue;
        }
        const bool rOut = madR > 0.0 && std::fabs(s.circle.rmse - medR) > 3.5 * madR;
        const bool radOut = madRad > 0.0 && std::fabs(s.circle.r - medRad) > 3.5 * madRad;
        if (rOut || radOut) {
            continue;
        }
        keep.push_back(s);
    }

    if (static_cast<int>(keep.size()) < 5) {
        axis.usedPcaFallback = true;
        axis.usedSlices = static_cast<int>(keep.size());
        return axis;
    }

    double wsum = 0.0;
    Point3 cBar{};
    std::vector<double> w;
    w.reserve(keep.size());
    for (const SliceCircle& s : keep) {
        const double wi = static_cast<double>(std::max(s.circle.pointCount, 1)) /
                          std::max(s.circle.rmse, 1e-12);
        w.push_back(wi);
        wsum += wi;
        cBar.x += wi * s.circle.xc;
        cBar.y += wi * s.circle.yc;
        cBar.z += wi * s.z;
    }
    cBar.x /= wsum;
    cBar.y /= wsum;
    cBar.z /= wsum;

    double C[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    for (std::size_t i = 0; i < keep.size(); ++i) {
        const double d[3] = {
            keep[i].circle.xc - cBar.x,
            keep[i].circle.yc - cBar.y,
            keep[i].z - cBar.z,
        };
        for (int a = 0; a < 3; ++a) {
            for (int b = 0; b < 3; ++b) {
                C[a][b] += w[i] * d[a] * d[b];
            }
        }
    }

    double eval[3];
    double V[3][3];
    jacobiEigen3(C, eval, V);
    int imax = 0;
    if (eval[1] > eval[imax]) {
        imax = 1;
    }
    if (eval[2] > eval[imax]) {
        imax = 2;
    }
    Point3 dir{V[0][imax], V[1][imax], V[2][imax]};
    if (dir.z < 0.0) {
        dir.x = -dir.x;
        dir.y = -dir.y;
        dir.z = -dir.z;
    }
    const double nrm = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (nrm < 1e-15) {
        axis.usedPcaFallback = true;
        return axis;
    }
    dir.x /= nrm;
    dir.y /= nrm;
    dir.z /= nrm;

    double sse = 0.0;
    for (const SliceCircle& s : keep) {
        const double px = s.circle.xc - cBar.x;
        const double py = s.circle.yc - cBar.y;
        const double pz = s.z - cBar.z;
        const double t = px * dir.x + py * dir.y + pz * dir.z;
        const double dx = px - t * dir.x;
        const double dy = py - t * dir.y;
        const double dz = pz - t * dir.z;
        sse += std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    axis.origin = cBar;
    axis.direction = dir;
    axis.rmse = sse / static_cast<double>(keep.size());
    axis.usedSlices = static_cast<int>(keep.size());
    axis.ok = true;
    return axis;
}

} // namespace KArch
} // namespace DongArch
