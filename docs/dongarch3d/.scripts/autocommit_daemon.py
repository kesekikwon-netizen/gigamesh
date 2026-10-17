#!/usr/bin/env python3
"""
DongArch3D Auto-Commit Daemon
1분마다 자동으로 Git 커밋을 수행하는 백그라운드 프로세스
"""

import os
import sys
import time
import json
import subprocess
from datetime import datetime
from pathlib import Path

# 설정
GIT_REPO_PATH = r"A:\1105"  # 수정: GigaMesh가 아닌 루트 디렉토리
PROGRESS_FILE = r"A:\1105\.dongarch3d-progress.json"
LOG_FILE = r"A:\1105\docs\dongarch3d\.logs\autocommit_log.json"
DAEMON_LOG = r"A:\1105\docs\dongarch3d\.logs\daemon.log"
LAST_COMMIT_FILE = r"A:\1105\docs\dongarch3d\.logs\last_commit.txt"
INTERVAL_SECONDS = 60  # 1분

def log_message(message):
    """로그 메시지 출력 및 파일 저장"""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_line = f"[{timestamp}] {message}\n"

    print(log_line, end='')

    # 로그 파일에 추가
    with open(DAEMON_LOG, 'a', encoding='utf-8') as f:
        f.write(log_line)

def read_progress():
    """진행 상황 JSON 파일 읽기"""
    try:
        with open(PROGRESS_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        log_message(f"Progress file read error: {e}")
        return None

def git_has_changes():
    """Git 변경사항 확인"""
    try:
        os.chdir(GIT_REPO_PATH)
        result = subprocess.run(
            ['git', 'status', '--porcelain'],
            capture_output=True,
            text=True,
            encoding='utf-8'
        )
        return len(result.stdout.strip()) > 0
    except Exception as e:
        log_message(f"Git status error: {e}")
        return False

def git_commit():
    """Git 커밋 수행"""
    try:
        os.chdir(GIT_REPO_PATH)

        # 진행 상황 읽기
        progress = read_progress()
        if not progress:
            return None

        current = progress.get('current', {})
        stats = progress.get('statistics', {})

        phase = current.get('phase', 0)
        task_id = current.get('task_id', '000')
        phase_name = current.get('phase_name', 'Unknown')

        # 타임스탬프
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # 커밋 메시지 생성
        commit_msg = f"""Auto-commit: [{timestamp}] Phase {phase}, Task {task_id}

Phase: {phase_name}
Progress: {stats.get('completion_percentage', 0)}%
Tasks Completed: {stats.get('tasks_completed', 0)}/{stats.get('tasks_total', 71)}

🤖 Auto-committed by DongArch3D Commander

Co-Authored-By: Claude <noreply@anthropic.com>"""

        # Git add
        subprocess.run(['git', 'add', '-A'], check=True)

        # Git commit
        result = subprocess.run(
            ['git', 'commit', '-m', commit_msg],
            capture_output=True,
            text=True,
            encoding='utf-8'
        )

        if result.returncode == 0:
            # 커밋 해시 가져오기
            hash_result = subprocess.run(
                ['git', 'rev-parse', 'HEAD'],
                capture_output=True,
                text=True,
                encoding='utf-8'
            )
            commit_hash = hash_result.stdout.strip()

            log_message(f"Commit successful: {commit_hash[:7]}")

            # 로그 파일 업데이트
            update_log(commit_hash, progress)

            # 마지막 커밋 정보 저장
            save_last_commit(commit_hash, progress)

            return commit_hash
        else:
            log_message(f"Commit failed: {result.stderr}")
            return None

    except Exception as e:
        log_message(f"Git commit error: {e}")
        return None

def update_log(commit_hash, progress):
    """커밋 로그 JSON 파일 업데이트"""
    try:
        # 기존 로그 읽기
        log_data = []
        if os.path.exists(LOG_FILE):
            with open(LOG_FILE, 'r', encoding='utf-8') as f:
                log_data = json.load(f)

        # 새 로그 엔트리 추가
        current = progress.get('current', {})
        stats = progress.get('statistics', {})

        # Git status로 변경된 파일 수 확인
        os.chdir(GIT_REPO_PATH)
        status_result = subprocess.run(
            ['git', 'diff', '--stat', 'HEAD~1', 'HEAD'],
            capture_output=True,
            text=True,
            encoding='utf-8'
        )
        files_changed = len([line for line in status_result.stdout.split('\n') if '|' in line])

        log_entry = {
            "timestamp": datetime.now().isoformat(),
            "commit_hash": commit_hash,
            "phase": current.get('phase', 0),
            "task": current.get('task_id', '000'),
            "files_changed": files_changed,
            "progress_percent": stats.get('completion_percentage', 0),
            "tasks_completed": stats.get('tasks_completed', 0),
            "tasks_total": stats.get('tasks_total', 71)
        }

        log_data.append(log_entry)

        # 로그 파일 저장 (최근 1000개만 유지)
        if len(log_data) > 1000:
            log_data = log_data[-1000:]

        with open(LOG_FILE, 'w', encoding='utf-8') as f:
            json.dump(log_data, f, indent=2, ensure_ascii=False)

    except Exception as e:
        log_message(f"Log update error: {e}")

def save_last_commit(commit_hash, progress):
    """마지막 커밋 정보 저장"""
    try:
        current = progress.get('current', {})
        stats = progress.get('statistics', {})

        content = f"""Last commit: {datetime.now().isoformat()}
Commit hash: {commit_hash}
Phase: {current.get('phase', 0)}
Task: {current.get('task_id', '000')}
Progress: {stats.get('completion_percentage', 0)}%
"""

        with open(LAST_COMMIT_FILE, 'w', encoding='utf-8') as f:
            f.write(content)

    except Exception as e:
        log_message(f"Last commit save error: {e}")

def run_daemon():
    """데몬 메인 루프"""
    log_message("=" * 60)
    log_message("DongArch3D Auto-Commit Daemon Started")
    log_message(f"Repository: {GIT_REPO_PATH}")
    log_message(f"Interval: {INTERVAL_SECONDS} seconds")
    log_message("=" * 60)

    commit_count = 0

    try:
        while True:
            try:
                # Git 변경사항 확인
                if git_has_changes():
                    log_message("Changes detected, committing...")
                    commit_hash = git_commit()
                    if commit_hash:
                        commit_count += 1
                        log_message(f"Total commits: {commit_count}")
                else:
                    log_message("No changes to commit")

                # 대기
                time.sleep(INTERVAL_SECONDS)

            except KeyboardInterrupt:
                log_message("\nReceived interrupt signal, stopping daemon...")
                break
            except Exception as e:
                log_message(f"Loop error: {e}")
                time.sleep(INTERVAL_SECONDS)

    except Exception as e:
        log_message(f"Daemon error: {e}")

    finally:
        log_message(f"Daemon stopped. Total commits: {commit_count}")

if __name__ == "__main__":
    # 로그 디렉토리 생성
    os.makedirs(os.path.dirname(LOG_FILE), exist_ok=True)

    # 데몬 실행
    run_daemon()
