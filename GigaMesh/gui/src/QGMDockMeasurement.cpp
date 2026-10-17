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

#include "QGMDockMeasurement.h"
#include <QTextStream>
#include <QFile>
#include <iostream>

using namespace std;

//! Constructor
QGMDockMeasurement::QGMDockMeasurement(QWidget *parent) :
	QDockWidget(parent),
	mMainLayout(nullptr),
	mMeasurementTable(nullptr),
	mStatsLabel(nullptr),
	mButtonLayout(nullptr),
	mExportButton(nullptr),
	mClearButton(nullptr),
	mContextMenu(nullptr)
{
	setWindowTitle(tr("Measurement Results"));
	setupUI();
}

//! Destructor
QGMDockMeasurement::~QGMDockMeasurement()
{
	// Qt handles child widget deletion automatically
}

//! Setup the user interface
void QGMDockMeasurement::setupUI()
{
	// Create main widget and layout
	QWidget* mainWidget = new QWidget(this);
	mMainLayout = new QVBoxLayout(mainWidget);
	mMainLayout->setSpacing(6);
	mMainLayout->setContentsMargins(9, 9, 9, 9);

	// Setup components
	setupTable();
	setupStatistics();

	// Setup buttons
	mButtonLayout = new QHBoxLayout();

	mExportButton = new QPushButton(tr("Export CSV"), mainWidget);
	mClearButton = new QPushButton(tr("Clear All"), mainWidget);

	mButtonLayout->addWidget(mExportButton);
	mButtonLayout->addWidget(mClearButton);
	mButtonLayout->addStretch();

	// Add components to main layout
	mMainLayout->addWidget(mMeasurementTable);
	mMainLayout->addWidget(mStatsLabel);
	mMainLayout->addLayout(mButtonLayout);

	// Set the main widget
	setWidget(mainWidget);

	// Setup context menu
	setupContextMenu();

	// Connect signals
	QObject::connect(mMeasurementTable, &QTableWidget::cellChanged,
	                 this, &QGMDockMeasurement::onVisibilityChanged);
	QObject::connect(mMeasurementTable, &QTableWidget::customContextMenuRequested,
	                 this, &QGMDockMeasurement::showContextMenu);
	QObject::connect(mExportButton, &QPushButton::clicked,
	                 this, &QGMDockMeasurement::exportToCSV);
	QObject::connect(mClearButton, &QPushButton::clicked,
	                 this, &QGMDockMeasurement::clearMeasurements);
}

//! Setup the table widget
void QGMDockMeasurement::setupTable()
{
	mMeasurementTable = new QTableWidget(0, COL_COUNT, nullptr);

	// Set column headers
	QStringList headers;
	headers << tr("ID") << tr("Type") << tr("Value") << tr("Unit")
	        << tr("Label") << tr("Visible");
	mMeasurementTable->setHorizontalHeaderLabels(headers);

	// Configure table properties
	mMeasurementTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	mMeasurementTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mMeasurementTable->setAlternatingRowColors(true);
	mMeasurementTable->setContextMenuPolicy(Qt::CustomContextMenu);
	mMeasurementTable->setSortingEnabled(true);

	// Set column widths
	mMeasurementTable->setColumnWidth(COL_ID, 50);
	mMeasurementTable->setColumnWidth(COL_TYPE, 80);
	mMeasurementTable->setColumnWidth(COL_VALUE, 100);
	mMeasurementTable->setColumnWidth(COL_UNIT, 60);
	mMeasurementTable->setColumnWidth(COL_LABEL, 150);
	mMeasurementTable->setColumnWidth(COL_VISIBLE, 70);

	// Stretch the label column
	mMeasurementTable->horizontalHeader()->setStretchLastSection(false);
	mMeasurementTable->horizontalHeader()->setSectionResizeMode(COL_LABEL, QHeaderView::Stretch);

	// Set vertical header
	mMeasurementTable->verticalHeader()->setVisible(false);
}

//! Setup the statistics panel
void QGMDockMeasurement::setupStatistics()
{
	mStatsLabel = new QLabel(tr("Total: 0 | Length Sum: 0.00mm | Area Sum: 0.00mm²"), nullptr);
	mStatsLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	mStatsLabel->setAlignment(Qt::AlignCenter);
	mStatsLabel->setMinimumHeight(30);
}

