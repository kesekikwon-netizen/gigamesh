# DongArch3D - Task별 독립 실행 계획

**작성일**: 2025-11-12
**목적**: 각 Task를 `/compact` → 작업 → `/clear` 사이클로 독립 실행

---

## 📋 전체 Task 목록 (15개)

```
Phase A: 빌드 환경 (4 tasks)
├─ A1: Qt5 설치 확인
├─ A2: 기본 빌드 (dongarch 제외)
├─ A3: 실행 파일 테스트
└─ A4: 한글 UI 확인

Phase B: DongArch 통합 (5 tasks)
├─ B1: dongarch 빌드 확인
├─ B2: Cutline 메뉴 연결
├─ B3: Cutline 동작 테스트
├─ B4: Outline 동작 테스트
└─ B5: 기타 기능 확인

Phase C: 안정화 (6 tasks)
├─ C1: nullptr 크래시 수정
├─ C2: Qt 시그널/슬롯 확인
├─ C3: OpenGL 오류 수정
├─ C4: 에러 메시지 한글화
├─ C5: 성능 측정
└─ C6: 최종 테스트
```

---

## 🚀 Task A1: Qt5 설치 확인

### 목표
Qt5가 설치되어 있는지 확인하고, 없으면 설치

### 실행 명령어
```bash
cd /workspaces/git11

# 1. Qt5 설치 확인
dpkg -l | grep -i "qt5-default\|qtbase5-dev" > qt5_check.txt
cat qt5_check.txt

# 2. 없으면 설치
if [ ! -s qt5_check.txt ]; then
    sudo apt-get update
    sudo apt-get install -y qt5-default qtbase5-dev qttools5-dev libqt5opengl5-dev
fi

# 3. 버전 확인
qmake --version
cmake --version
g++ --version

# 4. 결과 저장
echo "Qt5 Check Complete" > .task_a1_done
```

### 완료 조건
- [ ] `qmake --version` 출력: Qt 5.x 이상
- [ ] `cmake --version` 출력: 3.10 이상
- [ ] `g++ --version` 출력: 8.0 이상
- [ ] `.task_a1_done` 파일 생성됨

### 다음 Task
✅ 완료 → **Task A2** 진행

---

## 🚀 Task A2: 기본 빌드 (dongarch 제외)

### 목표
GigaMesh 기본 기능만 빌드 (dongarch는 일단 제외)

### 사전 확인
```bash
# Task A1 완료 확인
test -f .task_a1_done && echo "✅ Task A1 완료" || echo "❌ Task A1 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. CMakeLists.txt 백업
cp gui/CMakeLists.txt gui/CMakeLists.txt.backup

# 2. dongarch 라인 주석 처리 (임시)
sed -i 's/^[[:space:]]*src\/dongarch/#&/' gui/CMakeLists.txt

# 3. 빌드 디렉토리 생성
mkdir -p build
cd build

# 4. CMake 설정
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 2>&1 | tee cmake_config.log

# 5. 빌드
make -j$(nproc) 2>&1 | tee build.log

# 6. 결과 확인
ls -lh gui/gigamesh
file gui/gigamesh

# 7. 완료 표시
cd /workspaces/git11
echo "Basic Build Complete" > .task_a2_done
```

### 완료 조건
- [ ] `build/gui/gigamesh` 파일 존재
- [ ] 파일 크기 5MB 이상
- [ ] `file` 명령 결과: ELF 64-bit executable
- [ ] `.task_a2_done` 파일 생성됨

### 실패 시
```bash
# 에러 로그 확인
tail -50 build/build.log

# Qt 못 찾는 오류:
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/qt5
```

### 다음 Task
✅ 완료 → **Task A3** 진행

---

## 🚀 Task A3: 실행 파일 테스트

### 목표
빌드된 gigamesh가 실행되는지 확인

