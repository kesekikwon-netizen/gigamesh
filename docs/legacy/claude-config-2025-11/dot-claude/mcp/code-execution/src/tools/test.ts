import { exec } from 'child_process';
import { promisify } from 'util';

const execAsync = promisify(exec);

export interface TestConfig {
  buildDir: string;
  testPattern?: string;
  verbose?: boolean;
}

export interface TestResult {
  totalTests: number;
  passed: number;
  failed: number;
  failedTests: string[];
  summary: string;
  exitCode: number;
}

/**
 * Run CTest with intelligent filtering
 *
 * Token efficiency: 25x savings
 * - Full test log: ~500 lines
 * - Filtered output: ~20 lines (failures only)
 *
 * @param config Test configuration
 * @returns Filtered test result
 */
export async function runTests(config: TestConfig): Promise<TestResult> {
  const { buildDir, testPattern = '.*', verbose = true } = config;

  try {
    // CTest command
    const verboseFlag = verbose ? '--output-on-failure' : '';
    const testCmd = `cd "${buildDir}" && ctest -R "${testPattern}" ${verboseFlag} 2>&1`;

    // Execute tests
    const { stdout, stderr } = await execAsync(testCmd, {
      maxBuffer: 10 * 1024 * 1024, // 10MB buffer
    });

    const output = stdout + stderr;

    // Parse test results
    let totalTests = 0;
    let passed = 0;
    let failed = 0;
    const failedTests: string[] = [];

    const lines = output.split('\n');

    for (const line of lines) {
      // Parse summary line: "100% tests passed, 0 tests failed out of 10"
      const summaryMatch = line.match(/(\d+)% tests passed, (\d+) tests failed out of (\d+)/);
      if (summaryMatch) {
        failed = parseInt(summaryMatch[2]);
        totalTests = parseInt(summaryMatch[3]);
        passed = totalTests - failed;
      }

      // Parse failed test names
      const failMatch = line.match(/^\s*\d+\s+-\s+(\S+)\s+\(Failed\)/);
      if (failMatch) {
        failedTests.push(failMatch[1]);
      }
    }

    // Generate summary
    let summary = `Tests completed: ${passed}/${totalTests} passed\n`;
    if (failed === 0) {
      summary += `✅ All tests passed`;
    } else {
      summary += `❌ ${failed} tests failed:\n`;
      summary += failedTests.map((t) => `  - ${t}`).join('\n');
    }

    return {
      totalTests,
      passed,
      failed,
      failedTests,
      summary,
      exitCode: failed > 0 ? 1 : 0,
    };
  } catch (error: any) {
    // CTest returns non-zero exit code if tests fail
    const output = error.stdout || error.stderr || '';
    const lines = output.split('\n');

    let totalTests = 0;
    let passed = 0;
    let failed = 0;
    const failedTests: string[] = [];

    for (const line of lines) {
      const summaryMatch = line.match(/(\d+)% tests passed, (\d+) tests failed out of (\d+)/);
      if (summaryMatch) {
        failed = parseInt(summaryMatch[2]);
        totalTests = parseInt(summaryMatch[3]);
        passed = totalTests - failed;
      }

      const failMatch = line.match(/^\s*\d+\s+-\s+(\S+)\s+\(Failed\)/);
      if (failMatch) {
        failedTests.push(failMatch[1]);
      }
    }

    return {
      totalTests,
      passed,
      failed,
      failedTests,
      summary: `❌ ${failed}/${totalTests} tests failed`,
      exitCode: error.code || 1,
    };
  }
}
