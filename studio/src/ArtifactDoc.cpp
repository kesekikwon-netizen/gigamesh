#include "ArtifactDoc.h"

#include "AxisOdr.h"
#include "AxisPca.h"
#include "MeshIO.h"
#include "MeshLoader.h"
#include "RimDetector.h"
#include "SyntheticMeshGen.h"

#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace studio {

namespace {

constexpr double kDeg = std::numbers::pi / 180.0;

void rotateZ(KA::MeshData &m, double deg) {
  const double c = std::cos(deg * kDeg), s = std::sin(deg * kDeg);
  for (auto &p : m.vertices) {
    const double x = c * p.x - s * p.y, y = s * p.x + c * p.y;
    p.x = x;
    p.y = y;
  }
}

// 방향 d 를 +Z 로 보내는 회전 (Rodrigues)
void rotateDirToZ(KA::MeshData &m, KA::Point3 d) {
  const double n = std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
  if (n < 1e-12)
    return;
  d = {d.x / n, d.y / n, d.z / n};
  if (d.z < 0)
    d = {-d.x, -d.y, -d.z};
  const double kx = d.y, ky = -d.x; // k = d × z
  const double s = std::sqrt(kx * kx + ky * ky), c = d.z;
  if (s < 1e-12)
    return;
  const double ux = kx / s, uy = ky / s;
  for (auto &p : m.vertices) {
    const double dot = ux * p.x + uy * p.y;
    const double cx = uy * p.z, cy = -ux * p.z, cz = ux * p.y - uy * p.x;
    const KA::Point3 q{p.x * c + cx * s + ux * dot * (1 - c),
                       p.y * c + cy * s + uy * dot * (1 - c), p.z * c + cz * s};
    p = q;
  }
}

void translate(KA::MeshData &m, double dx, double dy, double dz) {
  for (auto &p : m.vertices) {
    p.x += dx;
    p.y += dy;
    p.z += dz;
  }
}

std::vector<double> zBand(const KA::MeshData &m, int n) {
  const double z0 = m.bboxMin.z + 0.12 * (m.bboxMax.z - m.bboxMin.z);
  const double z1 = m.bboxMin.z + 0.88 * (m.bboxMax.z - m.bboxMin.z);
  std::vector<double> zs;
  for (int i = 0; i < n; ++i)
    zs.push_back(z0 + (z1 - z0) * i / double(n - 1));
  return zs;
}

std::vector<KA::Point2> mirrorX(std::vector<KA::Point2> v) {
  for (auto &p : v)
    p.x = -p.x;
  return v;
}

double perpDist(const KA::Point2 &p, const KA::Point2 &a, const KA::Point2 &b) {
  const double dx = b.x - a.x, dy = b.y - a.y, L = std::hypot(dx, dy);
  if (L < 1e-12)
    return std::hypot(p.x - a.x, p.y - a.y);
  return std::fabs(dy * p.x - dx * p.y + b.x * a.y - b.y * a.x) / L;
}

void dpRec(const std::vector<KA::Point2> &p, size_t i0, size_t i1, double tol,
           std::vector<char> &keep) {
  if (i1 <= i0 + 1)
    return;
  double dmax = -1;
  size_t im = i0;
  for (size_t i = i0 + 1; i < i1; ++i) {
    const double d = perpDist(p[i], p[i0], p[i1]);
    if (d > dmax)
      dmax = d, im = i;
  }
  if (dmax > tol) {
    keep[im] = 1;
    dpRec(p, i0, im, tol, keep);
    dpRec(p, im, i1, tol, keep);
  }
}

// 합성 토기 정점에 자연스러운 Artec 컬러(소성 토기 색조 + 어깨 침선/타날 흔적)
// 부여
std::vector<QRgb> makeSyntheticArtecColors(const KA::MeshData &m) {
  std::vector<QRgb> cols(m.vertices.size());
  double zmin = 1e18, zmax = -1e18;
  for (const auto &p : m.vertices) {
    zmin = std::min(zmin, p.z);
    zmax = std::max(zmax, p.z);
  }
  const double h = std::max(1.0, zmax - zmin);
  for (size_t i = 0; i < m.vertices.size(); ++i) {
    const auto &p = m.vertices[i];
    const double t = (p.z - zmin) / h;
    const double ang = std::atan2(p.y, p.x);
    const double fire = 0.92 +
                        0.08 * std::sin(t * 5.0 + 0.7 * std::cos(2 * ang)) -
                        0.05 * std::exp(-std::pow((t - 0.68) / 0.06, 2.0));
    const int r = std::clamp(int(196 * fire), 60, 240);
    const int g = std::clamp(int(166 * fire), 50, 225);
    const int b = std::clamp(int(134 * fire), 40, 200);
    cols[i] = qRgb(r, g, b);
  }
  return cols;
}

} // namespace

