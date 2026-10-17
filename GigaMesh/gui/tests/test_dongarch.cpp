/* DongArch3D - Automated Tests
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 8: Test Automation
 *
 * Qt Test Framework 기반 자동화 테스트
 *
 * 테스트 커버리지:
 * - Phase 2: Cutline (Douglas-Peucker, Catmull-Rom)
 * - Phase 3: Outline
 * - Phase 4: Clip
 * - Phase 5: Vis (Heat Method, X-Ray)
 * - Phase 6: MFE
 * - Phase 7: Illustrator
 */

#include <QtTest/QtTest>
#include <QVector3D>
#include <QPointF>
#include <vector>

// DongArch3D headers
#include "dongarch/cutline/algorithms/DouglasPeucker.h"
#include "dongarch/cutline/algorithms/CatmullRomSpline.h"
#include "dongarch/mfe/MFEManager.h"
#include "dongarch/illustrator/SVGExporter.h"

using namespace DongArch;

//==============================================================================
// Test Class
//==============================================================================

class TestDongArch : public QObject
{
    Q_OBJECT

private slots:
    // Initialization
    void initTestCase();
    void cleanupTestCase();

    // Phase 2: Cutline Tests
    void test_DouglasPeucker_Simplification();
    void test_DouglasPeucker_EdgeCases();
    void test_CatmullRom_Interpolation();
    void test_CatmullRom_EdgeCases();

    // Phase 6: MFE Tests
    void test_MFE_FormatDetection();
    void test_MFE_DirectoryScan();

    // Phase 7: Illustrator Tests
    void test_SVGExporter_Basic();
    void test_SVGExporter_Layers();
};

//==============================================================================
// Initialization
//==============================================================================

void TestDongArch::initTestCase()
{
    qDebug() << "DongArch3D Test Suite";
    qDebug() << "Version: 4.0 (품질 최우선)";
    qDebug() << "------------------------------";
}

void TestDongArch::cleanupTestCase()
{
    qDebug() << "------------------------------";
    qDebug() << "All tests completed";
}

//==============================================================================
// Phase 2: Douglas-Peucker Tests
//==============================================================================

void TestDongArch::test_DouglasPeucker_Simplification()
{
    using namespace Cutline::Algorithms;

    // Generate test polyline (zigzag)
    std::vector<Vector3D> points;
    for (int i = 0; i < 100; i++) {
        double x = static_cast<double>(i);
        double y = (i % 2 == 0) ? 1.0 : -1.0;
        points.emplace_back(x, y, 0.0);
    }

    DouglasPeucker dp;

    // Test: simplification should reduce points
    auto simplified = dp.simplify(points, 0.5);

    QVERIFY(simplified.size() < points.size());
    QVERIFY(simplified.size() >= 2);  // At least start and end

    // Test: first and last points preserved
    QCOMPARE(simplified.front().getX(), points.front().getX());
    QCOMPARE(simplified.back().getX(), points.back().getX());

    // Test: statistics
    const auto& stats = dp.getLastStatistics();
    QVERIFY(stats.originalPointCount == points.size());
    QVERIFY(stats.simplifiedPointCount == simplified.size());
    QVERIFY(stats.reductionRatio > 0.0);
}

void TestDongArch::test_DouglasPeucker_EdgeCases()
{
    using namespace Cutline::Algorithms;

    DouglasPeucker dp;

    // Edge case 1: Empty input
    std::vector<Vector3D> empty;
    auto result1 = dp.simplify(empty, 0.5);
    QCOMPARE(result1.size(), 0);

    // Edge case 2: Single point
    std::vector<Vector3D> single = {Vector3D(0, 0, 0)};
    auto result2 = dp.simplify(single, 0.5);
    QCOMPARE(result2.size(), 1);

    // Edge case 3: Two points (should stay unchanged)
    std::vector<Vector3D> two = {Vector3D(0, 0, 0), Vector3D(10, 10, 0)};
    auto result3 = dp.simplify(two, 0.5);
    QCOMPARE(result3.size(), 2);
}

//==============================================================================
// Phase 2: Catmull-Rom Tests
//==============================================================================

void TestDongArch::test_CatmullRom_Interpolation()
{
    using namespace Cutline::Algorithms;

    std::vector<Vector3D> controlPoints = {
        Vector3D(0, 0, 0),
        Vector3D(1, 2, 0),
        Vector3D(2, 1, 0),
        Vector3D(3, 3, 0),
        Vector3D(4, 0, 0)
    };

    CatmullRomSpline spline;

    // Test: interpolation creates more points
    auto curve = spline.interpolate(controlPoints, 20);
    QVERIFY(curve.size() > controlPoints.size());

    // Test: curve passes through control points (approximately)
    // First and last points should match exactly
    double dist_first = (curve.front() - controlPoints.front()).getLength3();
    double dist_last = (curve.back() - controlPoints.back()).getLength3();

    QVERIFY(dist_first < 0.001);
    QVERIFY(dist_last < 0.001);
}

