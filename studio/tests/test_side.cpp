// 헤드리스 검증: 토기 도판 좌/우 D5 (JOB-004a 수용 기준 1-9)
// 기본 Right = 외면(외곽) 왼쪽 · 단면 오른쪽(국내 보고서 표준), R = 좌/우 반전 Left(같은 단면의 거울상).
// 계산값(치수·두께·점 수)은 반전으로 바뀌지 않는다. 화면·SVG·DXF·래스터(PNG/PDF 공용 paintSheet) 가 같은 쪽.
// 허용오차: 곡선 끔 0 mm · 곡선 켬 0.5 mm(test_pipeline.cpp 의 Catmull–Rom 넘침 여유) · 거울상 1e-12 mm(부호 곱셈만) ·
// SVG 0.002 / 0.001 mm(소수 3자리 기록 반올림 2배) · DXF 2e-6 mm(소수 6자리 2배) · 음영 영상 끝 0.01 mm.
#include "ArtifactDoc.h"
#include "Plate.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <map>
#include <string>

using namespace studio;

static int fails = 0;
#define CHECK(c, ...)                         \
    do {                                      \
        if (!(c)) {                           \
            std::printf("FAIL %s: ", #c);     \
            std::printf(__VA_ARGS__);         \
            std::printf("\n");                \
            ++fails;                          \
        }                                     \
    } while (0)

using Pts = std::vector<KA::Point2>;
using Polys = std::vector<Pts>;
using Segs = std::vector<std::pair<KA::Point2, KA::Point2>>;

static constexpr double kMirrorTol = 1e-12;  // mm, 부호 곱셈만이라 실제 0
static constexpr double kSmoothTol = 0.5;    // mm, 곡선 다듬기가 축 너머로 나가는 여유

// 화면·내보내기용 선 한 벌
struct Snap {
    Polys sec, solid, est;
    Pts outline;
    Segs hatch;
};
static Snap snap(const ArtifactDoc& d) {
    return {d.displaySection(), d.displayScannedSolid(), d.displayEstimatedInner(), d.displayOutline(), d.displayHatch()};
}
static Pts flat(const Polys& ps) {
    Pts o;
    for (const auto& p : ps) o.insert(o.end(), p.begin(), p.end());
    return o;
}
static Pts flat(const Segs& ss) {
    Pts o;
    for (const auto& [a, b] : ss) o.push_back(a), o.push_back(b);
    return o;
}
struct RangeX {
    double lo = 1e300, hi = -1e300;
    size_t n = 0;
};
static RangeX rangeX(const Pts& v) {
    RangeX r;
    for (const auto& q : v) r.lo = std::min(r.lo, q.x), r.hi = std::max(r.hi, q.x), ++r.n;
    return r;
}
// 인쇄용 "x[lo, hi] n=…" (빈 목록은 x[-] n=0)
static std::string fmt(const RangeX& r, int prec = 4) {
    char buf[96];
    if (r.n == 0) return "x[-] n=0";
    std::snprintf(buf, sizeof buf, "x[%.*f, %.*f] n=%zu", prec, r.lo, prec, r.hi, r.n);
    return buf;
}
static bool sameSizes(const Polys& a, const Polys& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].size() != b[i].size()) return false;
    return true;
}
// 거울상 오차: max(|xL + xR|, |yL − yR|). 길이가 다르면 무한대
static double mirrorErr(const Pts& r, const Pts& l) {
    if (r.size() != l.size()) return 1e300;
    double m = 0.0;
    for (size_t i = 0; i < r.size(); ++i) m = std::max({m, std::fabs(l[i].x + r[i].x), std::fabs(l[i].y - r[i].y)});
    return m;
}
static bool identical(const Pts& a, const Pts& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].x != b[i].x || a[i].y != b[i].y) return false;
    return true;
}
static bool identical(const Snap& a, const Snap& b) {
    return sameSizes(a.sec, b.sec) && sameSizes(a.solid, b.solid) && sameSizes(a.est, b.est) &&
           identical(flat(a.sec), flat(b.sec)) && identical(flat(a.solid), flat(b.solid)) &&
           identical(flat(a.est), flat(b.est)) && identical(a.outline, b.outline) && identical(flat(a.hatch), flat(b.hatch));
}

