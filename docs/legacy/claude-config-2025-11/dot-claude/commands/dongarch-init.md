---
description: DongArch3D 세션 시작 시 프로젝트 컨텍스트 자동 복구 (v4.0)
---

Read and load the DongArch3D project context at the start of every Claude Code session.

## Step 1: Read Progress State

Read `A:\1105\.dongarch3d-progress.json` to understand:
- Current Phase and Task
- Completion status
- What was being worked on last

## Step 2: Load Core Documentation

Read these core files silently to restore context:
1. `A:\1105\CLAUDE.md` - Project guide (v4.0 quality-first principles)
2. `A:\1105\.claude\DongArch3D_최종개선계획_v4_품질최우선.md` - Final master plan
3. `A:\1105\.claude\V4_UPDATE_SUMMARY.md` - v4.0 update summary
4. `A:\1105\docs\dongarch3d\CONTEXT_CORE.md` - Core context

## Step 3: Check Auto-Commit Daemon

Check if the auto-commit daemon is running:
```bash
cat A:\1105\docs\dongarch3d\.logs\daemon.log
```

If daemon is not running, automatically execute `/dongarch-autocommit` to start it.

## Step 4: Display Status Report

Show the user a concise status report:

```
✅ DongArch3D v4.0 Context Restored!

🎯 v4.0 Core Principles:
   Time Constraint: None (Quality First)
   Goal: Surpass Arch3D Liner
   GigaMesh Reuse: 70% + Modern Tech 30%
   C++20: Full adoption

📊 Current Progress:
   Phase: [X] ([Phase Name])
   Task: [XXX] ([Task Name])
   Quality: [XX.X%] (100% target)

🔄 Auto-Commit: [Running / Stopped]

📝 Next Actions:
   - [Description of next task to work on]

💡 Resources: 115 docs + 50 research papers available
```

## Step 5: Verify System Health

Silently check:
- Git status (clean working directory?)
- Last build status
- Any blockers from last session

If issues found, append warnings to the status report.

## When This Runs

- Automatically at session start (SessionStart hook)
- After `/compact` or `/clear`
- When returning after a break