### 사전 확인
```bash
test -f .task_a2_done && echo "✅ Task A2 완료" || echo "❌ Task A2 먼저 실행"
test -f GigaMesh/build/gui/gigamesh && echo "✅ 실행 파일 존재" || echo "❌ 빌드 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh/build/gui

# 1. 라이브러리 의존성 확인
ldd gigamesh | grep "not found"

# 2. 헬프 메시지 테스트 (GUI 없이)
./gigamesh --help 2>&1 | head -20

# 3. 버전 확인
./gigamesh --version 2>&1

# 4. 완료 표시
cd /workspaces/git11
echo "Execution Test Complete" > .task_a3_done
```

### 완료 조건
- [ ] `ldd gigamesh` 결과: "not found" 없음
- [ ] `--help` 또는 `--version` 정상 출력
- [ ] `.task_a3_done` 파일 생성됨

### 실패 시
```bash
# Qt 라이브러리 못 찾는 경우
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
./gigamesh --version
```

### 다음 Task
✅ 완료 → **Task A4** 진행

---

## 🚀 Task A4: 한글 UI 확인

### 목표
한글 번역이 제대로 적용되었는지 확인

### 사전 확인
```bash
test -f .task_a3_done && echo "✅ Task A3 완료" || echo "❌ Task A3 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. 번역 파일 확인
ls -lh gui/languages/GigaMesh_ko.qm
file gui/languages/GigaMesh_ko.qm

# 2. qrc_translations.cpp 확인 (번역 임베딩)
ls -lh build/gui/qrc_translations.cpp
grep -c "GigaMesh_ko.qm" build/gui/qrc_translations.cpp

# 3. 실행 파일에 번역 포함 확인
strings build/gui/gigamesh | grep -c "GigaMesh_ko.qm"

# 4. 완료 표시
cd /workspaces/git11
echo "Korean UI Check Complete" > .task_a4_done
echo "=== Phase A 완료 ===" >> .task_a4_done
```

### 완료 조건
- [ ] `GigaMesh_ko.qm` 크기 100KB 이상
- [ ] `qrc_translations.cpp` 존재
- [ ] 실행 파일에 번역 포함됨
- [ ] `.task_a4_done` 파일 생성됨

### 다음 Phase
✅ Phase A 완료 → **Phase B** 시작

---

## 🚀 Task B1: dongarch 빌드 확인

### 목표
dongarch 소스 코드를 빌드에 포함시키고 컴파일

### 사전 확인
```bash
test -f .task_a4_done && echo "✅ Phase A 완료" || echo "❌ Phase A 먼저 완료"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. CMakeLists.txt 복원 (dongarch 활성화)
cp gui/CMakeLists.txt.backup gui/CMakeLists.txt

# 2. dongarch 소스 파일 확인
find gui/src/dongarch -name "*.cpp" | wc -l
find gui/src/dongarch -name "*.h" | wc -l

# 3. 빌드 디렉토리 클린
cd build
rm -rf CMakeFiles CMakeCache.txt

# 4. 재설정 및 빌드
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 2>&1 | tee cmake_with_dongarch.log
make -j$(nproc) 2>&1 | tee build_with_dongarch.log

# 5. dongarch 컴파일 확인
grep "dongarch" build_with_dongarch.log | head -20

# 6. 실행 파일 크기 비교
ls -lh gui/gigamesh

# 7. 완료 표시
cd /workspaces/git11
echo "DongArch Build Complete" > .task_b1_done
```

