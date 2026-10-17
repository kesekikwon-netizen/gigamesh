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

#include "qgmdockproperty.h"
#include <GigaMesh/mesh/mesh.h>
#include "section.h"
#include <GigaMesh/mesh/polyline.h>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLocale>
#include <QEvent>
#include <cmath>

//! Constructor
QGMDockProperty::QGMDockProperty(QWidget* parent)
	: QDockWidget(parent),
	  mScrollArea(nullptr),
	  mContentWidget(nullptr),
	  mMainLayout(nullptr),
	  mMeshGroup(nullptr),
	  mSectionGroup(nullptr),
	  mPolyLineGroup(nullptr),
	  mMultiSelectGroup(nullptr),
	  mCurrentMesh(nullptr),
	  mCurrentSection(nullptr),
	  mCurrentPolyLine(nullptr)
{
	std::cout << "[QGMDockProperty::QGMDockProperty] Constructor started" << std::endl;
	setWindowTitle(tr("Properties"));
	setObjectName("QGMDockProperty");

	std::cout << "[QGMDockProperty::QGMDockProperty] About to call setupUI()..." << std::endl;
	setupUI();
	std::cout << "[QGMDockProperty::QGMDockProperty] Constructor completed successfully" << std::endl;
}

//! Destructor
QGMDockProperty::~QGMDockProperty()
{
	// Qt parent-child relationship handles cleanup
}

//! Setup the user interface
void QGMDockProperty::setupUI()
{
	std::cout << "[QGMDockProperty::setupUI] Creating scroll area..." << std::endl;
	// Create scroll area for properties
	mScrollArea = new QScrollArea(this);
	mScrollArea->setWidgetResizable(true);
	mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	std::cout << "[QGMDockProperty::setupUI] Creating content widget..." << std::endl;
	// Create content widget
	mContentWidget = new QWidget();
	mMainLayout = new QFormLayout(mContentWidget);
	mMainLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	mMainLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

	// Create property widgets for different object types
	std::cout << "[QGMDockProperty::setupUI] Calling createMeshPropertyWidgets()..." << std::endl;
	createMeshPropertyWidgets();
	std::cout << "[QGMDockProperty::setupUI] Calling createSectionPropertyWidgets()..." << std::endl;
	createSectionPropertyWidgets();
	std::cout << "[QGMDockProperty::setupUI] Calling createPolyLinePropertyWidgets()..." << std::endl;
	createPolyLinePropertyWidgets();
	std::cout << "[QGMDockProperty::setupUI] Calling createCommonPropertyWidgets()..." << std::endl;
	createCommonPropertyWidgets();

	std::cout << "[QGMDockProperty::setupUI] Hiding all property groups..." << std::endl;
	// Initially hide all groups
	hideAllPropertyGroups();

	std::cout << "[QGMDockProperty::setupUI] Setting widget..." << std::endl;
	mScrollArea->setWidget(mContentWidget);
	setWidget(mScrollArea);

	setMinimumWidth(250);
	std::cout << "[QGMDockProperty::setupUI] setupUI completed successfully" << std::endl;
}

//! Create mesh property widgets (read-only)
void QGMDockProperty::createMeshPropertyWidgets()
{
	mMeshGroup = new QGroupBox(tr("Mesh Properties"), mContentWidget);
	mMeshLayout = new QFormLayout(mMeshGroup);
	mMeshLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	// Vertex and face counts
	mMeshVertexCount = new QLabel("0");
	mMeshVertexCount->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Vertices:"), mMeshVertexCount);

	mMeshFaceCount = new QLabel("0");
	mMeshFaceCount->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Faces:"), mMeshFaceCount);

	// Bounding box
	mMeshBoundingBoxX = new QLabel("0.0");
	mMeshBoundingBoxX->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Bounding Box X:"), mMeshBoundingBoxX);

	mMeshBoundingBoxY = new QLabel("0.0");
	mMeshBoundingBoxY->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Bounding Box Y:"), mMeshBoundingBoxY);

	mMeshBoundingBoxZ = new QLabel("0.0");
	mMeshBoundingBoxZ->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Bounding Box Z:"), mMeshBoundingBoxZ);

	// Edge lengths
	mMeshEdgeLenMin = new QLabel("0.0");
	mMeshEdgeLenMin->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Min Edge Length:"), mMeshEdgeLenMin);

	mMeshEdgeLenMax = new QLabel("0.0");
	mMeshEdgeLenMax->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Max Edge Length:"), mMeshEdgeLenMax);

	// Volume (placeholder - requires implementation in Mesh class)
	mMeshVolume = new QLabel(tr("N/A"));
	mMeshVolume->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Volume:"), mMeshVolume);

	// Surface area (placeholder - requires implementation in Mesh class)
	mMeshSurfaceArea = new QLabel(tr("N/A"));
	mMeshSurfaceArea->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mMeshLayout->addRow(tr("Surface Area:"), mMeshSurfaceArea);

	mMainLayout->addRow(mMeshGroup);
	mMeshGroup->hide();
}

