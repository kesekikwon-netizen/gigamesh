/* DongArch3D - X-Ray Blend Shader
 * Phase 5: X-Ray Final Compositing
 *
 * 역할: 모든 레이어를 Alpha Blending으로 합성
 */

#version 330 core

// Input from fullscreen quad
in vec2 vTexCoord;

// Output
out vec4 fragColor;

// Uniforms
uniform sampler2D uLayerTextures[8];  // Layer color textures
uniform int uLayerCount;              // Number of layers (4-8)

void main()
{
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);  // Start with black background

    // Blend all layers (back-to-front)
    for (int i = uLayerCount - 1; i >= 0; i--) {
        vec4 layerColor = texture(uLayerTextures[i], vTexCoord);

        // Alpha blending
        finalColor.rgb = mix(finalColor.rgb, layerColor.rgb, layerColor.a);
        finalColor.a = max(finalColor.a, layerColor.a);
    }

    fragColor = finalColor;
}
