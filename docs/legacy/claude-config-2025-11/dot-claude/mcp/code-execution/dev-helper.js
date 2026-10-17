#!/usr/bin/env node

/**
 * MCP Code Execution - Development Helper
 *
 * 개발 중 빠르게 MCP 도구를 CLI로 사용할 수 있는 헬퍼
 */

import { buildProject } from './dist/src/tools/build.js';
import { runTests } from './dist/src/tools/test.js';
import { analyzeCode } from './dist/src/tools/analyze.js';

const BUILD_DIR = '/media/kwon/새 볼륨/1105/GigaMesh/build';
const SOURCE_DIR = '/media/kwon/새 볼륨/1105/GigaMesh';

const commands = {
  build: async () => {
    console.log('🔨 Building DongArch3D...\n');
    const result = await buildProject({
      buildDir: BUILD_DIR,
      target: 'DongArch3D',
      jobs: 4
    });
    console.log(result.summary);
    console.log(`\n✓ Errors: ${result.errors.length}, Warnings: ${result.warnings.length}`);
    return result.exitCode;
  },

  test: async () => {
    console.log('🧪 Running tests...\n');
    const result = await runTests({
      buildDir: BUILD_DIR,
      verbose: true
    });
    console.log(result.summary);
    console.log(`\n✓ Passed: ${result.passed}/${result.totalTests}`);
    return result.exitCode;
  },

  analyze: async () => {
    console.log('🔍 Analyzing code...\n');
    const result = await analyzeCode({
      sourceDir: SOURCE_DIR,
      tool: 'cppcheck',
      minSeverity: 'warning',
      filePattern: '**/*.cpp'
    });
    console.log(result.summary);
    console.log(`\n✓ Files: ${result.filesAnalyzed}, Issues: ${result.issuesFound}`);
    return 0;
  },

  help: () => {
    console.log('MCP Code Execution - Development Helper\n');
    console.log('Usage: node dev-helper.js <command>');
    console.log('\nCommands:');
    console.log('  build    - Build DongArch3D with error filtering');
    console.log('  test     - Run CTest with failure details');
    console.log('  analyze  - Run static code analysis');
    console.log('  help     - Show this help\n');
    return 0;
  }
};

async function main() {
  const command = process.argv[2] || 'help';

  if (!commands[command]) {
    console.error(`Unknown command: ${command}\n`);
    return commands.help();
  }

  try {
    const exitCode = await commands[command]();
    process.exit(exitCode);
  } catch (error) {
    console.error('\n❌ Error:', error.message);
    process.exit(1);
  }
}

main();
