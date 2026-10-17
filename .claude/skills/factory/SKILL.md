---
name: factory
description: DongArch3D 작업 공장. /factory <개발 요청>, /factory status, /factory next, /factory rework <ID>, /factory approve <ID> 를 해석해 명세→구현→검증→검토→사용자 승인 흐름을 운영한다. 작업 등록·진행·승인 요청이 있을 때 사용.
---

# /factory — 작업 공장 운영 규칙

이 스킬은 `factory/` 폴더와 `.claude/agents/*.md`의 역할별 서브에이전트를 묶어 **한 번에 한 작업씩**, 항상 **사용자 승인에서 끝나는** 개발 흐름을 운영한다. 먼저 `factory/project-profile.md`를 읽고 시작한다.

## 명령 해석

| 입력 | 동작 | 끝나는 상태 |
|---|---|---|
| `/factory <개발 요청>` | 다음 번호로 `factory/jobs/JOB-NNN/` 생성(`_template` 복사), `job.json`에 `status=queued`·요청 원문 기록, `factory/backlog.md`에 한 줄 추가. 이어서 **spec-writer** 에이전트로 `spec.md` 작성 → `status=spec`. 명세는 반드시 "목표 / 범위 밖 / 수용 기준(검증 가능한 문장) / 검증 명령 / 영향 파일 / 위험"을 포함 | `spec` — 사용자가 명세를 볼 수 있게 여기서 멈추고 요약을 보고한다 |
| `/factory status` | `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/factory-status.ps1` 실행 결과(작업표·현재 build 중 작업·대기 사유)를 그대로 보고 | 변화 없음 |
| `/factory next` | `spec` 상태(없으면 `queued`→명세 먼저) 중 `backlog.md` 순서상 첫 작업 1개를 진행: ① `git worktree add .worktrees/JOB-NNN -b factory/JOB-NNN <기준 브랜치>` ② **builder**가 그 worktree에서만 구현(`status=build`, `build-log.md`) ③ **test-runner**가 `powershell -NoProfile -ExecutionPolicy Bypass -File factory/scripts/verify.ps1 -Job JOB-NNN`을 실행해 `verify.md`에 명령·종료 코드·로그 경로 기록(`status=verify`) ④ 검증 통과 시 **code-reviewer, correctness-reviewer, security-reviewer, ui-reviewer**를 병렬로 실행(`status=review`) — 검토자는 제품 코드를 수정하지 않는다 ⑤ 넷 다 pass면 `status=review`(승인 대기), 하나라도 fail이면 `rework`(최대 2회) 후에도 fail이면 `needs-human` | `review` 또는 `needs-human` — **둘 다 사용자 승인 대기** |
| `/factory rework <ID>` | 사용자 지적(또는 검토 fail 사유)을 `decision.md`에 기록하고 같은 worktree에서 build부터 다시 진행 | `review` / `needs-human` |
| `/factory approve <ID>` | **사용자 승인 기록**. `job.json.approvals[]`에 `{by:"user", at:<ISO>, note}` 추가 → `status=approved` → 기준 브랜치에 `--no-ff` 병합(충돌 시 멈추고 보고) → `status=merged`, worktree 제거, `backlog.md` 갱신. **이 명령 없이는 어떤 작업도 approved가 되지 않는다.** AI 검토 통과(`review-*.md`의 pass)는 승인이 아니다 | `merged` |

사용자가 "승인", "approve", "합쳐" 같은 말을 자연어로 해도 **ID가 명시되고 대상이 `review`/`needs-human` 상태일 때만** approve로 해석한다. 모호하면 `/factory status`를 보여 주고 되묻는다.

## 상태 흐름 (허용 전이만)

`queued → spec → build → verify → review → approved → merged`, `verify → rework`(검증 실패), `review → rework`(검토 fail), `rework → build`, `review|rework → needs-human`, `needs-human → rework | approved | dropped`. `factory-status.ps1`이 `job.json`의 `history[]`로 전이를 검사한다.

## 운영 원칙

1. 동시 `build` 작업은 **1개**. 다른 작업이 build/verify/review 중이면 `/factory next`는 대기 사유만 보고한다.
2. 구현은 `factory/JOB-NNN` 브랜치 + `.worktrees/JOB-NNN`에서만. 기준 브랜치(`main` 또는 `factory/setup`)·`feat/dongarch-studio`·메인 체크아웃 `D:\gigamesh`는 건드리지 않는다.
3. 완료 증거는 `verify.md`의 **실제 명령·종료 코드·로그 경로·캡처**다. 마커 파일·"성공했다"는 문장은 증거가 아니다.
4. push는 사용자가 명시적으로 지시한 경우에만, 별도 보고 후 수행한다.
5. 모든 기록 파일은 한국어. 각 단계 끝에 `job.json.updated`와 `history[]`를 갱신한다.
6. 에이전트 호출 시 `factory/project-profile.md`와 해당 `spec.md` 경로를 반드시 프롬프트에 넣는다. 검토자에게는 결론을 주지 말고 diff와 명세만 준다.

## 작업 폴더 형식

`factory/jobs/README.md` 참조. 필수 파일: `job.json`, `spec.md`, `build-log.md`, `verify.md`, `review-code.md`, `review-correctness.md`, `review-security.md`, `review-ui.md`, `decision.md`. 산출물(로그·캡처·SVG/DXF)은 `artifacts/`(gitignore)에.
