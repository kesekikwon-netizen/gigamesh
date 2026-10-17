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

#ifndef QGMDOCKMEASUREMENTS_H
#define QGMDOCKMEASUREMENTS_H

#include <QDockWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QComboBox>

//!
//! \brief Measurements Panel Dock Widget (Task 101/104)
//!
//! Displays measurement data in a table format with columns:
//! - Number: Measurement ID
//! - Type: Distance/Angle/Area/etc
//! - Value: Measured value
//! - Unit: mm/cm/degrees/etc
//! - Time: Timestamp
//!
//! Task 101: Basic structure creation
//! Task 104: Full implementation with statistics, CSV export, filtering
//!

//! Measurement data structure
struct MeasurementData {
    int id;
    QString type;
    double value;
    QString unit;
    QDateTime timestamp;
};

class QGMDockMeasurements : public QDockWidget {
    Q_OBJECT

public:
    explicit QGMDockMeasurements(QWidget* parent = nullptr);
    ~QGMDockMeasurements() override;

    //! Add a new measurement to the table
    void addMeasurement(const MeasurementData& measurement);

    //! Clear all measurements
    void clearMeasurements();

    //! Get all measurements
    QVector<MeasurementData> getAllMeasurements() const;

public slots:
    //! Export measurements to CSV
    void exportToCSV();

    //! Update statistics display
    void updateStatistics();

    //! Filter measurements by type
    void filterByType(int index);

private:
    void setupUI();
    void updateTableDisplay();

    // Statistics calculation helpers
    double calculateAverage(const QVector<double>& values) const;
    double calculateStdDev(const QVector<double>& values, double average) const;

    QWidget* mContentWidget;
    QVBoxLayout* mMainLayout;
    QTableWidget* mTableWidget;
    QLabel* mStatisticsLabel;
    QComboBox* mFilterComboBox;
    QPushButton* mExportButton;
    QPushButton* mClearButton;

    QVector<MeasurementData> mMeasurements;
    QString mCurrentFilter;  // "All", "Distance", "Angle", "Area", etc.
    int mNextId;
};

#endif // QGMDOCKMEASUREMENTS_H
