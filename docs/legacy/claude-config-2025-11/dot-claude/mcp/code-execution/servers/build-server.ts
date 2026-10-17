#!/usr/bin/env ts-node
/**
 * MCP Build Server - CMake + Make automation with intelligent filtering
 *
 * Features:
 * - Automated CMake configuration
 * - Parallel Make execution
 * - Error/warning extraction and filtering
 * - Build time measurement
 * - Structured JSON output
 */

import { execSync } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';

// ============================================================================
// Type Definitions
// ============================================================================

export interface BuildConfig {
  sourceDir: string;
  buildDir: string;
  target?: string;                    // Build specific target (default: all)
  cmakeFlags?: string[];              // Additional CMake flags
  makeJobs?: number;                  // Parallel jobs (default: 4)
  filterPattern?: RegExp;             // Regex to filter errors/warnings
  captureWarnings?: boolean;          // Include warnings in output (default: false)
  timeout?: number;                   // Timeout in seconds (default: 600)
}

export interface BuildIssue {
  file: string;
  line: number;
  column?: number;
  severity: 'error' | 'warning' | 'note';
  message: string;
  context?: string;                   // Surrounding code context
}

export interface BuildResult {
  success: boolean;
  errors: BuildIssue[];
  warnings: BuildIssue[];
  buildTime: number;                  // Milliseconds
  summary: string;
  fullLog?: string;                   // Optional: full build log
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Parse GCC/Clang compiler output into structured BuildIssue
 *
 * Format examples:
 * /path/to/file.cpp:123:45: error: 'Foo' is not a member of 'Bar'
 * /path/to/file.cpp:123:45: warning: unused variable 'x' [-Wunused-variable]
 */
function parseCompilerOutput(output: string, filterPattern?: RegExp): BuildIssue[] {
  const issues: BuildIssue[] = [];
  const lines = output.split('\n');

  // Regex to match GCC/Clang error format
  const issueRegex = /^(.+?):(\d+):(\d+):\s+(error|warning|note):\s+(.+)$/;

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const match = line.match(issueRegex);

    if (match) {
      const [, file, lineNum, colNum, severity, message] = match;

      // Apply filter if provided
      if (filterPattern && !filterPattern.test(file) && !filterPattern.test(message)) {
        continue;
      }

      // Extract context (next 1-2 lines often contain code snippet)
      let context = '';
      if (i + 1 < lines.length && lines[i + 1].trim().length > 0) {
        context = lines[i + 1].trim();
      }

      issues.push({
        file: path.basename(file),
        line: parseInt(lineNum, 10),
        column: parseInt(colNum, 10),
        severity: severity as 'error' | 'warning' | 'note',
        message: message.trim(),
        context
      });
    }
  }

  return issues;
}

/**
 * Run CMake configuration
 */
function runCMake(sourceDir: string, buildDir: string, cmakeFlags: string[] = []): void {
  if (!fs.existsSync(buildDir)) {
    fs.mkdirSync(buildDir, { recursive: true });
  }

  const flags = cmakeFlags.join(' ');
  const cmd = `cmake ${flags} -S "${sourceDir}" -B "${buildDir}"`;

  console.log(`[CMake] Configuring: ${cmd}`);

  try {
    execSync(cmd, {
      stdio: 'inherit',
      cwd: buildDir
    });
  } catch (error) {
    throw new Error(`CMake configuration failed: ${error}`);
  }
}

/**
 * Run Make build
 */
function runMake(
  buildDir: string,
  target: string = '',
  jobs: number = 4,
  timeout: number = 600
): { output: string; success: boolean; buildTime: number } {

  const targetArg = target ? target : '';
  const cmd = `make -j${jobs} ${targetArg}`;

  console.log(`[Make] Building: ${cmd}`);

  const startTime = Date.now();
  let output = '';
  let success = false;

  try {
    output = execSync(cmd, {
      cwd: buildDir,
      encoding: 'utf-8',
      timeout: timeout * 1000,
      maxBuffer: 10 * 1024 * 1024  // 10MB buffer
    });
    success = true;
  } catch (error: any) {
    // execSync throws on non-zero exit, but we still want the output
    output = error.stdout || error.stderr || '';
    success = false;
  }

  const buildTime = Date.now() - startTime;

  return { output, success, buildTime };
}

// ============================================================================
// Main API
// ============================================================================

