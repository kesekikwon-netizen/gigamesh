# C++20 리소스 및 문서

DongArch3D 프로젝트를 위한 C++20 핵심 기술 문서 모음

## 프로젝트 C++ 버전 정보

- **현재 버전**: C++17 → **목표 버전**: C++20
- **업그레이드 이유**: Concepts, Ranges, std::span 등 최신 기능 활용
- **컴파일러 요구사항**:
  - GCC 10+
  - MSVC 19.20+ (Visual Studio 2019 16.0+)
  - Clang 10+
  - Apple Clang 13.0+

## C++20 주요 기능

### 1. Concepts (개념)
타입 제약을 통한 더 명확한 템플릿 API

**공식 문서**:
- [Understanding Range Concepts in C++20](https://github.com/MicrosoftDocs/cpp-docs/blob/main/docs/standard-library/range-concepts.md)
- [Understanding Iterator Concepts in C++20](https://github.com/MicrosoftDocs/cpp-docs/blob/main/docs/standard-library/iterator-concepts.md)

**DongArch3D 적용 예시**:
```cpp
// Before (C++17)
template<typename T>
void processVertices(const std::vector<T>& vertices);

// After (C++20)
template<std::ranges::range R>
requires std::same_as<std::ranges::range_value_t<R>, Vertex>
void processVertices(const R& vertices);
```

### 2. Ranges (범위)
더 간결하고 효율적인 컬렉션 처리

**공식 문서**:
- [Using Ranges and Views in C++20](https://github.com/MicrosoftDocs/cpp-docs/blob/main/docs/standard-library/ranges.md)

**DongArch3D 적용 예시**:
```cpp
// Cutline 추출 시 필터링 (C++17)
std::vector<Edge> silhouetteEdges;
for (const auto& edge : mesh.edges()) {
    if (edge.isSilhouette()) {
        silhouetteEdges.push_back(edge);
    }
}

// Cutline 추출 시 필터링 (C++20 Ranges)
auto silhouetteEdges = mesh.edges()
    | std::views::filter([](const Edge& e) { return e.isSilhouette(); })
    | std::ranges::to<std::vector>();
```

### 3. std::span
안전한 배열 뷰 (포인터 + 크기)

**DongArch3D 적용 예시**:
```cpp
// Before (C++17)
void renderMesh(const float* vertices, size_t vertexCount);

// After (C++20)
void renderMesh(std::span<const float> vertices);
```

### 4. Designated Initializers
구조체 초기화 간소화

**DongArch3D 적용 예시**:
```cpp
struct CutlineParameters {
    float planeHeight;
    int smoothingLevel;
    bool useSpline;
};

// C++20
CutlineParameters params {
    .planeHeight = 10.0f,
    .smoothingLevel = 3,
    .useSpline = true
};
```

### 5. Constexpr 확장
더 많은 함수를 컴파일 타임에 실행

**DongArch3D 적용 예시**:
```cpp
// 컴파일 타임 벡터 연산
constexpr Vector3 crossProduct(const Vector3& a, const Vector3& b) {
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
```

## 학습 순서 추천

### 초급 (1-2일)
1. Designated Initializers - 즉시 적용 가능
2. std::span - 안전한 배열 처리
3. `<=>` (Three-way comparison) - 비교 연산자 간소화

### 중급 (3-5일)
4. Ranges 기본 - filter, transform, take
5. Concepts 기본 - 타입 제약 이해
6. Constexpr 확장 - 컴파일 타임 연산

### 고급 (1주)
7. Coroutines - 비동기 처리 (선택사항)
8. Modules - 빌드 시간 단축 (부분 지원)

## cppreference.com 링크

- **C++20 기능 전체 목록**: https://en.cppreference.com/w/cpp/20
- **Concepts 라이브러리**: https://en.cppreference.com/w/cpp/concepts
- **Ranges 라이브러리**: https://en.cppreference.com/w/cpp/ranges
- **std::span**: https://en.cppreference.com/w/cpp/container/span

## Microsoft C++ 문서

- **C++20 새로운 기능**: https://learn.microsoft.com/en-us/cpp/overview/cpp-conformance-improvements?view=msvc-170
- **MSVC C++20 지원 상태**: https://learn.microsoft.com/en-us/cpp/overview/visual-cpp-language-conformance

## 온라인 튜토리얼

- **ModernesCpp**: https://www.modernescpp.com/index.php/category/c-20
- **C++ Stories**: https://www.cppstories.com/p/cpp20.html

## GigaMesh와의 호환성

GigaMesh Core는 C++17로 작성되었지만, DongArch3D의 새 코드는 C++20 기능을 자유롭게 사용할 수 있습니다:

- ✅ GigaMesh core 라이브러리: C++17 유지
- ✅ DongArch3D GUI 코드: C++20 사용
- ✅ DongArch3D Arch3D Liner 기능: C++20 사용

C++ 표준은 상위 호환되므로 문제 없습니다.

## 주의사항

### GCC 버전 업그레이드 필요
현재 GigaMesh는 GCC 8+를 요구하지만, C++20은 GCC 10+가 필요합니다.

**Linux 환경**:
```bash
# Ubuntu 20.04+
sudo apt install g++-10

# CMake 빌드 시
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-10
```

### MSVC는 이미 지원
Visual Studio 2019 16.0+ (MSVC 19.20+)는 C++20 핵심 기능을 완벽 지원합니다.

## DongArch3D에 적용할 C++20 기능 우선순위

| 우선순위 | 기능 | Phase | 적용 대상 |
|---------|------|-------|----------|
| ⭐⭐⭐⭐⭐ | std::span | Phase 0 | 메시 데이터 전달 |
| ⭐⭐⭐⭐⭐ | Ranges | Phase 2-3 | Cutline, Outline 필터링 |
| ⭐⭐⭐⭐ | Designated initializers | 전체 | 설정 구조체 |
| ⭐⭐⭐⭐ | Concepts | Phase 1-2 | 제네릭 알고리즘 |
| ⭐⭐⭐ | Constexpr 확장 | Phase 2-5 | 수학 연산 |
| ⭐⭐ | Modules | Phase 0 | 빌드 시간 단축 (선택) |

---

**생성일**: 2025-11-08
**프로젝트**: DongArch3D v2.0.0 (Arch3D Liner 기반)
**C++ 버전 목표**: C++20