//! Create section property widgets (editable)
void QGMDockProperty::createSectionPropertyWidgets()
{
	mSectionGroup = new QGroupBox(tr("Section Properties"), mContentWidget);
	mSectionLayout = new QFormLayout(mSectionGroup);
	mSectionLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	// Section name (editable)
	mSectionName = new QLineEdit();
	mSectionLayout->addRow(tr("Name:"), mSectionName);
	connect(mSectionName, &QLineEdit::editingFinished, this, &QGMDockProperty::onSectionNameChanged);

	// Section type (read-only)
	mSectionType = new QLabel("");
	mSectionType->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mSectionLayout->addRow(tr("Type:"), mSectionType);

	// Visibility toggle
	mSectionVisibilityButton = new QPushButton(tr("Visible"));
	mSectionVisibilityButton->setCheckable(true);
	mSectionLayout->addRow(tr("Visibility:"), mSectionVisibilityButton);
	connect(mSectionVisibilityButton, &QPushButton::toggled, this, &QGMDockProperty::onSectionVisibilityToggled);

	// Color picker
	mSectionColorButton = new QPushButton(tr("Choose Color"));
	mSectionColorButton->setMinimumHeight(30);
	mSectionLayout->addRow(tr("Color:"), mSectionColorButton);
	connect(mSectionColorButton, &QPushButton::clicked, this, &QGMDockProperty::onSectionColorClicked);

	// Line width
	mSectionLineWidth = new QDoubleSpinBox();
	mSectionLineWidth->setRange(0.1, 10.0);
	mSectionLineWidth->setSingleStep(0.1);
	mSectionLineWidth->setDecimals(1);
	mSectionLineWidth->setSuffix(" px");
	mSectionLayout->addRow(tr("Line Width:"), mSectionLineWidth);
	connect(mSectionLineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &QGMDockProperty::onSectionLineWidthChanged);

	// Plane position (read-only)
	mSectionPlanePosition = new QLabel("(0.0, 0.0, 0.0)");
	mSectionPlanePosition->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mSectionLayout->addRow(tr("Plane Position:"), mSectionPlanePosition);

	// Plane normal (read-only)
	mSectionPlaneNormal = new QLabel("(0.0, 0.0, 1.0)");
	mSectionPlaneNormal->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mSectionLayout->addRow(tr("Plane Normal:"), mSectionPlaneNormal);

	// Intersection length (read-only)
	mSectionIntersectionLength = new QLabel("0.0");
	mSectionIntersectionLength->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mSectionLayout->addRow(tr("Intersection Length:"), mSectionIntersectionLength);

	// Grid settings
	mSectionGridButton = new QPushButton(tr("Grid Enabled"));
	mSectionGridButton->setCheckable(true);
	mSectionLayout->addRow(tr("Grid:"), mSectionGridButton);
	connect(mSectionGridButton, &QPushButton::toggled, this, &QGMDockProperty::onSectionGridToggled);

	mSectionGridSpacing = new QDoubleSpinBox();
	mSectionGridSpacing->setRange(0.1, 100.0);
	mSectionGridSpacing->setSingleStep(1.0);
	mSectionGridSpacing->setDecimals(1);
	mSectionGridSpacing->setSuffix(" cm");
	mSectionLayout->addRow(tr("Grid Spacing:"), mSectionGridSpacing);
	connect(mSectionGridSpacing, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &QGMDockProperty::onSectionGridSpacingChanged);

	mMainLayout->addRow(mSectionGroup);
	mSectionGroup->hide();
}

