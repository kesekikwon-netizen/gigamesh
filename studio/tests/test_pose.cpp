// 헤드리스 검증: ① 바닥 정하기 (자세 단계, JOB-003 수용 기준 1-9)
// 90° 네 번 = 원래 · 뒤집기 두 번 = 원래 · 조작 뒤 접지·박스 중심 유지 · 처음 자세로 ·
// 눕힌 항아리 자동 세우기 · 확정 전 미계산 · 파일 열기는 자세를 바꾸지 않음 · 수동 확정.
// 허용오차: 기하 항등 1e-9 mm (K-Arch 테스트 규약), 두 파이프라인 비교 1.0 mm,
// 절대값 기고 184.5 ± 1.5 · 구경 142.0 ± 2.0 mm (test_pipeline.cpp 와 같음).
#include "ArtifactDoc.h"
#include "MeshIO.h"
#include "MeshLoader.h"
#include "SyntheticMeshGen.h"

#include <QDir>
#include <QGuiApplication>
#include <algorithm>
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

static constexpr double kGeomTol = 1e-9;  // mm

static double maxDelta(const std::vector<KA::Point3>& a, const std::vector<KA::Point3>& b) {
    if (a.size() != b.size()) return 1e300;
    double m = 0.0;
    for (size_t i = 0; i < a.size(); ++i)
        m = std::max({m, std::fabs(a[i].x - b[i].x), std::fabs(a[i].y - b[i].y), std::fabs(a[i].z - b[i].z)});
    return m;
}
static double midX(const KA::MeshData& m) { return 0.5 * (m.bboxMin.x + m.bboxMax.x); }
static double midY(const KA::MeshData& m) { return 0.5 * (m.bboxMin.y + m.bboxMax.y); }

// 예제와 같은 합성 항아리 (구경 142 · 최대복경 165.2 · 기고 184.5 · 기벽 6.8 mm, 입 열림·바닥 막힘)
static KA::MeshData vessel(double tiltDeg, KA::Point3 t, double noise) {
    KA::Testing::SyntheticParams sp;
    sp.radialSeg = 160;
    sp.heightSeg = 120;
    sp.capTop = false;
    sp.capBottom = true;
    sp.tiltDeg = tiltDeg;
    sp.translation = t;
    sp.noiseSigma = noise;
    KA::MeshData m = KA::Testing::MakeTorusProfileVessel(82.6, 71.0, 184.5, 6.8, sp).toMesh();
    KA::FillMeshStats(m);
    return m;
}

static const char* axisName(PoseAxis a) { return a == PoseAxis::X ? "X" : a == PoseAxis::Y ? "Y" : "Z"; }

