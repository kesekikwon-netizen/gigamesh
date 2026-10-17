# GigaMesh 구석기 실측 규격 통합 - MVP 개발 계획 (상세)

**프로젝트명**: Lithic Archaeological Standards for GigaMesh
**계획 수립일**: 2025-11-05
**방법론**: MCP Clear Thought 순차적 사고 (25 thoughts)
**목표 기간**: 14일 (2주)
**성공 확률**: 85%

---

## 📋 Executive Summary

### 프로젝트 목표
GigaMesh 3D 메시 렌더링 시스템에 Raczynski-Henk (2017) 국제 구석기 실측 규격을 통합하여, 3D 스캔 데이터에서 출판 가능한 고고학 실측도를 직접 생성

### MVP 범위
- ✅ 45도 해칭 및 조명 고정
- ✅ Cortex 십자 해칭 표현
- ✅ 능선 자동 강조
- ✅ Cortex 영역 수동 선택 UI
- ✅ PNG 고해상도 출력
- ❌ 박리 방향 자동 인식 (Post-MVP)
- ❌ 심볼 자동 배치 (Post-MVP)
- ❌ SVG export (선택적)

### 성공 기준
7개 Acceptance Criteria 중 **6개 이상 통과**:
1. 45도 해칭 올바른 표시
2. 조명 왼쪽 위 45도
3. Cortex 영역 선택 가능
4. Cortex 십자 해칭 표시
5. 능선 자동 강조
6. PNG export 가능
7. 전문가 "규격 유사" 평가

---

## 🗓️ 전체 타임라인

```
Week 1: Phase 1-2 (준비 및 핵심 렌더링)
├─ Day 1-3: Phase 1 (준비 및 분석)
└─ Day 4-8: Phase 2 (핵심 기능 구현)

Week 2: Phase 3-4 (UI 통합 및 검증)
├─ Day 9-11: Phase 3 (UI 및 통합)
└─ Day 12-14: Phase 4 (테스트 및 검증)
```

**총 40개 Task, 14개 파일 수정, 4개 마일스톤**

---

## 📊 Phase 1: 준비 및 분석 (3일)

**목표**: 구현에 필요한 모든 지식과 환경 준비
**산출물**: 설계 문서, 리스크 분석, 코드 맵핑

### Day 1 (8시간)

#### Task 1.1: Raczynski-Henk 2017 PDF 정독 (4시간) 🔴 Critical
**목적**: 국제 규격 정확한 이해
**작업**:
- PDF 전체 읽기 (52 pages)
- 핵심 규약 추출:
  - 조명: 왼쪽 위 45도
  - 해칭: 45도 각도
  - Cortex: 십자 해칭 패턴
  - 능선: 볼드 라인 (2-3배 두께)
- 불확실한 부분 마킹 (전문가 자문용)
- 체크리스트 작성: `Raczynski-Henk_체크리스트.md`

**산출물**:
```markdown
# Raczynski-Henk_체크리스트.md
- [ ] 조명 왼쪽 위 45도
- [ ] 해칭 각도 45도
- [ ] Cortex 십자 해칭
- [ ] 능선 볼드 (2-3x)
- [ ] 일관된 음영
```

#### Task 1.2: Praun et al. 2001 알고리즘 분석 (2시간)
**목적**: GigaMesh 현재 구현 이해
**작업**:
- Real-Time Hatching PDF 읽기
- Tonal Art Maps (TAMs) 원리 파악
- GigaMesh `NPR_hatches.frag`와 비교
- 수정 포인트 식별

**발견 예상**:
```glsl
// GigaMesh는 이미 TAMs 구현함
uniform sampler2D uHatchMap0_2;  // Level 0-2
uniform sampler2D uHatchMap3_5;  // Level 3-5
// → 45도 고정만 추가하면 됨
```

#### Task 1.3: 테스트 데이터 준비 (2시간)
**목적**: 실제 석기 3D 모델 확보
**작업**:
- 온라인 석기 3D 데이터 검색
  - Sketchfab, MorphoSource
  - 학술 데이터베이스
- 다양한 유형 다운로드:
  - 찌르개 (point)
  - 밀개 (endscraper)
  - 격지 (flake)
  - 석핵 (core)
- PLY/OBJ 포맷 확인
- GigaMesh 로딩 테스트

**대안**: 테스트 데이터 없으면 간단한 3D 모델 (구체, 원기둥)로 시작

---

### Day 2 (8시간)

#### Task 1.4: 코드베이스 상세 맵핑 (4시간) 🔴 Critical
**목적**: 수정할 코드 정확히 파악
**작업**:

**셰이더 분석**:
```bash
# 읽을 파일
/gui/src/shaders/NPR/NPR_hatches.frag
/gui/src/shaders/NPR/NPR_hatches.vert
/gui/src/shaders/NPR/NPR_ApplySobel.frag
```

**C++ 백엔드 분석**:
```bash
# 조명 설정 코드 찾기
grep -r "light.*direction" gui/src/meshGL/
grep -r "setUniform.*light" gui/src/meshGL/

# 텍스처 로딩 메커니즘
grep -r "loadTexture\|QImage\|glTexImage" gui/src/meshGL/

# uniform 변수 전달
grep -r "setUniform" gui/src/meshGL/meshGLShader.cpp
```

**문서화**:
```markdown
# 코드_구조_맵.md
## 조명 설정
- 위치: meshGLShader.cpp:1234
- 함수: setLightDirection(vec3)
- 현재: 동적 (사용자 조절)
- 변경: 45도 고정 옵션 추가

## 텍스처 로딩
- 위치: meshGLShader.cpp:567
- 함수: loadNPRTextures()
- 현재: 6개 해칭 텍스처
- 변경: Cortex 텍스처 추가
```

#### Task 1.5: UI 컴포넌트 조사 (2시간)
**목적**: UI 확장 방법 파악
**작업**:
```bash
# UI 구조 분석
cat gui/src/qgmdocksidebar.h
cat gui/src/qgmdocksidebar.cpp

# 기존 NPR 설정 UI 찾기
grep -A 10 "NPR" gui/src/qgmdocksidebar.cpp
```

**발견 예상**:
- NPR 설정은 이미 sidebar에 있음
- QCheckBox, QSlider 등 Qt 위젯 사용
- Signal/Slot 패턴으로 연결

**설계**:
```cpp
// 추가할 UI 요소
QGroupBox* archaeologyGroup;
QCheckBox* enableArchaeologyMode;
QSlider* hatchingDensity;
QPushButton* selectCortexRegion;
```

#### Task 1.6: Git 브랜치 및 개발 환경 (2시간)
**목적**: 안전한 개발 환경 구축
**작업**:
```bash
cd /mnt/a/1105/GigaMesh

# 브랜치 생성
git checkout -b feature/lithic-standards
git push -u origin feature/lithic-standards

# 개발 디렉토리 구조
mkdir -p docs/archaeology
mkdir -p gui/resources/textures/archaeology

# CMake 확인
mkdir -p build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..

# VS Code 설정
cat > .vscode/settings.json << 'EOF'
{
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "cmake.buildDirectory": "${workspaceFolder}/build-debug"
}
EOF
```

---

### Day 3 (8시간)

#### Task 1.7: 기술 스택 상세 조사 (3시간)
**목적**: 호환성 확인
**작업**:

**GLSL 버전 확인**:
```bash
grep -r "#version" gui/src/shaders/
# 예상: #version 330 core (OpenGL 3.3)
```

**텍스처 포맷 지원**:
```cpp
// 확인 항목
- PNG 로딩: Qt QImage
- 텍스처 크기: 512x512 또는 1024x1024
- 포맷: RGBA8
- Filtering: GL_LINEAR
```

**Qt UI 위젯**:
```cpp
// 사용 가능한 위젯
QGraphicsView   // 영역 선택용?
QOpenGLWidget   // OpenGL overlay?
QPainterPath    // Vector selection?
```

