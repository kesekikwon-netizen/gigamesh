# MCP 필수 사용 규칙

**작성일:** 2025-11-12
**중요도:** ⭐⭐⭐ CRITICAL

## 🔴 필수 사항

모든 대화/작업에서 다음 2개 MCP를 **반드시 사용**할 것:

### 1. docfork-mcp (문서 검색/수집)

**도구:**
- `mcp__docfork-mcp__docfork_search_docs`: 문서 검색
- `mcp__docfork-mcp__docfork_read_url`: URL 내용 읽기

**사용 시점:**
- 기술 문서가 필요할 때
- API 참조가 필요할 때
- 외부 정보 확인이 필요할 때
- GitHub README, 블로그 글 등을 읽을 때

**예시:**
```
사용자: "React useState 사용법 알려줘"
→ docfork_search_docs로 React 문서 검색
```

### 2. smithery-ai-server-sequential-thinking (순차적 사고)

**도구:**
- `mcp__smithery-ai-server-sequential-thinking__sequentialthinking`

**사용 시점:**
- 복잡한 문제를 분석할 때
- 다단계 계획을 세울 때
- 의사결정이 필요할 때
- 가설 검증이 필요할 때

**예시:**
```
사용자: "DongArch3D에 새 기능 추가하려면?"
→ sequentialthinking으로 단계별 계획 수립
```

## 📝 사용 방법

### Sequential Thinking 패턴

```typescript
// 1단계: 문제 분석
thought: "사용자가 요청한 것은 X이다. 이를 위해 Y와 Z를 고려해야 한다."
thoughtNumber: 1
totalThoughts: 5
nextThoughtNeeded: true

// 2단계: 가설 수립
thought: "가설: A 방법으로 해결할 수 있다."
thoughtNumber: 2
...

// 마지막: 결론
thought: "최종 답변: ..."
nextThoughtNeeded: false
```

### DocFork 패턴

```typescript
// 문서 검색
docfork_search_docs({
  query: "검색어 + 프레임워크명",
  tokens: "dynamic"
})

// URL 읽기
docfork_read_url({
  url: "https://..."
})
```

## ⚠️ 주의사항

1. **간단한 질문도 사고 과정 기록**
   - Sequential thinking으로 분석 → 결론

2. **정보 확인 필수**
   - 추측하지 말고 docfork로 확인

3. **기록 유지**
   - 모든 사고 과정을 순차적으로 기록
   - 가설 → 검증 → 결론 구조

## 📊 체크리스트

매 대화마다 확인:
- [ ] Sequential thinking 사용했나?
- [ ] 필요한 문서를 docfork로 검색했나?
- [ ] 추측 대신 확인했나?
- [ ] 사고 과정을 기록했나?

---

**이 규칙은 모든 세션에서 적용됩니다.**
