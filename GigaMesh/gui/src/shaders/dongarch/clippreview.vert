/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 4: Clipping Plane Preview Vertex Shader
 *
 * OpenGL 3.3 Core Profile
 */

#version 330 core

// Vertex Attributes
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec4 vertexColor;

// Uniforms
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

// Clipping Plane (Hesse Normal Form: ax + by + cz + d = 0)
uniform vec4 clipPlane;  // (a, b, c, d)

// Outputs to Fragment Shader
out VertexData {
    vec3 position;       // World space position
    vec3 normal;         // World space normal
    vec4 color;          // Vertex color
    float clipDistance;  // Distance to clipping plane
} vertexOut;

// OpenGL Built-in: Clip distance for hardware clipping
// gl_ClipDistance[0] will be set to distance from plane

void main() {
    // Transform to world space
    vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1.0);
    vertexOut.position = worldPosition.xyz;

    // Transform normal to world space
    vertexOut.normal = normalMatrix * vertexNormal;

    // Pass through color
    vertexOut.color = vertexColor;

    // Calculate signed distance to clipping plane
    // Plane equation: ax + by + cz + d = 0
    // Distance = a*x + b*y + c*z + d
    float distance = dot(clipPlane.xyz, worldPosition.xyz) + clipPlane.w;
    vertexOut.clipDistance = distance;

    // Set built-in clip distance (OpenGL 3.3+)
    // Fragments with gl_ClipDistance[0] < 0 will be discarded automatically
    gl_ClipDistance[0] = distance;

    // Transform to clip space
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
