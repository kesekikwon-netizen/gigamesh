#!/bin/bash
# 자동 빌드 결과 확인 스크립트

echo "🔄 Windows 빌드 완료 대기 중..."

for i in {1..10}; do
    echo "[$i/10] Git pull 중... ($(date +%H:%M:%S))"

    git pull origin claude/code-review-git11-011CUwHY1YsuoxBirMSXxJkm 2>&1 | grep -q "BUILD_DONE.txt"

    if [ -f ".claude/BUILD_DONE.txt" ]; then
        echo "✅ 빌드 완료 감지!"
        cat .claude/build-status.json
        rm .claude/BUILD_DONE.txt
        git add .claude/BUILD_DONE.txt
        git commit -m "Clean: BUILD_DONE 마커 제거"
        exit 0
    fi

    sleep 30  # 30초 대기
done

echo "⏰ 타임아웃 - 수동 확인 필요"