std::vector<KA::Point2> simplifyDP(const std::vector<KA::Point2> &pts,
                                   double tol, bool closed) {
  if (pts.size() < 3 || tol <= 0)
    return pts;
  std::vector<KA::Point2> p = pts;
  if (closed)
    p.push_back(pts.front());
  std::vector<char> keep(p.size(), 0);
  keep.front() = keep.back() = 1;
  if (closed && p.size() > 3) {
    size_t far = 0;
    double dm = -1;
    for (size_t i = 0; i < p.size(); ++i) {
      const double d = std::hypot(p[i].x - p[0].x, p[i].y - p[0].y);
      if (d > dm)
        dm = d, far = i;
    }
    keep[far] = 1;
    dpRec(p, 0, far, tol, keep);
    dpRec(p, far, p.size() - 1, tol, keep);
  } else {
    dpRec(p, 0, p.size() - 1, tol, keep);
  }
  std::vector<KA::Point2> out;
  for (size_t i = 0; i < p.size(); ++i)
    if (keep[i])
      out.push_back(p[i]);
  if (closed && out.size() > 1)
    out.pop_back();
  return out;
}

std::vector<KA::Point2> smoothCatmull(const std::vector<KA::Point2> &pts,
                                      bool closed, int sub) {
  const size_t n = pts.size();
  if (n < 3 || sub < 2)
    return pts;
  auto at = [&](long i) -> const KA::Point2 & {
    if (closed)
      return pts[size_t((i % long(n) + long(n)) % long(n))];
    return pts[size_t(std::clamp(i, 0L, long(n) - 1))];
  };
  std::vector<KA::Point2> out;
  const long segs = closed ? long(n) : long(n) - 1;
  for (long i = 0; i < segs; ++i) {
    const auto &p0 = at(i - 1), &p1 = at(i), &p2 = at(i + 1), &p3 = at(i + 2);
    for (int k = 0; k < sub; ++k) {
      const double t = double(k) / sub, t2 = t * t, t3 = t2 * t;
      auto f = [&](double a, double b, double c, double d) {
        return 0.5 * (2 * b + (-a + c) * t + (2 * a - 5 * b + 4 * c - d) * t2 +
                      (-a + 3 * b - 3 * c + d) * t3);
      };
      out.push_back({f(p0.x, p1.x, p2.x, p3.x), f(p0.y, p1.y, p2.y, p3.y)});
    }
  }
  if (!closed)
    out.push_back(pts.back());
  return out;
}

bool ArtifactDoc::loadFile(const QString &path, QString *err) {
  MeshReadResult r = readMeshAny(path);
  if (!r.ok) {
    if (err)
      *err = r.message;
    return false;
  }
  base_ = std::move(r.mesh);
  baseColors_ = std::move(r.vertexColors);
  scannerProfile_ = r.scannerProfile;
  unitNote_ = r.unitNote;
  removedNoiseFaces_ = r.removedNoiseFaces;
  path_ = path;
  name_ = QFileInfo(path).completeBaseName();
  frontAngle_ = 0.0;
  autoAlign();
  return true;
}

