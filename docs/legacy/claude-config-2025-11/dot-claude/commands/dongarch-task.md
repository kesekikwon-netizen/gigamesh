# /dongarch-task - Task 상세 정보 및 시작 (v4.0)

**용도**: 특정 Task의 상세 정보, 구현 가이드, 체크리스트를 제공합니다.

**버전**: v4.0 (품질 최우선, 시간 제약 없음)

## 사용법

```bash
# 현재 Task 정보
/dongarch-task

# 특정 Task 시작
/dongarch-task 205  # Task 205 시작
```

## 실행 내용

1. **Task 정보 읽기**:
   - `.dongarch3d-progress.json`에서 Task 정보 읽기
   - 계획서에서 Task 상세 정보 읽기

2. **Task 상세 정보 표시**:
   ```
   ═══════════════════════════════════════════════════════
   📝 Task 2.3: Line Simplification (Douglas-Peucker)
   ═══════════════════════════════════════════════════════
   
   📊 기본 정보:
      Phase:     2 (Cutline)
      Task ID:   203
      품질 목표: 100% (v4.0)
      상태:      진행 중
   
   🎯 목표:
      Douglas-Peucker 알고리즘을 구현하여 Cutline의 2D 라인을 
      간략화. Detail Level 파라미터로 사용자 조정 가능.
   
   📋 작업 내용:
      1. Douglas-Peucker 알고리즘 C++ 구현
         - 재귀 버전
         - 입력: std::vector<Vector2D> points, double tolerance
         - 출력: std::vector<Vector2D> simplified
      
      2. Detail Level UI 파라미터
         - QSlider: 0.1mm, 0.5mm, 1mm
         - 실시간 미리보기
      
      3. 단위 테스트
         - test_douglaspeucker.cpp
         - 5개 테스트 케이스
   
   📂 구현 위치:
      gui/src/arch3d/cutline/
      ├── DouglasPeucker.h      # 헤더 (클래스 정의)
      ├── DouglasPeucker.cpp    # 구현 (알고리즘)
      └── tests/
          └── test_douglaspeucker.cpp  # 단위 테스트
   
   💻 코드 스켈레톤:
      ```cpp
      // DouglasPeucker.h
      #ifndef DOUGLASPEUCKER_H
      #define DOUGLASPEUCKER_H
      
      #include <vector>
      #include "Vector2D.h"
      
      class DouglasPeucker {
      public:
          std::vector<Vector2D> simplify(
              const std::vector<Vector2D>& points,
              double tolerance
          );
      
      private:
          void simplifyRecursive(
              const std::vector<Vector2D>& points,
              int start, int end,
              double tolerance,
              std::vector<bool>& keep
          );
          
          double perpendicularDistance(
              const Vector2D& point,
              const Vector2D& lineStart,
              const Vector2D& lineEnd
          );
      };
      
      #endif
      ```
   
   🎓 참고 자료:
      - Douglas-Peucker Wikipedia: 
        https://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm
      - C++ 구현 예제:
        https://github.com/mourner/simplify-cpp
      - Arch3D Liner 매뉴얼: Detail Level 설명 (p.25)
   
   ✅ 체크리스트:
      [ ] DouglasPeucker.h 헤더 작성
      [ ] DouglasPeucker.cpp 알고리즘 구현
      [ ] perpendicularDistance() 함수 구현
      [ ] simplifyRecursive() 함수 구현
      [ ] test_douglaspeucker.cpp 단위 테스트 작성
      [ ] CMakeLists.txt에 파일 추가
      [ ] 빌드 성공 확인
      [ ] 5개 테스트 케이스 통과
      [ ] CutlineDialog UI 연동 (Detail Level Slider)
      [ ] 실시간 미리보기 동작 확인
   
   ⚠️ 주의사항:
      - tolerance 파라미터는 mm 단위 (0.1, 0.5, 1)
      - 재귀 깊이 제한 (max 1000)
      - 빈 배열 입력 처리
      - 2점 이하 입력은 그대로 반환
   
   🚀 시작하기:
      1. DouglasPeucker.h 파일 생성
      2. 위 코드 스켈레톤 복사
      3. 알고리즘 구현 시작
   
   ═══════════════════════════════════════════════════════
   ```

3. **Task 시작 시 진행 상황 업데이트 가이드**:
   ```json
   {
     "current": {
       "phase": 2,
       "task_id": "203",
       "status": "in_progress"
     },
     "phases": {
       "2": {
         "tasks": {
           "203": {
             "status": "in_progress",
             "started": "2025-11-08T15:00:00Z"
           }
         }
       }
     }
   }
   ```

4. **다음 단계 안내**:
   ```
   📝 Task 완료 후:
      1. 빌드 성공 확인
      2. 단위 테스트 통과
      3. /dongarch-complete 실행
      4. 구현 보고서 작성
      5. Git 커밋
   ```

## 사용 시점

- 새 Task 시작 전
- Task 구현 가이드 확인 시
- 코드 스켈레톤 필요 시

## 참고

- Task ID는 Phase 번호 + 순번 (예: 203 = Phase 2, Task 3)
- 체크리스트는 복사하여 구현 진행 상황 추적 가능
- 코드 스켈레톤은 바로 사용 가능한 템플릿
