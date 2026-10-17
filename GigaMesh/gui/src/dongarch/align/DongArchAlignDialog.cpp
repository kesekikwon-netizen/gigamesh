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

#include "DongArchAlignDialog.h"
#include <QGridLayout>
#include <QMessageBox>

namespace DongArch {
namespace Align {

// ============================================================================
// 생성자
// ============================================================================

AlignDialog::AlignDialog(AlignManager* alignManager, QWidget* parent)
    : QDialog(parent)
    , mAlignManager(alignManager)
{
    setWindowTitle(tr("메시 정렬"));
    setModal(false);
    setupUI();
}

// ============================================================================
// UI 초기화
// ============================================================================

void AlignDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // 1. Rotation 그룹
    mainLayout->addWidget(createRotationGroup());

    // 2. Alignment 그룹
    mainLayout->addWidget(createAlignmentGroup());

    // 3. Standard View 그룹
    mainLayout->addWidget(createStandardViewGroup());

    // 4. Close 버튼
    mBtnClose = new QPushButton(tr("닫기"), this);
    connect(mBtnClose, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(mBtnClose);

    setLayout(mainLayout);
}

QGroupBox* AlignDialog::createRotationGroup() {
    auto* group = new QGroupBox(tr("회전 (Rotation)"), this);
    auto* layout = new QGridLayout(group);

    // X축 회전
    layout->addWidget(new QLabel(tr("X축:"), this), 0, 0);
    mSpinRotateX = new QDoubleSpinBox(this);
    mSpinRotateX->setRange(-360.0, 360.0);
    mSpinRotateX->setSingleStep(90.0);
    mSpinRotateX->setValue(90.0);
    mSpinRotateX->setSuffix(tr("°"));
    layout->addWidget(mSpinRotateX, 0, 1);

    mBtnRotateX = new QPushButton(tr("X축 회전"), this);
    connect(mBtnRotateX, &QPushButton::clicked, this, &AlignDialog::onRotateX);
    layout->addWidget(mBtnRotateX, 0, 2);

    // Y축 회전
    layout->addWidget(new QLabel(tr("Y축:"), this), 1, 0);
    mSpinRotateY = new QDoubleSpinBox(this);
    mSpinRotateY->setRange(-360.0, 360.0);
    mSpinRotateY->setSingleStep(90.0);
    mSpinRotateY->setValue(90.0);
    mSpinRotateY->setSuffix(tr("°"));
    layout->addWidget(mSpinRotateY, 1, 1);

    mBtnRotateY = new QPushButton(tr("Y축 회전"), this);
    connect(mBtnRotateY, &QPushButton::clicked, this, &AlignDialog::onRotateY);
    layout->addWidget(mBtnRotateY, 1, 2);

    // Z축 회전
    layout->addWidget(new QLabel(tr("Z축:"), this), 2, 0);
    mSpinRotateZ = new QDoubleSpinBox(this);
    mSpinRotateZ->setRange(-360.0, 360.0);
    mSpinRotateZ->setSingleStep(90.0);
    mSpinRotateZ->setValue(90.0);
    mSpinRotateZ->setSuffix(tr("°"));
    layout->addWidget(mSpinRotateZ, 2, 1);

    mBtnRotateZ = new QPushButton(tr("Z축 회전"), this);
    connect(mBtnRotateZ, &QPushButton::clicked, this, &AlignDialog::onRotateZ);
    layout->addWidget(mBtnRotateZ, 2, 2);

    group->setLayout(layout);
    return group;
}

QGroupBox* AlignDialog::createAlignmentGroup() {
    auto* group = new QGroupBox(tr("정렬 (Alignment)"), this);
    auto* layout = new QHBoxLayout(group);

    mBtnGroundPlane = new QPushButton(tr("Ground Plane 정렬 (Z=0)"), this);
    mBtnGroundPlane->setToolTip(tr("메시 바닥을 Z=0 평면에 정렬합니다"));
    connect(mBtnGroundPlane, &QPushButton::clicked, this, &AlignDialog::onAlignGroundPlane);
    layout->addWidget(mBtnGroundPlane);

    mBtnCenter = new QPushButton(tr("중심 정렬 (원점)"), this);
    mBtnCenter->setToolTip(tr("메시 중심을 원점으로 이동합니다"));
    connect(mBtnCenter, &QPushButton::clicked, this, &AlignDialog::onCenterMesh);
    layout->addWidget(mBtnCenter);

    group->setLayout(layout);
    return group;
}

QGroupBox* AlignDialog::createStandardViewGroup() {
    auto* group = new QGroupBox(tr("표준 뷰 (Standard View)"), this);
    auto* layout = new QGridLayout(group);

    // 2x3 그리드
    mBtnViewTop = new QPushButton(tr("상단 (Top)"), this);
    mBtnViewTop->setToolTip(tr("Z축 상향"));
    layout->addWidget(mBtnViewTop, 0, 0);

    mBtnViewBottom = new QPushButton(tr("하단 (Bottom)"), this);
    mBtnViewBottom->setToolTip(tr("Z축 하향"));
    layout->addWidget(mBtnViewBottom, 0, 1);

    mBtnViewFront = new QPushButton(tr("전면 (Front)"), this);
    mBtnViewFront->setToolTip(tr("Y축 전방"));
    layout->addWidget(mBtnViewFront, 1, 0);

    mBtnViewBack = new QPushButton(tr("후면 (Back)"), this);
    mBtnViewBack->setToolTip(tr("Y축 후방"));
    layout->addWidget(mBtnViewBack, 1, 1);

    mBtnViewLeft = new QPushButton(tr("좌측 (Left)"), this);
    mBtnViewLeft->setToolTip(tr("X축 좌측"));
    layout->addWidget(mBtnViewLeft, 2, 0);

    mBtnViewRight = new QPushButton(tr("우측 (Right)"), this);
    mBtnViewRight->setToolTip(tr("X축 우측"));
    layout->addWidget(mBtnViewRight, 2, 1);

    // 모든 버튼을 onAlignStandardView에 연결
    connect(mBtnViewTop, &QPushButton::clicked, this, &AlignDialog::onAlignStandardView);
    connect(mBtnViewBottom, &QPushButton::clicked, this, &AlignDialog::onAlignStandardView);
    connect(mBtnViewFront, &QPushButton::clicked, this, &AlignDialog::onAlignStandardView);
    connect(mBtnViewBack, &QPushButton::clicked, this, &AlignDialog::onAlignStandardView);
    connect(mBtnViewLeft, &QPushButton::clicked, this, &AlignDialog::onAlignStandardView);
    connect(mBtnViewRight, &QPushButton::clicked, this, &AlignDialog::onAlignStandardView);

    group->setLayout(layout);
    return group;
}

// ============================================================================
// Slots
// ============================================================================

void AlignDialog::onRotateX() {
    if (!mAlignManager || !mAlignManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    double degrees = mSpinRotateX->value();
    if (mAlignManager->rotateX(degrees)) {
        emit meshTransformed();
    } else {
        QMessageBox::warning(this, tr("오류"), tr("X축 회전 실패"));
    }
}

void AlignDialog::onRotateY() {
    if (!mAlignManager || !mAlignManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    double degrees = mSpinRotateY->value();
    if (mAlignManager->rotateY(degrees)) {
        emit meshTransformed();
    } else {
        QMessageBox::warning(this, tr("오류"), tr("Y축 회전 실패"));
    }
}

void AlignDialog::onRotateZ() {
    if (!mAlignManager || !mAlignManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    double degrees = mSpinRotateZ->value();
    if (mAlignManager->rotateZ(degrees)) {
        emit meshTransformed();
    } else {
        QMessageBox::warning(this, tr("오류"), tr("Z축 회전 실패"));
    }
}

void AlignDialog::onAlignGroundPlane() {
    if (!mAlignManager || !mAlignManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    if (mAlignManager->alignToGroundPlane()) {
        emit meshTransformed();
        QMessageBox::information(this, tr("성공"), tr("Ground Plane 정렬 완료"));
    } else {
        QMessageBox::warning(this, tr("오류"), tr("Ground Plane 정렬 실패"));
    }
}

void AlignDialog::onCenterMesh() {
    if (!mAlignManager || !mAlignManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    if (mAlignManager->centerMesh()) {
        emit meshTransformed();
        QMessageBox::information(this, tr("성공"), tr("메시 중심 정렬 완료"));
    } else {
        QMessageBox::warning(this, tr("오류"), tr("메시 중심 정렬 실패"));
    }
}

void AlignDialog::onAlignStandardView() {
    if (!mAlignManager || !mAlignManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    // 어떤 버튼이 클릭되었는지 확인
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) {
        return;
    }

    AlignManager::StandardView view;
    if (btn == mBtnViewTop) {
        view = AlignManager::StandardView::TOP;
    } else if (btn == mBtnViewBottom) {
        view = AlignManager::StandardView::BOTTOM;
    } else if (btn == mBtnViewFront) {
        view = AlignManager::StandardView::FRONT;
    } else if (btn == mBtnViewBack) {
        view = AlignManager::StandardView::BACK;
    } else if (btn == mBtnViewLeft) {
        view = AlignManager::StandardView::LEFT;
    } else if (btn == mBtnViewRight) {
        view = AlignManager::StandardView::RIGHT;
    } else {
        return;
    }

    if (mAlignManager->alignToStandardView(view)) {
        emit meshTransformed();
    } else {
        QMessageBox::warning(this, tr("오류"), tr("표준 뷰 정렬 실패"));
    }
}

} // namespace Align
} // namespace DongArch
