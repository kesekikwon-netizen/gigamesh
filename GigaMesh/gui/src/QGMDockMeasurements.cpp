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

#include "QGMDockMeasurements.h"
#include <QHeaderView>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <cmath>
#include <iostream>

QGMDockMeasurements::QGMDockMeasurements(QWidget* parent)
    : QDockWidget(parent),
      mContentWidget(nullptr),
      mMainLayout(nullptr),
      mTableWidget(nullptr),
      mStatisticsLabel(nullptr),
      mFilterComboBox(nullptr),
      mExportButton(nullptr),
      mClearButton(nullptr),
      mCurrentFilter("All"),
      mNextId(1)
{
    std::cout << "[QGMDockMeasurements] Constructor started" << std::endl;
    setWindowTitle(tr("Measurements"));
    setObjectName("QGMDockMeasurements");
    std::cout << "[QGMDockMeasurements] About to call setupUI()..." << std::endl;
    setupUI();
    std::cout << "[QGMDockMeasurements] Constructor completed" << std::endl;
}

QGMDockMeasurements::~QGMDockMeasurements() {
    // Qt parent-child system handles cleanup
}

void QGMDockMeasurements::setupUI() {
    // Create main content widget
    mContentWidget = new QWidget(this);
    mMainLayout = new QVBoxLayout(mContentWidget);
    mMainLayout->setSpacing(4);
    mMainLayout->setContentsMargins(4, 4, 4, 4);

    // Create filter combo box layout (Task 104)
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel(tr("Filter by Type:"), mContentWidget);
    mFilterComboBox = new QComboBox(mContentWidget);
    mFilterComboBox->addItem(tr("All"));
    mFilterComboBox->addItem(tr("Distance"));
    mFilterComboBox->addItem(tr("Angle"));
    mFilterComboBox->addItem(tr("Area"));
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(mFilterComboBox);
    filterLayout->addStretch();
    mMainLayout->addLayout(filterLayout);

    // Create table widget
    mTableWidget = new QTableWidget(0, 5, mContentWidget);
    mTableWidget->setHorizontalHeaderLabels({tr("ID"), tr("Type"), tr("Value"), tr("Unit"), tr("Time")});
    mTableWidget->horizontalHeader()->setStretchLastSection(true);
    mTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTableWidget->setAlternatingRowColors(true);

    // Enable sorting by clicking column headers (Task 104)
    mTableWidget->setSortingEnabled(true);

    // Set column widths
    mTableWidget->setColumnWidth(0, 60);   // ID
    mTableWidget->setColumnWidth(1, 80);   // Type
    mTableWidget->setColumnWidth(2, 80);   // Value
    mTableWidget->setColumnWidth(3, 60);   // Unit

    mMainLayout->addWidget(mTableWidget);

    // Statistics label with enhanced display (Task 104)
    mStatisticsLabel = new QLabel(tr("Count: 0"), mContentWidget);
    mStatisticsLabel->setStyleSheet("padding: 4px; background-color: rgba(255, 255, 255, 20);");
    mStatisticsLabel->setWordWrap(true);
    mMainLayout->addWidget(mStatisticsLabel);

    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    mExportButton = new QPushButton(tr("Export CSV"), mContentWidget);
    mClearButton = new QPushButton(tr("Clear"), mContentWidget);

    buttonLayout->addWidget(mExportButton);
    buttonLayout->addWidget(mClearButton);

    mMainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(mExportButton, &QPushButton::clicked, this, &QGMDockMeasurements::exportToCSV);
    connect(mClearButton, &QPushButton::clicked, this, &QGMDockMeasurements::clearMeasurements);
    connect(mFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QGMDockMeasurements::filterByType);

    setWidget(mContentWidget);
    setMinimumHeight(200);
}

void QGMDockMeasurements::addMeasurement(const MeasurementData& measurement) {
    // Store measurement first
    MeasurementData storedData = measurement;
    storedData.id = mNextId;
    mMeasurements.append(storedData);
    mNextId++;

    // Update table display with filter applied
    updateTableDisplay();
    updateStatistics();
}

void QGMDockMeasurements::clearMeasurements() {
    mTableWidget->setRowCount(0);
    mMeasurements.clear();
    mNextId = 1;
    updateStatistics();
}