// ---- SVG ------------------------------------------------------------------
// 최상위 레이어 <g id="..."> 하나의 본문 (다음 레이어 시작 직전까지)
static QString svgLayer(const QString& svg, const QString& id) {
    const QString key = QStringLiteral("<g id=\"%1\"").arg(id);
    const qsizetype a = svg.indexOf(key);
    if (a < 0) return {};
    const QString mark = QStringLiteral("inkscape:groupmode=\"layer\"");
    const qsizetype own = svg.indexOf(mark, a);
    const qsizetype next = own < 0 ? -1 : svg.indexOf(mark, own + 1);
    const qsizetype end = next < 0 ? svg.size() : svg.lastIndexOf(QStringLiteral("<g id=\""), next);
    return svg.mid(a, end - a);
}
// tags 요소들의 points="x,y x,y …" 를 순서대로
static Polys svgPoints(const QString& seg, const QString& tags) {
    Polys out;
    const QRegularExpression re(QStringLiteral("<(?:%1)\\b[^>]*\\spoints=\"([^\"]*)\"").arg(tags));
    for (auto it = re.globalMatch(seg); it.hasNext();) {
        const auto m = it.next();
        Pts v;
        for (const auto& pr : m.captured(1).split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
            const auto xy = pr.split(QLatin1Char(','));
            if (xy.size() == 2) v.push_back({xy[0].toDouble(), xy[1].toDouble()});
        }
        out.push_back(std::move(v));
    }
    return out;
}
struct SvgShot {
    Polys section, outline;
    bool hasImage = false;
    double imgX = 0, imgW = 0;
};
static SvgShot readSvg(const QString& path) {
    SvgShot s;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return s;
    const QString svg = QString::fromUtf8(f.readAll());
    s.section = svgPoints(svgLayer(svg, "section_profile"), "polygon|polyline");
    s.outline = svgPoints(svgLayer(svg, "outline_axis"), "polyline");
    const QRegularExpression img(QStringLiteral("<image x=\"([-0-9.]+)\" y=\"[-0-9.]+\" width=\"([-0-9.]+)\""));
    const QString shade = svgLayer(svg, "shade_dtak");
    const auto m = img.match(shade);
    if (m.hasMatch()) s.hasImage = true, s.imgX = m.captured(1).toDouble(), s.imgW = m.captured(2).toDouble();
    return s;
}

// ---- DXF ------------------------------------------------------------------
// 층별 VERTEX 의 코드 10(x) 값
static std::map<std::string, std::vector<double>> readDxfVertexX(const QString& path) {
    std::map<std::string, std::vector<double>> out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return out;
    QStringList lines = QString::fromLatin1(f.readAll()).split(QLatin1Char('\n'));
    bool inVertex = false;
    std::string layer;
    for (qsizetype i = 0; i + 1 < lines.size(); i += 2) {
        const int code = lines[i].trimmed().toInt();
        const QString val = lines[i + 1].trimmed();
        if (code == 0) {
            inVertex = val == QLatin1String("VERTEX");
            layer.clear();
        } else if (inVertex && code == 8) {
            layer = val.toStdString();
        } else if (inVertex && code == 10) {
            out[layer].push_back(val.toDouble());
        }
    }
    return out;
}

