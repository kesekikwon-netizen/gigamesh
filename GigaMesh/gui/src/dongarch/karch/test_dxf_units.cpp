/* DongArch3D / K-ArchProfile T8u DXF R12 structural verification (GPLv3+)
 * JOB-001: parse the plate DXF into (group code, value) pairs and verify
 *   AC-2 HEADER   : $ACADVER=AC1009, $INSUNITS=4 (mm), HEADER->TABLES->ENTITIES->EOF
 *   AC-3 LAYER    : layer 0 + exactly 11 layers, exact name / ACI / linetype order
 *   AC-4 ENTITIES : layer and linetype reference integrity, R12 entity types only
 *   AC-5 mm scale : cylinder d=100 mm -> circles r=50, dimension line 100 mm
 *   AC-7 negative : 4 single-point mutations must each be rejected by the same check
 * Output file karch_t8u_cyl.dxf goes to argv[1] if given, else the current directory.
 */

#include "DimensionAnalyzer.h"
#include "DxfWriter.h"
#include "SectionComposer.h"
#include "Silhouette.h"
#include "SyntheticMeshGen.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using DongArch::KArch::AnalyzeDimensions;
using DongArch::KArch::ComposeSection;
using DongArch::KArch::ExtractSilhouette;
using DongArch::KArch::SilhouetteMode;
using DongArch::KArch::WritePlateDxf;
using DongArch::KArch::Testing::MakeCylinderShell;
using DongArch::KArch::Testing::SyntheticParams;

