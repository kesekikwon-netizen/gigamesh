#!/bin/bash
# Pre-Compact Hook
# autocompact 직전에 현재 컨텍스트를 자동 저장

CONTEXT_FILE="/media/kwon/새 볼륨/1105/.claude/PROJECT_CONTEXT.json"
SNAPSHOT_DIR="/media/kwon/새 볼륨/1105/.claude/snapshots"
GIT_DIR="/media/kwon/새 볼륨/1105/GigaMesh"

# 스냅샷 디렉토리 생성
mkdir -p "$SNAPSHOT_DIR"

# 타임스탬프 생성
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
SNAPSHOT_FILE="$SNAPSHOT_DIR/context_${TIMESTAMP}.json"

echo "[Pre-Compact Hook] Saving context snapshot before compact..."

# Git 정보 수집
cd "$GIT_DIR"
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
LAST_COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
LAST_COMMIT_MSG=$(git log -1 --pretty=%B 2>/dev/null | head -1 || echo "unknown")

# Git status 수집
GIT_STATUS=$(git status --short 2>/dev/null | head -20 || echo "unknown")

# 현재 PROJECT_CONTEXT.json 복사
if [ -f "$CONTEXT_FILE" ]; then
    cp "$CONTEXT_FILE" "$SNAPSHOT_FILE"

    # jq로 추가 정보 병합
    if command -v jq &> /dev/null; then
        jq --arg timestamp "$(date -u +"%Y-%m-%dT%H:%M:%SZ")" \
           --arg branch "$CURRENT_BRANCH" \
           --arg commit "$LAST_COMMIT" \
           --arg msg "$LAST_COMMIT_MSG" \
           --arg status "$GIT_STATUS" \
           '.snapshotInfo = {
              "timestamp": $timestamp,
              "reason": "pre-compact",
              "git": {
                "branch": $branch,
                "commit": $commit,
                "message": $msg,
                "status": $status
              }
            }' \
           "$SNAPSHOT_FILE" > "$SNAPSHOT_FILE.tmp" && mv "$SNAPSHOT_FILE.tmp" "$SNAPSHOT_FILE"

        echo "[Pre-Compact Hook] ✅ Snapshot saved: $SNAPSHOT_FILE"
        echo "[Pre-Compact Hook] Branch: $CURRENT_BRANCH"
        echo "[Pre-Compact Hook] Commit: $LAST_COMMIT"
    else
        echo "[Pre-Compact Hook] ⚠️  jq not installed, basic snapshot only"
    fi
else
    echo "[Pre-Compact Hook] ⚠️  Context file not found: $CONTEXT_FILE"
fi

# 최신 3개만 유지 (디스크 절약)
cd "$SNAPSHOT_DIR"
ls -t context_*.json | tail -n +4 | xargs -r rm
echo "[Pre-Compact Hook] Kept latest 3 snapshots"

exit 0
