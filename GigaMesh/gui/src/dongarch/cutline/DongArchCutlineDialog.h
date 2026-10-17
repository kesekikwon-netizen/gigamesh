/* * DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * This file is part of DongArch3D.
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_CUTLINE_DIALOG_H
#define DONGARCH_CUTLINE_DIALOG_H

#include "DongArchCutlineManager.h"
#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

//! \file DongArchCutlineDialog.h
//! \brief Phase 2: Cutline (단면 라인) 추출 UI 다이얼로그
//!
//! DongArch3D v4.0 Phase 2 구현
//! - GigaMesh calcIntersectionPolylineWithPlane 활용
//! - Qt signals/slots 진행률 표시
//! - C++20 DongArchCutlineManager 연동

namespace DongArch {
namespace Cutline {

//! \class CutlineDialog
//! \brief Cutline 추출 UI 다이얼로그
//!
//! 사용자가 평면 높이를 지정하고 교차선을 추출하는 UI
class CutlineDialog : public QDialog {
    Q_OBJECT

public:
    //! 생성자
    //! \param cutlineManager CutlineManager 인스턴스
    //! \param parent 부모 위젯
    explicit CutlineDialog(CutlineManager* cutlineManager, QWidget* parent = nullptr);

    //! 소멸자
    ~CutlineDialog() override = default;

signals:
    //! 교차선 추출 완료 시그널
    void cutlineExtracted();

private slots:
    //! 단일 평면 추출
    void onExtractSingle();

    //! 여러 평면 배치 추출
    void onExtractBatch();

    //! Octree 최적화 추출
    void onExtractWithOctree();

    //! 진행률 업데이트
    void onProgressChanged(int percent);

    //! 상태 메시지 업데이트
    void onStatusMessage(const QString& message);

    //! 추출 완료
    void onCutlineExtracted(size_t pointCount);

private:
    //! UI 초기화
    void setupUI();

    //! 단일 평면 그룹 생성
    QGroupBox* createSinglePlaneGroup();

    //! 배치 처리 그룹 생성
    QGroupBox* createBatchGroup();

    //! 최적화 옵션 그룹 생성
    QGroupBox* createOptimizationGroup();

    //! 진행률 그룹 생성
    QWidget* createProgressGroup();

    CutlineManager* mCutlineManager;  //!< CutlineManager (non-owning)

    // 단일 평면 컨트롤
    QDoubleSpinBox* mSpinPlaneHeight;   //!< 평면 높이 (Z 좌표)
    QDoubleSpinBox* mSpinPlaneNormalX;  //!< 평면 법선 X
    QDoubleSpinBox* mSpinPlaneNormalY;  //!< 평면 법선 Y
    QDoubleSpinBox* mSpinPlaneNormalZ;  //!< 평면 법선 Z
    QPushButton* mBtnExtractSingle;     //!< 단일 추출 버튼

    // 배치 처리 컨트롤
    QDoubleSpinBox* mSpinBatchStart;    //!< 시작 높이
    QDoubleSpinBox* mSpinBatchEnd;      //!< 종료 높이
    QDoubleSpinBox* mSpinBatchStep;     //!< 간격
    QPushButton* mBtnExtractBatch;      //!< 배치 추출 버튼

    // 최적화 옵션
    QCheckBox* mCheckUseOctree;         //!< Octree 사용 여부
    QCheckBox* mCheckRemoveDuplicates;  //!< 중복 제거 여부
    QCheckBox* mCheckSortPoints;        //!< 점 정렬 여부
    QDoubleSpinBox* mSpinTolerance;     //!< 중복 판정 거리

    // 진행률 표시
    QProgressBar* mProgressBar;         //!< 진행률 바
    QLabel* mLabelStatus;               //!< 상태 메시지
    QLabel* mLabelPointCount;           //!< 추출된 점 개수

    // 닫기 버튼
    QPushButton* mBtnClose;             //!< 닫기 버튼
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_DIALOG_H
