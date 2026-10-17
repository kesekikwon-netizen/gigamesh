# Phase Checkpoint - DongArch3D (v4.0)

**Usage**: `/dongarch-checkpoint`

**버전**: v4.0 (품질 최우선, 시간 제약 없음)

## Purpose
Create a comprehensive checkpoint when completing a Phase (0-7) in DongArch3D v4.0 development (품질 100% 기준).

## What This Command Does

1. **Verifies Phase Completion**:
   - Checks all tasks in current Phase are 100% complete
   - Verifies build succeeds
   - Runs basic functionality tests
   - Confirms no critical blockers remain

2. **Creates Phase Completion Report**:
   - Summary of all tasks completed
   - Code metrics (files added/modified, lines of code)
   - Build artifacts (executable size, build time)
   - Test results
   - Known issues and workarounds

3. **Updates Project Context**:
   - Updates `.dongarch3d-progress.json`
   - Creates Phase completion document
   - Updates CONTEXT_CORE.md

4. **Creates Git Checkpoint**:
   - Creates git tag: `phase-X-complete`
   - Optional: Creates git branch for next Phase
   - Documents commit range for Phase

5. **Prepares Next Phase**:
   - Displays next Phase overview
   - Lists next Phase tasks
   - Estimates time remaining

## Instructions for Claude

When this command is invoked:

### Step 1: Read Current Progress

```bash
# Read progress file
cat .dongarch3d-progress.json
```

Verify:
- Current Phase number (0-7)
- All tasks in current Phase are "completed"
- Completion percentage for Phase = 100%

If Phase is NOT 100% complete:
- **STOP** and inform user
- List incomplete tasks
- Ask if user wants to force checkpoint anyway

### Step 2: Verify Build

```bash
cd GigaMesh/build_korean
cmake --build . --config Release --target DongArch3D
```

If build fails:
- **STOP** and inform user
- Show build errors
- Recommend fixing before checkpoint

If build succeeds:
- Record executable size
- Record build time
- Note any warnings

### Step 3: Run Basic Tests

Depending on Phase, run appropriate tests:

**Phase 0**: Load test PLY files
**Phase 1**: Test rotation and ViewPoint
**Phase 2**: Test Cutline extraction
**Phase 3**: Test Outline extraction
**Phase 4**: Test Clip operation
**Phase 5**: Test X-Ray and D-Tak rendering
**Phase 6**: Test MFE file operations
**Phase 7**: Test SVG export

If tests fail:
- **WARN** user but allow checkpoint
- Document failing tests in report

### Step 4: Collect Metrics

Gather Phase metrics:

```bash
# Count commits in Phase
git log --oneline --since="[Phase start date]" | wc -l

# Count files modified
git diff --stat phase-[X-1]-complete..HEAD | tail -1

# Code metrics (optional, if cloc available)
cloc GigaMesh/gui/src/
```

### Step 5: Create Phase Completion Report

File path: `A:\1105\docs\dongarch3d\implementation_reports\Phase_X_COMPLETE.md`

Structure:

```markdown
# Phase X Completion Report - [Phase Name]

**Completion Date**: YYYY-MM-DD
**Quality**: 100% (v4.0 기준)
**Status**: ✅ Complete / ⚠️ Complete with Issues

## Summary

Brief 2-3 sentence summary of Phase accomplishments.

## Tasks Completed

### Task X.1: [Task Name]
- **Status**: ✅ Complete
- **Duration**: X hours
- **Files**: [list]
- **Key Changes**: [description]

[Repeat for all tasks in Phase]

## Code Metrics

- **Files Added**: X
- **Files Modified**: Y
- **Lines Added**: +XXXX
- **Lines Removed**: -YYYY
- **Net Lines**: +ZZZZ
- **Commits**: XX commits

## Build Results

- **Build Status**: ✅ Success
- **Executable Size**: XX.X MB
- **Build Time**: X minutes
- **Warnings**: X warnings (acceptable)

## Test Results

[Phase-specific test results]

### Functional Tests
- ✅ Test 1: [description]
- ✅ Test 2: [description]
- ⚠️ Test 3: [description] - Known issue documented

### Performance Tests
- Load time: X.XX seconds (target: < Y seconds) ✅
- Render FPS: XX FPS (target: > 60 FPS) ✅

## Known Issues

1. **Issue**: [description]
   - **Severity**: Low/Medium/High
   - **Workaround**: [if available]
   - **Resolution**: Deferred to Phase X / Will fix in Phase X+1

[Repeat for all known issues]

## Technical Decisions

Key decisions made during this Phase:
- **ADR-XXX**: [decision title] - [brief summary]

[Reference .claude/decisions.md for full details]

## GigaMesh Integration

Parts of GigaMesh utilized:
- ✅ [Function/Class]: Used for [purpose]
- ✅ [Function/Class]: Used for [purpose]

Time saved: X days (due to GigaMesh reuse)

## Lessons Learned

1. [Lesson 1]
2. [Lesson 2]
3. [Lesson 3]

## Next Phase Preview

**Next**: Phase X+1 - [Phase Name]
**Duration**: Y days
**Key Tasks**: [list 3-5 main tasks]
**Dependencies**: [any dependencies on current Phase]

## Git Checkpoint

- **Tag**: phase-X-complete
- **Commit Range**: [hash1]...[hash2]
- **Total Commits**: XX commits

---

**Completed By**: Claude + User
**Sign-off Date**: YYYY-MM-DD
```