// ---- 래스터 ---------------------------------------------------------------
struct Counts {
    long left = 0, right = 0;
};
// 용지 한 장을 4 px/mm 로 그려 도판 사각형 안의 비흰 화소를 축 x ± m 밖 왼쪽·오른쪽으로 센다
static Counts rasterCounts(const ArtifactDoc& doc, const PlateOptions& o) {
    const double ppm = 4.0;
    const int m = 5;  // = 0.5 mm × 4 + 3 px (곡선 여유 + 선 두께·안티에일리어싱)
    ExportRequest r;
    r.opts = o;
    const PaperLayout L = layoutPaper(doc, 0, r.paper, r.scale);
    QImage img(int(L.paperMm.width() * ppm), int(L.paperMm.height() * ppm), QImage::Format_RGB32);
    img.fill(Qt::white);
    {
        QPainter p(&img);
        paintSheet(p, doc, r, ppm);
    }
    const double axisPx = (L.plateRectMm.left() - Plate::extentMm(doc, 0).left()) * ppm;
    const QRect plate(int(std::floor(L.plateRectMm.left() * ppm)), int(std::floor(L.plateRectMm.top() * ppm)),
                      int(std::ceil(L.plateRectMm.width() * ppm)), int(std::ceil(L.plateRectMm.height() * ppm)));
    Counts c;
    for (int y = std::max(0, plate.top()); y <= std::min(img.height() - 1, plate.bottom()); ++y) {
        const auto* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = std::max(0, plate.left()); x <= std::min(img.width() - 1, plate.right()); ++x) {
            const QRgb px = line[x];
            if (qRed(px) >= 245 && qGreen(px) >= 245 && qBlue(px) >= 245) continue;
            const double cx = x + 0.5;
            if (cx < axisPx - m) ++c.left;
            else if (cx > axisPx + m) ++c.right;
        }
    }
    return c;
}

static const char* sideName(SectionSide s) { return s == SectionSide::Right ? "Right" : "Left"; }

