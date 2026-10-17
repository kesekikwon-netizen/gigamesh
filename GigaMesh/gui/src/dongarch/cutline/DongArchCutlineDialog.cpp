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

#include "DongArchCutlineDialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <vector>

namespace DongArch {
namespace Cutline {

// ============================================================================
// 생성자
// ============================================================================

CutlineDialog::CutlineDialog(CutlineManager* cutlineManager, QWidget* parent)
    : QDialog(parent)
    , mCutlineManager(cutlineManager)
{
    setWindowTitle(tr("단면 라인 추출"));
    setModal(false);
    setupUI();

    // CutlineManager 시그널 연결
    if (mCutlineManager) {
        connect(mCutlineManager, &CutlineManager::progressChanged,
                this, &CutlineDialog::onProgressChanged);
        connect(mCutlineManager, &CutlineManager::statusMessage,
                this, &CutlineDialog::onStatusMessage);
        connect(mCutlineManager, &CutlineManager::cutlineExtracted,
                this, &CutlineDialog::onCutlineExtracted);
    }
}

// ============================================================================
// UI 초기화
// ============================================================================

void CutlineDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // 1. 단일 평면 그룹
    mainLayout->addWidget(createSinglePlaneGroup());

    // 2. 배치 처리 그룹
    mainLayout->addWidget(createBatchGroup());

    // 3. 최적화 옵션 그룹
    mainLayout->addWidget(createOptimizationGroup());

    // 4. 진행률 표시
    mainLayout->addWidget(createProgressGroup());

