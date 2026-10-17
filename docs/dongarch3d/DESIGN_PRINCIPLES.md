# DongArch3D 설계 원칙

**작성일**: 2025-11-08
**버전**: 2.0.0 (Arch3D Liner 기반)

---

## 🎯 핵심 철학

> **"기존 GigaMesh와 새로운 DongArch3D가 섞여있어서 어지럽다"**
> **"새로 만드는 모든 것은 직관적이며 모두 한국어로 만들어져야 한다"**

이것이 DongArch3D의 가장 중요한 설계 원칙입니다.

### Arch3D Liner 워크플로우 원칙
DongArch3D는 Arch3D Liner의 3D → 2D 도면 생성 워크플로우를 구현하며, 다음 핵심 기능을 한국어로 제공합니다:
- **Align (정렬)**: 3D 모델 방향 조정
- **Cutline (단면)**: 단면 라인 추출
- **Outline (외곽)**: 외곽 라인 추출
- **Clip (절단)**: 3D 메시 절단
- **Vis (시각화)**: X-Ray, D-Tak 렌더링
- **SVG Export**: Adobe Illustrator 연동

---

## 1. 완전한 한국어화 원칙

### 규칙
- **새로 만드는 모든 UI, 메뉴, 다이얼로그는 100% 한국어**
- **영어는 코드 주석과 내부 변수명에만 사용**
- **사용자가 보는 모든 텍스트는 한국어**

### 좋은 예
```cpp
// ✅ 올바른 예
mMenuFile = menuBar()->addMenu(tr("파일(&F)"));
mMenuFile->addAction(tr("열기(&O)"));
mMenuFile->addAction(tr("저장(&S)"));
mMenuFile->addAction(tr("내보내기(&E)"));

// 메뉴 항목
- 파일
- 편집
- 보기
- 측정
- 도구
- 도움말
```

### 나쁜 예
```cpp
// ❌ 잘못된 예 - 영어와 한국어 혼재
mMenuFile = menuBar()->addMenu(tr("File"));  // ❌
mMenuFile->addAction(tr("열기"));            // ❌ 혼란스러움

// ❌ 잘못된 메뉴 구조
- File (파일)  // 영어와 한국어 혼재
- Mesh        // 전문 용어
- 보기
```

---

## 2. 직관적 UX 원칙

### 규칙
- **고고학자가 바로 이해할 수 있는 용어 사용**
- **기술 용어 최소화, 설명적 용어 사용**
- **복잡한 메뉴 구조 지양, 단순하고 명확하게**

### 용어 가이드

| ❌ 기술 용어 | ✅ 직관적 용어 | 설명 |
|------------|--------------|------|
| Mesh | 3D 메시 / 3D 모델 | "메시"는 전문가만 이해 |
| PLY Export | PLY 내보내기 / 3D 파일 저장 | 파일 형식보다 행위 중심 |
| Feature Vectors | 특징 분석 | 일반인이 이해 불가 |
| MSII Filter | 다중 스케일 필터 | 약어 사용 금지 |
| NPR Rendering | 일러스트 렌더링 | 기술 용어 회피 |
| Vertex Selection | 점 선택 | 간단명료하게 |
| Face Culling | 뒷면 숨기기 | 기능 중심 설명 |
| Ambient Occlusion | 그림자 효과 | 효과 중심 설명 |

### UI 구조 예시

**기존 GigaMesh (복잡함):**
```
File
  └─ Import
      └─ PLY
      └─ OBJ
  └─ Export
      └─ PLY
      └─ GLTF
  └─ Recent Files
```

**DongArch3D (직관적):**
```
파일
  └─ 열기
  └─ 저장
  └─ 내보내기
      └─ 3D 파일 (PLY)
      └─ 보고서용 (PDF)
  └─ 최근 파일
```

---

## 3. 명확한 분리 원칙

### 코드 구조

```
GigaMesh/ (기존 코드)
├─ core/           ⚠️ 최소 수정만 (버그 수정, 유지보수)
├─ gui/src/        ⚠️ 기존 클래스는 유지
    └─ QGM*.cpp   (GigaMesh 원본)

DongArch3D/ (새 코드)
└─ gui/src/
    ├─ DongArch*.cpp      ✅ 완전히 새로운 한국어 모듈
    ├─ QGMDock*.cpp       ✅ 새로운 한국어 UI
    └─ SectionManager.cpp ✅ 고고학 전용 기능
```

### 클래스 명명 규칙

```cpp
// ❌ 혼재된 명명
class QGMMainWindow {
    void exportMesh();        // 기존 영어
    void 메시내보내기();       // ❌ 혼란스러움
};

// ✅ 명확한 분리
// 기존 GigaMesh 클래스 (최소 수정)
class QGMMainWindow {
    void exportMesh();        // 유지
};

// 새 DongArch3D 클래스 (완전 한국어)
class DongArchExportManager {
    void exportToArchaeology();  // 영어 함수명
    // UI: "고고학 보고서 내보내기"
};
```

---

## 4. 개발 워크플로우

### 새 기능 추가 시

