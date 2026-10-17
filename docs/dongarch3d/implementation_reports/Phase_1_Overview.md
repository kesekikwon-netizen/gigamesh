# Phase 1: Align (정렬) 시스템

**기간**: 10일
**우선순위**: 필수
**난이도**: ★★★☆☆
**상태**: 🔴 대기

---

## 🎯 Phase 목표

3D 모델 방향 조정 및 정렬 시스템 구현:
1. **Rotation UI** - 회전 조정 (Scroll Bar + Angle Buttons)
2. **Ground Plane** - 바닥면 표시 (망안지 스타일)
3. **ViewPoint** - 6방향 카메라 프리셋
4. **Fit Ground Plane** - 자동 정렬 (사용자 3점 선택 또는 PCA)

---

## 📋 Task 목록

### Task 1.1: Rotation UI (3일)
- Rotation Scroll Bar (QSlider, -180° ~ 180°)
- Rotation Angle Buttons (QPushButton: 0.1°, 1°, 5°, 90°)
- glRotatef 기반 3D 변환
- 실시간 미리보기

### Task 1.2: Ground Plane 표시 (2일)
- OpenGL 그리드 렌더링
- 망안지 스타일 (회색 그리드)
- 그리드 크기/간격 조정

### Task 1.3: ViewPoint 버튼 (2일)
- 6방향 카메라: Front, Back, Left, Right, Top, Bottom
- Diagonal View (45도 각도)
- 단축키: Ctrl+1~7

### Task 1.4: Fit Ground Plane (자동 정렬) (3일)
- 옵션 1: 사용자 3점 선택 → 평면 계산
- 옵션 2: PCA 기반 자동 평면 감지
- Z축 자동 정렬

---

## ✅ 성공 기준
- [ ] Rotation Scroll Bar로 3D 모델 회전
- [ ] Ground Plane 그리드 표시
- [ ] ViewPoint 6방향 전환 동작
- [ ] Fit Ground Plane 자동 정렬 동작

---

**문서 버전**: 1.0.0
**최종 업데이트**: 2025-11-08
