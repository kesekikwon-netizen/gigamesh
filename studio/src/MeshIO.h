// DongArch3D Studio — Artec 3D 스캐너 특화 고속 메시 로더 & 전처리기
// 지원 포맷:
//   • PLY (ASCII / Binary Little·Big Endian, 정점 RGB 컬러, TextureFile UV
//   주석) • OBJ + MTL + JPG/PNG 텍스처 (고속 스트리밍 파서, 정점 컬러 및 UV
//   텍스처 샘플링) • WRL (Artec Studio VRML 2.0 — Coordinate, Color,
//   TextureCoordinate, ImageTexture) • STL (ASCII / Binary)
// 자동 보정:
//   • 스캔 부스러기(턴테이블·공중 부유 노이즈 조각) 자동 제거
//   • 단위(m / cm → mm) 자동 감지 및 환산
//   • Artec 스캐너 기종(Space Spider / Eva·Leo) 해상도 프로파일 자동 판별
//   • 초고해상도(100만~1,000만 면) 3D 뷰용 고속 공간 클러스터링 LOD 생성
#pragma once
#include "RimDetector.h"

#include <QColor>
#include <QString>
#include <vector>

namespace studio {

struct MeshReadResult {
  DongArch::KArch::MeshData mesh;
  std::vector<QRgb>
      vertexColors; // mesh.vertices 와 1:1 대응 (없으면 비어 있음)
  std::vector<float> vertexQuality; // GigaMesh MSII quality / funcval 스칼라
                                    // (없으면 비어 있음)
  bool ok = false;
  bool hasColor = false;
  bool hasTexture = false;
  bool hasQuality = false;
  double unitScaleApplied = 1.0; // 1.0(mm), 10.0(cm→mm), 1000.0(m→mm)
  int removedNoiseFaces = 0;     // 자동 제거된 부유 노이즈 삼각형 수
  int erodedBorderFaces =
      0; // GigaMesh clean: 경계 잔여 삼각형(2변 이상 개방) 정리 수
  double recommendedDpMm = 0.5;   // 기종별 권장 DP 간략화 간격 (mm)
  double recommendedMsiiMm = 2.0; // 기종별 권장 MSII 문양 살리기 반경 (mm)
  QString scannerProfile; // 예: "Artec Space Spider 급" / "Artec Eva·Leo 급"
  QString unitNote;       // 예: "m→mm 자동환산(×1000)"
  QString message;
};

// Artec SDK (cdsdk.dll / Snx3Device_x64.dll / RAY2API.dll) 런타임 감지 상태
// (재배포 없이 로컬 설치 탐색)
struct ArtecSdkStatus {
  bool sdkFound = false;
  bool snxFound = false;
  bool rayFound = false;
  int connectedDevices = 0;
  QString sdkPath;
  QString koreanSummary; // UI 표시용 100% 한국어 상태 안내
};

ArtecSdkStatus probeArtecSdk();

MeshReadResult readMeshAny(const QString &path);

// 같은 좌표(허용오차 1e-6 × 대각선)의 정점을 합치고 퇴화 삼각형을 버린다 (정점
// 컬러·MSII quality 보존).
void weldVertices(DongArch::KArch::MeshData &m,
                  std::vector<QRgb> *colors = nullptr,
                  std::vector<float> *quality = nullptr);

// 스캐너 턴테이블·부유 노이즈 조각(주 유물 면 수의 2% 미만 분리 성분)을 자동
// 제거한다.
int removeFloatingNoise(DongArch::KArch::MeshData &m,
                        std::vector<QRgb> *colors = nullptr,
                        std::vector<float> *quality = nullptr);

// GigaMesh clean 규칙: 경계에 덜렁거리는 잔여 삼각형(2변 이상이 열린 경계변)을
// 침식 정리한다.
int erodeDanglingBorderFaces(DongArch::KArch::MeshData &m);

// GigaMesh borders 규칙: 열린 경계 루프(구연부 개구부·파손면 테두리)를 정점
// 인덱스 사슬로 추출한다.
std::vector<std::vector<int>>
extractBorderLoops(const DongArch::KArch::MeshData &m);

// 대용량 Artec 메시(수백만 면)를 3D 보조창 회전용 경량 LOD 메시(기본 35,000면
// 이하)로 즉시 축소한다.
void buildLodMesh(const DongArch::KArch::MeshData &src,
                  const std::vector<QRgb> &srcColors,
                  DongArch::KArch::MeshData &lodOut,
                  std::vector<QRgb> &lodColorsOut, size_t targetFaces = 35000);

// GigaMesh 호환 내보내기: .pline (경계 폴리라인), .gltf (glTF 2.0 3D 웹
// 아카이브), _Legacy.ply (품질 스칼라 포함)
bool exportBordersPline(const DongArch::KArch::MeshData &m, const QString &path,
                        QString *err = nullptr);
bool exportGltf2(const DongArch::KArch::MeshData &m,
                 const std::vector<QRgb> &colors, const QString &path,
                 QString *err = nullptr);
bool exportLegacyPly(const DongArch::KArch::MeshData &m,
                     const std::vector<QRgb> &colors,
                     const std::vector<float> &quality, const QString &path,
                     bool binary, QString *err = nullptr);

} // namespace studio
