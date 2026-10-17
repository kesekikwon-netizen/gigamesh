/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DONGARCHMEASUREMENTPANEL_H
#define DONGARCHMEASUREMENTPANEL_H

#include <QDockWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>

//!
//! \brief DongArch3D 전용 측정 결과 패널
//!
//! 측정 데이터 이력 및 통계를 표시하는 패널입니다.
//! 고고학 실측 작업에서 수행한 모든 측정 결과를 기록하고 관리합니다.
//!
//! 주요 기능:
//! - 측정 이력 테이블: 번호, 타입, 값, 단위, 시간
//! - 통계 정보: 평균, 표준편차, 최대/최소, 측정 개수
//! - 데이터 관리: CSV 내보내기, 선택 삭제, 전체 삭제
//!
class DongArchMeasurementPanel : public QDockWidget
{
	Q_OBJECT

public:
	explicit DongArchMeasurementPanel(QWidget* parent = nullptr);
	~DongArchMeasurementPanel();

	//! \brief 새로운 측정 결과 추가
	//! \param type 측정 타입 (예: "거리 측정", "각도 측정")
	//! \param value 측정값 (문자열)
	//! \param unit 단위 (예: "mm", "°", "cm²")
	//! \param timestamp 측정 시간 (ISO 8601 형식)
	void addMeasurement(const QString& type, const QString& value,
	                   const QString& unit, const QString& timestamp);

	//! \brief 모든 측정 데이터 삭제
	void clearAllMeasurements();

	//! \brief 선택된 측정 데이터 삭제
	void deleteSelectedMeasurements();

	//! \brief 샘플 데이터로 패널 채우기 (테스트용)
	void loadSampleData();

	//! \brief 통계 정보 업데이트
	void updateStatistics();

private slots:
	//! \brief CSV 내보내기 버튼 클릭
	void onExportToCSV();

	//! \brief 선택 삭제 버튼 클릭
	void onDeleteSelected();

	//! \brief 전체 삭제 버튼 클릭
	void onDeleteAll();

private:
	void setupUI();
	void createTableWidget();
	void createStatisticsGroup();
	void createButtonControls();

	// Helper methods
	QString getCurrentTimestamp() const;
	double calculateAverage() const;
	double calculateStdDev(double average) const;

	// Main layout
	QWidget* mContentWidget;
	QVBoxLayout* mMainLayout;

	// Table widget for measurement history
	QTableWidget* mMeasurementTable;

	// Statistics group
	QGroupBox* mStatisticsGroup;
	QLabel* mAverageLabel;       // 평균
	QLabel* mStdDevLabel;        // 표준편차
	QLabel* mMaxValueLabel;      // 최대값
	QLabel* mMinValueLabel;      // 최소값
	QLabel* mCountLabel;         // 측정 개수

	// Button controls
	QPushButton* mExportButton;      // CSV 내보내기
	QPushButton* mDeleteButton;      // 선택 삭제
	QPushButton* mDeleteAllButton;   // 전체 삭제
};

#endif // DONGARCHMEASUREMENTPANEL_H
