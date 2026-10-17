---
description: 현재 DongArch3D 프로젝트 진행 상황 빠른 확인 (v4.0)
---

Display the current DongArch3D project status including Phase, Task, and completion metrics.

## Step 1: Read Progress File

```bash
cat A:\1105\.dongarch3d-progress.json
```

Parse the following information:
- Current Phase (0-7) and Phase name
- Current Task ID and Task name
- Task status (pending/in_progress/completed)
- Tasks completed vs total in current Phase
- Overall project completion percentage

## Step 2: Display Status Report

Format and display:

```
═══════════════════════════════════════════════════════
📊 DongArch3D Project Status (v4.0 - Quality First)
═══════════════════════════════════════════════════════

🎯 v4.0 Core Principles:
   Time Constraint: None (Quality first priority)
   Goal: Surpass Arch3D Liner
   GigaMesh: 70% reuse + Modern Tech 30%
   C++20: Full adoption

📍 Current Position:
   Phase: [X] ([Phase Name])
   Task:  [XXX] ([Task Name])
   Status: [In Progress / Pending / Completed]

📈 Completion (Quality-based):
   Task:    [X]/[Y] ([ZZ]% complete)
   Phase:   [X]/8 ([ZZ]% complete)
   Overall: Function first, time independent

🎯 Phase [X] Goals:
   - [Goal 1] [✅/🔄/⏳]
   - [Goal 2] [✅/🔄/⏳]
   - [Goal 3] [✅/🔄/⏳]
   - [Goal 4] [✅/🔄/⏳]
   - [Goal 5] [✅/🔄/⏳]

📝 Next Task:
   Task [XXX]: [Task Name]
   - [Subtask 1]
   - [Subtask 2]
   - [Subtask 3]
   Reference: docs/references/algorithms/research-papers.md

💡 Available Resources:
   - 115 reference documents
   - 50 research papers (2008-2025)
   - GigaMesh proven codebase

═══════════════════════════════════════════════════════
```

## Step 3: Show Recent Commits (Optional)

If user might find it helpful, also display last 3-5 commits:

```
🔄 Recent Commits:
   - [hash1]: [commit message 1]
   - [hash2]: [commit message 2]
   - [hash3]: [commit message 3]
```

## Step 4: Highlight Any Issues

If there are blockers or warnings in the progress file, highlight them:

```
⚠️ Current Blockers:
   - [Blocker description]

💡 Suggested Actions:
   - [Action to resolve blocker]
```

## When to Use

- Quick status check during development
- Before planning next work session
- During daily standup reporting
- When switching between tasks
- After returning from a break

## Notes

This command provides a quick snapshot. For comprehensive briefing with system status, git activity, and detailed context, use `/dongarch-briefing` instead.