//! Setup the context menu
void QGMDockMeasurement::setupContextMenu()
{
	mContextMenu = new QMenu(this);

	mCopyAction = new QAction(tr("Copy Measurement"), this);
	mExportAction = new QAction(tr("Export to CSV"), this);
	mDeleteAction = new QAction(tr("Delete Selected"), this);
	mShowAllAction = new QAction(tr("Show All"), this);
	mHideAllAction = new QAction(tr("Hide All"), this);

	mContextMenu->addAction(mCopyAction);
	mContextMenu->addSeparator();
	mContextMenu->addAction(mExportAction);
	mContextMenu->addSeparator();
	mContextMenu->addAction(mDeleteAction);
	mContextMenu->addSeparator();
	mContextMenu->addAction(mShowAllAction);
	mContextMenu->addAction(mHideAllAction);

	// Connect actions
	QObject::connect(mCopyAction, &QAction::triggered,
	                 this, &QGMDockMeasurement::copySelectedMeasurement);
	QObject::connect(mExportAction, &QAction::triggered,
	                 this, &QGMDockMeasurement::exportToCSV);
	QObject::connect(mDeleteAction, &QAction::triggered,
	                 this, &QGMDockMeasurement::deleteSelectedMeasurements);
	QObject::connect(mShowAllAction, &QAction::triggered,
	                 this, &QGMDockMeasurement::showAllMeasurements);
	QObject::connect(mHideAllAction, &QAction::triggered,
	                 this, &QGMDockMeasurement::hideAllMeasurements);
}

//! Add a new measurement to the table
void QGMDockMeasurement::addMeasurement(const Measurement& m)
{
	// Add to internal storage
	mMeasurements.append(m);

	// Add to table
	int row = mMeasurementTable->rowCount();
	mMeasurementTable->insertRow(row);

	// Disable sorting while adding items to prevent issues
	mMeasurementTable->setSortingEnabled(false);

	// Add data items
	QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(m.id));
	idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
	mMeasurementTable->setItem(row, COL_ID, idItem);

	QTableWidgetItem* typeItem = new QTableWidgetItem(m.typeName());
	typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
	mMeasurementTable->setItem(row, COL_TYPE, typeItem);

	QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(m.value, 'f', 2));
	valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
	valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mMeasurementTable->setItem(row, COL_VALUE, valueItem);

	QTableWidgetItem* unitItem = new QTableWidgetItem(m.unit);
	unitItem->setFlags(unitItem->flags() & ~Qt::ItemIsEditable);
	mMeasurementTable->setItem(row, COL_UNIT, unitItem);

	QTableWidgetItem* labelItem = new QTableWidgetItem(m.label);
	mMeasurementTable->setItem(row, COL_LABEL, labelItem);

	// Add checkbox for visibility
	QCheckBox* visibleCheckbox = new QCheckBox();
	visibleCheckbox->setChecked(m.visible);

	// Center the checkbox in the cell
	QWidget* checkboxWidget = new QWidget();
	QHBoxLayout* checkboxLayout = new QHBoxLayout(checkboxWidget);
	checkboxLayout->addWidget(visibleCheckbox);
	checkboxLayout->setAlignment(Qt::AlignCenter);
	checkboxLayout->setContentsMargins(0, 0, 0, 0);

	mMeasurementTable->setCellWidget(row, COL_VISIBLE, checkboxWidget);

	// Connect checkbox signal
	QObject::connect(visibleCheckbox, &QCheckBox::stateChanged,
	                 [this, m](int state) {
		                 emit measurementVisibilityChanged(m.id, state == Qt::Checked);
	                 });

	// Re-enable sorting
	mMeasurementTable->setSortingEnabled(true);

	// Update statistics
	updateStatistics();

	cout << "[QGMDockMeasurement::addMeasurement] Added measurement ID " << m.id
	     << " (" << m.typeName().toStdString() << ")" << endl;
}

//! Remove a measurement by ID
void QGMDockMeasurement::removeMeasurement(int id)
{
	// Find and remove from internal storage
	for (int i = 0; i < mMeasurements.size(); ++i) {
		if (mMeasurements[i].id == id) {
			mMeasurements.removeAt(i);
			break;
		}
	}

	// Find and remove from table
	for (int row = 0; row < mMeasurementTable->rowCount(); ++row) {
		QTableWidgetItem* idItem = mMeasurementTable->item(row, COL_ID);
		if (idItem && idItem->text().toInt() == id) {
			mMeasurementTable->removeRow(row);
			break;
		}
	}

	updateStatistics();

	cout << "[QGMDockMeasurement::removeMeasurement] Removed measurement ID " << id << endl;
}