void ArtifactDoc::loadSample() {
  KA::Testing::SyntheticParams sp;
  sp.radialSeg = 160;
  sp.heightSeg = 120;
  sp.capTop = false; // 열린 구연부
  sp.capBottom = true;
  sp.tiltDeg = 7.0; // 일부러 기울여 정위치 과정을 보여준다
  sp.translation = {12.0, -5.0, 3.0};
  sp.noiseSigma = 0.05;
  const auto vessel =
      KA::Testing::MakeTorusProfileVessel(82.6, 71.0, 184.5, 6.8, sp);
  base_ = vessel.toMesh();
  KA::FillMeshStats(base_);
  baseColors_ = makeSyntheticArtecColors(base_);
  scannerProfile_ = QStringLiteral("Artec Space Spider 급 (내장 샘플)");
  unitNote_.clear();
  removedNoiseFaces_ = 0;
  path_.clear();
  name_ = QStringLiteral("예제_합성토기_01호");
  frontAngle_ = 0.0;
  autoAlign();
}

void ArtifactDoc::loadArtecBlindSample() {
  // 좁은 아가리 항아리에서 광학 스캐너(Artec Space Spider / Eva)가 구연부
  // 안쪽(상부 35%)만 찍고 동체부·저부 내벽은 찍지 못해 외벽 단일 쉘(또는 홀필
  // 마개)로 남은 실제 현장 상황을 재현
  KA::Testing::SyntheticParams sp;
  sp.radialSeg = 160;
  sp.heightSeg = 120;
  sp.capTop = false;
  sp.capBottom = false;
  sp.tiltDeg = 4.5;
  sp.translation = {-6.0, 4.0, 1.5};
  sp.noiseSigma = 0.04;
  const auto vessel =
      KA::Testing::MakeTorusProfileVessel(84.0, 62.0, 190.0, 7.0, sp);
  KA::MeshData full = vessel.toMesh();
  KA::FillMeshStats(full);
  // 내벽 정점(innerBase 이상 인덱스) 중 높이 하위 65% 구간의 삼각형을 제거하여
  // 광학 스캐너 음영(내벽 미스캔) 상태 재현
  const int rings = sp.heightSeg + 1;
  const int innerBase = rings * sp.radialSeg;
  const int cutRing = int(sp.heightSeg * 0.65);
  std::vector<std::array<int, 3>> kept;
  kept.reserve(full.faces.size());
  for (const auto &f : full.faces) {
    const bool isInner =
        f[0] >= innerBase || f[1] >= innerBase || f[2] >= innerBase;
    if (isInner) {
      const int r0 = (f[0] - innerBase) / sp.radialSeg;
      const int r1 = (f[1] - innerBase) / sp.radialSeg;
      const int r2 = (f[2] - innerBase) / sp.radialSeg;
      if (std::min({r0, r1, r2}) < cutRing)
        continue; // 동체부·저부 내벽 미스캔
    }
    kept.push_back(f);
  }
  full.faces = std::move(kept);
  KA::FillMeshStats(full);
  base_ = std::move(full);
  baseColors_ = makeSyntheticArtecColors(base_);
  scannerProfile_ =
      QStringLiteral("Artec Space Spider (내벽 미스캔 보정 샘플)");
  unitNote_.clear();
  removedNoiseFaces_ = 0;
  path_.clear();
  name_ = QStringLiteral("Artec_내벽미스캔_단경호_02호");
  frontAngle_ = 0.0;
  inferInnerWall_ = true;
  autoAlign();
}

void ArtifactDoc::rescaleUnits(double factor) {
  if (base_.vertices.empty() || factor <= 0 || std::fabs(factor - 1.0) < 1e-9)
    return;
  for (auto &p : base_.vertices) {
    p.x *= factor;
    p.y *= factor;
    p.z *= factor;
  }
  unitNote_ = QStringLiteral("단위 배율 ×%1 적용됨").arg(factor, 0, 'g', 4);
  autoAlign();
}

void ArtifactDoc::setInferInnerWall(bool on) {
  if (inferInnerWall_ == on)
    return;
  inferInnerWall_ = on;
  if (isLoaded()) {
    buildSectionLoops();
  }
}

void ArtifactDoc::setEstimatedWallMm(double mm) {
  estimatedWallMm_ = std::clamp(mm, 1.5, 40.0);
  if (isLoaded() && inferInnerWall_) {
    buildSectionLoops();
  }
}