### 완료 조건
- [ ] dongarch/*.cpp 파일들이 컴파일됨
- [ ] 링크 오류 없음
- [ ] 실행 파일 크기 10MB 이상 (dongarch 포함)
- [ ] `.task_b1_done` 파일 생성됨

### 실패 시
```bash
# 컴파일 에러 확인
grep "error:" build/build_with_dongarch.log | head -20

# C++20 에러 → C++17로 변경
cmake .. -DCMAKE_CXX_STANDARD=17
```

### 다음 Task
✅ 완료 → **Task B2** 진행

---

## 🚀 Task B2: Cutline 메뉴 연결

### 목표
QGMMainWindow에서 Cutline 메뉴가 보이는지 확인

### 사전 확인
```bash
test -f .task_b1_done && echo "✅ Task B1 완료" || echo "❌ Task B1 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. QGMMainWindow에서 cutline 메뉴 코드 확인
grep -n "cutlineMenu\|onCutlineTop" gui/src/QGMMainWindow.cpp | head -10

# 2. 메뉴 생성 라인 수 확인
grep -c "Cutline" gui/src/QGMMainWindow.cpp

# 3. DongArchCutlineManager 헤더 include 확인
grep "#include.*DongArchCutlineManager" gui/src/QGMMainWindow.cpp

# 4. 없으면 추가 필요
if ! grep -q "DongArchCutlineManager" gui/src/QGMMainWindow.cpp; then
    echo "❌ QGMMainWindow.cpp에 DongArchCutlineManager include 필요"
else
    echo "✅ include 확인됨"
fi

# 5. 완료 표시
cd /workspaces/git11
echo "Cutline Menu Check Complete" > .task_b2_done
```

### 완료 조건
- [ ] `cutlineMenu` 코드 존재
- [ ] `onCutlineTop()` 함수 존재
- [ ] `DongArchCutlineManager` include 확인
- [ ] `.task_b2_done` 파일 생성됨

### 수정 필요 시
```cpp
// gui/src/QGMMainWindow.h에 추가
#include "dongarch/cutline/DongArchCutlineManager.h"

// gui/src/QGMMainWindow.h 멤버 변수 추가
private:
    DongArch::Cutline::CutlineManager* mCutlineManager;
```

### 다음 Task
✅ 완료 → **Task B3** 진행

---

## 🚀 Task B3: Cutline 동작 테스트

### 목표
Cutline 기능이 **Arch3D Liner 매뉴얼 방식대로** 작동하는지 테스트

### ⭐ Arch3D Liner UI 기준 (필수)

**PDF 페이지 32-34 참고**: `/workspaces/git11/docs/archaeology/Arch3D Liner User Guide ver.2023.07.01.01.pdf`

```
┌──────────────────────────────────────────────────────────────┐
│  Cutline                                            [Main >] │
├────────────────────────────┬─────────────────────────────────┤
│                            │                                 │
│   3D 뷰 (왼쪽)             │   2D 미리보기 (오른쪽)           │
│                            │                                 │
│  ┌──────────────────────┐ │  ┌───────────────────────────┐ │
│  │                      │ │  │                           │ │
│  │  [3D 모델]           │ │  │   추출된 단면선            │ │
│  │                      │ │  │   (파란색 벡터 라인)       │ │
│  │  + [절단 평면]       │ │  │                           │ │
│  │    (반투명 주황색)   │ │  │   흰색 배경               │ │
│  │                      │ │  │                           │ │
│  │  + [교차선]          │ │  │                           │ │
│  │    (주황색 하이라이트)│ │  │                           │ │
│  │                      │ │  │                           │ │
│  └──────────────────────┘ │  └───────────────────────────┘ │
│                            │                                 │
│  [Rotation Scroll Bar]     │  [Checkered Grid] [Fit] [100%] │
│  [-180° ████████████ 180°] │                                 │
│                            │                                 │
│  [Translation Scroll Bar]  │  [Save Slots]                   │
│  [0.5%]                    │  [Save0][Save1][Save2]...       │
│                            │                                 │
│  [Rotation Angle Buttons]  │  [Line Property]                │
│  [-90°][-5°][-1°][+1°]...  │  - Scale Lv: 2. Mid            │
│                            │  - Detail Lv: 2. Mid            │
│  [Line Style]              │  - Curve Lv: 2. Mid             │
│  ✓ Polyline Origin         │                                 │
│  ✓ Spline Curves           │                                 │
└────────────────────────────┴─────────────────────────────────┘
```

### 필수 구현 요소

**1️⃣ 3D 뷰 (왼쪽) - 반드시 표시되어야 함**
```cpp
✅ 3D 모델 렌더링
✅ 절단 평면 시각화 (반투명 회색 또는 주황색)
✅ 교차선 하이라이트 (주황색 굵은 선으로 3D 모델 위에 표시)
✅ 회전 스크롤 바 (-180° ~ 180°)
✅ Translation 스크롤 바 (0.1% 단위)
✅ Rotation Angle 버튼 (-90°, -5°, -1°, +1°, +5°, +90°)
```

**2️⃣ 2D 미리보기 (오른쪽) - 반드시 표시되어야 함**
```cpp
✅ 추출된 단면선만 표시 (깨끗한 벡터 라인)
✅ 흰색 배경
✅ 파란색 또는 검은색 선
✅ Douglas-Peucker 간략화 적용됨
✅ Catmull-Rom Spline 스무딩 적용됨
✅ Checkered Grid 옵션
✅ Fit In / 100% Zoom / Zoom In/Out 버튼
```

**3️⃣ Line Property 설정**
```cpp
struct LineProperty {
    int scaleLv = 2;      // Detail Lv의 스케일 배수
    int detailLv = 2;     // 2. Mid (라인 디테일)
    int pointsNumLv = 2;  // 2. Mid (점 개수)
    int curveLv = 2;      // 2. Mid (곡선 부드러움)
};
```

**4️⃣ Line Style 설정**
```cpp
struct LineStyle {
    // Polyline Origin (원본 교차선)
    struct {
        int penWidth = 2;
        float transparent = 0.80f;
        QColor color = Qt::blue;
    } polylineOrigin;

    // Spline Curves (스무딩 곡선)
    struct {
        int penWidth = 1;
        float transparent = 0.80f;
        QColor color = Qt::darkBlue;
    } splineCurves;
};
```

**5️⃣ Save Slot 시스템**
```cpp
// 5개 슬롯 (Save0 ~ Save4)
enum SaveSlot {
    SAVE0 = 0,  // P0
    SAVE1 = 1,  // P1
    SAVE2 = 2,  // P2
    SAVE3 = 3,  // P3
    SAVE4 = 4   // P4
};

// 각 슬롯은 라인 데이터 저장
struct CutlineSlot {
    std::vector<Vector3D> rawPoints;      // 원본 교차점
    std::vector<Vector3D> simplifiedPoints; // Douglas-Peucker 적용
    std::vector<Vector3D> smoothedPoints;   // Catmull-Rom 적용
    double planeHeight;
    Vector3D planeNormal;
    LineProperty property;
    LineStyle style;
};
```

### 예상 결과 (Arch3D Liner 기준)

**Top Cut (상면 - 페이지 34)**:
```
3D 뷰: 위에서 본 모델 + 수평 절단 평면
2D 뷰: 원형/타원형 외곽선 (깨끗한 벡터)
```

**Front Cut (정면)**:
```
3D 뷰: 앞에서 본 모델 + 수직 절단 평면
2D 뷰: 정면 실루엣 (직사각형/윤곽선)
```

**Right Cut (우측)**:
```
3D 뷰: 오른쪽에서 본 모델 + 수직 절단 평면
2D 뷰: 측면 실루엣 (곡선형 윤곽)
```

### 사전 확인
```bash
test -f .task_b2_done && echo "✅ Task B2 완료" || echo "❌ Task B2 먼저 실행"
test -f GigaMesh/build/gui/gigamesh && echo "✅ 실행 파일 존재" || echo "❌ 빌드 필요"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. 테스트용 PLY 파일 확인
find testdata -name "*.ply" | head -5

# 2. 실행 파일에 DongArchCutlineManager 심볼 확인
nm build/gui/gigamesh | grep -i "cutline" | head -10

# 3. onCutlineTop 함수 구현 확인
grep -A 30 "void QGMMainWindow::onCutlineTop()" gui/src/QGMMainWindow.cpp

# 4. DongArchCutlineDialog 클래스 확인
grep -n "class DongArchCutlineDialog" gui/src/dongarch/cutline/DongArchCutlineDialog.h

# 5. Split View 구조 확인 (3D 뷰 + 2D 미리보기)
grep -n "QSplitter\|Q3DView\|Q2DPreview" gui/src/dongarch/cutline/DongArchCutlineDialog.cpp

# 6. 완료 표시
cd /workspaces/git11
echo "Cutline Function Test Complete" > .task_b3_done
```

### 완료 조건 (Arch3D Liner 기준)
- [ ] `onCutlineTop()` 함수 구현 확인됨
- [ ] DongArchCutlineDialog 클래스 존재
- [ ] **Split View (3D 뷰 + 2D 미리보기) 구조 확인**
- [ ] **절단 평면이 3D 뷰에 표시됨**
- [ ] **교차선이 3D 모델에 하이라이트됨**
- [ ] **2D 미리보기에 추출된 라인만 표시됨**
- [ ] 회전 스크롤 바 존재
- [ ] Translation 스크롤 바 존재
- [ ] Line Property 설정 패널 존재
- [ ] Save Slot 버튼 (5개) 존재
- [ ] `.task_b3_done` 파일 생성됨

### 수정 필요 시

**최소 구현 (MVP)**:
```cpp
// gui/src/QGMMainWindow.cpp
void QGMMainWindow::onCutlineTop() {
    std::cout << "[DEBUG] Cutline Top called" << std::endl;

    if (mMeshWidget == nullptr) {
        QMessageBox::warning(this, tr("경고"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    // Arch3D Liner 스타일 Dialog 생성
    DongArchCutlineDialog* dialog = new DongArchCutlineDialog(this);
    dialog->setMesh(mMeshWidget->getMesh());
    dialog->setCutlineType(DongArchCutlineDialog::TOP_CUT);
    dialog->show();
}
```

**DongArchCutlineDialog 구조 (필수)**:
```cpp
class DongArchCutlineDialog : public QDialog {
    Q_OBJECT
public:
    enum CutlineType { TOP_CUT, FRONT_CUT, RIGHT_CUT };

    DongArchCutlineDialog(QWidget* parent = nullptr);
    void setMesh(Mesh* mesh);
    void setCutlineType(CutlineType type);

private:
    // Arch3D Liner 스타일 Split View
    QSplitter* mSplitter;

    // 왼쪽: 3D 뷰
    QWidget* m3DView;           // 3D 모델 + 절단 평면 표시
    QScrollBar* mRotationBar;   // -180 ~ 180
    QScrollBar* mTranslationBar; // 0.1% 단위

    // 오른쪽: 2D 미리보기
    QWidget* m2DPreview;        // 추출된 라인만 표시
    QPushButton* mFitButton;    // Fit In
    QPushButton* mZoom100Button; // 100%

    // Line Property
    QSpinBox* mScaleLv;
    QComboBox* mDetailLv;       // Low/Mid-Low/Mid/Mid-High/High
    QComboBox* mPointsNumLv;
    QComboBox* mCurveLv;

    // Save Slots
    QPushButton* mSaveSlots[5]; // Save0 ~ Save4

    // Line Style
    QSpinBox* mPolylineWidth;
    QDoubleSpinBox* mPolylineTransparent;
    QColorDialog* mPolylineColor;

    Mesh* mMesh;
    CutlineType mCutlineType;

    void extractCutline();      // GigaMesh calcIntersectionPolylineWithPlane() 사용
    void applyDouglasPeucker(); // 간략화
    void applyCatmullRom();     // 스무딩
    void update3DView();        // 3D 뷰 업데이트 (절단 평면 + 하이라이트)
    void update2DPreview();     // 2D 미리보기 업데이트 (라인만)
};
```

### 참고: Arch3D Liner PDF
```bash
# PDF 확인
evince "/workspaces/git11/docs/archaeology/Arch3D Liner User Guide ver.2023.07.01.01.pdf" &

# 핵심 페이지:
# - 페이지 32: Cutline UI 전체 구조
# - 페이지 33: Line Property, Save Slot
# - 페이지 34: Top Cut / Front Cut / Right Cut 예시
```

### 다음 Task
✅ 완료 → **Task B4** 진행

---

## 🚀 Task B4: Outline 동작 테스트

### 목표
Outline 기능 코드 확인

### 사전 확인
```bash
test -f .task_b3_done && echo "✅ Task B3 완료" || echo "❌ Task B3 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. Outline 소스 파일 확인
ls -lh gui/src/dongarch/outline/

# 2. QGMMainWindow에서 outline 메뉴 확인
grep -n "outlineMenu\|onOutline" gui/src/QGMMainWindow.cpp | head -10

# 3. SilhouetteDetector 심볼 확인
nm build/gui/gigamesh | grep -i "silhouette" | head -10

# 4. 완료 표시
cd /workspaces/git11
echo "Outline Function Test Complete" > .task_b4_done
```

### 완료 조건
- [ ] outline 소스 파일 존재
- [ ] outline 메뉴 코드 존재
- [ ] SilhouetteDetector 심볼 존재
- [ ] `.task_b4_done` 파일 생성됨

### 다음 Task
✅ 완료 → **Task B5** 진행

---

## 🚀 Task B5: 기타 기능 확인

### 목표
Align, Clip, Vis 기능 코드 확인

### 사전 확인
```bash
test -f .task_b4_done && echo "✅ Task B4 완료" || echo "❌ Task B4 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. 각 모듈 소스 확인
echo "=== Align ==="
ls gui/src/dongarch/align/

echo "=== Clip ==="
ls gui/src/dongarch/clip/

echo "=== Vis ==="
ls gui/src/dongarch/vis/

# 2. 실행 파일에 포함 확인
nm build/gui/gigamesh | grep -E "Align|Clip|XRay|DTak" | wc -l

# 3. 완료 표시
cd /workspaces/git11
echo "Other Functions Check Complete" > .task_b5_done
echo "=== Phase B 완료 ===" >> .task_b5_done
```

### 완료 조건
- [ ] align, clip, vis 소스 존재
- [ ] 심볼 10개 이상
- [ ] `.task_b5_done` 파일 생성됨

### 다음 Phase
✅ Phase B 완료 → **Phase C** 시작

---

## 🚀 Task C1: nullptr 크래시 수정

### 목표
nullptr 역참조로 인한 크래시 방지

### 사전 확인
```bash
test -f .task_b5_done && echo "✅ Phase B 완료" || echo "❌ Phase B 먼저 완료"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. nullptr 체크 누락 찾기
grep -n "mMeshWidget->" gui/src/QGMMainWindow.cpp | head -20

# 2. nullptr 체크 패턴 찾기
grep -B 2 "if.*nullptr" gui/src/QGMMainWindow.cpp | grep "mMeshWidget" | wc -l

# 3. 수정 필요한 곳 찾기
grep -n "mMeshWidget->" gui/src/QGMMainWindow.cpp | \
    while read line; do
        linenum=$(echo $line | cut -d: -f1)
        prevline=$((linenum - 2))
        sed -n "${prevline},${linenum}p" gui/src/QGMMainWindow.cpp | \
            grep -q "nullptr" || echo "Line $linenum: nullptr check 필요"
    done

# 4. 완료 표시
cd /workspaces/git11
echo "Nullptr Check Complete" > .task_c1_done
```

### 완료 조건
- [ ] nullptr 체크 패턴 확인됨
- [ ] 수정 필요한 곳 리스트업
- [ ] `.task_c1_done` 파일 생성됨

### 수정 패턴
```cpp
// 수정 전
mMeshWidget->doSomething();

// 수정 후
if (mMeshWidget != nullptr) {
    mMeshWidget->doSomething();
}
```

### 다음 Task
✅ 완료 → **Task C2** 진행

---

## 🚀 Task C2: Qt 시그널/슬롯 확인

### 목표
Qt 시그널/슬롯이 제대로 연결되었는지 확인

### 사전 확인
```bash
test -f .task_c1_done && echo "✅ Task C1 완료" || echo "❌ Task C1 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. connect 호출 찾기
grep -n "connect(" gui/src/QGMMainWindow.cpp | grep "cutline\|outline" | head -10

# 2. 시그널/슬롯 선언 확인
grep "slots:" gui/src/QGMMainWindow.h -A 20 | grep "onCutline\|onOutline"

# 3. MOC 파일 확인 (Qt 메타 오브젝트)
ls -lh build/gui/moc_QGMMainWindow.cpp

# 4. 완료 표시
cd /workspaces/git11
echo "Qt Signal/Slot Check Complete" > .task_c2_done
```

### 완료 조건
- [ ] connect() 호출 확인됨
- [ ] slots: 선언 확인됨
- [ ] moc 파일 생성됨
- [ ] `.task_c2_done` 파일 생성됨

### 다음 Task
✅ 완료 → **Task C3** 진행

---

## 🚀 Task C3: OpenGL 오류 수정

### 목표
OpenGL 컨텍스트 오류 방지

### 사전 확인
```bash
test -f .task_c2_done && echo "✅ Task C2 완료" || echo "❌ Task C2 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. OpenGL 호출 찾기
grep -n "glBegin\|glEnd\|glDrawArrays" gui/src/meshGL/meshGL.cpp | head -10

# 2. makeCurrent/doneCurrent 쌍 확인
grep -n "makeCurrent\|doneCurrent" gui/src/meshGL/meshGL.cpp | head -20

# 3. isValid 체크 확인
grep -n "isValid()" gui/src/meshGL/meshGL.cpp | head -10

# 4. 완료 표시
cd /workspaces/git11
echo "OpenGL Check Complete" > .task_c3_done
```

### 완료 조건
- [ ] OpenGL 호출 확인됨
- [ ] makeCurrent/doneCurrent 쌍 확인됨
- [ ] isValid 체크 확인됨
- [ ] `.task_c3_done` 파일 생성됨

### 다음 Task
✅ 완료 → **Task C4** 진행

---

## 🚀 Task C4: 에러 메시지 한글화

### 목표
에러 메시지를 한글로 표시

### 사전 확인
```bash
test -f .task_c3_done && echo "✅ Task C3 완료" || echo "❌ Task C3 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. QMessageBox 호출 찾기
grep -n "QMessageBox" gui/src/QGMMainWindow.cpp | head -10

# 2. tr() 사용 확인
grep "QMessageBox.*tr(" gui/src/QGMMainWindow.cpp | wc -l

# 3. 영어 메시지 찾기
grep 'QMessageBox.*"[A-Z]' gui/src/QGMMainWindow.cpp | head -5

# 4. 완료 표시
cd /workspaces/git11
echo "Korean Error Message Check Complete" > .task_c4_done
```

### 완료 조건
- [ ] QMessageBox 호출 확인됨
- [ ] 대부분 tr() 사용 중
- [ ] `.task_c4_done` 파일 생성됨

### 수정 패턴
```cpp
// 수정 전
QMessageBox::warning(this, "Warning", "Mesh not loaded");

// 수정 후
QMessageBox::warning(this, tr("경고"), tr("메시가 로드되지 않았습니다"));
```

### 다음 Task
✅ 완료 → **Task C5** 진행

---

## 🚀 Task C5: 성능 측정

### 목표
주요 기능의 실행 시간 측정

### 사전 확인
```bash
test -f .task_c4_done && echo "✅ Task C4 완료" || echo "❌ Task C4 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. 성능 측정 코드 확인
grep -n "std::chrono\|clock()" gui/src/dongarch/cutline/DongArchCutlineManager.cpp | head -10

# 2. 로그 출력 확인
grep "elapsed\|msec\|duration" gui/src/dongarch/cutline/DongArchCutlineManager.cpp | head -5

# 3. 완료 표시
cd /workspaces/git11
echo "Performance Check Complete" > .task_c5_done
```

### 완료 조건
- [ ] 성능 측정 코드 존재 확인
- [ ] 로그 출력 확인
- [ ] `.task_c5_done` 파일 생성됨

### 추가 필요 시
```cpp
// 성능 측정 코드
auto start = std::chrono::high_resolution_clock::now();
// ... 작업
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Elapsed: " << duration.count() << " ms" << std::endl;
```

### 다음 Task
✅ 완료 → **Task C6** 진행

---

## 🚀 Task C6: 최종 테스트

### 목표
전체 빌드 + 실행 확인

### 사전 확인
```bash
test -f .task_c5_done && echo "✅ Task C5 완료" || echo "❌ Task C5 먼저 실행"
```

### 실행 명령어
```bash
cd /workspaces/git11/GigaMesh

# 1. 클린 빌드
cd build
make clean
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
make -j$(nproc) 2>&1 | tee final_build.log

# 2. 경고/에러 개수 확인
echo "=== Errors ==="
grep -c "error:" final_build.log
echo "=== Warnings ==="
grep -c "warning:" final_build.log

# 3. 실행 파일 크기
ls -lh gui/gigamesh

# 4. 심볼 개수 확인
nm gui/gigamesh | grep -c "DongArch"

# 5. 완료 표시
cd /workspaces/git11
echo "Final Test Complete" > .task_c6_done
echo "=== Phase C 완료 ===" >> .task_c6_done
echo "=== 전체 완료! ===" >> .task_c6_done
```

### 완료 조건
- [ ] 빌드 에러 0개
- [ ] 실행 파일 생성됨
- [ ] DongArch 심볼 50개 이상
- [ ] `.task_c6_done` 파일 생성됨

### 최종 확인
```bash
# 모든 Task 완료 확인
ls -lh .task_*_done

# 예상 출력:
# .task_a1_done
# .task_a2_done
# .task_a3_done
# .task_a4_done
# .task_b1_done
# .task_b2_done
# .task_b3_done
# .task_b4_done
# .task_b5_done
# .task_c1_done
# .task_c2_done
# .task_c3_done
# .task_c4_done
# .task_c5_done
# .task_c6_done
```

### 🎉 전체 완료!

---

## 📊 진행 상황 추적

### 현재 진행 확인
```bash
cd /workspaces/git11
ls .task_*.done | tail -1
```

### 다음 Task 확인
```bash
# 마지막 완료 Task 확인
LAST_TASK=$(ls .task_*.done 2>/dev/null | tail -1 | sed 's/\.task_//' | sed 's/_done//')
echo "마지막 완료: Task $LAST_TASK"

# 다음 Task 제안
case $LAST_TASK in
    a1) echo "다음: Task A2 (기본 빌드)" ;;
    a2) echo "다음: Task A3 (실행 테스트)" ;;
    a3) echo "다음: Task A4 (한글 UI)" ;;
    a4) echo "다음: Task B1 (dongarch 빌드)" ;;
    b1) echo "다음: Task B2 (Cutline 메뉴)" ;;
    b2) echo "다음: Task B3 (Cutline 테스트)" ;;
    b3) echo "다음: Task B4 (Outline 테스트)" ;;
    b4) echo "다음: Task B5 (기타 기능)" ;;
    b5) echo "다음: Task C1 (nullptr 수정)" ;;
    c1) echo "다음: Task C2 (Qt 시그널)" ;;
    c2) echo "다음: Task C3 (OpenGL)" ;;
    c3) echo "다음: Task C4 (에러 메시지)" ;;
    c4) echo "다음: Task C5 (성능 측정)" ;;
    c5) echo "다음: Task C6 (최종 테스트)" ;;
    c6) echo "🎉 전체 완료!" ;;
    *) echo "다음: Task A1부터 시작" ;;
esac
```

---

## 🔄 Task 실행 템플릿

각 Task는 다음 순서로 실행:

```bash
# 1. /compact (컨텍스트 압축)

# 2. Task 확인
cat DongArch3D_Task별_실행계획.md | grep -A 50 "Task A1"

# 3. 사전 확인 실행

# 4. 실행 명령어 복사-붙여넣기

# 5. 완료 조건 확인

# 6. /clear (컨텍스트 클리어)

# 7. 다음 Task로
```

---

**버전**: 1.0
**총 Task 수**: 15개
**예상 소요 시간**: 3-5일