namespace {

constexpr size_t kNone = static_cast<size_t>(-1);

struct Pair {
    int code = 0;
    std::string value;
    size_t line = 0; // 1-based line of the group code
};

// One 0/<TYPE> record (LAYER, LINE, VERTEX, ...): pairs[0] is the 0/<TYPE> pair itself.
struct Record {
    std::string type;
    std::vector<Pair> pairs;
    const std::string* str(int code) const {
        for (size_t i = 1; i < pairs.size(); ++i) {
            if (pairs[i].code == code) {
                return &pairs[i].value;
            }
        }
        return nullptr;
    }
    size_t line() const { return pairs.empty() ? 0 : pairs[0].line; }
};

struct Section {
    std::string name;
    size_t first = 0;  // index of the first pair after 2/<name>
    size_t endsec = 0; // index of 0/ENDSEC
};

struct LayerExpect {
    const char* name;
    int aci;
    const char* lt;
};

// AC-3 expected table: layer 0 + the 11 K-Arch plate layers, in writer order (regression lock).
constexpr LayerExpect kLayers[12] = {
    {"0", 7, "CONTINUOUS"},
    {"01_AXIS_CENTER", 1, "CENTER"},
    {"02_EXTERIOR_OUTLINE", 7, "CONTINUOUS"},
    {"03_EXTERIOR_FEATURES", 3, "CONTINUOUS"},
    {"04_SECTION_WALL", 4, "CONTINUOUS"},
    {"05_SECTION_HATCH", 8, "CONTINUOUS"},
    {"06_INTERIOR_OUTLINE", 5, "CONTINUOUS"},
    {"07_PROJECTION_RIM", 6, "CONTINUOUS"},
    {"08_PROJECTION_BASE", 6, "CONTINUOUS"},
    {"09_DIMENSION_MEASURE", 2, "CONTINUOUS"},
    {"10_ANNOTATION_TEXT", 7, "CONTINUOUS"},
    {"11_SCALE_BAR", 7, "CONTINUOUS"},
};
constexpr const char* kLinetypes[] = {"CONTINUOUS", "CENTER", "DASHED"};
constexpr const char* kR12Entities[] = {"LINE", "POLYLINE", "VERTEX", "SEQEND",
                                        "CIRCLE", "ARC", "TEXT", "SOLID"};
// Layers WritePlateDxf always draws on (03 never, 04/05/06 depend on the section result).
constexpr const char* kMustHaveEntities[] = {"01_AXIS_CENTER", "02_EXTERIOR_OUTLINE",
                                             "07_PROJECTION_RIM", "08_PROJECTION_BASE",
                                             "09_DIMENSION_MEASURE", "10_ANNOTATION_TEXT",
                                             "11_SCALE_BAR"};

struct Verdict {
    std::vector<std::string> fails;
    size_t total = 0;
    void fail(const std::string& item, const std::string& why) {
        ++total;
        if (fails.size() < 20) {
            fails.push_back("FAIL " + item + " " + why);
        }
    }
    bool ok() const { return total == 0; }
    void print() const {
        for (const auto& f : fails) {
            std::fprintf(stderr, "%s\n", f.c_str());
        }
        if (total > fails.size()) {
            std::fprintf(stderr, "... and %zu more failures\n", total - fails.size());
        }
    }
};

std::string fmt(double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.6f", v);
    return buf;
}

std::string orNone(const std::string* s) {
    return s != nullptr ? *s : std::string("<none>");
}

bool parseInt(const std::string& s, int* out) {
    const size_t b = s.find_first_not_of(" \t");
    if (b == std::string::npos) {
        return false;
    }
    const std::string t = s.substr(b, s.find_last_not_of(" \t") - b + 1);
    char* end = nullptr;
    const long v = std::strtol(t.c_str(), &end, 10);
    if (end == t.c_str() || *end != '\0') {
        return false;
    }
    *out = static_cast<int>(v);
    return true;
}

// Symmetric to the writer: classic "C" locale, '.' decimal point.
bool parseDouble(const std::string& s, double* out) {
    std::istringstream is(s);
    is.imbue(std::locale::classic());
    double v = 0.0;
    is >> v;
    if (is.fail()) {
        return false;
    }
    is >> std::ws;
    if (!is.eof()) {
        return false;
    }
    *out = v;
    return true;
}

bool numOf(const Record& r, int code, double* out) {
    const std::string* s = r.str(code);
    return s != nullptr && parseDouble(*s, out);
}

bool parsePairs(const std::string& dxf, std::vector<Pair>* out, std::string* why) {
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < dxf.size()) {
        size_t nl = dxf.find('\n', pos);
        if (nl == std::string::npos) {
            nl = dxf.size();
        }
        std::string ln = dxf.substr(pos, nl - pos);
        if (!ln.empty() && ln.back() == '\r') {
            ln.pop_back();
        }
        lines.push_back(ln);
        pos = nl + 1;
    }
    if (lines.empty() || (lines.size() % 2) != 0) {
        *why = "line count " + std::to_string(lines.size()) + " is not an even number of code/value lines";
        return false;
    }
    for (size_t i = 0; i < lines.size(); i += 2) {
        Pair p;
        p.line = i + 1;
        if (!parseInt(lines[i], &p.code)) {
            *why = "line " + std::to_string(p.line) + " group code '" + lines[i] + "' is not an integer";
            return false;
        }
        p.value = lines[i + 1];
        out->push_back(p);
    }
    return true;
}

bool splitSections(const std::vector<Pair>& P, std::vector<Section>* out, std::string* why) {
    size_t i = 0;
    while (i < P.size()) {
        if (P[i].code == 0 && P[i].value == "EOF") {
            if (i + 1 != P.size()) {
                *why = "pairs after 0/EOF (line " + std::to_string(P[i].line) + ")";
                return false;
            }
            return !out->empty();
        }
        if (P[i].code != 0 || P[i].value != "SECTION") {
            *why = "expected 0/SECTION at line " + std::to_string(P[i].line);
            return false;
        }
        if (i + 1 >= P.size() || P[i + 1].code != 2) {
            *why = "0/SECTION without 2/<name> at line " + std::to_string(P[i].line);
            return false;
        }
        Section s;
        s.name = P[i + 1].value;
        s.first = i + 2;
        size_t j = s.first;
        while (j < P.size() && !(P[j].code == 0 && P[j].value == "ENDSEC")) {
            ++j;
        }
        if (j >= P.size()) {
            *why = "SECTION " + s.name + " without 0/ENDSEC";
            return false;
        }
        s.endsec = j;
        out->push_back(s);
        i = j + 1;
    }
    *why = "missing 0/EOF";
    return false;
}

