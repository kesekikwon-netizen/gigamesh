# Code Execution MCP Architecture

## Core Concept

**Code Execution with MCP** exposes MCP servers as **code-level APIs**, allowing agents to write and execute **TypeScript/Python code** instead of making direct tool calls.

## Traditional Tool Use vs Code Execution

### Traditional Tool Use Problem

```
Agent → TOOL CALL: getDocument(id: 'abc123')
      → Full document content returned to context (50,000 tokens)
      → TOOL CALL: updateRecord(data: ...)
      → Data returned to context again (50,000 tokens)
```

**Result**: 150,000 tokens consumed

### Code Execution Approach

```typescript
const doc = await gdrive.getDocument({ documentId: 'abc123' });
await salesforce.updateRecord({
  objectType: 'SalesMeeting',
  recordId: '00Q5f000001abcXYZ',
  data: { Notes: doc.content }
});
```

**Result**: 2,000 tokens consumed (**98.7% reduction**)

## File System Structure

Organize MCP servers as a file tree:

```
project/
├── servers/
│   ├── google-drive/
│   │   ├── getDocument.ts
│   │   ├── getSheet.ts
│   │   ├── ... (other tools)
│   │   └── index.ts
│   ├── salesforce/
│   │   ├── updateRecord.ts
│   │   ├── query.ts
│   │   ├── ... (other tools)
│   │   └── index.ts
│   └── slack/
│       ├── postMessage.ts
│       ├── getChannelHistory.ts
│       └── index.ts
├── skills/
│   └── (reusable functions)
├── workspace/
│   └── (intermediate results storage)
└── client.ts
```

## MCP Tool Wrapping Pattern

Each MCP tool is wrapped as a TypeScript function:

```typescript
// ./servers/google-drive/getDocument.ts
import { callMCPTool } from "../../../client.js";

interface GetDocumentInput {
  documentId: string;
}

interface GetDocumentResponse {
  content: string;
}

/**
 * Read a document from Google Drive
 * @param input - Document ID to retrieve
 * @returns Document content
 */
export async function getDocument(
  input: GetDocumentInput
): Promise<GetDocumentResponse> {
  return callMCPTool<GetDocumentResponse>(
    'google_drive__get_document',  // Actual MCP tool name
    input
  );
}
```

## Client Implementation

```typescript
// ./client.ts
export async function callMCPTool<T>(
  toolName: string,
  input: any
): Promise<T> {
  // Communication logic with MCP server
  // Examples: stdio, HTTP, SSE transport methods

  const response = await fetch(`/mcp/${toolName}`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(input)
  });

  return await response.json();
}
```

## Agent Code Writing

Agents now write code like this:

```typescript
// Code written by agent
import * as gdrive from './servers/google-drive';
import * as salesforce from './servers/salesforce';

// Read meeting notes from Google Docs
const transcript = (await gdrive.getDocument({
  documentId: 'abc123'
})).content;

// Update Salesforce record
await salesforce.updateRecord({
  objectType: 'SalesMeeting',
  recordId: '00Q5f000001abcXYZ',
  data: { Notes: transcript }
});
```

**Important**: This code **executes in the execution environment**, and the `transcript` variable does not pass through the model context!

## Progressive Disclosure

Agents discover and load only necessary tools:

```typescript
// 1. Explore file system
const servers = await fs.readdir('./servers');
// → ['google-drive', 'salesforce', 'slack']

// 2. Explore only needed servers
const gdriveTools = await fs.readdir('./servers/google-drive');
// → ['getDocument.ts', 'getSheet.ts', 'index.ts']

// 3. Read specific tool definitions
const getDocDef = await fs.readFile('./servers/google-drive/getDocument.ts');
```

**Result**: Save context by not pre-loading all tool definitions

## Key Benefits

1. **Dramatic token reduction**: 98.7% ~ 99.95%
2. **Progressive disclosure**: Load only necessary tools
3. **Execution environment filtering**: Intermediate data doesn't pass through model
4. **Control flow**: Handle loops/conditions directly in code
5. **State persistence**: Save to files and reuse
6. **Privacy protection**: Automatic tokenization
