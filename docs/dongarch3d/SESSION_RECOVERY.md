# Session Recovery Guide

**For Claude Code**: Use this guide to recover full context after `/compact`, `/clear`, or starting a new session.

---

## Quick Recovery (2 minutes)

**When to use**: Normal session start, minor context loss

### Step 1: Read Core Files (30 seconds)

```bash
# Essential project context
cat A:\1105\CLAUDE.md
cat A:\1105\.dongarch3d-progress.json
cat A:\1105\docs\dongarch3d\CONTEXT_CORE.md
```

These 3 files provide:
- ✅ Project overview and principles
- ✅ Current Phase/Task
- ✅ Recent progress summary

### Step 2: Check Recent Work (30 seconds)

```bash
# Find most recent daily log
ls -lt A:\1105\docs\dongarch3d\.logs\daily\ | head -5
# Read it
cat A:\1105\docs\dongarch3d\.logs\daily\[YYYY-MM-DD].md
```

This shows:
- ✅ What was completed last session
- ✅ What's in progress
- ✅ What to work on next
- ✅ Any blockers

### Step 3: Verify Build State (30 seconds)

```bash
cd A:\1105\GigaMesh\build_korean
cmake --build . --config Release --target DongArch3D
```

This confirms:
- ✅ Code compiles
- ✅ No broken state from last session

### Step 4: Status Check (30 seconds)

Use command:
```bash
/dongarch-status
```

Or manually:
```bash
cat .dongarch3d-progress.json
git status
git log -5 --oneline
```

**Recovery Complete**: You now have enough context to continue work.

---

## Full Recovery (5 minutes)

**When to use**: After `/compact`, significant context loss, or multi-day break

### Step 1: Read Core Files (1 minute)

```bash
# Essential project context
cat A:\1105\CLAUDE.md                           # Project guide
cat A:\1105\README.md                           # Project overview
cat A:\1105\.dongarch3d-progress.json           # Current state
cat A:\1105\docs\dongarch3d\CONTEXT_CORE.md     # Development context
cat A:\1105\docs\dongarch3d\DESIGN_PRINCIPLES.md # Design rules
```

### Step 2: Review Current Phase (1 minute)

```bash
# Determine current Phase from progress.json
# Then read Phase overview
cat A:\1105\docs\dongarch3d\implementation_reports\Phase_X_Overview.md

# If Phase is complete, read completion report
cat A:\1105\docs\dongarch3d\implementation_reports\Phase_X_COMPLETE.md
```

### Step 3: Review Recent History (2 minutes)

```bash
# Last 5 daily logs
ls -lt A:\1105\docs\dongarch3d\.logs\daily\ | head -6
cat A:\1105\docs\dongarch3d\.logs\daily\[recent dates].md

# Last 10 commits
git log -10 --oneline --stat

# Git status
git status
```

### Step 4: Review Architecture & Guidelines (1 minute)

```bash
# Detailed system architecture
cat A:\1105\.claude\architecture.md | head -100  # First 100 lines

# Coding guidelines
cat A:\1105\.claude\guidelines.md | head -100    # First 100 lines

# Recent decisions
cat A:\1105\.claude\decisions.md | tail -50      # Last 50 lines
```

### Step 5: Check Auto-commit Daemon (30 seconds)

```bash
# Check if daemon is running
ps aux | grep autocommit_daemon.py

# If not running, check logs
cat A:\1105\docs\dongarch3d\.logs\daemon.log | tail -20

# Restart if needed
cd A:\1105\docs\dongarch3d\.scripts
python autocommit_daemon.py > /dev/null 2>&1 &
```

### Step 6: Verify Build (30 seconds)

```bash
cd A:\1105\GigaMesh\build_korean
cmake --build . --config Release --target DongArch3D

# Check executable exists
ls -lh gui\Release\DongArch3D.exe
```

**Full Recovery Complete**: You now have complete context.

---

## Emergency Recovery (10 minutes)

**When to use**: Total context loss, corrupted files, or critical blocker

### Step 1: Verify Git State (1 minute)

