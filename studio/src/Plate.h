// DongArch3D Studio — 실측 도판 그리기(R5)와 내보내기(R6) 공통 엔진.
// 화면 도판, 내보내기 미리보기, PDF/PNG/SVG/DXF 가 모두 이 한 곳의 그리기 순서를 쓴다.
#pragma once
#include "ArtifactDoc.h"
#include "MeshRaster.h"

#include <QImage>
#include <QPainter>
#include <QRectF>
#include <QString>

namespace studio {

struct PlateOptions {
    bool grid = true;     // 방안 눈금·치수·축척 막대
    bool shade = true;    // 유물 음영 / 디지털 탁본
    bool outline = true;  // 외곽선·중심축
    bool section = true;  // 단면선 (빨강)
    bool hatch = false;   // 단면 빗금 (보고서 양식 선택)
    bool labels = true;   // 머리 치수(구경·기고) 표기
    ShadeMode shadeMode = ShadeMode::DTak;
    int slot = 0;         // 슬롯 번호 (수평 슬롯이면 평면 단면)
};

struct ShadeImage {
    QImage image;
    QRectF rectMm;  // 도면 좌표 (x=R, y=H 위로 +)
    bool isNull() const { return image.isNull(); }
};

class Plate {
public:
    // 도면 내용이 차지하는 영역 (mm, y 위로 +), 여백 포함
    static QRectF extentMm(const ArtifactDoc& doc, int slot);
    // 외면 음영 영상 (전체 폭, 그릴 때 외면 쪽 절반만 쓴다). D5 기본(Right): yaw 0° 정면 —
    // 영상 왼쪽 절반 = x<0 외면, Left(R 반전): yaw 180° 뒤 — 영상 오른쪽 절반이 외면
    static ShadeImage renderShade(const ArtifactDoc& doc, ShadeMode mode, double pxPerMm);
    // mmToDev: 도면 mm(y 위로 +) → 장치 좌표. px1mm: 장치에서 도면 1 mm 의 길이, textPx: 글자 크기(장치 px).
    static void paint(QPainter& p, const ArtifactDoc& doc, const QTransform& mmToDev, double px1mm, double textPx,
                      const PlateOptions& o, const ShadeImage* shade);
};

// ---- 용지 배치 (R6) -------------------------------------------------------
enum class Paper { A4Portrait, A4Landscape, A3Portrait };
struct PaperLayout {
    QSizeF paperMm;
    QRectF drawAreaMm;   // 용지 좌표 (y 아래로 +)
    QRectF plateRectMm;  // 축척 적용 후 도면이 놓이는 곳
    QRectF titleRectMm;
    double scale = 1.0;  // 2:1 → 2, 1:2 → 0.5
    bool fits = true;
    QString advice;      // 맞춤 안내 문장
};
QSizeF paperSize(Paper p);
QString paperName(Paper p);
QString scaleName(double s);
PaperLayout layoutPaper(const ArtifactDoc& doc, int slot, Paper paper, double scale);

struct ExportRequest {
    QString path;
    QString title;  // 도면명
    Paper paper = Paper::A4Portrait;
    double scale = 1.0;
    PlateOptions opts;
};
// 용지 한 장(표제란·축척 막대 포함)을 QPainter 로 그린다. 장치 단위는 pxPerMm.
void paintSheet(QPainter& p, const ArtifactDoc& doc, const ExportRequest& r, double pxPerMm);

bool exportSvgLayered(const ArtifactDoc& doc, const ExportRequest& r, QString* err);
bool exportPdf(const ArtifactDoc& doc, const ExportRequest& r, QString* err);
bool exportPng(const ArtifactDoc& doc, const ExportRequest& r, int dpi, QString* err);
bool exportDxf(const ArtifactDoc& doc, const ExportRequest& r, QString* err);

}  // namespace studio
