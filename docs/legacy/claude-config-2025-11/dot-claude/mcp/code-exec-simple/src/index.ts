#!/usr/bin/env node

/**
 * Simple Code Execution MCP Server
 * Executes JavaScript code in a sandboxed environment
 */

import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import * as vm from 'vm';
import * as util from 'util';

const server = new Server(
  {
    name: 'code-exec-simple',
    version: '1.0.0',
  },
  {
    capabilities: {
      tools: {},
    },
  }
);

// Execute code in a sandbox
function executeCode(code: string, context: Record<string, any> = {}, timeout: number = 5000): any {
  const sandbox = {
    console: {
      log: (...args: any[]) => console.error('[sandbox]', ...args),
    },
    Math,
    JSON,
    Date,
    Array,
    Object,
    String,
    Number,
    Boolean,
    Promise,
    setTimeout,
    setInterval,
    clearTimeout,
    clearInterval,
    ...context,
  };

  const script = new vm.Script(`(async () => { ${code} })()`, {
    filename: 'sandbox.js',
  });

  const vmContext = vm.createContext(sandbox);

  return script.runInContext(vmContext, {
    timeout,
    displayErrors: true,
  });
}

// Tool definitions
server.setRequestHandler(ListToolsRequestSchema, async () => {
  return {
    tools: [
      {
        name: 'execute_code',
        description: `Execute JavaScript code in a secure sandbox environment.

Features:
- Sandboxed execution with timeout (default: 5 seconds)
- Access to Math, JSON, Date, Array, Object, String, Number, Boolean
- Support for async/await operations
- Custom context variables

Parameters:
- code: JavaScript code to execute (string)
- context: Optional variables to inject into execution context (object)
- timeout: Execution timeout in milliseconds (number, default: 5000)

Example:
{
  "code": "const sum = numbers.reduce((a, b) => a + b, 0); return sum;",
  "context": {"numbers": [1, 2, 3, 4, 5]},
  "timeout": 5000
}

Returns: The result of code execution or error message.`,
        inputSchema: {
          type: 'object',
          properties: {
            code: {
              type: 'string',
              description: 'JavaScript code to execute',
            },
            context: {
              type: 'object',
              description: 'Variables to inject into execution context',
              additionalProperties: true,
            },
            timeout: {
              type: 'number',
              description: 'Execution timeout in milliseconds (default: 5000)',
              default: 5000,
            },
          },
          required: ['code'],
        },
      },
    ],
  };
});

// Tool execution
server.setRequestHandler(CallToolRequestSchema, async (request) => {
  if (request.params.name !== 'execute_code') {
    throw new Error(`Unknown tool: ${request.params.name}`);
  }

  const { code, context = {}, timeout = 5000 } = request.params.arguments as {
    code: string;
    context?: Record<string, any>;
    timeout?: number;
  };

  try {
    const startTime = Date.now();
    const result = await executeCode(code, context, timeout);
    const executionTime = Date.now() - startTime;

    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({
            success: true,
            result: result,
            executionTime: `${executionTime}ms`,
          }, null, 2),
        },
      ],
    };
  } catch (error) {
    const errorMessage = error instanceof Error ? error.message : String(error);
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({
            success: false,
            error: errorMessage,
          }, null, 2),
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
  console.error('✅ Simple Code Execution MCP Server running on stdio');
}

main().catch((error) => {
  console.error('Fatal error:', error);
  process.exit(1);
});
