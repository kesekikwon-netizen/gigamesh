# /dongarch-complete - Task 완료 기록 (v4.0)

**용도**: 현재 Task 완료 후 진행 상황 업데이트 및 구현 보고서 작성을 가이드합니다.

**버전**: v4.0 (품질 최우선, 시간 제약 없음)

## 실행 내용

1. **현재 Task 확인**:
   - `A:\1105\.dongarch3d-progress.json` 읽기
   - 현재 진행 중인 Task 정보 표시

2. **완료 확인 체크리스트**:
   ```
   ✅ Task XXX 완료 체크리스트:
   
   1. 코드 작성
      - [ ] 핵심 기능 구현
      - [ ] 에러 처리
      - [ ] 코드 주석 (Doxygen)
   
   2. 빌드 확인
      - [ ] Release 모드 빌드 성공
      - [ ] 실행 파일 크기 확인
      - [ ] 실행 테스트
   
   3. 문서화
      - [ ] 구현 보고서 작성 (docs/dongarch3d/implementation_reports/)
      - [ ] CLAUDE.md 업데이트 (필요 시)
      - [ ] architecture.md 업데이트 (구조 변경 시)
   
   4. Git 커밋
      - [ ] 변경 파일 확인 (git status)
      - [ ] 커밋 메시지 작성
      - [ ] 커밋 실행
   
   5. 진행 상황 업데이트
      - [ ] .dongarch3d-progress.json 업데이트
         - Task status: "in_progress" → "completed"
         - completed 시각 기록
         - 다음 Task로 이동 (해당 시)
   ```

3. **구현 보고서 템플릿 제공**:
   ```markdown
   # Task XXX: (Task 이름) 구현 보고서
   
   **Phase**: X (Phase 이름)
   **Task ID**: XXX
   **시작**: YYYY-MM-DD
   **완료**: YYYY-MM-DD
   **품질 완성도**: 100% (v4.0 기준)
   
   ## 구현 내용
   
   ### 주요 기능
   - (기능 1 설명)
   - (기능 2 설명)
   
   ### 구현 세부사항
   - **파일 추가**: (파일명 목록)
   - **파일 수정**: (파일명 목록)
   - **핵심 클래스/함수**: (클래스명, 함수명)
   
   ### 기술적 결정
   - (ADR 링크 또는 간단한 설명)
   
   ## 빌드 결과
   
   - **빌드 성공**: ✅
   - **실행 파일**: DongArch3D.exe (XXmb)
   - **빌드 시간**: Xm Ys
   
   ## 테스트 결과
   
   - **단위 테스트**: X개 통과
   - **수동 테스트**: 정상 동작 확인
   
   ## 문제 및 해결
   
   ### 문제 1: (문제 설명)
   - **해결**: (해결 방법)
   
   ## 다음 단계
   
   - Task XXX: (다음 Task 이름)
   ```

4. **진행 상황 JSON 업데이트 가이드**:
   ```json
   {
     "current": {
       "phase": X,
       "task_id": "XXX",
       "status": "completed"  // "in_progress" → "completed"
     },
     "phases": {
       "X": {
         "tasks": {
           "XXX": {
             "status": "completed",
             "completed": "2025-11-08T14:30:00Z",
             "notes": [
               "실제 소요 시간: X시간 (예상 Y시간)"
             ]
           }
         }
       }
     },
     "statistics": {
       "tasks_completed": XX,  // +1
       "completion_percentage": XX.X  // 재계산
     }
   }
   ```

5. **다음 Task 시작 가이드**:
   ```
   🎯 다음 Task: XXX (Task 이름)
   
   📋 작업 내용:
   - (작업 1)
   - (작업 2)
   
   📂 구현 위치:
   - (파일 경로)
   
   ⏱️ 예상 시간: X시간
   
   🚀 바로 시작하려면 다음 명령어를 실행하세요:
   /dongarch-task XXX
   ```

## 사용 시점

- Task 완료 직후
- 빌드 성공 후
- Git 커밋 전

## 사용 예시

```bash
# 현재 Task 완료
/dongarch-complete

# 특정 Task 완료 (선택)
/dongarch-complete 205
```

## 참고

- 이 커맨드는 자동으로 파일을 수정하지 않습니다.
- 체크리스트와 가이드만 제공하며, 실제 작업은 사용자가 수행합니다.
- 구현 보고서는 `docs/dongarch3d/implementation_reports/` 폴더에 저장합니다.