//! Create polyline property widgets (editable)
void QGMDockProperty::createPolyLinePropertyWidgets()
{
	mPolyLineGroup = new QGroupBox(tr("Polyline Properties"), mContentWidget);
	mPolyLineLayout = new QFormLayout(mPolyLineGroup);
	mPolyLineLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	// Label (read-only)
	mPolyLineLabel = new QLabel("");
	mPolyLineLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mPolyLineLayout->addRow(tr("Label:"), mPolyLineLabel);

	// Vertex count (read-only)
	mPolyLineVertexCount = new QLabel("0");
	mPolyLineVertexCount->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mPolyLineLayout->addRow(tr("Vertices:"), mPolyLineVertexCount);

	// Length (read-only)
	mPolyLineLength = new QLabel("0.0");
	mPolyLineLength->setTextInteractionFlags(Qt::TextSelectableByMouse);
	mPolyLineLayout->addRow(tr("Length:"), mPolyLineLength);

	// Color picker
	mPolyLineColorButton = new QPushButton(tr("Choose Color"));
	mPolyLineColorButton->setMinimumHeight(30);
	mPolyLineLayout->addRow(tr("Color:"), mPolyLineColorButton);
	connect(mPolyLineColorButton, &QPushButton::clicked, this, &QGMDockProperty::onPolyLineColorClicked);

	// Line width
	mPolyLineWidth = new QDoubleSpinBox();
	mPolyLineWidth->setRange(0.1, 10.0);
	mPolyLineWidth->setSingleStep(0.1);
	mPolyLineWidth->setDecimals(1);
	mPolyLineWidth->setSuffix(" px");
	mPolyLineLayout->addRow(tr("Line Width:"), mPolyLineWidth);
	connect(mPolyLineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &QGMDockProperty::onPolyLineWidthChanged);

	mMainLayout->addRow(mPolyLineGroup);
	mPolyLineGroup->hide();
}

//! Create common property widgets (for multi-selection)
void QGMDockProperty::createCommonPropertyWidgets()
{
	mMultiSelectGroup = new QGroupBox(tr("Multiple Selection"), mContentWidget);
	mMultiSelectLayout = new QFormLayout(mMultiSelectGroup);
	mMultiSelectLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

	mMultiSelectCount = new QLabel("0");
	mMultiSelectLayout->addRow(tr("Selected:"), mMultiSelectCount);

	mMultiSelectType = new QLabel("");
	mMultiSelectLayout->addRow(tr("Type:"), mMultiSelectType);

	mMainLayout->addRow(mMultiSelectGroup);
	mMultiSelectGroup->hide();
}

//! Update mesh properties display
void QGMDockProperty::updateMeshProperties(Mesh* mesh)
{
	if (!mesh) {
		clearProperties();
		return;
	}

	mCurrentMesh = mesh;
	mCurrentSection = nullptr;
	mCurrentPolyLine = nullptr;

	// Update vertex and face counts
	mMeshVertexCount->setText(formatInteger(mesh->getVertexNr()));
	mMeshFaceCount->setText(formatInteger(mesh->getFaceNr()));

	// Update bounding box
	Vector3D bbSize;
	if (mesh->getBoundingBoxSize(bbSize)) {
		mMeshBoundingBoxX->setText(formatNumber(bbSize.getX()) + " units");
		mMeshBoundingBoxY->setText(formatNumber(bbSize.getY()) + " units");
		mMeshBoundingBoxZ->setText(formatNumber(bbSize.getZ()) + " units");
	}

	// Update edge lengths
	double edgeLenMin = mesh->getEdgeLenMin();
	double edgeLenMax = mesh->getEdgeLenMax();
	mMeshEdgeLenMin->setText(formatNumber(edgeLenMin) + " units");
	mMeshEdgeLenMax->setText(formatNumber(edgeLenMax) + " units");

	// Volume and surface area (not yet implemented in Mesh class)
	mMeshVolume->setText(tr("N/A"));
	mMeshSurfaceArea->setText(tr("N/A"));

	showMeshProperties();
}

