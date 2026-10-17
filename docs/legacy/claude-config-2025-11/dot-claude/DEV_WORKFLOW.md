# DongArch3D Development Workflow

**작성일:** 2025-11-12
**목적:** 체계적인 개발 진행 및 기억 유지

---

## 🎯 개발 목표

**최종 목표:** Arch3D Liner 수준의 고고학 3D 측정 도구 완성

**주요 문서:**
- 실행 계획: `DongArch3D_Task별_실행계획.md` (15개 Task)
- 완성 비전: `docs/dongarch3d/VISION_완료시_모습.md`
- 참고 매뉴얼: `docs/archaeology/Arch3D Liner User Guide ver.2023.07.01.01.pdf`

---

## 📋 Task 구조

```
Phase A: 빌드 환경 (4 tasks)      ← 현재 위치
├─ A1: Qt5 설치 확인
├─ A2: 기본 빌드 (dongarch 제외)
├─ A3: 실행 파일 테스트
└─ A4: 한글 UI 확인

Phase B: DongArch 통합 (5 tasks)
├─ B1: dongarch 빌드 확인
├─ B2: Cutline 메뉴 연결
├─ B3: Cutline 동작 테스트 ⭐ (Arch3D Liner 기준)
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

## 🔄 작업 사이클

각 Task는 다음 순서로 진행:

### 1️⃣ Task 시작

```bash
# 현재 상태 확인
./.claude/scripts/dev-progress.sh status

# 다음 Task 확인
./.claude/scripts/dev-progress.sh next
```

### 2️⃣ Task 실행

```bash
# 계획 문서에서 명령어 복사
# DongArch3D_Task별_실행계획.md의 "실행 명령어" 섹션 참고

# 실행하면서 관찰사항 기록
./.claude/scripts/dev-progress.sh note "Qt5 설치 확인 중..."
```

### 3️⃣ 완료 확인

```bash
# 완료 조건 체크 (계획 문서의 "완료 조건" 참고)
# 예: .task_a1_done 파일 생성 확인

# 완료 표시
./.claude/scripts/dev-progress.sh complete A1
```

### 4️⃣ 문제 발생 시

```bash
# 문제 기록
./.claude/scripts/dev-progress.sh note "오류: Qt5 못 찾음. 해결: CMAKE_PREFIX_PATH 설정"

# 상세 내역은 세션 로그에 자동 기록됨
```

---

## 📝 기록 시스템

### 자동 기록

**`.dongarch3d-dev-progress.json`**
- 모든 Task 상태
- 완료 시간
- 노트 및 관찰사항
- 진행 통계

### 수동 기록 (Claude가 유지)

**기억해야 할 것:**
1. **중요한 결정사항** → `memory.recentDecisions`에 기록
2. **막힌 부분** → `memory.blockers`에 기록
3. **새로 발견한 것** → `memory.discoveries`에 기록

**예시:**
```json
{
  "memory": {
    "recentDecisions": [
      "2025-11-12: C++20 표준 사용하기로 결정 (람다, concepts)"
    ],
    "blockers": [
      "Task B3: Arch3D Liner PDF 32페이지 UI 구현 필요"
    ],
    "discoveries": [
      "GigaMesh calcIntersectionPolylineWithPlane() API 확인"
    ]
  }
}
```

---

## ⚡ 빠른 명령어

```bash
# 상태 확인
alias devstatus='./.claude/scripts/dev-progress.sh status'

# 다음 Task
alias devnext='./.claude/scripts/dev-progress.sh next'

# Task 완료
alias devcomplete='./.claude/scripts/dev-progress.sh complete'

# 노트 추가
alias devnote='./.claude/scripts/dev-progress.sh note'
```

---

## 🎯 핵심 원칙

### 1. 한 번에 하나의 Task만

- Task 순서 엄수 (의존성 때문)
- A1 → A2 → A3 → ... 순차 진행
- 건너뛰지 않기

### 2. 완료 조건 명확히

- `.task_XX_done` 파일 존재 확인
- 계획 문서의 체크리스트 전부 확인
- 확신이 없으면 완료 표시 안 함

### 3. 모든 것을 기록

- 성공한 것
- 실패한 것
- 배운 것
- 다음에 해야 할 것

### 4. 문제 발생 시

- 즉시 기록
- 해결 방법도 기록
- 다음 세션에서 참고 가능하도록

---

## 📊 진행 상황 확인

### Task별 완료 파일

```bash
ls -lh .task_*_done

# 출력 예시:
# .task_a1_done
# .task_a2_done
# .task_a3_done
# ...
```

### JSON 파일 직접 확인

```bash
cat .dongarch3d-dev-progress.json | jq '.statistics'

# 출력:
# {
#   "totalTasks": 15,
#   "completedTasks": 3,
#   "remainingTasks": 12,
#   "estimatedTotalTime": "4-5 hours",
#   "actualTimeSpent": "45 minutes"
# }
```

---

## 🔍 Task B3 주의사항 (중요!)

**Task B3: Cutline 동작 테스트**는 가장 복잡한 Task입니다.

**필수 확인사항 (Arch3D Liner PDF 페이지 32-34 기준):**

1. **Split View 구조**
   - 왼쪽: 3D 뷰 (모델 + 절단 평면 + 하이라이트)
   - 오른쪽: 2D 미리보기 (추출된 라인만)

2. **필수 UI 요소**
   - Rotation Scroll Bar (-180° ~ 180°)
   - Translation Scroll Bar (0.1% 단위)
   - Rotation Angle 버튼 (-90°, -5°, -1°, +1°, +5°, +90°)
   - Save Slot 5개 (Save0 ~ Save4)
   - Line Property (Scale/Detail/Curve Lv)
   - Line Style (Width/Transparency/Color)

3. **핵심 알고리즘**
   - Douglas-Peucker 간략화
   - Catmull-Rom Spline 스무딩
   - GigaMesh `calcIntersectionPolylineWithPlane()` 사용

**Task B3에서 막히면:**
- PDF 페이지 32-34 다시 확인
- `docs/dongarch3d/VISION_완료시_모습.md` 참고
- `gui/src/dongarch/cutline/` 소스 코드 검토

---

## 🚀 시작하기

```bash
# 1. 현재 상태 확인
./.claude/scripts/dev-progress.sh status

# 2. 다음 Task 확인
./.claude/scripts/dev-progress.sh next

# 3. Task 계획 문서 열기
cat DongArch3D_Task별_실행계획.md | grep -A 50 "Task A1"

# 4. 실행!
```

---

**Let's build DongArch3D! 🎉**
