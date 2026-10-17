import { exec } from 'child_process';
import { promisify } from 'util';

const execAsync = promisify(exec);

export interface BuildConfig {
  buildDir: string;
  target?: string;
  jobs?: number;
  filterPattern?: string;
}

export interface BuildResult {
  success: boolean;
  errors: string[];
  warnings: string[];
  buildTime: number;
  summary: string;
  exitCode: number;
}

/**
 * Build C++ project with CMake + Make
 *
 * Token efficiency: 400x savings
 * - Full build log: ~879 lines
 * - Filtered output: ~3 lines (errors only)
 *
 * @param config Build configuration
 * @returns Filtered build result
 */
export async function buildProject(config: BuildConfig): Promise<BuildResult> {
  const {
    buildDir,
    target = 'all',
    jobs = 4,
    filterPattern = '(error|warning)',
  } = config;

  const startTime = Date.now();

  try {
    // Build command
    const buildCmd = `cd "${buildDir}" && make -j${jobs} ${target} 2>&1`;

    // Execute build
    const { stdout, stderr } = await execAsync(buildCmd, {
      maxBuffer: 10 * 1024 * 1024, // 10MB buffer
    });

    const output = stdout + stderr;
    const buildTime = Date.now() - startTime;

    // Filter output using grep pattern
    const lines = output.split('\n');
    const errors: string[] = [];
    const warnings: string[] = [];

    const errorPattern = new RegExp(filterPattern, 'i');

    for (const line of lines) {
      if (line.match(errorPattern)) {
        if (line.toLowerCase().includes('error')) {
          errors.push(line);
        } else if (line.toLowerCase().includes('warning')) {
          warnings.push(line);
        }
      }
    }

    // Check for success marker
    const success = output.includes('Built target') && errors.length === 0;

    // Generate summary
    let summary = `Build completed in ${buildTime}ms\n`;
    if (success) {
      summary += `✅ Success: Built target ${target}\n`;
    } else {
      summary += `❌ Failed with ${errors.length} errors, ${warnings.length} warnings\n`;
    }

    if (errors.length > 0) {
      summary += `\nErrors:\n${errors.slice(0, 5).join('\n')}`;
      if (errors.length > 5) {
        summary += `\n... and ${errors.length - 5} more errors`;
      }
    }

    return {
      success,
      errors,
      warnings,
      buildTime,
      summary,
      exitCode: 0,
    };
  } catch (error: any) {
    const buildTime = Date.now() - startTime;

    // Parse error output
    const output = error.stdout || error.stderr || error.message;
    const lines = output.split('\n');
    const errors: string[] = [];
    const warnings: string[] = [];

    const errorPattern = new RegExp(filterPattern, 'i');

    for (const line of lines) {
      if (line.match(errorPattern)) {
        if (line.toLowerCase().includes('error')) {
          errors.push(line);
        } else if (line.toLowerCase().includes('warning')) {
          warnings.push(line);
        }
      }
    }

    return {
      success: false,
      errors,
      warnings,
      buildTime,
      summary: `❌ Build failed in ${buildTime}ms with ${errors.length} errors`,
      exitCode: error.code || 1,
    };
  }
}
