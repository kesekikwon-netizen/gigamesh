/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "VectorTracer.h"
#include <GigaMesh/mesh/vertex.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

namespace DongArch {
namespace Outline {
namespace Algorithms {

// ============================================================================
// Public Methods
// ============================================================================

bool VectorTracer::traceEdges(std::span<const EdgeRef> edges,
                               const TracingParams& params,
                               TracingResult& result)
{
    if (edges.empty()) {
        std::cerr << "[VectorTracer] No edges to trace" << std::endl;
        return false;
    }

    result.totalEdges = edges.size();
    result.polylines3D.clear();
    result.totalPoints = 0;

    // Step 1: Build Edge Graph (Vertex → connected Edges)
    auto edgeGraph = buildEdgeGraph(edges);

    // Step 2: Contour Following
    std::unordered_set<const Face*> visited;

    for (const auto& edge : edges) {
        // Skip if already visited
        if (visited.count(edge.face) > 0) {
            continue;
        }

        // Follow contour from this edge
        auto polyline = followContour(edge, edgeGraph, visited);

        // Filter short polylines
        if (polyline.size() < static_cast<size_t>(params.minPolylineLength)) {
            continue;
        }

        // Step 3: Simplify if requested
        if (params.applySimplifcation && polyline.size() > 2) {
            polyline = simplifyPolyline(polyline, params.simplificationTolerance);
        }

        if (!polyline.empty()) {
            result.polylines3D.push_back(std::move(polyline));
            result.totalPoints += result.polylines3D.back().size();
        }
    }

    result.polylineCount = result.polylines3D.size();

    // Calculate simplification ratio
    if (params.applySimplifcation) {
        result.simplificationRatio = 100.0 * (1.0 - static_cast<double>(result.totalPoints) /
                                                     static_cast<double>(edges.size() * 2));
    } else {
        result.simplificationRatio = 0.0;
    }

    std::cout << "[VectorTracer] Traced " << result.polylineCount << " polylines "
              << "(" << result.totalPoints << " points from " << edges.size() << " edges)"
              << std::endl;

    return true;
}

bool VectorTracer::traceEdgesWithProjection(std::span<const EdgeRef> edges,
                                             const TracingParams& params,
                                             const QMatrix4x4& viewMatrix,
                                             TracingResult& result)
{
    // First, trace 3D polylines
    if (!traceEdges(edges, params, result)) {
        return false;
    }

    // Then, project to 2D
    result.polylines2D = project3DTo2D(result.polylines3D, viewMatrix);

    return true;
}

std::vector<std::vector<QPointF>> VectorTracer::project3DTo2D(
    std::span<const std::vector<Vector3D>> polylines3D,
    const QMatrix4x4& viewMatrix)
{
    std::vector<std::vector<QPointF>> polylines2D;
    polylines2D.reserve(polylines3D.size());

    for (const auto& polyline3D : polylines3D) {
        std::vector<QPointF> polyline2D;
        polyline2D.reserve(polyline3D.size());

        for (const auto& pt3D : polyline3D) {
            // Convert Vector3D to QVector3D
            QVector3D qpt3D(pt3D.getX(), pt3D.getY(), pt3D.getZ());

            // Apply view matrix
            QVector3D projected = viewMatrix.map(qpt3D);

            // Extract 2D coordinates
            polyline2D.emplace_back(projected.x(), projected.y());
        }

        if (!polyline2D.empty()) {
            polylines2D.push_back(std::move(polyline2D));
        }
    }

    return polylines2D;
}

QMatrix4x4 VectorTracer::createOrthographicViewMatrix(ViewDirection viewDir)
{
    QMatrix4x4 matrix;
    matrix.setToIdentity();

    // Create orthographic projection matrix based on view direction
    // Simple approach: just use identity for now (XY plane projection)
    // For proper implementation, rotate based on ViewDirection

    switch (viewDir) {
        case ViewDirection::TOP:
            // View from top (Y-axis down): X-Z plane
            // Swap Y ↔ Z
            matrix.rotate(-90, 1, 0, 0);
            break;

        case ViewDirection::BOTTOM:
            // View from bottom (Y-axis up): X-Z plane
            matrix.rotate(90, 1, 0, 0);
            break;

        case ViewDirection::FRONT:
            // View from front (Z-axis down): X-Y plane
            // Identity (default)
            break;

        case ViewDirection::BACK:
            // View from back (Z-axis up): X-Y plane
            matrix.rotate(180, 0, 1, 0);
            break;

        case ViewDirection::RIGHT:
            // View from right (X-axis left): Y-Z plane
            matrix.rotate(90, 0, 1, 0);
            break;

        case ViewDirection::LEFT:
            // View from left (X-axis right): Y-Z plane
            matrix.rotate(-90, 0, 1, 0);
            break;
    }

    return matrix;
}

// ============================================================================
// Private Methods
// ============================================================================

std::unordered_map<Vertex*, std::vector<EdgeRef>> VectorTracer::buildEdgeGraph(
    std::span<const EdgeRef> edges)
{
    std::unordered_map<Vertex*, std::vector<EdgeRef>> graph;

    for (const auto& edge : edges) {
        Vertex* v1 = nullptr;
        Vertex* v2 = nullptr;

        if (edge.getVertices(v1, v2)) {
            graph[v1].push_back(edge);
            graph[v2].push_back(edge);
        }
    }

    return graph;
}

std::vector<Vector3D> VectorTracer::followContour(
    const EdgeRef& startEdge,
    const std::unordered_map<Vertex*, std::vector<EdgeRef>>& edgeGraph,
    std::unordered_set<const Face*>& visited)
{
    std::vector<Vector3D> polyline;

    // Mark starting edge as visited
    visited.insert(startEdge.face);

    // Get starting vertices
    Vertex* v1 = nullptr;
    Vertex* v2 = nullptr;

    if (!startEdge.getVertices(v1, v2)) {
        return polyline;
    }

    // Add first two vertices
    polyline.push_back(v1->getPositionVector());
    polyline.push_back(v2->getPositionVector());

    // Follow contour from v2
    Vertex* currentVertex = v2;
    Vertex* previousVertex = v1;

    // Contour following (max 10000 iterations to prevent infinite loops)
    const size_t MAX_ITERATIONS = 10000;
    size_t iteration = 0;

    while (iteration < MAX_ITERATIONS) {
        iteration++;

        // Find next edge connected to currentVertex
        auto it = edgeGraph.find(currentVertex);
        if (it == edgeGraph.end()) {
            break;
        }

        const auto& connectedEdges = it->second;
        bool foundNext = false;

        for (const auto& edge : connectedEdges) {
            // Skip if already visited
            if (visited.count(edge.face) > 0) {
                continue;
            }

            // Get vertices of this edge
            Vertex* e1 = nullptr;
            Vertex* e2 = nullptr;

            if (!edge.getVertices(e1, e2)) {
                continue;
            }

            // Determine next vertex (the one that's not previousVertex or currentVertex)
            Vertex* nextVertex = nullptr;

            if (e1 == currentVertex && e2 != previousVertex) {
                nextVertex = e2;
            } else if (e2 == currentVertex && e1 != previousVertex) {
                nextVertex = e1;
            }

            if (nextVertex) {
                // Add next vertex to polyline
                polyline.push_back(nextVertex->getPositionVector());

                // Update state
                visited.insert(edge.face);
                previousVertex = currentVertex;
                currentVertex = nextVertex;
                foundNext = true;
                break;
            }
        }

        if (!foundNext) {
            // No more connected edges - end of contour
            break;
        }
    }

    return polyline;
}

std::vector<Vector3D> VectorTracer::simplifyPolyline(
    std::span<const Vector3D> points,
    double tolerance)
{
    if (points.size() <= 2) {
        return std::vector<Vector3D>(points.begin(), points.end());
    }

    // Douglas-Peucker algorithm
    std::vector<bool> keep(points.size(), false);
    keep[0] = true;
    keep[points.size() - 1] = true;

    douglasPeuckerRecursive(points, 0, points.size() - 1, tolerance, keep);

    // Build result
    std::vector<Vector3D> result;
    for (size_t i = 0; i < points.size(); i++) {
        if (keep[i]) {
            result.push_back(points[i]);
        }
    }

    return result;
}

void VectorTracer::douglasPeuckerRecursive(
    std::span<const Vector3D> points,
    size_t startIdx,
    size_t endIdx,
    double tolerance,
    std::vector<bool>& keep)
{
    if (endIdx <= startIdx + 1) {
        return;
    }

    // Find point with maximum distance
    double maxDist = 0.0;
    size_t maxIdx = startIdx;

    for (size_t i = startIdx + 1; i < endIdx; i++) {
        double dist = pointToLineDistance(points[i], points[startIdx], points[endIdx]);
        if (dist > maxDist) {
            maxDist = dist;
            maxIdx = i;
        }
    }

    // If max distance exceeds tolerance, keep the point and recurse
    if (maxDist > tolerance) {
        keep[maxIdx] = true;
        douglasPeuckerRecursive(points, startIdx, maxIdx, tolerance, keep);
        douglasPeuckerRecursive(points, maxIdx, endIdx, tolerance, keep);
    }
}

double VectorTracer::pointToLineDistance(
    const Vector3D& point,
    const Vector3D& lineStart,
    const Vector3D& lineEnd)
{
    // Vector from lineStart to lineEnd
    double dx = lineEnd.getX() - lineStart.getX();
    double dy = lineEnd.getY() - lineStart.getY();
    double dz = lineEnd.getZ() - lineStart.getZ();

    double lineLengthSq = dx * dx + dy * dy + dz * dz;

    if (lineLengthSq < 1e-10) {
        // Line is a point
        double px = point.getX() - lineStart.getX();
        double py = point.getY() - lineStart.getY();
        double pz = point.getZ() - lineStart.getZ();
        return std::sqrt(px * px + py * py + pz * pz);
    }

    // Vector from lineStart to point
    double px = point.getX() - lineStart.getX();
    double py = point.getY() - lineStart.getY();
    double pz = point.getZ() - lineStart.getZ();

    // Project point onto line
    double t = (px * dx + py * dy + pz * dz) / lineLengthSq;
    t = std::max(0.0, std::min(1.0, t));

    // Closest point on line segment
    double closestX = lineStart.getX() + t * dx;
    double closestY = lineStart.getY() + t * dy;
    double closestZ = lineStart.getZ() + t * dz;

    // Distance from point to closest point
    double distX = point.getX() - closestX;
    double distY = point.getY() - closestY;
    double distZ = point.getZ() - closestZ;

    return std::sqrt(distX * distX + distY * distY + distZ * distZ);
}

} // namespace Algorithms
} // namespace Outline
} // namespace DongArch