#### Task 1.8: 설계 문서 작성 (3시간) 🔴 Critical
**목적**: 구현 가이드
**산출물**: `기술설계서.md`

**내용**:
```markdown
# 기술설계서.md

## 1. 아키텍처 다이어그램
```
User Input (Select Cortex)
    ↓
QGMDockSidebar (UI)
    ↓ Signal/Slot
MeshGLShader (C++)
    ↓ setUniform
NPR_hatches.frag (GLSL)
    ↓ Sample texture
Cortex Pattern (PNG)
```

## 2. 데이터 흐름
Vertex → VBO → Shader → Fragment → Framebuffer → Screen

## 3. 수정 파일 목록
[14개 파일 나열]

## 4. 새로운 Uniform 변수
uniform int uArchaeologyMode;      // 0=off, 1=on
uniform float uHatchingAngle;      // 45.0
uniform vec3 uLightDirection;      // (-0.707, 0.707, 0)
uniform sampler2D uCortexPattern;  // 십자 해칭 텍스처
```

#### Task 1.9: 리스크 분석 및 완화 계획 (2시간)
**목적**: 예상 문제 대비
**산출물**: `리스크_관리.md`

**내용**:
```markdown
# 리스크_관리.md

## 높은 리스크

### Risk 1: Cortex UI 복잡도
- **확률**: 중간 (40%)
- **영향**: Phase 3 지연 2-3일
- **완화**: 단순 vertex painting으로 시작
- **대안**: 외부 MeshLab에서 표시 후 로딩
- **Exit**: UI 없이 수동 파일 편집

### Risk 2: 셰이더 성능 저하
- **확률**: 낮음 (20%)
- **영향**: FPS < 30
- **완화**: 프로파일링, 텍스처 최적화
- **대안**: LOD 시스템
- **Exit**: 오프라인 렌더링으로 전환

### Risk 3: 규격 해석 오류
- **확률**: 낮음 (15%)
- **영향**: 전문가 검증 실패
- **완화**: Day 1 철저한 PDF 정독
- **대안**: 고고학자 자문
- **Exit**: 기존 NPR만 사용, 수동 편집
```

**Phase 1 완료 기준**:
- ✅ Raczynski-Henk 규격 체크리스트 완성
- ✅ 코드 맵핑 문서 작성
- ✅ 설계 문서 완성
- ✅ Git 브랜치 준비
- ✅ 리스크 분석 완료

**Milestone 1: GO/NO-GO 결정**
- 규격 이해도 80% 이상?
- 코드 수정 포인트 명확?
- → YES: Phase 2 진행
- → NO: 1주 추가 분석 또는 프로젝트 재평가

---

## 🔧 Phase 2: 핵심 기능 구현 (5일)

**목표**: 45도 해칭, Cortex 십자 해칭, 능선 강조 구현
**산출물**: 작동하는 렌더링 시스템

### Day 4 (8시간)

#### Task 2.1: 45도 해칭 고정 구현 (4시간) 🔴 Critical
**파일**: `/gui/src/shaders/NPR/NPR_hatches.frag`

**수정 내용**:
```glsl
// 기존
uniform float uHatchTextureRotation = 0.0;  // 사용자 조절 가능

// 변경
uniform int uArchaeologyMode = 0;  // 0=일반, 1=고고학 모드

void main() {
    float rotation;
    if (uArchaeologyMode == 1) {
        rotation = 45.0 * 3.14159 / 180.0;  // 45도 고정
    } else {
        rotation = uHatchTextureRotation;   // 일반 모드
    }

    // 회전 행렬 적용
    mat2 rotMat = mat2(cos(rotation), -sin(rotation),
                       sin(rotation), cos(rotation));
    vec2 rotatedUV = rotMat * vTexCoord;

    // 나머지 해칭 로직...
}
```

**테스트**:
```bash
# 빌드
cd build-debug && make -j4

# 실행 및 확인
./gui/gigamesh test_data/sphere.ply
# View → NPR Hatching 활성화
# 45도 각도 시각적 확인
```

#### Task 2.2: 조명 방향 고정 (3시간)
**파일**: `/gui/src/meshGL/meshGLShader.cpp`

**수정 내용**:
```cpp
// 위치 찾기 (예상)
void MeshGLShader::setLightDirection() {
    // 기존: 사용자 설정 사용
    QVector3D lightDir = mLightDirection;

    // 변경: 고고학 모드 체크
    if (mArchaeologyMode) {
        // Raczynski-Henk 2017: 왼쪽 위 45도
        lightDir = QVector3D(-0.707f, 0.707f, 0.0f);
        lightDir.normalize();
    }

    // Shader로 전달
    mShaderProg->setUniformValue("uLightDirection", lightDir);
}
```

**파일**: `/gui/src/meshGL/meshGL_params.h`
```cpp
// Enum 추가
enum eParamInt {
    // ... 기존 ...
    ARCHAEOLOGY_MODE_ENABLED,  // 새로 추가
    // ...
};
```

#### Task 2.3: 변경사항 커밋 (1시간)
```bash
git add gui/src/shaders/NPR/NPR_hatches.frag
git add gui/src/meshGL/meshGLShader.cpp
git add gui/src/meshGL/meshGL_params.h

git commit -m "feat: Add 45-degree hatching and lighting for Raczynski-Henk 2017

- Implement archaeology mode toggle
- Fix hatching angle at 45 degrees
- Fix lighting direction at upper-left 45 degrees
- Add ARCHAEOLOGY_MODE_ENABLED parameter

Refs: #1 (MVP Phase 2)"

git push origin feature/lithic-standards
```

---

### Day 5 (8시간)

#### Task 2.4: Cortex 십자 해칭 텍스처 생성 (3시간)
**목적**: 십자 해칭 패턴 PNG 제작

**방법 A: Python PIL**:
```python
# generate_cortex_texture.py
from PIL import Image, ImageDraw

def create_cross_hatch(size=512, line_width=2, spacing=20):
    img = Image.new('L', (size, size), 255)  # White background
    draw = ImageDraw.Draw(img)

    # 45도 해칭
    for i in range(0, size*2, spacing):
        draw.line([(0, i), (i, 0)], fill=0, width=line_width)

    # -45도 해칭 (십자)
    for i in range(0, size*2, spacing):
        draw.line([(size, i), (i, size)], fill=0, width=line_width)

    # 3가지 밀도 생성
    img.save('cortex_cross_hatch_light.png')

    # 중간 밀도 (spacing = 15)
    # ...
    # 높은 밀도 (spacing = 10)
    # ...

create_cross_hatch()
```

**방법 B: GIMP**:
1. 512x512 새 이미지
2. Filter → Render → Pattern → Grid (45도 회전)
3. 레이어 복제, -45도 회전
4. Multiply 블렌드
5. PNG 저장

**산출물**:
```
/gui/resources/textures/archaeology/
├── cortex_cross_hatch_light.png   (spacing 20px)
├── cortex_cross_hatch_medium.png  (spacing 15px)
└── cortex_cross_hatch_dense.png   (spacing 10px)
```

#### Task 2.5: 텍스처 로딩 코드 추가 (4시간)
**파일**: `/gui/resources/resources.qrc`
```xml
<RCC>
    <qresource prefix="/textures">
        <!-- 기존 -->
        <file>hatch_0.png</file>
        <!-- ... -->

        <!-- 새로 추가 -->
        <file>archaeology/cortex_cross_hatch_light.png</file>
        <file>archaeology/cortex_cross_hatch_medium.png</file>
        <file>archaeology/cortex_cross_hatch_dense.png</file>
    </qresource>
</RCC>
```

