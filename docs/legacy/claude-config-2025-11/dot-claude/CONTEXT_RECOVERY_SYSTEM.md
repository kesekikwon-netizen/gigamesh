# Context Recovery System - 완전 자동화

재접속, `/clear`, `/compact` 후에도 **완벽한 컨텍스트 복원**을 제공하는 **완전 자동화 시스템**

## ✅ 자동 실행 확인

이 시스템은 **Claude Code가 자동으로 실행**합니다. 별도 설정 불필요!

### 1. 매 메시지마다 자동 업데이트 ✅
```
User: [메시지]
  ↓
UserPromptSubmit Hook 자동 실행
  ↓
context-save.sh 자동 실행 (백그라운드)
  ↓
PROJECT_CONTEXT.json 업데이트
```

### 2. Autocompact 시 자동 저장/복원 ✅
```
메시지 길이 초과 (autocompact 트리거)
  ↓
PreCompact Hook 자동 실행
  ↓
pre-compact.sh:
  - PROJECT_CONTEXT.json → 스냅샷 저장
  - Git 정보 기록
  - .claude/snapshots/ 저장 (최신 3개 유지)
  ↓
Compact 실행
  ↓
SessionStart (matcher: "compact") Hook 자동 실행
  ↓
post-compact.sh:
  - 최신 스냅샷 → PROJECT_CONTEXT.json 복원
  ↓
✅ 컨텍스트 손실 0%
```

### 3. /compact 명령어 시 자동 저장/복원 ✅
```
User: /compact
  ↓
PreCompact Hook 자동 실행 (matcher: "manual")
  ↓
pre-compact.sh 실행
  ↓
Compact 실행
  ↓
SessionStart Hook 자동 실행 (matcher: "compact")
  ↓
post-compact.sh 실행
  ↓
✅ 완전 자동 복원
```

## 📝 자동 실행 설정 (.claude/settings.json)

```json
{
  "hooks": {
    "PreCompact": [
      {
        "matcher": "auto|manual",
        "hooks": [
          {
            "type": "command",
            "command": "\"$CLAUDE_PROJECT_DIR\"/.claude/hooks/pre-compact.sh",
            "description": "Save context snapshot before compact"
          }
        ]
      }
    ],
    "SessionStart": [
      {
        "matcher": "compact",
        "hooks": [
          {
            "type": "command",
            "command": "\"$CLAUDE_PROJECT_DIR\"/.claude/hooks/post-compact.sh",
            "description": "Restore context after compact"
          }
        ]
      }
    ],
    "UserPromptSubmit": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "\"$CLAUDE_PROJECT_DIR\"/.claude/hooks/context-save.sh",
            "description": "Update context on every user message"
          }
        ]
      }
    ]
  }
}
```

## 🎯 Hook 이벤트 설명

| Hook | Matcher | 실행 시점 | 목적 |
|------|---------|----------|------|
| **PreCompact** | `auto\|manual` | Compact 전 | 스냅샷 저장 |
| **SessionStart** | `compact` | Compact 후 | 스냅샷 복원 |
| **UserPromptSubmit** | (없음) | 매 메시지 | 컨텍스트 업데이트 |

### PreCompact Hook
- **Matcher**: `auto` (autocompact) 또는 `manual` (`/compact`)
- **실행**: Compact 직전
- **역할**: 현재 상태를 스냅샷으로 저장

### SessionStart Hook (matcher: "compact")
- **Matcher**: `compact` (Compact 완료 후 세션 시작)
- **실행**: Compact 직후
- **역할**: 스냅샷에서 컨텍스트 복원

### UserPromptSubmit Hook
- **Matcher**: 없음 (모든 메시지)
- **실행**: 매 사용자 메시지
- **역할**: 실시간 컨텍스트 업데이트

## 📁 파일 구조

```
.claude/
├── PROJECT_CONTEXT.json           ← 마스터 파일 (자동 업데이트)
├── MCP_USAGE_RULES.md
├── CONTEXT_RECOVERY_SYSTEM.md
├── settings.json                  ← Hook 자동 실행 설정
├── hooks/
│   ├── context-save.sh            ← 매 메시지마다 자동 실행
│   ├── pre-compact.sh             ← Compact 전 자동 실행
│   └── post-compact.sh            ← Compact 후 자동 실행
├── snapshots/
│   └── context_*.json             ← 자동 스냅샷 (최신 3개)
├── commands/
│   └── restore-context.md         ← /restore-context 명령어
└── skills/
    ├── project-context-manager/
    └── auto-context-loader/
```

## 🚀 실제 동작 예시

### ✅ 일반 작업 중 (자동 업데이트)
```
User: Stage 5 진행
  → context-save.sh 자동 실행 (백그라운드)
  → PROJECT_CONTEXT.json 업데이트
  → 사용자는 아무것도 모름 (투명)
