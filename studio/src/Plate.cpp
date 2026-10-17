#include "Plate.h"

#include "DxfWriter.h"
#include "Theme.h"

#include <QBuffer>
#include <QFile>
#include <QPainterPath>
#include <QPdfWriter>
#include <QTextStream>
#include <algorithm>
#include <cmath>

namespace studio {

namespace {

constexpr double kSvgSection = 0.35, kSvgOutline = 0.50;

double maxR(const ArtifactDoc &doc) {
  double r = 10.0;
  for (const auto &q : doc.displayOutline())
    r = std::max(r, std::fabs(q.x));
  for (const auto &poly : doc.displaySection())
    for (const auto &q : poly)
      r = std::max(r, std::fabs(q.x));
  return r;
}

double height(const ArtifactDoc &doc) {
  return std::max(1.0, doc.mesh().bboxMax.z - doc.mesh().bboxMin.z);
}

bool isHorizontal(const ArtifactDoc &doc, int slot) {
  return slot >= 0 && slot < int(doc.slotList().size()) &&
         doc.slotList()[size_t(slot)].kind == SlotKind::Horizontal;
}

QString num(double v, int prec = 1) { return QString::number(v, 'f', prec); }

// 막을 움직이는 중(LOD 미리보기 단면 · 치수·두께는 옛 각)에는 파일을 만들기 전에 거부한다
bool refusePreview(const ArtifactDoc &doc, QString *err) {
  if (!doc.isPreviewing())
    return false;
  if (err)
    *err = QStringLiteral(
        "막을 움직이는 중입니다 — 놓은 뒤(원본 단면 계산 뒤) 내보내세요.");
  return true;
}

QPolygonF mapPoly(const std::vector<KA::Point2> &pts, const QTransform &t) {
  QPolygonF poly;
  poly.reserve(qsizetype(pts.size()));
  for (const auto &q : pts)
    poly << t.map(QPointF(q.x, q.y));
  return poly;
}

void labelBox(QPainter &p, QPointF anchor, const QString &text, const QFont &f,
              Qt::Alignment al) {
  p.setFont(f);
  const QFontMetricsF fm(f);
  const double pad = fm.height() * 0.3;
  QRectF r(0, 0, fm.horizontalAdvance(text) + 2 * pad, fm.height() + pad);
  if (al & Qt::AlignHCenter)
    r.moveCenter(QPointF(anchor.x(), r.center().y()));
  else if (al & Qt::AlignRight)
    r.moveRight(anchor.x());
  else
    r.moveLeft(anchor.x());
  if (al & Qt::AlignVCenter)
    r.moveTop(anchor.y() - r.height() / 2);
  else if (al & Qt::AlignBottom)
    r.moveBottom(anchor.y());
  else
    r.moveTop(anchor.y());
  p.setPen(QPen(theme::Edge, 1));
  p.setBrush(theme::Surface);
  p.drawRect(r);
  p.setPen(theme::Ink);
  p.drawText(r, Qt::AlignCenter, text);
}

void scaleBar(QPainter &p, QPointF leftBottom, double px1mm, double lenMm,
              const QFont &f, const QString &caption) {
  const double w = lenMm * px1mm, h = std::max(3.0, 0.8 * f.pixelSize() * 0.5);
  p.setPen(QPen(theme::Ink, 1));
  for (int i = 0; i < 2; ++i) {
    p.setBrush(i == 0 ? theme::Ink : theme::Surface);
    p.drawRect(
        QRectF(leftBottom.x() + i * w / 2, leftBottom.y() - h, w / 2, h));
  }
  p.setFont(f);
  const QFontMetricsF fm(f);
  p.drawText(QPointF(leftBottom.x() - fm.horizontalAdvance("0") / 2,
                     leftBottom.y() + fm.ascent() + 2),
             "0");
  const QString t = caption.isEmpty()
                        ? QStringLiteral("%1 mm").arg(lenMm, 0, 'f', 0)
                        : caption;
  p.drawText(QPointF(leftBottom.x() + w - fm.horizontalAdvance(t) / 2,
                     leftBottom.y() + fm.ascent() + 2),
             t);
}

} // namespace

QRectF Plate::extentMm(const ArtifactDoc &doc, int slot) {
  if (!doc.isLoaded())
    return QRectF(-60, -10, 120, 140);
  if (isHorizontal(doc, slot)) {
    const auto &m = doc.mesh();
    const double r =
        std::max({std::fabs(m.bboxMin.x), std::fabs(m.bboxMax.x),
                  std::fabs(m.bboxMin.y), std::fabs(m.bboxMax.y)}) +
        10.0;
    return QRectF(-r, -r, 2 * r, 2 * r);
  }
  const double r = maxR(doc) + 8.0, h = height(doc);
  return QRectF(-r, -8.0, 2 * r, h + 26.0);
}

ShadeImage Plate::renderShade(const ArtifactDoc &doc, ShadeMode mode,
                              double pxPerMm) {
  ShadeImage s;
  if (!doc.isLoaded())
    return s;
  const double r = maxR(doc) + 2.0, h = height(doc) + 4.0;
  pxPerMm = std::clamp(pxPerMm, 0.5, std::min(4000.0 / (2 * r), 4000.0 / h));
  RasterView v;
  // D5: Right(기본) = 정면(카메라 −Y, yaw 0°)에서 본 x<0 반쪽 → 영상 왼쪽 절반이
  // 외면. Left(R 반전) = 뒤(yaw 180°)에서 본 모양 → 영상 오른쪽 절반이 외면.
  v.yawDeg = doc.sectionSide() == SectionSide::Right ? 0.0 : 180.0;
  v.pitchDeg = 0.0;
  v.pxPerMm = pxPerMm;
  const QSize sz(int(std::ceil(2 * r * pxPerMm)), int(std::ceil(h * pxPerMm)));
  v.centerPx = QPointF(sz.width() / 2.0, sz.height() / 2.0);
  v.centerZ = (h - 4.0) / 2.0 + doc.mesh().bboxMin.z;
  s.image = renderMesh(doc.mesh(), sz, v, mode, theme::ModelClay, nullptr,
                       doc.hasColor() ? &doc.vertexColors() : nullptr);
  s.rectMm = QRectF(-r, v.centerZ - h / 2.0, 2 * r, h);
  return s;
}

void Plate::paint(QPainter &p, const ArtifactDoc &doc, const QTransform &t,
                  double px1mm, double textPx, const PlateOptions &o,
                  const ShadeImage *shade) {
  if (!doc.isLoaded())
    return;
  p.save();
  p.setRenderHint(QPainter::Antialiasing, true);
  const QRectF E = extentMm(doc, o.slot);
  const QRectF Edev = t.mapRect(E);
  const QFont mono = theme::monoFont(int(std::lround(std::max(8.0, textPx))));
  const QFont sans = theme::uiFont(int(std::lround(std::max(8.0, textPx))));
  const bool horiz = isHorizontal(doc, o.slot);
  const double H = height(doc);
  const double lineSec = std::max(1.5, 0.35 * px1mm),
               lineOut = std::max(1.2, 0.30 * px1mm);

  // 1) 방안 눈금 ------------------------------------------------------------
  if (o.grid) {
    p.setClipRect(Edev.adjusted(-1, -1, 1, 1));
    const bool fine = px1mm >= 2.5;
    for (int pass = 0; pass < 2; ++pass) {
      const int step = pass == 0 ? 1 : 10;
      if (pass == 0 && !fine)
        continue;
      p.setPen(QPen(pass == 0 ? theme::Grid1mm : theme::Grid10mm,
                    pass == 0 ? 0.75 : 1.0));
      for (int x = int(std::ceil(E.left())); x <= int(std::floor(E.right()));
           ++x) {
        if (x % step)
          continue;
        if (pass == 0 && x % 10 == 0)
          continue;
        p.drawLine(t.map(QPointF(x, E.top())), t.map(QPointF(x, E.bottom())));
      }
      for (int y = int(std::ceil(E.top())); y <= int(std::floor(E.bottom()));
           ++y) {
        if (y % step)
          continue;
        if (pass == 0 && y % 10 == 0)
          continue;
        p.drawLine(t.map(QPointF(E.left(), y)), t.map(QPointF(E.right(), y)));
      }
    }
    p.setClipping(false);
    p.setPen(QPen(theme::Edge, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(Edev);
    // 50 mm 숫자 (모노)
    p.setFont(mono);
    p.setPen(theme::Ink2);
    const QFontMetricsF fm(mono);
    const bool rimRef = doc.heightRef() == HeightRef::Rim;
    if (!horiz) {
      for (int y = 0; y <= int(E.bottom()); y += 50) {
        const QPointF a = t.map(QPointF(E.left(), y)),
                      b = t.map(QPointF(E.right(), y));
        const QString s = QString::number(rimRef ? y - std::lround(H) : y);
        p.drawText(QPointF(a.x() - fm.horizontalAdvance(s) - 6,
                           a.y() + fm.ascent() / 2 - 1),
                   s);
        p.drawText(QPointF(b.x() + 6, b.y() + fm.ascent() / 2 - 1),
                   y == 0 ? (rimRef ? s + QStringLiteral(" mm")
                                    : QStringLiteral("0 mm (바닥)"))
                          : s);
      }
      for (int x = 0; x <= int(E.right()); x += 50) {
        for (int sgn : {-1, 1}) {
          if (x == 0 && sgn < 0)
            continue;
          const QPointF a = t.map(QPointF(sgn * x, E.top()));
          const QString s =
              x == 0 ? QStringLiteral("0 (중심축)") : QString::number(x);
          p.drawText(QPointF(a.x() - fm.horizontalAdvance(s) / 2,
                             a.y() + fm.ascent() + 6),
                     s);
        }
      }
      // 바닥선 H 0
      p.setPen(QPen(theme::Ink, 1));
      p.drawLine(t.map(QPointF(E.left(), 0)), t.map(QPointF(E.right(), 0)));
    } else {
      p.setPen(QPen(theme::Ink2, 1, Qt::DashDotLine));
      p.drawLine(t.map(QPointF(E.left(), 0)), t.map(QPointF(E.right(), 0)));
      p.drawLine(t.map(QPointF(0, E.top())), t.map(QPointF(0, E.bottom())));
    }
  }

  if (horiz) {
    // 수평 슬롯: 평면 단면 ------------------------------------------------
    const auto &slot = doc.slotList()[size_t(o.slot)];
    double rOut = 0.0;
    QPainterPath path;
    for (const auto &pl : doc.horizontalSlice(slot.z)) {
      if (pl.pts.size() < 2)
        continue;
      QPolygonF poly;
      for (const auto &q : pl.pts) {
        poly << t.map(QPointF(q.x, q.y));
        rOut = std::max(rOut, std::hypot(q.x, q.y));
      }
      path.addPolygon(poly);
      path.closeSubpath();
    }
    if (o.section) {
      path.setFillRule(Qt::OddEvenFill);
      p.setPen(QPen(theme::ProfileRed, lineSec));
      p.setBrush(theme::ProfileFill);
      p.drawPath(path);
    }
    if (o.labels) {
      labelBox(p, t.map(QPointF(E.left() + 3, E.bottom() - 3)),
               QStringLiteral("H %1 mm 수평 절단 · 외경 ⌀ %2 mm")
                   .arg(num(slot.z), num(2 * rOut)),
               mono, Qt::AlignLeft | Qt::AlignTop);
    }
    p.restore();
    return;
  }

  // 2) 유물 음영 (외면 쪽 절반: Right 왼쪽 x ≤ 0, Left 오른쪽 x ≥ 0) ---------
  if (o.shade && shade && !shade->isNull()) {
    const QRectF dst = t.mapRect(shade->rectMm);
    const QRectF &sr = shade->rectMm;
    const QRectF half = t.mapRect(
        doc.sectionSide() == SectionSide::Right
            ? QRectF(sr.left(), sr.top(), -sr.left(), sr.height())
            : QRectF(0, sr.top(), sr.right(), sr.height()));
    p.save();
    p.setClipRect(half);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.drawImage(dst, shade->image);
    p.restore();
  }

  // 3) 외곽선·중심축
  // -----------------------------------------------------------
  const auto outline = doc.displayOutline();
  if (o.outline) {
    p.setPen(QPen(theme::OuterInk, 1));
    p.drawLine(t.map(QPointF(0, -4)), t.map(QPointF(0, H + 6)));
    if (outline.size() >= 2) {
      p.setPen(QPen(theme::OuterInk, lineOut, Qt::SolidLine, Qt::RoundCap,
                    Qt::RoundJoin));
      p.setBrush(Qt::NoBrush);
      p.drawPolyline(mapPoly(outline, t));
      const auto &b = outline.front(), &e = outline.back();
      p.drawLine(t.map(QPointF(0, b.y)), t.map(QPointF(b.x, b.y)));
      p.drawLine(t.map(QPointF(0, e.y)), t.map(QPointF(e.x, e.y)));
    }
  }

  // 4) 단면선 (맨 위, 순수 빨강) ---------------------------------------------
  if (o.section) {
    if (doc.hasEstimatedInner()) {
      p.setPen(Qt::NoPen);
      p.setBrush(theme::ProfileFill);
      for (const auto &poly : doc.displaySection()) {
        if (poly.size() < 3)
          continue;
        p.drawPolygon(mapPoly(poly, t));
      }
      p.setBrush(Qt::NoBrush);
      p.setPen(QPen(theme::ProfileRed, lineSec, Qt::SolidLine, Qt::RoundCap,
                    Qt::RoundJoin));
      for (const auto &solid : doc.displayScannedSolid()) {
        if (solid.size() < 2)
          continue;
        p.drawPolyline(mapPoly(solid, t));
      }
      QPen dashPen(theme::ProfileRed, lineSec, Qt::CustomDashLine, Qt::RoundCap,
                   Qt::RoundJoin);
      dashPen.setDashPattern({3.2, 2.2});
      p.setPen(dashPen);
      for (const auto &est : doc.displayEstimatedInner()) {
        if (est.size() < 2)
          continue;
        p.drawPolyline(mapPoly(est, t));
      }
    } else {
      for (const auto &poly : doc.displaySection()) {
        if (poly.size() < 3)
          continue;
        p.setPen(QPen(theme::ProfileRed, lineSec, Qt::SolidLine, Qt::RoundCap,
                      Qt::RoundJoin));
        p.setBrush(theme::ProfileFill);
        p.drawPolygon(mapPoly(poly, t));
      }
    }
    if (o.hatch) {
      p.setPen(QPen(theme::ProfileRed, std::max(0.8, 0.13 * px1mm)));
      for (const auto &[a, b] : doc.displayHatch())
        p.drawLine(t.map(QPointF(a.x, a.y)), t.map(QPointF(b.x, b.y)));
    }
  }

  // 5) 머리 치수 · 축척 막대 -------------------------------------------------
  if (o.labels && doc.dims().ok) {
    const auto &d = doc.dims();
    const double yDim = H + 8.0;
    p.setPen(QPen(theme::Ink, 1));
    const QPointF a = t.map(QPointF(-d.rim / 2, yDim)),
                  b = t.map(QPointF(d.rim / 2, yDim));
    p.drawLine(a, b);
    p.drawLine(a + QPointF(0, -4), a + QPointF(0, 4));
    p.drawLine(b + QPointF(0, -4), b + QPointF(0, 4));
    labelBox(p, (a + b) / 2, QStringLiteral("구경 ⌀ %1 mm").arg(num(d.rim)),
             mono, Qt::AlignHCenter | Qt::AlignVCenter);
    p.setPen(QPen(theme::Caution, 1, Qt::DashLine));
    const QPointF h0 = t.map(QPointF(E.left() + 2, H)),
                  h1 = t.map(QPointF(0, H));
    p.drawLine(h0, h1);
    labelBox(p, h0 + QPointF(4, 0),
             QStringLiteral("기고 H %1 mm").arg(num(d.height)), mono,
             Qt::AlignLeft | Qt::AlignVCenter);
  }
  if (o.grid) {
    const double len = px1mm * 50 > Edev.width() * 0.3 ? 10.0 : 50.0;
    const QPointF lb = t.map(QPointF(E.right() - len - 6, E.top() + 4));
    scaleBar(p, lb, px1mm, len, mono, QString());
  }
  p.restore();
}

// ---- 용지 ----------------------------------------------------------------
QSizeF paperSize(Paper p) {
  switch (p) {
  case Paper::A4Landscape:
    return {297, 210};
  case Paper::A3Portrait:
    return {297, 420};
  default:
    return {210, 297};
  }
}
QString paperName(Paper p) {
  switch (p) {
  case Paper::A4Landscape:
    return QStringLiteral("A4 가로");
  case Paper::A3Portrait:
    return QStringLiteral("A3 세로");
  default:
    return QStringLiteral("A4 세로");
  }
}
QString scaleName(double s) {
  if (s >= 1.5)
    return QStringLiteral("2:1");
  if (s >= 0.99)
    return QStringLiteral("1:1");
  return QStringLiteral("1:%1").arg(std::lround(1.0 / s));
}

PaperLayout layoutPaper(const ArtifactDoc &doc, int slot, Paper paper,
                        double scale) {
  PaperLayout L;
  L.paperMm = paperSize(paper);
  L.scale = scale;
  const double m = 10.0, titleH = 24.0, gap = 4.0;
  L.drawAreaMm = QRectF(m, m, L.paperMm.width() - 2 * m,
                        L.paperMm.height() - 2 * m - titleH - gap);
  L.titleRectMm = QRectF(m, L.paperMm.height() - m - titleH,
                         L.paperMm.width() - 2 * m, titleH);
  const QRectF E = Plate::extentMm(doc, slot);
  const QSizeF ps(E.width() * scale, E.height() * scale);
  L.plateRectMm = QRectF(QPointF(0, 0), ps);
  L.plateRectMm.moveCenter(L.drawAreaMm.center());
  L.fits = ps.width() <= L.drawAreaMm.width() + 1e-6 &&
           ps.height() <= L.drawAreaMm.height() + 1e-6;
  const auto &d = doc.dims();
  const QString size =
      QStringLiteral("%1 × %2 mm").arg(num(d.belly), num(d.height));
  if (L.fits) {
    L.advice =
        QStringLiteral(
            "✓ %1 축척에서 유물 크기(%2)가 %3 한 장에 여유 있게 들어갑니다.")
            .arg(scaleName(scale), size, paperName(paper));
  } else {
    QString rec;
    for (double s : {1.0, 0.5, 1.0 / 3.0, 0.25}) {
      if (s >= scale)
        continue;
      if (E.width() * s <= L.drawAreaMm.width() &&
          E.height() * s <= L.drawAreaMm.height()) {
        rec = scaleName(s);
        break;
      }
    }
    L.advice = QStringLiteral("▲ %1에서 유물 높이 %2 mm는 %3 한 장을 넘습니다. "
                              "추천: %4로 줄이거나 A3 세로 선택")
                   .arg(scaleName(scale), num(d.height), paperName(paper),
                        rec.isEmpty() ? QStringLiteral("1:4") : rec);
  }
  return L;
}

void paintSheet(QPainter &p, const ArtifactDoc &doc, const ExportRequest &r,
                double ppm) {
  const PaperLayout L = layoutPaper(doc, r.opts.slot, r.paper, r.scale);
  p.save();
  p.setRenderHint(QPainter::Antialiasing, true);
  p.fillRect(QRectF(0, 0, L.paperMm.width() * ppm, L.paperMm.height() * ppm),
             Qt::white);
  const QRectF E = Plate::extentMm(doc, r.opts.slot);
  QTransform t;
  // x_dev = (plate.left + (x - E.left) * s) * ppm ; y_dev = (plate.top +
  // (E.bottom - y) * s) * ppm
  t.setMatrix(r.scale * ppm, 0, 0, 0, -r.scale * ppm, 0,
              (L.plateRectMm.left() - E.left() * r.scale) * ppm,
              (L.plateRectMm.top() + E.bottom() * r.scale) * ppm, 1);
  ShadeImage shade;
  if (r.opts.shade)
    shade = Plate::renderShade(doc, r.opts.shadeMode,
                               std::min(12.0, r.scale * ppm));
  PlateOptions o = r.opts;
  Plate::paint(p, doc, t, r.scale * ppm, 2.6 * ppm, o, &shade);

  // 표제란
  const QRectF tr(L.titleRectMm.left() * ppm, L.titleRectMm.top() * ppm,
                  L.titleRectMm.width() * ppm, L.titleRectMm.height() * ppm);
  p.setPen(QPen(theme::Ink, std::max(1.0, 0.3 * ppm)));
  p.drawLine(tr.topLeft(), tr.topRight());
  const QFont small = theme::uiFont(int(2.6 * ppm)),
              mono = theme::monoFont(int(2.4 * ppm));
  p.setFont(small);
  p.setPen(theme::Ink);
  p.drawText(QPointF(tr.left(), tr.top() + 7 * ppm),
             QStringLiteral("축척 %1%2 · 단위 mm")
                 .arg(scaleName(r.scale), r.scale == 1.0
                                              ? QStringLiteral(" (실제 크기)")
                                              : QString()));
  scaleBar(p, QPointF(tr.left(), tr.top() + 15 * ppm), r.scale * ppm, 50.0,
           mono, QStringLiteral("50 mm"));
  const QRectF box(tr.right() - 92 * ppm, tr.top() + 3 * ppm, 92 * ppm,
                   18 * ppm);
  p.setBrush(Qt::NoBrush);
  p.setPen(QPen(theme::Ink, std::max(1.0, 0.25 * ppm)));
  p.drawRect(box);
  p.setFont(theme::uiFont(int(3.0 * ppm), true));
  p.drawText(box.adjusted(3 * ppm, 2 * ppm, -3 * ppm, 0),
             Qt::AlignLeft | Qt::AlignTop, r.title);
  const auto &d = doc.dims();
  p.setFont(mono);
  p.drawText(box.adjusted(3 * ppm, 0, -3 * ppm, -2.5 * ppm),
             Qt::AlignLeft | Qt::AlignBottom,
             QStringLiteral("기고 %1 · 구경 %2 · 저경 %3 mm")
                 .arg(num(d.height), num(d.rim), num(d.base)));
  p.restore();
}

bool exportPdf(const ArtifactDoc &doc, const ExportRequest &r, QString *err) {
  if (refusePreview(doc, err))
    return false;
  QPdfWriter w(r.path);
  const QSizeF ps = paperSize(r.paper);
  w.setPageSize(QPageSize(ps, QPageSize::Millimeter));
  w.setPageMargins(QMarginsF(0, 0, 0, 0));
  w.setResolution(600);
  w.setTitle(r.title);
  QPainter p;
  if (!p.begin(&w)) {
    if (err)
      *err = QStringLiteral("PDF 파일을 만들 수 없습니다.");
    return false;
  }
  paintSheet(p, doc, r, 600.0 / 25.4);
  p.end();
  return true;
}

bool exportPng(const ArtifactDoc &doc, const ExportRequest &r, int dpi,
               QString *err) {
  if (refusePreview(doc, err))
    return false;
  const double ppm = dpi / 25.4;
  const QSizeF ps = paperSize(r.paper);
  QImage img(int(ps.width() * ppm), int(ps.height() * ppm),
             QImage::Format_RGB32);
  img.setDotsPerMeterX(int(ppm * 1000));
  img.setDotsPerMeterY(int(ppm * 1000));
  QPainter p(&img);
  paintSheet(p, doc, r, ppm);
  p.end();
  if (!img.save(r.path, "PNG")) {
    if (err)
      *err = QStringLiteral("PNG 저장 실패");
    return false;
  }
  return true;
}

// ---- 레이어 분리 SVG (일러스트레이터 1:1 mm) --------------------------------
bool exportSvgLayered(const ArtifactDoc &doc, const ExportRequest &r,
                      QString *err) {
  if (refusePreview(doc, err))
    return false;
  QFile f(r.path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    if (err)
      *err = QStringLiteral("SVG 파일을 만들 수 없습니다.");
    return false;
  }
  const PaperLayout L = layoutPaper(doc, r.opts.slot, r.paper, r.scale);
  const QRectF E = Plate::extentMm(doc, r.opts.slot);
  const double s = r.scale;
  auto X = [&](double x) { return L.plateRectMm.left() + (x - E.left()) * s; };
  auto Y = [&](double y) { return L.plateRectMm.top() + (E.bottom() - y) * s; };
  auto n3 = [](double v) { return QString::number(v, 'f', 3); };
  auto pts = [&](const std::vector<KA::Point2> &v) {
    QString o;
    for (const auto &q : v)
      o += n3(X(q.x)) + ',' + n3(Y(q.y)) + ' ';
    return o.trimmed();
  };
  QTextStream out(&f);
  out.setEncoding(QStringConverter::Utf8);
  const QSizeF ps = L.paperMm;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
         "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
         "xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" "
      << "width=\"" << ps.width() << "mm\" height=\"" << ps.height()
      << "mm\" viewBox=\"0 0 " << ps.width() << ' ' << ps.height() << "\">\n"
      << "<title>" << r.title.toHtmlEscaped() << "</title>\n";
  auto layer = [&](const char *id, const QString &label) {
    out << "<g id=\"" << id
        << "\" inkscape:groupmode=\"layer\" inkscape:label=\"" << label
        << "\">\n";
  };
  const bool horiz = isHorizontal(doc, r.opts.slot);
  const double H = height(doc);

  // (1) 방안 눈금·치수·축척
  if (r.opts.grid) {
    layer("grid_dims_scale", QStringLiteral("방안 눈금·치수·축척"));
    out << "<g fill=\"none\">\n";
    for (int pass = 0; pass < 2; ++pass) {
      const char *col = pass == 0 ? "#EDEBE2" : "#D5D2C6";
      const double w = pass == 0 ? 0.08 : 0.15;
      for (int x = int(std::ceil(E.left())); x <= int(std::floor(E.right()));
           ++x)
        if ((pass == 0) == (x % 10 != 0))
          out << "<line x1=\"" << n3(X(x)) << "\" y1=\"" << n3(Y(E.top()))
              << "\" x2=\"" << n3(X(x)) << "\" y2=\"" << n3(Y(E.bottom()))
              << "\" stroke=\"" << col << "\" stroke-width=\"" << w << "\"/>\n";
      for (int y = int(std::ceil(E.top())); y <= int(std::floor(E.bottom()));
           ++y)
        if ((pass == 0) == (y % 10 != 0))
          out << "<line x1=\"" << n3(X(E.left())) << "\" y1=\"" << n3(Y(y))
              << "\" x2=\"" << n3(X(E.right())) << "\" y2=\"" << n3(Y(y))
              << "\" stroke=\"" << col << "\" stroke-width=\"" << w << "\"/>\n";
    }
    out << "<rect x=\"" << n3(X(E.left())) << "\" y=\"" << n3(Y(E.bottom()))
        << "\" width=\"" << n3(E.width() * s) << "\" height=\""
        << n3(E.height() * s)
        << "\" stroke=\"#C2C0B6\" stroke-width=\"0.2\"/>\n";
    if (!horiz)
      out << "<line x1=\"" << n3(X(E.left())) << "\" y1=\"" << n3(Y(0))
          << "\" x2=\"" << n3(X(E.right())) << "\" y2=\"" << n3(Y(0))
          << "\" stroke=\"#141413\" stroke-width=\"0.25\"/>\n";
    out << "</g>\n";
    const auto &d = doc.dims();
    const double ty = L.titleRectMm.top();
    out << "<g font-family=\"Malgun Gothic, Noto Sans KR\" font-size=\"2.6\" "
           "fill=\"#141413\">\n"
        << "<line x1=\"" << L.titleRectMm.left() << "\" y1=\"" << ty
        << "\" x2=\"" << L.titleRectMm.right() << "\" y2=\"" << ty
        << "\" stroke=\"#141413\" stroke-width=\"0.3\"/>\n"
        << "<text x=\"" << L.titleRectMm.left() << "\" y=\"" << ty + 7
        << "\">축척 " << scaleName(s) << " · 단위 mm</text>\n";
    const double sb = 50.0 * s, sx = L.titleRectMm.left(), sy = ty + 12;
    out << "<rect x=\"" << sx << "\" y=\"" << sy << "\" width=\"" << n3(sb / 2)
        << "\" height=\"1.5\" fill=\"#141413\"/>\n"
        << "<rect x=\"" << n3(sx + sb / 2) << "\" y=\"" << sy << "\" width=\""
        << n3(sb / 2)
        << "\" height=\"1.5\" fill=\"#FFFFFF\" stroke=\"#141413\" "
           "stroke-width=\"0.2\"/>\n"
        << "<text x=\"" << sx << "\" y=\"" << sy + 5
        << "\" font-family=\"Consolas\">0</text>\n"
        << "<text x=\"" << n3(sx + sb - 4) << "\" y=\"" << sy + 5
        << "\" font-family=\"Consolas\">50 mm</text>\n";
    const double bx = L.titleRectMm.right() - 92;
    out << "<rect x=\"" << bx << "\" y=\"" << ty + 3
        << "\" width=\"92\" height=\"18\" fill=\"none\" stroke=\"#141413\" "
           "stroke-width=\"0.25\"/>\n"
        << "<text x=\"" << bx + 3 << "\" y=\"" << ty + 9
        << "\" font-size=\"3\" font-weight=\"700\">" << r.title.toHtmlEscaped()
        << "</text>\n"
        << "<text x=\"" << bx + 3 << "\" y=\"" << ty + 17
        << "\" font-family=\"Consolas\" font-size=\"2.4\">기고 "
        << num(d.height) << " · 구경 " << num(d.rim) << " · 저경 "
        << num(d.base) << " mm</text>\n";
    if (!horiz && d.ok) {
      out << "<text x=\"" << n3(X(0)) << "\" y=\"" << n3(Y(H + 8) - 1)
          << "\" text-anchor=\"middle\" font-family=\"Consolas\">구경 ⌀ "
          << num(d.rim) << " mm</text>\n"
          << "<line x1=\"" << n3(X(-d.rim / 2)) << "\" y1=\"" << n3(Y(H + 8))
          << "\" x2=\"" << n3(X(d.rim / 2)) << "\" y2=\"" << n3(Y(H + 8))
          << "\" stroke=\"#141413\" stroke-width=\"0.18\"/>\n";
    }
    out << "</g>\n</g>\n";
  }
  // (2) 음영·탁본
  if (r.opts.shade && !horiz) {
    const ShadeImage sh = Plate::renderShade(
        doc, r.opts.shadeMode, 300.0 / 25.4 / std::max(0.25, 1.0 / s));
    if (!sh.isNull()) {
      // 외면 쪽 절반만 (Right: 영상 왼쪽 절반을 축 왼쪽에, Left: 오른쪽 절반을 축
      // 오른쪽에)
      const bool right = doc.sectionSide() == SectionSide::Right;
      const int half = sh.image.width() / 2;
      const QImage part =
          right ? sh.image.copy(0, 0, half, sh.image.height())
                : sh.image.copy(half, 0, sh.image.width() - half,
                                sh.image.height());
      QByteArray png;
      QBuffer buf(&png);
      buf.open(QIODevice::WriteOnly);
      part.save(&buf, "PNG");
      layer("shade_dtak", QStringLiteral("음영·탁본"));
      out << "<image x=\"" << n3(X(right ? sh.rectMm.left() : 0.0))
          << "\" y=\"" << n3(Y(sh.rectMm.bottom()))
          << "\" width=\"" << n3(sh.rectMm.width() / 2 * s) << "\" height=\""
          << n3(sh.rectMm.height() * s)
          << "\" preserveAspectRatio=\"none\" "
             "xlink:href=\"data:image/png;base64,"
          << png.toBase64() << "\"/>\n</g>\n";
    }
  }
  // (3) 외곽선·중심선
  if (r.opts.outline && !horiz) {
    layer("outline_axis", QStringLiteral("외곽선·중심선"));
    const auto ol = doc.displayOutline();
    out << "<g fill=\"none\" stroke=\"#141413\" stroke-linejoin=\"round\" "
           "stroke-linecap=\"round\">\n"
        << "<line x1=\"" << n3(X(0)) << "\" y1=\"" << n3(Y(-4)) << "\" x2=\""
        << n3(X(0)) << "\" y2=\"" << n3(Y(H + 6))
        << "\" stroke-width=\"0.25\" stroke-dasharray=\"6 1 1 1\"/>\n";
    if (ol.size() >= 2) {
      out << "<polyline stroke-width=\"" << kSvgOutline << "\" points=\""
          << pts(ol) << "\"/>\n";
      out << "<line x1=\"" << n3(X(0)) << "\" y1=\"" << n3(Y(ol.front().y))
          << "\" x2=\"" << n3(X(ol.front().x)) << "\" y2=\""
          << n3(Y(ol.front().y)) << "\" stroke-width=\"" << kSvgOutline
          << "\"/>\n";
      out << "<line x1=\"" << n3(X(0)) << "\" y1=\"" << n3(Y(ol.back().y))
          << "\" x2=\"" << n3(X(ol.back().x)) << "\" y2=\""
          << n3(Y(ol.back().y)) << "\" stroke-width=\"" << kSvgOutline
          << "\"/>\n";
    }
    out << "</g>\n</g>\n";
  }
  // (4) 단면선 (맨 위)
  if (r.opts.section) {
    layer("section_profile", QStringLiteral("단면선"));
    out << "<g stroke=\"#FF0000\" stroke-width=\"" << kSvgSection
        << "\" stroke-linejoin=\"round\">\n";
    if (!horiz) {
      if (doc.hasEstimatedInner()) {
        for (const auto &poly : doc.displaySection())
          out << "<polygon fill=\"#FF0000\" fill-opacity=\"0.12\" "
                 "stroke=\"none\" points=\""
              << pts(poly) << "\"/>\n";
        for (const auto &solid : doc.displayScannedSolid())
          out << "<polyline fill=\"none\" stroke=\"#FF0000\" stroke-width=\""
              << kSvgSection << "\" points=\"" << pts(solid) << "\"/>\n";
        for (const auto &est : doc.displayEstimatedInner())
          out << "<polyline fill=\"none\" stroke=\"#FF0000\" stroke-width=\""
              << kSvgSection << "\" stroke-dasharray=\"2.0,1.3\" points=\""
              << pts(est) << "\"/>\n";
      } else {
        for (const auto &poly : doc.displaySection())
          out << "<polygon fill=\"#FF0000\" fill-opacity=\"0.12\" points=\""
              << pts(poly) << "\"/>\n";
      }
      if (r.opts.hatch)
        for (const auto &[a, b] : doc.displayHatch())
          out << "<line x1=\"" << n3(X(a.x)) << "\" y1=\"" << n3(Y(a.y))
              << "\" x2=\"" << n3(X(b.x)) << "\" y2=\"" << n3(Y(b.y))
              << "\" stroke-width=\"0.13\"/>\n";
    } else {
      const auto &sl = doc.slotList()[size_t(r.opts.slot)];
      for (const auto &pl : doc.horizontalSlice(sl.z)) {
        std::vector<KA::Point2> v;
        for (const auto &q : pl.pts)
          v.push_back({q.x, q.y});
        out << "<polygon fill=\"none\" points=\"" << pts(v) << "\"/>\n";
      }
    }
    out << "</g>\n</g>\n";
  }
  out << "</svg>\n";
  return true;
}

// ---- DXF R12 (K-Arch 11 레이어, 1:1 mm, 화면과 같은 배치(D5 기본 외면 왼쪽·단면
// 오른쪽, R 반전 따름)) ----------------------------------------------------------
bool exportDxf(const ArtifactDoc &doc, const ExportRequest &r, QString *err) {
  if (refusePreview(doc, err))
    return false;
  KA::DxfWriter w;
  if (!w.Open(r.path.toLocal8Bit().toStdString())) {
    if (err)
      *err = QStringLiteral("DXF 파일을 만들 수 없습니다.");
    return false;
  }
  using LT = KA::DxfLinetype;
  w.DefineLayer("01_AXIS_CENTER", 7, LT::Center);
  w.DefineLayer("02_EXTERIOR_OUTLINE", 7, LT::Continuous);
  w.DefineLayer("03_EXTERIOR_FEATURES", 8, LT::Continuous);
  w.DefineLayer("04_SECTION_WALL", 1, LT::Continuous);
  w.DefineLayer("05_SECTION_HATCH", 1, LT::Continuous);
  w.DefineLayer("06_INTERIOR_OUTLINE", 1, LT::Dashed);
  w.DefineLayer("07_PROJECTION_RIM", 8, LT::Dashed);
  w.DefineLayer("08_PROJECTION_BASE", 8, LT::Dashed);
  w.DefineLayer("09_DIMENSION_MEASURE", 3, LT::Continuous);
  w.DefineLayer("10_ANNOTATION_TEXT", 7, LT::Continuous);
  w.DefineLayer("11_SCALE_BAR", 7, LT::Continuous);
  const double H = height(doc);
  w.WriteLine("01_AXIS_CENTER", {0, -4}, {0, H + 6}, LT::Center);
  const auto ol = doc.displayOutline();
  if (ol.size() >= 2) {
    w.WritePolyline("02_EXTERIOR_OUTLINE", ol, false);
    w.WriteLine("02_EXTERIOR_OUTLINE", {0, ol.front().y}, ol.front());
    w.WriteLine("02_EXTERIOR_OUTLINE", {0, ol.back().y}, ol.back());
  }
  if (doc.hasEstimatedInner()) {
    for (const auto &solid : doc.displayScannedSolid())
      w.WritePolyline("04_SECTION_WALL", solid, false);
    for (const auto &est : doc.displayEstimatedInner())
      w.WritePolyline("06_INTERIOR_OUTLINE", est, false);
  } else {
    for (const auto &poly : doc.displaySection())
      w.WritePolyline("04_SECTION_WALL", poly, true);
  }
  if (r.opts.hatch)
    for (const auto &[a, b] : doc.displayHatch())
      w.WriteLine("05_SECTION_HATCH", a, b);
  const auto &d = doc.dims();
  if (d.ok) {
    w.WriteLine("09_DIMENSION_MEASURE", {-d.rim / 2, H + 8},
                {d.rim / 2, H + 8});
    char buf[96];
    std::snprintf(buf, sizeof buf, "RIM D. %.1fmm", d.rim);
    w.WriteText("10_ANNOTATION_TEXT", {-d.rim / 2, H + 10}, 2.5, buf);
    std::snprintf(buf, sizeof buf, "H %.1fmm  BASE D. %.1fmm  MAX D. %.1fmm",
                  d.height, d.base, d.belly);
    w.WriteText("10_ANNOTATION_TEXT", {-d.rim / 2, -8}, 2.5, buf);
  }
  const double sx = maxR(doc) - 40, sy = -14;
  w.WriteSolid("11_SCALE_BAR", {sx, sy}, {sx + 25, sy}, {sx, sy + 1.5},
               {sx + 25, sy + 1.5});
  w.WritePolyline(
      "11_SCALE_BAR",
      {{sx, sy}, {sx + 50, sy}, {sx + 50, sy + 1.5}, {sx, sy + 1.5}}, true);
  w.WriteText("11_SCALE_BAR", {sx + 52, sy}, 2.5, "50 mm (1:1)");
  if (!w.Close()) {
    if (err)
      *err = QStringLiteral("DXF 저장 실패");
    return false;
  }
  return true;
}

} // namespace studio
