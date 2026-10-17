//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

#include "DongArchMeasurementPanel.h"
#include <QHeaderView>
#include <QDateTime>
#include <QDebug>
#include <QFormLayout>
#include <cmath>
#include <iostream>

//! Constructor
DongArchMeasurementPanel::DongArchMeasurementPanel(QWidget* parent)
	: QDockWidget(parent),
	  mContentWidget(nullptr),
	  mMainLayout(nullptr),
	  mMeasurementTable(nullptr),
	  mStatisticsGroup(nullptr),
	  mAverageLabel(nullptr),
	  mStdDevLabel(nullptr),
	  mMaxValueLabel(nullptr),
	  mMinValueLabel(nullptr),
	  mCountLabel(nullptr),
	  mExportButton(nullptr),
	  mDeleteButton(nullptr),
	  mDeleteAllButton(nullptr)
{
	setWindowTitle(tr("측정 결과"));
	setObjectName("DongArchMeasurementPanel");

	setupUI();
	loadSampleData(); // Initialize with sample data for testing
}

//! Destructor
DongArchMeasurementPanel::~DongArchMeasurementPanel()
{
	// Qt parent-child relationship handles cleanup
}

//! Setup the user interface
void DongArchMeasurementPanel::setupUI()
{
	// Create main content widget
	mContentWidget = new QWidget(this);
	mMainLayout = new QVBoxLayout(mContentWidget);
	mMainLayout->setSpacing(10);
	mMainLayout->setContentsMargins(10, 10, 10, 10);

	// Create UI components
	createTableWidget();
	createStatisticsGroup();
	createButtonControls();

	setWidget(mContentWidget);
	setMinimumWidth(400);
	setMinimumHeight(300);
}

//! Create measurement history table widget
void DongArchMeasurementPanel::createTableWidget()
{
	mMeasurementTable = new QTableWidget(0, 5, mContentWidget);

	// Set column headers (Korean)
	QStringList headers;
	headers << "번호" << "측정 타입" << "측정값" << "단위" << "측정 시간";
	mMeasurementTable->setHorizontalHeaderLabels(headers);

	// Configure table properties
	mMeasurementTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	mMeasurementTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mMeasurementTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	mMeasurementTable->setAlternatingRowColors(true);
	mMeasurementTable->setSortingEnabled(true);

	// Configure column widths
	mMeasurementTable->horizontalHeader()->setStretchLastSection(true);
	mMeasurementTable->setColumnWidth(0, 50);   // 번호
	mMeasurementTable->setColumnWidth(1, 100);  // 측정 타입
	mMeasurementTable->setColumnWidth(2, 80);   // 측정값
	mMeasurementTable->setColumnWidth(3, 50);   // 단위

	// Enable row selection
	mMeasurementTable->verticalHeader()->setVisible(false);

	mMainLayout->addWidget(mMeasurementTable, 1); // Stretch factor 1
}

//! Create statistics group box
void DongArchMeasurementPanel::createStatisticsGroup()
{
	mStatisticsGroup = new QGroupBox("통계", mContentWidget);
	QFormLayout* layout = new QFormLayout(mStatisticsGroup);
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
	layout->setSpacing(8);

	// Average (평균)
	mAverageLabel = new QLabel("-");
	mAverageLabel->setStyleSheet("QLabel { padding: 4px; background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 3px; }");
	layout->addRow("평균:", mAverageLabel);

	// Standard deviation (표준편차)
	mStdDevLabel = new QLabel("-");
	mStdDevLabel->setStyleSheet("QLabel { padding: 4px; background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 3px; }");
	layout->addRow("표준편차:", mStdDevLabel);

	// Max and Min in single row
	QHBoxLayout* minMaxLayout = new QHBoxLayout();

	mMaxValueLabel = new QLabel("-");
	mMaxValueLabel->setStyleSheet("QLabel { padding: 4px; background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 3px; }");
	QHBoxLayout* maxLayout = new QHBoxLayout();
	maxLayout->addWidget(new QLabel("최대:"));
	maxLayout->addWidget(mMaxValueLabel, 1);

	mMinValueLabel = new QLabel("-");
	mMinValueLabel->setStyleSheet("QLabel { padding: 4px; background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 3px; }");
	QHBoxLayout* minLayout = new QHBoxLayout();
	minLayout->addWidget(new QLabel("최소:"));
	minLayout->addWidget(mMinValueLabel, 1);

	minMaxLayout->addLayout(maxLayout);
	minMaxLayout->addSpacing(10);
	minMaxLayout->addLayout(minLayout);
	layout->addRow("", minMaxLayout);

	// Count (측정 개수)
	mCountLabel = new QLabel("-");
	mCountLabel->setStyleSheet("QLabel { padding: 4px; background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 3px; }");
	layout->addRow("측정 개수:", mCountLabel);

	mMainLayout->addWidget(mStatisticsGroup);
}

