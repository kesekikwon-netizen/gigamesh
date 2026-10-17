#!/bin/bash
# DongArch3D SessionStart Hook
# 간단한 환영 메시지 + 핵심 정보만 표시 (100자 이내)

# 프로젝트 루트
PROJECT_ROOT="A:/1105"

# Progress 파일에서 핵심 정보 추출
PHASE=$(grep '"phase"' "$PROJECT_ROOT/.dongarch3d-progress.json" | head -1 | grep -o '[0-9]*')
PHASE_NAME=$(grep '"phase_name"' "$PROJECT_ROOT/.dongarch3d-progress.json" | head -1 | sed 's/.*": "\(.*\)".*/\1/')
COMPLETION=$(grep '"completion_percentage"' "$PROJECT_ROOT/.dongarch3d-progress.json" | grep -o '[0-9.]*' | head -1)

# 간단한 환영 메시지 (100자 이내)
echo "🚀 DongArch3D | Phase $PHASE: $PHASE_NAME | $COMPLETION% 완료"

# 상세 로그는 파일에 기록 (화면에 출력 안됨)
{
  echo "=== Session Start: $(date '+%Y-%m-%d %H:%M:%S') ==="
  echo ""
  echo "=== Current Status ==="
  cat "$PROJECT_ROOT/.dongarch3d-progress.json" | grep -E '(current|phase_name|task_id|completion_percentage)' | head -10
  echo ""
  echo "=== Recent Commits ==="
  git -C "$PROJECT_ROOT" log -5 --oneline
  echo ""
  echo "=== Git Status ==="
  git -C "$PROJECT_ROOT" status --short
  echo ""
} >> "$PROJECT_ROOT/.claude/hooks/session-start.log" 2>&1

# 로그 파일 크기 제한 (최대 100KB)
LOG_FILE="$PROJECT_ROOT/.claude/hooks/session-start.log"
if [ -f "$LOG_FILE" ]; then
  LOG_SIZE=$(stat -c%s "$LOG_FILE" 2>/dev/null || stat -f%z "$LOG_FILE" 2>/dev/null || echo 0)
  if [ "$LOG_SIZE" -gt 102400 ]; then
    # 마지막 50줄만 유지
    tail -50 "$LOG_FILE" > "$LOG_FILE.tmp"
    mv "$LOG_FILE.tmp" "$LOG_FILE"
  fi
fi

exit 0
