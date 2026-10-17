# Phase 3: Outline (외곽 라인 추출) ⚠️

**기간**: 8일 (10일 → 8일, 2일 단축)
**우선순위**: **최우선** (핵심 기술)
**난이도**: ★★★★☆ (두 번째 어려운 부분)
**상태**: 🔴 대기

---

## 🎯 Phase 목표

Arch3D Liner 핵심 기능인 **Outline (외곽 라인 추출)** 구현:
1. **Silhouette Edge Detection** (View-dependent)
2. **벡터화** (Vector Tracing)
3. **Outline UI** (6개 View 선택)

**GigaMesh 활용**: Sobel Edge Detection GLSL 셰이더 참고

---

## 📋 Task 목록

### Task 3.1: Silhouette Edge Detection (5일) 🟡 CPU 버전 구현

**알고리즘**: View-dependent edge detection
```cpp
// 조건: dot(edge_normal, view_direction) == 0
for (auto& edge : mesh->edges()) {
    Face* f1 = edge->face1();
    Face* f2 = edge->face2();
    
    double dot1 = dot3(f1->normal(), viewDirection);
    double dot2 = dot3(f2->normal(), viewDirection);
    
    if (dot1 * dot2 < 0) {
        // Silhouette edge!
        silhouetteEdges.push_back(edge);
    }
}
```

**GigaMesh 참고**:
- `NPR_ApplySobel.frag` - GLSL 셰이더 로직 참고
- CPU 버전으로 변환 필요

### Task 3.2: 벡터화 (Vector Tracing) (3일)
- Silhouette edge → 연속된 2D 라인
- Line simplification 적용 (Task 2.3 재사용)

### Task 3.3: Outline UI (2일)
- 6개 View 선택 버튼
- Save Select View 기능
- 2D 미리보기 창

---

## 🔧 GigaMesh 알고리즘 활용

| 기능 | GigaMesh 소스 | 활용 방법 |
|------|---------------|----------|
| **Sobel Edge Detection** | `NPR_ApplySobel.frag` | GLSL 로직 참고 |

**단축 시간**: 5일 → **3일** (GLSL 구조 참고, 2일 단축)

---

## ✅ 성공 기준
- [ ] Silhouette Edge Detection 동작
- [ ] 6방향 (Front/Back/Left/Right/Top/Bottom) 외곽선 추출
- [ ] 2D 벡터 라인 생성
- [ ] Outline UI에서 View 선택 및 미리보기

---

**문서 버전**: 1.0.0
**최종 업데이트**: 2025-11-08

**핵심 메시지**:
> Phase 3는 Phase 2 다음으로 중요한 Phase입니다!
> Outline 없이는 완성된 2D 도면 생성 불가능!