```bash
cd A:\1105
git status
git log -1

# If detached HEAD or other issues
git checkout master
git pull origin master
```

### Step 2: Verify File Integrity (2 minutes)

```bash
# Check all critical files exist
test -f CLAUDE.md && echo "✅ CLAUDE.md" || echo "❌ CLAUDE.md MISSING"
test -f .dongarch3d-progress.json && echo "✅ progress.json" || echo "❌ progress.json MISSING"
test -f docs/dongarch3d/CONTEXT_CORE.md && echo "✅ CONTEXT_CORE.md" || echo "❌ CONTEXT_CORE.md MISSING"
test -d GigaMesh/gui/src && echo "✅ Source code" || echo "❌ Source MISSING"

# If any files missing, restore from git
git checkout HEAD -- [missing file]
```

### Step 3: Read ALL Documentation (5 minutes)

Execute **Full Recovery** steps above, then additionally:

```bash
# Read complete architecture
cat A:\1105\.claude\architecture.md

# Read complete guidelines
cat A:\1105\.claude\guidelines.md

# Read ALL Phase overviews
cat A:\1105\docs\dongarch3d\implementation_reports\Phase_*.md

# Read changelog
cat A:\1105\.claude\changelog.md

# Read all decisions
cat A:\1105\.claude\decisions.md
```

### Step 4: Rebuild from Scratch (2 minutes)

```bash
# Clean build
cd A:\1105\GigaMesh\build_korean
cmake --build . --config Release --target clean
cmake --build . --config Release --target DongArch3D

# Verify success
test -f gui\Release\DongArch3D.exe && echo "✅ Build successful" || echo "❌ Build FAILED"
```

### Step 5: Manual Progress Check

```bash
# Read progress.json manually
cat .dongarch3d-progress.json | python -m json.tool

# Verify Phase/Task makes sense
# If progress.json corrupted, restore from last daily log or git history
```

**Emergency Recovery Complete**: Context fully restored.

---

## Automated Recovery Hooks

### SessionStart Hook (Auto-runs on every new session)

Located: `A:\1105\.claude\settings.local.json`

```json
{
  "hooks": {
    "SessionStart": [
      {
        "command": "/dongarch-briefing"
      }
    ]
  }
}
```

This automatically:
- ✅ Reads CLAUDE.md
- ✅ Reads .dongarch3d-progress.json
- ✅ Displays current Phase/Task
- ✅ Shows recent commits
- ✅ Lists next actions

**No manual action needed** - recovery is automatic!

### UserPromptSubmit Hook (Auto-detects context loss)

Located: `A:\1105\.claude\commands\hook-auto-recovery.md`

Automatically detects:
- ❌ Missing DongArch3D context
- ❌ Unknown current Phase/Task
- ❌ Out-of-date information

And auto-recovers by reading necessary files.

---

## Recovery Checklist

Use this checklist to verify successful recovery:

### Context Recovery:
- [ ] Know current Phase (0-7)
- [ ] Know current Task (X.Y)
- [ ] Know overall progress percentage
- [ ] Know what was completed last session
- [ ] Know what to work on next
- [ ] Know any blockers or issues

### Technical State:
- [ ] Code builds successfully
- [ ] Git repository is clean (no unexpected changes)
- [ ] Auto-commit daemon is running
- [ ] All critical files exist and are readable

### Understanding:
- [ ] Understand project structure (GigaMesh vs DongArch3D)
- [ ] Understand current Phase goals
- [ ] Understand design principles (Korean UI, etc.)
- [ ] Understand coding guidelines
- [ ] Know which GigaMesh functions to use

### Ready to Work:
- [ ] Have clear next action
- [ ] Know expected outcome
- [ ] Know how to test changes
- [ ] Know how to commit changes

**If all checked**: ✅ Recovery successful, ready to code!

**If some unchecked**: Run appropriate recovery level again.

---

## Common Recovery Scenarios

### Scenario 1: "/compact was just run"

**Recovery**: Quick Recovery (2 min)
- Read CLAUDE.md, progress.json, CONTEXT_CORE.md
- Read most recent daily log
- Check git status
- **Done**

