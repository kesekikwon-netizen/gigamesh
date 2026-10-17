#!/usr/bin/env node

/**
 * Test MCP Code Execution Server - Build Function
 */

import { buildProject } from './dist/src/tools/build.js';

async function main() {
  console.log('='.repeat(80));
  console.log('MCP Code Execution Server - Build Test');
  console.log('='.repeat(80));
  console.log('');

  const buildDir = '/media/kwon/새 볼륨/1105/GigaMesh/build';

  console.log(`Building GigaMesh project in: ${buildDir}`);
  console.log('Target: DongArch3D');
  console.log('Jobs: 4');
  console.log('');

  const startTime = Date.now();

  try {
    const result = await buildProject({
      buildDir,
      target: 'DongArch3D',
      jobs: 4,
      filterPattern: '(error|warning)'
    });

    const elapsed = Date.now() - startTime;

    console.log('='.repeat(80));
    console.log('BUILD RESULT:');
    console.log('='.repeat(80));
    console.log(result.summary);
    console.log('');
    console.log(`Total time: ${elapsed}ms`);
    console.log(`Build time reported: ${result.buildTime}ms`);
    console.log(`Exit code: ${result.exitCode}`);
    console.log(`Success: ${result.success ? '✅' : '❌'}`);
    console.log('');
    console.log(`Errors: ${result.errors.length}`);
    console.log(`Warnings: ${result.warnings.length}`);
    console.log('');

    if (result.errors.length > 0) {
      console.log('ERRORS:');
      console.log('-'.repeat(80));
      result.errors.slice(0, 10).forEach((err, idx) => {
        console.log(`${idx + 1}. ${err}`);
      });
      if (result.errors.length > 10) {
        console.log(`... and ${result.errors.length - 10} more errors`);
      }
      console.log('');
    }

    if (result.warnings.length > 0 && result.warnings.length <= 10) {
      console.log('WARNINGS:');
      console.log('-'.repeat(80));
      result.warnings.forEach((warn, idx) => {
        console.log(`${idx + 1}. ${warn}`);
      });
      console.log('');
    }

    console.log('='.repeat(80));
    console.log('TOKEN EFFICIENCY COMPARISON:');
    console.log('='.repeat(80));
    console.log('Traditional approach: Full build log (~879 lines, ~50KB)');
    console.log(`MCP approach: ${result.summary.split('\n').length} lines summary`);
    console.log('Estimated token reduction: ~400x (98%+)');
    console.log('='.repeat(80));

    process.exit(result.success ? 0 : 1);
  } catch (error) {
    console.error('');
    console.error('❌ BUILD FAILED:');
    console.error(error.message);
    console.error('');
    process.exit(1);
  }
}

main();