//! Clear all measurements
void QGMDockMeasurement::clearMeasurements()
{
	mMeasurements.clear();
	mMeasurementTable->setRowCount(0);
	updateStatistics();

	cout << "[QGMDockMeasurement::clearMeasurements] Cleared all measurements" << endl;
}

//! Update the statistics display
void QGMDockMeasurement::updateStatistics()
{
	int totalCount = mMeasurements.size();
	double totalLength = 0.0;
	double totalArea = 0.0;
	int angleCount = 0;

	for (const auto& m : mMeasurements) {
		if (m.type == MeasurementType::Distance) {
			totalLength += m.value;
		} else if (m.type == MeasurementType::Area) {
			totalArea += m.value;
		} else if (m.type == MeasurementType::Angle) {
			angleCount++;
		}
	}

	QString statsText = tr("Total: %1 | Length Sum: %2mm | Area Sum: %3mm²")
		.arg(totalCount)
		.arg(totalLength, 0, 'f', 2)
		.arg(totalArea, 0, 'f', 2);

	if (angleCount > 0) {
		statsText += tr(" | Angles: %1").arg(angleCount);
	}

	mStatsLabel->setText(statsText);
}

//! Handle visibility checkbox changes (currently unused but kept for future editable cells)
void QGMDockMeasurement::onVisibilityChanged(int row, int column)
{
	// This slot is for handling cell changes
	// Visibility is handled through checkbox signals
	Q_UNUSED(row);
	Q_UNUSED(column);
}

//! Show context menu
void QGMDockMeasurement::showContextMenu(const QPoint& pos)
{
	QPoint globalPos = mMeasurementTable->viewport()->mapToGlobal(pos);

	// Enable/disable actions based on selection
	bool hasSelection = !mMeasurementTable->selectedItems().isEmpty();
	mCopyAction->setEnabled(hasSelection);
	mDeleteAction->setEnabled(hasSelection);

	mContextMenu->exec(globalPos);
}

//! Copy selected measurement
void QGMDockMeasurement::copySelectedMeasurement()
{
	QList<QTableWidgetItem*> selectedItems = mMeasurementTable->selectedItems();
	if (selectedItems.isEmpty()) {
		return;
	}

	// Get the first selected row
	int row = selectedItems.first()->row();

	// Build a string with all measurement data
	QString copyText;
	copyText += tr("ID: ") + mMeasurementTable->item(row, COL_ID)->text() + "\n";
	copyText += tr("Type: ") + mMeasurementTable->item(row, COL_TYPE)->text() + "\n";
	copyText += tr("Value: ") + mMeasurementTable->item(row, COL_VALUE)->text() + " ";
	copyText += mMeasurementTable->item(row, COL_UNIT)->text() + "\n";
	copyText += tr("Label: ") + mMeasurementTable->item(row, COL_LABEL)->text();

	// Copy to clipboard
	QApplication::clipboard()->setText(copyText);

	cout << "[QGMDockMeasurement::copySelectedMeasurement] Copied measurement to clipboard" << endl;
}

//! Export to CSV
void QGMDockMeasurement::exportToCSV()
{
	if (mMeasurements.isEmpty()) {
		QMessageBox::information(this, tr("Export CSV"),
		                         tr("No measurements to export."));
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export Measurements to CSV"),
		"measurements.csv",
		tr("CSV Files (*.csv);;All Files (*)"));

	if (fileName.isEmpty()) {
		return;
	}

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, tr("Export Error"),
		                      tr("Could not open file for writing: %1").arg(fileName));
		return;
	}

	QTextStream out(&file);

	// Write UTF-8 BOM for Excel compatibility
	out.setCodec("UTF-8");
	out << "\xEF\xBB\xBF";

	// Write header
	out << "ID,Type,Value,Unit,Label,Visible\n";

	// Write data
	for (const auto& m : mMeasurements) {
		out << m.id << ",";
		out << "\"" << m.typeName() << "\",";
		out << QString::number(m.value, 'f', 2) << ",";
		out << "\"" << m.unit << "\",";
		out << "\"" << m.label << "\",";
		out << (m.visible ? "Yes" : "No") << "\n";
	}

	file.close();

	QMessageBox::information(this, tr("Export Complete"),
	                         tr("Exported %1 measurements to:\n%2")
	                         .arg(mMeasurements.size()).arg(fileName));

	cout << "[QGMDockMeasurement::exportToCSV] Exported " << mMeasurements.size()
	     << " measurements to " << fileName.toStdString() << endl;
}