// 확정 전에는 단면·외곽·치수·슬롯을 계산하지 않는다 (수용 기준 6)
static void checkUnconfirmed(const ArtifactDoc& d, const char* where) {
    CHECK(!d.poseConfirmed(), "%s: should be unconfirmed", where);
    CHECK(!d.dims().ok, "%s: dims must not be computed", where);
    CHECK(d.dims().height == 0.0, "%s: height %.3f", where, d.dims().height);
    CHECK(d.displaySection().empty(), "%s: section must be empty", where);
    CHECK(d.displayOutline().empty(), "%s: outline must be empty", where);
    CHECK(d.slotList().empty(), "%s: slots must be empty (%zu)", where, d.slotList().size());
    CHECK(d.rawPointCount() == 0, "%s: rawPointCount %d", where, d.rawPointCount());
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    const QString out = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QDir::currentPath();

    // ---- 1·2·3: 90° 네 번 / 뒤집기 두 번 = 원래, 매 조작 뒤 접지·박스 중심 유지 ----
    {
        ArtifactDoc doc;
        doc.loadSample(false);
        doc.dropToFloor();
        CHECK(std::fabs(doc.mesh().bboxMin.z) <= kGeomTol, "drop: bboxMin.z %.3g", doc.mesh().bboxMin.z);
        const auto P0 = doc.mesh().vertices;
        double worstGround = 0.0, worstCenter = 0.0;
        auto step = [&](auto&& op) {
            const double bx = midX(doc.mesh()), by = midY(doc.mesh());
            op();
            worstGround = std::max(worstGround, std::fabs(doc.mesh().bboxMin.z));
            worstCenter = std::max({worstCenter, std::fabs(midX(doc.mesh()) - bx), std::fabs(midY(doc.mesh()) - by)});
        };
        for (PoseAxis a : {PoseAxis::X, PoseAxis::Y, PoseAxis::Z}) {
            for (double deg : {90.0, -90.0}) {
                for (int k = 0; k < 4; ++k) step([&] { doc.rotatePose(a, deg); });
                const double d = maxDelta(doc.mesh().vertices, P0);
                std::printf("rotate %s %+.0f x4: max|d|=%.3g mm\n", axisName(a), deg, d);
                CHECK(d <= kGeomTol, "%s %+.0f x4 max|d| %.3g", axisName(a), deg, d);
            }
        }
        for (int k = 0; k < 2; ++k) step([&] { doc.flipPose(); });
        const double df = maxDelta(doc.mesh().vertices, P0);
        std::printf("flip x2: max|d|=%.3g mm\n", df);
        CHECK(df <= kGeomTol, "flip x2 max|d| %.3g", df);
        std::printf("after each op: max|bboxMin.z|=%.3g mm, max|center xy shift|=%.3g mm\n", worstGround, worstCenter);
        CHECK(worstGround <= kGeomTol, "ground after op %.3g", worstGround);
        CHECK(worstCenter <= kGeomTol, "bbox center shift %.3g", worstCenter);
        CHECK(!doc.poseConfirmed(), "pose ops must leave pose unconfirmed");
    }

    // ---- 4: 처음 자세로 = 읽은 정점 그대로 (오차 0) ----
    {
        ArtifactDoc doc;
        doc.loadSample(false);
        const auto R0 = doc.mesh().vertices;
        doc.rotatePose(PoseAxis::X, 90);
        doc.rotatePose(PoseAxis::Y, -90);
        doc.flipPose();
        doc.dropToFloor();
        doc.rotatePose(PoseAxis::Y, -74);
        doc.resetPose();
        const double d = maxDelta(doc.mesh().vertices, R0);
        std::printf("reset: max|d|=%.3g mm\n", d);
        CHECK(d == 0.0, "reset must restore raw vertices exactly, max|d| %.3g", d);
        CHECK(!doc.poseConfirmed(), "reset must leave pose unconfirmed");
    }

    // ---- 5: 눕힌 항아리(Y −74°) → 자동 세우기 → 확정 = 예제(자동 세우기→확정)와 같은 치수 ----
    {
        ArtifactDoc ref;
        ref.loadSample();
        const Dims5 r = ref.dims();
        ArtifactDoc doc;
        doc.loadSample(false);
        doc.rotatePose(PoseAxis::Y, -74);
        doc.autoAlign();
        CHECK(!doc.poseConfirmed(), "autoAlign must not confirm");
        CHECK(doc.autoCentered(), "autoAlign should find the axis");
        doc.confirmPose();
        const Dims5 d = doc.dims();
        std::printf("ref:          H=%.2f rim=%.2f belly=%.2f base=%.2f\n", r.height, r.rim, r.belly, r.base);
        std::printf("laid Y-74 auto: H=%.2f rim=%.2f belly=%.2f base=%.2f note=%s\n", d.height, d.rim, d.belly, d.base,
                    doc.alignNote().toUtf8().constData());
        CHECK(d.ok, "laid vessel dims not ok");
        CHECK(std::fabs(d.height - r.height) <= 1.0, "H %.2f vs ref %.2f", d.height, r.height);
        CHECK(std::fabs(d.rim - r.rim) <= 1.0, "rim %.2f vs ref %.2f", d.rim, r.rim);
        CHECK(std::fabs(d.height - 184.5) <= 1.5, "H %.2f", d.height);
        CHECK(std::fabs(d.rim - 142.0) <= 2.0, "rim %.2f", d.rim);
        CHECK(doc.poseConfirmed(), "confirmPose must confirm");
    }

    // ---- 6: 확정 전 미계산, 확정 뒤 계산, 다시 돌리면 확정 풀림 ----
    {
        // (6a) 들어온 자세(7° 기울어짐) 그대로 사람 확정: 계산은 돌고(단면·접지) 회전은 하지 않는다.
        //      K-Arch AnalyzeDimensions 는 정위치 메시를 전제로 zMax 부근 구연 점 20개 이상을 요구하므로
        //      7° 기울어진 그대로면 구경 0 → dims().ok=false 가 정상 동작이다 (build-log 주의 사항 참고).
        ArtifactDoc doc;
        doc.loadSample(false);
        checkUnconfirmed(doc, "loadSample(false)");
        doc.confirmPose();
        std::printf("confirm as loaded (tilted 7deg): ok=%d H=%.2f rim=%.2f section=%zu bboxMin.z=%.3g\n",
                    int(doc.dims().ok), doc.dims().height, doc.dims().rim, doc.displaySection().size(),
                    doc.mesh().bboxMin.z);
        CHECK(doc.poseConfirmed(), "confirmPose must confirm");
        CHECK(doc.dims().height > 0.0, "dims must be computed after confirm");
        CHECK(!doc.displaySection().empty(), "section after confirm");
        CHECK(std::fabs(doc.mesh().bboxMin.z) <= kGeomTol, "confirm must ground, bboxMin.z %.3g", doc.mesh().bboxMin.z);
        doc.rotatePose(PoseAxis::X, 90);
        CHECK(!doc.poseConfirmed(), "rotate after confirm must unconfirm");
        CHECK(!doc.dims().ok, "rotate after confirm must clear dims");
        checkUnconfirmed(doc, "rotate after confirm");
    }
    {
        // (6b) 예제의 확정 경로(자동 세우기 → 확정): 자동 세우기만으로는 계산하지 않고, 확정 뒤 dims().ok
        ArtifactDoc doc;
        doc.loadSample(false);
        checkUnconfirmed(doc, "loadSample(false)");
        doc.autoAlign();
        checkUnconfirmed(doc, "autoAlign");
        doc.confirmPose();
        std::printf("auto -> confirm: ok=%d H=%.2f rim=%.2f section=%zu slots=%zu bboxMin.z=%.3g\n",
                    int(doc.dims().ok), doc.dims().height, doc.dims().rim, doc.displaySection().size(),
                    doc.slotList().size(), doc.mesh().bboxMin.z);
        CHECK(doc.poseConfirmed(), "confirmPose must confirm");
        CHECK(doc.dims().ok, "dims after confirm");
        CHECK(!doc.displaySection().empty(), "section after confirm");
        CHECK(!doc.slotList().empty(), "slots after confirm");
        CHECK(std::fabs(doc.mesh().bboxMin.z) <= kGeomTol, "confirm must ground, bboxMin.z %.3g", doc.mesh().bboxMin.z);
        doc.rotatePose(PoseAxis::X, 90);
        CHECK(!doc.poseConfirmed(), "rotate after confirm must unconfirm");
        CHECK(!doc.dims().ok, "rotate after confirm must clear dims");
    }

    // ---- 7: 파일 열기는 자세를 바꾸지 않는다 (기울기 7°, 이동 (12, −5, 40) mm) ----
    {
        const KA::MeshData m = vessel(7.0, {12.0, -5.0, 40.0}, 0.05);
        const QString path = out + "/studio_pose_tilted.ply";
        QString err;
        CHECK(exportLegacyPly(m, {}, {}, path, false, &err), "ply write %s", err.toUtf8().constData());
        const MeshReadResult rr = readMeshAny(path);
        CHECK(rr.ok, "readMeshAny %s", rr.message.toUtf8().constData());
        ArtifactDoc doc;
        CHECK(doc.loadFile(path, &err), "loadFile %s", err.toUtf8().constData());
        const auto &a = doc.mesh(), &b = rr.mesh;
        const double d = std::max({std::fabs(a.bboxMin.x - b.bboxMin.x), std::fabs(a.bboxMin.y - b.bboxMin.y),
                                   std::fabs(a.bboxMin.z - b.bboxMin.z), std::fabs(a.bboxMax.x - b.bboxMax.x),
                                   std::fabs(a.bboxMax.y - b.bboxMax.y), std::fabs(a.bboxMax.z - b.bboxMax.z)});
        std::printf("loadFile: bbox min (%.2f, %.2f, %.2f) max (%.2f, %.2f, %.2f) max|d| vs read=%.3g mm\n", a.bboxMin.x,
                    a.bboxMin.y, a.bboxMin.z, a.bboxMax.x, a.bboxMax.y, a.bboxMax.z, d);
        CHECK(d <= kGeomTol, "loadFile changed pose, max|d| %.3g", d);
        CHECK(a.bboxMin.z > 1.0, "loadFile must not ground, bboxMin.z %.3f", a.bboxMin.z);
        checkUnconfirmed(doc, "loadFile");
    }

    // ---- 8: 수동 확정 (자동 세우기 없이 곧게 선 항아리) ----
    {
        ArtifactDoc doc;
        doc.loadMesh(vessel(0.0, {12.0, -5.0, 3.0}, 0.05), {}, QStringLiteral("곧게 선 항아리"));
        doc.confirmPose();
        const Dims5 d = doc.dims();
        std::printf("manual confirm: H=%.2f rim=%.2f belly=%.2f base=%.2f note=%s\n", d.height, d.rim, d.belly, d.base,
                    doc.alignNote().toUtf8().constData());
        CHECK(std::fabs(doc.mesh().bboxMin.z) <= kGeomTol, "manual confirm ground %.3g", doc.mesh().bboxMin.z);
        CHECK(d.ok, "manual confirm dims not ok");
        CHECK(std::fabs(d.height - 184.5) <= 1.5, "H %.2f", d.height);
        CHECK(std::fabs(d.rim - 142.0) <= 2.0, "rim %.2f", d.rim);
        CHECK(!doc.displaySection().empty(), "manual confirm section empty");
        CHECK(doc.aligned() && doc.axisRmse() < 0, "manual confirm should center by circles (aligned=%d rmse=%.3f)",
              int(doc.aligned()), doc.axisRmse());
    }

    std::puts(fails ? "FAILED" : "PASS");
    return fails ? 1 : 0;
}
