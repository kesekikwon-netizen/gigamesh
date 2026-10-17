#!/usr/bin/env ts-node
/**
 * MCP Analysis Server - Static code analysis with clang-tidy/cppcheck
 *
 * Features:
 * - Run clang-tidy on specific files or directories
 * - Execute cppcheck with custom rules
 * - Filter issues by severity
 * - Group issues by file or category
 * - Generate summary reports
 */

import { execSync } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import { glob } from 'glob';

// ============================================================================
// Type Definitions
// ============================================================================

export interface AnalysisConfig {
  tool: 'clang-tidy' | 'cppcheck';
  sourceDir: string;
  checks?: string;                    // clang-tidy checks (e.g., 'modernize-*')
  severityFilter?: string[];          // ['error', 'warning', 'style']
  excludePatterns?: RegExp[];         // Exclude files matching patterns
  buildDir?: string;                  // For clang-tidy compile_commands.json
  maxIssues?: number;                 // Limit output (default: 100)
}

export interface Issue {
  file: string;
  line: number;
  column?: number;
  severity: string;                   // 'error', 'warning', 'style', 'performance', etc.
  message: string;
  check: string;                      // Check name (e.g., 'modernize-use-auto')
  context?: string;
}

export interface AnalysisResult {
  totalIssues: number;
  byFile: Map<string, Issue[]>;
  bySeverity: Map<string, Issue[]>;
  byCheck: Map<string, Issue[]>;
  summary: string;
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Find all C++ source files in directory
 */
function findSourceFiles(sourceDir: string, excludePatterns: RegExp[] = []): string[] {
  const extensions = ['cpp', 'cc', 'cxx', 'c'];
  const files: string[] = [];

  extensions.forEach(ext => {
    const pattern = path.join(sourceDir, `**/*.${ext}`);
    const matches = glob.sync(pattern, { nodir: true });
    files.push(...matches);
  });

  // Apply exclusion patterns
  return files.filter(file => {
    return !excludePatterns.some(pattern => pattern.test(file));
  });
}

/**
 * Parse clang-tidy output
 *
 * Format:
 * /path/to/file.cpp:123:45: warning: use auto when initializing with new [modernize-use-auto]
 *   Foo* foo = new Foo();
 *   ^~~~~~~~~~~
 *   auto
 */
function parseClangTidyOutput(output: string, excludePatterns: RegExp[] = []): Issue[] {
  const issues: Issue[] = [];
  const lines = output.split('\n');

  const issueRegex = /^(.+?):(\d+):(\d+):\s+(error|warning|note):\s+(.+?)\s+\[(.+?)\]$/;

  for (const line of lines) {
    const match = line.match(issueRegex);
    if (!match) continue;

    const [, file, lineNum, colNum, severity, message, check] = match;

    // Apply exclusion
    if (excludePatterns.some(pattern => pattern.test(file))) {
      continue;
    }

    issues.push({
      file: path.basename(file),
      line: parseInt(lineNum, 10),
      column: parseInt(colNum, 10),
      severity,
      message: message.trim(),
      check: check.trim()
    });
  }

  return issues;
}

/**
 * Parse cppcheck output
 *
 * Format:
 * [file.cpp:123]: (warning) Unused variable: foo
 */
function parseCppcheckOutput(output: string, excludePatterns: RegExp[] = []): Issue[] {
  const issues: Issue[] = [];
  const lines = output.split('\n');

  const issueRegex = /^\[(.+?):(\d+)\]:\s+\((.+?)\)\s+(.+)$/;

  for (const line of lines) {
    const match = line.match(issueRegex);
    if (!match) continue;

    const [, file, lineNum, severity, message] = match;

    // Apply exclusion
    if (excludePatterns.some(pattern => pattern.test(file))) {
      continue;
    }

    issues.push({
      file: path.basename(file),
      line: parseInt(lineNum, 10),
      severity,
      message: message.trim(),
      check: 'cppcheck'
    });
  }

  return issues;
}

/**
 * Run clang-tidy analysis
 */
function runClangTidy(
  sourceFiles: string[],
  checks: string = '*',
  buildDir?: string
): string {
  // Build clang-tidy command
  let cmd = `clang-tidy --checks="${checks}"`;

  if (buildDir) {
    const compileCommandsPath = path.join(buildDir, 'compile_commands.json');
    if (fs.existsSync(compileCommandsPath)) {
      cmd += ` -p "${buildDir}"`;
    }
  }

  cmd += ' ' + sourceFiles.map(f => `"${f}"`).join(' ');

  console.log(`[clang-tidy] Running: ${cmd}`);

  try {
    return execSync(cmd, {
      encoding: 'utf-8',
      maxBuffer: 50 * 1024 * 1024,  // 50MB buffer
      timeout: 300000  // 5 minutes
    });
  } catch (error: any) {
    // clang-tidy returns non-zero if issues found
    return error.stdout || '';
  }
}

/**
 * Run cppcheck analysis
 */
function runCppcheck(sourceFiles: string[]): string {
  const cmd = `cppcheck --enable=all --inline-suppr --quiet ${sourceFiles.map(f => `"${f}"`).join(' ')}`;

  console.log(`[cppcheck] Running: ${cmd}`);

  try {
    return execSync(cmd, {
      encoding: 'utf-8',
      stderr: 'pipe',  // cppcheck outputs to stderr
      maxBuffer: 50 * 1024 * 1024,
      timeout: 300000
    });
  } catch (error: any) {
    return error.stderr || '';
  }
}

// ============================================================================
// Main API
// ============================================================================

/**
 * Analyze code with clang-tidy or cppcheck
 *
 * Example:
 * ```typescript
 * const result = analyzeCode({
 *   tool: 'clang-tidy',
 *   sourceDir: '/path/to/src/dongarch/cutline',
 *   checks: 'modernize-*,readability-*',
 *   severityFilter: ['error', 'warning'],
 *   excludePatterns: [/moc_.*/, /ui_.*\.h/]
 * });
 *
 * console.log(result.summary);
 * result.byFile.forEach((issues, file) => {
 *   console.log(`${file}: ${issues.length} issues`);
 * });
 * ```
 */
export function analyzeCode(config: AnalysisConfig): AnalysisResult {
  const {
    tool,
    sourceDir,
    checks = '*',
    severityFilter = [],
    excludePatterns = [],
    buildDir,
    maxIssues = 100
  } = config;

  // Validate source directory
  if (!fs.existsSync(sourceDir)) {
    throw new Error(`Source directory does not exist: ${sourceDir}`);
  }

  // Find source files
  const sourceFiles = findSourceFiles(sourceDir, excludePatterns);
  if (sourceFiles.length === 0) {
    return {
      totalIssues: 0,
      byFile: new Map(),
      bySeverity: new Map(),
      byCheck: new Map(),
      summary: 'No source files found'
    };
  }

  console.log(`[Analysis] Found ${sourceFiles.length} source file(s)`);

  // Run analysis tool
  let output = '';
  let issues: Issue[] = [];

  if (tool === 'clang-tidy') {
    output = runClangTidy(sourceFiles, checks, buildDir);
    issues = parseClangTidyOutput(output, excludePatterns);
  } else if (tool === 'cppcheck') {
    output = runCppcheck(sourceFiles);
    issues = parseCppcheckOutput(output, excludePatterns);
  } else {
    throw new Error(`Unknown analysis tool: ${tool}`);
  }

  // Apply severity filter
  if (severityFilter.length > 0) {
    issues = issues.filter(issue => severityFilter.includes(issue.severity));
  }

  // Limit output
  if (issues.length > maxIssues) {
    console.log(`[Analysis] Limiting output to ${maxIssues} issues (total: ${issues.length})`);
    issues = issues.slice(0, maxIssues);
  }

  // Group issues
  const byFile = new Map<string, Issue[]>();
  const bySeverity = new Map<string, Issue[]>();
  const byCheck = new Map<string, Issue[]>();

  issues.forEach(issue => {
    // By file
    if (!byFile.has(issue.file)) {
      byFile.set(issue.file, []);
    }
    byFile.get(issue.file)!.push(issue);

    // By severity
    if (!bySeverity.has(issue.severity)) {
      bySeverity.set(issue.severity, []);
    }
    bySeverity.get(issue.severity)!.push(issue);

    // By check
    if (!byCheck.has(issue.check)) {
      byCheck.set(issue.check, []);
    }
    byCheck.get(issue.check)!.push(issue);
  });

  // Generate summary
  const severityCounts = Array.from(bySeverity.entries())
    .map(([sev, iss]) => `${iss.length} ${sev}`)
    .join(', ');

  const summary = `Found ${issues.length} issue(s) in ${byFile.size} file(s): ${severityCounts}`;

  return {
    totalIssues: issues.length,
    byFile,
    bySeverity,
    byCheck,
    summary
  };
}

/**
 * Quick analysis with error-only output
 */
export function analyzeQuick(sourceDir: string, tool: 'clang-tidy' | 'cppcheck' = 'clang-tidy'): AnalysisResult {
  return analyzeCode({
    tool,
    sourceDir,
    severityFilter: ['error']
  });
}

// ============================================================================
// CLI Interface (if run directly)
// ============================================================================

if (require.main === module) {
  const args = process.argv.slice(2);

  if (args.length < 2) {
    console.error('Usage: ts-node analysis-server.ts <tool> <sourceDir> [checks] [--errors-only]');
    console.error('');
    console.error('Tools: clang-tidy, cppcheck');
    console.error('');
    console.error('Example:');
    console.error('  ts-node analysis-server.ts clang-tidy /path/to/src "modernize-*" --errors-only');
    process.exit(1);
  }

  const tool = args[0] as 'clang-tidy' | 'cppcheck';
  const sourceDir = args[1];
  const checks = args[2] && !args[2].startsWith('--') ? args[2] : '*';
  const errorsOnly = args.includes('--errors-only');

  try {
    const result = analyzeCode({
      tool,
      sourceDir,
      checks,
      severityFilter: errorsOnly ? ['error'] : []
    });

    console.log('\n' + '='.repeat(80));
    console.log('ANALYSIS RESULT');
    console.log('='.repeat(80));
    console.log(result.summary);
    console.log('');

    if (result.totalIssues > 0) {
      console.log('ISSUES BY FILE:');
      Array.from(result.byFile.entries())
        .sort((a, b) => b[1].length - a[1].length)  // Sort by issue count
        .forEach(([file, issues]) => {
          console.log(`  ${file}: ${issues.length} issue(s)`);
        });
      console.log('');

      console.log('ISSUES BY SEVERITY:');
      Array.from(result.bySeverity.entries())
        .forEach(([severity, issues]) => {
          console.log(`  ${severity}: ${issues.length}`);
        });
      console.log('');

      console.log('TOP ISSUES:');
      result.byFile.forEach((issues, file) => {
        issues.slice(0, 3).forEach(issue => {
          console.log(`  ${file}:${issue.line}: [${issue.severity}] ${issue.message}`);
          console.log(`    Check: ${issue.check}`);
        });
      });
    }

    console.log('='.repeat(80));

    // Exit with appropriate code
    process.exit(result.totalIssues > 0 ? 1 : 0);

  } catch (error) {
    console.error('Analysis server error:', error);
    process.exit(1);
  }
}
