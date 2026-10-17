# JavaScript Executor MCP 사용 가이드

## 🎯 설치 완료 확인

### 등록된 MCP 서버 (2개)

1. **code-execution** - C++ 빌드/테스트/분석
   - `build_project` - CMake + Make 빌드
   - `run_tests` - CTest 실행
   - `analyze_code` - 정적 분석

2. **js-executor** - JavaScript 코드 실행 ✨ (NEW!)
   - `execute_code` - 샌드박스 JavaScript 실행

## 🚀 Claude Code 재시작

**필수 단계:**
1. Claude Code 종료
2. Claude Code 재시작
3. MCP 서버 자동 로드 확인

재시작 후 `execute_code` 도구 사용 가능

## 📝 사용 예제

### 예제 1: 간단한 계산

**요청:**
```
숫자 배열 [10, 20, 30, 40, 50]의 합계를 계산해줘
```

**Claude의 동작:**
```javascript
// execute_code 도구 호출
{
  "code": "const numbers = [10, 20, 30, 40, 50]; return numbers.reduce((a, b) => a + b, 0);"
}

// 결과: 150
```

### 예제 2: 데이터 변환

**요청:**
```
이 데이터에서 가격이 50,000원 이상인 항목만 필터링하고 총합을 구해줘
[{name: "A", price: 30000}, {name: "B", price: 70000}, {name: "C", price: 60000}]
```

**Claude의 동작:**
```javascript
// execute_code 도구 호출
{
  "code": "return items.filter(item => item.price >= 50000).reduce((sum, item) => sum + item.price, 0);",
  "context": {
    "items": [
      {"name": "A", "price": 30000},
      {"name": "B", "price": 70000},
      {"name": "C", "price": 60000}
    ]
  }
}

// 결과: 130000
```

### 예제 3: 복잡한 데이터 처리

**요청:**
```
DongArch3D 프로젝트 파일 목록에서 .cpp 파일의 평균 라인 수를 계산해줘
```

**Claude의 동작:**
```javascript
// execute_code 도구 호출
{
  "code": `
    const cppFiles = files.filter(f => f.name.endsWith('.cpp'));
    const totalLines = cppFiles.reduce((sum, f) => sum + f.lines, 0);
    const avgLines = totalLines / cppFiles.length;
    return {
      totalFiles: cppFiles.length,
      totalLines: totalLines,
      averageLines: Math.round(avgLines)
    };
  `,
  "context": {
    "files": [/* 파일 목록 */]
  }
}
```

### 예제 4: JSON 처리

**요청:**
```
이 JSON 데이터를 파싱해서 age가 30 이상인 사람들의 이름만 추출해줘
```

**Claude의 동작:**
```javascript
{
  "code": "const data = JSON.parse(jsonStr); return data.filter(p => p.age >= 30).map(p => p.name);",
  "context": {
    "jsonStr": '[{"name":"Alice","age":25},{"name":"Bob","age":35},{"name":"Carol","age":40}]'
  }
}

// 결과: ["Bob", "Carol"]
```

## 🎨 활용 시나리오

### 1. 코드 분석 자동화
```
"GigaMesh 소스코드에서 함수 개수를 세어줘"
→ execute_code로 파일 읽고 정규식으로 함수 카운트
```

### 2. 빌드 로그 분석
```
"빌드 로그에서 warning 개수를 카운트해줘"
→ execute_code로 로그 파싱 및 집계
```

### 3. 테스트 결과 요약
```
"테스트 결과 JSON을 분석해서 실패한 테스트만 보여줘"
→ execute_code로 JSON 필터링
```

### 4. 통계 계산
```
"Phase별 구현 파일 수와 평균 라인 수를 계산해줘"
→ execute_code로 집계 및 통계
```

## ⚡ 성능 이점

### 기존 방식 (MCP 도구 직접 호출)
```
1. Salesforce에서 1000개 레코드 조회 → 150K tokens
2. Claude에게 전체 데이터 전송
3. Claude가 데이터 필터링/집계
4. 결과 반환
```

### execute_code 방식
```
1. Salesforce에서 1000개 레코드 조회 → 150K tokens
2. execute_code로 필터링/집계 (로컬 실행)
3. 요약된 결과만 Claude에게 전송 → 2K tokens
```

**토큰 절감: 98.7% (150K → 2K)**

## 🔒 제한사항

### 허용된 built-in 모듈
- ✅ Math, JSON, Date
- ✅ Array, Object, String, Number, Boolean
- ✅ Promise, setTimeout, setInterval

### 허용되지 않음
- ❌ require() / import (외부 모듈)
- ❌ fs (파일 시스템)
- ❌ child_process (프로세스 실행)
- ❌ network 요청

### 타임아웃
- 기본: 5초
- 변경 가능: `{"timeout": 10000}` (10초)

## 🐛 트러블슈팅

### MCP 도구가 안 보일 때
```bash
# 1. Claude Code 완전 종료
pkill -f "claude"

# 2. MCP 서버 상태 확인
ps aux | grep "code-exec"

# 3. 수동 테스트
cd /media/kwon/새\ 볼륨/1105/.claude/mcp/code-exec-simple
echo '{"jsonrpc":"2.0","id":1,"method":"tools/list"}' | node dist/index.js
```

### 에러 발생 시
```javascript
// 에러 정보 확인
{
  "success": false,
  "error": "ReferenceError: xxx is not defined"
}
```

## 📚 참고자료

- [Anthropic - Code Execution with MCP](https://www.anthropic.com/engineering/code-execution-with-mcp)
- MCP SDK: `@modelcontextprotocol/sdk`
- 소스코드: `/media/kwon/새 볼륨/1105/.claude/mcp/code-exec-simple/`

---

**설치 완료:** 2025-11-12
**버전:** 1.0.0
**상태:** ✅ 테스트 완료, 프로덕션 준비됨
