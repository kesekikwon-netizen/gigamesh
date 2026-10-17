/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 3-C: GPU Geometry Shader Silhouette Detection (2008)
 *
 * Geometry Shader for Silhouette Edge Detection
 * Based on:
 * - "Silhouette Smoothing for Real-Time Rendering of Mesh Surfaces" (2008)
 *   by Lu Wang et al., DOI: 10.1109/TVCG.2008.8
 * - "A Highly Parallelized Approach to Silhouette Edge Detection" (2008)
 *   by Chad Mourning
 *
 * OpenGL 3.3 Core Profile
 *
 * Algorithm:
 *   For each edge (v0, v1):
 *     face1 = triangle containing (v0, v1)
 *     face2 = adjacent triangle sharing (v0, v1)
 *
 *     if (face1.normal · viewDir) * (face2.normal · viewDir) < 0:
 *       Emit line (v0, v1) as silhouette edge
 *
 * Performance: 10-100x faster than CPU (GPU parallel processing)
 */

#version 330 core

// Input: Triangles with adjacency information
// Layout: [v0, v1_adj, v2, v3_adj, v4, v5_adj]
//   Main triangle: v0, v2, v4
//   Adjacent vertices: v1_adj (opposite to edge v0-v2)
//                      v3_adj (opposite to edge v2-v4)
//                      v5_adj (opposite to edge v4-v0)
layout(triangles_adjacency) in;

// Output: Lines (silhouette edges)
layout(line_strip, max_vertices = 6) out;

// Input from Vertex Shader
in VertexData {
    vec3 position;    // Vertex position (world space)
    vec3 normal;      // Vertex normal (world space)
} vertexIn[6];

// Output to Fragment Shader
out FragmentData {
    vec3 position;       // Edge position (world space)
    vec3 edgeDirection;  // Edge direction for rendering
    float edgeLength;    // Edge length
} fragmentOut;

// Uniforms
uniform mat4 viewMatrix;          // View transformation
uniform mat4 projectionMatrix;    // Projection transformation
uniform vec3 cameraPosition;      // Camera position (world space)
uniform bool useViewDirection;    // true: view direction, false: camera position

// Compute face normal from 3 vertices
vec3 computeFaceNormal(vec3 v0, vec3 v1, vec3 v2) {
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    return normalize(cross(edge1, edge2));
}

// Check if edge (v0, v1) with adjacent vertices v2 (main face) and v3 (adjacent face) is silhouette
bool isSilhouetteEdge(vec3 v0, vec3 v1, vec3 v2, vec3 v3) {
    // Compute face normals
    vec3 normal1 = computeFaceNormal(v0, v1, v2);  // Main face
    vec3 normal2 = computeFaceNormal(v0, v1, v3);  // Adjacent face

    // View direction
    vec3 viewDir;
    if (useViewDirection) {
        // Orthographic projection: view direction is constant
        viewDir = normalize(vec3(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]));
    } else {
        // Perspective projection: view direction from edge midpoint to camera
        vec3 edgeMidpoint = (v0 + v1) * 0.5;
        viewDir = normalize(cameraPosition - edgeMidpoint);
    }

    // Dot products
    float dot1 = dot(normal1, viewDir);
    float dot2 = dot(normal2, viewDir);

    // Silhouette condition: (n1 · view) * (n2 · view) < 0
    // One face is front-facing, the other is back-facing
    return (dot1 * dot2) < 0.0;
}

// Emit silhouette edge as line
void emitSilhouetteEdge(int idx0, int idx1) {
    vec3 v0 = vertexIn[idx0].position;
    vec3 v1 = vertexIn[idx1].position;

    // Edge properties
    vec3 edgeDir = v1 - v0;
    float edgeLen = length(edgeDir);
    edgeDir = normalize(edgeDir);

    // Emit first vertex
    fragmentOut.position = v0;
    fragmentOut.edgeDirection = edgeDir;
    fragmentOut.edgeLength = edgeLen;
    gl_Position = projectionMatrix * viewMatrix * vec4(v0, 1.0);
    EmitVertex();

    // Emit second vertex
    fragmentOut.position = v1;
    fragmentOut.edgeDirection = edgeDir;
    fragmentOut.edgeLength = edgeLen;
    gl_Position = projectionMatrix * viewMatrix * vec4(v1, 1.0);
    EmitVertex();

    // End line strip
    EndPrimitive();
}

void main() {
    // Extract main triangle vertices
    vec3 v0 = vertexIn[0].position;  // Vertex 0
    vec3 v2 = vertexIn[2].position;  // Vertex 2
    vec3 v4 = vertexIn[4].position;  // Vertex 4

    // Extract adjacent vertices (opposite to each edge)
    vec3 v1_adj = vertexIn[1].position;  // Adjacent to edge v0-v2
    vec3 v3_adj = vertexIn[3].position;  // Adjacent to edge v2-v4
    vec3 v5_adj = vertexIn[5].position;  // Adjacent to edge v4-v0

    // Check each edge of the main triangle
    // Edge 0: v0 - v2 (adjacent vertex: v1_adj)
    if (isSilhouetteEdge(v0, v2, v4, v1_adj)) {
        emitSilhouetteEdge(0, 2);
    }

    // Edge 1: v2 - v4 (adjacent vertex: v3_adj)
    if (isSilhouetteEdge(v2, v4, v0, v3_adj)) {
        emitSilhouetteEdge(2, 4);
    }

    // Edge 2: v4 - v0 (adjacent vertex: v5_adj)
    if (isSilhouetteEdge(v4, v0, v2, v5_adj)) {
        emitSilhouetteEdge(4, 0);
    }
}
