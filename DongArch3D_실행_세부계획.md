# DongArch3D 실행 가능한 프로그램 만들기 - 세부 계획

**작성일**: 2025-11-12
**목표**: 코드가 아닌 **실제로 작동하는 실행 파일** 만들기
**현재 문제**: 코드는 있지만 빌드되지 않아 실행 불가

---

## 📊 현재 상태

### ✅ 완료된 것
- [x] GigaMesh 한글 번역 100%
- [x] dongarch 소스 코드 작성 (6,111줄)
  - cutline/ (단면 라인)
  - outline/ (외곽선)
  - clip/ (절단)
  - vis/ (시각화)
  - align/ (정렬)
  - mfe/ (파일 탐색)
  - illustrator/ (SVG 내보내기)
- [x] CMakeLists.txt에 등록
- [x] QGMMainWindow에 메뉴 연결

### ❌ 안 되는 것
- [ ] 빌드 (build 디렉토리 없음)
- [ ] 실행 파일 없음
- [ ] Qt5 설치 확인 필요
- [ ] 새 기능이 UI에서 작동하지 않음

---

## 🎯 실행 가능한 프로그램 만들기 - 3단계

### **Phase A: 환경 구축 및 빌드** (1-2일)
**목표**: GigaMesh + 한글 UI가 실행되는 상태까지

#### Task A1: 빌드 환경 확인 (1시간)
```bash
# Linux (Ubuntu 24.04)
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake \
    qt5-default qtbase5-dev qttools5-dev \
    libqt5opengl5-dev \
    libgl1-mesa-dev libtiff-dev

# 확인
cmake --version  # 3.10+
g++ --version    # 8.0+
qmake --version  # Qt 5.15.2
```

**체크리스트**:
- [ ] CMake 3.10 이상
- [ ] GCC 8.0 이상 (C++17 지원)
- [ ] Qt5 설치 (Core, Widgets, Gui, OpenGL, Network)
- [ ] OpenGL 개발 헤더

---

#### Task A2: 기본 빌드 (2시간)
```bash
cd /workspaces/git11/GigaMesh
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
make -j$(nproc)
```

**예상 결과**:
```
build/gui/gigamesh         # 실행 파일 (약 5-10MB)
build/cli/gigamesh-clean   # CLI 도구
```

**체크리스트**:
- [ ] CMake 설정 성공 (Qt5 찾기)
- [ ] 컴파일 성공 (경고는 무시 가능)
- [ ] 실행 파일 생성 확인

**실패 시 대응**:
- Qt5 못 찾음 → `CMAKE_PREFIX_PATH` 설정
- C++20 오류 → `CMAKE_CXX_STANDARD=17`로 변경
- dongarch 컴파일 오류 → 일단 제외하고 빌드

---

#### Task A3: 기본 실행 확인 (30분)
```bash
cd build/gui
./gigamesh
```

**확인 사항**:
- [ ] 프로그램 실행됨
- [ ] 한글 UI 표시됨 (Settings → Language → 한국어)
- [ ] 3D 파일 열기 가능 (PLY, OBJ, STL)
- [ ] 기본 GigaMesh 기능 작동 (회전, 줌)

**실패 시**:
- Qt 라이브러리 못 찾음 → `LD_LIBRARY_PATH` 설정
- 한글 안 나옴 → 번역 파일 확인

---

### **Phase B: DongArch3D 신규 기능 연결** (2-3일)
**목표**: 메뉴에서 Cutline, Outline 등 새 기능 호출 가능

#### Task B1: dongarch 빌드 확인 (2시간)
```bash
# 빌드 시 dongarch 파일들이 컴파일되는지 확인
cd build
make -j$(nproc) 2>&1 | grep "dongarch"

# 예상 출력:
# [ 80%] Building CXX object gui/CMakeFiles/gui.dir/src/dongarch/cutline/DongArchCutlineManager.cpp.o
# [ 81%] Building CXX object gui/CMakeFiles/gui.dir/src/dongarch/cutline/algorithms/DouglasPeucker.cpp.o
```

