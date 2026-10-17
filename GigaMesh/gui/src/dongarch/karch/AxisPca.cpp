/* DongArch3D / K-ArchProfile rim PCA alignment (plan 5.1) (GPLv3+) */

#include "AxisPca.h"

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

void setColMajor(Transform4& T, int r, int c, double v) {
    T.m[c * 4 + r] = v;
}

} // namespace

Point3 Transform4::apply(const Point3& p) const {
    return Point3{
        m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12],
        m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13],
        m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14],
    };
}

Point3 MeshCentroid(const MeshData& mesh) {
    Point3 g{};
    if (mesh.vertices.empty()) {
        return g;
    }
    for (const Point3& p : mesh.vertices) {
        g.x += p.x;
        g.y += p.y;
        g.z += p.z;
    }
    const double n = static_cast<double>(mesh.vertices.size());
    g.x /= n;
    g.y /= n;
    g.z /= n;
    return g;
}

PcaAlignment ComputePcaAlignment(const std::vector<Point3>& rimPoints,
                                 const Point3& meshCentroid) {
    PcaAlignment out;
    if (rimPoints.size() < 20) {
        return out;
    }

    Point3 pBar{};
    for (const Point3& p : rimPoints) {
        pBar.x += p.x;
        pBar.y += p.y;
        pBar.z += p.z;
    }
    const double n = static_cast<double>(rimPoints.size());
    pBar.x /= n;
    pBar.y /= n;
    pBar.z /= n;
    out.rimCentroid = pBar;

    double C[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    for (const Point3& p : rimPoints) {
        const double d[3] = {p.x - pBar.x, p.y - pBar.y, p.z - pBar.z};
        for (int a = 0; a < 3; ++a) {
            for (int b = 0; b < 3; ++b) {
                C[a][b] += d[a] * d[b];
            }
        }
    }
    const double invN = 1.0 / n;
    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            C[a][b] *= invN;
        }
    }

    double eval[3];
    double V[3][3];
    jacobiEigen3(C, eval, V);
    int imin = 0;
    if (eval[1] < eval[imin]) {
        imin = 1;
    }
    if (eval[2] < eval[imin]) {
        imin = 2;
    }
    Point3 nRim{V[0][imin], V[1][imin], V[2][imin]};
    const double gx = pBar.x - meshCentroid.x;
    const double gy = pBar.y - meshCentroid.y;
    const double gz = pBar.z - meshCentroid.z;
    if (nRim.x * gx + nRim.y * gy + nRim.z * gz < 0.0) {
        nRim.x = -nRim.x;
        nRim.y = -nRim.y;
        nRim.z = -nRim.z;
    }
    out.rimNormal = nRim;

    const double sumL = eval[0] + eval[1] + eval[2];
    out.planarity = (sumL > 0.0) ? std::max(0.0, eval[imin] / sumL) : 0.0;
    out.planarityWarn = out.planarity > 0.02;

    const double cDot = nRim.z;
    double R[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    if (cDot > 1.0 - 1e-9) {
        // already +Z
    } else if (cDot < -1.0 + 1e-9) {
        double ax = 1.0;
        double ay = 0.0;
        double az = 0.0;
        if (std::fabs(nRim.x) > 0.9) {
            ax = 0.0;
            ay = 1.0;
        }
        const double cx = nRim.y * az - nRim.z * ay;
        const double cy = nRim.z * ax - nRim.x * az;
        const double cz = nRim.x * ay - nRim.y * ax;
        const double clen = std::sqrt(cx * cx + cy * cy + cz * cz);
        const double ux = cx / clen;
        const double uy = cy / clen;
        const double uz = cz / clen;
        R[0][0] = 2.0 * ux * ux - 1.0;
        R[0][1] = 2.0 * ux * uy;
        R[0][2] = 2.0 * ux * uz;
        R[1][0] = 2.0 * uy * ux;
        R[1][1] = 2.0 * uy * uy - 1.0;
        R[1][2] = 2.0 * uy * uz;
        R[2][0] = 2.0 * uz * ux;
        R[2][1] = 2.0 * uz * uy;
        R[2][2] = 2.0 * uz * uz - 1.0;
    } else {
        const double wx = nRim.y;
        const double wy = -nRim.x;
        const double wz = 0.0;
        const double wlen = std::sqrt(wx * wx + wy * wy + wz * wz);
        const double ux = wx / wlen;
        const double uy = wy / wlen;
        const double uz = wz / wlen;
        const double theta = std::atan2(wlen, cDot);
        const double s = std::sin(theta);
        const double oc = 1.0 - std::cos(theta);
        const double K[3][3] = {{0, -uz, uy}, {uz, 0, -ux}, {-uy, ux, 0}};
        double K2[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    K2[i][j] += K[i][k] * K[k][j];
                }
            }
        }
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                R[i][j] = (i == j ? 1.0 : 0.0) + s * K[i][j] + oc * K2[i][j];
            }
        }
    }

    const double tx = -(R[0][0] * pBar.x + R[0][1] * pBar.y + R[0][2] * pBar.z);
    const double ty = -(R[1][0] * pBar.x + R[1][1] * pBar.y + R[1][2] * pBar.z);
    const double tz = -(R[2][0] * pBar.x + R[2][1] * pBar.y + R[2][2] * pBar.z);

    Transform4 T;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            setColMajor(T, r, c, R[r][c]);
        }
    }
    setColMajor(T, 0, 3, tx);
    setColMajor(T, 1, 3, ty);
    setColMajor(T, 2, 3, tz);
    setColMajor(T, 3, 0, 0);
    setColMajor(T, 3, 1, 0);
    setColMajor(T, 3, 2, 0);
    setColMajor(T, 3, 3, 1);
    out.transform = T;
    out.ok = true;
    return out;
}

} // namespace KArch
} // namespace DongArch
