# MCP 사용 규칙

**필독**: 이 규칙은 모든 개발 작업에 적용됩니다.

## 핵심 규칙

### ⚠️ 항상 MCP 도구를 사용하라

**위치**: `/media/kwon/새 볼륨/1105/.claude/mcp/code-execution`

**사용 시점**:
1. 빌드 에러 진단
2. 테스트 실행
3. 코드 분석
4. 성능 프로파일링

## 빌드 시

### ✅ 올바른 방법
```bash
cd "/media/kwon/새 볼륨/1105/GigaMesh/build"
make -j4 2>&1 | grep -E "(error|warning.*DongArch)" | head -20
```

**효과**: 879줄 → 3줄 (400배 토큰 절약)

### ❌ 잘못된 방법
```bash
make -j4 2>&1 | tee build.log  # 전체 로그 읽기 (200K 토큰 낭비)
cat build.log                  # 879줄 전체
```

## 테스트 시

### ✅ 올바른 방법
```bash
npx ts-node /media/kwon/새\ 볼륨/1105/.claude/mcp/code-execution/servers/test-server.ts \
  "/media/kwon/새 볼륨/1105/GigaMesh/build" \
  "Cutline.*"
```

**효과**: 전체 출력 → 실패한 테스트만 (25배 절약)

## 코드 분석 시

### ✅ 올바른 방법
```bash
npx ts-node /media/kwon/새\ 볼륨/1105/.claude/mcp/code-execution/servers/analysis-server.ts \
  clang-tidy \
  "/media/kwon/새 볼륨/1105/GigaMesh/gui/src/dongarch/cutline" \
  "modernize-*,readability-*"
```

**효과**: 전체 리포트 → 심각도 필터링 (10배 절약)

## 토큰 절약 통계

| 작업 | 기존 방식 | MCP 방식 | 절감률 |
|------|----------|---------|-------|
| 빌드 에러 진단 | 879줄 (200K 토큰) | 3줄 (500 토큰) | **400배** |
| 테스트 실행 | 전체 출력 (50K 토큰) | 실패만 (2K 토큰) | **25배** |
| 코드 분석 | 전체 리포트 (30K 토큰) | 필터링 (3K 토큰) | **10배** |

## 기억할 것

1. **모든 빌드**: grep으로 에러만 필터링
2. **모든 테스트**: test-server.ts로 실패만 확인
3. **모든 분석**: analysis-server.ts로 심각도 필터링

## 예외

다음 경우에만 전체 로그 읽기 허용:
- 디버깅 시 전체 컨텍스트 필요
- 사용자가 명시적으로 요청
- MCP 도구 자체 디버깅

---

**이 규칙을 위반하면 토큰을 낭비하게 됩니다!**
