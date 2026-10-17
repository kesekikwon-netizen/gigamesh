// DongArch3D Studio — 유물 문서: 메시 로드(Artec PLY/OBJ+텍스처/WRL/STL) → 부유
// 노이즈 제거·단위 환산 → ① 바닥 정하기(들어온 자세 그대로 → 사람이 돌려 놓고
// 확정, 좌표: mm · +Z 위 · 바닥판 Z = 0) → 단면·외곽·5대 치수. 계산은 전부
// K-Arch 엔진(GigaMesh/gui/src/dongarch/karch)에 맡기고, 여기서는 순서와 Artec
// 특화 보정 상태만 관리한다.
#pragma once
#include "DimensionAnalyzer.h"
#include "MeshSlicer.h"
#include "SectionComposer.h"
#include "Silhouette.h"

#include <QColor>
#include <QString>
#include <vector>

namespace studio {

namespace KA = DongArch::KArch;

enum class SlotKind { Vertical, Horizontal };

struct SectionSlot {
  QString name; // "A–A′ 정면 실측도"
  QString meta; // "좌단면·우외곽 · 184.5 mm"
  QString note; // "보고서 주 도판 (1:1)"
  SlotKind kind = SlotKind::Vertical;
  double angleDeg = 0.0; // Vertical: 정면 회전각
  double z = 0.0;        // Horizontal: 절단 높이 (mm)
};

enum class HeightRef { Base, Rim }; // B: 바닥 0 mm, T: 구연부 0 mm

struct Dims5 {
  double height = 0, rim = 0, belly = 0, bellyZ = 0, base = 0, meanWall = 0;
  bool ok = false;
};

// ① 바닥 정하기: 바닥판(Z = 0 평면)의 고정 월드축
enum class PoseAxis { X, Y, Z };

class ArtifactDoc {
public:
  // 들어온 자세 그대로 보관한다(확정 전 — 단면·치수·도판을 계산하지 않음).
  // 경로·스캐너 프로파일·단위 메모는 바꾸지 않는다(loadFile/loadSample 몫).
  void loadMesh(KA::MeshData m, std::vector<QRgb> colors, const QString &name);
  bool loadFile(const QString &path,
                QString *err); // 들어온 자세 그대로 (자동 세우기 안 함)
  // 합성 토기(항아리) — Artec 컬러·타날문 포함 기본 예제.
  // confirm=true: 자동 세우기 → 확정 (예전과 같은 결과), false: 들어온 자세
  void loadSample(bool confirm = true);
  // 좁은 아가리 토기(내벽 미스캔·Artec 홀필 마개 보정 예제), confirm 은 위와 같음
  void loadArtecBlindSample(bool confirm = true);
  bool isLoaded() const { return !mesh_.vertices.empty(); }

  // ---- ① 바닥 정하기 (자세 단계) ----
  // 현재 자세의 바운딩 박스 중심을 지나는 고정 월드축 기준 오른손 법칙 회전,
  // 뒤에 바닥에 내려놓기. 90° 정수배는 정수 행렬(누적 오차 없음).
  void rotatePose(PoseAxis a, double deg);
  void flipPose();    // 뒤집기 = X 축 180°
  void resetPose();   // 처음 자세로: 읽은 정점 좌표 그대로 (접지 안 함)
  void dropToFloor(); // 바닥에 내려놓기: z 만 평행이동해 bboxMin.z = 0
  // 자동 세우기: 구연부 PCA → 회전 중심축(ODR) 정밀화 → 축을 R 0, 바닥을 H 0
  // 으로. 확정은 하지 않는다.
  void autoAlign();
  // 이 자세로 확정: (자동 세우기 직후가 아니면) 중심선을 원점으로, 접지,
  // 단면·외곽·치수·슬롯 계산
  void confirmPose();
  bool poseConfirmed() const { return poseConfirmed_; }
  double heightAboveFloor() const { return mesh_.bboxMin.z; } // mm
  bool isGrounded() const; // |bboxMin.z| ≤ 0.3 mm (디자인 v2 접지 판정 권고값)
  // 직전 자세 조작이 자동 세우기이고 회전 중심축을 찾았는지
  bool autoCentered() const { return centered_ && pendingAligned_; }
  // 접지 상태 문구 (ctx 줄·3D 정보 줄 공용): "바닥판에서 45.0 mm 위" 등
  QString groundNote() const;

  void setFrontAngle(double deg); // 정면 회전 (, .)
  void setHeightRef(HeightRef r) { heightRef_ = r; }
  void setSimplifyTol(double mm) { simplifyTol_ = mm; }
  void setSmooth(bool on) { smooth_ = on; }

  // Artec 스캐너 음영지대(내벽 미스캔·Watertight 홀필 마개) 기벽 자동 추정 보정
  void setInferInnerWall(bool on);
  bool inferInnerWall() const { return inferInnerWall_; }
  void setEstimatedWallMm(double mm);
  double estimatedWallMm() const { return estimatedWallMm_; }
  bool hasEstimatedInner() const { return !estimatedInnerLines_.empty(); }

  // 수동 단위 배율 보정 (예: 10.0 = cm→mm, 0.1 = mm→cm)
  void rescaleUnits(double factor);

  const KA::MeshData &mesh() const {
    return mesh_;
  } // 원본 정밀 메시 (단면·D-Tak 용)
  const KA::MeshData &lodMesh() const {
    return lodMesh_;
  } // 3D 보조창 고속 회전용 경량 LOD 메시
  const std::vector<QRgb> &vertexColors() const { return baseColors_; }
  const std::vector<QRgb> &lodVertexColors() const { return lodColors_; }
  bool hasColor() const {
    return !baseColors_.empty() && baseColors_.size() == mesh_.vertices.size();
  }
  QString scannerProfile() const { return scannerProfile_; }
  QString unitNote() const { return unitNote_; }
  int removedNoiseFaces() const { return removedNoiseFaces_; }