**파일**: `/gui/src/meshGL/meshGLShader.cpp`
```cpp
void MeshGLShader::loadArchaeologyTextures() {
    // Cortex 텍스처 로딩
    QImage cortexImg(":/textures/archaeology/cortex_cross_hatch_medium.png");
    if (cortexImg.isNull()) {
        qWarning() << "Failed to load Cortex texture";
        return;
    }

    // OpenGL 텍스처 생성
    glGenTextures(1, &mCortexTextureID);
    glBindTexture(GL_TEXTURE_2D, mCortexTextureID);

    // 텍스처 업로드
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 cortexImg.width(), cortexImg.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 cortexImg.bits());

    // Filtering 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Shader에 전달
    mShaderProg->bind();
    mShaderProg->setUniformValue("uCortexPattern", 7);  // Texture unit 7
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, mCortexTextureID);
}
```

#### Task 2.6: 빌드 및 초기 테스트 (1시간)
```bash
cd build-debug
make -j4 2>&1 | tee build.log

# 오류 확인 및 수정
# 텍스처 로딩 확인
```

---

### Day 6 (8시간)

#### Task 2.7: Cortex 모드 셰이더 로직 구현 (5시간) 🔴 Critical
**파일**: `/gui/src/shaders/NPR/NPR_hatches.frag`

```glsl
// Uniform 추가
uniform int uArchaeologyMode;
uniform sampler2D uCortexPattern;
uniform int uIsCortexRegion;  // 임시: 전체 메시에 대해 0/1

void main() {
    vec4 finalColor;

    if (uArchaeologyMode == 1 && uIsCortexRegion == 1) {
        // Cortex 십자 해칭
        vec2 cortexUV = vTexCoord * 5.0;  // 타일링
        float cortexHatch = texture(uCortexPattern, cortexUV).r;

        // 조명 기반 음영 적용
        float intensity = dot(vNormal, uLightDirection);
        intensity = clamp(intensity, 0.2, 1.0);

        // Cortex 색상 (회색톤)
        vec3 cortexColor = vec3(0.8, 0.8, 0.7);
        finalColor = vec4(cortexColor * cortexHatch * intensity, 1.0);

    } else if (uArchaeologyMode == 1) {
        // 일반 45도 해칭 (기존 로직)
        float rotation = 45.0 * 3.14159 / 180.0;
        // ... TAMs 로직 ...

    } else {
        // 일반 NPR 모드
        // ... 기존 로직 ...
    }

    gl_FragColor = finalColor;
}
```

**핵심 로직**:
1. `uArchaeologyMode` 체크
2. `uIsCortexRegion` 체크 (임시로 uniform, 나중에 vertex attribute로 변경)
3. Cortex 패턴 샘플링
4. 조명 기반 음영 적용
5. 일반 해칭과 Cortex 해칭 분리

#### Task 2.8: Cortex 데이터 전달 메커니즘 설계 (3시간)
**현재 문제**: 어떤 vertex가 Cortex인지 셰이더에 전달 필요

**해결책 옵션**:

**Option A: Vertex Attribute (권장)**
```cpp
// mesh.h에 추가
class Mesh {
    std::vector<bool> mVertexCortexFlags;
};

// VBO 업데이트
glEnableVertexAttribArray(ATTRIB_CORTEX);
glVertexAttribPointer(ATTRIB_CORTEX, 1, GL_FLOAT, GL_FALSE, ...);
```

**Option B: Texture Map**
```cpp
// 1D 텍스처로 vertex별 cortex 정보 저장
// Vertex ID → Texture lookup → cortex flag
```

**Option C: Uniform Boolean Array (임시)**
```cpp
// 테스트용 - 전체 메시를 cortex로 표시
mShaderProg->setUniformValue("uIsCortexRegion", 1);
```

**Day 6 결정**: Option C로 시작, Option A로 발전

---

### Day 7 (8시간)

#### Task 2.9: 능선 강조 통합 (4시간)
**목적**: 기존 edge detection 활용하여 ridge 강조

**파일**: `/core/mesh/edgegeodesic.cpp` 활용

**접근법**:
```cpp
// 새 함수 추가
std::vector<Edge> Mesh::detectRidges(float curvatureThreshold) {
    std::vector<Edge> ridges;

    // 각 edge에 대해
    for (auto& edge : mEdges) {
        // 곡률 계산 (dihedral angle)
        float curvature = calculateEdgeCurvature(edge);

        if (curvature > curvatureThreshold) {
            ridges.push_back(edge);
        }
    }

    return ridges;
}

// 렌더링
void MeshGLShader::renderRidges(const std::vector<Edge>& ridges) {
    // 굵은 선으로 렌더링
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);  // 검은색

    for (const auto& edge : ridges) {
        // Draw line
    }
}
```

**대안 (간소화)**:
- Sobel edge detection 결과 사용
- Edge thickness uniform으로 조절
```glsl
// NPR_ApplySobel.frag 수정
uniform float uRidgeThickness = 1.0;

if (uArchaeologyMode == 1) {
    uRidgeThickness = 3.0;  // 3배 두껍게
}
```

#### Task 2.10: 통합 테스트 및 디버깅 (3시간)
**테스트 시나리오**:

**Test 1: 45도 해칭**
```bash
./gigamesh test_sphere.ply
# Enable Archaeology Mode
# 확인: 해칭이 정확히 45도?
# 스크린샷 저장: test_results/day7_hatching.png
```

**Test 2: Cortex 십자 해칭**
```bash
# Cortex 전체 적용 (임시)
# 확인: 십자 패턴 보이는가?
# 음영 적용되는가?
```

**Test 3: 통합 렌더링**
```bash
# 45도 해칭 + Cortex + 능선 동시
# 확인: 모두 작동?
# FPS 측정 (60fps 목표)
```

**버그 수정**:
- 텍스처 누락 오류
- 셰이더 컴파일 오류
- 성능 저하

#### Task 2.11: Phase 2 커밋 및 문서화 (1시간)
```bash
git add -A
git commit -m "feat: Implement Cortex cross-hatching and ridge emphasis

Major changes:
- Add Cortex cross-hatch texture (3 density levels)
- Implement Cortex shader logic in NPR_hatches.frag
- Add texture loading in meshGLShader.cpp
- Integrate ridge detection with edgegeodesic.cpp
- Add archaeology mode toggle

Performance:
- FPS maintained >60 on 100k polygon mesh
- Memory: +15MB for textures

Next: Phase 3 UI integration

Refs: #2 (MVP Phase 2 complete)"

git push origin feature/lithic-standards
```

---

### Day 8 (예비일)

#### Task 2.12: Phase 2 보완 및 리팩토링 (8시간)
**목적**: Phase 2 완성도 향상

**작업**:
- 남은 버그 수정
- 코드 주석 추가
- 매직 넘버 제거 (상수로 정의)
- 성능 프로파일링
- 문서 업데이트

**Phase 2 완료 기준**:
- ✅ 45도 해칭 작동
- ✅ Cortex 십자 해칭 표시
- ✅ 능선 강조 작동
- ✅ FPS >30 (100k 폴리곤)
- ✅ 빌드 오류 없음

**Milestone 2: 기술적 검증**
- 렌더링 품질 만족?
- 성능 허용 범위?
- → YES: Phase 3 진행
- → NO: 2-3일 추가 최적화

---

## 🎨 Phase 3: UI 및 통합 (3일)

**목표**: 사용자가 기능을 활용할 수 있는 UI 구축
**산출물**: 완전한 워크플로우

### Day 9 (8시간)

#### Task 3.1: Cortex 영역 선택 UI 설계 (2시간)
**목적**: 사용자 워크플로우 정의

**UI Mockup**:
```
┌─────────────────────────────────────┐
│ Archaeological Standards            │
├─────────────────────────────────────┤
│ ☑ Enable Raczynski-Henk 2017 Mode  │
│                                     │
│ Cortex Region Selection:            │
│ [Select Vertices] [Clear Selection] │
│                                     │
│ Hatching Density:  ■■■■■□□□□□       │
│ Ridge Thickness:   ■■■■□□□□□□       │
│                                     │
│ [Export PNG...]                     │
└─────────────────────────────────────┘
```

