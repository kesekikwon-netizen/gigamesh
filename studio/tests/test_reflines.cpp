// 헤드리스 검증: 기준선 레이어(중심선·구연선·바닥선)와 정면 방위각 (JOB-007 수용 기준 1-9, 판정 꼬리표 [J7-n])
// 기준선 = 3D 기준에서 내려온 선: 중심선 = 3D 중심축(도판 x 0), 바닥선 = 바닥판(y 0), 구연선 = 구연 평면(y H).
// 허용오차: 도면 mm 1e-9(같은 식) · SVG ±0.002 mm(소수 3자리 반올림 두 번) · DXF ±2e-6 mm(소수 6자리) ·
// 기고 H ±0.05 mm(치수 0.1 mm 반올림) · 래스터 = 3×3 화소 창의 비흰(min(R,G,B) < 245) / Caution(R − B > 40) 유무.
// 출력 폴더 = argv[1] — 파이썬 독립 판정(명세 §5-3)이 여기 남긴 SVG·DXF 를 다시 읽는다.
#include "ArtifactDoc.h"
#include "Plate.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QXmlStreamReader>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <optional>
#include <utility>
#include <vector>

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

static bool near(double a, double b, double tol) { return std::fabs(a - b) <= tol; }
static double meshH(const ArtifactDoc& d) { return d.mesh().bboxMax.z - d.mesh().bboxMin.z; }
static bool lineIs(const std::optional<RefLine>& l, double ax, double ay, double bx, double by, double tol) {
    return l && near(l->a.x, ax, tol) && near(l->a.y, ay, tol) && near(l->b.x, bx, tol) && near(l->b.y, by, tol);
}
static bool samePts(const std::optional<RefLine>& a, const std::optional<RefLine>& b) {
    if (a.has_value() != b.has_value()) return false;
    return !a || (a->a.x == b->a.x && a->a.y == b->a.y && a->b.x == b->b.x && a->b.y == b->b.y);
}

// ---- 기준 1·3: 기하 -----------------------------------------------------------
static void checkGeometry(const char* tag, const ArtifactDoc& d) {
    const double H = meshH(d);
    const auto& m = d.dims();
    CHECK(std::fabs(d.mesh().bboxMin.z) <= 1e-9, "%s bboxMin.z %.12f", tag, d.mesh().bboxMin.z);
    CHECK(near(H, m.height, 0.05), "%s H %.4f vs dims.height %.4f", tag, H, m.height);
    const PlateRefLines r = plateRefLines(d, 3.0);
    CHECK(lineIs(r.center, 0, -3, 0, H + 3, 1e-9), "%s center", tag);
    CHECK(lineIs(r.rim, -m.rim / 2, H, m.rim / 2, H, 1e-9), "%s rim (rim %.3f)", tag, m.rim);
    if (m.base > 0) CHECK(lineIs(r.base, -m.base / 2, 0, m.base / 2, 0, 1e-9), "%s base (base %.3f)", tag, m.base);
    else CHECK(!r.base, "%s base should be absent (base 0)", tag);
    CHECK(lineIs(plateRefLines(d, 9.0).center, 0, -9, 0, H + 9, 1e-9), "%s center ext 9", tag);
}