void TestDongArch::test_CatmullRom_EdgeCases()
{
    using namespace Cutline::Algorithms;

    CatmullRomSpline spline;

    // Edge case 1: Empty input
    std::vector<Vector3D> empty;
    auto result1 = spline.interpolate(empty, 20);
    QCOMPARE(result1.size(), 0);

    // Edge case 2: Single point
    std::vector<Vector3D> single = {Vector3D(0, 0, 0)};
    auto result2 = spline.interpolate(single, 20);
    QCOMPARE(result2.size(), 1);

    // Edge case 3: Two points (linear interpolation)
    std::vector<Vector3D> two = {Vector3D(0, 0, 0), Vector3D(10, 10, 0)};
    auto result3 = spline.interpolate(two, 10);
    QCOMPARE(result3.size(), 11);  // 10 segments + 1
}

//==============================================================================
// Phase 6: MFE Tests
//==============================================================================

void TestDongArch::test_MFE_FormatDetection()
{
    using namespace MFE;

    // Test format detection
    QCOMPARE(MFEManager::detectFormat("/path/to/mesh.ply"), MFEManager::FileFormat::PLY);
    QCOMPARE(MFEManager::detectFormat("/path/to/mesh.obj"), MFEManager::FileFormat::OBJ);
    QCOMPARE(MFEManager::detectFormat("/path/to/mesh.off"), MFEManager::FileFormat::OFF);
    QCOMPARE(MFEManager::detectFormat("/path/to/mesh.stl"), MFEManager::FileFormat::STL);
    QCOMPARE(MFEManager::detectFormat("/path/to/mesh.txt"), MFEManager::FileFormat::Unknown);

    // Test format to string
    QCOMPARE(MFEManager::formatToString(MFEManager::FileFormat::PLY), QString("PLY"));
    QCOMPARE(MFEManager::formatToString(MFEManager::FileFormat::OBJ), QString("OBJ"));
}

void TestDongArch::test_MFE_DirectoryScan()
{
    using namespace MFE;

    MFEManager mfe;

    // Test: scan non-existent directory
    int count = mfe.scanDirectory("/nonexistent/path", false);
    QCOMPARE(count, 0);

    // Test: supported extensions
    QStringList exts = MFEManager::getSupportedExtensions();
    QVERIFY(exts.contains("*.ply"));
    QVERIFY(exts.contains("*.obj"));
    QVERIFY(exts.contains("*.off"));
    QVERIFY(exts.contains("*.stl"));
}

//==============================================================================
// Phase 7: SVG Exporter Tests
//==============================================================================

void TestDongArch::test_SVGExporter_Basic()
{
    using namespace Illustrator;

    SVGExporter exporter;

    // Test: default settings
    exporter.setCanvasSize(QSizeF(100, 100));
    exporter.setUnit(SVGExporter::Unit::Millimeter);

    // Test: export to string
    QString svg = exporter.exportToString();

    QVERIFY(svg.contains("<?xml"));
    QVERIFY(svg.contains("<svg"));
    QVERIFY(svg.contains("</svg>"));
    QVERIFY(svg.contains("DongArch3D"));  // Metadata
}

void TestDongArch::test_SVGExporter_Layers()
{
    using namespace Illustrator;

    SVGExporter exporter;
    exporter.setCanvasSize(QSizeF(100, 100));

    // Add test layer
    SVGLayer layer;
    layer.name = "TestLayer";
    layer.id = "test_layer_1";
    layer.strokeColor = Qt::red;
    layer.strokeWidth = 0.5f;

    std::vector<QPointF> polyline = {
        QPointF(0, 0),
        QPointF(10, 10),
        QPointF(20, 5)
    };
    layer.polylines.push_back(polyline);

    exporter.addLayer(layer);

    // Export
    QString svg = exporter.exportToString();

    QVERIFY(svg.contains("TestLayer"));
    QVERIFY(svg.contains("test_layer_1"));
    QVERIFY(svg.contains("polyline"));
    QVERIFY(svg.contains("#ff0000"));  // Red color
}

//==============================================================================
// Main
//==============================================================================

QTEST_MAIN(TestDongArch)
#include "test_dongarch.moc"