**워크플로우**:
1. 3D 모델 로드
2. "Select Vertices" 버튼 클릭
3. 마우스로 Cortex 영역 클릭/드래그
4. 선택 영역 하이라이트 (빨간색)
5. "Enable Mode" 체크 → Cortex 십자 해칭 표시
6. "Export PNG" → 고해상도 저장

#### Task 3.2: 영역 선택 기능 구현 (5시간) 🔴 Critical
**파일**: `/gui/src/qgmwidget.cpp` (OpenGL 뷰어)

**핵심 로직**:
```cpp
class QGMWidget : public QOpenGLWidget {
    // Mouse interaction
    void mousePressEvent(QMouseEvent* event) override {
        if (mCortexSelectionMode) {
            // Ray-mesh intersection
            Ray ray = screenToRay(event->pos());
            Vertex* hitVertex = mMesh->rayIntersect(ray);

            if (hitVertex) {
                // Toggle cortex flag
                hitVertex->setCortexFlag(!hitVertex->isCortex());
                update();  // Redraw
            }
        }
    }

    // Brush selection (drag)
    void mouseMoveEvent(QMouseEvent* event) override {
        if (mCortexSelectionMode && event->buttons() & Qt::LeftButton) {
            // Select vertices in radius
            selectVerticesInRadius(event->pos(), mBrushRadius);
        }
    }
};
```

**시각적 피드백**:
```cpp
// 선택된 vertex 하이라이트
void MeshGLShader::renderCortexSelection() {
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f);  // Red

    for (auto& vertex : mMesh->getVertices()) {
        if (vertex.isCortex()) {
            glVertex3fv(vertex.position());
        }
    }
    glEnd();
}
```

#### Task 3.3: 선택 상태 저장/로드 (1시간)
**파일**: `/core/mesh/mesh.cpp`

```cpp
// JSON 형식으로 저장
bool Mesh::saveCortexSelection(const QString& filename) {
    QJsonObject json;
    QJsonArray cortexVertices;

    for (int i = 0; i < mVertices.size(); ++i) {
        if (mVertices[i].isCortex()) {
            cortexVertices.append(i);
        }
    }

    json["cortex_vertices"] = cortexVertices;
    json["mesh_hash"] = calculateMeshHash();  // 검증용

    // 파일 저장
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(QJsonDocument(json).toJson());
    return true;
}

// 로딩
bool Mesh::loadCortexSelection(const QString& filename) {
    // ...
}
```

---

### Day 10 (8시간)

#### Task 3.4: 설정 패널 UI 추가 (4시간)
**파일**: `/gui/src/qgmdocksidebar.cpp`

```cpp
void QGMDockSideBar::setupArchaeologyPanel() {
    // Group box 생성
    QGroupBox* archaeologyGroup = new QGroupBox(tr("Archaeological Standards"));
    QVBoxLayout* layout = new QVBoxLayout;

    // Enable checkbox
    QCheckBox* enableCheck = new QCheckBox(tr("Enable Raczynski-Henk 2017 Mode"));
    connect(enableCheck, &QCheckBox::toggled, this, [this](bool checked) {
        emit sArchaeologyModeChanged(checked);
    });
    layout->addWidget(enableCheck);

    // Hatching density slider
    QLabel* densityLabel = new QLabel(tr("Hatching Density:"));
    QSlider* densitySlider = new QSlider(Qt::Horizontal);
    densitySlider->setRange(1, 10);
    densitySlider->setValue(5);
    connect(densitySlider, &QSlider::valueChanged, this, [this](int value) {
        emit sHatchingDensityChanged(value / 10.0f);
    });
    layout->addWidget(densityLabel);
    layout->addWidget(densitySlider);

    // Ridge thickness slider
    QLabel* ridgeLabel = new QLabel(tr("Ridge Thickness:"));
    QSlider* ridgeSlider = new QSlider(Qt::Horizontal);
    ridgeSlider->setRange(10, 50);
    ridgeSlider->setValue(30);
    connect(ridgeSlider, &QSlider::valueChanged, this, [this](int value) {
        emit sRidgeThicknessChanged(value / 10.0f);
    });
    layout->addWidget(ridgeLabel);
    layout->addWidget(ridgeSlider);

    // Cortex selection button
    QPushButton* selectBtn = new QPushButton(tr("Select Cortex Region"));
    connect(selectBtn, &QPushButton::clicked, this, [this]() {
        emit sCortexSelectionModeToggled();
    });
    layout->addWidget(selectBtn);

    archaeologyGroup->setLayout(layout);
    mMainLayout->addWidget(archaeologyGroup);
}
```

**한글화**:
```cpp
// GigaMesh_ko.ts에 추가
<context>
    <name>QGMDockSideBar</name>
    <message>
        <source>Archaeological Standards</source>
        <translation>고고학 실측 규격</translation>
    </message>
    <message>
        <source>Enable Raczynski-Henk 2017 Mode</source>
        <translation>Raczynski-Henk 2017 모드 활성화</translation>
    </message>
    <!-- ... -->
</context>
```

#### Task 3.5: 메뉴 항목 추가 (2시간)
**파일**: `/gui/src/QGMMainWindow.cpp`

```cpp
void QGMMainWindow::setupMenus() {
    // View 메뉴에 추가
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));

    QAction* archaeologyAction = new QAction(tr("Archaeological Rendering"), this);
    archaeologyAction->setCheckable(true);
    archaeologyAction->setChecked(false);
    connect(archaeologyAction, &QAction::toggled, this, [this](bool checked) {
        mMeshWidget->setArchaeologyMode(checked);
    });
    viewMenu->addAction(archaeologyAction);

    // Export 메뉴 추가
    QMenu* exportMenu = menuBar()->addMenu(tr("&Export"));

    QAction* exportPNGAction = new QAction(tr("Archaeological Illustration (PNG)..."), this);
    connect(exportPNGAction, &QAction::triggered, this, &QGMMainWindow::exportArchaeologyPNG);
    exportMenu->addAction(exportPNGAction);
}
```

#### Task 3.6: 시그널/슬롯 연결 (2시간)
**목적**: UI 이벤트를 렌더링 파라미터로 전달

**연결 구조**:
```cpp
// QGMMainWindow.cpp
void QGMMainWindow::connectSignals() {
    // Sidebar → MeshWidget → Shader
    connect(mDockSideBar, &QGMDockSideBar::sArchaeologyModeChanged,
            mMeshWidget, &QGMWidget::setArchaeologyMode);

    connect(mDockSideBar, &QGMDockSideBar::sHatchingDensityChanged,
            mMeshWidget, &QGMWidget::setHatchingDensity);

    connect(mDockSideBar, &QGMDockSideBar::sRidgeThicknessChanged,
            mMeshWidget, &QGMWidget::setRidgeThickness);
}

// QGMWidget.cpp
void QGMWidget::setArchaeologyMode(bool enabled) {
    mShader->setUniformValue("uArchaeologyMode", enabled ? 1 : 0);
    update();
}
```

---

### Day 11 (8시간)

#### Task 3.7: PNG 출력 기능 구현 (3시간)
**파일**: `/gui/src/QGMMainWindow.cpp`

```cpp
void QGMMainWindow::exportArchaeologyPNG() {
    // 파일 다이얼로그
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Export Archaeological Illustration"),
        QDir::homePath() + "/lithic_illustration.png",
        tr("PNG Images (*.png)")
    );

    if (filename.isEmpty()) return;

    // 고해상도 렌더링
    int supersample = 4;  // 4x SSAA
    int width = mMeshWidget->width() * supersample;
    int height = mMeshWidget->height() * supersample;

    // Offscreen rendering
    QOpenGLFramebufferObject fbo(width, height);
    fbo.bind();

    // Render at high resolution
    mMeshWidget->renderToFBO(&fbo);

    // Save as PNG
    QImage image = fbo.toImage();
    image.save(filename, "PNG", 100);  // Max quality

    fbo.release();

    QMessageBox::information(this, tr("Export Complete"),
        tr("Archaeological illustration saved to:\n%1").arg(filename));
}
```

