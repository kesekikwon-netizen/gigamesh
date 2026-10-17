/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 5: Heat Method Fragment Shader (GPU Geodesic Distance)
 *
 * 참조: "Parallel and Scalable Heat Methods for Geodesic Distance Computation"
 *       J. Tao et al., 2018, DOI: 10.1109/TPAMI.2019.2933209
 *       "Geodesics in Heat: A New Approach to Computing Distance"
 *       K. Crane et al., 2013, DOI: 10.1145/2516971.2516977
 *
 * Heat Method Algorithm:
 * 1. Heat Diffusion: ∂u/∂t = Δu (solve: u = u₀ * e^(t*Δ))
 * 2. Normalize Gradient: X = -∇u / |∇u|
 * 3. Poisson Equation: Δφ = ∇·X (φ = geodesic distance)
 *
 * Multi-pass Rendering:
 * - Pass 0: Heat source initialization
 * - Pass 1-N: Heat diffusion iterations
 * - Pass Final: Distance extraction
 *
 * OpenGL 3.3 Core Profile
 */

#version 330 core

// Inputs from Vertex Shader
in VertexData {
    vec3 position;       // World space position
    vec3 normal;         // World space normal
    vec2 texCoord;       // Texture coordinates
} fragmentIn;

// Outputs
out vec4 fragColor;

// Uniforms - Rendering Mode
uniform int renderMode;  // 0: Init, 1: Diffusion, 2: Distance

// Uniforms - Heat Source
uniform vec3 heatSourcePosition;  // Source vertex position (world space)
uniform float heatSourceRadius;   // Radius for source detection (default: 0.01)

// Uniforms - Heat Diffusion
uniform sampler2D heatTexture;         // Previous pass heat values
uniform float timeStep;                // Time step for diffusion (default: 0.1)
uniform float meshAverageEdgeLength;   // Average edge length for Laplacian
uniform int diffusionIterations;       // Current iteration number

// Uniforms - Texture Sampling
uniform vec2 texelSize;  // 1.0 / textureSize (for neighbor sampling)

// Uniforms - Distance Extraction
uniform sampler2D gradientTexture;  // Gradient field texture

// Constants
const float EPSILON = 1e-6;
const float PI = 3.14159265359;

//==============================================================================
// Pass 0: Heat Source Initialization
//==============================================================================
vec4 initializeHeatSource() {
    // Check if current fragment is near heat source
    float dist = distance(fragmentIn.position, heatSourcePosition);

    if (dist < heatSourceRadius) {
        // Heat source vertex: set heat to 1.0
        return vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        // Other vertices: set heat to 0.0
        return vec4(0.0, 0.0, 0.0, 1.0);
    }
}

//==============================================================================
// Pass 1-N: Heat Diffusion (∂u/∂t = Δu)
//==============================================================================
vec4 diffuseHeat() {
    // Sample current heat value
    vec2 uv = fragmentIn.texCoord;
    float u_center = texture(heatTexture, uv).r;

    // Compute discrete Laplacian using 5-point stencil (2D) or 7-point (3D)
    // Δu ≈ (u_neighbors - u_center) / h²
    // For texture-based: use texel neighbors

    float u_left   = texture(heatTexture, uv + vec2(-texelSize.x, 0.0)).r;
    float u_right  = texture(heatTexture, uv + vec2( texelSize.x, 0.0)).r;
    float u_top    = texture(heatTexture, uv + vec2(0.0,  texelSize.y)).r;
    float u_bottom = texture(heatTexture, uv + vec2(0.0, -texelSize.y)).r;

    // 5-point Laplacian (2D approximation)
    float laplacian = (u_left + u_right + u_top + u_bottom - 4.0 * u_center);

    // Optional: 9-point stencil for better accuracy
    float u_tl = texture(heatTexture, uv + vec2(-texelSize.x,  texelSize.y)).r;
    float u_tr = texture(heatTexture, uv + vec2( texelSize.x,  texelSize.y)).r;
    float u_bl = texture(heatTexture, uv + vec2(-texelSize.x, -texelSize.y)).r;
    float u_br = texture(heatTexture, uv + vec2( texelSize.x, -texelSize.y)).r;

    // 9-point Laplacian with diagonal weights (0.5)
    laplacian += 0.5 * (u_tl + u_tr + u_bl + u_br - 4.0 * u_center);
    laplacian /= (meshAverageEdgeLength * meshAverageEdgeLength);

    // Forward Euler integration: u_{n+1} = u_n + dt * Δu
    float u_new = u_center + timeStep * laplacian;

    // Clamp to [0, 1] to maintain stability
    u_new = clamp(u_new, 0.0, 1.0);

    return vec4(u_new, 0.0, 0.0, 1.0);
}

