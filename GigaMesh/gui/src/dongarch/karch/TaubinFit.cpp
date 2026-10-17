/* DongArch3D / K-ArchProfile TaubinSVD circle fit (GPLv3+) */

#include "TaubinFit.h"

#include <algorithm>
#include <cmath>

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

} // namespace

Circle2D FitCircleTaubin(const std::vector<Point2>& points) {
    Circle2D out;
    out.pointCount = static_cast<int>(points.size());
    if (points.size() < 3) {
        return out;
    }

    double xBar = 0.0;
    double yBar = 0.0;
    for (const Point2& p : points) {
        xBar += p.x;
        yBar += p.y;
    }
    const double n = static_cast<double>(points.size());
    xBar /= n;
    yBar /= n;

    std::vector<double> u(points.size());
    std::vector<double> v(points.size());
    std::vector<double> z(points.size());
    double zBar = 0.0;
    for (std::size_t i = 0; i < points.size(); ++i) {
        u[i] = points[i].x - xBar;
        v[i] = points[i].y - yBar;
        z[i] = u[i] * u[i] + v[i] * v[i];
        zBar += z[i];
    }
    zBar /= n;
    if (zBar < 1e-12) {
        return out;
    }

    const double invDen = 1.0 / (2.0 * std::sqrt(zBar));
    double G[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    for (std::size_t i = 0; i < points.size(); ++i) {
        const double z0 = (z[i] - zBar) * invDen;
        const double row[3] = {z0, u[i], v[i]};
        for (int a = 0; a < 3; ++a) {
            for (int b = 0; b < 3; ++b) {
                G[a][b] += row[a] * row[b];
            }
        }
    }

    double eval[3];
    double V[3][3];
    jacobiEigen3(G, eval, V);

    int imin = 0;
    if (eval[1] < eval[imin]) {
        imin = 1;
    }
    if (eval[2] < eval[imin]) {
        imin = 2;
    }

    const double A0 = V[0][imin];
    const double A1 = V[1][imin];
    const double A2 = V[2][imin];

    const double A = A0 * invDen;
    const double B = A1;
    const double C = A2;
    const double D = -zBar * A;
    if (std::fabs(A) < 1e-12) {
        return out;
    }

    const double uc = -B / (2.0 * A);
    const double vc = -C / (2.0 * A);
    const double disc = B * B + C * C - 4.0 * A * D;
    if (disc < 0.0) {
        return out;
    }

    out.xc = uc + xBar;
    out.yc = vc + yBar;
    out.r = std::sqrt(disc) / (2.0 * std::fabs(A));

    double sse = 0.0;
    for (const Point2& p : points) {
        const double dx = p.x - out.xc;
        const double dy = p.y - out.yc;
        const double ri = std::sqrt(dx * dx + dy * dy) - out.r;
        sse += ri * ri;
    }
    out.rmse = std::sqrt(sse / n);
    out.ok = std::isfinite(out.r) && out.r > 0.0;
    return out;
}

} // namespace KArch
} // namespace DongArch
