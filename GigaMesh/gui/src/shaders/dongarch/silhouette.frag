/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 3-C: GPU Geometry Shader Silhouette Detection (2008)
 *
 * Fragment Shader for Silhouette Edge Rendering
 * Based on:
 * - "Silhouette Smoothing for Real-Time Rendering of Mesh Surfaces" (2008)
 *   by Lu Wang et al., DOI: 10.1109/TVCG.2008.8
 *
 * OpenGL 3.3 Core Profile
 */

#version 330 core

// Input from Geometry Shader
in FragmentData {
    vec3 position;       // Edge position (world space)
    vec3 edgeDirection;  // Edge direction for rendering
    float edgeLength;    // Edge length
} fragmentIn;

// Output: Fragment color
out vec4 fragColor;

// Uniforms
uniform vec3 silhouetteColor;     // Silhouette edge color (default: black)
uniform float lineWidth;           // Line width (default: 2.0)
uniform bool useSmoothing;         // Enable edge smoothing (2008 paper)
uniform float smoothingFactor;     // Smoothing factor (0.0 - 1.0)

void main() {
    // Base silhouette color
    vec3 color = silhouetteColor;

    // Optional: Edge smoothing (2008 paper feature)
    if (useSmoothing) {
        // Distance from edge center (for anti-aliasing)
        float dist = abs(gl_FragCoord.z - 0.5);
        float alpha = 1.0 - smoothstep(0.0, smoothingFactor, dist);
        fragColor = vec4(color, alpha);
    } else {
        // Solid color
        fragColor = vec4(color, 1.0);
    }

    // Debug: Visualize edge length (optional)
    // Uncomment to see edge length as color intensity
    // float intensity = clamp(fragmentIn.edgeLength / 10.0, 0.0, 1.0);
    // fragColor = vec4(vec3(intensity), 1.0);
}
