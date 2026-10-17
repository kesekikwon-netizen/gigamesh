#include "MeshRaster.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace studio {

namespace KA = DongArch::KArch;

namespace {
struct V3 {
    double x, y, z;
};
inline V3 sub(V3 a, V3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline V3 cross(V3 a, V3 b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }

// 월드 → 카메라 (x 오른쪽, y 위, z 카메라 쪽으로 +)
inline V3 toCam(const KA::Point3& p, const RasterView& v) {
    const double yw = v.yawDeg * std::numbers::pi / 180.0, pt = v.pitchDeg * std::numbers::pi / 180.0;
    const double cy = std::cos(yw), sy = std::sin(yw);
    const double x1 = cy * p.x - sy * p.y, y1 = sy * p.x + cy * p.y, z1 = p.z - v.centerZ;
    // 카메라는 -Y 에서 바라봄: 깊이 = -y1. pitch 는 X축 회전.
    const double cp = std::cos(pt), sp = std::sin(pt);
    const double up = cp * z1 - sp * y1;
    const double depth = -(sp * z1 + cp * y1);
    return {x1, up, depth};
}
}  // namespace

QPointF projectPoint(const KA::Point3& p, const RasterView& v, double* depth) {
    const V3 c = toCam(p, v);
    if (depth) *depth = c.z;
    return {v.centerPx.x() + c.x * v.pxPerMm, v.centerPx.y() - c.y * v.pxPerMm};
}

QImage renderMesh(const KA::MeshData& mesh, QSize size, const RasterView& v, ShadeMode mode, QColor base,
                  std::vector<float>* depthOut, const std::vector<QRgb>* vertexColors) {
    const int W = std::max(1, size.width()), H = std::max(1, size.height());
    QImage img(W, H, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    const float NaN = std::numeric_limits<float>::quiet_NaN();
    std::vector<float> zb(size_t(W) * H, -std::numeric_limits<float>::infinity());
    std::vector<float> lum(size_t(W) * H, 0.f);
    const bool useVCol = (mode == ShadeMode::Real) && vertexColors && vertexColors->size() == mesh.vertices.size();
    std::vector<QRgb> cbBuf;
    if (useVCol) cbBuf.assign(size_t(W) * H, base.rgb());

    std::vector<V3> sv(mesh.vertices.size()), cv(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        const V3 c = toCam(mesh.vertices[i], v);
        cv[i] = c;
        sv[i] = {v.centerPx.x() + c.x * v.pxPerMm, v.centerPx.y() - c.y * v.pxPerMm, c.z};
    }
    const V3 L = [] {
        V3 l{-0.45, 0.55, 0.70};
        const double n = std::sqrt(l.x * l.x + l.y * l.y + l.z * l.z);
        return V3{l.x / n, l.y / n, l.z / n};
    }();

    for (const auto& f : mesh.faces) {
        if (size_t(f[0]) >= sv.size() || size_t(f[1]) >= sv.size() || size_t(f[2]) >= sv.size()) continue;
        const V3 a = sv[size_t(f[0])], b = sv[size_t(f[1])], c = sv[size_t(f[2])];
        const V3 ca = cv[size_t(f[0])], cb = cv[size_t(f[1])], cc = cv[size_t(f[2])];
        V3 n = cross(sub(cb, ca), sub(cc, ca));
        const double nl = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        if (nl < 1e-15) continue;
        n = {n.x / nl, n.y / nl, n.z / nl};
        if (n.z < 0) n = {-n.x, -n.y, -n.z};  // 양면 조명 (스캔 메시 방향 불일치 대비)
        const float shade = float(0.28 + 0.72 * std::max(0.0, n.x * L.x + n.y * L.y + n.z * L.z));

        const int x0 = std::max(0, int(std::floor(std::min({a.x, b.x, c.x}))));
        const int x1 = std::min(W - 1, int(std::ceil(std::max({a.x, b.x, c.x}))));
        const int y0 = std::max(0, int(std::floor(std::min({a.y, b.y, c.y}))));
        const int y1 = std::min(H - 1, int(std::ceil(std::max({a.y, b.y, c.y}))));
        if (x0 > x1 || y0 > y1) continue;
        const double area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        if (std::fabs(area) < 1e-12) continue;
        QRgb c0 = 0, c1 = 0, c2 = 0;
        if (useVCol) {
            c0 = (*vertexColors)[size_t(f[0])];
            c1 = (*vertexColors)[size_t(f[1])];
            c2 = (*vertexColors)[size_t(f[2])];
        }
        const double invS = 1.0 / std::fabs(area);
        for (int y = y0; y <= y1; ++y) {
            const double py = y + 0.5;
            for (int x = x0; x <= x1; ++x) {
                const double px = x + 0.5;
                double w0 = (b.x - px) * (c.y - py) - (b.y - py) * (c.x - px);
                double w1 = (c.x - px) * (a.y - py) - (c.y - py) * (a.x - px);
                double w2 = (a.x - px) * (b.y - py) - (a.y - py) * (b.x - px);
                if (area < 0) w0 = -w0, w1 = -w1, w2 = -w2;
                if (w0 < 0 || w1 < 0 || w2 < 0) continue;
                const float z = float((w0 * a.z + w1 * b.z + w2 * c.z) * invS);
                const size_t k = size_t(y) * W + x;
                if (z > zb[k]) {
                    zb[k] = z;
                    lum[k] = shade;
                    if (useVCol) {
                        const int rr = int((w0 * qRed(c0) + w1 * qRed(c1) + w2 * qRed(c2)) * invS + 0.5);
                        const int gg = int((w0 * qGreen(c0) + w1 * qGreen(c1) + w2 * qGreen(c2)) * invS + 0.5);
                        const int bb = int((w0 * qBlue(c0) + w1 * qBlue(c1) + w2 * qBlue(c2)) * invS + 0.5);
                        cbBuf[k] = qRgb(std::clamp(rr, 0, 255), std::clamp(gg, 0, 255), std::clamp(bb, 0, 255));
                    }
                }
            }
        }
    }

    // 깊이 범위
    float zmin = std::numeric_limits<float>::infinity(), zmax = -zmin;
    for (float z : zb)
        if (std::isfinite(z)) zmin = std::min(zmin, z), zmax = std::max(zmax, z);
    const float zr = std::max(1e-3f, zmax - zmin);

    std::vector<float> rel;
    if (mode == ShadeMode::DTak) {
        // 디지털 탁본: 깊이 - 국소 평균(상자 흐림, 반경 ≈ 2.2 mm) + 미세 법선 곡률 강조 → 높은 곳 어둡게(먹), 파인 곳 밝게(종이)
        const int r = std::max(2, int(std::lround(2.2 * v.pxPerMm)));
        std::vector<double> sat(size_t(W + 1) * (H + 1), 0.0), cnt(size_t(W + 1) * (H + 1), 0.0);
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x) {
                const float z = zb[size_t(y) * W + x];
                const bool ok = std::isfinite(z);
                const size_t k = size_t(y + 1) * (W + 1) + (x + 1);
                sat[k] = (ok ? z : 0.0) + sat[k - 1] + sat[k - (W + 1)] - sat[k - (W + 1) - 1];
                cnt[k] = (ok ? 1.0 : 0.0) + cnt[k - 1] + cnt[k - (W + 1)] - cnt[k - (W + 1) - 1];
            }
        rel.assign(size_t(W) * H, 0.f);
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x) {
                const size_t k = size_t(y) * W + x;
                if (!std::isfinite(zb[k])) continue;
                const int ax = std::max(0, x - r), bx = std::min(W, x + r + 1), ay = std::max(0, y - r),
                          by = std::min(H, y + r + 1);
                auto S = [&](const std::vector<double>& t) {
                    return t[size_t(by) * (W + 1) + bx] - t[size_t(ay) * (W + 1) + bx] - t[size_t(by) * (W + 1) + ax] +
                           t[size_t(ay) * (W + 1) + ax];
                };
                const double c = S(cnt);
                rel[k] = c > 0 ? float(zb[k] - S(sat) / c) : 0.f;
            }
    }

