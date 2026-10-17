/* DongArch3D - X-Ray Vertex Shader
 * Phase 5: X-Ray Depth Peeling
 *
 * 역할: 버텍스를 MVP 변환하여 클립 공간으로 변환
 */

#version 330 core

// Input
layout(location = 0) in vec3 aPosition;  // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal
layout(location = 2) in vec4 aColor;     // Vertex color (optional)

// Output to fragment shader
out vec3 vPosition;    // World space position
out vec3 vNormal;      // World space normal
out vec4 vColor;       // Vertex color
out float vDepth;      // View-space depth

// Uniforms
uniform mat4 uMVPMatrix;       // Model-View-Projection matrix
uniform mat4 uModelMatrix;     // Model matrix
uniform mat4 uViewMatrix;      // View matrix

void main()
{
    // Transform to clip space
    gl_Position = uMVPMatrix * vec4(aPosition, 1.0);

    // Pass world space position and normal
    vPosition = (uModelMatrix * vec4(aPosition, 1.0)).xyz;
    vNormal = normalize((uModelMatrix * vec4(aNormal, 0.0)).xyz);

    // Pass color
    vColor = aColor;

    // Calculate view-space depth for X-Ray effect
    vec4 viewPos = uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
    vDepth = -viewPos.z;  // Negative z is forward in OpenGL
}
