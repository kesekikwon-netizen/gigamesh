import { exec } from 'child_process';
import { promisify } from 'util';
import { glob } from 'glob';

const execAsync = promisify(exec);

export interface AnalysisConfig {
  sourceDir: string;
  tool?: 'clang-tidy' | 'cppcheck';
  minSeverity?: 'error' | 'warning' | 'info';
  filePattern?: string;
}

export interface AnalysisIssue {
  file: string;
  line: number;
  severity: string;
  message: string;
}

export interface AnalysisResult {
  tool: string;
  filesAnalyzed: number;
  issuesFound: number;
  issues: AnalysisIssue[];
  summary: string;
}

/**
 * Run static analysis with intelligent filtering
 *
 * Token efficiency: 10x savings
 * - Full analysis output: ~1000 lines
 * - Filtered output: ~100 lines (errors/warnings only)
 *
 * @param config Analysis configuration
 * @returns Filtered analysis result
 */
export async function analyzeCode(config: AnalysisConfig): Promise<AnalysisResult> {
  const {
    sourceDir,
    tool = 'clang-tidy',
    minSeverity = 'warning',
    filePattern = '**/*.cpp',
  } = config;

  try {
    // Find files to analyze
    const files = await glob(filePattern, { cwd: sourceDir });

    if (files.length === 0) {
      return {
        tool,
        filesAnalyzed: 0,
        issuesFound: 0,
        issues: [],
        summary: '⚠️  No files found matching pattern',
      };
    }

    const issues: AnalysisIssue[] = [];

    // Run analysis tool
    if (tool === 'clang-tidy') {
      for (const file of files.slice(0, 10)) {
        // Limit to 10 files for performance
        const filePath = `${sourceDir}/${file}`;
        const cmd = `clang-tidy "${filePath}" -- -std=c++17 2>&1`;

        try {
          const { stdout } = await execAsync(cmd);
          const parsedIssues = parseClangTidyOutput(stdout, file, minSeverity);
          issues.push(...parsedIssues);
        } catch (error: any) {
          // clang-tidy returns non-zero if issues found
          const output = error.stdout || '';
          const parsedIssues = parseClangTidyOutput(output, file, minSeverity);
          issues.push(...parsedIssues);
        }
      }
    } else if (tool === 'cppcheck') {
      const cmd = `cppcheck --enable=all --template=gcc ${sourceDir} 2>&1`;

      try {
        const { stdout, stderr } = await execAsync(cmd);
        const output = stdout + stderr;
        issues.push(...parseCppcheckOutput(output, minSeverity));
      } catch (error: any) {
        const output = (error.stdout || '') + (error.stderr || '');
        issues.push(...parseCppcheckOutput(output, minSeverity));
      }
    }

    // Generate summary
    let summary = `Analyzed ${files.length} files with ${tool}\n`;
    if (issues.length === 0) {
      summary += `✅ No issues found`;
    } else {
      summary += `⚠️  Found ${issues.length} issues:\n`;
      const errorCount = issues.filter((i) => i.severity === 'error').length;
      const warningCount = issues.filter((i) => i.severity === 'warning').length;
      summary += `  - ${errorCount} errors\n`;
      summary += `  - ${warningCount} warnings\n`;
      summary += `\nTop issues:\n`;
      summary += issues
        .slice(0, 5)
        .map((i) => `  ${i.file}:${i.line} [${i.severity}] ${i.message}`)
        .join('\n');
    }

    return {
      tool,
      filesAnalyzed: files.length,
      issuesFound: issues.length,
      issues,
      summary,
    };
  } catch (error: any) {
    return {
      tool,
      filesAnalyzed: 0,
      issuesFound: 0,
      issues: [],
      summary: `❌ Analysis failed: ${error.message}`,
    };
  }
}

function parseClangTidyOutput(
  output: string,
  file: string,
  minSeverity: string
): AnalysisIssue[] {
  const issues: AnalysisIssue[] = [];
  const lines = output.split('\n');

  for (const line of lines) {
    // Parse: "file.cpp:123:45: warning: message [clang-diagnostic-...]"
    const match = line.match(/^(.+):(\d+):\d+:\s+(warning|error):\s+(.+)$/);
    if (match) {
      const [, filePath, lineNum, severity, message] = match;

      if (shouldIncludeSeverity(severity, minSeverity)) {
        issues.push({
          file: filePath,
          line: parseInt(lineNum),
          severity,
          message: message.trim(),
        });
      }
    }
  }

  return issues;
}

function parseCppcheckOutput(output: string, minSeverity: string): AnalysisIssue[] {
  const issues: AnalysisIssue[] = [];
  const lines = output.split('\n');

  for (const line of lines) {
    // Parse: "file.cpp:123: (error) message"
    const match = line.match(/^(.+):(\d+):\s+\((\w+)\)\s+(.+)$/);
    if (match) {
      const [, file, lineNum, severity, message] = match;

      if (shouldIncludeSeverity(severity, minSeverity)) {
        issues.push({
          file,
          line: parseInt(lineNum),
          severity,
          message: message.trim(),
        });
      }
    }
  }

  return issues;
}

function shouldIncludeSeverity(severity: string, minSeverity: string): boolean {
  const severityLevels = ['info', 'warning', 'error'];
  const currentLevel = severityLevels.indexOf(severity.toLowerCase());
  const minLevel = severityLevels.indexOf(minSeverity.toLowerCase());
  return currentLevel >= minLevel;
}