1. **기능 정의**
   - 한국어로 기능 설명 작성
   - 고고학자 관점에서 용어 선택

2. **UI 설계**
   - 모든 텍스트를 한국어로 작성
   - 직관적인 메뉴 구조 설계

3. **코드 작성**
   - 새 클래스/파일로 분리
   - 기존 GigaMesh 코드 최소 수정
   - 함수명은 영어, 주석은 한국어

4. **테스트**
   - 한국어 UI 확인
   - 용어의 직관성 검증

---

## 5. 실전 예시

### 예시 1: 파일 메뉴

**Before (GigaMesh 스타일):**
```cpp
QMenu* fileMenu = menuBar()->addMenu("&File");
fileMenu->addAction("&Open");
fileMenu->addAction("&Save");
fileMenu->addAction("&Export PLY");
```

**After (DongArch3D 스타일):**
```cpp
QMenu* menuFile = menuBar()->addMenu(tr("파일(&F)"));
menuFile->addAction(tr("열기(&O)"));
menuFile->addAction(tr("저장(&S)"));

// 서브메뉴로 명확하게 분류
QMenu* menuExport = menuFile->addMenu(tr("내보내기(&E)"));
menuExport->addAction(tr("3D 파일 (PLY)"));
menuExport->addAction(tr("SVG (일러스트용)"));  // Arch3D Liner 워크플로우
menuExport->addAction(tr("보고서용 (PDF)"));
```

### 예시 2: Arch3D Liner 워크플로우 메뉴

**DongArch3D 스타일 (Arch3D Liner 기능):**
```cpp
// Align (정렬) 메뉴
QMenu* menuAlign = menuBar()->addMenu(tr("정렬(&A)"));
menuAlign->addAction(tr("회전(&R)"));
menuAlign->addAction(tr("방향 설정(&V)"));
menuAlign->addAction(tr("바닥면 자동 정렬(&F)"));

// Cutline (단면) 메뉴
QMenu* menuCutline = menuBar()->addMenu(tr("단면(&C)"));
menuCutline->addAction(tr("위에서 자른 단면 (Top Cut)"));
menuCutline->addAction(tr("앞에서 자른 단면 (Front Cut)"));
menuCutline->addAction(tr("오른쪽에서 자른 단면 (Right Cut)"));

// Outline (외곽) 메뉴
QMenu* menuOutline = menuBar()->addMenu(tr("외곽선(&O)"));
menuOutline->addAction(tr("외곽선 추출"));
menuOutline->addAction(tr("보기 방향 선택"));
```

### 예시 3: 다이얼로그

**Before:**
```cpp
QMessageBox::information(this, "Export Complete",
    "The mesh has been exported successfully.");
```

**After (Arch3D Liner 워크플로우):**
```cpp
// SVG 내보내기 (Adobe Illustrator 연동)
QMessageBox::information(this, tr("SVG 내보내기 완료"),
    tr("단면 라인과 외곽선을 SVG 파일로 내보냈습니다.\n"
       "Adobe Illustrator에서 열어 편집할 수 있습니다."));

// Cutline 추출 완료
QMessageBox::information(this, tr("단면 추출 완료"),
    tr("Top Cut 단면 라인을 성공적으로 추출했습니다.\n"
       "라인 상세도: 0.5mm, 곡선 레벨: Mid"));
```

---

## 6. 점진적 한국어화 전략

### Phase 1 (현재): 새 기능은 100% 한국어
- ✅ 새로운 메뉴, 툴바, 다이얼로그
- ✅ 고고학 전용 기능

### Phase 2: 핵심 UI 한국어화
- 메인 메뉴 바
- 주요 다이얼로그
- 툴바 툴팁

### Phase 3: 전체 UI 한국어화
- 모든 메뉴
- 모든 다이얼로그
- 모든 상태 메시지

### Phase 4: 문서 한국어화
- 도움말
- 튜토리얼
- 오류 메시지

---

## 7. 체크리스트

새 코드를 작성할 때 항상 확인:

- [ ] 사용자가 보는 모든 텍스트가 한국어인가?
- [ ] 고고학자가 바로 이해할 수 있는 용어인가?
- [ ] 기술 용어를 사용했다면 더 직관적인 표현이 없는가?
- [ ] 기존 GigaMesh 코드와 명확히 분리되어 있는가?
- [ ] 메뉴 구조가 단순하고 명확한가?

---

## 8. 금지 사항

❌ **절대 하지 말 것:**
1. 영어와 한국어 메뉴 혼재
2. 전문 기술 용어를 번역 없이 사용
3. 기존 GigaMesh 클래스에 한국어 함수 추가
4. 복잡한 메뉴 계층 구조
5. 약어 사용 (MSII, NPR 등)

---

## 9. 참고 자료

- **UI 가이드**: `.claude/guidelines.md`
- **코딩 규칙**: `CLAUDE.md`
- **용어 사전**: `docs/dongarch3d/TERMINOLOGY.md` (작성 예정)

---

**이 원칙을 모든 개발에 적용하여 DongArch3D를 진정한 한국어 고고학 전용 프로그램으로 만듭니다.**