std::vector<Record> records(const std::vector<Pair>& P, size_t from, size_t to) {
    std::vector<Record> out;
    for (size_t i = from; i < to; ++i) {
        if (P[i].code == 0) {
            Record r;
            r.type = P[i].value;
            r.pairs.push_back(P[i]);
            out.push_back(r);
        } else if (!out.empty()) {
            out.back().pairs.push_back(P[i]);
        }
    }
    return out;
}

template <size_t N>
std::set<std::string> toSet(const char* const (&arr)[N]) {
    return std::set<std::string>(arr, arr + N);
}

std::string at(const Record& r) {
    return r.type + " (line " + std::to_string(r.line()) + ")";
}

// AC-2 .. AC-5 on one DXF text. All failures are accumulated in v.
void checkDxf(const std::string& dxf, Verdict* v) {
    std::vector<Pair> P;
    std::vector<Section> S;
    std::string why;
    if (!parsePairs(dxf, &P, &why)) {
        v->fail("AC-2 parse", why);
        return;
    }
    if (!splitSections(P, &S, &why)) {
        v->fail("AC-2.1 sections", why);
        return;
    }
    if (S.size() != 3 || S[0].name != "HEADER" || S[1].name != "TABLES" || S[2].name != "ENTITIES") {
        std::string names;
        for (const auto& s : S) {
            names += s.name + " ";
        }
        v->fail("AC-2.1 section order", "expected HEADER TABLES ENTITIES, got " + names);
        return;
    }

    // ---- AC-2 HEADER ----
    size_t acadver = kNone;
    size_t insunits = kNone;
    for (size_t i = S[0].first; i < S[0].endsec; ++i) {
        if (P[i].code != 9) {
            continue;
        }
        const Pair& next = P[i + 1]; // i + 1 <= endsec, which exists
        if (P[i].value == "$ACADVER") {
            acadver = i;
            if (next.code != 1 || next.value != "AC1009") {
                v->fail("AC-2.2 $ACADVER", "next pair is " + std::to_string(next.code) + "/" + next.value +
                                               ", expected 1/AC1009");
            }
        } else if (P[i].value == "$INSUNITS") {
            insunits = i;
            int units = -1;
            if (next.code != 70 || !parseInt(next.value, &units) || units != 4) {
                v->fail("AC-2.3 $INSUNITS", "next pair is " + std::to_string(next.code) + "/" + next.value +
                                                ", expected 70/4 (mm)");
            }
        }
    }
    if (acadver == kNone) {
        v->fail("AC-2.2 $ACADVER", "not present in HEADER");
    }
    if (insunits == kNone) {
        v->fail("AC-2.3 $INSUNITS", "not present in HEADER");
    }
    if (acadver != kNone && insunits != kNone && !(acadver < insunits && insunits < S[0].endsec)) {
        v->fail("AC-2.4 HEADER order", "$ACADVER must precede $INSUNITS before ENDSEC");
    }

    // ---- AC-3 LAYER table ----
    const std::set<std::string> ltSet = toSet(kLinetypes);
    const auto tabs = records(P, S[1].first, S[1].endsec);
    size_t k = kNone;
    for (size_t i = 0; i < tabs.size(); ++i) {
        const std::string* n = tabs[i].str(2);
        if (tabs[i].type == "TABLE" && n != nullptr && *n == "LAYER") {
            k = i;
            break;
        }
    }
    if (k == kNone) {
        v->fail("AC-3 LAYER table", "0/TABLE + 2/LAYER not found in TABLES");
    } else {
        std::vector<const Record*> layers;
        size_t j = k + 1;
        for (; j < tabs.size() && tabs[j].type != "ENDTAB"; ++j) {
            if (tabs[j].type != "LAYER") {
                v->fail("AC-3 LAYER table", "unexpected record " + at(tabs[j]));
            } else {
                layers.push_back(&tabs[j]);
            }
        }
        if (j >= tabs.size()) {
            v->fail("AC-3 LAYER table", "no ENDTAB");
        }
        int declared = -1;
        const std::string* cnt = tabs[k].str(70);
        if (cnt == nullptr || !parseInt(*cnt, &declared)) {
            v->fail("AC-3a layer count", "TABLE/LAYER has no integer 70");
        } else if (declared != 12 || layers.size() != 12) {
            v->fail("AC-3a layer count", "70=" + std::to_string(declared) + " records=" +
                                             std::to_string(layers.size()) + ", expected 12 (0 + 11)");
        }
        std::set<std::string> seen;
        for (size_t i = 0; i < layers.size() && i < 12; ++i) {
            const Record& r = *layers[i];
            const LayerExpect& x = kLayers[i];
            const std::string* name = r.str(2);
            const std::string* lt = r.str(6);
            const std::string* aciS = r.str(62);
            const std::string* flagS = r.str(70);
            int aci = -1;
            int flags = -1;
            const std::string tag = (i == 0) ? "AC-3b layer[0]" : "AC-3c layer[" + std::to_string(i) + "]";
            if (name == nullptr || *name != x.name) {
                v->fail(tag, "name " + orNone(name) + ", expected " + x.name);
            } else if (!seen.insert(*name).second) {
                v->fail(tag, "duplicate layer name " + *name);
            }
            if (aciS == nullptr || !parseInt(*aciS, &aci) || aci != x.aci) {
                v->fail(tag, std::string(x.name) + " ACI(62) " + orNone(aciS) + ", expected " + std::to_string(x.aci));
            }
            if (lt == nullptr || *lt != x.lt) {
                v->fail(tag, std::string(x.name) + " linetype(6) " + orNone(lt) + ", expected " + x.lt);
            } else if (ltSet.count(*lt) == 0) {
                v->fail("AC-4.2 layer linetype", *lt + " is not in the LTYPE table");
            }
            if (flagS == nullptr || !parseInt(*flagS, &flags) || flags != 0) {
                v->fail(tag, std::string(x.name) + " flags(70) " + orNone(flagS) + ", expected 0");
            }
        }
    }

    // ---- AC-4 ENTITIES reference integrity ----
    std::set<std::string> plateLayers; // the 11 names; layer 0 is deliberately excluded
    for (size_t i = 1; i < 12; ++i) {
        plateLayers.insert(kLayers[i].name);
    }
    const std::set<std::string> typeSet = toSet(kR12Entities);
    const auto ents = records(P, S[2].first, S[2].endsec);
    std::map<std::string, size_t> perLayer;
    if (ents.empty()) {
        v->fail("AC-4 ENTITIES", "section is empty");
    }
    for (const Record& e : ents) {
        if (typeSet.count(e.type) == 0) {
            v->fail("AC-4.3 entity type", at(e) + " is not an R12 entity");
        }
        const std::string* L = e.str(8);
        if (L == nullptr) {
            v->fail("AC-4.1 layer ref", at(e) + " has no layer (8)");
        } else if (plateLayers.count(*L) == 0) {
            v->fail("AC-4.1 layer ref", at(e) + " on undefined layer " + *L);
        } else {
            ++perLayer[*L];
        }
        const std::string* lt = e.str(6);
        if (lt != nullptr && ltSet.count(*lt) == 0) {
            v->fail("AC-4.2 linetype ref", at(e) + " uses undefined linetype " + *lt);
        }
    }
    for (const char* L : kMustHaveEntities) {
        if (perLayer[L] == 0) {
            v->fail("AC-4.4 layer usage", std::string("layer ") + L + " has no entities");
        }
    }

    // ---- AC-5 mm scale (sign-neutral: absolute values, distances, radii only) ----
    struct Circ {
        double x = 0.0, y = 0.0, r = 0.0;
        size_t n = 0;
    };
    Circ rim, base;
    size_t dimLines = 0;
    bool dimOk = false;
    std::string dimWhy;
    size_t polylines02 = 0;
    std::vector<double> vx, vy;
    for (const Record& e : ents) {
        const std::string* L = e.str(8);
        if (L == nullptr) {
            continue;
        }
        if (e.type == "CIRCLE" && (*L == "07_PROJECTION_RIM" || *L == "08_PROJECTION_BASE")) {
            Circ& c = (*L == "07_PROJECTION_RIM") ? rim : base;
            ++c.n;
            numOf(e, 10, &c.x);
            numOf(e, 20, &c.y);
            numOf(e, 40, &c.r);
        } else if (e.type == "LINE" && *L == "09_DIMENSION_MEASURE") {
            ++dimLines;
            double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
            if (numOf(e, 10, &x0) && numOf(e, 20, &y0) && numOf(e, 11, &x1) && numOf(e, 21, &y1)) {
                const double dy = std::fabs(y1 - y0);
                const double dx = std::fabs(x1 - x0);
                if (dx <= 0.001 && std::fabs(dy - 100.0) <= 0.1) {
                    dimOk = true;
                } else if (dimWhy.empty()) {
                    dimWhy = "|21-20|=" + fmt(dy) + " (expected 100.0+-0.1), |11-10|=" + fmt(dx) + " (<=0.001)";
                }
            }
        } else if (*L == "02_EXTERIOR_OUTLINE") {
            if (e.type == "POLYLINE") {
                ++polylines02;
            } else if (e.type == "VERTEX") {
                double x = 0.0, y = 0.0;
                if (numOf(e, 10, &x) && numOf(e, 20, &y)) {
                    vx.push_back(x);
                    vy.push_back(y);
                }
            }
        }
    }
    if (rim.n != 1) {
        v->fail("AC-5.1 rim circle", "07_PROJECTION_RIM CIRCLE count " + std::to_string(rim.n) + ", expected 1");
    } else if (std::fabs(rim.r - 50.0) > 0.1 || std::fabs(rim.x) > 0.1 || std::fabs(rim.y - 165.0) > 0.5) {
        v->fail("AC-5.1 rim circle", "r=" + fmt(rim.r) + " x=" + fmt(rim.x) + " y=" + fmt(rim.y) +
                                         ", expected r=50+-0.1 x=0+-0.1 y=165+-0.5 (mm)");
    }
    if (base.n != 1) {
        v->fail("AC-5.2 base circle", "08_PROJECTION_BASE CIRCLE count " + std::to_string(base.n) + ", expected 1");
    } else if (std::fabs(base.r - 50.0) > 0.1 || std::fabs(base.y + 65.0) > 0.5) {
        v->fail("AC-5.2 base circle", "r=" + fmt(base.r) + " y=" + fmt(base.y) + ", expected r=50+-0.1 y=-65+-0.5 (mm)");
    }
    if (dimLines == 0) {
        v->fail("AC-5.3 dimension line", "no LINE on 09_DIMENSION_MEASURE");
    } else if (!dimOk) {
        v->fail("AC-5.3 dimension line", dimWhy.empty() ? "LINE without numeric 10/20/11/21" : dimWhy);
    }
    if (polylines02 == 0 || vx.size() < 2) {
        v->fail("AC-5.4 exterior outline", "POLYLINE=" + std::to_string(polylines02) + " VERTEX=" +
                                               std::to_string(vx.size()) + " on 02_EXTERIOR_OUTLINE");
    } else {
        double maxAbsX = 0.0, minY = vy[0], maxY = vy[0];
        for (size_t i = 0; i < vx.size(); ++i) {
            maxAbsX = std::fmax(maxAbsX, std::fabs(vx[i]));
            minY = std::fmin(minY, vy[i]);
            maxY = std::fmax(maxY, vy[i]);
        }
        if (maxAbsX < 49.5 || maxAbsX > 50.5 || minY < -0.5 || maxY > 100.5 || (maxY - minY) < 99.0) {
            v->fail("AC-5.4 exterior outline", "max|10|=" + fmt(maxAbsX) + " (49.5..50.5), 20 in [" + fmt(minY) +
                                                   ", " + fmt(maxY) + "] (expected within -0.5..100.5, range >= 99)");
        }
    }
}