//! Update section properties display
void QGMDockProperty::updateSectionProperties(Section* section)
{
	if (!section) {
		clearProperties();
		return;
	}

	mCurrentMesh = nullptr;
	mCurrentSection = section;
	mCurrentPolyLine = nullptr;

	// Block signals while updating
	mSectionName->blockSignals(true);
	mSectionLineWidth->blockSignals(true);
	mSectionVisibilityButton->blockSignals(true);
	mSectionGridButton->blockSignals(true);
	mSectionGridSpacing->blockSignals(true);

	// Update section name
	mSectionName->setText(section->getName());

	// Update section type
	QString typeStr;
	switch (section->getType()) {
		case Section::SECTION_CUSTOM:      typeStr = tr("Custom"); break;
		case Section::SECTION_TOP:         typeStr = tr("Top View"); break;
		case Section::SECTION_FRONT:       typeStr = tr("Front View"); break;
		case Section::SECTION_SIDE_RIGHT:  typeStr = tr("Right Side"); break;
		case Section::SECTION_SIDE_LEFT:   typeStr = tr("Left Side"); break;
		case Section::SECTION_OBLIQUE:     typeStr = tr("Oblique"); break;
		default:                           typeStr = tr("Unknown"); break;
	}
	mSectionType->setText(typeStr);

	// Update visibility
	mSectionVisibilityButton->setChecked(section->isVisible());
	mSectionVisibilityButton->setText(section->isVisible() ? tr("Visible") : tr("Hidden"));

	// Update color
	setButtonColor(mSectionColorButton, section->getColor());

	// Update line width
	mSectionLineWidth->setValue(section->getLineWidth());

	// Update plane information
	Vector3D planeHNF = section->getPlaneHNF();
	// HNF format: (nx, ny, nz, d) where d is distance from origin
	mSectionPlaneNormal->setText(QString("(%1, %2, %3)")
	                             .arg(formatNumber(planeHNF.getX()))
	                             .arg(formatNumber(planeHNF.getY()))
	                             .arg(formatNumber(planeHNF.getZ())));
	mSectionPlanePosition->setText(tr("Distance: ") + formatNumber(planeHNF.getH()));

	// Update intersection length
	if (section->hasPolyLine()) {
		PolyLine* polyline = section->getPolyLine();
		double length = 0.0;
		polyline->getLengthAbs(&length);
		mSectionIntersectionLength->setText(formatNumber(length) + " units");
	} else {
		mSectionIntersectionLength->setText(tr("No intersection"));
	}

	// Update grid settings
	mSectionGridButton->setChecked(section->isGridEnabled());
	mSectionGridButton->setText(section->isGridEnabled() ? tr("Grid Enabled") : tr("Grid Disabled"));
	mSectionGridSpacing->setValue(section->getGridSpacing());

	// Unblock signals
	mSectionName->blockSignals(false);
	mSectionLineWidth->blockSignals(false);
	mSectionVisibilityButton->blockSignals(false);
	mSectionGridButton->blockSignals(false);
	mSectionGridSpacing->blockSignals(false);

	showSectionProperties();
}

//! Update polyline properties display
void QGMDockProperty::updatePolyLineProperties(PolyLine* polyline, const QString& label)
{
	if (!polyline) {
		clearProperties();
		return;
	}

	mCurrentMesh = nullptr;
	mCurrentSection = nullptr;
	mCurrentPolyLine = polyline;

	// Block signals while updating
	mPolyLineWidth->blockSignals(true);

	// Update label
	mPolyLineLabel->setText(label.isEmpty() ? tr("Unnamed") : label);

	// Update vertex count
	mPolyLineVertexCount->setText(formatInteger(polyline->length()));

	// Update length
	double length = 0.0;
	polyline->getLengthAbs(&length);
	mPolyLineLength->setText(formatNumber(length) + " units");

	// Update color (PolyLine uses RGB color)
	// Note: PolyLine class may not have color property, this is a placeholder
	// setButtonColor(mPolyLineColorButton, QColor(255, 0, 0));

	// Update line width (placeholder - PolyLine may not have this property)
	mPolyLineWidth->setValue(1.0);

	// Unblock signals
	mPolyLineWidth->blockSignals(false);

	showPolyLineProperties();
}

//! Clear all properties
void QGMDockProperty::clearProperties()
{
	mCurrentMesh = nullptr;
	mCurrentSection = nullptr;
	mCurrentPolyLine = nullptr;
	hideAllPropertyGroups();
}

//! Update display for multiple selection
void QGMDockProperty::updateMultipleSelection(int count, const QString& type)
{
	mCurrentMesh = nullptr;
	mCurrentSection = nullptr;
	mCurrentPolyLine = nullptr;

	mMultiSelectCount->setText(QString::number(count));
	mMultiSelectType->setText(type);

	showMultipleSelectionProperties();
}

// ============================================================================
// Private slot implementations - Section properties
// ============================================================================

void QGMDockProperty::onSectionNameChanged()
{
	if (mCurrentSection) {
		QString newName = mSectionName->text();
		emit sectionNameChanged(mCurrentSection, newName);
	}
}

void QGMDockProperty::onSectionColorClicked()
{
	if (!mCurrentSection) {
		return;
	}

	QColor currentColor = getColorFromButton(mSectionColorButton);
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Choose Section Color"));

	if (newColor.isValid() && newColor != currentColor) {
		setButtonColor(mSectionColorButton, newColor);
		emit sectionColorChanged(mCurrentSection, newColor);
	}
}