void ArtifactDoc::autoAlign() {
  KA::MeshData m = base_;
  KA::FillMeshStats(m);
  aligned_ = false;
  alignNote_.clear();
  axisRmse_ = 0.0;

  // 초고해상도 Artec 메시(25만 면 초과)는 경량 프록시로 회전축(PCA+ODR)을 즉시
  // 찾고 원본에 변환 적용
  KA::MeshData proxy;
  std::vector<QRgb> dummyCol;
  const bool useProxy = m.faces.size() > 250000;
  if (useProxy)
    buildLodMesh(m, {}, proxy, dummyCol, 50000);
  KA::MeshData &work = useProxy ? proxy : m;

  // 1) 구연부 → PCA 로 입구를 위로
  const KA::RimResult rim = KA::DetectRim(work, KA::RimMode::Auto);
  if (rim.error == KA::RimError::Ok) {
    const auto pca =
        KA::ComputePcaAlignment(rim.rimPoints, KA::MeshCentroid(work));
    if (pca.ok) {
      for (auto &p : work.vertices)
        p = pca.transform.apply(p);
      KA::FillMeshStats(work);
      if (useProxy) {
        for (auto &p : m.vertices)
          p = pca.transform.apply(p);
        KA::FillMeshStats(m);
      }
    }
  }
  // 2) 수평 외곽 원들로 회전 중심축 정밀화
  const KA::Axis3D ax =
      KA::OptimizeRotationAxis(KA::FitHorizontalOuters(work, zBand(work, 24)));
  if (ax.ok) {
    translate(work, -ax.origin.x, -ax.origin.y, -ax.origin.z);
    rotateDirToZ(work, ax.direction);
    KA::FillMeshStats(work);
    if (useProxy) {
      translate(m, -ax.origin.x, -ax.origin.y, -ax.origin.z);
      rotateDirToZ(m, ax.direction);
      KA::FillMeshStats(m);
    }
    const auto fits = KA::FitHorizontalOuters(work, zBand(work, 12));
    double sx = 0, sy = 0;
    int n = 0;
    for (const auto &f : fits)
      if (f.circle.ok)
        sx += f.circle.xc, sy += f.circle.yc, ++n;
    if (n > 0) {
      translate(m, -sx / n, -sy / n, 0);
    }
    aligned_ = true;
    axisRmse_ = ax.rmse;
    alignNote_ = rim.error == KA::RimError::Ok
                     ? QStringLiteral("바닥 수평(0 mm) · 회전 중심축 정렬됨")
                     : QStringLiteral("회전 중심축 정렬됨 (구연부 미검출)");
  } else {
    const double cx = 0.5 * (m.bboxMin.x + m.bboxMax.x),
                 cy = 0.5 * (m.bboxMin.y + m.bboxMax.y);
    translate(m, -cx, -cy, 0);
    alignNote_ = QStringLiteral("중심축 미정렬 — [중심축 맞춤] 필요");
  }
  if (!unitNote_.isEmpty())
    alignNote_ += QStringLiteral(" · ") + unitNote_;
  if (removedNoiseFaces_ > 0)
    alignNote_ +=
        QStringLiteral(" · 부유노이즈 %1면 제거").arg(removedNoiseFaces_);

  KA::FillMeshStats(m);
  translate(m, 0, 0, -m.bboxMin.z); // 바닥 H 0
  KA::FillMeshStats(m);
  base_ = std::move(m);
  buildLodMesh(base_, baseColors_, lodBase_, lodColors_, 35000);
  resetSlots();
  recompute();
}

void ArtifactDoc::setFrontAngle(double deg) {
  frontAngle_ = std::remainder(deg, 360.0);
  recompute();
}

