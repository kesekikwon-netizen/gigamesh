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

#ifndef QGMDOCKMEASUREMENT_H
#define QGMDOCKMEASUREMENT_H

#include <QDockWidget>
#include <QTableWidget>
#include <QLabel>
#include <QCheckBox>
#include <QMenu>
#include <QAction>
#include <QString>
#include <QVector>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>

//! Measurement type enumeration
enum class MeasurementType {
	Distance,
	Angle,
	Area
};

//! Measurement data structure
struct Measurement {
	int id;
	MeasurementType type;
	double value;
	QString unit;
	QString label;
	bool visible;

	//! Get the type name as a string
	QString typeName() const {
		switch (type) {
			case MeasurementType::Distance:
				return QObject::tr("Distance");
			case MeasurementType::Angle:
				return QObject::tr("Angle");
			case MeasurementType::Area:
				return QObject::tr("Area");
			default:
				return QObject::tr("Unknown");
		}
	}
};

//! QGMDockMeasurement - Measurement Results Panel
//! Displays all measurements in table format with statistics
class QGMDockMeasurement : public QDockWidget
{
	Q_OBJECT

public:
	explicit QGMDockMeasurement(QWidget *parent = nullptr);
	~QGMDockMeasurement();

	//! Add a new measurement to the table
	void addMeasurement(const Measurement& m);

	//! Remove a measurement by ID
	void removeMeasurement(int id);

	//! Clear all measurements
	void clearMeasurements();

	//! Get all measurements
	const QVector<Measurement>& getMeasurements() const { return mMeasurements; }

public slots:
	//! Update the statistics display
	void updateStatistics();

	//! Handle visibility checkbox changes
	void onVisibilityChanged(int row, int column);

	//! Show context menu
	void showContextMenu(const QPoint& pos);

	//! Copy selected measurement
	void copySelectedMeasurement();

	//! Export to CSV
	void exportToCSV();

	//! Delete selected measurements
	void deleteSelectedMeasurements();

	//! Show all measurements
	void showAllMeasurements();

	//! Hide all measurements
	void hideAllMeasurements();

signals:
	//! Emitted when a measurement's visibility changes
	void measurementVisibilityChanged(int id, bool visible);

	//! Emitted when a measurement is deleted
	void measurementDeleted(int id);

private:
	//! Setup the user interface
	void setupUI();

	//! Setup the table widget
	void setupTable();

	//! Setup the statistics panel
	void setupStatistics();

	//! Setup the context menu
	void setupContextMenu();

	//! Get the next available measurement ID
	int getNextMeasurementId() const;

	//! Main layout
	QVBoxLayout* mMainLayout;

	//! Table widget for displaying measurements
	QTableWidget* mMeasurementTable;

	//! Statistics label
	QLabel* mStatsLabel;

	//! Button layout
	QHBoxLayout* mButtonLayout;

	//! Export button
	QPushButton* mExportButton;

	//! Clear button
	QPushButton* mClearButton;

	//! Context menu
	QMenu* mContextMenu;

	//! Context menu actions
	QAction* mCopyAction;
	QAction* mExportAction;
	QAction* mDeleteAction;
	QAction* mShowAllAction;
	QAction* mHideAllAction;

	//! Storage for all measurements
	QVector<Measurement> mMeasurements;

	//! Column indices
	enum ColumnIndex {
		COL_ID = 0,
		COL_TYPE = 1,
		COL_VALUE = 2,
		COL_UNIT = 3,
		COL_LABEL = 4,
		COL_VISIBLE = 5,
		COL_COUNT = 6
	};

protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKMEASUREMENT_H
