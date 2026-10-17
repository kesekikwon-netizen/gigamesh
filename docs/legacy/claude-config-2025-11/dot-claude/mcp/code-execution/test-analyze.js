#!/usr/bin/env node

/**
 * Test MCP Code Execution Server - Analyze Code Function
 */

import { analyzeCode } from './dist/src/tools/analyze.js';

async function main() {
  console.log('='.repeat(80));
  console.log('MCP Code Execution Server - Code Analysis Test');
  console.log('='.repeat(80));
  console.log('');

  const sourceDir = '/media/kwon/새 볼륨/1105/GigaMesh';

  console.log(`Analyzing code in: ${sourceDir}`);
  console.log('Tool: cppcheck (more reliable without compile commands)');
  console.log('Min severity: warning');
  console.log('File pattern: **/*.cpp');
  console.log('');

  const startTime = Date.now();

  try {
    const result = await analyzeCode({
      sourceDir,
      tool: 'cppcheck',
      minSeverity: 'warning',
      filePattern: '**/*.cpp'
    });

    const elapsed = Date.now() - startTime;

    console.log('='.repeat(80));
    console.log('ANALYSIS RESULT:');
    console.log('='.repeat(80));
    console.log(result.summary);
    console.log('');
    console.log(`Total time: ${elapsed}ms`);
    console.log(`Tool: ${result.tool}`);
    console.log(`Files analyzed: ${result.filesAnalyzed}`);
    console.log(`Issues found: ${result.issuesFound}`);
    console.log('');

    if (result.issues.length > 0) {
      console.log('DETAILED ISSUES (first 10):');
      console.log('-'.repeat(80));
      result.issues.slice(0, 10).forEach((issue, idx) => {
        console.log(`${idx + 1}. ${issue.file}:${issue.line}`);
        console.log(`   [${issue.severity.toUpperCase()}] ${issue.message}`);
      });
      if (result.issues.length > 10) {
        console.log(`... and ${result.issues.length - 10} more issues`);
      }
      console.log('');
    }

    console.log('='.repeat(80));
    console.log('TOKEN EFFICIENCY COMPARISON:');
    console.log('='.repeat(80));
    console.log('Traditional approach: Full analysis output (~1000 lines)');
    console.log(`MCP approach: ${result.summary.split('\n').length} lines summary + filtered issues`);
    console.log('Estimated token reduction: ~10x (90%+)');
    console.log('='.repeat(80));

    process.exit(0);
  } catch (error) {
    console.error('');
    console.error('❌ ANALYSIS FAILED:');
    console.error(error.message);
    console.error('');
    process.exit(1);
  }
}

main();
