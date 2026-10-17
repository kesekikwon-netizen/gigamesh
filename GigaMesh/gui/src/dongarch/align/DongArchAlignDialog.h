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

#ifndef DONGARCH_ALIGN_DIALOG_H
#define DONGARCH_ALIGN_DIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "DongArchAlignManager.h"

//! \file DongArchAlignDialog.h
//! \brief Phase 1: Mesh Alignment UI Dialog
//!
//! Qt 기반 메시 정렬 다이얼로그

namespace DongArch {
namespace Align {

//! \class AlignDialog
//! \brief 메시 정렬 UI 다이얼로그
//!
//! AlignManager를 사용하여 메시 회전/정렬 기능을 제공하는 Qt 다이얼로그
class AlignDialog : public QDialog {
    Q_OBJECT

public:
    //! 생성자
    //! \param alignManager AlignManager 인스턴스
    //! \param parent 부모 위젯
    explicit AlignDialog(AlignManager* alignManager, QWidget* parent = nullptr);

    //! 소멸자
    ~AlignDialog() override = default;

signals:
    //! 메시 변환 완료 시그널
    void meshTransformed();

private slots:
    // Rotation slots
    void onRotateX();
    void onRotateY();
    void onRotateZ();

    // Alignment slots
    void onAlignGroundPlane();
    void onCenterMesh();
    void onAlignStandardView();

private:
    //! UI 초기화
    void setupUI();

    //! 회전 그룹박스 생성
    QGroupBox* createRotationGroup();

    //! 정렬 그룹박스 생성
    QGroupBox* createAlignmentGroup();

    //! 표준 뷰 그룹박스 생성
    QGroupBox* createStandardViewGroup();

    // UI 위젯
    QDoubleSpinBox* mSpinRotateX;
    QDoubleSpinBox* mSpinRotateY;
    QDoubleSpinBox* mSpinRotateZ;

    QPushButton* mBtnRotateX;
    QPushButton* mBtnRotateY;
    QPushButton* mBtnRotateZ;

    QPushButton* mBtnGroundPlane;
    QPushButton* mBtnCenter;

    QPushButton* mBtnViewTop;
    QPushButton* mBtnViewBottom;
    QPushButton* mBtnViewFront;
    QPushButton* mBtnViewBack;
    QPushButton* mBtnViewLeft;
    QPushButton* mBtnViewRight;

    QPushButton* mBtnClose;

    // AlignManager
    AlignManager* mAlignManager;
};

} // namespace Align
} // namespace DongArch

#endif // DONGARCH_ALIGN_DIALOG_H
