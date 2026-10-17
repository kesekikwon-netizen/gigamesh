/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 5: Heat Method Vertex Shader (GPU Geodesic Distance)
 *
 * 참조: "Parallel and Scalable Heat Methods for Geodesic Distance Computation"
 *       J. Tao et al., 2018, DOI: 10.1109/TPAMI.2019.2933209
 *
 * OpenGL 3.3 Core Profile - Fragment Shader 기반 Multi-pass Rendering
 */

#version 330 core

// Vertex Attributes
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

// Uniforms
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

// Outputs to Fragment Shader
out VertexData {
    vec3 position;       // World space position
    vec3 normal;         // World space normal
    vec2 texCoord;       // Texture coordinates for FBO sampling
} vertexOut;

void main() {
    // Transform to world space
    vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1.0);
    vertexOut.position = worldPosition.xyz;

    // Transform normal to world space
    vertexOut.normal = normalize(normalMatrix * vertexNormal);

    // Pass texture coordinates for multi-pass FBO rendering
    vertexOut.texCoord = vertexTexCoord;

    // Transform to clip space
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