//==============================================================================
// Helper: Compute Gradient (∇u)
//==============================================================================
vec3 computeGradient(sampler2D tex, vec2 uv) {
    // Central difference for gradient
    // ∇u ≈ (u(x+h) - u(x-h)) / 2h

    float u_left   = texture(tex, uv + vec2(-texelSize.x, 0.0)).r;
    float u_right  = texture(tex, uv + vec2( texelSize.x, 0.0)).r;
    float u_top    = texture(tex, uv + vec2(0.0,  texelSize.y)).r;
    float u_bottom = texture(tex, uv + vec2(0.0, -texelSize.y)).r;

    vec3 gradient;
    gradient.x = (u_right - u_left) / (2.0 * texelSize.x);
    gradient.y = (u_top - u_bottom) / (2.0 * texelSize.y);
    gradient.z = 0.0;  // 2D texture approximation

    return gradient;
}

//==============================================================================
// Pass Final: Distance Extraction (Δφ = ∇·X, where X = -∇u / |∇u|)
//==============================================================================
vec4 extractDistance() {
    vec2 uv = fragmentIn.texCoord;

    // 1. Compute heat gradient: ∇u
    vec3 gradU = computeGradient(heatTexture, uv);

    // 2. Normalize gradient: X = -∇u / |∇u|
    float gradMag = length(gradU);
    vec3 X = vec3(0.0);
    if (gradMag > EPSILON) {
        X = -gradU / gradMag;
    }

    // 3. Solve Poisson equation: Δφ = ∇·X
    // Discrete divergence: ∇·X ≈ (X_right.x - X_left.x) / 2h + (X_top.y - X_bottom.y) / 2h

    vec3 X_left   = computeGradient(heatTexture, uv + vec2(-texelSize.x, 0.0));
    vec3 X_right  = computeGradient(heatTexture, uv + vec2( texelSize.x, 0.0));
    vec3 X_top    = computeGradient(heatTexture, uv + vec2(0.0,  texelSize.y));
    vec3 X_bottom = computeGradient(heatTexture, uv + vec2(0.0, -texelSize.y));

    // Normalize neighbors
    if (length(X_left) > EPSILON)   X_left   = -X_left   / length(X_left);
    if (length(X_right) > EPSILON)  X_right  = -X_right  / length(X_right);
    if (length(X_top) > EPSILON)    X_top    = -X_top    / length(X_top);
    if (length(X_bottom) > EPSILON) X_bottom = -X_bottom / length(X_bottom);

    // Divergence
    float divX = (X_right.x - X_left.x) / (2.0 * texelSize.x)
               + (X_top.y - X_bottom.y) / (2.0 * texelSize.y);

    // 4. Solve for distance using Laplacian inverse (Poisson solver)
    // For simplicity, use direct integration (approximation)
    // In practice, need iterative solver (Jacobi, Gauss-Seidel, or Multigrid)

    // Simplified: Use heat value as distance proxy (needs refinement)
    float distance = texture(heatTexture, uv).r;

    // Better approximation: Integrate divergence
    // φ ≈ φ_neighbors + divX * h²
    float phi_left   = texture(gradientTexture, uv + vec2(-texelSize.x, 0.0)).r;
    float phi_right  = texture(gradientTexture, uv + vec2( texelSize.x, 0.0)).r;
    float phi_top    = texture(gradientTexture, uv + vec2(0.0,  texelSize.y)).r;
    float phi_bottom = texture(gradientTexture, uv + vec2(0.0, -texelSize.y)).r;

    float phi = (phi_left + phi_right + phi_top + phi_bottom - divX) / 4.0;

    return vec4(phi, distance, gradMag, 1.0);
}

//==============================================================================
// Main
//==============================================================================
void main() {
    if (renderMode == 0) {
        // Pass 0: Initialize heat source
        fragColor = initializeHeatSource();
    } else if (renderMode == 1) {
        // Pass 1-N: Heat diffusion
        fragColor = diffuseHeat();
    } else if (renderMode == 2) {
        // Pass Final: Extract geodesic distance
        fragColor = extractDistance();
    } else {
        // Invalid mode: output error color
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);  // Magenta
    }
}
