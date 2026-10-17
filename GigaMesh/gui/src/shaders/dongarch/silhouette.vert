/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 3-C: GPU Geometry Shader Silhouette Detection (2008)
 *
 * Vertex Shader for Silhouette Edge Detection
 * Based on:
 * - "Silhouette Smoothing for Real-Time Rendering of Mesh Surfaces" (2008)
 *   by Lu Wang et al., DOI: 10.1109/TVCG.2008.8
 * - "A Highly Parallelized Approach to Silhouette Edge Detection" (2008)
 *   by Chad Mourning
 *
 * OpenGL 3.3 Core Profile
 */

#version 330 core

// Input: Vertex attributes
layout(location = 0) in vec3 vertexPosition;   // Vertex position
layout(location = 1) in vec3 vertexNormal;     // Vertex normal

// Output to Geometry Shader
out VertexData {
    vec3 position;    // Vertex position (world space)
    vec3 normal;      // Vertex normal (world space)
} vertexOut;

// Uniforms
uniform mat4 modelMatrix;         // Model transformation
uniform mat4 viewMatrix;          // View transformation
uniform mat4 projectionMatrix;    // Projection transformation
uniform mat3 normalMatrix;        // Normal matrix (inverse transpose of model-view)

void main() {
    // Transform vertex position to world space
    vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1.0);
    vertexOut.position = worldPosition.xyz;

    // Transform normal to world space
    vertexOut.normal = normalMatrix * vertexNormal;

    // Transform to clip space for rasterization
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
