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

#ifndef QGMDOCKSECTIONS_H
#define QGMDOCKSECTIONS_H

#include <QDockWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include "sectionmanager.h"

//!
//! \brief Dock widget for managing archaeological sections
//!
//! Displays a list of sections with controls for adding, removing,
//! and exporting sections to SVG.
//!
class QGMDockSections : public QDockWidget
{
	Q_OBJECT

public:
	explicit QGMDockSections(QWidget* parent = nullptr);
	~QGMDockSections();

	void setSectionManager(SectionManager* manager);
	void refreshSectionList();
	void enterEditMode(Section* section);
	void exitEditMode();

signals:
	void sectionSelected(Section* section);
	void createSectionRequested(Section::SectionType type);
	void removeSectionRequested(int index);
	void exportSectionRequested(int index);
	void exportAllSectionsRequested();
	void sectionFinalized(Section* section);
	void sectionEditCanceled();

private slots:
	void onSectionItemClicked(QListWidgetItem* item);
	void onSectionItemDoubleClicked(QListWidgetItem* item);
	void onAddTopSection();
	void onAddFrontSection();
	void onAddSideSection();
	void onAddCustomSection();
	void onRemoveSection();
	void onExportSection();
	void onExportAll();
	void onToggleVisibility();
	void onToggleGrid();

	// Interactive editing slots
	void onTranslationSliderChanged(int value);
	void onRotationSliderChanged(int value);
	void onFinalizeClicked();
	void onCancelClicked();

	// SectionManager slots
	void onSectionAdded(Section* section);
	void onSectionRemoved(int index);
	void onActiveSectionChanged(Section* section);

private:
	void setupUI();
	void updateButtons();
	QListWidgetItem* createSectionItem(Section* section, int index);

	SectionManager* mSectionManager;
	Section* mEditingSection;  //!< Currently editing section

	// UI widgets
	QListWidget* mSectionList;
	QPushButton* mBtnAddTop;
	QPushButton* mBtnAddFront;
	QPushButton* mBtnAddSide;
	QPushButton* mBtnAddCustom;
	QPushButton* mBtnRemove;
	QPushButton* mBtnExport;
	QPushButton* mBtnExportAll;
	QPushButton* mBtnToggleVisible;
	QPushButton* mBtnToggleGrid;

	// Interactive editing UI
	QGroupBox* mEditGroup;
	QSlider* mTranslationSlider;
	QLabel* mTranslationLabel;
	QSlider* mRotationSlider;
	QLabel* mRotationLabel;
	QPushButton* mBtnFinalize;
	QPushButton* mBtnCancel;
};

#endif // QGMDOCKSECTIONS_H