void ArtifactDoc::recompute() {
  mesh_ = base_;
  lodMesh_ = lodBase_;
  if (std::fabs(frontAngle_) > 1e-9) {
    rotateZ(mesh_, -frontAngle_);
    rotateZ(lodMesh_, -frontAngle_);
  }
  KA::FillMeshStats(mesh_);
  KA::FillMeshStats(lodMesh_);

  const KA::MeshData &fastMesh = mesh_.faces.size() > 250000 ? lodMesh_ : mesh_;
  sil_ = KA::ExtractSilhouette(fastMesh, KA::SilhouetteMode::MaxRadius, 0.25);
  dims_ = KA::AnalyzeDimensions(fastMesh, sil_);

  d5_ = {};
  d5_.ok = dims_.ok;
  d5_.height = dims_.vesselHeight;
  d5_.rim = dims_.rimDiameter;
  d5_.belly = dims_.maxBodyDiameter;
  d5_.bellyZ = dims_.maxBodyZ - mesh_.bboxMin.z;
  d5_.base = dims_.baseDiameter;

  // 기벽 두께 프로파일: 수평 단면을 높이별로 잘라 바깥 고리 - 안쪽 고리 (둘레
  // 평균)
  const double h = mesh_.bboxMax.z - mesh_.bboxMin.z;
  wallZ0_ = mesh_.bboxMin.z;
  wallStep_ = std::max(1.0, h / 160.0);
  wallProfile_.clear();
  for (double z = wallZ0_; z <= mesh_.bboxMax.z; z += wallStep_)
    wallProfile_.push_back(z <= wallZ0_ + 0.02 * h ||
                                   z >= mesh_.bboxMax.z - 0.02 * h
                               ? -1.0
                               : measureWallBySlice(z));

  double sum = 0;
  int n = 0;
  for (size_t i = 0; i < wallProfile_.size(); ++i) {
    const double z = wallZ0_ + double(i) * wallStep_;
    if (z < wallZ0_ + 0.1 * h || z > wallZ0_ + 0.9 * h || wallProfile_[i] <= 0)
      continue;
    sum += wallProfile_[i], ++n;
  }
  d5_.meanWall = n ? sum / n : estimatedWallMm_;
  if (d5_.meanWall > 1.5 && d5_.meanWall < 35.0)
    estimatedWallMm_ = d5_.meanWall;

  buildSectionLoops();
}

void ArtifactDoc::resetSlots() {
  slotList_.clear();
  const double h = base_.bboxMax.z - base_.bboxMin.z;
  slotList_.push_back(
      {QStringLiteral("A–A′ 정면 실측도"), QStringLiteral("좌단면·우외곽"),
       QStringLiteral("보고서 주 도판 (1:1)"), SlotKind::Vertical, 0.0, 0.0});
  slotList_.push_back({QStringLiteral("B–B′ 구연부 횡단면"),
                       QStringLiteral("수평절단"),
                       QStringLiteral("구연부 두께 확인용"),
                       SlotKind::Horizontal, 0.0, h * 0.94});
  slotList_.push_back(
      {QStringLiteral("C–C′ 동체부 횡단면"), QStringLiteral("최대복경"),
       QStringLiteral("몸통 외경 확인"), SlotKind::Horizontal, 0.0, h * 0.5});
  slotList_.push_back({QStringLiteral("저부(바닥) 평면·단면"),
                       QStringLiteral("바닥 굽"),
                       QStringLiteral("바닥 굽 접지면"), SlotKind::Horizontal,
                       0.0, std::min(3.0, h * 0.03)});
}

std::vector<std::vector<KA::Point2>> ArtifactDoc::displaySection() const {
  if (!scannedSolidLines_.empty() && !estimatedInnerLines_.empty()) {
    const auto solids = displayScannedSolid();
    const auto dashed = displayEstimatedInner();
    if (!solids.empty() && !dashed.empty()) {
      std::vector<KA::Point2> poly = solids.front();
      for (size_t i = dashed.front().size(); i-- > 0;)
        poly.push_back(dashed.front()[i]);
      if (poly.size() >= 3)
        return {std::move(poly)};
    }
  }
  std::vector<std::vector<KA::Point2>> out;
  for (const auto &poly : sectionLoops_) {
    auto p = simplifyDP(mirrorX(poly), simplifyTol_ * 0.5, true);
    if (smooth_)
      p = smoothCatmull(p, true, 3);
    if (p.size() >= 3)
      out.push_back(std::move(p));
  }
  return out;
}

std::vector<std::vector<KA::Point2>> ArtifactDoc::displayScannedSolid() const {
  std::vector<std::vector<KA::Point2>> out;
  for (const auto &line : scannedSolidLines_) {
    auto p = simplifyDP(mirrorX(line), simplifyTol_ * 0.5, false);
    if (smooth_ && p.size() >= 3)
      p = smoothCatmull(p, false, 3);
    if (p.size() >= 2)
      out.push_back(std::move(p));
  }
  return out;
}