std::string outPath(int argc, char** argv, const char* name) {
    if (argc > 1 && argv[1] != nullptr && argv[1][0] != '\0') {
        std::string dir = argv[1];
        if (dir.back() != '/' && dir.back() != '\\') {
            dir += '/';
        }
        return dir + name;
    }
    return name; // relative to the CTest working directory (build folder)
}

std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

} // namespace

int main(int argc, char** argv) {
    SyntheticParams sp;
    sp.radialSeg = 180;
    sp.heightSeg = 20;
    sp.capTop = true;
    sp.capBottom = true;
    const auto cyl = MakeCylinderShell(50.0, 100.0, 5.0, sp); // d = 100 mm, z in [0, 100] mm
    const auto mesh = cyl.toMesh();
    const auto sil = ExtractSilhouette(mesh, SilhouetteMode::MaxRadius, 0.25);
    const auto sec = ComposeSection(mesh);
    const auto dim = AnalyzeDimensions(mesh, sil);
    const std::string path = outPath(argc, argv, "karch_t8u_cyl.dxf");
    if (!dim.ok) {
        std::fprintf(stderr, "FAIL pipeline AnalyzeDimensions not ok %s\n", dim.warning.c_str());
        return 1;
    }
    if (!WritePlateDxf(path, sil.leftOutline, sec, dim)) {
        std::fprintf(stderr, "FAIL write %s\n", path.c_str());
        return 1;
    }
    const std::string dxf = slurp(path);
    if (dxf.empty()) {
        std::fprintf(stderr, "FAIL read back %s\n", path.c_str());
        return 1;
    }

    Verdict v;
    checkDxf(dxf, &v);
    if (!v.ok()) {
        v.print();
        return 1;
    }

    // ---- AC-7 negative controls: each mutation must be rejected ----
    struct Mutation {
        const char* name;
        const char* from;
        const char* to;
        bool afterEntities;
    };
    const Mutation muts[4] = {
        {"1 ($INSUNITS 4 -> 1 inch)", "\n9\n$INSUNITS\n70\n4\n", "\n9\n$INSUNITS\n70\n1\n", false},
        {"2 ($ACADVER AC1009 -> AC1015)", "\n1\nAC1009\n", "\n1\nAC1015\n", false},
        {"3 (01_AXIS_CENTER ACI 1 -> 2)", "\n2\n01_AXIS_CENTER\n70\n0\n62\n1\n",
         "\n2\n01_AXIS_CENTER\n70\n0\n62\n2\n", false},
        {"4 (first entity on 02 -> 99_UNKNOWN)", "\n8\n02_EXTERIOR_OUTLINE\n", "\n8\n99_UNKNOWN\n", true},
    };
    int rc = 0;
    for (const Mutation& m : muts) {
        size_t start = 0;
        if (m.afterEntities) {
            start = dxf.find("\n2\nENTITIES\n");
            if (start == std::string::npos) {
                std::fprintf(stderr, "FAIL mutation %s ENTITIES marker not found\n", m.name);
                rc = 1;
                continue;
            }
        }
        const size_t pos = dxf.find(m.from, start);
        if (pos == std::string::npos) {
            std::fprintf(stderr, "FAIL mutation %s pattern not present in original\n", m.name);
            rc = 1;
            continue;
        }
        std::string mutated = dxf;
        mutated.replace(pos, std::strlen(m.from), m.to);
        Verdict mv;
        checkDxf(mutated, &mv);
        if (mv.ok()) {
            std::fprintf(stderr, "FAIL mutation %s was not detected\n", m.name);
            rc = 1;
        } else {
            std::printf("mutation %s rejected: %s\n", m.name, mv.fails.front().c_str());
        }
    }
    if (rc != 0) {
        return rc;
    }
    std::printf("dxf %s bytes=%zu HEADER R12 mm, 12 layer records, entity refs, mm scale ok; 4/4 mutations rejected\n",
                path.c_str(), dxf.size());
    return 0;
}
