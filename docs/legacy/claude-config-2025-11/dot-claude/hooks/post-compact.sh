#!/bin/bash
# Post-Compact Hook
# compact 직후에 최신 스냅샷을 PROJECT_CONTEXT.json으로 복원

CONTEXT_FILE="/media/kwon/새 볼륨/1105/.claude/PROJECT_CONTEXT.json"
SNAPSHOT_DIR="/media/kwon/새 볼륨/1105/.claude/snapshots"

echo "[Post-Compact Hook] Restoring context from latest snapshot..."

# 최신 스냅샷 찾기
LATEST_SNAPSHOT=$(ls -t "$SNAPSHOT_DIR"/context_*.json 2>/dev/null | head -1)

if [ -f "$LATEST_SNAPSHOT" ]; then
    # 스냅샷에서 snapshotInfo 제거하고 복원
    if command -v jq &> /dev/null; then
        jq 'del(.snapshotInfo)' "$LATEST_SNAPSHOT" > "$CONTEXT_FILE"
        echo "[Post-Compact Hook] ✅ Context restored from: $(basename $LATEST_SNAPSHOT)"
    else
        cp "$LATEST_SNAPSHOT" "$CONTEXT_FILE"
        echo "[Post-Compact Hook] ✅ Context restored (basic copy)"
    fi

    # 복원된 정보 출력
    if command -v jq &> /dev/null; then
        FEATURE=$(jq -r '.currentWork.feature' "$CONTEXT_FILE")
        STAGE=$(jq -r '.currentWork.stage' "$CONTEXT_FILE")
        COMMIT=$(jq -r '.currentWork.lastCommit' "$CONTEXT_FILE")

        echo "[Post-Compact Hook] Feature: $FEATURE"
        echo "[Post-Compact Hook] Stage: $STAGE"
        echo "[Post-Compact Hook] Commit: $COMMIT"
    fi
else
    echo "[Post-Compact Hook] ⚠️  No snapshot found, context not restored"
fi

exit 0
