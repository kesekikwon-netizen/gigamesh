#!/usr/bin/env ts-node
/**
 * Example: Diagnose Build Errors
 *
 * This script demonstrates how to use the build server to quickly
 * identify and diagnose build errors without reading the entire build log.
 *
 * Usage:
 *   npx ts-node examples/diagnose-build-error.ts
 */

import { buildProject } from '../servers/build-server';

// Configuration
const GIGAMESH_ROOT = '/media/kwon/새 볼륨/1105/GigaMesh';
const BUILD_DIR = `${GIGAMESH_ROOT}/build`;

async function main() {
  console.log('='.repeat(80));
  console.log('Diagnose Build Errors - DongArch3D');
  console.log('='.repeat(80));
  console.log('');

  // Build with filtering for DongArch-specific errors
  const result = buildProject({
    sourceDir: GIGAMESH_ROOT,
    buildDir: BUILD_DIR,
    target: 'DongArch3D',
    filterPattern: /dongarch|cutline|clip|align/i,  // Filter for DongArch modules
    captureWarnings: false,  // Only errors
    makeJobs: 4
  });

  // Display results
  console.log(result.summary);
  console.log('');

  if (result.errors.length > 0) {
    console.log('ERROR DETAILS:');
    console.log('-'.repeat(80));
    result.errors.forEach((error, idx) => {
      console.log(`\n${idx + 1}. ${error.file}:${error.line}:${error.column || 0}`);
      console.log(`   [${error.severity}] ${error.message}`);
      if (error.context) {
        console.log(`   Context: ${error.context}`);
      }
    });
    console.log('');

    // Provide suggestions based on error patterns
    console.log('SUGGESTED FIXES:');
    console.log('-'.repeat(80));

    result.errors.forEach((error, idx) => {
      if (error.message.includes('is not a member of')) {
        console.log(`${idx + 1}. Namespace issue - check using declarations and qualifiers`);
      } else if (error.message.includes('undefined reference')) {
        console.log(`${idx + 1}. Linker error - check CMakeLists.txt for missing source files`);
      } else if (error.message.includes('no matching function')) {
        console.log(`${idx + 1}. Function signature mismatch - check parameter types`);
      } else {
        console.log(`${idx + 1}. Review error message above`);
      }
    });
  } else {
    console.log('✓ No errors found in DongArch modules!');
  }

  console.log('');
  console.log('='.repeat(80));

  process.exit(result.success ? 0 : 1);
}

// Run
main().catch(error => {
  console.error('Fatal error:', error);
  process.exit(1);
});