void QGMDockProperty::onSectionLineWidthChanged(double value)
{
	if (mCurrentSection) {
		emit sectionLineWidthChanged(mCurrentSection, value);
	}
}

void QGMDockProperty::onSectionVisibilityToggled(bool checked)
{
	if (mCurrentSection) {
		mSectionVisibilityButton->setText(checked ? tr("Visible") : tr("Hidden"));
		emit sectionVisibilityChanged(mCurrentSection, checked);
	}
}

void QGMDockProperty::onSectionGridToggled(bool checked)
{
	if (mCurrentSection) {
		mSectionGridButton->setText(checked ? tr("Grid Enabled") : tr("Grid Disabled"));
		double spacing = mSectionGridSpacing->value();
		emit sectionGridChanged(mCurrentSection, checked, spacing);
	}
}

void QGMDockProperty::onSectionGridSpacingChanged(double value)
{
	if (mCurrentSection) {
		bool enabled = mSectionGridButton->isChecked();
		emit sectionGridChanged(mCurrentSection, enabled, value);
	}
}

// ============================================================================
// Private slot implementations - PolyLine properties
// ============================================================================

void QGMDockProperty::onPolyLineColorClicked()
{
	if (!mCurrentPolyLine) {
		return;
	}

	QColor currentColor = getColorFromButton(mPolyLineColorButton);
	QColor newColor = QColorDialog::getColor(currentColor, this, tr("Choose Polyline Color"));

	if (newColor.isValid() && newColor != currentColor) {
		setButtonColor(mPolyLineColorButton, newColor);
		emit polyLineColorChanged(mCurrentPolyLine, newColor);
	}
}

void QGMDockProperty::onPolyLineWidthChanged(double value)
{
	if (mCurrentPolyLine) {
		emit polyLineWidthChanged(mCurrentPolyLine, value);
	}
}

// ============================================================================
// Private helper methods
// ============================================================================

void QGMDockProperty::showMeshProperties()
{
	hideAllPropertyGroups();
	mMeshGroup->show();
}

void QGMDockProperty::showSectionProperties()
{
	hideAllPropertyGroups();
	mSectionGroup->show();
}

void QGMDockProperty::showPolyLineProperties()
{
	hideAllPropertyGroups();
	mPolyLineGroup->show();
}

void QGMDockProperty::showMultipleSelectionProperties()
{
	hideAllPropertyGroups();
	mMultiSelectGroup->show();
}

void QGMDockProperty::hideAllPropertyGroups()
{
	mMeshGroup->hide();
	mSectionGroup->hide();
	mPolyLineGroup->hide();
	mMultiSelectGroup->hide();
}

QString QGMDockProperty::formatNumber(double value, int decimals) const
{
	// Use locale-aware formatting
	QLocale locale;
	return locale.toString(value, 'f', decimals);
}

QString QGMDockProperty::formatInteger(uint64_t value) const
{
	// Use locale-aware formatting with thousand separators
	QLocale locale;
	return locale.toString(static_cast<qulonglong>(value));
}

QColor QGMDockProperty::getColorFromButton(QPushButton* button) const
{
	if (!button) {
		return Qt::black;
	}

	// Extract color from button's background
	QPalette palette = button->palette();
	return palette.color(QPalette::Button);
}

void QGMDockProperty::setButtonColor(QPushButton* button, const QColor& color)
{
	if (!button) {
		return;
	}

	// Set button background color
	QString styleSheet = QString("QPushButton { background-color: %1; border: 1px solid #888; }")
	                     .arg(color.name());
	button->setStyleSheet(styleSheet);

	// Update button text to show color values
	button->setText(QString("RGB(%1, %2, %3)")
	                .arg(color.red())
	                .arg(color.green())
	                .arg(color.blue()));
}

void QGMDockProperty::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate UI elements
		setWindowTitle(tr("Properties"));

		// Update group titles
		mMeshGroup->setTitle(tr("Mesh Properties"));
		mSectionGroup->setTitle(tr("Section Properties"));
		mPolyLineGroup->setTitle(tr("Polyline Properties"));
		mMultiSelectGroup->setTitle(tr("Multiple Selection"));

		// Note: Form labels will need to be recreated for full translation
		// This is a simplified version that only updates window/group titles
	}

	QDockWidget::changeEvent(event);
}
