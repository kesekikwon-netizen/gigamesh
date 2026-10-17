# Qt 5.15 리소스 및 문서

DongArch3D 프로젝트를 위한 Qt 5.15 핵심 기술 문서 모음

## 프로젝트 Qt 정보

- **Qt 버전**: Qt 5.15.2
- **사용 모듈**: Core, Widgets, Gui, OpenGL, Network
- **빌드 시스템**: CMake 3.10+
- **번역 시스템**: Qt Linguist (lupdate/lrelease)

## 핵심 클래스 문서

### 1. QOpenGLWidget
3D 메시 렌더링을 위한 OpenGL 위젯

**공식 문서**:
- Qt 5.15 QOpenGLWidget: https://doc.qt.io/qt-5/qopenglwidget.html
- QOpenGLFunctions: https://doc.qt.io/qt-5/qopenglfunctions.html
- QOpenGLContext: https://doc.qt.io/qt-5/qopenglcontext.html

**DongArch3D 적용**:
```cpp
// GigaMesh/gui/src/meshwidget.h 기반
class DongArchGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
};
```

**주요 메서드**:
- `initializeGL()`: OpenGL 초기화 (VBO, 셰이더 로드)
- `paintGL()`: 매 프레임 렌더링
- `resizeGL()`: 뷰포트 리사이즈
- `update()`: 재렌더링 요청

### 2. Signals and Slots
Qt의 이벤트 시스템

**공식 문서**:
- Signals & Slots: https://doc.qt.io/qt-5/signalsandslots.html
- Meta-Object System: https://doc.qt.io/qt-5/metaobjects.html

**DongArch3D 적용 예시**:
```cpp
// Cutline 추출 시그널
class DongArchCutlineManager : public QObject
{
    Q_OBJECT
signals:
    void cutlineExtracted(const std::vector<Polyline>& lines);
    void progressChanged(int percentage);

public slots:
    void extractTopCut(float height);
    void extractFrontCut(float yPosition);
};

// 연결
connect(cutlineManager, &DongArchCutlineManager::cutlineExtracted,
        mainWindow, &DongArchMainWindow::onCutlineReady);
```

### 3. QMainWindow
메인 윈도우 구조

**공식 문서**:
- QMainWindow: https://doc.qt.io/qt-5/qmainwindow.html
- QDockWidget: https://doc.qt.io/qt-5/qdockwidget.html
- QToolBar: https://doc.qt.io/qt-5/qtoolbar.html

**DongArch3D 구조**:
```
QGMMainWindow (메인 윈도우)
├── MeshWidget (중앙 3D 뷰)
├── DongArchToolPalette (좌측 도구 팔레트)
├── DongArchPropertyPanel (우측 속성 패널)
└── DongArchMeasurementPanel (하단 측정 패널)
```

### 4. Qt Resource System
리소스 파일 관리 (.qrc)

**공식 문서**:
- Qt Resource System: https://doc.qt.io/qt-5/resources.html

**DongArch3D 리소스**:
```xml
<!-- gui/resources.qrc -->
<RCC>
    <qresource prefix="/icons">
        <file>icons/cutline.png</file>
        <file>icons/outline.png</file>
        <file>icons/clip.png</file>
    </qresource>
    <qresource prefix="/shaders">
        <file>shaders/vertex.glsl</file>
        <file>shaders/fragment.glsl</file>
    </qresource>
</RCC>
```

**사용법**:
```cpp
QIcon cutlineIcon(":/icons/cutline.png");
QString shaderCode = readFile(":/shaders/vertex.glsl");
```

## Qt 국제화 (i18n) 시스템

### 번역 워크플로우

**공식 문서**:
- Internationalization: https://doc.qt.io/qt-5/internationalization.html
- Qt Linguist Manual: https://doc.qt.io/qt-5/qtlinguist-index.html

**DongArch3D 번역 프로세스**:
```bash
# 1. 소스에서 번역 문자열 추출
cd GigaMesh
lupdate -recursive gui/src -ts gui/languages/DongArch3D_ko.ts

# 2. Qt Linguist로 번역 (GUI 도구)
linguist gui/languages/DongArch3D_ko.ts

# 3. 바이너리 .qm 파일 생성
lrelease gui/languages/DongArch3D_ko.ts

# 4. 빌드에 포함 (CMake가 자동 처리)
# translations.qrc에 등록됨
```

**코드에서 사용**:
```cpp
// 번역 가능 문자열
QPushButton* button = new QPushButton(tr("단면 라인 추출"));
QString message = tr("메시 로드 완료: %1개 정점").arg(vertexCount);

// 번역 로드 (main.cpp)
QTranslator translator;
translator.load(":/languages/DongArch3D_ko.qm");
app.installTranslator(&translator);
```

## CMake와 Qt 통합

**공식 문서**:
- Qt CMake Manual: https://doc.qt.io/qt-5/cmake-manual.html