// ---- SVG ----------------------------------------------------------------------
struct SvgEl {
    QString tag;
    QXmlStreamAttributes at;
};
struct SvgLayer {
    QString id, label;
    std::vector<SvgEl> els;  // 층 안의 모든 자손 요소
};
static std::vector<SvgLayer> readLayers(const QString& path) {
    std::vector<SvgLayer> out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        CHECK(false, "[J7-6] open %s", qUtf8Printable(path));
        return out;
    }
    QXmlStreamReader x(&f);
    int depth = 0;
    bool inLayer = false;
    while (!x.atEnd()) {
        const auto t = x.readNext();
        if (t == QXmlStreamReader::StartElement) {
            ++depth;
            if (depth == 2) {
                inLayer = x.name() == QLatin1String("g");
                if (inLayer)
                    out.push_back({x.attributes().value(QLatin1String("id")).toString(),
                                   x.attributes()
                                       .value(QLatin1String("http://www.inkscape.org/namespaces/inkscape"), QLatin1String("label"))
                                       .toString(),
                                   {}});
            } else if (depth > 2 && inLayer) {
                out.back().els.push_back({x.name().toString(), x.attributes()});
            }
        } else if (t == QXmlStreamReader::EndElement) {
            if (depth == 2) inLayer = false;
            --depth;
        }
    }
    CHECK(!x.hasError(), "[J7-6] svg parse %s: %s", qUtf8Printable(path), qUtf8Printable(x.errorString()));
    return out;
}
static QStringList layerIds(const std::vector<SvgLayer>& ls) {
    QStringList ids;
    for (const auto& l : ls) ids << l.id;
    return ids;
}
static const SvgLayer* findLayer(const std::vector<SvgLayer>& ls, const QString& id) {
    for (const auto& l : ls)
        if (l.id == id) return &l;
    return nullptr;
}
static std::vector<const SvgEl*> elsOf(const SvgLayer& l, const char* tag) {
    std::vector<const SvgEl*> v;
    for (const auto& e : l.els)
        if (e.tag == QLatin1String(tag)) v.push_back(&e);
    return v;
}
static double attr(const SvgEl& e, const char* k) { return e.at.value(QLatin1String(k)).toDouble(); }
static QString readText(const QString& path) {
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

// 기준 6: 층 순서 · 기준선 층마다 선 하나·레이블·굵기·색·좌표 · 외곽선 층 · 방안 층
static void checkSvg(const char* name, const QString& path, const ArtifactDoc& d, double s) {
    const auto ls = readLayers(path);
    const QStringList want = {"grid_dims_scale", "shade_dtak",   "refline_center", "refline_rim",
                              "refline_base",    "outline_axis", "section_profile"};
    CHECK(layerIds(ls) == want, "[J7-6] %s layer ids %s", name, qUtf8Printable(layerIds(ls).join(',')));
    const PaperLayout L = layoutPaper(d, 0, Paper::A4Portrait, s);
    const QRectF E = Plate::extentMm(d, 0);
    const double X0 = L.plateRectMm.left() - E.left() * s, Yb = L.plateRectMm.top() + E.bottom() * s, H = meshH(d);
    const auto& m = d.dims();
    struct Want {
        const char* id;
        QString label;
        double w, x1, y1, x2, y2;
    };
    const Want ws[] = {{"refline_center", QStringLiteral("중심선"), 0.10, X0, Yb + 3, X0, Yb - H * s - 3},
                       {"refline_rim", QStringLiteral("구연선"), 0.13, X0 - m.rim * s / 2, Yb - H * s, X0 + m.rim * s / 2, Yb - H * s},
                       {"refline_base", QStringLiteral("바닥선"), 0.25, X0 - m.base * s / 2, Yb, X0 + m.base * s / 2, Yb}};
    for (const auto& w : ws) {
        const SvgLayer* l = findLayer(ls, QLatin1String(w.id));
        if (!l) {
            CHECK(false, "[J7-6] %s layer %s missing", name, w.id);
            continue;
        }
        const auto lines = elsOf(*l, "line");
        CHECK(lines.size() == 1 && l->els.size() == 1, "[J7-6] %s %s: %zu line / %zu elements", name, w.id, lines.size(),
              l->els.size());
        CHECK(l->label == w.label, "[J7-6] %s %s label %s", name, w.id, qUtf8Printable(l->label));
        if (lines.empty()) continue;
        const SvgEl& e = *lines[0];
        CHECK(e.at.value(QLatin1String("stroke")).toString().toUpper() == QLatin1String("#141413"), "[J7-6] %s %s stroke %s",
              name, w.id, qUtf8Printable(e.at.value(QLatin1String("stroke")).toString()));
        CHECK(near(attr(e, "stroke-width"), w.w, 1e-9), "[J7-6] %s %s stroke-width %.4f", name, w.id, attr(e, "stroke-width"));
        CHECK(!e.at.hasAttribute(QLatin1String("stroke-dasharray")), "[J7-6] %s %s has dasharray", name, w.id);
        const double got[] = {attr(e, "x1"), attr(e, "y1"), attr(e, "x2"), attr(e, "y2")};
        const double exp[] = {w.x1, w.y1, w.x2, w.y2};
        for (int i = 0; i < 4; ++i)
            CHECK(near(got[i], exp[i], 0.002), "[J7-6] %s %s coord %d: %.4f vs %.4f", name, w.id, i, got[i], exp[i]);
    }
    if (const SvgLayer* o = findLayer(ls, QStringLiteral("outline_axis")))
        CHECK(elsOf(*o, "line").empty() && elsOf(*o, "polyline").size() == 1, "[J7-6] %s outline_axis: %zu line, %zu polyline",
              name, elsOf(*o, "line").size(), elsOf(*o, "polyline").size());
    if (const SvgLayer* g = findLayer(ls, QStringLiteral("grid_dims_scale")))
        for (const SvgEl* e : elsOf(*g, "line"))
            CHECK(!(e->at.value(QLatin1String("stroke")).toString().toUpper() == QLatin1String("#141413") &&
                    near(attr(*e, "y1"), Yb, 0.002) && near(attr(*e, "y2"), Yb, 0.002)),
                  "[J7-6] %s grid layer still has an ink base line at y %.3f", name, Yb);
    CHECK(!readText(path).contains(QLatin1String("6 1 1 1")), "[J7-6] %s has dasharray 6 1 1 1", name);
}

// ---- DXF ----------------------------------------------------------------------
struct DxfRec {
    QString type;
    std::vector<std::pair<int, QString>> kv;
    QString get(int c) const {
        for (const auto& [k, v] : kv)
            if (k == c) return v;
        return {};
    }
    bool has(int c) const {
        return std::any_of(kv.begin(), kv.end(), [c](const auto& p) { return p.first == c; });
    }
};
static std::vector<DxfRec> readDxf(const QString& path) {
    std::vector<DxfRec> out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        CHECK(false, "[J7-8] open %s", qUtf8Printable(path));
        return out;
    }
    const QStringList lines = QString::fromLatin1(f.readAll()).split(QLatin1Char('\n'));
    for (qsizetype i = 0; i + 1 < lines.size(); i += 2) {
        const int code = lines[i].trimmed().toInt();
        const QString val = lines[i + 1].trimmed();
        if (code == 0) out.push_back({val, {}});
        else if (!out.empty()) out.back().kv.push_back({code, val});
    }
    return out;
}
static std::vector<const DxfRec*> dxfOf(const std::vector<DxfRec>& rs, const char* type, const char* layer) {
    std::vector<const DxfRec*> v;
    for (const auto& r : rs)
        if (r.type == QLatin1String(type) && (!layer || r.get(8) == QLatin1String(layer))) v.push_back(&r);
    return v;
}
// 정면 방위각 TEXT: 값이 az 인 10_ANNOTATION_TEXT 가 정확히 1개, FRONT AZIMUTH 로 시작하는 TEXT 도 그 하나뿐
static void checkDxfAzimuth(const char* name, const std::vector<DxfRec>& rs, const QString& az) {
    int exact = 0, any = 0;
    for (const DxfRec* t : dxfOf(rs, "TEXT", nullptr)) {
        if (t->get(8) == QLatin1String("10_ANNOTATION_TEXT") && t->get(1) == az) ++exact;
        if (t->get(1).startsWith(QLatin1String("FRONT AZIMUTH"))) ++any;
    }
    CHECK(exact == 1 && any == 1, "[J7-8] %s azimuth text '%s': exact %d, FRONT AZIMUTH* %d", name, qUtf8Printable(az), exact, any);
}
static void checkDxf(const char* name, const QString& path, const ArtifactDoc& d, const QString& az) {
    const auto rs = readDxf(path);
    QStringList names;
    for (const DxfRec* l : dxfOf(rs, "LAYER", nullptr)) names << l->get(2);
    const QStringList want = {"0",
                              "01_AXIS_CENTER",
                              "02_EXTERIOR_OUTLINE",
                              "03_EXTERIOR_FEATURES",
                              "04_SECTION_WALL",
                              "05_SECTION_HATCH",
                              "06_INTERIOR_OUTLINE",
                              "07_PROJECTION_RIM",
                              "08_PROJECTION_BASE",
                              "09_DIMENSION_MEASURE",
                              "10_ANNOTATION_TEXT",
                              "11_SCALE_BAR"};
    CHECK(names == want, "[J7-8] %s layers %s", name, qUtf8Printable(names.join(',')));
    for (const DxfRec* l : dxfOf(rs, "LAYER", nullptr)) {
        const QString n = l->get(2);
        if (n == QLatin1String("01_AXIS_CENTER") || n == QLatin1String("07_PROJECTION_RIM") || n == QLatin1String("08_PROJECTION_BASE"))
            CHECK(l->get(62) == QLatin1String("7") && l->get(6) == QLatin1String("CONTINUOUS"), "[J7-8] %s layer %s aci %s lt %s",
                  name, qUtf8Printable(n), qUtf8Printable(l->get(62)), qUtf8Printable(l->get(6)));
    }
    const double H = meshH(d);
    const auto& m = d.dims();
    struct Want {
        const char* layer;
        double x1, y1, x2, y2;
    };
    const Want ws[] = {{"01_AXIS_CENTER", 0, -3, 0, H + 3},
                       {"07_PROJECTION_RIM", -m.rim / 2, H, m.rim / 2, H},
                       {"08_PROJECTION_BASE", -m.base / 2, 0, m.base / 2, 0}};
    for (const auto& w : ws) {
        const auto ls = dxfOf(rs, "LINE", w.layer);
        CHECK(ls.size() == 1, "[J7-8] %s %s: %zu LINE", name, w.layer, ls.size());
        if (ls.size() != 1) continue;
        CHECK(!ls[0]->has(6), "[J7-8] %s %s LINE has linetype override", name, w.layer);
        const double got[] = {ls[0]->get(10).toDouble(), ls[0]->get(20).toDouble(), ls[0]->get(11).toDouble(),
                              ls[0]->get(21).toDouble()};
        const double exp[] = {w.x1, w.y1, w.x2, w.y2};
        for (int i = 0; i < 4; ++i)
            CHECK(near(got[i], exp[i], 2e-6), "[J7-8] %s %s coord %d: %.6f vs %.6f", name, w.layer, i, got[i], exp[i]);
    }
    CHECK(dxfOf(rs, "LINE", "02_EXTERIOR_OUTLINE").empty(), "[J7-8] %s 02_EXTERIOR_OUTLINE still has %zu LINE", name,
          dxfOf(rs, "LINE", "02_EXTERIOR_OUTLINE").size());
    checkDxfAzimuth(name, rs, az);
}

