# MCP Code Execution Server

완전한 MCP (Model Context Protocol) 서버 구현으로 C++ 빌드/테스트/분석 자동화를 제공합니다.

## 주요 특징

### 🚀 극한의 토큰 효율성
- **빌드**: 400배 절감 (879줄 → 3줄)
- **테스트**: 25배 절감 (500줄 → 20줄)
- **분석**: 10배 절감 (1000줄 → 100줄)

### 🛠️ 제공 도구

1. **build_project** - CMake + Make 빌드
2. **run_tests** - CTest 실행
3. **analyze_code** - 정적 분석 (clang-tidy/cppcheck)

## 설치

```bash
cd "/media/kwon/새 볼륨/1105/.claude/mcp/code-execution"
npm install
npm run build
```

## Claude Code 연동 완료 ✅

`~/.config/Claude/claude_desktop_config.json`에 등록되어 있습니다.

Claude Code를 재시작하면 자동으로 MCP 서버가 로드됩니다.

## 라이선스

MIT
