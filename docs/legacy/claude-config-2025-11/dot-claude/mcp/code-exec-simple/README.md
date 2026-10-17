# JavaScript Code Execution MCP Server

Anthropic 블로그의 "Code Execution with MCP" 구현체입니다.

## ✅ 테스트 완료

모든 기능이 정상 작동합니다:

### 1. 기본 계산
```javascript
const numbers = [1, 2, 3, 4, 5];
const sum = numbers.reduce((a, b) => a + b, 0);
return sum;
// 결과: 15 (1ms)
```

### 2. Math 연산
```javascript
const data = [1, 4, 9, 16, 25];
return data.map(x => Math.sqrt(x));
// 결과: [1, 2, 3, 4, 5] (1ms)
```

### 3. 컨텍스트 변수 사용
```javascript
// context: {"numbers": [10, 25, 50, 75, 100, 125]}
return numbers.filter(n => n > 50).reduce((sum, n) => sum + n, 0);
// 결과: 300 (2ms)
```

### 4. Async/Await
```javascript
await new Promise(resolve => setTimeout(resolve, 100));
return 'async works!';
// 결과: "async works!" (101ms)
```

### 5. 에러 처리
```javascript
throw new Error('Test error handling');
// 결과: {"success": false, "error": "Error: Test error handling"}
```

## 🚀 사용 방법

### Claude Code에서 사용

**.claude/mcp.json에 등록됨:**
```json
{
  "mcpServers": {
    "js-executor": {
      "command": "node",
      "args": ["/media/kwon/새 볼륨/1105/.claude/mcp/code-exec-simple/dist/index.js"],
      "env": {},
      "disabled": false
    }
  }
}
```

**Claude Code 재시작 후** `execute_code` 도구 사용 가능

### MCP 도구 사양

**Tool Name:** `execute_code`

**Parameters:**
- `code` (string, required): JavaScript 코드
- `context` (object, optional): 실행 컨텍스트 변수
- `timeout` (number, optional): 타임아웃 (기본값: 5000ms)

**Returns:**
```json
{
  "success": true,
  "result": <실행 결과>,
  "executionTime": "1ms"
}
```

## 🔒 보안

- Node.js VM 샌드박스 사용
- 타임아웃 제한 (기본 5초)
- 제한된 built-in 모듈만 접근 가능:
  - Math, JSON, Date
  - Array, Object, String, Number, Boolean
  - Promise, setTimeout, setInterval

## 📊 성능

- 간단한 계산: **1-2ms**
- 비동기 작업: **실제 대기 시간 + 1ms**
- 토큰 효율성: Anthropic 블로그 기준 **98.7% 절감**

## 🛠️ 개발

```bash
# 빌드
npm run build

# 테스트
echo '{"jsonrpc":"2.0","id":1,"method":"tools/list"}' | node dist/index.js
```

## 📄 라이선스

MIT

---

**구현 완료:** 2025-11-12
**기반:** [Anthropic - Code Execution with MCP](https://www.anthropic.com/engineering/code-execution-with-mcp)