// ---- 래스터 (PDF·PNG 공용 paintSheet) ---------------------------------------------
struct Sheet {
    QImage img;
    double X0 = 0, Yb = 0, s = 1;
};
static constexpr double kPpm = 8.0;
static Sheet drawSheet(const ArtifactDoc& d, double s, bool labels) {
    ExportRequest r;
    r.scale = s;
    r.opts.grid = r.opts.shade = r.opts.section = false;
    r.opts.outline = true;
    r.opts.labels = labels;
    const PaperLayout L = layoutPaper(d, 0, r.paper, s);
    const QRectF E = Plate::extentMm(d, 0);
    Sheet sh;
    sh.img = QImage(int(L.paperMm.width() * kPpm), int(L.paperMm.height() * kPpm), QImage::Format_RGB32);
    sh.img.fill(Qt::white);
    {
        QPainter p(&sh.img);
        paintSheet(p, d, r, kPpm);
    }
    sh.X0 = L.plateRectMm.left() - E.left() * s;
    sh.Yb = L.plateRectMm.top() + E.bottom() * s;
    sh.s = s;
    return sh;
}
// 도면점 (x,y) → 화소 ((X0 + x·s)·8, (Yb − y·s)·8) 둘레 3×3 에 조건을 만족하는 화소가 있는지
static bool around(const Sheet& sh, double x, double y, bool caution) {
    const int cx = int(std::floor((sh.X0 + x * sh.s) * kPpm)), cy = int(std::floor((sh.Yb - y * sh.s) * kPpm));
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            const int px = cx + dx, py = cy + dy;
            if (px < 0 || py < 0 || px >= sh.img.width() || py >= sh.img.height()) continue;
            const QRgb c = sh.img.pixel(px, py);
            if (caution ? qRed(c) - qBlue(c) > 40 : std::min({qRed(c), qGreen(c), qBlue(c)}) < 245) return true;
        }
    return false;
}
static bool ink(const Sheet& sh, double x, double y) { return around(sh, x, y, false); }
static bool caution(const Sheet& sh, double x, double y) { return around(sh, x, y, true); }