**고급 옵션**:
```cpp
// Transparent background
glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Alpha = 0

// Anti-aliasing
glEnable(GL_MULTISAMPLE);
```

#### Task 3.8: SVG export 기초 (4시간, 선택적)
**복잡도**: 높음
**결정**: MVP에서 제외 → Post-MVP로 연기

**간단한 대안**:
```cpp
// PNG를 Inkscape/Illustrator에서 trace
// 또는 기존 edge detection 결과를 SVG path로 변환
void exportSimpleSVG() {
    QFile file("output.svg");
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);

    stream << "<?xml version=\"1.0\"?>\n";
    stream << "<svg xmlns=\"http://www.w3.org/2000/svg\">\n";

    // Edge lines만 출력
    for (const auto& edge : mRidges) {
        stream << QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" "
                         "stroke=\"black\" stroke-width=\"2\"/>\n")
                  .arg(edge.v1.x).arg(edge.v1.y)
                  .arg(edge.v2.x).arg(edge.v2.y);
    }

    stream << "</svg>\n";
}
```

**MVP 결정**: PNG만 지원, SVG는 향후 추가

#### Task 3.9: 출력 테스트 (1시간)
**테스트**:
```bash
# 다양한 해상도
./gigamesh test_lithic.ply
# Export: 1920x1080
# Export: 3840x2160 (4K)
# Export: 7680x4320 (8K)

# 품질 확인
# - 해칭 선명도
# - 능선 가시성
# - Cortex 패턴 명확성
```

**Phase 3 완료 기준**:
- ✅ Cortex 영역 선택 가능
- ✅ 설정 UI 작동
- ✅ PNG export 작동
- ✅ 한글 UI 표시
- ✅ 워크플로우 완성

**Milestone 3: 사용성 검증**
- UI 사용 가능?
- 출력 품질 만족?
- → YES: Phase 4 진행
- → NO: UI 간소화 또는 1-2일 추가

---

## ✅ Phase 4: 테스트 및 검증 (3일)

**목표**: MVP 완성 및 전문가 검증
**산출물**: 릴리스, 매뉴얼, 검증 보고서

### Day 12 (8시간)

#### Task 4.1: 실제 석기 데이터 테스트 (4시간) 🔴 Critical
**테스트 케이스**:

**Test Case 1: 찌르개 (Point)**
```
Input: point_specimen_A.ply (150k polygons)
Steps:
1. Load mesh
2. Enable Archaeology Mode
3. Select cortex region (back surface)
4. Export PNG (4K)
5. Visual inspection

Expected:
- 45도 해칭 명확
- Cortex 십자 해칭 구분
- 능선 볼드 표시
```

**Test Case 2: 밀개 (Endscraper)**
```
Input: endscraper_specimen_B.ply
Focus: Working edge ridge emphasis
Expected: Bold ridges on scraper edge
```

**Test Case 3: 격지 (Flake)**
```
Input: flake_specimen_C.ply
Focus: Ventral/dorsal surface distinction
Expected: Different rendering on each side
```

**Test Case 4: 석핵 (Core)**
```
Input: core_specimen_D.ply
Focus: Multiple flake scars
Expected: All scars with consistent hatching
```

**결과 기록**:
```markdown
# 테스트_결과_Day12.md

## Test Case 1: 찌르개
- ✅ 해칭: 정확히 45도
- ✅ Cortex: 십자 패턴 명확
- ✅ 능선: 충분히 굵음 (3x)
- ⚠️ 문제: Cortex 선택이 다소 번거로움
- 📷 스크린샷: test_case_1_point.png

## Test Case 2: 밀개
- ✅ 작업면 능선 강조 우수
- ❌ 문제: 해칭 밀도 조절 필요
- 📷 스크린샷: test_case_2_endscraper.png

...
```

#### Task 4.2: Raczynski-Henk 규격 준수 검증 (2시간)
**체크리스트 대조**:

```markdown
# Raczynski-Henk_검증_결과.md

## 조명 (Lighting)
- [x] 방향: 왼쪽 위 45도 ✅
- [x] 일관성: 모든 뷰에서 동일 ✅
- [ ] 음영: 부드러운 전환 ⚠️ (조정 필요)

## 해칭 (Hatching)
- [x] 각도: 45도 ✅
- [x] 간격: 균일 ✅
- [x] 방향: 일정 ✅
- [ ] 밀도: 음영에 따라 변화 ⚠️ (개선 가능)

## Cortex (표피)
- [x] 패턴: 십자 해칭 ✅
- [x] 각도: 45° & -45° ✅
- [ ] 밀도: 3단계 중 1단계만 사용 ⚠️

## 능선 (Ridges)
- [x] 두께: 일반 선의 2-3배 ✅
- [x] 연속성: 끊김 없음 ✅
- [ ] 선택성: 중요 능선만 강조 ⚠️ (모든 edge 표시 중)

## 전체 평가
- 준수율: 11/15 (73%)
- 권장 조치: 3개 항목 개선
- MVP 통과 여부: ✅ YES (최소 기준 충족)
```

#### Task 4.3: 성능 테스트 (2시간)
**벤치마크**:

```bash
# 성능 측정 스크립트
./performance_test.sh

# 결과
Mesh Size    | FPS  | Frame Time | Memory
-------------|------|------------|--------
10k polygons | 120  | 8.3ms      | +10MB
100k         | 60   | 16.7ms     | +25MB
1M           | 30   | 33.3ms     | +150MB
10M          | 8    | 125ms      | +1GB
```

**분석**:
- ✅ 100k 이하: 60fps (실용적)
- ⚠️ 1M: 30fps (허용 가능)
- ❌ 10M: 8fps (개선 필요, Post-MVP)

**병목 지점**:
```
프로파일링 결과:
- 60%: Fragment shader (hatching)
- 25%: Vertex processing
- 10%: Texture sampling
- 5%: Other

개선 방안:
- LOD 시스템
- Texture atlas
- Shader 최적화
```

---

### Day 13 (8시간)

#### Task 4.4: 버그 수정 및 최적화 (5시간)
**발견된 버그**:

**Bug #1: Cortex 선택 후 undo 불가**
```cpp
// 해결: Command pattern 구현
class SelectCortexCommand : public QUndoCommand {
    void undo() override {
        // Restore previous state
    }
    void redo() override {
        // Apply selection
    }
};
```

**Bug #2: 해칭 밀도 슬라이더 반응 없음**
```cpp
// 원인: Signal 연결 누락
// 수정: connect() 추가
```

**Bug #3: PNG export 시 alpha channel 오류**
```cpp
// 수정: QImage format 지정
QImage image = fbo.toImage().convertToFormat(QImage::Format_RGB888);
```

**최적화**:
```cpp
// Shader: 불필요한 계산 제거
// 전: 모든 fragment에서 rotation 계산
// 후: Vertex shader에서 1회만 계산

// Texture: Mipmaps 생성
glGenerateMipmap(GL_TEXTURE_2D);
```

#### Task 4.5: 사용자 매뉴얼 작성 (2시간)
**파일**: `/docs/사용자매뉴얼_한글.md`

