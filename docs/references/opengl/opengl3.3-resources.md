# OpenGL 3.3 Core Profile 리소스 및 문서

DongArch3D 프로젝트를 위한 OpenGL 3.3 핵심 기술 문서 모음

## 프로젝트 OpenGL 정보

- **OpenGL 버전**: 3.3 Core Profile
- **GLSL 버전**: 3.30
- **통합**: QOpenGLWidget + QOpenGLFunctions_3_3_Core
- **렌더링 기법**: NPR (Non-Photorealistic Rendering)

## OpenGL 3.3 Core Profile 개요

### 고정 파이프라인 제거
OpenGL 3.3 Core Profile은 고정 파이프라인 함수를 제거하고 셰이더 기반으로만 작동합니다.

**제거된 함수들** (사용 금지):
```cpp
// ❌ 더 이상 사용 불가
glBegin()/ glEnd()
glVertex*()
glColor*()
glNormal*()
glMatrixMode()
glLoadIdentity()
glPushMatrix()/ glPopMatrix()
```

**대체 방법** (VBO + 셰이더):
```cpp
// ✅ 올바른 방법
GLuint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```

## 핵심 개념

### 1. VBO (Vertex Buffer Objects)
GPU 메모리에 정점 데이터 저장

**DongArch3D 적용** (meshGL.cpp 기반):
```cpp
// Mesh 데이터를 VBO로 전송
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

std::vector<Vertex> vertices;
// ... 메시 데이터 채우기

GLuint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER,
             vertices.size() * sizeof(Vertex),
             vertices.data(),
             GL_STATIC_DRAW);
```

### 2. VAO (Vertex Array Objects)
정점 속성 레이아웃 관리

```cpp
GLuint vao;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);

// 정점 위치 속성 (location = 0)
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                      sizeof(Vertex), (void*)0);

// 정점 법선 속성 (location = 1)
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                      sizeof(Vertex), (void*)offsetof(Vertex, normal));

// 텍스처 좌표 속성 (location = 2)
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                      sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
```

### 3. GLSL 셰이더
GPU에서 실행되는 프로그램

**Vertex Shader** (예: GigaMesh/gui/src/shaders/NPR/NPR_hatches.vert):
```glsl
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 vTexCoord;

uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat3 uNormalMatrix;

void main() {
    gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0);
    vFragPos = vec3(uModelMatrix * vec4(aPosition, 1.0));
    vNormal = uNormalMatrix * aNormal;
    vTexCoord = aTexCoord;
}
```

**Fragment Shader** (예: GigaMesh/gui/src/shaders/NPR/NPR_hatches.frag):
```glsl
#version 330 core

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoord;

out vec4 FragColor;

uniform vec3 uLightDirection;
uniform vec3 uViewPosition;
uniform sampler2D uHatchTexture;

void main() {
    // Lambertian 조명 계산
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDirection);
    float diffuse = max(dot(N, L), 0.0);

    // Hatching 밀도 결정
    vec4 hatchColor = texture(uHatchTexture, vTexCoord);
    FragColor = vec4(mix(hatchColor.rgb, vec3(1.0), diffuse), 1.0);
}
```

### 4. Uniform 변수
CPU에서 셰이더로 데이터 전달

**DongArch3D 적용** (meshGLShader.cpp 기반):
```cpp
// 셰이더 프로그램 활성화
glUseProgram(shaderProgram);

// Uniform 변수 설정
GLint mvpLoc = glGetUniformLocation(shaderProgram, "uModelViewProjectionMatrix");
glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

GLint lightLoc = glGetUniformLocation(shaderProgram, "uLightDirection");
glUniform3f(lightLoc, lightDir.x, lightDir.y, lightDir.z);

GLint texLoc = glGetUniformLocation(shaderProgram, "uHatchTexture");
glUniform1i(texLoc, 0);  // Texture unit 0
```

## DongArch3D 렌더링 파이프라인

### Arch3D Liner NPR 렌더링

**참고 파일**:
- `GigaMesh/gui/src/meshGL/meshGL.cpp`: 메인 렌더링 루프
- `GigaMesh/gui/src/meshGL/meshGLShader.cpp`: 셰이더 관리
- `GigaMesh/gui/src/shaders/NPR/`: NPR 셰이더

**렌더링 단계**:
```cpp
void DongArchGLWidget::paintGL() {
    // 1. Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. 셰이더 활성화
    glUseProgram(nprShaderProgram);

    // 3. Uniform 설정
    setUniforms(mvpMatrix, lightDirection, viewPos);

    // 4. Texture 바인딩
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hatchTextureID);

    // 5. VAO 바인딩 및 그리기
    glBindVertexArray(meshVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // 6. Silhouette 렌더링 (Phase 3)
    renderSilhouetteEdges();
}
```

## NPR (Non-Photorealistic Rendering) 기법

### 1. Silhouette Edge Rendering
외곽선 렌더링

**Sobel Edge Detection** (GigaMesh/gui/src/shaders/NPR/NPR_ApplySobel.frag):
```glsl
// Sobel 커널
const mat3 sobelX = mat3(
    -1.0, 0.0, 1.0,
    -2.0, 0.0, 2.0,
    -1.0, 0.0, 1.0
);

const mat3 sobelY = mat3(
    -1.0, -2.0, -1.0,
     0.0,  0.0,  0.0,
     1.0,  2.0,  1.0
);

// 이미지 공간 엣지 검출
float edgeStrength = sqrt(gx*gx + gy*gy);
```

