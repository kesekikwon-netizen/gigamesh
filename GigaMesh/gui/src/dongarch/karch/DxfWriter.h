/* DongArch3D / K-ArchProfile T8 DXF R12 (GPLv3+)
 * R12 group codes: Autodesk acad_r12_dxf.pdf (AC1009)
 * $INSUNITS 4 = mm: Autodesk HEADER section (cloudhelp 2024)
 */

#ifndef DONGARCH_KARCH_DXF_WRITER_H
#define DONGARCH_KARCH_DXF_WRITER_H

#include "DimensionAnalyzer.h"

#include <fstream>
#include <string>
#include <vector>

namespace DongArch {
namespace KArch {

enum class DxfLinetype { Continuous, Center, Dashed };

class DxfWriter {
public:
    bool Open(const std::string& path);
    void DefineLayer(const std::string& name, int aci, DxfLinetype lt);
    void WriteLine(const std::string& layer, Point2 a, Point2 b,
                   DxfLinetype overrideLt = DxfLinetype::Continuous);
    void WritePolyline(const std::string& layer, const std::vector<Point2>& pts, bool closed);
    void WriteCircle(const std::string& layer, Point2 c, double r, bool dashed = false);
    void WriteArc(const std::string& layer, Point2 c, double r, double startDeg, double endDeg,
                  bool dashed = false);
    void WriteText(const std::string& layer, Point2 pos, double height, const std::string& ascii);
    void WriteSolid(const std::string& layer, Point2 a, Point2 b, Point2 c, Point2 d);
    bool Close();

private:
    void ensureEntities();
    void writePair(int code, const std::string& val);
    void writePair(int code, double val);
    void writePairInt(int code, int val);
    const char* ltName(DxfLinetype lt) const;

    struct LayerDef {
        std::string name;
        int aci = 7;
        DxfLinetype lt = DxfLinetype::Continuous;
    };

    std::ofstream out;
    std::vector<LayerDef> layers;
    bool entitiesStarted = false;
};

// 1:1 mm plate. TEXT is ASCII only (R12).
bool WritePlateDxf(const std::string& path, const std::vector<Point2>& leftOutline,
                   const SectionResult& section, const Dimensions& dims);

} // namespace KArch
} // namespace DongArch

#endif