static void runExample(const char* name, const std::function<void(ArtifactDoc&)>& load, bool blind, const QString& out) {
    std::printf("== %s ==\n", name);
    ArtifactDoc doc;
    // 1. 기본값
    CHECK(doc.sectionSide() == SectionSide::Right, "%s: new doc side should be Right", name);
    CHECK(doc.sectionSign() == 1.0, "%s: sign should be +1", name);
    load(doc);
    CHECK(doc.poseConfirmed(), "%s: example should be confirmed", name);
    CHECK(doc.sectionSide() == SectionSide::Right, "%s: side after load should be Right", name);

    // 2. D5 부호, 곡선 끔 — 허용오차 없음
    doc.setSmooth(false);
    {
        const Snap s = snap(doc);
        const RangeX sec = rangeX(flat(s.sec)), sol = rangeX(flat(s.solid)), est = rangeX(flat(s.est)), ol = rangeX(s.outline);
        std::printf("[2] smooth=off Right: section %s | solid %s | est %s | outline %s\n", fmt(sec).c_str(), fmt(sol).c_str(),
                    fmt(est).c_str(), fmt(ol).c_str());
        CHECK(sec.n > 0, "%s: section empty", name);
        CHECK(ol.n > 0, "%s: outline empty", name);
        CHECK(sec.n == 0 || sec.lo >= 0.0, "%s: section must be x>=0, min=%.6f", name, sec.lo);
        CHECK(sol.n == 0 || sol.lo >= 0.0, "%s: scanned solid must be x>=0, min=%.6f", name, sol.lo);
        CHECK(est.n == 0 || est.lo >= 0.0, "%s: estimated inner must be x>=0, min=%.6f", name, est.lo);
        CHECK(ol.n == 0 || ol.hi <= 0.0, "%s: outline must be x<=0, max=%.6f", name, ol.hi);
        if (blind) {
            CHECK(doc.hasEstimatedInner(), "%s: blind sample should have estimated inner", name);
            CHECK(sol.n > 0 && est.n > 0, "%s: blind solid/est empty (%zu/%zu)", name, sol.n, est.n);
        }
    }

    // 3. D5 부호, 곡선 켬(기본) — 0.5 mm 여유
    doc.setSmooth(true);
    {
        const Snap s = snap(doc);
        const RangeX sec = rangeX(flat(s.sec)), sol = rangeX(flat(s.solid)), est = rangeX(flat(s.est)),
                     ht = rangeX(flat(s.hatch)), ol = rangeX(s.outline);
        std::printf("[3] smooth=on  Right: section %s | solid %s | est %s | hatch %s | outline %s\n", fmt(sec).c_str(),
                    fmt(sol).c_str(), fmt(est).c_str(), fmt(ht).c_str(), fmt(ol).c_str());
        CHECK(sec.n > 0 && ht.n > 0 && ol.n > 0, "%s: section/hatch/outline empty", name);
        CHECK(sec.n == 0 || sec.lo >= -kSmoothTol, "%s: smooth section min=%.4f", name, sec.lo);
        CHECK(sol.n == 0 || sol.lo >= -kSmoothTol, "%s: smooth solid min=%.4f", name, sol.lo);
        CHECK(est.n == 0 || est.lo >= -kSmoothTol, "%s: smooth est min=%.4f", name, est.lo);
        CHECK(ht.n == 0 || ht.lo >= -kSmoothTol, "%s: hatch min=%.4f", name, ht.lo);
        CHECK(ol.n == 0 || ol.hi <= kSmoothTol, "%s: smooth outline max=%.4f", name, ol.hi);
    }

    // 4. R = 거울상 (곡선 끔·켬), 계산값 불변, 한 번 더 누르면 처음과 정확히 같음
    for (const bool smooth : {false, true}) {
        doc.setSmooth(smooth);
        doc.setSectionSide(SectionSide::Right);
        const Snap R = snap(doc);
        const Dims5 d0 = doc.dims();
        const double fa0 = doc.frontAngle();
        const int raw0 = doc.rawPointCount(), pc0 = doc.pointCount();
        std::vector<double> wall0;
        for (int z = 10; z <= 180; z += 10) wall0.push_back(doc.wallThicknessAt(z));

        doc.setSectionSide(SectionSide::Left);
        CHECK(doc.sectionSign() == -1.0, "%s: Left sign should be -1", name);
        const Snap L = snap(doc);
        const bool sizes = sameSizes(R.sec, L.sec) && sameSizes(R.solid, L.solid) && sameSizes(R.est, L.est) &&
                           R.outline.size() == L.outline.size() && R.hatch.size() == L.hatch.size();
        const double eSec = mirrorErr(flat(R.sec), flat(L.sec)), eSol = mirrorErr(flat(R.solid), flat(L.solid)),
                     eEst = mirrorErr(flat(R.est), flat(L.est)), eOl = mirrorErr(R.outline, L.outline),
                     eHt = mirrorErr(flat(R.hatch), flat(L.hatch));
        std::printf("[4] smooth=%d mirror err: section %.3g solid %.3g est %.3g outline %.3g hatch %.3g (n=%zu/%zu/%zu/%zu/%zu)\n",
                    int(smooth), eSec, eSol, eEst, eOl, eHt, flat(R.sec).size(), flat(R.solid).size(), flat(R.est).size(),
                    R.outline.size(), R.hatch.size());
        CHECK(sizes, "%s: Left/Right sizes differ (smooth=%d)", name, int(smooth));
        CHECK(std::max({eSec, eSol, eEst, eOl, eHt}) <= kMirrorTol, "%s: mirror err %.3g (smooth=%d)", name,
              std::max({eSec, eSol, eEst, eOl, eHt}), int(smooth));
        // 빗금 방향: Right "/" (끝점 차의 x·y 부호 같음), Left "\"
        if (!R.hatch.empty()) {
            const auto& [a, b] = R.hatch.front();
            const auto& [c, e] = L.hatch.front();
            CHECK((b.x - a.x) * (b.y - a.y) > 0 && (e.x - c.x) * (e.y - c.y) < 0, "%s: hatch slope Right / , Left \\", name);
        }
        const Dims5& d1 = doc.dims();
        CHECK(d1.ok == d0.ok && d1.height == d0.height && d1.rim == d0.rim && d1.belly == d0.belly && d1.bellyZ == d0.bellyZ &&
                  d1.base == d0.base && d1.meanWall == d0.meanWall,
              "%s: dims changed by flip", name);
        CHECK(doc.frontAngle() == fa0, "%s: frontAngle changed", name);
        CHECK(doc.rawPointCount() == raw0 && doc.pointCount() == pc0, "%s: point counts changed %d/%d -> %d/%d", name, pc0,
              raw0, doc.pointCount(), doc.rawPointCount());
        bool wallSame = true;
        for (int i = 0, z = 10; z <= 180; z += 10, ++i) wallSame = wallSame && doc.wallThicknessAt(z) == wall0[size_t(i)];
        CHECK(wallSame, "%s: wallThicknessAt changed by flip", name);

        doc.toggleSectionSide();
        CHECK(doc.sectionSide() == SectionSide::Right, "%s: toggle should return to Right", name);
        CHECK(identical(snap(doc), R), "%s: Right after toggle differs from first Right (smooth=%d)", name, int(smooth));
    }

    // 5. 세션 유지: 로드가 좌/우를 되돌리지 않는다
    doc.setSectionSide(SectionSide::Left);
    load(doc);
    CHECK(doc.sectionSide() == SectionSide::Left, "%s: load must keep Left", name);
    doc.setSectionSide(SectionSide::Right);
    doc.setSmooth(true);

    // 6. SVG 같은 쪽 (기본 ExportRequest, A4 세로 1:1)
    const double X0 = layoutPaper(doc, 0, Paper::A4Portrait, 1.0).plateRectMm.left() - Plate::extentMm(doc, 0).left();
    SvgShot svg[2];
    for (const SectionSide side : {SectionSide::Right, SectionSide::Left}) {
        doc.setSectionSide(side);
        ExportRequest r;
        r.title = QStringLiteral("좌우 시험");
        r.path = out + QStringLiteral("/studio_side_%1_%2.svg").arg(name, sideName(side));
        QString err;
        CHECK(exportSvgLayered(doc, r, &err), "%s: svg %s", name, err.toUtf8().constData());
        const SvgShot s = readSvg(r.path);
        svg[side == SectionSide::Right ? 0 : 1] = s;
        const RangeX sec = rangeX(flat(s.section)), ol = rangeX(flat(s.outline));
        std::printf("[6] SVG %s: X0=%.3f section %s | outline %s | image x=%.3f w=%.3f (x+w=%.3f)\n", sideName(side), X0,
                    fmt(sec, 3).c_str(), fmt(ol, 3).c_str(), s.imgX, s.imgW, s.imgX + s.imgW);
        CHECK(sec.n > 0 && ol.n > 0 && s.hasImage, "%s: svg %s layers empty", name, sideName(side));
        if (side == SectionSide::Right) {
            CHECK(sec.lo >= X0 - 0.5, "%s: svg Right section min %.3f < X0-0.5", name, sec.lo);
            CHECK(ol.hi <= X0 + 0.5, "%s: svg Right outline max %.3f > X0+0.5", name, ol.hi);
            CHECK(s.imgX + s.imgW <= X0 + 0.01, "%s: svg Right image right edge %.3f > X0", name, s.imgX + s.imgW);
        } else {
            CHECK(sec.hi <= X0 + 0.5, "%s: svg Left section max %.3f > X0+0.5", name, sec.hi);
            CHECK(ol.lo >= X0 - 0.5, "%s: svg Left outline min %.3f < X0-0.5", name, ol.lo);
            CHECK(s.imgX >= X0 - 0.01, "%s: svg Left image left edge %.3f < X0", name, s.imgX);
        }
    }
    {
        double e = 0.0;
        bool sameLen = true;
        const std::vector<std::pair<Pts, Pts>> pairs = {{flat(svg[0].section), flat(svg[1].section)},
                                                        {flat(svg[0].outline), flat(svg[1].outline)}};
        for (const auto& [r, l] : pairs) {
            sameLen = sameLen && r.size() == l.size();
            if (r.size() != l.size()) continue;
            for (size_t i = 0; i < r.size(); ++i) {
                CHECK(std::fabs((l[i].x - X0) + (r[i].x - X0)) <= 0.002 && std::fabs(l[i].y - r[i].y) <= 0.001,
                      "%s: svg point %zu not mirrored (%.3f,%.3f) vs (%.3f,%.3f)", name, i, r[i].x, r[i].y, l[i].x, l[i].y);
                e = std::max({e, std::fabs((l[i].x - X0) + (r[i].x - X0)), std::fabs(l[i].y - r[i].y)});
            }
        }
        std::printf("[6] SVG Left/Right mirror max err %.4f mm\n", e);
        CHECK(sameLen, "%s: svg Left/Right point counts differ", name);
    }

    // 7. DXF 같은 쪽
    std::map<std::string, std::vector<double>> dxf[2];
    for (const SectionSide side : {SectionSide::Right, SectionSide::Left}) {
        doc.setSectionSide(side);
        ExportRequest r;
        r.title = QStringLiteral("좌우 시험");
        r.path = out + QStringLiteral("/studio_side_%1_%2.dxf").arg(name, sideName(side));
        QString err;
        CHECK(exportDxf(doc, r, &err), "%s: dxf %s", name, err.toUtf8().constData());
        auto& m = dxf[side == SectionSide::Right ? 0 : 1];
        m = readDxfVertexX(r.path);
        const bool right = side == SectionSide::Right;
        for (const char* layer : {"04_SECTION_WALL", "06_INTERIOR_OUTLINE", "02_EXTERIOR_OUTLINE"}) {
            const auto& xs = m[layer];
            double lo = 1e300, hi = -1e300;
            for (double x : xs) lo = std::min(lo, x), hi = std::max(hi, x);
            std::printf("[7] DXF %s %s: %s\n", sideName(side), layer, fmt({lo, hi, xs.size()}, 6).c_str());
            const bool exterior = std::string(layer) == "02_EXTERIOR_OUTLINE";
            if (exterior == right) CHECK(xs.empty() || hi <= 0.5, "%s: dxf %s %s max %.6f > 0.5", name, sideName(side), layer, hi);
            else CHECK(xs.empty() || lo >= -0.5, "%s: dxf %s %s min %.6f < -0.5", name, sideName(side), layer, lo);
        }
        CHECK(!m["04_SECTION_WALL"].empty() && !m["02_EXTERIOR_OUTLINE"].empty(), "%s: dxf %s section/outline empty", name,
              sideName(side));
        if (blind) CHECK(!m["06_INTERIOR_OUTLINE"].empty(), "%s: dxf %s interior empty", name, sideName(side));
    }
    for (const char* layer : {"04_SECTION_WALL", "06_INTERIOR_OUTLINE", "02_EXTERIOR_OUTLINE"}) {
        const auto& r = dxf[0][layer];
        const auto& l = dxf[1][layer];
        CHECK(r.size() == l.size(), "%s: dxf %s vertex count %zu vs %zu", name, layer, r.size(), l.size());
        double e = 0.0;
        for (size_t i = 0; i < std::min(r.size(), l.size()); ++i) e = std::max(e, std::fabs(l[i] + r[i]));
        std::printf("[7] DXF %s Left/Right |xL+xR| max %.2g\n", layer, e);
        CHECK(e <= 2e-6, "%s: dxf %s not mirrored, err %.3g", name, layer, e);
    }
    doc.setSectionSide(SectionSide::Right);
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    const QString out = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath();

    runExample("sample", [](ArtifactDoc& d) { d.loadSample(); }, false, out);
    runExample("blind", [](ArtifactDoc& d) { d.loadArtecBlindSample(); }, true, out);

    // 8. 래스터 (화면·PNG·PDF 공용 paintSheet → Plate::paint): 예제 1, A4 1:1, 4 px/mm
    {
        ArtifactDoc doc;
        doc.loadSample();
        PlateOptions secOnly;
        secOnly.grid = secOnly.shade = secOnly.outline = secOnly.labels = false;
        secOnly.section = true;
        PlateOptions shadeOnly;
        shadeOnly.grid = shadeOnly.section = shadeOnly.outline = shadeOnly.labels = false;
        shadeOnly.shade = true;
        for (const SectionSide side : {SectionSide::Right, SectionSide::Left}) {
            doc.setSectionSide(side);
            const Counts a = rasterCounts(doc, secOnly), b = rasterCounts(doc, shadeOnly);
            std::printf("[8] raster %s: section-only left=%ld right=%ld | shade-only left=%ld right=%ld\n", sideName(side), a.left,
                        a.right, b.left, b.right);
            if (side == SectionSide::Right) {
                CHECK(a.left == 0 && a.right >= 1000, "raster Right section-only left=%ld right=%ld", a.left, a.right);
                CHECK(b.right == 0 && b.left >= 1000, "raster Right shade-only left=%ld right=%ld", b.left, b.right);
            } else {
                CHECK(a.right == 0 && a.left >= 1000, "raster Left section-only left=%ld right=%ld", a.left, a.right);
                CHECK(b.left == 0 && b.right >= 1000, "raster Left shade-only left=%ld right=%ld", b.left, b.right);
            }
        }
    }

    std::puts(fails ? "FAILED" : "PASS");
    return fails ? 1 : 0;
}
