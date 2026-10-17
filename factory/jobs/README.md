# factory/jobs — 작업 기록 형식

작업 하나 = 폴더 하나 `factory/jobs/JOB-NNN/` (`_template/`를 복사해 시작). 기록 파일은 모두 한국어 Markdown/JSON이며 커밋한다. 빌드 로그·캡처·SVG/DXF 같은 산출물은 `artifacts/`에 두고 커밋하지 않는다(.gitignore).

| 파일 | 쓰는 역할 | 내용 |
|---|---|---|
| `job.json` | 오케스트레이터(/factory) | `id, title, request, status, branch, worktree, base_sha, created, updated, history[] {at, from, to, by, note}, approvals[] {by, at, note}, reviews {code, correctness, security, ui: pass/fail/na}` |
| `spec.md` | spec-writer | 목표 / 범위 밖 / 수용 기준(번호) / 검증 명령 / 영향 파일 / 위험·가정 / 크기 |
| `build-log.md` | builder | 바꾼 파일·이유, 실행 명령·종료 코드, 경고, 제안, 커밋 SHA, 차단 사유 |
| `verify.md` | test-runner | 수용 기준별 명령·종료 코드·근거 표, 결론 PASS/FAIL/BLOCKED, 환경, 산출물 경로 |
| `review-code.md` | code-reviewer | 결론 PASS/FAIL + 항목별 근거(file:line) |
| `review-correctness.md` | correctness-reviewer | 결론 + 단위·축·축척·경계 조건 반례 |
| `review-security.md` | security-reviewer | 결론 + 심각도·근거·시나리오 |
| `review-ui.md` | ui-reviewer | 결론 + R1~R8 표(해당 없음 가능) |
| `decision.md` | 오케스트레이터/사용자 | 재작업 사유, 사용자 지시 원문, 승인/보류/폐기 기록 |

## 상태 전이 (허용 목록)

```
queued → spec → build → verify → review → approved → merged
verify → rework            (검증 실패)
review → rework            (검토 fail, 최대 2회)
rework → build
review | rework → needs-human
needs-human → rework | approved | dropped
```
`factory/scripts/factory-status.ps1`이 `history[]`를 검사해 허용되지 않은 전이와 동시 build 2개 이상을 오류로 보고한다.

## 승인의 의미

- `review` = AI 검토 4종 모두 pass. **승인이 아니다.** 사용자가 결과를 보고 `/factory approve JOB-NNN`을 해야 `approved`가 되고, 그때 기준 브랜치에 `--no-ff`로 병합한다(`merged`).
- `needs-human` = AI가 해결하지 못한 fail 또는 사용자 결정이 필요한 항목. 사용자의 지시(`/factory rework`, `approve`, 폐기) 없이는 움직이지 않는다.
- push는 승인과 별개로 사용자가 명시적으로 지시했을 때만.

## 브랜치·worktree

- 브랜치 `factory/JOB-NNN`, worktree `.worktrees/JOB-NNN`(gitignore). 기준 브랜치는 `job.json.base_sha`에 고정.
- 동시 build 1개. 완료(merged/dropped) 시 worktree 제거, 브랜치는 보존(태그 대신).
