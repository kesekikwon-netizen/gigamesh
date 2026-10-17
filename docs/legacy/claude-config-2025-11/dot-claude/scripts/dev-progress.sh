#!/bin/bash

# DongArch3D Development Progress Helper
# 개발 진행 상황 추적 및 기록

PROGRESS_FILE="/media/kwon/새 볼륨/1105/.dongarch3d-dev-progress.json"
BASE_DIR="/media/kwon/새 볼륨/1105"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

#######################################
# Show current progress
#######################################
show_progress() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  DongArch3D Development Progress${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""

    # Count completed tasks
    local completed_a=$(ls "$BASE_DIR"/.task_a*_done 2>/dev/null | wc -l)
    local completed_b=$(ls "$BASE_DIR"/.task_b*_done 2>/dev/null | wc -l)
    local completed_c=$(ls "$BASE_DIR"/.task_c*_done 2>/dev/null | wc -l)
    local total_completed=$((completed_a + completed_b + completed_c))

    echo -e "📊 전체 진행: ${GREEN}${total_completed}/15${NC} 완료"
    echo ""

    echo -e "Phase A (빌드 환경): ${GREEN}${completed_a}/4${NC}"
    echo -e "Phase B (DongArch 통합): ${GREEN}${completed_b}/5${NC}"
    echo -e "Phase C (안정화): ${GREEN}${completed_c}/6${NC}"
    echo ""

    # Show last completed task
    local last_task=$(ls "$BASE_DIR"/.task_*_done 2>/dev/null | tail -1)
    if [ -n "$last_task" ]; then
        local task_name=$(basename "$last_task" | sed 's/\.task_//' | sed 's/_done//' | tr 'a-z' 'A-Z')
        echo -e "✅ 마지막 완료: ${GREEN}Task $task_name${NC}"
    else
        echo -e "⏸️  아직 시작 안 함"
    fi

    echo ""
}

#######################################
# Get next task
#######################################
next_task() {
    local last_task=$(ls "$BASE_DIR"/.task_*_done 2>/dev/null | tail -1 | sed 's/.*\.task_//' | sed 's/_done//')

    case "$last_task" in
        a1) echo "A2" ;;
        a2) echo "A3" ;;
        a3) echo "A4" ;;
        a4) echo "B1" ;;
        b1) echo "B2" ;;
        b2) echo "B3" ;;
        b3) echo "B4" ;;
        b4) echo "B5" ;;
        b5) echo "C1" ;;
        c1) echo "C2" ;;
        c2) echo "C3" ;;
        c3) echo "C4" ;;
        c4) echo "C5" ;;
        c5) echo "C6" ;;
        c6) echo "DONE" ;;
        *) echo "A1" ;;
    esac
}

#######################################
# Show next task details
#######################################
show_next() {
    local next=$(next_task)

    if [ "$next" = "DONE" ]; then
        echo -e "${GREEN}🎉 모든 Task 완료!${NC}"
        return
    fi

    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}  다음 Task: $next${NC}"
    echo -e "${YELLOW}========================================${NC}"
    echo ""

    # Extract task info from plan
    grep -A 20 "## 🚀 Task $next:" "$BASE_DIR/DongArch3D_Task별_실행계획.md" | head -25
}

#######################################
# Mark task as complete
#######################################
complete_task() {
    local task=$1
    if [ -z "$task" ]; then
        echo -e "${RED}❌ Usage: $0 complete <task>${NC}"
        echo -e "   Example: $0 complete A1"
        return 1
    fi

    local task_lower=$(echo "$task" | tr 'A-Z' 'a-z')
    local checkpoint_file="$BASE_DIR/.task_${task_lower}_done"

    echo "Task $task Complete - $(date)" > "$checkpoint_file"
    echo -e "${GREEN}✅ Task $task 완료 표시됨${NC}"

    # Update JSON
    python3 <<EOF
import json
from datetime import datetime

with open('$PROGRESS_FILE', 'r') as f:
    data = json.load(f)

task_id = '$task'
phase = task_id[0].upper()
phase_key = 'phase' + phase

if phase_key in data['phases']:
    if task_id in data['phases'][phase_key]['tasks']:
        data['phases'][phase_key]['tasks'][task_id]['status'] = 'completed'
        data['phases'][phase_key]['tasks'][task_id]['completed'] = True
        data['phases'][phase_key]['tasks'][task_id]['completedAt'] = datetime.now().isoformat()
        data['phases'][phase_key]['completed'] += 1
        data['statistics']['completedTasks'] += 1
        data['statistics']['remainingTasks'] -= 1
        data['lastUpdated'] = datetime.now().isoformat()

with open('$PROGRESS_FILE', 'w') as f:
    json.dump(data, f, indent=2, ensure_ascii=False)

print('📝 Progress file updated')
EOF

    # Show next
    echo ""
    show_next
}

#######################################
# Add note to current task
#######################################
add_note() {
    local note="$1"
    if [ -z "$note" ]; then
        echo -e "${RED}❌ Usage: $0 note \"your note here\"${NC}"
        return 1
    fi

    local current=$(next_task)
    if [ "$current" = "DONE" ]; then
        echo -e "${YELLOW}⚠️  All tasks completed${NC}"
        return
    fi

    python3 <<EOF
import json
from datetime import datetime

with open('$PROGRESS_FILE', 'r') as f:
    data = json.load(f)

task_id = '$current'
phase = task_id[0].upper()
phase_key = 'phase' + phase

if phase_key in data['phases']:
    if task_id in data['phases'][phase_key]['tasks']:
        note_entry = {
            'timestamp': datetime.now().isoformat(),
            'note': '''$note'''
        }
        data['phases'][phase_key]['tasks'][task_id]['notes'].append(note_entry)
        data['lastUpdated'] = datetime.now().isoformat()

with open('$PROGRESS_FILE', 'w') as f:
    json.dump(data, f, indent=2, ensure_ascii=False)

print('📝 Note added to Task $current')
EOF
}

#######################################
# Show help
#######################################
show_help() {
    echo "DongArch3D Development Progress Helper"
    echo ""
    echo "Usage:"
    echo "  $0 status          - Show current progress"
    echo "  $0 next            - Show next task details"
    echo "  $0 complete <task> - Mark task as complete (e.g., A1)"
    echo "  $0 note \"text\"     - Add note to current task"
    echo ""
}

#######################################
# Main
#######################################
case "$1" in
    status)
        show_progress
        ;;
    next)
        show_next
        ;;
    complete)
        complete_task "$2"
        ;;
    note)
        add_note "$2"
        ;;
    *)
        show_help
        ;;
esac