    for (int y = 0; y < H; ++y) {
        auto* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < W; ++x) {
            const size_t k = size_t(y) * W + x;
            if (!std::isfinite(zb[k])) continue;
            QColor c;
            if (mode == ShadeMode::Real) {
                const float s = lum[k];
                const QColor vc = useVCol ? QColor::fromRgb(cbBuf[k]) : base;
                c = QColor::fromRgbF(std::min(1.f, float(vc.redF()) * s * 1.12f),
                                     std::min(1.f, float(vc.greenF()) * s * 1.12f),
                                     std::min(1.f, float(vc.blueF()) * s * 1.12f));
            } else if (mode == ShadeMode::Depth) {
                const float t = (zb[k] - zmin) / zr;  // 앞=1
                const float g = 0.55f + 0.40f * t * (0.6f + 0.4f * lum[k]);
                c = QColor::fromRgbF(g * 0.98f, g * 0.97f, g * 0.93f);
            } else {
                const float t = std::clamp(0.5f - rel[k] * 0.95f, 0.f, 1.f);  // 볼록=어둡게(먹)
                const float g = (0.38f + 0.58f * t) * (0.82f + 0.18f * lum[k]);
                c = QColor::fromRgbF(g * 0.97f, g * 0.96f, g * 0.93f);
            }
            line[x] = c.rgba();
        }
    }
    if (depthOut) {
        depthOut->assign(size_t(W) * H, NaN);
        for (size_t k = 0; k < zb.size(); ++k)
            if (std::isfinite(zb[k])) (*depthOut)[k] = zb[k];
    }
    return img;
}

}  // namespace studio
