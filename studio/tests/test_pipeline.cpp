// 헤드리스 검증: 기울어진 합성 토기 → 정위치 → 5대 치수 → 레이어 SVG / DXF / PNG / PDF / 바이너리 STL 왕복
// + Artec 3D 스캐너 특화 검증 (정점 컬러 · 3D LOD · VRML .wrl 미터 단위 자동환산 · 부유 노이즈 제거 · 내벽 미스캔 파선 보정)
#include "ArtifactDoc.h"
#include "MeshIO.h"
#include "Plate.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <cmath>
#include <cstdio>

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

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    const QString out = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath();

    ArtifactDoc doc;
    doc.loadSample();  // 기울기 7°, 이동 (12,-5,3), Artec 컬러
    const auto& d = doc.dims();
    std::printf("aligned=%d note=%s rmse=%.4f\n", doc.aligned(), doc.alignNote().toUtf8().constData(), doc.axisRmse());
    std::printf("H=%.2f rim=%.2f belly=%.2f base=%.2f wall=%.2f pts=%d/%d lodFaces=%zu\n", d.height, d.rim, d.belly, d.base,
                d.meanWall, doc.pointCount(), doc.rawPointCount(), doc.lodMesh().faces.size());
    CHECK(doc.aligned(), "axis not aligned");
    CHECK(doc.hasColor(), "Artec sample should have vertex colors");
    CHECK(!doc.lodMesh().faces.empty() && doc.lodMesh().faces.size() <= doc.mesh().faces.size(), "LOD mesh invalid");
    CHECK(d.ok, "dims not ok");
    CHECK(near(d.height, 184.5, 1.5), "height %.2f", d.height);
    CHECK(near(d.rim, 2 * 71.0, 2.0), "rim %.2f", d.rim);
    CHECK(near(d.belly, 2 * 82.6, 2.0), "belly %.2f", d.belly);
    CHECK(near(d.meanWall, 6.8, 1.0), "wall %.2f", d.meanWall);
    CHECK(!doc.displaySection().empty(), "no section");
    CHECK(doc.displayOutline().size() > 10, "outline too short");

    // 토기 표준: 단면은 왼쪽(x<0), 외곽은 오른쪽(x>0)
    double secMaxX = -1e9, olMinX = 1e9;
    for (const auto& p : doc.displaySection())
        for (const auto& q : p) secMaxX = std::max(secMaxX, q.x);
    for (const auto& q : doc.displayOutline()) olMinX = std::min(olMinX, q.x);
    CHECK(secMaxX <= 0.5, "section must be left, maxX=%.2f", secMaxX);
    CHECK(olMinX >= -0.5, "outline must be right, minX=%.2f", olMinX);

    // 단면 고리를 높이 92 mm 수평선으로 자른 폭 = 기벽 두께
    {
        std::vector<double> xs;
        for (const auto& p : doc.displaySection())
            for (size_t i = 0; i < p.size(); ++i) {
                const auto &a = p[i], &b = p[(i + 1) % p.size()];
                if ((a.y <= 92 && b.y > 92) || (b.y <= 92 && a.y > 92)) xs.push_back(a.x + (92 - a.y) * (b.x - a.x) / (b.y - a.y));
            }
        std::printf("section crossings at H92: %zu\n", xs.size());
        CHECK(xs.size() == 2, "section should cross H92 exactly twice, got %zu", xs.size());
        if (xs.size() == 2) CHECK(near(std::fabs(xs[0] - xs[1]), 6.8, 0.8), "section width %.2f", std::fabs(xs[0] - xs[1]));
    }
    CHECK(!doc.displayHatch().empty(), "hatch empty");

    // 간략화가 점 수를 줄이는지
    doc.setSmooth(false);
    doc.setSimplifyTol(1.0);
    const int p10 = doc.pointCount();
    doc.setSimplifyTol(0.1);
    const int p01 = doc.pointCount();
    CHECK(p10 < p01, "DP 1.0 (%d) should be < 0.1 (%d)", p10, p01);
    doc.setSimplifyTol(0.5);
    doc.setSmooth(true);

    // 내보내기
    ExportRequest r;
    r.title = QStringLiteral("도면 1. 예제 합성토기 01호");
    r.path = out + "/studio_plate.svg";
    QString err;
    CHECK(exportSvgLayered(doc, r, &err), "svg %s", err.toUtf8().constData());
    QFile f(r.path);
    CHECK(f.open(QIODevice::ReadOnly), "open svg");
    const QByteArray svg = f.readAll();
    for (const char* id : {"grid_dims_scale", "shade_dtak", "outline_axis", "section_profile"})
        CHECK(svg.contains(id), "svg layer %s missing", id);
    CHECK(svg.contains("width=\"210mm\""), "svg paper size");
    CHECK(svg.indexOf("grid_dims_scale") < svg.indexOf("section_profile"), "layer order (section on top)");

    r.path = out + "/studio_plate.dxf";
    CHECK(exportDxf(doc, r, &err), "dxf %s", err.toUtf8().constData());
    r.path = out + "/studio_plate.png";
    CHECK(exportPng(doc, r, 150, &err), "png %s", err.toUtf8().constData());
    r.path = out + "/studio_plate.pdf";
    CHECK(exportPdf(doc, r, &err), "pdf %s", err.toUtf8().constData());

    // 용지 맞춤 안내
    const auto L1 = layoutPaper(doc, 0, Paper::A4Portrait, 1.0);
    const auto L2 = layoutPaper(doc, 0, Paper::A4Portrait, 2.0);
    std::printf("A4 1:1 fits=%d | 2:1 fits=%d advice=%s\n", L1.fits, L2.fits, L2.advice.toUtf8().constData());
    CHECK(L1.fits, "1:1 should fit A4");
    CHECK(!L2.fits, "2:1 should not fit A4");

    // 바이너리 STL 왕복 + 미터(m) 단위 자동환산 + 부유 노이즈 제거 검증
    {
        const auto& m = doc.mesh();
        QFile s(out + "/studio_rt.stl");
        CHECK(s.open(QIODevice::WriteOnly), "open stl");
        QByteArray hdr(80, 0);
        s.write(hdr);
        const quint32 n = quint32(m.faces.size() + 2);  // +2: 공중 부유 노이즈 삼각형
        s.write(reinterpret_cast<const char*>(&n), 4);
        for (const auto& fc : m.faces) {
            float buf[12] = {0, 0, 0};
            for (int k = 0; k < 3; ++k) {
                const auto& v = m.vertices[size_t(fc[size_t(k)])];
                // 일부러 미터(m, ×0.001) 단위로 저장하여 Artec m→mm 자동 환산 검증
                buf[3 + 3 * k] = float(v.x * 0.001), buf[4 + 3 * k] = float(v.y * 0.001), buf[5 + 3 * k] = float(v.z * 0.001);
            }
            s.write(reinterpret_cast<const char*>(buf), 48);
            s.write("\0\0", 2);
        }
        // 멀리 떨어진 부유 노이즈 삼각형 2개 추가
        for (int t = 0; t < 2; ++t) {
            float buf[12] = {0, 0, 0, 0.5f, 0.5f, 0.5f, 0.501f, 0.5f, 0.5f, 0.5f, 0.501f, 0.5f + 0.001f * t};
            s.write(reinterpret_cast<const char*>(buf), 48);
            s.write("\0\0", 2);
        }
        s.close();
        ArtifactDoc d2;
        CHECK(d2.loadFile(out + "/studio_rt.stl", &err), "stl load %s", err.toUtf8().constData());
        std::printf("STL(m->mm): verts=%zu H=%.2f rim=%.2f noiseRemoved=%d unit=%s\n",
                    d2.mesh().vertices.size(), d2.dims().height, d2.dims().rim, d2.removedNoiseFaces(),
                    d2.unitNote().toUtf8().constData());
        CHECK(d2.removedNoiseFaces() == 2, "expected 2 floating noise faces removed, got %d", d2.removedNoiseFaces());
        CHECK(!d2.unitNote().isEmpty(), "expected m->mm unit auto-conversion");
        CHECK(near(d2.dims().rim, d.rim, 1.0), "stl rim %.2f vs %.2f", d2.dims().rim, d.rim);
    }

    // Artec 내벽 미스캔(좁은 아가리 토기) 파선 자동 보정 검증
    {
        ArtifactDoc blind;
        blind.loadArtecBlindSample();
        std::printf("ArtecBlind: H=%.2f rim=%.2f hasEst=%d estWall=%.2f\n",
                    blind.dims().height, blind.dims().rim, blind.hasEstimatedInner(), blind.estimatedWallMm());
        CHECK(blind.hasEstimatedInner(), "blind sample should synthesize estimated inner wall");
        CHECK(!blind.displaySection().empty(), "blind sample should have closed section loop");
        CHECK(blind.isWallEstimatedAt(50.0), "H=50mm should be in estimated inner-wall zone");
        ExportRequest br;
        br.title = QStringLiteral("도면 2. Artec 내벽 미스캔 보정");
        br.path = out + "/studio_blind.svg";
        CHECK(exportSvgLayered(blind, br, &err), "blind svg %s", err.toUtf8().constData());
        QFile bf(br.path);
        CHECK(bf.open(QIODevice::ReadOnly), "open blind svg");
        CHECK(bf.readAll().contains("stroke-dasharray=\"2.0,1.3\""), "blind svg must contain dashed estimated inner wall");
    }

    std::puts(fails ? "FAILED" : "PASS");
    return fails ? 1 : 0;
}