### Scenario 2: "New day, continuing work"

**Recovery**: Quick Recovery (2 min)
- SessionStart hook runs automatically
- Review yesterday's daily log
- **Done**

### Scenario 3: "Came back after 3+ days"

**Recovery**: Full Recovery (5 min)
- Read all core files
- Read Phase overview
- Review last 5 daily logs
- Rebuild to ensure clean state
- **Done**

### Scenario 4: "Don't remember what Phase we're in"

**Recovery**: Full Recovery (5 min)
- Read progress.json
- Read CONTEXT_CORE.md
- Read current Phase overview
- Read recent daily logs
- **Done**

### Scenario 5: "Code won't build"

**Recovery**: Emergency Recovery (10 min)
- Check git status
- Read last daily log for known issues
- Review recent commits for breaking changes
- Clean rebuild
- If still failing, restore from last Phase tag
- **Done**

### Scenario 6: "Progress.json is corrupted"

**Recovery**: Emergency Recovery (10 min)
- Check git log for last known good state
- Read recent daily logs to reconstruct progress
- Manually recreate progress.json from template:

```json
{
  "current": {
    "phase": X,
    "task_id": "X.Y",
    "phase_name": "[from daily log]"
  },
  "statistics": {
    "completion_percentage": XX,
    "tasks_completed": YY,
    "tasks_total": 71
  }
}
```

---

## File Locations Reference

### Core Context Files:
- `A:\1105\CLAUDE.md` - Project guide for Claude
- `A:\1105\README.md` - Project overview
- `A:\1105\.dongarch3d-progress.json` - Current state (JSON)
- `A:\1105\docs\dongarch3d\CONTEXT_CORE.md` - Development context
- `A:\1105\docs\dongarch3d\DESIGN_PRINCIPLES.md` - Design rules

### Documentation:
- `A:\1105\.claude\architecture.md` - System architecture (752 lines)
- `A:\1105\.claude\guidelines.md` - Coding guidelines (726 lines)
- `A:\1105\.claude\decisions.md` - ADRs (Architecture Decision Records)
- `A:\1105\.claude\changelog.md` - Version history

### Daily Logs:
- `A:\1105\docs\dongarch3d\.logs\daily\YYYY-MM-DD.md` - Daily work logs
- `A:\1105\docs\dongarch3d\.logs\daemon.log` - Auto-commit daemon log
- `A:\1105\docs\dongarch3d\.logs\autocommit_log.json` - Commit history (JSON)

### Phase Reports:
- `A:\1105\docs\dongarch3d\implementation_reports\Phase_X_Overview.md` - Phase plans
- `A:\1105\docs\dongarch3d\implementation_reports\Phase_X_COMPLETE.md` - Completion reports

### Commands:
- `A:\1105\.claude\commands\dongarch-*.md` - All DongArch3D commands
- `A:\1105\.claude\commands\hook-*.md` - Automated hooks

---

## Tips for Maintaining Context

### For Claude:

1. **Always run `/dongarch-daily-log` at end of session**
   - This preserves context for next session
   - Takes only 2 minutes
   - Saves 10+ minutes of recovery time

2. **Before `/compact`, save critical state**
   - Note current Phase/Task
   - Note next action
   - Run `/dongarch-daily-log` first

3. **Use `/dongarch-briefing` frequently**
   - Start of every session
   - After major task completion
   - When feeling lost

4. **Check progress.json regularly**
   - Ensures you're tracking progress correctly
   - Catches errors early

### For User:

1. **Let auto-commit daemon run**
   - Preserves git history automatically
   - Enables easy rollback if needed

2. **Use Phase checkpoints**
   - `/dongarch-checkpoint` at end of each Phase
   - Creates comprehensive snapshot

3. **Keep daily logs**
   - Run `/dongarch-daily-log` daily
   - Even if only small progress

---

**Document Version**: 2.0.0
**Last Updated**: 2025-11-08
**Related**: dongarch-briefing.md, dongarch-daily-log.md, CONTEXT_CORE.md
