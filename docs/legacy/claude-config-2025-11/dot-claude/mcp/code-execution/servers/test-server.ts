#!/usr/bin/env ts-node
/**
 * MCP Test Server - CTest execution with intelligent result filtering
 *
 * Features:
 * - Run all tests or filter by pattern
 * - Capture test output (stdout/stderr)
 * - Extract performance timing data
 * - Compare results across runs
 * - Generate structured JSON output
 */

import { execSync } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';

// ============================================================================
// Type Definitions
// ============================================================================

export interface TestConfig {
  buildDir: string;
  testPattern?: string;               // Regex pattern to filter tests (CTest -R)
  captureOutput?: boolean;            // Capture test stdout/stderr (default: false)
  timeoutSeconds?: number;            // Per-test timeout (default: 300)
  verbose?: boolean;                  // Verbose output (default: false)
  parallel?: number;                  // Number of parallel tests (default: 1)
}

export interface TestFailure {
  name: string;
  output: string;                     // stdout + stderr
  duration: number;                   // Milliseconds
  exitCode: number;
}

export interface TestResult {
  totalTests: number;
  passed: number;
  failed: TestFailure[];
  skipped: number;
  totalDuration: number;              // Milliseconds
  summary: string;
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Parse CTest output into structured TestResult
 *
 * CTest output format examples:
 * Test project /path/to/build
 *     Start 1: TestName
 * 1/5 Test #1: TestName .........................   Passed    0.12 sec
 *     Start 2: TestName2
 * 2/5 Test #2: TestName2 ........................***Failed    0.45 sec
 * ...
 * 100% tests passed, 0 tests failed out of 5
 * Total Test time (real) =   1.23 sec
 */
function parseCTestOutput(
  output: string,
  captureOutput: boolean
): TestResult {
  const lines = output.split('\n');

  let totalTests = 0;
  let passed = 0;
  const failed: TestFailure[] = [];
  let skipped = 0;
  let totalDuration = 0;

  // Regex patterns
  const testResultRegex = /^\d+\/\d+\s+Test\s+#\d+:\s+(.+?)\s+\.+(\*\*\*Failed|\*\*\*Skipped|Passed)\s+([\d.]+)\s+sec$/;
  const totalTestsRegex = /^(\d+)%\s+tests\s+passed,\s+(\d+)\s+tests\s+failed\s+out\s+of\s+(\d+)/;
  const totalTimeRegex = /^Total\s+Test\s+time\s+\(real\)\s+=\s+([\d.]+)\s+sec/;

  let currentTest: string | null = null;
  let currentOutput: string[] = [];

  for (const line of lines) {
    // Match test result line
    const resultMatch = line.match(testResultRegex);
    if (resultMatch) {
      const [, testName, status, durationStr] = resultMatch;
      const duration = parseFloat(durationStr) * 1000;  // Convert to ms

      if (status === 'Passed') {
        passed++;
      } else if (status === '***Failed') {
        failed.push({
          name: testName.trim(),
          output: captureOutput ? currentOutput.join('\n') : '',
          duration,
          exitCode: 1
        });
      } else if (status === '***Skipped') {
        skipped++;
      }

      currentTest = null;
      currentOutput = [];
      continue;
    }

    // Match summary line
    const totalMatch = line.match(totalTestsRegex);
    if (totalMatch) {
      const [, , failedCount, total] = totalMatch;
      totalTests = parseInt(total, 10);
      continue;
    }

    // Match total time
    const timeMatch = line.match(totalTimeRegex);
    if (timeMatch) {
      totalDuration = parseFloat(timeMatch[1]) * 1000;  // Convert to ms
      continue;
    }

    // Capture test output if enabled
    if (captureOutput && currentTest !== null) {
      currentOutput.push(line);
    }

    // Detect test start
    const startMatch = line.match(/^\s+Start\s+\d+:\s+(.+)$/);
    if (startMatch) {
      currentTest = startMatch[1].trim();
      currentOutput = [];
    }
  }

  // Generate summary
  const passRate = totalTests > 0 ? ((passed / totalTests) * 100).toFixed(1) : '0.0';
  const summary = `${passed}/${totalTests} tests passed (${passRate}%), ` +
                  `${failed.length} failed, ${skipped} skipped ` +
                  `in ${(totalDuration / 1000).toFixed(2)}s`;

  return {
    totalTests,
    passed,
    failed,
    skipped,
    totalDuration,
    summary
  };
}

// ============================================================================
// Main API
// ============================================================================

/**
 * Run CTest with intelligent result filtering
 *
 * Example:
 * ```typescript
 * const result = runTests({
 *   buildDir: '/path/to/GigaMesh/build',
 *   testPattern: 'Cutline.*',
 *   captureOutput: true,
 *   timeoutSeconds: 300
 * });
 *
 * console.log(result.summary);
 * result.failed.forEach(test => {
 *   console.log(`Failed: ${test.name}`);
 *   console.log(test.output);
 * });
 * ```
 */
export function runTests(config: TestConfig): TestResult {
  const {
    buildDir,
    testPattern,
    captureOutput = false,
    timeoutSeconds = 300,
    verbose = false,
    parallel = 1
  } = config;

  // Validate build directory
  if (!fs.existsSync(buildDir)) {
    throw new Error(`Build directory does not exist: ${buildDir}`);
  }

  const ctestPath = path.join(buildDir, 'CTestTestfile.cmake');
  if (!fs.existsSync(ctestPath)) {
    throw new Error(`No CTest configuration found in: ${buildDir}`);
  }

  // Build CTest command
  let cmd = 'ctest';

  if (testPattern) {
    cmd += ` -R "${testPattern}"`;
  }

  if (verbose) {
    cmd += ' -V';  // Verbose output
  } else if (captureOutput) {
    cmd += ' --output-on-failure';
  }

  if (parallel > 1) {
    cmd += ` -j${parallel}`;
  }

  cmd += ` --timeout ${timeoutSeconds}`;

  console.log(`[CTest] Running: ${cmd}`);
  console.log(`[CTest] Working directory: ${buildDir}`);

  let output = '';
  try {
    output = execSync(cmd, {
      cwd: buildDir,
      encoding: 'utf-8',
      timeout: timeoutSeconds * 1000 * 10,  // Total timeout (10x per-test)
      maxBuffer: 10 * 1024 * 1024  // 10MB buffer
    });
  } catch (error: any) {
    // CTest returns non-zero if any test fails, but we still want the output
    output = error.stdout || '';
  }

  // Parse output
  return parseCTestOutput(output, captureOutput);
}

/**
 * Run specific test suite
 */
export function runTestSuite(buildDir: string, suiteName: string): TestResult {
  return runTests({
    buildDir,
    testPattern: `^${suiteName}`,
    captureOutput: true
  });
}

/**
 * Run all tests with minimal output
 */
export function runAllTests(buildDir: string): TestResult {
  return runTests({
    buildDir,
    captureOutput: false
  });
}

/**
 * Run tests and extract performance data
 */
export function runPerformanceTests(buildDir: string, pattern: string): {
  result: TestResult;
  timings: Map<string, number>;
} {
  const result = runTests({
    buildDir,
    testPattern: pattern,
    captureOutput: true
  });

  // Extract timing data from test output
  const timings = new Map<string, number>();

  result.failed.forEach(test => {
    // Look for timing patterns in output (e.g., "Frame time: 16.7ms")
    const frameTimeMatch = test.output.match(/Frame\s+time:\s+([\d.]+)\s*ms/i);
    if (frameTimeMatch) {
      timings.set(test.name, parseFloat(frameTimeMatch[1]));
    }
  });

  return { result, timings };
}

// ============================================================================
// CLI Interface (if run directly)
// ============================================================================

if (require.main === module) {
  const args = process.argv.slice(2);

  if (args.length < 1) {
    console.error('Usage: ts-node test-server.ts <buildDir> [testPattern] [--verbose] [--capture]');
    console.error('');
    console.error('Example:');
    console.error('  ts-node test-server.ts /path/to/GigaMesh/build "Cutline.*" --capture');
    process.exit(1);
  }

  const buildDir = args[0];
  const testPattern = args[1] && !args[1].startsWith('--') ? args[1] : undefined;
  const verbose = args.includes('--verbose');
  const captureOutput = args.includes('--capture');

  try {
    const result = runTests({
      buildDir,
      testPattern,
      verbose,
      captureOutput
    });

    console.log('\n' + '='.repeat(80));
    console.log('TEST RESULT');
    console.log('='.repeat(80));
    console.log(result.summary);
    console.log('');

    if (result.failed.length > 0) {
      console.log('FAILED TESTS:');
      result.failed.forEach((test, idx) => {
        console.log(`  ${idx + 1}. ${test.name} (${test.duration.toFixed(2)}ms)`);
        if (test.output) {
          console.log(`     Output:`);
          test.output.split('\n').slice(0, 10).forEach(line => {
            console.log(`       ${line}`);
          });
          if (test.output.split('\n').length > 10) {
            console.log(`       ... (${test.output.split('\n').length - 10} more lines)`);
          }
        }
      });
      console.log('');
    }

    if (result.passed > 0) {
      console.log(`PASSED: ${result.passed} test(s)`);
    }

    if (result.skipped > 0) {
      console.log(`SKIPPED: ${result.skipped} test(s)`);
    }

    console.log('='.repeat(80));

    // Exit with appropriate code
    process.exit(result.failed.length > 0 ? 1 : 0);

  } catch (error) {
    console.error('Test server error:', error);
    process.exit(1);
  }
}