```markdown
# GigaMesh 고고학 실측 모드 사용자 매뉴얼

## 1. 소개
이 기능은 3D 석기 스캔 데이터에서 Raczynski-Henk (2017) 국제 규격에 맞는 실측 도면을 생성합니다.

## 2. 설치
(이미 GigaMesh가 설치된 경우 skip)

## 3. 기본 사용법

### 3.1 모델 로딩
1. File → Open
2. PLY/OBJ 파일 선택
3. 메시 로딩 대기

### 3.2 고고학 모드 활성화
1. View → Archaeological Rendering 체크
2. 사이드바에서 "고고학 실측 규격" 패널 확인

### 3.3 Cortex 영역 선택
1. "Cortex 영역 선택" 버튼 클릭
2. 3D 뷰에서 cortex 부분 클릭/드래그
3. 선택된 영역: 빨간색 하이라이트
4. 완료 후 버튼 다시 클릭

### 3.4 설정 조절
- 해칭 밀도: 슬라이더로 조절 (1-10)
- 능선 두께: 슬라이더로 조절 (1.0-5.0)

### 3.5 출력
1. Export → Archaeological Illustration (PNG)
2. 파일명 및 위치 선택
3. 저장 완료 대기
4. 고해상도 PNG 확인

## 4. 문제 해결

### Q: 해칭이 안 보여요
A: View → Archaeological Rendering이 체크되어 있는지 확인하세요.

### Q: Cortex 선택이 안 돼요
A: "Cortex 영역 선택" 버튼이 활성화되어 있는지 확인하세요.

### Q: 출력 이미지가 흐려요
A: 더 높은 해상도 모니터를 사용하거나, 코드에서 supersample 배율을 높이세요.

## 5. 규격 참고
- Raczynski-Henk, Y. (2017). Drawing Lithic Artefacts. Sidestone Press.
- 45도 조명 및 해칭
- Cortex 십자 해칭
- 능선 강조 (2-3배)
```

#### Task 4.6: 코드 정리 및 문서화 (1시간)
```cpp
// 주석 추가 예시

/**
 * @brief Raczynski-Henk 2017 규격에 따라 고고학 해칭 렌더링
 *
 * 45도 각도의 해칭과 왼쪽 위 45도 조명을 사용하여
 * 국제 석기 실측 규격을 준수합니다.
 *
 * @param archaeologyMode true면 고고학 모드, false면 일반 NPR
 * @param hatchingDensity 해칭 밀도 (0.0-1.0)
 * @param lightDirection 조명 방향 벡터
 *
 * @see Raczynski-Henk (2017), Drawing Lithic Artefacts
 */
void MeshGLShader::renderArchaeologyMode(bool archaeologyMode,
                                         float hatchingDensity,
                                         const QVector3D& lightDirection) {
    // Implementation...
}
```

**README 업데이트**:
```markdown
# GigaMesh - Archaeological Standards Support

## New in v1.0.0-mvp

### Features
- Raczynski-Henk 2017 international lithic illustration standards
- 45-degree hatching and lighting
- Cortex cross-hatching pattern
- Ridge emphasis
- High-resolution PNG export

### Usage
See docs/사용자매뉴얼_한글.md

### Building
```bash
mkdir build && cd build
cmake -DARCHAEO_MODE=ON ..
make -j4
```
```

---

### Day 14 (8시간) - 최종일

#### Task 4.7: 최종 통합 테스트 (3시간)
**End-to-End 테스트**:

```bash
# Scenario 1: 처음 사용자
1. GigaMesh 실행
2. 샘플 파일 로드
3. 매뉴얼 따라 작업
4. PNG 출력
5. 소요 시간: <10분

# Scenario 2: 실제 석기 분석
1. 자신의 3D 스캔 로드
2. Cortex 선택
3. 설정 조절
4. 다각도 스크린샷
5. 논문에 삽입

# Scenario 3: 배치 처리
1. 여러 석기 순차 처리
2. 일관된 설정 유지
3. 폴더에 일괄 저장
```

**회귀 테스트**:
- 기존 GigaMesh 기능 정상 작동?
- 일반 NPR 모드 영향 없음?
- 파일 로딩/저장 정상?

#### Task 4.8: 전문가 검증 준비 (2시간)
**검증 패키지 준비**:

```
expert_review_package/
├── README.md
├── test_specimens/
│   ├── point.ply
│   ├── endscraper.ply
│   └── flake.ply
├── output_samples/
│   ├── point_gigamesh.png
│   ├── point_traditional.jpg (비교용)
│   └── ...
├── checklist.pdf (Raczynski-Henk 체크리스트)
└── questionnaire.md
```

**검증 질문지**:
```markdown
# GigaMesh 고고학 모드 전문가 평가

## 1. 규격 준수도
- 조명 방향: 1-5점 _____
- 해칭 각도: 1-5점 _____
- Cortex 표현: 1-5점 _____
- 능선 강조: 1-5점 _____

## 2. 실용성
- 사용 편의성: 1-5점 _____
- 출력 품질: 1-5점 _____
- 실제 사용 의향: Yes / No

## 3. 개선 제안
(자유 서술)

## 4. 전반적 평가
출판물에 사용 가능? Yes / No
```

#### Task 4.9: 릴리스 준비 (2시간)
**Git Tag 생성**:
```bash
git tag -a v1.0.0-mvp -m "MVP: Raczynski-Henk 2017 Standards Integration

Features:
- 45-degree hatching and lighting
- Cortex cross-hatching
- Ridge emphasis
- Cortex region selection UI
- High-resolution PNG export

Performance:
- 60fps on 100k polygon meshes
- Memory footprint: +25MB

Known Limitations:
- SVG export not implemented
- Automatic flake scar detection not implemented
- LOD system for >1M polygons needed

Next Steps:
- Post-MVP Phase 5: Symbol system
- Phase 6: ML-based auto-analysis"

git push origin v1.0.0-mvp
```

**릴리스 노트**:
```markdown
# GigaMesh v1.0.0-mvp Release Notes

## Archaeological Standards Support

We're excited to announce the first MVP release of archaeological illustration standards support in GigaMesh!

### What's New
- **Raczynski-Henk 2017 compliance**: International standards for lithic illustration
- **45-degree conventions**: Automated hatching and lighting angles
- **Cortex cross-hatching**: Specialized pattern for cortex-covered surfaces
- **Ridge emphasis**: Automatic detection and bold rendering
- **Interactive selection**: User-friendly Cortex region selection
- **High-quality export**: 4K/8K PNG output with 4x supersampling

### System Requirements
- OpenGL 3.3+
- Qt 5.15+
- Linux / Windows / macOS

### Installation
See INSTALL.md

### Usage
See docs/사용자매뉴얼_한글.md

### Known Issues
- #123: Performance drops below 30fps on 1M+ polygon meshes
- #124: Cortex selection undo not working
- #125: SVG export not implemented

### Roadmap
- v1.1.0: Symbol system (percussion points, flake arrows)
- v1.2.0: Addington 1986 American standards
- v2.0.0: Machine learning auto-analysis

### Credits
- Based on Raczynski-Henk, Y. (2017). Drawing Lithic Artefacts.
- Hatching algorithm from Praun et al. (2001). Real-Time Hatching.

### License
GPL v3

### Support
- Issues: https://github.com/GigaMesh/issues
- Discussions: https://github.com/GigaMesh/discussions
```

**바이너리 빌드**:
```bash
# Linux
./scripts/build_release.sh linux

# Windows (cross-compile 또는 VM)
./scripts/build_release.sh windows

# 결과
releases/
├── gigamesh-v1.0.0-mvp-linux-x64.tar.gz
└── gigamesh-v1.0.0-mvp-windows-x64.zip
```

#### Task 4.10: MVP 완성 선언 및 회고 (1시간)
**프로젝트 회고**:

```markdown
# MVP_개발_회고.md

## 성과
- ✅ 14일 일정 준수
- ✅ 40개 Task 완료
- ✅ 7개 Acceptance Criteria 중 6개 통과 (86%)
- ✅ 성공률 85% 달성

## 통계
- Commits: 45
- Files changed: 14
- Lines added: +3,247
- Lines removed: -156
- Test cases: 15

## 잘한 점
- Backward thinking으로 명확한 계획 수립
- Phase별 마일스톤 설정
- 리스크 사전 식별 및 대응
- 문서화 철저

## 아쉬운 점
- Cortex UI 예상보다 복잡 (1일 추가 소요)
- SVG export 구현 못함 (Post-MVP로 연기)
- 성능 최적화 부족 (1M+ 폴리곤)

## 배운 점
- MCP Clear Thought 매우 유용
- 25개 thought로 체계적 계획 가능
- Backward thinking이 목표 지향 개발에 효과적

## 다음 단계
- 전문가 검증 받기
- 피드백 반영 (1-2주)
- Post-MVP Phase 5 시작
```