static QString azNote(const char* angle) {
    return QStringLiteral("정면 방위각 ") + QString::fromUtf8(angle) + QStringLiteral(" (위에서 보아 반시계)");
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    const QString out = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath();
    auto P = [&](const char* n) { return out + QLatin1Char('/') + QLatin1String(n); };

    ArtifactDoc d;
    d.loadSample();
    // 기준 1: 기하 (예제 0° · Right)
    checkGeometry("[J7-1] sample", d);
    {
        const auto ol = d.displayOutline();
        const double H = meshH(d);
        std::printf("[J7-1] sample H=%.4f rim=%.2f base=%.2f outline ends=(%.3f,%.3f)…(%.3f,%.3f) gap low %.3f high %.3f mm\n", H,
                    d.dims().rim, d.dims().base, ol.empty() ? 0.0 : ol.front().x, ol.empty() ? 0.0 : ol.front().y,
                    ol.empty() ? 0.0 : ol.back().x, ol.empty() ? 0.0 : ol.back().y, ol.empty() ? 0.0 : ol.front().y,
                    ol.empty() ? 0.0 : H - ol.back().y);
    }
    // 기준 2: 좌/우 반전에도 기준선은 같다 (도면 좌표에서 좌우 대칭)
    {
        const PlateRefLines r = plateRefLines(d, 3.0);
        d.setSectionSide(SectionSide::Left);
        const PlateRefLines l = plateRefLines(d, 3.0);
        d.setSectionSide(SectionSide::Right);
        CHECK(samePts(r.center, l.center) && samePts(r.rim, l.rim) && samePts(r.base, l.base), "[J7-2] Left != Right");
        std::printf("[J7-2] Left == Right: %d\n", int(samePts(r.center, l.center) && samePts(r.rim, l.rim) && samePts(r.base, l.base)));
    }
    // 기준 5 (0° · 가로 슬롯)
    CHECK(azimuthNote(d, 0) == azNote("0°"), "[J7-5] 0deg note '%s'", qUtf8Printable(azimuthNote(d, 0)));
    CHECK(azimuthNote(d, 1).isEmpty(), "[J7-5] horizontal slot note '%s'", qUtf8Printable(azimuthNote(d, 1)));

    // 내보내기 (기본 요청 R0 = A4 세로 · 1:1 · opts 기본값 · slot 0)
    ExportRequest R0;
    R0.title = QStringLiteral("기준선 시험");
    QString err;
    auto svgOut = [&](const char* n, ExportRequest r) {
        r.path = P(n);
        CHECK(exportSvgLayered(d, r, &err), "[J7-6] export %s: %s", n, qUtf8Printable(err));
    };
    auto dxfOut = [&](const char* n) {
        ExportRequest r = R0;
        r.path = P(n);
        CHECK(exportDxf(d, r, &err), "[J7-8] export %s: %s", n, qUtf8Printable(err));
    };
    svgOut("studio_reflines_s1.svg", R0);
    {
        ExportRequest r = R0;
        r.scale = 1.0 / 3.0;
        svgOut("studio_reflines_s3.svg", r);
    }
    d.setSectionSide(SectionSide::Left);
    svgOut("studio_reflines_L.svg", R0);
    d.setSectionSide(SectionSide::Right);
    {
        ExportRequest r = R0;
        r.opts.slot = 1;
        svgOut("studio_reflines_h.svg", r);
    }
    dxfOut("studio_reflines.dxf");
    // 기준 6 · 7 · 8 (0°)
    checkSvg("_s1", P("studio_reflines_s1.svg"), d, 1.0);
    checkSvg("_s3", P("studio_reflines_s3.svg"), d, 1.0 / 3.0);
    checkSvg("_L", P("studio_reflines_L.svg"), d, 1.0);
    {
        const QStringList hids = layerIds(readLayers(P("studio_reflines_h.svg")));
        CHECK(hids == QStringList({"grid_dims_scale", "section_profile"}), "[J7-6] _h layer ids %s", qUtf8Printable(hids.join(',')));
        CHECK(!readText(P("studio_reflines_h.svg")).contains(QLatin1String("6 1 1 1")), "[J7-6] _h has 6 1 1 1");
    }
    std::printf("[J7-6] svg layers checked (_s1 · _s3 · _L · _h)\n");
    CHECK(readText(P("studio_reflines_s1.svg")).contains(azNote("0°")), "[J7-7] _s1 azimuth missing");
    CHECK(!readText(P("studio_reflines_h.svg")).contains(QStringLiteral("정면 방위각")), "[J7-7] _h has azimuth");
    checkDxf("studio_reflines.dxf", P("studio_reflines.dxf"), d, QStringLiteral("FRONT AZIMUTH 0.0 DEG (CCW, TOP VIEW)"));
    std::printf("[J7-8] dxf 0deg checked\n");

    // 기준 9: 래스터 (paintSheet = PDF·PNG 길), 8 px/mm
    {
        const double H = meshH(d), rim = d.dims().rim, base = d.dims().base;
        const Sheet a = drawSheet(d, 1.0, false);
        const bool r1 = ink(a, rim / 2 - 2, H) && !ink(a, rim / 2 + 3, H) && ink(a, base / 2 - 2, 0) && !ink(a, base / 2 + 3, 0) &&
                        ink(a, 0, H + 1.5) && !ink(a, 0, H + 5) && ink(a, 0, -1.5) && !ink(a, 0, -5);
        CHECK(r1, "[J7-9] s=1 ink: rim %d/%d base %d/%d center top %d/%d bottom %d/%d", int(ink(a, rim / 2 - 2, H)),
              int(ink(a, rim / 2 + 3, H)), int(ink(a, base / 2 - 2, 0)), int(ink(a, base / 2 + 3, 0)), int(ink(a, 0, H + 1.5)),
              int(ink(a, 0, H + 5)), int(ink(a, 0, -1.5)), int(ink(a, 0, -5)));
        const Sheet b = drawSheet(d, 1.0 / 3.0, false);
        const bool r3 = ink(b, rim / 2 - 6, H) && !ink(b, rim / 2 + 9, H) && ink(b, 0, H + 4.5) && !ink(b, 0, H + 15);
        CHECK(r3, "[J7-9] s=1/3 ink: rim %d/%d center %d/%d", int(ink(b, rim / 2 - 6, H)), int(ink(b, rim / 2 + 9, H)),
              int(ink(b, 0, H + 4.5)), int(ink(b, 0, H + 15)));
        const Sheet c = drawSheet(d, 1.0, true);
        const bool lab = !caution(c, -rim / 2 + 3, H) && !caution(c, -rim / 4, H) && !caution(c, -3, H) && ink(c, -rim / 4, H) &&
                         caution(c, -rim / 2 - 6, H);
        CHECK(lab, "[J7-9] labels: caution(-rim/2+3) %d caution(-rim/4) %d caution(-3) %d ink(-rim/4) %d caution(-rim/2-6) %d",
              int(caution(c, -rim / 2 + 3, H)), int(caution(c, -rim / 4, H)), int(caution(c, -3, H)), int(ink(c, -rim / 4, H)),
              int(caution(c, -rim / 2 - 6, H)));
        std::printf("[J7-9] raster s=1 %d · s=1/3 %d · labels %d\n", int(r1), int(r3), int(lab));
    }

    // 12° (A4 세로 1:1 — R0 그대로)
    d.setFrontAngle(12);
    CHECK(azimuthNote(d, 0) == azNote("12°"), "[J7-5] 12deg note '%s'", qUtf8Printable(azimuthNote(d, 0)));
    svgOut("studio_reflines_12.svg", R0);
    dxfOut("studio_reflines_12.dxf");
    checkSvg("_12", P("studio_reflines_12.svg"), d, 1.0);
    CHECK(readText(P("studio_reflines_12.svg")).contains(azNote("12°")), "[J7-7] _12 azimuth missing");
    checkDxf("studio_reflines_12.dxf", P("studio_reflines_12.dxf"), d, QStringLiteral("FRONT AZIMUTH 12.0 DEG (CCW, TOP VIEW)"));
    std::printf("[J7-8] dxf 12deg checked\n");

    // −20° · −0.04° (부호 · −0 없음)
    d.setFrontAngle(-20);
    CHECK(azimuthNote(d, 0) == azNote("−20°"), "[J7-5] -20deg note '%s'", qUtf8Printable(azimuthNote(d, 0)));
    dxfOut("studio_reflines_m20.dxf");
    checkDxfAzimuth("_m20.dxf", readDxf(P("studio_reflines_m20.dxf")), QStringLiteral("FRONT AZIMUTH -20.0 DEG (CCW, TOP VIEW)"));
    d.setFrontAngle(-0.04);
    CHECK(azimuthNote(d, 0) == azNote("0°"), "[J7-5] -0.04deg note '%s'", qUtf8Printable(azimuthNote(d, 0)));
    dxfOut("studio_reflines_m004.dxf");
    checkDxfAzimuth("_m004.dxf", readDxf(P("studio_reflines_m004.dxf")), QStringLiteral("FRONT AZIMUTH 0.0 DEG (CCW, TOP VIEW)"));
    std::printf("[J7-5] notes: 0° · 12° · −20° · −0.04° → %s\n", qUtf8Printable(azimuthNote(d, 0)));

    // 기준 3: 미스캔 예제
    {
        ArtifactDoc b;
        b.loadArtecBlindSample();
        checkGeometry("[J7-3] blind", b);
        const PlateRefLines r = plateRefLines(b, 3.0);
        CHECK(r.rim && near(r.rim->a.x, -b.dims().rim / 2, 1e-9), "[J7-3] blind rim.a.x");
        std::printf("[J7-3] blind H=%.4f rim=%.2f base=%.2f rim.a.x=%.3f base line=%d\n", meshH(b), b.dims().rim, b.dims().base,
                    r.rim ? r.rim->a.x : 0.0, int(r.base.has_value()));
    }
    // 기준 4: 빈 문서 · 확정 전
    {
        ArtifactDoc e;
        const PlateRefLines r = plateRefLines(e, 3.0);
        CHECK(!r.center && !r.rim && !r.base && azimuthNote(e, 0).isEmpty(), "[J7-4] empty doc");
        ArtifactDoc u;
        u.loadSample(false);
        const PlateRefLines q = plateRefLines(u, 3.0);
        CHECK(!q.center && !q.rim && !q.base && azimuthNote(u, 0).isEmpty(), "[J7-4] unconfirmed doc");
        std::printf("[J7-4] empty/unconfirmed: none\n");
    }

    std::puts(fails ? "FAILED" : "PASS");
    return fails ? 1 : 0;
}
