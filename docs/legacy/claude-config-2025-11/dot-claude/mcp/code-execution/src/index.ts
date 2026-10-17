#!/usr/bin/env node

/**
 * MCP Code Execution Server
 *
 * Provides tools for efficient C++ build/test/analysis automation
 * with extreme token efficiency (400x savings on builds).
 *
 * Tools:
 * - build_project: CMake + Make with error filtering
 * - run_tests: CTest execution with pattern matching
 * - analyze_code: clang-tidy/cppcheck integration
 */

import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import { buildProject } from './tools/build.js';
import { runTests } from './tools/test.js';
import { analyzeCode } from './tools/analyze.js';

const server = new Server(
  {
    name: 'code-execution',
    version: '1.0.0',
  },
  {
    capabilities: {
      tools: {},
    },
  }
);

// Tool definitions
server.setRequestHandler(ListToolsRequestSchema, async () => {
  return {
    tools: [
      {
        name: 'build_project',
        description:
          'Build C++ project using CMake + Make with intelligent error filtering. ' +
          'Returns only errors/warnings/summary (400x token savings). ' +
          'Example: build_project with buildDir="/path/to/build", target="all", jobs=4',
        inputSchema: {
          type: 'object',
          properties: {
            buildDir: {
              type: 'string',
              description: 'Build directory path (e.g., /media/kwon/새 볼륨/1105/GigaMesh/build)',
            },
            target: {
              type: 'string',
              description: 'Build target (default: "all")',
              default: 'all',
            },
            jobs: {
              type: 'number',
              description: 'Number of parallel jobs (default: 4)',
              default: 4,
            },
            filterPattern: {
              type: 'string',
              description: 'Grep pattern for filtering output (default: "(error|warning)")',
              default: '(error|warning)',
            },
          },
          required: ['buildDir'],
        },
      },
      {
        name: 'run_tests',
        description:
          'Run CTest with pattern filtering. Returns only failed tests or summary. ' +
          'Supports test name patterns and verbose mode for failures.',
        inputSchema: {
          type: 'object',
          properties: {
            buildDir: {
              type: 'string',
              description: 'Build directory path',
            },
            testPattern: {
              type: 'string',
              description: 'Test name pattern (regex)',
              default: '.*',
            },
            verbose: {
              type: 'boolean',
              description: 'Verbose output for failed tests',
              default: true,
            },
          },
          required: ['buildDir'],
        },
      },
      {
        name: 'analyze_code',
        description:
          'Run static analysis using clang-tidy or cppcheck. ' +
          'Returns only issues with specified severity or higher.',
        inputSchema: {
          type: 'object',
          properties: {
            sourceDir: {
              type: 'string',
              description: 'Source directory path',
            },
            tool: {
              type: 'string',
              enum: ['clang-tidy', 'cppcheck'],
              description: 'Analysis tool to use',
              default: 'clang-tidy',
            },
            minSeverity: {
              type: 'string',
              enum: ['error', 'warning', 'info'],
              description: 'Minimum severity level',
              default: 'warning',
            },
            filePattern: {
              type: 'string',
              description: 'File pattern to analyze (e.g., "**/*.cpp")',
              default: '**/*.cpp',
            },
          },
          required: ['sourceDir'],
        },
      },
    ],
  };
});

// Tool execution
server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  try {
    switch (name) {
      case 'build_project': {
        const result = await buildProject(args as any);
        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify(result, null, 2),
            },
          ],
        };
      }

      case 'run_tests': {
        const result = await runTests(args as any);
        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify(result, null, 2),
            },
          ],
        };
      }

      case 'analyze_code': {
        const result = await analyzeCode(args as any);
        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify(result, null, 2),
            },
          ],
        };
      }

      default:
        throw new Error(`Unknown tool: ${name}`);
    }
  } catch (error) {
    const errorMessage = error instanceof Error ? error.message : String(error);
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({ error: errorMessage }, null, 2),
        },
      ],
      isError: true,
    };
  }
});

// Start server
async function main() {
  const transport = new StdioServerTransport();
  await server.connect(transport);
  console.error('MCP Code Execution Server running on stdio');
}

main().catch((error) => {
  console.error('Fatal error:', error);
  process.exit(1);
});
