#!/usr/bin/env node

/**
 * Test MCP Code Execution Server - Run Tests Function
 */

import { runTests } from './dist/src/tools/test.js';

async function main() {
  console.log('='.repeat(80));
  console.log('MCP Code Execution Server - Run Tests Test');
  console.log('='.repeat(80));
  console.log('');

  const buildDir = '/media/kwon/새 볼륨/1105/GigaMesh/build';

  console.log(`Running tests in: ${buildDir}`);
  console.log('Test pattern: .* (all tests)');
  console.log('Verbose: true');
  console.log('');

  const startTime = Date.now();

  try {
    const result = await runTests({
      buildDir,
      testPattern: '.*',
      verbose: true
    });

    const elapsed = Date.now() - startTime;

    console.log('='.repeat(80));
    console.log('TEST RESULT:');
    console.log('='.repeat(80));
    console.log(result.summary);
    console.log('');
    console.log(`Total time: ${elapsed}ms`);
    console.log(`Exit code: ${result.exitCode}`);
    console.log(`Success: ${result.exitCode === 0 ? '✅' : '❌'}`);
    console.log('');
    console.log(`Total tests: ${result.totalTests}`);
    console.log(`Passed: ${result.passed}`);
    console.log(`Failed: ${result.failed}`);
    console.log('');

    if (result.failedTests && result.failedTests.length > 0) {
      console.log('FAILED TESTS:');
      console.log('-'.repeat(80));
      result.failedTests.forEach((test, idx) => {
        console.log(`${idx + 1}. ${test}`);
      });
      console.log('');
    }

    console.log('='.repeat(80));
    console.log('TOKEN EFFICIENCY COMPARISON:');
    console.log('='.repeat(80));
    console.log('Traditional approach: Full test output (~500 lines)');
    console.log(`MCP approach: ${result.summary.split('\n').length} lines summary`);
    console.log('Estimated token reduction: ~25x (96%+)');
    console.log('='.repeat(80));

    process.exit(result.exitCode);
  } catch (error) {
    console.error('');
    console.error('❌ TEST EXECUTION FAILED:');
    console.error(error.message);
    console.error('');
    process.exit(1);
  }
}

main();