**DongArch3D CMakeLists.txt** (gui/CMakeLists.txt 기반):
```cmake
find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL Network REQUIRED)

set(CMAKE_AUTOMOC ON)  # Meta-Object Compiler 자동 실행
set(CMAKE_AUTORCC ON)  # Resource Compiler 자동 실행
set(CMAKE_AUTOUIC ON)  # UI Compiler 자동 실행

qt5_add_resources(GUI_RESOURCES
    forms/gigamesh.qrc
    src/shaders/shaders.qrc
    languages/translations.qrc
)

add_executable(DongArch3D ${GUI_SOURCES} ${GUI_RESOURCES})
target_link_libraries(DongArch3D PRIVATE Qt5::Widgets Qt5::OpenGL)
```

## Qt Designer & UI 파일

**공식 문서**:
- Qt Designer Manual: https://doc.qt.io/qt-5/qtdesigner-manual.html
- Using UI Files: https://doc.qt.io/qt-5/designer-using-a-ui-file.html

**DongArch3D UI 파일 사용**:
```cpp
// forms/mainWin.ui → ui_mainWin.h (자동 생성)
#include "ui_mainWin.h"

class DongArchMainWindow : public QMainWindow
{
private:
    Ui::MainWindow *ui;

public:
    DongArchMainWindow() {
        ui = new Ui::MainWindow;
        ui->setupUi(this);

        // UI 요소 접근
        connect(ui->actionCutline, &QAction::triggered,
                this, &DongArchMainWindow::onCutlineAction);
    }
};
```

## Qt 스타일 시트 (QSS)

**공식 문서**:
- Qt Style Sheets: https://doc.qt.io/qt-5/stylesheet.html
- Qt Style Sheets Reference: https://doc.qt.io/qt-5/stylesheet-reference.html

**DongArch3D 다크 모드** (참고: GigaMesh/gui/src/QGMDarkModeManager.cpp):
```cpp
QString darkStyleSheet = R"(
    QMainWindow {
        background-color: #2b2b2b;
        color: #ffffff;
    }
    QPushButton {
        background-color: #3c3c3c;
        border: 1px solid #555555;
        padding: 5px;
        border-radius: 3px;
    }
    QPushButton:hover {
        background-color: #4c4c4c;
    }
)";
app.setStyleSheet(darkStyleSheet);
```

## DongArch3D에서 사용하는 Qt 클래스 목록

### 핵심 클래스
| 클래스 | 용도 | 파일 |
|--------|------|------|
| QOpenGLWidget | 3D 렌더링 | meshwidget.cpp |
| QMainWindow | 메인 윈도우 | QGMMainWindow.cpp |
| QDockWidget | 패널 (Tool, Property, Measurement) | DongArch*Panel.cpp |
| QAction | 메뉴/도구바 액션 | QGMMainWindow.cpp |
| QSettings | 설정 저장/로드 | DongArchPreferencesDialog.cpp |

### 다이얼로그
| 클래스 | 용도 |
|--------|------|
| QDialog | 커스텀 다이얼로그 기반 |
| QFileDialog | 파일 열기/저장 |
| QColorDialog | 색상 선택 |
| QMessageBox | 알림/확인 메시지 |

### 레이아웃
| 클래스 | 용도 |
|--------|------|
| QVBoxLayout | 세로 레이아웃 |
| QHBoxLayout | 가로 레이아웃 |
| QGridLayout | 그리드 레이아웃 |
| QSplitter | 크기 조절 가능한 분할 |

## GitHub 참고 문서

수집된 문서:
- [Building Qt 6 from Source with CMake](https://github.com/qt/qt5/blob/dev/README.md)
- [Configuring Qt Modules](https://github.com/qt/qtbase/blob/dev/cmake/README.md)
- [CXX-Qt CMake Integration](https://github.com/KDAB/cxx-qt/blob/v0.7.2/book/src/getting-started/5-cmake-integration.md)

## Qt 학습 순서

### 1단계: Qt 기초 (2-3일)
- Signals & Slots
- QObject 상속
- QWidget 기본

### 2단계: GUI 구성 (3-4일)
- QMainWindow, QDockWidget
- Qt Designer 사용법
- 레이아웃 시스템

### 3단계: OpenGL 통합 (4-5일)
- QOpenGLWidget
- QOpenGLFunctions
- 셰이더 로드

### 4단계: 고급 기능 (1주)
- Resource System
- Internationalization
- Custom Widgets

## 온라인 튜토리얼

- **Qt 공식 튜토리얼**: https://doc.qt.io/qt-5/qtexamplesandtutorials.html
- **Qt 공식 예제**: https://doc.qt.io/qt-5/qtexamples.html
- **Qt Wiki**: https://wiki.qt.io/Main
- **Qt Forum**: https://forum.qt.io/

## 주의사항

### Qt 5 vs Qt 6
DongArch3D는 Qt 5.15.2를 사용합니다 (GigaMesh 기반).
Qt 6 문서를 참고할 때 API 변경사항 주의하세요.

### Windows 배포
```bash
# windeployqt로 의존성 자동 포함
cd build_korean/gui/Release
C:/Qt/5.15.2/msvc2019_64/bin/windeployqt.exe DongArch3D.exe
```

---

**생성일**: 2025-11-08
**프로젝트**: DongArch3D v2.0.0 (Arch3D Liner 기반)
**Qt 버전**: 5.15.2
