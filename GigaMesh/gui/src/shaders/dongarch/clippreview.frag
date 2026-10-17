/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 4: Clipping Plane Preview Fragment Shader
 *
 * Features:
 * - Hardware clipping: gl_ClipDistance[0] < 0 → automatic discard
 * - Edge highlighting: abs(clipDistance) < threshold → highlight color
 * - Phong lighting: Diffuse + Specular
 *
 * OpenGL 3.3 Core Profile
 */

#version 330 core

// Inputs from Vertex Shader
in VertexData {
    vec3 position;       // World space position
    vec3 normal;         // World space normal
    vec4 color;          // Vertex color
    float clipDistance;  // Distance to clipping plane
} fragmentIn;

// Output
out vec4 fragColor;

// Uniforms
uniform vec3 cameraPosition;           // Camera position (world space)
uniform vec3 lightDirection;           // Light direction (world space)
uniform vec4 edgeHighlightColor;       // Color for clipping plane edge
uniform float edgeThickness;           // Thickness of edge highlighting (default: 0.1)
uniform bool enableEdgeHighlight;      // Enable edge highlighting
uniform float ambientStrength;         // Ambient light strength (default: 0.3)
uniform float specularStrength;        // Specular light strength (default: 0.5)
uniform float shininess;               // Shininess (default: 32.0)

void main() {
    // Note: gl_ClipDistance[0] < 0 fragments are automatically discarded by OpenGL
    // We only need to handle edge highlighting and lighting

    // 1. Check if fragment is near clipping plane edge
    bool isEdge = enableEdgeHighlight && (abs(fragmentIn.clipDistance) < edgeThickness);

    if (isEdge) {
        // Edge highlighting: Use highlight color
        fragColor = edgeHighlightColor;
        return;
    }

    // 2. Phong Lighting for non-edge fragments
    vec3 normal = normalize(fragmentIn.normal);
    vec3 viewDir = normalize(cameraPosition - fragmentIn.position);
    vec3 lightDir = normalize(-lightDirection);  // Directional light

    // Ambient
    vec3 ambient = ambientStrength * fragmentIn.color.rgb;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * fragmentIn.color.rgb;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    // Combine
    vec3 result = ambient + diffuse + specular;

    // Optional: Add distance-based fading near clipping plane
    // float fadeFactor = smoothstep(0.0, edgeThickness * 2.0, abs(fragmentIn.clipDistance));
    // result = mix(edgeHighlightColor.rgb, result, fadeFactor);

    fragColor = vec4(result, fragmentIn.color.a);
}