//! Delete selected measurements
void QGMDockMeasurement::deleteSelectedMeasurements()
{
	QList<QTableWidgetItem*> selectedItems = mMeasurementTable->selectedItems();
	if (selectedItems.isEmpty()) {
		return;
	}

	// Get unique row indices
	QSet<int> rowsToDelete;
	for (auto* item : selectedItems) {
		rowsToDelete.insert(item->row());
	}

	// Confirm deletion
	int count = rowsToDelete.size();
	QMessageBox::StandardButton reply = QMessageBox::question(this,
		tr("Delete Measurements"),
		tr("Are you sure you want to delete %1 measurement(s)?").arg(count),
		QMessageBox::Yes | QMessageBox::No);

	if (reply != QMessageBox::Yes) {
		return;
	}

	// Collect IDs to delete
	QVector<int> idsToDelete;
	for (int row : rowsToDelete) {
		QTableWidgetItem* idItem = mMeasurementTable->item(row, COL_ID);
		if (idItem) {
			idsToDelete.append(idItem->text().toInt());
		}
	}

	// Delete measurements
	for (int id : idsToDelete) {
		removeMeasurement(id);
		emit measurementDeleted(id);
	}

	cout << "[QGMDockMeasurement::deleteSelectedMeasurements] Deleted "
	     << count << " measurements" << endl;
}

//! Show all measurements
void QGMDockMeasurement::showAllMeasurements()
{
	for (int row = 0; row < mMeasurementTable->rowCount(); ++row) {
		QWidget* widget = mMeasurementTable->cellWidget(row, COL_VISIBLE);
		if (widget) {
			QCheckBox* checkbox = widget->findChild<QCheckBox*>();
			if (checkbox) {
				checkbox->setChecked(true);
			}
		}
	}

	// Update internal storage
	for (auto& m : mMeasurements) {
		m.visible = true;
	}

	cout << "[QGMDockMeasurement::showAllMeasurements] Set all measurements visible" << endl;
}

//! Hide all measurements
void QGMDockMeasurement::hideAllMeasurements()
{
	for (int row = 0; row < mMeasurementTable->rowCount(); ++row) {
		QWidget* widget = mMeasurementTable->cellWidget(row, COL_VISIBLE);
		if (widget) {
			QCheckBox* checkbox = widget->findChild<QCheckBox*>();
			if (checkbox) {
				checkbox->setChecked(false);
			}
		}
	}

	// Update internal storage
	for (auto& m : mMeasurements) {
		m.visible = false;
	}

	cout << "[QGMDockMeasurement::hideAllMeasurements] Set all measurements hidden" << endl;
}

//! Get the next available measurement ID
int QGMDockMeasurement::getNextMeasurementId() const
{
	int maxId = 0;
	for (const auto& m : mMeasurements) {
		if (m.id > maxId) {
			maxId = m.id;
		}
	}
	return maxId + 1;
}

//! Handle change events (for translations)
void QGMDockMeasurement::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Update window title
		setWindowTitle(tr("Measurement Results"));

		// Update table headers
		QStringList headers;
		headers << tr("ID") << tr("Type") << tr("Value") << tr("Unit")
		        << tr("Label") << tr("Visible");
		mMeasurementTable->setHorizontalHeaderLabels(headers);

		// Update buttons
		mExportButton->setText(tr("Export CSV"));
		mClearButton->setText(tr("Clear All"));

		// Update context menu actions
		mCopyAction->setText(tr("Copy Measurement"));
		mExportAction->setText(tr("Export to CSV"));
		mDeleteAction->setText(tr("Delete Selected"));
		mShowAllAction->setText(tr("Show All"));
		mHideAllAction->setText(tr("Hide All"));

		// Update statistics
		updateStatistics();
	}

	QDockWidget::changeEvent(event);
}
