#!/bin/bash
# Context Auto-Save Hook
# Automatically saves project context on every significant operation

CONTEXT_FILE="/media/kwon/새 볼륨/1105/.claude/PROJECT_CONTEXT.json"
GIT_DIR="/media/kwon/새 볼륨/1105/GigaMesh"

# Update timestamp
update_context() {
    local timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

    # Get current git info
    cd "$GIT_DIR"
    local current_branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
    local last_commit=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
    local last_commit_msg=$(git log -1 --pretty=%B 2>/dev/null | head -1 || echo "unknown")

    # Update context file with jq
    if command -v jq &> /dev/null; then
        jq --arg timestamp "$timestamp" \
           --arg branch "$current_branch" \
           --arg commit "$last_commit" \
           --arg msg "$last_commit_msg" \
           '.lastUpdated = $timestamp |
            .project.currentBranch = $branch |
            .currentWork.lastCommit = $commit |
            .currentWork.lastCommitMessage = $msg' \
           "$CONTEXT_FILE" > "$CONTEXT_FILE.tmp" && mv "$CONTEXT_FILE.tmp" "$CONTEXT_FILE"

        echo "[Context Hook] Updated context: $last_commit - $(echo $last_commit_msg | cut -c1-50)..."
    else
        echo "[Context Hook] Warning: jq not installed, context not updated"
    fi
}

# Execute update
update_context

exit 0