std::vector<std::vector<KA::Point2>>
ArtifactDoc::displayEstimatedInner() const {
  std::vector<std::vector<KA::Point2>> out;
  for (const auto &line : estimatedInnerLines_) {
    auto p = simplifyDP(mirrorX(line), simplifyTol_ * 0.5, false);
    if (smooth_ && p.size() >= 3)
      p = smoothCatmull(p, false, 3);
    if (p.size() >= 2)
      out.push_back(std::move(p));
  }
  return out;
}

std::vector<KA::Point2> ArtifactDoc::displayOutline() const {
  auto p = simplifyDP(mirrorX(sil_.leftOutline), simplifyTol_, false);
  if (smooth_)
    p = smoothCatmull(p, false, 4);
  return p;
}

std::vector<std::pair<KA::Point2, KA::Point2>>
ArtifactDoc::displayHatch() const {
  std::vector<std::pair<KA::Point2, KA::Point2>> out;
  const auto polys = displaySection();
  const double c = std::sqrt(0.5), spacing = 1.5;
  auto rot = [&](KA::Point2 p, double s) {
    return KA::Point2{c * p.x + s * c * p.y, -s * c * p.x + c * p.y};
  };
  double lo = 1e18, hi = -1e18;
  std::vector<std::vector<KA::Point2>> rp;
  for (const auto &poly : polys) {
    std::vector<KA::Point2> q;
    for (const auto &p : poly)
      q.push_back(rot(p, 1.0)), lo = std::min(lo, q.back().y),
                                hi = std::max(hi, q.back().y);
    rp.push_back(std::move(q));
  }
  for (double y = std::ceil(lo / spacing) * spacing; y <= hi; y += spacing) {
    std::vector<double> xs;
    for (const auto &q : rp)
      for (size_t i = 0; i < q.size(); ++i) {
        const auto &a = q[i], &b = q[(i + 1) % q.size()];
        if ((a.y <= y && b.y > y) || (b.y <= y && a.y > y))
          xs.push_back(a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y));
      }
    std::sort(xs.begin(), xs.end());
    for (size_t i = 0; i + 1 < xs.size(); i += 2)
      if (xs[i + 1] - xs[i] > 0.3)
        out.push_back({rot({xs[i], y}, -1.0), rot({xs[i + 1], y}, -1.0)});
  }
  return out;
}

