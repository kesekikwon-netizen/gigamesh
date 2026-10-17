// DongArch3D Studio — ② 막으로 자르기 입력 합치기 (Qt 무의존, 시각 ms 는 밖에서 넣는다 → 시험 가능).
// 끄는 동안·키·단추로 막 각도를 바꾸는 입력을 모아 미리보기(LOD 단면 1장)는 16 ms 마다 한 번(throttle),
// 확정(원본 전체 계산)은 놓는 즉시 · 키를 뗀 뒤 300 ms · 단추 마지막 누름 뒤 300 ms(debounce)에 한 번.
// 키를 누르고 있는 동안(자동 반복 포함)은 확정을 미룬다.
#pragma once

namespace studio {

class CutGesture {
public:
    static constexpr long long kPreviewMs = 16;  // 미리보기 합치기 (≈ 60 fps)
    static constexpr long long kCommitMs = 300;  // 키·단추 마지막 입력 뒤 확정
    enum class Due { None, Preview, Commit };

    // 3D 끌기 한 걸음: 확정은 놓을 때까지 미룬다
    void drag(double baseDeg, double deltaDeg, long long nowMs) {
        input(baseDeg, deltaDeg, nowMs);
        commitAt_ = -1;
    }
    // 놓기 = 즉시 확정 (누르고 있던 키도 끝난 것으로 본다 — 다른 동작 전 먼저 확정에 쓰임)
    void release(long long nowMs) {
        if (!active_) return;
        held_ = false;
        commitAt_ = nowMs;
        previewAt_ = -1;
    }
    // 각도 키 누름 (첫 누름·자동 반복 모두): 누르고 있는 동안 확정 보류
    void keyDown(double baseDeg, double deltaDeg, long long nowMs) {
        input(baseDeg, deltaDeg, nowMs);
        held_ = true;
        commitAt_ = -1;
    }
    // 각도 키 뗌 (자동 반복이 아닌 뗌만): 마지막 입력 300 ms 뒤 확정
    void keyUp(long long nowMs) {
        held_ = false;
        if (active_) commitAt_ = nowMs + kCommitMs;
    }
    // 리본 단추: 마지막 누름 300 ms 뒤 확정
    void click(double baseDeg, double deltaDeg, long long nowMs) {
        input(baseDeg, deltaDeg, nowMs);
        commitAt_ = nowMs + kCommitMs;
    }
    void cancel() {
        active_ = held_ = false;
        previewAt_ = commitAt_ = -1;
    }
    // 마감이 지난 일 하나 (확정이 먼저). 확정하면 비활성 — target() 은 계속 읽힌다
    Due poll(long long nowMs) {
        if (active_ && !held_ && commitAt_ >= 0 && commitAt_ <= nowMs) {
            active_ = false;
            previewAt_ = commitAt_ = -1;
            return Due::Commit;
        }
        if (previewAt_ >= 0 && previewAt_ <= nowMs) {
            previewAt_ = -1;
            return Due::Preview;
        }
        return Due::None;
    }
    // 다음 마감 시각, 없으면 −1
    long long nextDueMs() const {
        long long d = previewAt_;
        if (active_ && !held_ && commitAt_ >= 0 && (d < 0 || commitAt_ < d)) d = commitAt_;
        return d;
    }
    bool active() const { return active_; }  // 시작 ~ 확정 사이
    double target() const { return target_; }  // 목표각 (누적, 정규화 안 함)

private:
    void input(double baseDeg, double deltaDeg, long long nowMs) {
        if (!active_) {  // 첫 입력이 지금 확정각에서 시작 (활성 중 base 무시)
            active_ = true;
            target_ = baseDeg;
        }
        target_ += deltaDeg;
        if (previewAt_ < 0) previewAt_ = nowMs + kPreviewMs;  // throttle: 이미 잡혀 있으면 다시 잡지 않는다
    }

    bool active_ = false, held_ = false;
    double target_ = 0.0;
    long long previewAt_ = -1, commitAt_ = -1;
};

}  // namespace studio
