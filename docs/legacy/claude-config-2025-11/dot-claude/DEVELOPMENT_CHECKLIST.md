# DongArch3D 개발 체크리스트

**빠른 참조용** - 매 세션마다 이 파일을 확인하세요!

---

## ✅ 세션 시작 체크리스트

```bash
□ /restore-context 실행
□ ./.claude/scripts/dev-progress.sh status
□ ./.claude/scripts/dev-progress.sh next
□ git status
□ git log --oneline -5
```

---

## ✅ Task 작업 체크리스트

### 시작 전
```bash
□ 계획 문서 확인: cat DongArch3D_실행_세부계획.md | grep -A 30 "Task XX"
□ 이전 Task 완료 확인: ls -l .task_*_done
□ Git 상태 깨끗한지 확인: git status
```

### 작업 중
```bash
□ 빌드 로그 저장: make 2>&1 | tee build.log
□ 중요 사항 노트 기록: dev-progress.sh note "..."
□ 오류 발생 시 즉시 기록
```

### 완료 시
```bash
□ 완료 조건 전부 확인 (계획 문서)
□ 체크포인트 파일 존재 확인: ls -l .task_XX_done
□ Task 완료 표시: dev-progress.sh complete XX
□ Git 커밋 (한국어 메시지)
```

---

## ❌ 절대 금지 사항

```
□ Task 건너뛰기 - NEVER!
□ 체크포인트 없이 완료 표시 - NEVER!
□ GigaMesh/core/ 수정 - NEVER!
□ 빌드 로그 없이 진행 - NEVER!
□ 영어로 커밋 메시지 - NEVER!
```

---

## 🎯 5대 핵심 원칙

1. **항상 순서대로** - A1 → A2 → A3 → ... → C6
2. **항상 기록하기** - 성공/실패/배운 것 모두
3. **항상 검증하기** - 완료 조건 전부 확인
4. **항상 커밋하기** - Task 단위로 의미 있게
5. **항상 한국어로** - 문서, 노트, 커밋 메시지

---

## 🚨 긴급 상황 대응

### 빌드 실패
```bash
1. tail -100 GigaMesh/build/build.log
2. dev-progress.sh note "빌드 실패: [원인]"
3. 해결 후: dev-progress.sh note "해결: [방법]"
```

### Task 완료 확신 없을 때
```bash
1. 계획 문서 완료 조건 재확인
2. 체크포인트 파일 확인
3. 확신 없으면 완료 표시 하지 않기!
```

### 잘못 완료 표시했을 때
```bash
1. rm .task_XX_done  # 체크포인트 삭제
2. .dongarch3d-dev-progress.json 수동 수정
3. 다시 Task 진행
```

---

## 📊 진행 상황 확인 명령어

```bash
# 빠른 상태 확인
./.claude/scripts/dev-progress.sh status

# 다음 할 일
./.claude/scripts/dev-progress.sh next

# 완료된 Task 목록
ls -l .task_*_done

# 전체 히스토리
cat .dongarch3d-dev-progress.json | jq '.phases'
```

---

## 🔗 주요 문서 위치

- 전체 가이드: `CLAUDE.md`
- 실행 계획: `DongArch3D_실행_세부계획.md`
- 워크플로우: `.claude/DEV_WORKFLOW.md`
- 진행 상황: `.dongarch3d-dev-progress.json`

---

**이 체크리스트를 항상 열어두고 하나씩 체크하면서 작업하세요!**