void ArtifactDoc::buildSectionLoops() {
  sectionLoops_.clear();
  scannedSolidLines_.clear();
  estimatedInnerLines_.clear();
  estZMin_ = -1.0;
  estZMax_ = -1.0;

  std::vector<std::vector<KA::Point2>>
      openBranches; // 각 가지: 위(zHi) → 아래(zLo) 순서

  for (const auto &pl : KA::Slice(mesh_, KA::Plane{{0, 1, 0}, 0.0})) {
    if (pl.pts.size() < 3)
      continue;
    std::vector<KA::Point2> in;
    in.reserve(pl.pts.size());
    for (const auto &p : pl.pts)
      in.push_back({p.x, p.z});

    // 중심축(x=0) 기준 오른쪽(x >= 0) 반평면 클리핑
    std::vector<KA::Point2> clip;
    for (size_t i = 0; i < in.size(); ++i) {
      const auto &a = in[i], &b = in[(i + 1) % in.size()];
      const bool ia = a.x >= 0, ib = b.x >= 0;
      if (ia)
        clip.push_back(a);
      if (ia != ib) {
        const double t = a.x / (a.x - b.x);
        clip.push_back({0.0, a.y + t * (b.y - a.y)});
      }
    }
    if (clip.size() < 3)
      continue;

    double maxJump = 0.0;
    size_t jumpIdx = 0;
    for (size_t i = 0; i < clip.size(); ++i) {
      const auto &a = clip[i], &b = clip[(i + 1) % clip.size()];
      if (a.x < 1.0 && b.x < 1.0)
        continue;
      const double d = std::hypot(b.x - a.x, b.y - a.y);
      if (d > maxJump) {
        maxJump = d;
        jumpIdx = i;
      }
    }

    if (maxJump <= 25.0) {
      double area = 0;
      for (size_t i = 0; i < clip.size(); ++i) {
        const auto &a = clip[i], &b = clip[(i + 1) % clip.size()];
        area += a.x * b.y - b.x * a.y;
      }
      if (std::fabs(area) * 0.5 >= 0.5)
        sectionLoops_.push_back(std::move(clip));
      continue;
    }

    // 열린 사슬을 끊어진 변(jumpIdx) 다음 점부터 순서대로 정렬
    std::vector<KA::Point2> chain;
    chain.reserve(clip.size());
    for (size_t k = 1; k <= clip.size(); ++k)
      chain.push_back(clip[(jumpIdx + k) % clip.size()]);

    size_t iTop = 0;
    for (size_t i = 1; i < chain.size(); ++i)
      if (chain[i].y > chain[iTop].y)
        iTop = i;

    const bool isUTurn = iTop > 2 && iTop + 2 < chain.size() &&
                         (chain[iTop].y - chain.front().y > 12.0) &&
                         (chain[iTop].y - chain.back().y > 12.0);
    if (isUTurn) {
      // 구연부 입술이 연결된 U자형 열린 사슬 → 두 가지로 분리
      std::vector<KA::Point2> b1(chain.begin(), chain.begin() + long(iTop) + 1);
      std::vector<KA::Point2> b2(chain.begin() + long(iTop), chain.end());
      std::reverse(b1.begin(), b1.end());
      if (std::fabs(b1.front().y - b1.back().y) > 8.0)
        openBranches.push_back(std::move(b1));
      if (std::fabs(b2.front().y - b2.back().y) > 8.0)
        openBranches.push_back(std::move(b2));
    } else {
      if (chain.front().y < chain.back().y)
        std::reverse(chain.begin(), chain.end());
      if (std::fabs(chain.front().y - chain.back().y) > 8.0)
        openBranches.push_back(std::move(chain));
    }
  }

  if (!inferInnerWall_ || !sectionLoops_.empty() || openBranches.empty())
    return;

  // 가장 바닥(minY)까지 내려가고 평균 반경이 바깥쪽인 가지를 외벽(outer)으로,
  // 나머지를 스캔된 상부 내벽(innerScanned)으로 결합
  size_t outerIdx = 0;
  double bestScore = -1e18;
  for (size_t i = 0; i < openBranches.size(); ++i) {
    const auto &b = openBranches[i];
    double meanX = 0;
    for (const auto &p : b)
      meanX += p.x;
    meanX /= double(b.size());
    const double spanY = b.front().y - b.back().y;
    const double score = spanY * 2.0 + meanX;
    if (score > bestScore) {
      bestScore = score;
      outerIdx = i;
    }
  }
  const std::vector<KA::Point2> outer = openBranches[outerIdx];
  std::vector<KA::Point2> innerScanned;
  for (size_t i = 0; i < openBranches.size(); ++i) {
    if (i == outerIdx)
      continue;
    if (openBranches[i].size() > innerScanned.size())
      innerScanned = openBranches[i];
  }

  double zScanEnd = outer.front().y;
  double realWall = std::max(2.0, estimatedWallMm_);
  if (innerScanned.size() >= 2) {
    zScanEnd = innerScanned.back().y;
    for (size_t i = 1; i < outer.size(); ++i) {
      const auto &a = outer[i - 1], &b = outer[i];
      if ((a.y >= zScanEnd && b.y <= zScanEnd) ||
          (b.y >= zScanEnd && a.y <= zScanEnd)) {
        const double t =
            std::fabs(b.y - a.y) < 1e-9 ? 0.0 : (zScanEnd - a.y) / (b.y - a.y);
        const double rOut = a.x + t * (b.x - a.x);
        const double w = rOut - innerScanned.back().x;
        if (w > 1.5 && w < 35.0)
          realWall = w;
        break;
      }
    }
  }
  estimatedWallMm_ = realWall;

  std::vector<KA::Point2> estInner;
  if (!innerScanned.empty())
    estInner.push_back(innerScanned.back());
  for (size_t i = 0; i < outer.size(); ++i) {
    if (outer[i].y > zScanEnd)
      continue;
    const auto &p0 = outer[i > 0 ? i - 1 : i];
    const auto &p1 = outer[i + 1 < outer.size() ? i + 1 : i];
    const double tx = p1.x - p0.x, ty = p1.y - p0.y;
    const double L = std::hypot(tx, ty);
    double nx = -1.0, ny = 0.0;
    if (L > 1e-9) {
      nx = ty / L;
      ny = -tx / L;
      if (nx > 0) {
        nx = -nx;
        ny = -ny;
      }
    }
    estInner.push_back({std::max(0.0, outer[i].x + nx * realWall),
                        std::max(realWall * 0.7, outer[i].y + ny * realWall)});
  }
  if (estInner.size() >= 2) {
    for (const auto &q : estInner) {
      estZMin_ = (estZMin_ < 0) ? q.y : std::min(estZMin_, q.y);
      estZMax_ = std::max(estZMax_, q.y);
    }
    estimatedInnerLines_.push_back(estInner);

    std::vector<KA::Point2> solidLine;
    if (!innerScanned.empty()) {
      for (size_t i = innerScanned.size(); i-- > 0;)
        solidLine.push_back(innerScanned[i]);
    }
    for (const auto &p : outer)
      solidLine.push_back(p);
    solidLine.push_back(estInner.back());
    scannedSolidLines_.push_back(std::move(solidLine));
  }

  std::vector<KA::Point2> closedPoly = outer;
  for (auto it = estInner.rbegin(); it != estInner.rend(); ++it)
    closedPoly.push_back(*it);
  if (innerScanned.size() >= 2) {
    for (size_t i = innerScanned.size() - 1; i-- > 0;)
      closedPoly.push_back(innerScanned[i]);
  }
  if (closedPoly.size() >= 3)
    sectionLoops_.push_back(std::move(closedPoly));
}

