/* DongArch3D / K-ArchProfile T1 mesh I/O (GPLv3+) */

#ifndef DONGARCH_KARCH_MESH_LOADER_H
#define DONGARCH_KARCH_MESH_LOADER_H

#include "RimDetector.h"

#include <string>

namespace DongArch {
namespace KArch {

enum class LoadError {
    Ok = 0,
    FileNotFound,
    ParseFailed,
    EmptyMesh,
    UnsupportedFormat
};

struct LoadResult {
    MeshData mesh;
    LoadError error = LoadError::FileNotFound;
    std::string message;
};

// .ply (ASCII) or .obj. unitScale multiplies vertex coords (e.g. m→mm = 1000).
// Weld 1e-6*bbox_diag, drop degenerate faces, fill bbox/meanEdge/hasBoundary.
// Never throws.
LoadResult LoadMesh(const std::string& path, double unitScale = 1.0);
void FillMeshStats(MeshData& mesh);
bool WriteObjAscii(const MeshData& mesh, const std::string& path);

} // namespace KArch
} // namespace DongArch

#endif