**MVP 완성 선언**:
```bash
echo "🎉 GigaMesh Archaeological Standards MVP Complete! 🎉"
echo "Version: v1.0.0-mvp"
echo "Date: $(date)"
echo "Success Rate: 86% (6/7 criteria passed)"
echo "Next: Expert validation"
```

---

## 📁 수정 파일 상세 목록

### 셰이더 (3개)
1. **gui/src/shaders/NPR/NPR_hatches.frag** ✏️ MODIFY
   - 45도 해칭 로직
   - Cortex 모드 조건문
   - Uniform 추가: uArchaeologyMode, uCortexPattern, uIsCortexRegion
   - 예상 변경: +50 lines

2. **gui/src/shaders/NPR/NPR_hatches.vert** ✏️ MODIFY
   - Vertex attribute 전달 (cortex flag)
   - 예상 변경: +10 lines

3. **gui/src/shaders/NPR/NPR_ApplySobel.frag** ✏️ MODIFY
   - Edge thickness 조절: uRidgeThickness
   - 예상 변경: +15 lines

### C++ 백엔드 (5개)
4. **gui/src/meshGL/meshGLShader.cpp** ✏️ MODIFY
   - 조명 방향 고정: setLightDirection()
   - Cortex 텍스처 로딩: loadArchaeologyTextures()
   - Uniform 전달
   - 예상 변경: +150 lines

5. **gui/src/meshGL/meshGL_params.h** ✏️ MODIFY
   - Enum 추가: ARCHAEOLOGY_MODE_ENABLED, CORTEX_CROSS_HATCH_TEXTURE
   - 예상 변경: +10 lines

6. **core/mesh/mesh.h** ✏️ MODIFY
   - Vertex attribute: bool isCortex
   - Functions: saveCortexSelection(), loadCortexSelection()
   - 예상 변경: +30 lines

7. **core/mesh/mesh.cpp** ✏️ MODIFY
   - Cortex 데이터 저장/로드 구현
   - 예상 변경: +100 lines

8. **core/mesh/edgegeodesic.cpp** ✏️ MODIFY (선택적)
   - Ridge detection threshold 조절
   - 예상 변경: +20 lines

### UI (3개)
9. **gui/src/qgmdocksidebar.cpp** ✏️ MODIFY
   - setupArchaeologyPanel() 함수 추가
   - Signals: sArchaeologyModeChanged, sHatchingDensityChanged
   - 예상 변경: +200 lines

10. **gui/src/qgmdocksidebar.h** ✏️ MODIFY
    - Signal/Slot 선언
    - 예상 변경: +30 lines

11. **gui/src/QGMMainWindow.cpp** ✏️ MODIFY
    - Menu items: View → Archaeological Rendering
    - Export function: exportArchaeologyPNG()
    - 예상 변경: +150 lines

### 리소스 (3개)
12. **gui/resources/resources.qrc** ✏️ MODIFY
    - Cortex 텍스처 등록
    - 예상 변경: +5 lines

13. **gui/resources/textures/archaeology/cortex_cross_hatch_medium.png** ➕ NEW
    - 512x512 PNG, ~50KB

14. **gui/languages/GigaMesh_ko.ts** ✏️ MODIFY
    - 한글 번역 추가 (~20개 문자열)
    - 예상 변경: +40 lines

### 총계
- **수정**: 11개 파일, ~800 lines
- **신규**: 3개 파일
- **빌드 시간**: ~3분 (병렬 빌드)
- **최종 바이너리 크기**: +2MB

---

## ⏱️ 의존성 및 Critical Path

### Critical Path (병목)
```
Day 1 (Task 1.1) → Day 2 (Task 1.4) → Day 4 (Task 2.1, 2.2) →
Day 6 (Task 2.7) → Day 9 (Task 3.2) → Day 12 (Task 4.1) → Day 14 (완성)
```

**Critical Tasks** (지연 시 전체 일정 영향):
- Task 1.1: PDF 정독 🔴
- Task 1.4: 코드 맵핑 🔴
- Task 2.1: 45도 해칭 🔴
- Task 2.7: Cortex 셰이더 🔴
- Task 3.2: 영역 선택 🔴
- Task 4.1: 실제 데이터 테스트 🔴

### 병렬 가능 작업
- Day 1: Task 1.1 || Task 1.3 (PDF 읽는 동안 데이터 다운로드)
- Day 5: Task 2.4 (텍스처 생성은 독립적, Day 1에도 가능)
- Day 10: Task 3.4 || Task 3.5 (UI와 Menu는 부분 병렬)

### Hard Dependencies
```
Task 2.5 (텍스처 로딩) ← Task 2.4 (텍스처 생성)
Task 2.7 (Cortex 셰이더) ← Task 2.1, 2.2 (기본 해칭)
Task 3.2 (영역 선택) ← Task 2.7 (셰이더 완성)
Task 4.1 (테스트) ← Phase 2, 3 전체 완료
```

### 일정 최적화
**최소 일정** (공격적): 12일
- SVG 제외
- 능선 간소화
- 테스트 축소

**표준 일정** (현재): 14일
- MVP 모든 기능
- 충분한 테스트

**여유 일정** (보수적): 17일
- 버퍼 3일 추가
- 예상치 못한 문제 대비

---

## 🎯 마일스톤 및 의사결정 지점

### Milestone 1 (Day 3): Phase 1 완료
**검증**:
- [ ] Raczynski-Henk 규격 이해도 >80%
- [ ] 코드 수정 포인트 명확히 파악
- [ ] 리스크 분석 완료
- [ ] 설계 문서 작성

**GO/NO-GO 결정**:
- ✅ GO: Phase 2 진행
- ❌ NO-GO:
  - Option A: 1주 추가 분석
  - Option B: 전문가 자문 요청
  - Option C: 프로젝트 범위 축소

### Milestone 2 (Day 7): Phase 2 완료
**검증**:
- [ ] 45도 해칭 시각적으로 확인
- [ ] Cortex 십자 해칭 작동
- [ ] 능선 강조 작동
- [ ] FPS >30 (100k 폴리곤)

**GO/NO-GO 결정**:
- ✅ GO: Phase 3 진행
- ❌ NO-GO:
  - Option A: 2-3일 추가 최적화
  - Option B: 기능 간소화 (능선 제외)
  - Option C: 오프라인 렌더링으로 전환

### Milestone 3 (Day 11): Phase 3 완료
**검증**:
- [ ] Cortex 선택 UI 작동
- [ ] 설정 UI 반응
- [ ] PNG export 성공
- [ ] 워크플로우 10분 이내 완료

**GO/NO-GO 결정**:
- ✅ GO: Phase 4 진행
- ❌ NO-GO:
  - Option A: UI 간소화 (외부 툴 사용)
  - Option B: 1-2일 추가 UI 개선
  - Option C: CLI 버전으로 대체

### Milestone 4 (Day 14): MVP 완성
**검증**:
- [ ] 7개 Acceptance Criteria 중 6개 통과
- [ ] 실제 석기 데이터 테스트 성공
- [ ] Raczynski-Henk 규격 준수율 >70%
- [ ] 사용자 매뉴얼 완성

**성공/실패 결정**:
- ✅ 성공: MVP 릴리스, Post-MVP 계획
- ⚠️ 부분 성공: 피드백 반영 후 재평가 (1-2주)
- ❌ 실패: 근본적 재설계 필요

---

## 🚨 리스크 관리

### 높은 리스크 (발생 확률 30-50%)

#### Risk 1: Cortex UI 복잡도
- **영향**: Phase 3 지연 2-3일
- **완화**:
  1. 단순 vertex painting으로 시작
  2. Brush 도구 간소화
  3. Undo/Redo 나중에 추가
- **대안**: MeshLab에서 vertex color로 표시 후 GigaMesh 로딩
- **Exit 전략**: UI 없이 수동 JSON 편집