//! Create button controls
void DongArchMeasurementPanel::createButtonControls()
{
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setSpacing(8);

	// CSV Export button
	mExportButton = new QPushButton("CSV 내보내기", mContentWidget);
	mExportButton->setToolTip("측정 결과를 CSV 파일로 내보내기");
	connect(mExportButton, &QPushButton::clicked, this, &DongArchMeasurementPanel::onExportToCSV);
	buttonLayout->addWidget(mExportButton);

	// Delete selected button
	mDeleteButton = new QPushButton("선택 삭제", mContentWidget);
	mDeleteButton->setToolTip("선택한 측정 결과 삭제");
	connect(mDeleteButton, &QPushButton::clicked, this, &DongArchMeasurementPanel::onDeleteSelected);
	buttonLayout->addWidget(mDeleteButton);

	// Delete all button
	mDeleteAllButton = new QPushButton("전체 삭제", mContentWidget);
	mDeleteAllButton->setToolTip("모든 측정 결과 삭제");
	connect(mDeleteAllButton, &QPushButton::clicked, this, &DongArchMeasurementPanel::onDeleteAll);
	buttonLayout->addWidget(mDeleteAllButton);

	mMainLayout->addLayout(buttonLayout);
}

//! Add a new measurement to the table
void DongArchMeasurementPanel::addMeasurement(const QString& type, const QString& value,
                                             const QString& unit, const QString& timestamp)
{
	// Temporarily disable sorting while adding items to avoid Qt sorting bug
	// that causes existing items to become NULL
	bool sortingWasEnabled = mMeasurementTable->isSortingEnabled();
	mMeasurementTable->setSortingEnabled(false);

	int row = mMeasurementTable->rowCount();
	mMeasurementTable->insertRow(row);

	// Column 0: Number (번호)
	QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(row + 1));
	numItem->setTextAlignment(Qt::AlignCenter);
	mMeasurementTable->setItem(row, 0, numItem);

	// Column 1: Type (측정 타입)
	QTableWidgetItem* typeItem = new QTableWidgetItem(type);
	typeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	mMeasurementTable->setItem(row, 1, typeItem);

	// Column 2: Value (측정값)
	QTableWidgetItem* valueItem = new QTableWidgetItem(value);
	valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mMeasurementTable->setItem(row, 2, valueItem);

	// Column 3: Unit (단위)
	QTableWidgetItem* unitItem = new QTableWidgetItem(unit);
	unitItem->setTextAlignment(Qt::AlignCenter);
	mMeasurementTable->setItem(row, 3, unitItem);

	// Column 4: Timestamp (측정 시간)
	QTableWidgetItem* timeItem = new QTableWidgetItem(timestamp);
	timeItem->setTextAlignment(Qt::AlignCenter);
	mMeasurementTable->setItem(row, 4, timeItem);

	// Re-enable sorting
	mMeasurementTable->setSortingEnabled(sortingWasEnabled);

	// Update statistics after adding new measurement
	updateStatistics();
}

//! Clear all measurements from the table
void DongArchMeasurementPanel::clearAllMeasurements()
{
	mMeasurementTable->setRowCount(0);
	updateStatistics();
}

//! Delete selected measurements
void DongArchMeasurementPanel::deleteSelectedMeasurements()
{
	QList<QTableWidgetItem*> selectedItems = mMeasurementTable->selectedItems();
	if (selectedItems.isEmpty()) {
		return;
	}

	// Get unique row indices
	QSet<int> rowsToDelete;
	for (QTableWidgetItem* item : selectedItems) {
		rowsToDelete.insert(item->row());
	}

	// Delete rows in reverse order to maintain indices
	QList<int> sortedRows = rowsToDelete.values();
	std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

	for (int row : sortedRows) {
		mMeasurementTable->removeRow(row);
	}

	// Renumber remaining rows
	for (int i = 0; i < mMeasurementTable->rowCount(); ++i) {
		mMeasurementTable->item(i, 0)->setText(QString::number(i + 1));
	}

	updateStatistics();
}

//! Load sample data for testing
void DongArchMeasurementPanel::loadSampleData()
{
	// Add 10 sample measurements
	addMeasurement("거리 측정", "45.2", "mm", "2025-11-08 12:30:15");
	addMeasurement("각도 측정", "30.5", "°", "2025-11-08 12:31:22");
	addMeasurement("면적 측정", "125.4", "cm²", "2025-11-08 12:32:40");
	addMeasurement("거리 측정", "67.8", "mm", "2025-11-08 12:33:18");
	addMeasurement("거리 측정", "150.2", "mm", "2025-11-08 12:34:55");
	addMeasurement("각도 측정", "45.0", "°", "2025-11-08 12:35:30");
	addMeasurement("면적 측정", "98.7", "cm²", "2025-11-08 12:36:12");
	addMeasurement("거리 측정", "120.5", "mm", "2025-11-08 12:37:45");
	addMeasurement("부피 측정", "50.2", "cm³", "2025-11-08 12:38:20");
	addMeasurement("거리 측정", "88.3", "mm", "2025-11-08 12:39:08");
}

