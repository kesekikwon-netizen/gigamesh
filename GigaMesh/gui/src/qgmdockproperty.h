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

#ifndef QGMDOCKPROPERTY_H
#define QGMDOCKPROPERTY_H

#include <QDockWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QGroupBox>
#include <QScrollArea>
#include <QWidget>

// Forward declarations
class Mesh;
class Section;
class PolyLine;

//!
//! \brief Dock widget for displaying and editing object properties
//!
//! Provides a property panel that displays detailed information about
//! selected objects (mesh, sections, measurements, annotations) and
//! allows real-time editing of certain properties like color, thickness,
//! and text content.
//!
class QGMDockProperty : public QDockWidget
{
	Q_OBJECT

public:
	explicit QGMDockProperty(QWidget* parent = nullptr);
	~QGMDockProperty();

public slots:
	// Update property displays
	void updateMeshProperties(Mesh* mesh);
	void updateSectionProperties(Section* section);
	void updatePolyLineProperties(PolyLine* polyline, const QString& label);
	void clearProperties();

	// Multi-selection
	void updateMultipleSelection(int count, const QString& type);

signals:
	// Property change signals
	void sectionColorChanged(Section* section, const QColor& color);
	void sectionLineWidthChanged(Section* section, double width);
	void sectionNameChanged(Section* section, const QString& name);
	void sectionVisibilityChanged(Section* section, bool visible);
	void sectionGridChanged(Section* section, bool enabled, double spacing);
	void polyLineColorChanged(PolyLine* polyline, const QColor& color);
	void polyLineWidthChanged(PolyLine* polyline, double width);

private slots:
	// Mesh property slots (read-only, no editing)
	// None needed - mesh properties are display-only

	// Section property editing slots
	void onSectionNameChanged();
	void onSectionColorClicked();
	void onSectionLineWidthChanged(double value);
	void onSectionVisibilityToggled(bool checked);
	void onSectionGridToggled(bool checked);
	void onSectionGridSpacingChanged(double value);

	// PolyLine property editing slots
	void onPolyLineColorClicked();
	void onPolyLineWidthChanged(double value);

private:
	void setupUI();
	void createMeshPropertyWidgets();
	void createSectionPropertyWidgets();
	void createPolyLinePropertyWidgets();
	void createCommonPropertyWidgets();

	void showMeshProperties();
	void showSectionProperties();
	void showPolyLineProperties();
	void showMultipleSelectionProperties();
	void hideAllPropertyGroups();

	// Helper methods
	QString formatNumber(double value, int decimals = 3) const;
	QString formatInteger(uint64_t value) const;
	QColor getColorFromButton(QPushButton* button) const;
	void setButtonColor(QPushButton* button, const QColor& color);

	// Main layout
	QScrollArea* mScrollArea;
	QWidget* mContentWidget;
	QFormLayout* mMainLayout;

	// Property group widgets
	QGroupBox* mMeshGroup;
	QGroupBox* mSectionGroup;
	QGroupBox* mPolyLineGroup;
	QGroupBox* mMultiSelectGroup;

	// Mesh property widgets (read-only)
	QFormLayout* mMeshLayout;
	QLabel* mMeshVertexCount;
	QLabel* mMeshFaceCount;
	QLabel* mMeshBoundingBoxX;
	QLabel* mMeshBoundingBoxY;
	QLabel* mMeshBoundingBoxZ;
	QLabel* mMeshVolume;
	QLabel* mMeshSurfaceArea;
	QLabel* mMeshEdgeLenMin;
	QLabel* mMeshEdgeLenMax;

	// Section property widgets (editable)
	QFormLayout* mSectionLayout;
	QLineEdit* mSectionName;
	QPushButton* mSectionColorButton;
	QDoubleSpinBox* mSectionLineWidth;
	QPushButton* mSectionVisibilityButton;
	QLabel* mSectionType;
	QLabel* mSectionPlanePosition;
	QLabel* mSectionPlaneNormal;
	QLabel* mSectionIntersectionLength;
	QPushButton* mSectionGridButton;
	QDoubleSpinBox* mSectionGridSpacing;

	// PolyLine property widgets (editable)
	QFormLayout* mPolyLineLayout;
	QLabel* mPolyLineLabel;
	QLabel* mPolyLineVertexCount;
	QLabel* mPolyLineLength;
	QPushButton* mPolyLineColorButton;
	QDoubleSpinBox* mPolyLineWidth;

	// Multi-selection widgets
	QFormLayout* mMultiSelectLayout;
	QLabel* mMultiSelectCount;
	QLabel* mMultiSelectType;

	// Current objects being displayed
	Mesh* mCurrentMesh;
	Section* mCurrentSection;
	PolyLine* mCurrentPolyLine;

	// QWidget interface
protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKPROPERTY_H