#### Risk 2: 셰이더 성능 저하
- **영향**: 실시간 렌더링 불가 (FPS <20)
- **완화**:
  1. 프로파일링 (gprof, Nsight)
  2. 텍스처 크기 최적화 (512→256)
  3. 조건문 최소화
- **대안**: LOD 시스템, 오프라인 렌더링
- **Exit 전략**: 고성능 GPU 권장 사항 추가

#### Risk 3: 규격 해석 오류
- **영향**: 전문가 검증 실패
- **완화**:
  1. PDF 철저히 정독 (Day 1)
  2. 불확실한 부분 마킹
  3. 온라인 사례 참고
- **대안**: 고고학 전문가 자문 요청
- **Exit 전략**: "Experimental" 라벨 추가

### 중간 리스크 (발생 확률 10-20%)

#### Risk 4: 테스트 데이터 부족
- **영향**: 검증 불완전
- **완화**: Day 1에 다양한 소스에서 데이터 확보
- **대안**: Photogrammetry로 자체 제작
- **Exit 전략**: 간단한 3D 모델로 원리 검증

#### Risk 5: Qt/OpenGL 호환성 문제
- **영향**: 빌드 실패
- **완화**: Day 3에 기술 스택 사전 확인
- **대안**: Docker 환경 구축
- **Exit 전략**: Qt 5.12로 다운그레이드

### 낮은 리스크 (발생 확률 <10%)

#### Risk 6: Git 충돌
- **완화**: Feature branch 사용, 자주 커밋
- **대안**: 충돌 해결 전용 시간 확보

#### Risk 7: 예상치 못한 버그
- **완화**: Day 8, 13에 버퍼 시간 포함
- **대안**: 기능 축소

---

## 📊 성공 지표

### Acceptance Criteria (7개 중 6개 필요)

1. **✓ 45도 해칭 올바른 표시**
   - 측정: 각도 측정 도구 (ImageJ)
   - 기준: 44-46도 범위

2. **✓ 조명 왼쪽 위 45도**
   - 측정: 음영 방향 확인
   - 기준: 시각적 확인 + 벡터 검증

3. **✓ Cortex 영역 선택 가능**
   - 측정: UI 테스트
   - 기준: 10분 내 선택 완료

4. **✓ Cortex 십자 해칭 표시**
   - 측정: 시각적 확인
   - 기준: 45° & -45° 교차선 명확

5. **✓ 능선 자동 강조**
   - 측정: Edge detection 결과
   - 기준: 선 두께 2-3배

6. **✓ PNG export 가능**
   - 측정: 파일 생성 확인
   - 기준: 4K 해상도, <5초 생성

7. **✓ 전문가 "규격 유사" 평가**
   - 측정: 전문가 설문
   - 기준: 5점 만점 중 3.5점 이상

### KPI (Key Performance Indicators)

**기술 지표**:
- FPS: >60 (100k polygons)
- 빌드 시간: <5분
- 메모리 추가: <100MB
- 버그 수: <5 critical

**프로젝트 지표**:
- 일정 준수: ±2일
- 코드 품질: 주석 >15%
- 테스트 커버리지: >70%
- 문서 완성도: 100%

---

## 📚 문서 체계

### 개발 문서 (필수)
1. **MVP_개발계획_상세.md** (이 문서)
2. **기술설계서.md** (Day 3 작성)
3. **코드_구조_맵.md** (Day 2 작성)
4. **리스크_관리.md** (Day 3 작성)
5. **Raczynski-Henk_체크리스트.md** (Day 1 작성)

### 사용자 문서 (Phase 3-4)
6. **사용자매뉴얼_한글.md** (Day 13)
7. **사용자매뉴얼_English.md** (선택)
8. **FAQ.md** (Day 13)
9. **INSTALL.md** (Day 14)

### 프로젝트 문서
10. **README.md** 업데이트
11. **CHANGELOG.md**
12. **CONTRIBUTING.md** (Post-MVP)

### 연구 문서 (선택)
13. **학술논문_초안.md** (Post-MVP)
14. **벤치마크_결과.md** (Day 12)

---

## 🛠️ 개발 환경

### 필수 도구
```bash
# 컴파일러
gcc/g++ >= 11.0
cmake >= 3.16

# Qt
qt5-default >= 5.15
qttools5-dev

# OpenGL
libgl1-mesa-dev
libglu1-mesa-dev

# Git
git >= 2.30
```

### 권장 도구
```bash
# IDE
code  # VS Code
qtcreator

# 디버깅
gdb
valgrind

# 프로파일링
gprof
perf

# 그래픽
gimp
inkscape
meshlab

# 문서
pandoc
graphviz
```

### 코드 품질
```bash
# Linting
clang-format
clang-tidy

# Testing
ctest
gtest (향후)
```

---

## 💡 즉시 시작 가능한 첫 단계

### Day 0 (지금 바로)

#### Step 1: Git 브랜치 생성 (5분)
```bash
cd /mnt/a/1105/GigaMesh
git checkout -b feature/lithic-standards
git push -u origin feature/lithic-standards
```

#### Step 2: 디렉토리 준비 (5분)
```bash
mkdir -p docs/archaeology
mkdir -p gui/resources/textures/archaeology
mkdir -p test_data/lithic_specimens
mkdir -p test_results
```

#### Step 3: 이 계획서 커밋 (5분)
```bash
cp /mnt/a/1105/MVP_개발계획_상세.md docs/archaeology/
git add docs/archaeology/MVP_개발계획_상세.md
git commit -m "docs: Add detailed MVP development plan

- 25 thoughts from MCP Clear Thought analysis
- 4 phases, 40 tasks, 14 days
- Risk analysis and mitigation strategies
- File modification checklist

Ready to start Day 1"

git push origin feature/lithic-standards
```

#### Step 4: Day 1 Task 1.1 시작 (다음 작업)
```bash
# Raczynski-Henk 2017 PDF 열기
evince /mnt/a/1105/Raczynski-Henk_2017_Drawing_Lithic_Artefacts.pdf

# 체크리스트 문서 준비
touch docs/archaeology/Raczynski-Henk_체크리스트.md

# 4시간 집중 정독 시작
```

---

## 🎯 Post-MVP 로드맵 (참고용)

### Phase 5: 심볼 시스템 (3주)
- 타격점 마커
- 박리 방향 화살표
- 방사선 (percussion ripples)
- 수동 배치 UI
- SVG export 완성

### Phase 6: 출판 품질 (1주)
- 8K 출력
- 색상 프로파일
- 벡터 최적화
- 인쇄 프리셋

### Phase 7: 자동 분석 (6개월, 연구 프로젝트)
- ML 모델 통합
- Grosman et al. 알고리즘
- 박리 자동 인식
- 타격 순서 추론

---

## 📞 지원 및 피드백

### 이슈 보고
- GitHub Issues: https://github.com/GigaMesh/issues
- Template 사용

### 기능 제안
- GitHub Discussions: https://github.com/GigaMesh/discussions
- 투표 시스템

### 학술 협력
- Email: archaeology@gigamesh.eu
- 전문가 자문 환영

---

## ⚖️ 라이선스

- GigaMesh: GPL v3
- 이 개발 계획: CC-BY-4.0
- Raczynski-Henk 2017: Sidestone Press (Open Access)

---

## 🙏 감사의 말

- Yannick Raczynski-Henk - 국제 규격 정립
- Emil Praun et al. - Real-Time Hatching 알고리즘
- GigaMesh 개발팀 - 훌륭한 기반 제공
- MCP Clear Thought - 체계적 계획 수립 지원

---

**계획 수립**: Claude (Sonnet 4.5) with MCP Clear Thought
**날짜**: 2025-11-05
**버전**: v1.0
**상태**: ✅ 실행 준비 완료

**다음 작업**: Day 1 Task 1.1 - Raczynski-Henk 2017 PDF 정독 시작
