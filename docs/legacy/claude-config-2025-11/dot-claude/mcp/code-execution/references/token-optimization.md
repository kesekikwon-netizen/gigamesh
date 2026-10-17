# Token Optimization Mechanisms

## Measured Cases (from source article)

| Scenario | Traditional Tool Use | Code Execution | Reduction Rate |
|---------|---------------------|----------------|----------------|
| Meeting transcript processing | 150,000 tokens | 2,000 tokens | **98.7%** |
| Filtering 10,000 rows | 500,000 tokens | 250 tokens | **99.95%** |
| Iterative tasks | Evaluated each iteration | Executed once | **~99%** |

## Optimization Principles

### 1. On-demand Loading

Only read necessary tool definitions:

```typescript
// Instead of loading all tools upfront
// Load only when needed
const toolDef = await fs.readFile('./servers/google-drive/getDocument.ts');
```

### 2. Execution Environment Filtering

Intermediate data doesn't pass through model:

```typescript
// Fetch 10,000 rows but...
const allRows = await gdrive.getSheet({ sheetId: 'abc123' });

// Filter in execution environment (model doesn't see this)
const pendingOrders = allRows.filter(row =>
  row["Status"] === 'pending'
);

// Agent sees only 5 rows (passed through model)
console.log(`Found ${pendingOrders.length} pending orders`);
console.log(pendingOrders.slice(0, 5));
```

**Token savings**:
- Traditional: All 10,000 rows pass through model (~500,000 tokens)
- Code Execution: Only 5 rows pass through model (~250 tokens)
- **Reduction: 99.95%**

### 3. Control Flow

Loops and conditions execute directly in code:

```typescript
// Poll until deployment notification arrives
let found = false;
while (!found) {
  const messages = await slack.getChannelHistory({
    channel: 'C123456'
  });

  found = messages.some(m =>
    m.text.includes('deployment complete')
  );

  if (!found) {
    await new Promise(r => setTimeout(r, 5000));  // Wait 5 seconds
  }
}

console.log('Deployment notification received');
```

**Benefit**: Model doesn't need to evaluate each loop iteration. Code executes automatically in execution environment.

### 4. Automatic Tokenization

Model sees tokenized values:

```typescript
// Processing sensitive data
const sheet = await gdrive.getSheet({ sheetId: 'abc123' });

for (const row of sheet.rows) {
  await salesforce.updateRecord({
    objectType: 'Lead',
    recordId: row.salesforceId,
    data: {
      Email: row.email,      // john@example.com
      Phone: row.phone,      // 010-1234-5678
      Name: row.name         // Hong Gildong
    }
  });
}
```

**What model sees (automatically tokenized):**

```json
[
  {
    "salesforceId": "00Q...",
    "email": "[EMAIL_1]",
    "phone": "[PHONE_1]",
    "name": "[NAME_1]"
  },
  {
    "salesforceId": "00Q...",
    "email": "[EMAIL_2]",
    "phone": "[PHONE_2]",
    "name": "[NAME_2]"
  }
]
```

Actual values are **restored only during tool calls**, and model only sees tokenized values.

## When to Use Code Execution

- ✅ Processing large amounts of data (100+ items)
- ✅ Complex filtering/aggregation
- ✅ Combining multiple MCP tools
- ✅ Iterative tasks
- ✅ Processing sensitive data
- ❌ Simple single tool calls
- ❌ Small data sets (<10 items)
- ❌ One-time queries

## Optimization Best Practices

1. **Filter early**: Process data in execution environment before showing to model
2. **Use workspace**: Cache intermediate results to files
3. **Batch operations**: Combine multiple tool calls in single code block
4. **Progressive loading**: Load tool definitions only when needed
5. **Structured logging**: Show only summary results to model, not raw data
