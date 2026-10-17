/* DongArch3D / K-ArchProfile T8 DXF R12 (GPLv3+) */

#include "DxfWriter.h"

#include <iomanip>
#include <locale>
#include <sstream>

namespace DongArch {
namespace KArch {
namespace {

std::string asciiDim(const char* prefix, double mm) {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << prefix << std::fixed << std::setprecision(1) << RoundTenths(mm) << "mm";
    return s.str();
}

void cross(DxfWriter& w, const std::string& layer, Point2 c, double arm) {
    w.WriteLine(layer, Point2{c.x - arm, c.y}, Point2{c.x + arm, c.y}, DxfLinetype::Center);
    w.WriteLine(layer, Point2{c.x, c.y - arm}, Point2{c.x, c.y + arm}, DxfLinetype::Center);
}

void projection(DxfWriter& w, const std::string& layer, const ArcSpec& a) {
    if (a.radius <= 0.0) {
        return;
    }
    if (a.isRestored && (a.endAngleDeg - a.startAngleDeg) < 359.0 &&
        (a.endAngleDeg - a.startAngleDeg) > 1.0) {
        w.WriteArc(layer, a.center, a.radius, a.startAngleDeg, a.endAngleDeg, true);
    } else {
        w.WriteCircle(layer, a.center, a.radius, a.isRestored);
    }
}

} // namespace

const char* DxfWriter::ltName(DxfLinetype lt) const {
    switch (lt) {
    case DxfLinetype::Center:
        return "CENTER";
    case DxfLinetype::Dashed:
        return "DASHED";
    default:
        return "CONTINUOUS";
    }
}

void DxfWriter::writePair(int code, const std::string& val) {
    out << code << '\n' << val << '\n';
}

void DxfWriter::writePairInt(int code, int val) {
    out << code << '\n' << val << '\n';
}

void DxfWriter::writePair(int code, double val) {
    out << code << '\n' << std::fixed << std::setprecision(6) << val << '\n';
}

bool DxfWriter::Open(const std::string& path) {
    out.open(path, std::ios::out | std::ios::binary);
    if (!out) {
        return false;
    }
    out.imbue(std::locale::classic());
    layers.clear();
    entitiesStarted = false;
    return true;
}

void DxfWriter::DefineLayer(const std::string& name, int aci, DxfLinetype lt) {
    layers.push_back(LayerDef{name, aci, lt});
}

void DxfWriter::ensureEntities() {
    if (entitiesStarted) {
        return;
    }
    writePair(0, "SECTION");
    writePair(2, "HEADER");
    writePair(9, "$ACADVER");
    writePair(1, "AC1009");
    writePair(9, "$INSUNITS");
    writePairInt(70, 4);
    writePair(0, "ENDSEC");

    writePair(0, "SECTION");
    writePair(2, "TABLES");

    writePair(0, "TABLE");
    writePair(2, "LTYPE");
    writePairInt(70, 3);
    writePair(0, "LTYPE");
    writePair(2, "CONTINUOUS");
    writePairInt(70, 0);
    writePair(3, "Solid line");
    writePairInt(72, 65);
    writePairInt(73, 0);
    writePair(40, 0.0);

    writePair(0, "LTYPE");
    writePair(2, "CENTER");
    writePairInt(70, 0);
    writePair(3, "Center ____ _ ____");
    writePairInt(72, 65);
    writePairInt(73, 4);
    writePair(40, 2.0);
    writePair(49, 1.25);
    writePair(49, -0.25);
    writePair(49, 0.25);
    writePair(49, -0.25);

    writePair(0, "LTYPE");
    writePair(2, "DASHED");
    writePairInt(70, 0);
    writePair(3, "Dashed __ __");
    writePairInt(72, 65);
    writePairInt(73, 2);
    writePair(40, 0.75);
    writePair(49, 0.5);
    writePair(49, -0.25);
    writePair(0, "ENDTAB");

    writePair(0, "TABLE");
    writePair(2, "LAYER");
    writePairInt(70, static_cast<int>(layers.size()) + 1);
    writePair(0, "LAYER");
    writePair(2, "0");
    writePairInt(70, 0);
    writePairInt(62, 7);
    writePair(6, "CONTINUOUS");
    for (const auto& L : layers) {
        writePair(0, "LAYER");
        writePair(2, L.name);
        writePairInt(70, 0);
        writePairInt(62, L.aci);
        writePair(6, ltName(L.lt));
    }
    writePair(0, "ENDTAB");
    writePair(0, "ENDSEC");

    writePair(0, "SECTION");
    writePair(2, "ENTITIES");
    entitiesStarted = true;
}

void DxfWriter::WriteLine(const std::string& layer, Point2 a, Point2 b, DxfLinetype overrideLt) {
    ensureEntities();
    writePair(0, "LINE");
    writePair(8, layer);
    if (overrideLt != DxfLinetype::Continuous) {
        writePair(6, ltName(overrideLt));
    }
    writePair(10, a.x);
    writePair(20, a.y);
    writePair(30, 0.0);
    writePair(11, b.x);
    writePair(21, b.y);
    writePair(31, 0.0);
}

void DxfWriter::WritePolyline(const std::string& layer, const std::vector<Point2>& pts, bool closed) {
    if (pts.size() < 2) {
        return;
    }
    ensureEntities();
    writePair(0, "POLYLINE");
    writePair(8, layer);
    writePairInt(66, 1);
    writePair(10, 0.0);
    writePair(20, 0.0);
    writePair(30, 0.0);
    writePairInt(70, closed ? 1 : 0);
    for (const Point2& p : pts) {
        writePair(0, "VERTEX");
        writePair(8, layer);
        writePair(10, p.x);
        writePair(20, p.y);
        writePair(30, 0.0);
    }
    writePair(0, "SEQEND");
    writePair(8, layer);
}

void DxfWriter::WriteCircle(const std::string& layer, Point2 c, double r, bool dashed) {
    if (r <= 0.0) {
        return;
    }
    ensureEntities();
    writePair(0, "CIRCLE");
    writePair(8, layer);
    if (dashed) {
        writePair(6, "DASHED");
    }
    writePair(10, c.x);
    writePair(20, c.y);
    writePair(30, 0.0);
    writePair(40, r);
}

void DxfWriter::WriteArc(const std::string& layer, Point2 c, double r, double startDeg, double endDeg,
                         bool dashed) {
    if (r <= 0.0) {
        return;
    }
    ensureEntities();
    writePair(0, "ARC");
    writePair(8, layer);
    if (dashed) {
        writePair(6, "DASHED");
    }
    writePair(10, c.x);
    writePair(20, c.y);
    writePair(30, 0.0);
    writePair(40, r);
    writePair(50, startDeg);
    writePair(51, endDeg);
}

void DxfWriter::WriteText(const std::string& layer, Point2 pos, double height,
                          const std::string& ascii) {
    ensureEntities();
    writePair(0, "TEXT");
    writePair(8, layer);
    writePair(10, pos.x);
    writePair(20, pos.y);
    writePair(30, 0.0);
    writePair(40, height);
    writePair(1, ascii);
}

void DxfWriter::WriteSolid(const std::string& layer, Point2 a, Point2 b, Point2 c, Point2 d) {
    ensureEntities();
    writePair(0, "SOLID");
    writePair(8, layer);
    writePair(10, a.x);
    writePair(20, a.y);
    writePair(30, 0.0);
    writePair(11, b.x);
    writePair(21, b.y);
    writePair(31, 0.0);
    writePair(12, c.x);
    writePair(22, c.y);
    writePair(32, 0.0);
    writePair(13, d.x);
    writePair(23, d.y);
    writePair(33, 0.0);
}

bool DxfWriter::Close() {
    if (!out) {
        return false;
    }
    ensureEntities();
    writePair(0, "ENDSEC");
    writePair(0, "EOF");
    out.close();
    return !out.fail();
}

bool WritePlateDxf(const std::string& path, const std::vector<Point2>& leftOutline,
                   const SectionResult& section, const Dimensions& dims) {
    DxfWriter w;
    if (!w.Open(path)) {
        return false;
    }
    w.DefineLayer("01_AXIS_CENTER", 1, DxfLinetype::Center);
    w.DefineLayer("02_EXTERIOR_OUTLINE", 7, DxfLinetype::Continuous);
    w.DefineLayer("03_EXTERIOR_FEATURES", 3, DxfLinetype::Continuous);
    w.DefineLayer("04_SECTION_WALL", 4, DxfLinetype::Continuous);
    w.DefineLayer("05_SECTION_HATCH", 8, DxfLinetype::Continuous);
    w.DefineLayer("06_INTERIOR_OUTLINE", 5, DxfLinetype::Continuous);
    w.DefineLayer("07_PROJECTION_RIM", 6, DxfLinetype::Continuous);
    w.DefineLayer("08_PROJECTION_BASE", 6, DxfLinetype::Continuous);
    w.DefineLayer("09_DIMENSION_MEASURE", 2, DxfLinetype::Continuous);
    w.DefineLayer("10_ANNOTATION_TEXT", 7, DxfLinetype::Continuous);
    w.DefineLayer("11_SCALE_BAR", 7, DxfLinetype::Continuous);

    const double z0 = dims.zMin;
    const double z1 = dims.zMax;
    w.WriteLine("01_AXIS_CENTER", Point2{0.0, z0 - 20.0}, Point2{0.0, z1 + 20.0},
                DxfLinetype::Center);
    if (dims.rimArc.radius > 0.0) {
        cross(w, "01_AXIS_CENTER", dims.rimArc.center, 4.0);
    }
    if (dims.baseArc.radius > 0.0) {
        cross(w, "01_AXIS_CENTER", dims.baseArc.center, 4.0);
    }

    w.WritePolyline("02_EXTERIOR_OUTLINE", leftOutline, false);
    for (const auto& poly : section.wallPolygons) {
        w.WritePolyline("04_SECTION_WALL", poly, true);
    }
    for (const auto& ln : section.hatchLines) {
        w.WriteLine("05_SECTION_HATCH", ln.first, ln.second);
    }
    if (section.interiorOutline.size() >= 2) {
        w.WritePolyline("06_INTERIOR_OUTLINE", section.interiorOutline, false);
    }

    projection(w, "07_PROJECTION_RIM", dims.rimArc);
    projection(w, "08_PROJECTION_BASE", dims.baseArc);

    w.WriteLine("09_DIMENSION_MEASURE", Point2{-62.0, z0}, Point2{-62.0, z1});
    const double th = 3.2;
    w.WriteText("10_ANNOTATION_TEXT", Point2{-78.0, z0 + dims.vesselHeight * 0.55}, th,
                asciiDim("H ", dims.vesselHeight));
    w.WriteText("10_ANNOTATION_TEXT", Point2{-78.0, dims.maxBodyZ}, th,
                asciiDim("BODY D. ", dims.maxBodyDiameter));
    w.WriteText("10_ANNOTATION_TEXT", Point2{8.0, dims.rimArc.center.y}, th,
                asciiDim("RIM D. ", dims.rimDiameter));
    w.WriteText("10_ANNOTATION_TEXT", Point2{8.0, dims.baseArc.center.y}, th,
                asciiDim("BASE D. ", dims.baseDiameter));

    const double barY = (dims.baseArc.radius > 0.0)
                            ? (dims.baseArc.center.y - dims.baseArc.radius - 20.0)
                            : (z0 - 40.0);
    const double barH = 4.0;
    for (int i = 0; i < 10; ++i) {
        if ((i % 2) != 0) {
            continue;
        }
        const double x0 = static_cast<double>(i) * 10.0 - 50.0;
        const double x1 = x0 + 10.0;
        w.WriteSolid("11_SCALE_BAR", Point2{x0, barY}, Point2{x1, barY}, Point2{x0, barY + barH},
                     Point2{x1, barY + barH});
    }
    w.WriteLine("11_SCALE_BAR", Point2{-50.0, barY}, Point2{50.0, barY});
    w.WriteText("11_SCALE_BAR", Point2{-50.0, barY - 6.0}, th, "SCALE 1:1");
    return w.Close();
}

} // namespace KArch
} // namespace DongArch