**체크리스트**:
- [ ] dongarch/*.cpp 파일들이 컴파일됨
- [ ] 링크 오류 없음
- [ ] 실행 파일 크기 증가 (5MB → 10-15MB)

**실패 시**:
- 컴파일 오류 → 각 파일 수정 필요
- 링크 오류 → CMakeLists.txt 확인

---

#### Task B2: Cutline 기능 테스트 (3시간)
```bash
# 1. 프로그램 실행
./build/gui/gigamesh

# 2. 3D 파일 열기 (테스트 데이터)
# 파일 → 열기 → PLY/OBJ 파일 선택

# 3. Cutline 메뉴 클릭
# 측정 → Cutline (단면 라인) → Top Cutline (상면)

# 4. 예상 동작:
# - 대화상자 표시
# - 평면 높이 입력
# - 단면 라인 추출 및 3D 뷰에 표시
```

**체크리스트**:
- [ ] 메뉴가 보임
- [ ] 클릭 시 대화상자 열림
- [ ] 단면 라인이 계산됨 (콘솔 메시지 확인)
- [ ] 3D 뷰에 라인 표시됨

**실패 시 디버깅**:
```cpp
// QGMMainWindow.cpp의 onCutlineTop() 함수 확인
void QGMMainWindow::onCutlineTop() {
    std::cout << "[DEBUG] Cutline Top called" << std::endl;
    // ... 실제 구현
}
```

**수정 우선순위**:
1. 메뉴 안 보임 → QGMMainWindow.cpp 메뉴 생성 코드 확인
2. 대화상자 안 열림 → DongArchCutlineDialog 생성 확인
3. 계산 안 됨 → DongArchCutlineManager 로직 확인
4. 표시 안 됨 → MeshWidget OpenGL 렌더링 확인

---

#### Task B3: Outline 기능 테스트 (3시간)
```bash
# 측정 → Outline (외곽선) → 6방향 자동 추출
```

**체크리스트**:
- [ ] 메뉴가 보임
- [ ] Silhouette Detection 작동
- [ ] 6방향 외곽선 계산됨
- [ ] 3D 뷰에 표시됨

---

#### Task B4: 기타 기능 테스트 (2시간)
- [ ] Align (정렬): Rotation, ViewPoint
- [ ] Clip (절단): 평면으로 메시 자르기
- [ ] Vis (시각화): X-Ray, D-Tak
- [ ] MFE (파일 탐색): 파일 목록 표시
- [ ] Illustrator: SVG 내보내기

---

### **Phase C: 버그 수정 및 안정화** (1-2일)
**목표**: 모든 기능이 안정적으로 작동

#### Task C1: 크래시 수정 (가장 중요!)
**자주 발생하는 문제**:
1. nullptr 역참조
   ```cpp
   // 잘못된 코드
   mesh->doSomething();  // mesh가 nullptr일 수 있음

   // 올바른 코드
   if (mesh != nullptr) {
       mesh->doSomething();
   }
   ```

2. Qt 시그널/슬롯 연결 누락
   ```cpp
   // 연결 확인
   connect(button, &QPushButton::clicked,
           this, &Dialog::onButtonClicked);
   ```

3. OpenGL 컨텍스트 오류
   ```cpp
   // OpenGL 호출 전 컨텍스트 확인
   if (!isValid()) return;
   makeCurrent();
   // ... OpenGL 호출
   doneCurrent();
   ```

---

#### Task C2: 성능 최적화 (선택)
- [ ] Cutline 계산 속도 (1초 이내 목표)
- [ ] Outline 6방향 계산 (3초 이내 목표)
- [ ] 3D 렌더링 프레임레이트 (30 FPS 이상)

---

#### Task C3: 사용성 개선 (선택)
- [ ] 진행 표시줄 추가
- [ ] 에러 메시지 한글화
- [ ] 단축키 설정
- [ ] 마지막 설정 저장/복원

---

## 🚦 진행 기준

### ✅ Phase A 완료 조건
- GigaMesh가 실행되고 한글 UI 작동
- 3D 파일 열기 가능
- 기본 뷰 조작 (회전, 줌) 가능

### ✅ Phase B 완료 조건
- 메뉴에서 Cutline 선택 가능
- 단면 라인이 계산되고 표시됨
- Outline이 계산되고 표시됨

### ✅ Phase C 완료 조건
- 모든 기능이 크래시 없이 작동
- 에러 발생 시 적절한 메시지 표시
- 사용자가 실제 작업에 사용 가능

---

## 🔧 필수 도구

### 빌드 도구
```bash
sudo apt-get install -y \
    build-essential cmake ninja-build \
    gdb valgrind  # 디버깅용
```

### Qt 개발 도구
```bash
sudo apt-get install -y \
    qt5-default qtcreator \
    qttools5-dev-tools  # Qt Linguist, Designer
```

### 디버깅
```bash
# GDB로 크래시 원인 찾기
gdb ./build/gui/gigamesh
(gdb) run
# ... 크래시 발생
(gdb) backtrace  # 콜스택 확인
```

---

## 📊 예상 일정

| Phase | 작업 | 시간 | 누적 |
|-------|------|------|------|
| **A1** | 빌드 환경 구축 | 1시간 | 1시간 |
| **A2** | 기본 빌드 | 2시간 | 3시간 |
| **A3** | 기본 실행 확인 | 0.5시간 | 3.5시간 |
| **B1** | dongarch 빌드 | 2시간 | 5.5시간 |
| **B2** | Cutline 테스트 | 3시간 | 8.5시간 |
| **B3** | Outline 테스트 | 3시간 | 11.5시간 |
| **B4** | 기타 기능 테스트 | 2시간 | 13.5시간 |
| **C1** | 크래시 수정 | 4시간 | 17.5시간 |
| **C2** | 성능 최적화 | 2시간 | 19.5시간 |
| **C3** | 사용성 개선 | 2시간 | 21.5시간 |

**총 예상 시간**: 약 22시간 (3일)

---

## 🎯 최소 목표 (MVP)

**1일차**: Phase A 완료
- GigaMesh 실행 가능
- 한글 UI 작동
- 3D 파일 열기 가능

**2일차**: Phase B 완료
- Cutline 메뉴 작동
- 단면 라인 추출 및 표시
- Outline 추출 및 표시

**3일차**: Phase C 완료
- 버그 수정
- 안정화
- 사용자 테스트

---

## 📝 다음 단계

바로 시작하시려면:

```bash
# 1. 환경 확인
cd /workspaces/git11/GigaMesh
cmake --version
g++ --version
dpkg -l | grep qt5

# 2. Qt5 설치 (필요 시)
sudo apt-get update
sudo apt-get install -y qt5-default qtbase5-dev qttools5-dev libqt5opengl5-dev

# 3. 빌드 시작
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
make -j$(nproc)

# 4. 실행
./gui/gigamesh
```

---

**버전**: 1.0
**작성자**: Claude Code
**마지막 수정**: 2025-11-12