/**
 * Build project with intelligent error filtering
 *
 * Example:
 * ```typescript
 * const result = await buildProject({
 *   sourceDir: '/path/to/GigaMesh',
 *   buildDir: '/path/to/GigaMesh/build',
 *   target: 'DongArch3D',
 *   filterPattern: /cutline|dongarch/i,
 *   captureWarnings: true
 * });
 *
 * console.log(`Build ${result.success ? 'succeeded' : 'failed'}`);
 * console.log(`Errors: ${result.errors.length}`);
 * ```
 */
export function buildProject(config: BuildConfig): BuildResult {
  const {
    sourceDir,
    buildDir,
    target = '',
    cmakeFlags = [],
    makeJobs = 4,
    filterPattern,
    captureWarnings = false,
    timeout = 600
  } = config;

  // Validate paths
  if (!fs.existsSync(sourceDir)) {
    throw new Error(`Source directory does not exist: ${sourceDir}`);
  }

  // Step 1: Run CMake if CMakeLists.txt exists but build dir is empty
  const cmakeListsPath = path.join(sourceDir, 'CMakeLists.txt');
  const cmakeCachePath = path.join(buildDir, 'CMakeCache.txt');

  if (fs.existsSync(cmakeListsPath) && !fs.existsSync(cmakeCachePath)) {
    console.log('[Build] Running CMake configuration...');
    runCMake(sourceDir, buildDir, cmakeFlags);
  }

  // Step 2: Run Make
  const { output, success, buildTime } = runMake(buildDir, target, makeJobs, timeout);

  // Step 3: Parse output
  const allIssues = parseCompilerOutput(output, filterPattern);

  const errors = allIssues.filter(i => i.severity === 'error');
  const warnings = captureWarnings
    ? allIssues.filter(i => i.severity === 'warning')
    : [];

  // Step 4: Generate summary
  let summary = success
    ? `Build succeeded in ${(buildTime / 1000).toFixed(1)}s`
    : `Build failed with ${errors.length} error(s)`;

  if (captureWarnings && warnings.length > 0) {
    summary += `, ${warnings.length} warning(s)`;
  }

  if (filterPattern) {
    summary += ` (filtered by: ${filterPattern.source})`;
  }

  return {
    success,
    errors,
    warnings,
    buildTime,
    summary
  };
}

/**
 * Quick build with error-only output (no warnings)
 */
export function buildQuick(buildDir: string, target: string = ''): BuildResult {
  return buildProject({
    sourceDir: path.dirname(buildDir),  // Assume parent dir is source
    buildDir,
    target,
    captureWarnings: false
  });
}

/**
 * Build with comprehensive error and warning capture
 */
export function buildVerbose(
  sourceDir: string,
  buildDir: string,
  target: string = ''
): BuildResult {
  return buildProject({
    sourceDir,
    buildDir,
    target,
    captureWarnings: true
  });
}

// ============================================================================
// CLI Interface (if run directly)
// ============================================================================

if (require.main === module) {
  const args = process.argv.slice(2);

  if (args.length < 2) {
    console.error('Usage: ts-node build-server.ts <sourceDir> <buildDir> [target] [--warnings]');
    console.error('');
    console.error('Example:');
    console.error('  ts-node build-server.ts /path/to/GigaMesh /path/to/GigaMesh/build DongArch3D --warnings');
    process.exit(1);
  }

  const sourceDir = args[0];
  const buildDir = args[1];
  const target = args[2] && !args[2].startsWith('--') ? args[2] : '';
  const captureWarnings = args.includes('--warnings');

  try {
    const result = buildProject({
      sourceDir,
      buildDir,
      target,
      captureWarnings
    });

    console.log('\n' + '='.repeat(80));
    console.log('BUILD RESULT');
    console.log('='.repeat(80));
    console.log(result.summary);
    console.log('');

    if (result.errors.length > 0) {
      console.log('ERRORS:');
      result.errors.forEach((err, idx) => {
        console.log(`  ${idx + 1}. ${err.file}:${err.line}:${err.column || 0}`);
        console.log(`     ${err.message}`);
        if (err.context) {
          console.log(`     ${err.context}`);
        }
      });
      console.log('');
    }

    if (result.warnings.length > 0) {
      console.log('WARNINGS:');
      result.warnings.forEach((warn, idx) => {
        console.log(`  ${idx + 1}. ${warn.file}:${warn.line}`);
        console.log(`     ${warn.message}`);
      });
      console.log('');
    }

    console.log('='.repeat(80));

    // Exit with appropriate code
    process.exit(result.success ? 0 : 1);

  } catch (error) {
    console.error('Build server error:', error);
    process.exit(1);
  }
}
