# /dongarch-phase - Phase 정보 및 계획 확인 (v4.0)

**용도**: 특정 Phase의 목표, Task 목록을 상세히 확인합니다. (v4.0: 시간 제약 없음)

**버전**: v4.0 (품질 최우선)

## 사용법

```bash
# 현재 Phase 정보
/dongarch-phase

# 특정 Phase 정보
/dongarch-phase 2  # Phase 2 정보
```

## 실행 내용

1. **Phase 정보 읽기**:
   - `.dongarch3d-progress.json` 또는 계획서에서 Phase 정보 읽기
   - 해당 Phase의 목표, Task, 일정 표시

2. **Phase 상세 정보 표시**:
   ```
   ═══════════════════════════════════════════════════════
   📘 Phase 2: Cutline (단면 라인 추출) ⭐ 핵심
   ═══════════════════════════════════════════════════════
   
   🎯 목표:
      3D 메시와 평면의 교차점을 계산하여 2D 단면 라인을 추출하는 
      핵심 기능 구현. Arch3D Liner의 가장 중요한 기능.
   
   🎯 v4.0 원칙:
      - 시간 제약: 없음 (품질 최우선)
      - 완성도: 100% 목표
      - 참조: 115개 문서 + 50개 논문
   
   🛠️ GigaMesh 활용:
      ✅ mesh.cpp:4041 calcIntersectionPolylineWithPlane()
      ✅ plane.cpp Plane 클래스
      ✅ polyline.cpp PolyLine 클래스
      🟡 Douglas-Peucker (새로 구현 필요)
      🟡 Catmull-Rom Spline (새로 구현 필요)
   
   📋 Task 목록 (5개):
      
      ✅ Task 2.1: Mesh-Plane Intersection (2일)
         - GigaMesh calcIntersectionPolylineWithPlane() 활용
         - Top/Front/Right Cut 평면 정의
         - 3D 교차점 계산
         - 상태: 완료
      
      ✅ Task 2.2: 3D → 2D 투영 (1일)
         - 평면 로컬 좌표계 변환
         - 2D 라인 추출
         - 상태: 완료
      
      🔄 Task 2.3: Line Simplification (3일)
         - Douglas-Peucker 알고리즘 구현
         - Detail Level 파라미터 (0.1mm, 0.5mm, 1mm)
         - 단위 테스트
         - 상태: 진행 중
      
      ⏳ Task 2.4: Spline Curve Fitting (3일)
         - Catmull-Rom Spline 구현
         - Curve Level 파라미터 (Low/Mid/High)
         - Polyline vs Spline 전환
         - 상태: 대기 중
      
      ⏳ Task 2.5: Cutline UI (2일)
         - CutlineDialog 구현
         - 2D 미리보기 창
         - Save Slots (5개, JSON)
         - 상태: 대기 중
   
   📊 완성도 (품질 기준):
      Task:  2/5 (40% 완성)
      품질:  100% 목표 (시간 무관)
   
   📂 구현 위치:
      gui/src/arch3d/cutline/
      ├── DongArchCutlineManager.h/cpp      # 메인 관리자
      ├── CutlineDialog.h/cpp               # UI 다이얼로그
      ├── CutlinePreviewWidget.h/cpp        # 2D 미리보기
      ├── DouglasPeucker.h/cpp              # 라인 간략화 ⭐
      ├── CatmullRomSpline.h/cpp            # 곡선 피팅 ⭐
      └── CutlineSaveSlot.h/cpp             # Save Slots
   
   🎓 참고 자료:
      - ADR-010: Cutline - GigaMesh Mesh-Plane Intersection 활용
      - architecture.md: Section 2.1 Cutline 구조
      - GigaMesh mesh.cpp:4041 소스코드
      - Douglas-Peucker Wikipedia
      - Catmull-Rom Spline 알고리즘 문서
   
   ⚠️ 주의사항:
      - GigaMesh 함수는 수정 금지, 래핑만 가능
      - Detail/Curve Level UI 파라미터 필수
      - Save Slots JSON 포맷 정의 필요
   
   ═══════════════════════════════════════════════════════
   ```

3. **Phase 간 의존성 표시** (선택):
   ```
   🔗 의존성:
      Phase 1 (Align) ✅ → Phase 2 (Cutline) 🔄 → Phase 3 (Outline) ⏳
   
   ⚠️ Phase 2 완료 전 Phase 3 시작 불가
   ```

## 사용 시점

- Phase 시작 전 계획 확인
- Task 목록 파악 시
- Phase 진행 중 전체 상황 확인 시

## 지원 Phase

- `0`: 기반 시스템 (4일)
- `1`: Align (10일)
- `2`: Cutline (8일) ⭐
- `3`: Outline (8일) ⭐
- `4`: Clip (2일)
- `5`: Vis (7일)
- `6`: MFE (5일)
- `7`: Illustrator (5일)
- `8`: 테스트 (10일)
