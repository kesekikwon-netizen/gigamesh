# Implementation Examples

## 1. Progressive Disclosure Example

Agent discovers and loads only necessary tools:

```typescript
// Step 1: Explore available servers
const servers = await fs.readdir('./servers');
// Result: ['google-drive', 'salesforce', 'slack']

// Step 2: Explore specific server
const gdriveTools = await fs.readdir('./servers/google-drive');
// Result: ['getDocument.ts', 'getSheet.ts', 'index.ts']

// Step 3: Read specific tool definition
const getDocDef = await fs.readFile('./servers/google-drive/getDocument.ts');

// Step 4: Import and use
import * as gdrive from './servers/google-drive';
const doc = await gdrive.getDocument({ documentId: 'abc123' });
```

**Benefit**: Avoid loading all tool definitions upfront, saving context

## 2. Large Data Filtering

```typescript
// Fetch 10,000 rows
const allRows = await gdrive.getSheet({ sheetId: 'abc123' });

// Filter in execution environment (model doesn't see this)
const pendingOrders = allRows.filter(row =>
  row["Status"] === 'pending'
);

// Agent sees only 5 rows (passed through model)
console.log(`Found ${pendingOrders.length} pending orders`);
console.log(pendingOrders.slice(0, 5));
```

**Token Savings**:
- Traditional: 10,000 rows through model (~500,000 tokens)
- Code Execution: 5 rows through model (~250 tokens)
- **Reduction: 99.95%**

## 3. Control Flow (Loops and Conditions)

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

**Benefit**: Model doesn't evaluate each loop iteration. Code executes automatically in execution environment.

## 4. State Persistence

### First Execution: Save Data

```typescript
const leads = await salesforce.query({
  query: 'SELECT Id, Email FROM Lead LIMIT 1000'
});

const csvData = leads.map(l =>
  `${l.Id},${l.Email}`
).join('\n');

await fs.writeFile('./workspace/leads.csv', csvData);
console.log('Saved 1000 leads to CSV');
```

### Second Execution (Later): Load Saved Data

```typescript
const saved = await fs.readFile('./workspace/leads.csv', 'utf-8');
const rows = saved.split('\n');
console.log(`Loaded ${rows.length} leads from cache`);
```

**Benefit**: Resume workflow after interruption, no re-fetching needed

## 5. Reusable Skills

### Define Skill

```typescript
// ./skills/save-sheet-as-csv.ts
import * as gdrive from '../servers/google-drive';
import * as fs from 'fs/promises';

/**
 * Reusable skill: Save Google Sheet as CSV
 */
export async function saveSheetAsCsv(sheetId: string): Promise<string> {
  const data = await gdrive.getSheet({ sheetId });

  const csv = data.map(row => row.join(',')).join('\n');

  const filepath = `./workspace/sheet-${sheetId}.csv`;
  await fs.writeFile(filepath, csv);

  return filepath;
}
```

### Reuse in Other Agent Executions

```typescript
import { saveSheetAsCsv } from './skills/save-sheet-as-csv';

// Simple invocation
const csvPath = await saveSheetAsCsv('abc123');
console.log(`Saved to ${csvPath}`);
```

## 6. Privacy Protection (Automatic Tokenization)

### Processing Sensitive Data

```typescript
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

console.log(`Updated ${sheet.rows.length} leads`);
```

### What Model Sees (Automatically Tokenized)

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

## 7. Complex Multi-Tool Workflow

```typescript
// Fetch meeting transcript from Google Docs
const transcript = (await gdrive.getDocument({
  documentId: 'meeting-notes-2025-01-15'
})).content;

// Extract action items (processed in execution environment)
const actionItems = transcript
  .split('\n')
  .filter(line => line.includes('TODO:'))
  .map(line => line.replace('TODO:', '').trim());

// Create Salesforce tasks for each action item
for (const item of actionItems) {
  await salesforce.createRecord({
    objectType: 'Task',
    data: {
      Subject: item,
      Status: 'Not Started',
      Priority: 'Normal'
    }
  });
}

// Send Slack notification
await slack.postMessage({
  channel: '#team-updates',
  text: `Created ${actionItems.length} tasks from meeting notes`
});

console.log(`Processed ${actionItems.length} action items`);
```

## 8. Data Aggregation and Analysis

```typescript
// Fetch sales data from multiple sheets
const q1Data = await gdrive.getSheet({ sheetId: 'q1-sales' });
const q2Data = await gdrive.getSheet({ sheetId: 'q2-sales' });
const q3Data = await gdrive.getSheet({ sheetId: 'q3-sales' });
const q4Data = await gdrive.getSheet({ sheetId: 'q4-sales' });

// Combine and analyze (all in execution environment)
const allData = [...q1Data, ...q2Data, ...q3Data, ...q4Data];
const totalRevenue = allData.reduce((sum, row) =>
  sum + parseFloat(row.revenue), 0
);
const avgDealSize = totalRevenue / allData.length;
const topDeals = allData
  .sort((a, b) => b.revenue - a.revenue)
  .slice(0, 10);

// Only show summary to model
console.log(`Total deals: ${allData.length}`);
console.log(`Total revenue: $${totalRevenue.toFixed(2)}`);
console.log(`Average deal size: $${avgDealSize.toFixed(2)}`);
console.log(`Top 10 deals:`, topDeals);
```

**Token Savings**: Instead of passing thousands of rows through model, only summary statistics are shown.

## 9. Error Handling and Retries

```typescript
async function fetchWithRetry<T>(
  fn: () => Promise<T>,
  maxRetries: number = 3
): Promise<T> {
  for (let i = 0; i < maxRetries; i++) {
    try {
      return await fn();
    } catch (error) {
      if (i === maxRetries - 1) throw error;
      console.log(`Attempt ${i + 1} failed, retrying...`);
      await new Promise(r => setTimeout(r, 1000 * (i + 1)));
    }
  }
  throw new Error('Max retries exceeded');
}

// Use with retry logic
const doc = await fetchWithRetry(() =>
  gdrive.getDocument({ documentId: 'abc123' })
);
```

## 10. Batch Processing with Progress Tracking

```typescript
const leads = await salesforce.query({
  query: 'SELECT Id, Email FROM Lead WHERE Status = "New" LIMIT 1000'
});

const batchSize = 100;
const totalBatches = Math.ceil(leads.length / batchSize);

for (let i = 0; i < totalBatches; i++) {
  const batch = leads.slice(i * batchSize, (i + 1) * batchSize);

  for (const lead of batch) {
    await salesforce.updateRecord({
      objectType: 'Lead',
      recordId: lead.Id,
      data: { Status: 'Contacted' }
    });
  }

  console.log(`Processed batch ${i + 1}/${totalBatches}`);
}

console.log(`Completed processing ${leads.length} leads`);
```

**Benefit**: Model only sees progress updates, not every individual operation.
