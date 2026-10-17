#!/bin/bash

cd "$(dirname "$0")"

echo "========================================="
echo "  JavaScript Executor MCP 테스트"
echo "========================================="
echo ""

run_test() {
  local name="$1"
  local code="$2"
  local context="${3:-{}}"

  echo "📝 $name"

  if [ "$context" = "{}" ]; then
    request="{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":{\"name\":\"execute_code\",\"arguments\":{\"code\":\"$code\"}}}"
  else
    request="{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\",\"params\":{\"name\":\"execute_code\",\"arguments\":{\"code\":\"$code\",\"context\":$context}}}"
  fi

  result=$(echo "$request" | node dist/index.js 2>&1 | tail -1 | python3 -c "import json, sys; data = json.load(sys.stdin); print(data['result']['content'][0]['text'])" 2>/dev/null)

  if [ $? -eq 0 ]; then
    echo "$result" | head -5
    echo "  ✅ 성공"
  else
    echo "  ❌ 실패"
  fi
  echo ""
}

# Test 1
run_test "Test 1: 기본 계산 (1+1)" "return 1+1;"

# Test 2
run_test "Test 2: 배열 합계" "const arr = [1,2,3,4,5]; return arr.reduce((a,b) => a+b, 0);"

# Test 3
run_test "Test 3: Math.sqrt(16)" "return Math.sqrt(16);"

# Test 4
run_test "Test 4: 컨텍스트 변수" "return x * y;" '{"x":10,"y":20}'

# Test 5
run_test "Test 5: 배열 map" "return [1,4,9,16,25].map(n => Math.sqrt(n));"

echo "========================================="
echo "  테스트 완료!"
echo "========================================="