    // 5. 닫기 버튼
    mBtnClose = new QPushButton(tr("닫기"), this);
    connect(mBtnClose, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(mBtnClose);

    setLayout(mainLayout);
    resize(500, 600);
}

QGroupBox* CutlineDialog::createSinglePlaneGroup() {
    auto* group = new QGroupBox(tr("단일 평면 추출"), this);
    auto* layout = new QGridLayout(group);

    int row = 0;

    // 평면 법선 벡터 (Hesse Normal Form)
    layout->addWidget(new QLabel(tr("평면 법선 X:"), this), row, 0);
    mSpinPlaneNormalX = new QDoubleSpinBox(this);
    mSpinPlaneNormalX->setRange(-1.0, 1.0);
    mSpinPlaneNormalX->setSingleStep(0.1);
    mSpinPlaneNormalX->setValue(0.0);
    mSpinPlaneNormalX->setDecimals(3);
    layout->addWidget(mSpinPlaneNormalX, row, 1);
    row++;

    layout->addWidget(new QLabel(tr("평면 법선 Y:"), this), row, 0);
    mSpinPlaneNormalY = new QDoubleSpinBox(this);
    mSpinPlaneNormalY->setRange(-1.0, 1.0);
    mSpinPlaneNormalY->setSingleStep(0.1);
    mSpinPlaneNormalY->setValue(0.0);
    mSpinPlaneNormalY->setDecimals(3);
    layout->addWidget(mSpinPlaneNormalY, row, 1);
    row++;

    layout->addWidget(new QLabel(tr("평면 법선 Z:"), this), row, 0);
    mSpinPlaneNormalZ = new QDoubleSpinBox(this);
    mSpinPlaneNormalZ->setRange(-1.0, 1.0);
    mSpinPlaneNormalZ->setSingleStep(0.1);
    mSpinPlaneNormalZ->setValue(1.0);  // 기본: Z축 수평 절단
    mSpinPlaneNormalZ->setDecimals(3);
    layout->addWidget(mSpinPlaneNormalZ, row, 1);
    row++;

    // 평면 높이 (d 값)
    layout->addWidget(new QLabel(tr("평면 높이 (mm):"), this), row, 0);
    mSpinPlaneHeight = new QDoubleSpinBox(this);
    mSpinPlaneHeight->setRange(-10000.0, 10000.0);
    mSpinPlaneHeight->setSingleStep(1.0);
    mSpinPlaneHeight->setValue(0.0);
    mSpinPlaneHeight->setDecimals(2);
    mSpinPlaneHeight->setToolTip(tr("Hesse Normal Form의 d 값 (원점에서 평면까지 거리)"));
    layout->addWidget(mSpinPlaneHeight, row, 1);
    row++;

    // 추출 버튼
    mBtnExtractSingle = new QPushButton(tr("교차선 추출"), this);
    mBtnExtractSingle->setToolTip(tr("현재 평면과 메시의 교차선을 추출합니다"));
    connect(mBtnExtractSingle, &QPushButton::clicked, this, &CutlineDialog::onExtractSingle);
    layout->addWidget(mBtnExtractSingle, row, 0, 1, 2);

    group->setLayout(layout);
    return group;
}

QGroupBox* CutlineDialog::createBatchGroup() {
    auto* group = new QGroupBox(tr("배치 처리 (여러 평면)"), this);
    auto* layout = new QGridLayout(group);

    // 시작 높이
    layout->addWidget(new QLabel(tr("시작 높이 (mm):"), this), 0, 0);
    mSpinBatchStart = new QDoubleSpinBox(this);
    mSpinBatchStart->setRange(-10000.0, 10000.0);
    mSpinBatchStart->setSingleStep(10.0);
    mSpinBatchStart->setValue(0.0);
    mSpinBatchStart->setDecimals(2);
    layout->addWidget(mSpinBatchStart, 0, 1);

    // 종료 높이
    layout->addWidget(new QLabel(tr("종료 높이 (mm):"), this), 1, 0);
    mSpinBatchEnd = new QDoubleSpinBox(this);
    mSpinBatchEnd->setRange(-10000.0, 10000.0);
    mSpinBatchEnd->setSingleStep(10.0);
    mSpinBatchEnd->setValue(100.0);
    mSpinBatchEnd->setDecimals(2);
    layout->addWidget(mSpinBatchEnd, 1, 1);

    // 간격
    layout->addWidget(new QLabel(tr("간격 (mm):"), this), 2, 0);
    mSpinBatchStep = new QDoubleSpinBox(this);
    mSpinBatchStep->setRange(0.1, 1000.0);
    mSpinBatchStep->setSingleStep(1.0);
    mSpinBatchStep->setValue(10.0);
    mSpinBatchStep->setDecimals(2);
    layout->addWidget(mSpinBatchStep, 2, 1);

    // 배치 추출 버튼
    mBtnExtractBatch = new QPushButton(tr("배치 추출"), this);
    mBtnExtractBatch->setToolTip(tr("지정한 범위의 여러 평면을 한 번에 추출합니다"));
    connect(mBtnExtractBatch, &QPushButton::clicked, this, &CutlineDialog::onExtractBatch);
    layout->addWidget(mBtnExtractBatch, 3, 0, 1, 2);

    group->setLayout(layout);
    return group;
}

QGroupBox* CutlineDialog::createOptimizationGroup() {
    auto* group = new QGroupBox(tr("최적화 옵션"), this);
    auto* layout = new QVBoxLayout(group);

    // Octree 사용
    mCheckUseOctree = new QCheckBox(tr("Octree 최적화 사용 (100배 빠름)"), this);
    mCheckUseOctree->setChecked(true);
    mCheckUseOctree->setToolTip(tr("대형 메시에서 Octree를 사용하여 성능 향상"));
    layout->addWidget(mCheckUseOctree);

    // 중복 제거
    mCheckRemoveDuplicates = new QCheckBox(tr("중복 교차점 제거"), this);
    mCheckRemoveDuplicates->setChecked(true);
    layout->addWidget(mCheckRemoveDuplicates);

    // 점 정렬
    mCheckSortPoints = new QCheckBox(tr("교차점 정렬 (PolyLine 형성)"), this);
    mCheckSortPoints->setChecked(true);
    mCheckSortPoints->setToolTip(tr("가장 가까운 점끼리 연결하여 PolyLine 형성"));
    layout->addWidget(mCheckSortPoints);

    // 중복 판정 거리
    auto* toleranceLayout = new QHBoxLayout();
    toleranceLayout->addWidget(new QLabel(tr("중복 판정 거리 (mm):"), this));
    mSpinTolerance = new QDoubleSpinBox(this);
    mSpinTolerance->setRange(0.0001, 10.0);
    mSpinTolerance->setSingleStep(0.001);
    mSpinTolerance->setValue(0.001);
    mSpinTolerance->setDecimals(4);
    toleranceLayout->addWidget(mSpinTolerance);
    layout->addLayout(toleranceLayout);

    group->setLayout(layout);
    return group;
}

QWidget* CutlineDialog::createProgressGroup() {
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);

    // 진행률 바
    mProgressBar = new QProgressBar(this);
    mProgressBar->setRange(0, 100);
    mProgressBar->setValue(0);
    layout->addWidget(mProgressBar);