  const KA::SilhouetteResult &silhouette() const { return sil_; }
  const KA::SectionResult &section() const { return sec_; }
  const KA::Dimensions &rawDims() const { return dims_; }
  const Dims5 &dims() const { return d5_; }

  // 화면/내보내기용 선 (간략화·곡선 옵션 적용, 도면 좌표 x=R, y=H, 토기 표준:
  // 단면 왼쪽 / 외곽 오른쪽)
  std::vector<std::vector<KA::Point2>> displaySection() const;
  // Artec 미스캔 보정 시 실제 스캔된 실선 구간(외벽·구연·상부내벽·바닥)
  std::vector<std::vector<KA::Point2>> displayScannedSolid() const;
  // Artec 미스캔/홀필 구간 추정 내벽선 (고고학 실측 표준: 빨강 파선 표기용)
  std::vector<std::vector<KA::Point2>> displayEstimatedInner() const;
  std::vector<KA::Point2> displayOutline() const;
  std::vector<std::pair<KA::Point2, KA::Point2>> displayHatch() const;
  std::vector<KA::Polyline3> horizontalSlice(double z) const;
  double wallThicknessAt(double z) const; // 해당 높이 기벽 두께, 없으면 -1
  bool
  isWallEstimatedAt(double z) const; // 해당 높이가 Artec 미스캔 추정 구간인지

  QString name() const { return name_; }
  QString path() const { return path_; }
  bool aligned() const { return aligned_; }
  QString alignNote() const { return alignNote_; }
  double axisRmse() const { return axisRmse_; }
  double frontAngle() const { return frontAngle_; }
  HeightRef heightRef() const { return heightRef_; }
  double simplifyTol() const { return simplifyTol_; }
  bool smooth() const { return smooth_; }
  int pointCount() const; // 단면+외곽 점 수 (간략화 후)
  int rawPointCount() const;

  std::vector<SectionSlot> &slotList() { return slotList_; }
  const std::vector<SectionSlot> &slotList() const { return slotList_; }

private:
  void resetSlots();
  void recompute();
  void unconfirm(); // 자세가 바뀜 → 확정 풀기·슬롯 비우기·계산 비우기
  // 회전 p' = c + R(p − c) 를 원본·LOD 에 함께 적용
  void rotateAbout(const double (&R)[3][3], const KA::Point3 &c);
  // 원본·LOD 를 같은 거리만큼 평행이동 (바운딩 박스 갱신)
  void shiftBoth(double dx, double dy, double dz);
  double measureWallBySlice(double z) const;
  void buildSectionLoops();

  std::vector<std::vector<KA::Point2>>
      sectionLoops_; // 실제 Y=0 절단 고리 (x ≥ 0, K-Arch 좌표)
  std::vector<std::vector<KA::Point2>>
      scannedSolidLines_; // 미스캔 보정 시 실선으로 그릴 외벽·상부내벽 구간
  std::vector<std::vector<KA::Point2>>
      estimatedInnerLines_; // 미스캔/홀필 추정 내벽선 (x ≥ 0)
  double estZMin_ = -1.0, estZMax_ = -1.0;

  std::vector<double> wallProfile_;
  double wallZ0_ = 0.0, wallStep_ = 1.0;

  KA::MeshData base_; // 정위치까지 적용된 메시 (정면 회전 전)
  KA::MeshData mesh_;
  KA::MeshData lodBase_;
  KA::MeshData lodMesh_;
  std::vector<QRgb> baseColors_;
  std::vector<QRgb> lodColors_;
  QString scannerProfile_;
  QString unitNote_;
  int removedNoiseFaces_ = 0;

  KA::SilhouetteResult sil_;
  KA::SectionResult sec_;
  KA::Dimensions dims_;
  Dims5 d5_;
  QString name_, path_, alignNote_;
  bool aligned_ = false;
  double axisRmse_ = 0.0;
  double frontAngle_ = 0.0;
  HeightRef heightRef_ = HeightRef::Base;
  double simplifyTol_ = 0.5;
  bool smooth_ = true;
  bool inferInnerWall_ = true;   // Artec 미스캔 내벽/홀필 마개 자동 보정
  double estimatedWallMm_ = 6.8; // 추정 기벽 두께 (mm)
  std::vector<SectionSlot> slotList_;

  // ① 바닥 정하기
  std::vector<KA::Point3> rawVertices_; // 들어온 자세의 정점 (면은 공유)
  KA::MeshData rawLod_;                 // 들어온 자세의 LOD
  std::vector<QRgb> rawLodColors_;
  bool poseConfirmed_ = false;
  bool centered_ = false; // 직전 자세 조작이 자동 세우기(중심 맞춤 끝남)
  // 자동 세우기 결과 — 확정 때 aligned_/alignNote_/axisRmse_ 로 옮긴다
  bool pendingAligned_ = false;
  QString pendingNote_;
  double pendingRmse_ = 0.0;
};

// 선 다듬기 유틸 (Douglas–Peucker, Catmull–Rom)
std::vector<KA::Point2> simplifyDP(const std::vector<KA::Point2> &pts,
                                   double tol, bool closed);
std::vector<KA::Point2> smoothCatmull(const std::vector<KA::Point2> &pts,
                                      bool closed, int sub = 4);

} // namespace studio