### 2. Hatching
해칭 패턴 렌더링 (고고학 도면 표준)

**Tonal Art Maps (TAMs)** 기반:
- 조명 밝기에 따라 6단계 해칭 밀도
- 45도 고정 각도 (Raczynski-Henk 2017 표준)

```glsl
// 조명 강도에 따른 해칭 레벨 선택
float intensity = dot(N, L);
vec4 hatch;
if (intensity > 0.95) hatch = vec4(1.0);       // Level 0: 흰색
else if (intensity > 0.75) hatch = texture(hatch1, vTexCoord);
else if (intensity > 0.50) hatch = texture(hatch2, vTexCoord);
else if (intensity > 0.25) hatch = texture(hatch3, vTexCoord);
else if (intensity > 0.05) hatch = texture(hatch4, vTexCoord);
else hatch = texture(hatch5, vTexCoord);  // Level 5: 완전 어두움
```

## OpenGL 디버깅

### 1. 에러 체크
```cpp
void checkGLError(const char* location) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        qDebug() << location << ": OpenGL error" << err;
    }
}

// 사용
glDrawArrays(GL_TRIANGLES, 0, vertexCount);
checkGLError("After draw call");
```

### 2. 셰이더 컴파일 에러
```cpp
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    qWarning() << "Shader compilation failed:" << infoLog;
}
```

### 3. 프로그램 링크 에러
```cpp
GLint success;
glGetProgramiv(program, GL_LINK_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    qWarning() << "Shader linking failed:" << infoLog;
}
```

## GitHub 참고 문서

수집된 문서:
- [GLSL Modulo Operator Implementation](https://github.com/pema99/shader-knowledge/blob/main/tips-and-tricks.md)
- [Replacing Deprecated OpenGL with GLM](https://github.com/icaven/glm/blob/master/manual.md)
- [cglm Vector Math for OpenGL](https://github.com/recp/cglm/blob/v0.9.6/docs/source/opengl.rst)

## 학습 리소스

### 공식 문서
- **OpenGL 3.3 Specification**: https://www.khronos.org/registry/OpenGL/specs/gl/glspec33.core.pdf
- **GLSL 3.30 Specification**: https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.3.30.pdf
- **OpenGL Reference Pages**: https://www.khronos.org/registry/OpenGL-Refpages/gl4/

### 온라인 튜토리얼
- **LearnOpenGL**: https://learnopengl.com/
  - 최고의 OpenGL 초보자 튜토리얼
  - 3.3 Core Profile 기준
  - C++ 예제 코드
- **open.gl**: https://open.gl/
  - 간결한 Core Profile 가이드
  - 실전 예제 중심
- **OGLdev**: https://ogldev.org/
  - 고급 OpenGL 기법
  - 3D 게임 개발 관점

### 수학 라이브러리
- **GLM (OpenGL Mathematics)**: https://github.com/g-truc/glm
  - GLSL과 동일한 문법
  - DongArch3D에서 사용 중
  ```cpp
  #include <glm/glm.hpp>
  #include <glm/gtc/matrix_transform.hpp>
  #include <glm/gtc/type_ptr.hpp>

  glm::mat4 model = glm::rotate(glm::mat4(1.0f),
                                glm::radians(45.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
  ```

## OpenGL 3.3 학습 순서

### 1단계: 기초 (3-4일)
1. Context 생성 (QOpenGLWidget 사용)
2. VBO/VAO 개념
3. 기본 셰이더 작성
4. Triangle 그리기

### 2단계: 3D 렌더링 (4-5일)
5. MVP 행렬 이해
6. 조명 (Phong, Blinn-Phong)
7. 텍스처 매핑
8. 깊이 테스트

### 3단계: 고급 기법 (1주)
9. 프레임버퍼 (FBO)
10. 다중 렌더 타겟 (MRT)
11. 스텐실 버퍼
12. NPR 셰이더

### 4단계: 최적화 (3-4일)
13. 인스턴싱
14. Frustum Culling
15. LOD (Level of Detail)

## DongArch3D Phase별 OpenGL 사용

| Phase | OpenGL 기능 | 우선순위 |
|-------|-------------|----------|
| Phase 0 | VBO/VAO 기본 렌더링 | ⭐⭐⭐⭐⭐ |
| Phase 1 | MVP 행렬 변환 | ⭐⭐⭐⭐⭐ |
| Phase 2 | FBO로 Cutline 렌더링 | ⭐⭐⭐⭐⭐ |
| Phase 3 | Silhouette 셰이더 | ⭐⭐⭐⭐⭐ |
| Phase 4 | Clipping Plane | ⭐⭐⭐ |
| Phase 5 | X-Ray 렌더링, D-Tak | ⭐⭐⭐ |

## Qt + OpenGL 통합

### QOpenGLWidget 사용 예시
```cpp
// GigaMesh/gui/src/meshwidget.h 기반
class MeshWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();  // 중요!
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        // VBO/VAO 초기화
        setupBuffers();
        // 셰이더 로드
        loadShaders();
    }

    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderMesh();
    }

    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
        projectionMatrix = glm::perspective(
            glm::radians(45.0f),
            float(w) / float(h),
            0.1f, 100.0f
        );
    }
};
```

---

**생성일**: 2025-11-08
**프로젝트**: DongArch3D v2.0.0 (Arch3D Liner 기반)
**OpenGL 버전**: 3.3 Core Profile