### Step 6: Update Project Files

1. **Update `.dongarch3d-progress.json`**:
```json
{
  "current": {
    "phase": X+1,
    "task_id": "[X+1].1",
    "phase_name": "[Next Phase Name]"
  },
  "phases_completed": [0, 1, ..., X],
  "statistics": {
    "completion_percentage": XX,
    "tasks_completed": YY,
    "tasks_total": 71,
    "phases_completed": X+1,
    "phases_total": 8
  }
}
```

2. **Update `docs/dongarch3d/CONTEXT_CORE.md`**:
```markdown
## Phases Completed

### Phase X - [Phase Name] ✅
- **Completed**: YYYY-MM-DD
- **Duration**: X days
- **Tasks**: All X tasks complete
- **Report**: [Link to Phase_X_COMPLETE.md]
```

### Step 7: Create Git Tag

```bash
git tag -a phase-X-complete -m "Phase X: [Phase Name] - Complete

All tasks finished:
- Task X.1: [name]
- Task X.2: [name]
...

Build successful: DongArch3D.exe (XX.X MB)
Tests passing: X/X tests

Ready for Phase X+1: [Next Phase Name]"

# Show tag
git show phase-X-complete
```

### Step 8: Display Next Phase Overview

Read and display: `docs/dongarch3d/implementation_reports/Phase_[X+1]_Overview.md`

Show:
- Next Phase goals
- Task breakdown
- Estimated duration
- GigaMesh integration points

### Step 9: Confirmation

Display summary:

```
🎉 Phase X Checkpoint Complete!

📊 Phase Summary:
   - Duration: X days (planned: Y days)
   - Tasks: XX/XX complete ✅
   - Build: Success ✅
   - Tests: Passing ✅

📄 Reports Created:
   - Phase_X_COMPLETE.md
   - Git tag: phase-X-complete

📈 Project Progress:
   - Overall: XX% complete (YY/71 tasks)
   - Phases: X/8 complete
   - Days Remaining: ZZ days

🚀 Next Up: Phase X+1 - [Phase Name]
   - Duration: Y days
   - Key Focus: [brief description]
   - First Task: [X+1].1 - [task name]

Would you like to start Phase X+1 now?
```

## When to Use

- **Mandatory**: At the end of each Phase (0-7)
- **Optional**: Mid-Phase if a major milestone is reached
- **Recommended**: Before taking a break longer than 3 days

## Integration with Other Commands

- **Before checkpoint**: Run `/dongarch-daily-log` to record final session
- **After checkpoint**: Run `/dongarch-briefing` to preview next Phase
- **During checkpoint**: Auto-commit daemon continues running

## Benefits

1. **Milestone Tracking**: Clear markers of progress
2. **Quality Assurance**: Forced verification at each Phase
3. **Context Preservation**: Comprehensive documentation for recovery
4. **Git History**: Clean tags for easy navigation
5. **Team Communication**: Shareable completion reports

---

**Version**: 4.0.0 (품질 최우선)
**Last Updated**: 2025-11-08
**Related**: dongarch-complete.md, dongarch-daily-log.md, dongarch-phase.md