std::vector<KA::Polyline3> ArtifactDoc::horizontalSlice(double z) const {
  return KA::Slice(mesh_, KA::Plane{{0, 0, 1}, -z});
}

bool ArtifactDoc::isWallEstimatedAt(double z) const {
  return !estimatedInnerLines_.empty() && z >= estZMin_ - 1.0 &&
         z <= estZMax_ + 1.0;
}

double ArtifactDoc::wallThicknessAt(double z) const {
  if (isWallEstimatedAt(z))
    return estimatedWallMm_;
  if (wallProfile_.size() < 2)
    return -1.0;
  const double t = (z - wallZ0_) / wallStep_;
  const long i = long(std::floor(t));
  if (i < 0 || i + 1 >= long(wallProfile_.size()))
    return -1.0;
  const double a = wallProfile_[size_t(i)], b = wallProfile_[size_t(i + 1)];
  if (a <= 0 || b <= 0)
    return std::max(a, b);
  return a + (b - a) * (t - double(i));
}

double ArtifactDoc::measureWallBySlice(double z) const {
  const KA::MeshData &m = mesh_.faces.size() > 250000 ? lodMesh_ : mesh_;
  const auto loops = KA::Slice(m, KA::Plane{{0, 0, 1}, -z});
  std::vector<std::pair<double, std::vector<double>>> rs;
  for (const auto &l : loops) {
    if (l.pts.size() < 3)
      continue;
    std::vector<double> r;
    r.reserve(l.pts.size());
    double s = 0;
    for (const auto &p : l.pts)
      r.push_back(std::hypot(p.x, p.y)), s += r.back();
    rs.push_back({s / double(r.size()), std::move(r)});
  }
  if (rs.empty())
    return -1.0;
  std::sort(rs.begin(), rs.end(),
            [](auto &a, auto &b) { return a.first > b.first; });
  if (rs.size() >= 2)
    return rs[0].first - rs[1].first;
  // 단일 열린 쉘(내벽 미스캔)인 경우 -1 반환하여 추정 기벽두께와 구분
  return -1.0;
}

int ArtifactDoc::pointCount() const {
  int n = int(displayOutline().size());
  for (const auto &p : displaySection())
    n += int(p.size());
  return n;
}

int ArtifactDoc::rawPointCount() const {
  int n = int(sil_.leftOutline.size());
  for (const auto &p : sectionLoops_)
    n += int(p.size());
  return n;
}

} // namespace studio