QVector<MeasurementData> QGMDockMeasurements::getAllMeasurements() const {
    return mMeasurements;
}

void QGMDockMeasurements::exportToCSV() {
    if (mMeasurements.isEmpty()) {
        QMessageBox::information(this, tr("Export"), tr("No measurements to export."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save CSV File"),
        "",
        tr("CSV Files (*.csv)"));

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file for writing."));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);  // UTF-8 BOM for Excel compatibility

    // Write header
    out << "ID,Type,Value,Unit,Time\n";

    // Write data
    for (const MeasurementData& measurement : mMeasurements) {
        out << measurement.id << ","
            << measurement.type << ","
            << measurement.value << ","
            << measurement.unit << ","
            << measurement.timestamp.toString("yyyy-MM-dd HH:mm:ss") << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Export Complete"), tr("Measurements exported to CSV file."));
}

// Task 104: Update statistics display with comprehensive metrics
void QGMDockMeasurements::updateStatistics() {
    // Collect values based on current filter
    QVector<double> values;
    QString filterType = mFilterComboBox->currentText();

    for (const MeasurementData& m : mMeasurements) {
        if (filterType == "All" || m.type == filterType) {
            values.append(m.value);
        }
    }

    int count = values.size();

    if (count == 0) {
        mStatisticsLabel->setText(tr("Count: 0"));
        return;
    }

    // Calculate statistics
    double min = values[0];
    double max = values[0];
    double sum = 0.0;

    for (double val : values) {
        if (val < min) min = val;
        if (val > max) max = val;
        sum += val;
    }

    double average = calculateAverage(values);
    double stdDev = calculateStdDev(values, average);

    // Format statistics display
    QString statsText = tr("Count: %1 | Avg: %2 | Min: %3 | Max: %4 | StdDev: %5")
        .arg(count)
        .arg(average, 0, 'f', 2)
        .arg(min, 0, 'f', 2)
        .arg(max, 0, 'f', 2)
        .arg(stdDev, 0, 'f', 2);

    mStatisticsLabel->setText(statsText);
}

// Task 104: Filter measurements by type
void QGMDockMeasurements::filterByType(int index) {
    mCurrentFilter = mFilterComboBox->itemText(index);
    updateTableDisplay();
    updateStatistics();
}

// Task 104: Update table display based on current filter
void QGMDockMeasurements::updateTableDisplay() {
    // Temporarily disable sorting while updating to avoid crashes
    bool sortingWasEnabled = mTableWidget->isSortingEnabled();
    mTableWidget->setSortingEnabled(false);

    // Clear table
    mTableWidget->setRowCount(0);

    // Add filtered measurements
    QString filterType = mFilterComboBox->currentText();

    for (const MeasurementData& m : mMeasurements) {
        if (filterType == "All" || m.type == filterType) {
            int row = mTableWidget->rowCount();
            mTableWidget->insertRow(row);

            // Create items with proper numeric sorting for ID and Value columns
            QTableWidgetItem* idItem = new QTableWidgetItem();
            idItem->setData(Qt::DisplayRole, m.id);
            mTableWidget->setItem(row, 0, idItem);

            mTableWidget->setItem(row, 1, new QTableWidgetItem(m.type));

            QTableWidgetItem* valueItem = new QTableWidgetItem();
            valueItem->setData(Qt::DisplayRole, m.value);
            mTableWidget->setItem(row, 2, valueItem);

            mTableWidget->setItem(row, 3, new QTableWidgetItem(m.unit));
            mTableWidget->setItem(row, 4, new QTableWidgetItem(m.timestamp.toString("HH:mm:ss")));
        }
    }

    // Re-enable sorting
    mTableWidget->setSortingEnabled(sortingWasEnabled);
}

// Task 104: Calculate average of values
double QGMDockMeasurements::calculateAverage(const QVector<double>& values) const {
    if (values.isEmpty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (double val : values) {
        sum += val;
    }

    return sum / values.size();
}

// Task 104: Calculate standard deviation
double QGMDockMeasurements::calculateStdDev(const QVector<double>& values, double average) const {
    if (values.size() <= 1) {
        return 0.0;
    }

    double sumSquaredDiff = 0.0;
    for (double val : values) {
        double diff = val - average;
        sumSquaredDiff += diff * diff;
    }

    return std::sqrt(sumSquaredDiff / values.size());
}