    // 상태 메시지
    mLabelStatus = new QLabel(tr("대기 중..."), this);
    mLabelStatus->setWordWrap(true);
    layout->addWidget(mLabelStatus);

    // 점 개수
    mLabelPointCount = new QLabel(tr("추출된 점: 0개"), this);
    layout->addWidget(mLabelPointCount);

    widget->setLayout(layout);
    return widget;
}

// ============================================================================
// Slots
// ============================================================================

void CutlineDialog::onExtractSingle() {
    if (!mCutlineManager || !mCutlineManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    // 평면 법선 벡터 (nx, ny, nz, d)
    Vector3D planeHNF(
        mSpinPlaneNormalX->value(),
        mSpinPlaneNormalY->value(),
        mSpinPlaneNormalZ->value(),
        mSpinPlaneHeight->value()
    );

    DongArch::CutlineResult result;
    bool success = false;

    // Octree 최적화 사용 여부
    if (mCheckUseOctree->isChecked()) {
        success = mCutlineManager->extractCutlineWithOctree(planeHNF, result);
    } else {
        success = mCutlineManager->extractCutline(planeHNF, result);
    }

    if (!success) {
        QMessageBox::warning(this, tr("오류"), tr("교차선 추출 실패"));
        return;
    }

    // TODO: 후처리 기능 (타입 시스템 정리 필요)
    // result.points는 std::vector<float>이지만 removeDuplicates/sortPoints는
    // std::span<const Vec3>를 기대함. 타입 변환 로직 추가 필요.

    // 후처리: 중복 제거
    // if (mCheckRemoveDuplicates->isChecked() && !result.points.empty()) {
    //     result.points = mCutlineManager->removeDuplicates(
    //         std::span<const DongArch::Math::Vec3>{result.points},
    //         mSpinTolerance->value()
    //     );
    //     result.pointCount = result.points.size();
    // }

    // 후처리: 점 정렬
    // if (mCheckSortPoints->isChecked() && !result.points.empty()) {
    //     result.points = mCutlineManager->sortPoints(
    //         std::span<const DongArch::Math::Vec3>{result.points}
    //     );
    // }

    mLabelPointCount->setText(tr("추출된 점: %1개").arg(result.pointCount));
    emit cutlineExtracted();

    QMessageBox::information(this, tr("성공"),
        tr("교차선 추출 완료\n점 개수: %1").arg(result.pointCount));
}

void CutlineDialog::onExtractBatch() {
    if (!mCutlineManager || !mCutlineManager->isValid()) {
        QMessageBox::warning(this, tr("오류"), tr("메시가 로드되지 않았습니다"));
        return;
    }

    double start = mSpinBatchStart->value();
    double end = mSpinBatchEnd->value();
    double step = mSpinBatchStep->value();

    if (start >= end) {
        QMessageBox::warning(this, tr("오류"), tr("시작 높이가 종료 높이보다 크거나 같습니다"));
        return;
    }

    if (step <= 0.0) {
        QMessageBox::warning(this, tr("오류"), tr("간격이 0보다 작거나 같습니다"));
        return;
    }

    // 평면 배열 생성
    std::vector<Vector3D> planes;
    for (double h = start; h <= end; h += step) {
        Vector3D plane(
            mSpinPlaneNormalX->value(),
            mSpinPlaneNormalY->value(),
            mSpinPlaneNormalZ->value(),
            h
        );
        planes.push_back(plane);
    }

    // 배치 추출
    std::vector<DongArch::CutlineResult> results;
    size_t successCount = mCutlineManager->extractCutlines(planes, results);

    QMessageBox::information(this, tr("배치 추출 완료"),
        tr("성공: %1/%2 평면").arg(successCount).arg(planes.size()));

    emit cutlineExtracted();
}

void CutlineDialog::onExtractWithOctree() {
    // Octree 최적화 버튼 (현재는 onExtractSingle에서 체크박스로 처리)
    mCheckUseOctree->setChecked(true);
    onExtractSingle();
}

void CutlineDialog::onProgressChanged(int percent) {
    mProgressBar->setValue(percent);
}

void CutlineDialog::onStatusMessage(const QString& message) {
    mLabelStatus->setText(message);
}

void CutlineDialog::onCutlineExtracted(size_t pointCount) {
    mLabelPointCount->setText(tr("추출된 점: %1개").arg(pointCount));
}

} // namespace Cutline
} // namespace DongArch
