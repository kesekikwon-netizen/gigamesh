#!/bin/bash

cd "$(dirname "$0")"

echo "========================================="
echo "  JavaScript Executor MCP 종합 테스트"
echo "========================================="
echo ""

test_count=0
pass_count=0

# Test 1: 기본 계산
echo "Test 1: 기본 계산 (1+1)"
result=$(echo '{"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"return 1+1;"}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '"result": 2'; then
  echo "  ✅ PASS - 결과: 2"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# Test 2: 배열 합계
echo "Test 2: 배열 합계 [1,2,3,4,5]"
result=$(echo '{"jsonrpc":"2.0","id":2,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"const arr = [1,2,3,4,5]; return arr.reduce((a,b) => a+b, 0);"}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '"result": 15'; then
  echo "  ✅ PASS - 결과: 15"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# Test 3: Math 연산
echo "Test 3: Math.sqrt() 연산"
result=$(echo '{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"return Math.sqrt(16);"}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '"result": 4'; then
  echo "  ✅ PASS - 결과: 4"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# Test 4: 컨텍스트 변수
echo "Test 4: 컨텍스트 변수 사용"
result=$(echo '{"jsonrpc":"2.0","id":4,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"return x * y;","context":{"x":10,"y":20}}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '"result": 200'; then
  echo "  ✅ PASS - 결과: 200"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# Test 5: 문자열 처리
echo "Test 5: 문자열 처리"
result=$(echo '{"jsonrpc":"2.0","id":5,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"return \"hello\".toUpperCase();"}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '"result": "HELLO"'; then
  echo "  ✅ PASS - 결과: HELLO"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# Test 6: JSON 처리
echo "Test 6: JSON 처리"
result=$(echo '{"jsonrpc":"2.0","id":6,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"const obj = {a:1,b:2}; return JSON.stringify(obj);"}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '{\\"a\\":1,\\"b\\":2}'; then
  echo "  ✅ PASS - 결과: {\"a\":1,\"b\":2}"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# Test 7: 배열 필터링
echo "Test 7: 배열 필터링 (>50)"
result=$(echo '{"jsonrpc":"2.0","id":7,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"return nums.filter(n => n > 50).length;","context":{"nums":[10,30,50,70,90]}}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '"result": 2'; then
  echo "  ✅ PASS - 결과: 2"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# Test 8: 에러 처리
echo "Test 8: 에러 처리"
result=$(echo '{"jsonrpc":"2.0","id":8,"method":"tools/call","params":{"name":"execute_code","arguments":{"code":"throw new Error(\"test error\");"}}}' | node dist/index.js 2>&1 | tail -1)
if echo "$result" | grep -q '"success": false'; then
  echo "  ✅ PASS - 에러 정상 처리"
  ((pass_count++))
else
  echo "  ❌ FAIL"
fi
((test_count++))
echo ""

# 결과 요약
echo "========================================="
echo "  테스트 결과: $pass_count/$test_count 통과"
echo "========================================="

if [ $pass_count -eq $test_count ]; then
  echo "✅ 모든 테스트 통과!"
  exit 0
else
  echo "❌ 일부 테스트 실패"
  exit 1
fi
