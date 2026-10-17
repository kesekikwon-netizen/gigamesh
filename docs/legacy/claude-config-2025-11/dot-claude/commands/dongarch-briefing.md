---
description: 세션 시작 시 DongArch3D 프로젝트 전체 브리핑 제공 (v4.0)
---

Provide a comprehensive project briefing at the start of each Claude Code session to enable immediate productive work.

## Step 1: Read Current State (Silent)

Read these files without displaying to user:
```bash
cat A:\1105\CLAUDE.md
cat A:\1105\.dongarch3d-progress.json
cat A:\1105\docs\dongarch3d\CONTEXT_CORE.md
```

Parse:
- Project overview and v4.0 principles
- Current Phase/Task
- Progress statistics (tasks completed, quality metrics)

## Step 2: Read Recent History (Silent)

```bash
# Find most recent daily log
ls -lt A:\1105\docs\dongarch3d\.logs\daily\ | head -2

# Read the latest log
cat A:\1105\docs\dongarch3d\.logs\daily\[YYYY-MM-DD].md

# Get last 5 commits
git log -5 --oneline

# Check git status
git status --short
```

## Step 3: Check System State (Silent)

```bash
# Check auto-commit daemon
cat A:\1105\docs\dongarch3d\.logs\daemon.log | tail -1

# Quick build check (if feasible)
cd A:\1105\GigaMesh\build_korean
cmake --build . --config Release --target DongArch3D 2>&1 | grep -E "(error|warning|succeeded)"
```

## Step 4: Display Comprehensive Briefing

Format the briefing as follows:

```
╔═══════════════════════════════════════════════════════════════════╗
║           DongArch3D Development Session Briefing                 ║
╚═══════════════════════════════════════════════════════════════════╝

📊 PROJECT OVERVIEW (v4.0)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Project:    DongArch3D v4.0 (Quality First, Surpass Arch3D Liner)
Goal:       Archaeological 3D Measurement System for Dongguk Institute
Timeline:   No time constraint (Quality first priority) ⭐
Foundation: GigaMesh 70% + Modern Tech 30% (C++20, 2025 papers)

📍 CURRENT STATE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Phase:      Phase [X] - [Phase Name]
Task:       Task [X.Y] - [Task Name]
Completion: [YY]/71 tasks (100% quality target)
Approach:   Time-independent, quality-first

Phase Progress:
  ✅ Completed: [list of completed tasks]
  🔄 In Progress: Task [X.Y] - [Task Name] ([XX]% done)
  📝 Remaining: [list of remaining tasks]

🎯 v4.0 PRINCIPLES
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Time Constraint:    None (quality first)
Goal:              Surpass Arch3D Liner
GigaMesh Reuse:    70% (proven code)
New Tech:          30% (C++20, 2025 papers: 3 latest)
GPU Acceleration:  Geometry/Fragment Shaders (10-100x faster)

📋 LAST SESSION SUMMARY ([YYYY-MM-DD])
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Completed:
  ✅ [Task completed 1]
  ✅ [Task completed 2]

In Progress:
  🔄 [Task in progress] - [XX]% complete

Blockers:
  ⚠️ [Blocker if any] / None ✅

📝 NEXT ACTIONS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Priority 1: [Most important next task]
Priority 2: [Second priority task]
Priority 3: [Third priority task]

Recommended: Start with [specific task] - [reason]

🔧 SYSTEM STATUS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Build:          ✅ Success / ⚠️ Warnings / ❌ Failed
Auto-commit:    ✅ Running (check daemon.log) / ⚠️ Not running
Git:            ✅ Clean / ⚠️ [X] uncommitted files
Branch:         master
Last Commit:    [commit hash] - [commit message]

📂 RECENT ACTIVITY
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Last 5 Commits:
  [hash1] - [message 1]
  [hash2] - [message 2]
  [hash3] - [message 3]
  [hash4] - [message 4]
  [hash5] - [message 5]

Files Modified Recently:
  - [file 1]
  - [file 2]
  - [file 3]

🎯 PHASE [X] GOALS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Brief description of current Phase goals]

Key Deliverables:
  - [Deliverable 1]
  - [Deliverable 2]
  - [Deliverable 3]

GigaMesh Integration:
  - Using: [GigaMesh function/class being utilized]
  - Time Saved: [X] days through code reuse

🚀 UPCOMING MILESTONES
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Next Milestone: [Phase X Task Y] - Target: [YYYY-MM-DD]
Phase Completion: Phase [X] - Estimated: [YYYY-MM-DD]
Project Completion: v1.0.0 - Estimated: [YYYY-MM-DD]

📚 QUICK REFERENCE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Commands:
  /dongarch-status     - Detailed progress report
  /dongarch-complete   - Mark current task complete
  /dongarch-checkpoint - Complete current Phase

Docs:
  CLAUDE.md                              - Project guide
  .claude/DongArch3D_최종개선계획_v4_*.md - Master plan
  .claude/architecture.md                - System architecture

Build:
  cd GigaMesh/build_korean
  cmake --build . --config Release --target DongArch3D

⚠️ IMPORTANT REMINDERS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ All new UI must be 100% Korean (no English mixed!)
✅ Use GigaMesh functions where possible (70% code reuse target)
✅ Follow Arch3D Liner workflow (see PDF guide)
✅ DongArch* prefix for new classes (separate from QGM*)
✅ Always verify build before committing

═══════════════════════════════════════════════════════════════════════

Ready to start work! 🚀

Type `/dongarch-status` for detailed progress.
```

## Step 5: Append Warnings (If Issues Detected)

If any problems found, append:

```
⚠️ WARNINGS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
❌ Build failed - Last build had errors
   → Run build to see errors: cd GigaMesh/build_korean && cmake --build . --config Release

⚠️ Auto-commit daemon not running
   → Check: cat docs/dongarch3d/.logs/daemon.log

⚠️ Uncommitted files detected
   → Run: git status
```

## Step 6: Suggest Next Action

Based on context, suggest specific next action:

```
💡 SUGGESTED NEXT ACTION
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Based on current state, I recommend:

[Option 1]: Continue Task [X.Y] - [Task Name]
  - You're [XX]% complete on this task
  - Next step: [specific action]

[Option 2]: Fix blocker: [Blocker description]
  - Blocking progress on Task [X.Z]

What would you like to work on?
```

## When This Runs

### Automatic:
- Every time Claude Code session starts (SessionStart hook)
- After `/compact` (new session)
- After `/clear` (new session)

### Manual:
- User types `/dongarch-briefing`
- After returning from a break
- When needing full context refresh