//! Update statistics display
void DongArchMeasurementPanel::updateStatistics()
{
	int rowCount = mMeasurementTable->rowCount();

	if (rowCount == 0) {
		mAverageLabel->setText("-");
		mStdDevLabel->setText("-");
		mMaxValueLabel->setText("-");
		mMinValueLabel->setText("-");
		mCountLabel->setText("0개");
		return;
	}

	// Calculate statistics (only from numeric values)
	QVector<double> values;
	QString firstUnit;

	for (int i = 0; i < rowCount; ++i) {
		QString valueStr = mMeasurementTable->item(i, 2)->text();
		QString unit = mMeasurementTable->item(i, 3)->text();

		bool ok;
		double value = valueStr.toDouble(&ok);
		if (ok) {
			values.append(value);
			if (firstUnit.isEmpty()) {
				firstUnit = unit;
			}
		}
	}

	if (values.isEmpty()) {
		mAverageLabel->setText("-");
		mStdDevLabel->setText("-");
		mMaxValueLabel->setText("-");
		mMinValueLabel->setText("-");
		mCountLabel->setText(QString::number(rowCount) + "개");
		return;
	}

	// Calculate average
	double average = 0.0;
	for (double val : values) {
		average += val;
	}
	average /= values.size();

	// Calculate standard deviation
	double variance = 0.0;
	for (double val : values) {
		variance += (val - average) * (val - average);
	}
	variance /= values.size();
	double stdDev = std::sqrt(variance);

	// Find max and min
	double maxVal = *std::max_element(values.begin(), values.end());
	double minVal = *std::min_element(values.begin(), values.end());

	// Update labels
	mAverageLabel->setText(QString::number(average, 'f', 1) + " " + firstUnit);
	mStdDevLabel->setText(QString::number(stdDev, 'f', 1) + " " + firstUnit);
	mMaxValueLabel->setText(QString::number(maxVal, 'f', 1) + " " + firstUnit);
	mMinValueLabel->setText(QString::number(minVal, 'f', 1) + " " + firstUnit);
	mCountLabel->setText(QString::number(rowCount) + "개");
}

// ============================================================================
// Slots
// ============================================================================

//! CSV Export button clicked
void DongArchMeasurementPanel::onExportToCSV()
{
	qDebug() << "CSV 내보내기 버튼 클릭됨 - 기능은 향후 구현 예정";
	// TODO: Implement CSV export functionality
}

//! Delete selected button clicked
void DongArchMeasurementPanel::onDeleteSelected()
{
	qDebug() << "선택 삭제 버튼 클릭됨";
	deleteSelectedMeasurements();
}

//! Delete all button clicked
void DongArchMeasurementPanel::onDeleteAll()
{
	qDebug() << "전체 삭제 버튼 클릭됨";
	clearAllMeasurements();
}

// ============================================================================
// Helper methods
// ============================================================================

//! Get current timestamp in ISO 8601 format
QString DongArchMeasurementPanel::getCurrentTimestamp() const
{
	return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

//! Calculate average of measurement values
double DongArchMeasurementPanel::calculateAverage() const
{
	int rowCount = mMeasurementTable->rowCount();
	if (rowCount == 0) return 0.0;

	double sum = 0.0;
	int validCount = 0;

	for (int i = 0; i < rowCount; ++i) {
		QString valueStr = mMeasurementTable->item(i, 2)->text();
		bool ok;
		double value = valueStr.toDouble(&ok);
		if (ok) {
			sum += value;
			validCount++;
		}
	}

	return (validCount > 0) ? (sum / validCount) : 0.0;
}

//! Calculate standard deviation
double DongArchMeasurementPanel::calculateStdDev(double average) const
{
	int rowCount = mMeasurementTable->rowCount();
	if (rowCount == 0) return 0.0;

	double variance = 0.0;
	int validCount = 0;

	for (int i = 0; i < rowCount; ++i) {
		QString valueStr = mMeasurementTable->item(i, 2)->text();
		bool ok;
		double value = valueStr.toDouble(&ok);
		if (ok) {
			variance += (value - average) * (value - average);
			validCount++;
		}
	}

	return (validCount > 0) ? std::sqrt(variance / validCount) : 0.0;
}
