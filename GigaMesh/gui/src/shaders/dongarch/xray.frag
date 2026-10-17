/* DongArch3D - X-Ray Fragment Shader (Depth Peeling)
 * Phase 5: X-Ray Depth Peeling
 *
 * 참조: "Dual Depth Peeling" (Bavoil & Myers, 2008)
 *
 * Depth Peeling:
 * - Layer 0: 가장 앞쪽 표면
 * - Layer N: N-1번째 레이어보다 뒤에 있는 표면
 *
 * X-Ray 효과:
 * - 깊이에 따라 투명도 조절
 * - 두께에 따라 밝기 조절
 */

#version 330 core

// Input from vertex shader
in vec3 vPosition;
in vec3 vNormal;
in vec4 vColor;
in float vDepth;

// Output
out vec4 fragColor;

// Uniforms
uniform int uLayerIndex;           // Current layer index (0-7)
uniform float uOpacity;            // Global opacity (0.0-1.0)
uniform int uColorMode;            // Color mode (0-3)

// Previous layer depth texture
uniform sampler2D uPrevDepthTex;   // Previous layer depth
uniform bool uUsePrevDepth;        // Use previous depth (false for layer 0)

// Constants
const vec3 XRAY_WHITE = vec3(1.0, 1.0, 1.0);
const vec3 XRAY_BLACK = vec3(0.0, 0.0, 0.0);
const vec3 BACKGROUND_BLACK = vec3(0.0, 0.0, 0.0);
const vec3 BACKGROUND_WHITE = vec3(1.0, 1.0, 1.0);

void main()
{
    // Depth Peeling: Discard if behind previous layer
    if (uUsePrevDepth) {
        vec2 screenCoord = gl_FragCoord.xy / vec2(textureSize(uPrevDepthTex, 0));
        float prevDepth = texture(uPrevDepthTex, screenCoord).r;

        // Discard if not deeper than previous layer
        if (gl_FragCoord.z <= prevDepth + 0.00001) {
            discard;
        }
    }

    // X-Ray opacity calculation
    // - 더 깊은 레이어일수록 더 투명
    // - 표면 법선과 뷰 방향의 각도에 따라 투명도 조절
    vec3 viewDir = normalize(-vPosition);
    float fresnel = abs(dot(vNormal, viewDir));  // 0 = edge, 1 = center

    // Edge glow effect (edges are more visible in X-Ray)
    float edgeFactor = 1.0 - fresnel;
    edgeFactor = pow(edgeFactor, 2.0);  // Sharpen edge

    // Layer-based opacity (deeper layers are more transparent)
    float layerOpacity = 1.0 - (float(uLayerIndex) / 8.0);
    layerOpacity = mix(0.5, 1.0, layerOpacity);  // Clamp to 0.5-1.0

    // Final opacity
    float alpha = uOpacity * layerOpacity * (0.3 + 0.7 * edgeFactor);

    // Color modes
    vec3 color;

    if (uColorMode == 0) {
        // WhiteOnBlack
        color = XRAY_WHITE;

    } else if (uColorMode == 1) {
        // BlackOnWhite
        color = XRAY_BLACK;

    } else if (uColorMode == 2) {
        // DepthGradient
        float depthFactor = clamp(vDepth / 100.0, 0.0, 1.0);
        color = mix(vec3(0.0, 0.5, 1.0), vec3(1.0, 0.0, 0.0), depthFactor);

    } else {
        // Thickness (based on layer index)
        float thicknessFactor = float(uLayerIndex) / 8.0;
        color = mix(vec3(0.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0), thicknessFactor);
    }

    // Edge enhancement
    color = mix(color, vec3(1.0, 1.0, 1.0), edgeFactor * 0.3);

    // Output
    fragColor = vec4(color, alpha);
}
