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

#include "DongArchPropertyPanel.h"
#include <QVBoxLayout>

//! Constructor
DongArchPropertyPanel::DongArchPropertyPanel(QWidget* parent)
	: QDockWidget(parent),
	  mScrollArea(nullptr),
	  mContentWidget(nullptr),
	  mMainLayout(nullptr),
	  mCoordinateGroup(nullptr),
	  mSizeGroup(nullptr),
	  mMetadataGroup(nullptr),
	  mPosX(nullptr),
	  mPosY(nullptr),
	  mPosZ(nullptr),
	  mLength(nullptr),
	  mWidth(nullptr),
	  mHeight(nullptr),
	  mArea(nullptr),
	  mVolume(nullptr),
	  mArtifactName(nullptr),
	  mSiteName(nullptr),
	  mExcavDate(nullptr),
	  mGridID(nullptr),
	  mLayer(nullptr)
{
	setWindowTitle(tr("속성"));
	setObjectName("DongArchPropertyPanel");

	setupUI();
	loadSampleData(); // Initialize with sample data for testing
}

//! Destructor
DongArchPropertyPanel::~DongArchPropertyPanel()
{
	// Qt parent-child relationship handles cleanup
}

//! Setup the user interface
void DongArchPropertyPanel::setupUI()
{
	// Create scroll area for properties
	mScrollArea = new QScrollArea(this);
	mScrollArea->setWidgetResizable(true);
	mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget
	mContentWidget = new QWidget();
	mMainLayout = new QFormLayout(mContentWidget);
	mMainLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	mMainLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
	mMainLayout->setSpacing(10);

	// Create property groups
	createCoordinateGroup();
	createSizeGroup();
	createMetadataGroup();

	mScrollArea->setWidget(mContentWidget);
	setWidget(mScrollArea);

	setMinimumWidth(280);
}

//! Create coordinate property group (좌표 정보)
void DongArchPropertyPanel::createCoordinateGroup()
{
	mCoordinateGroup = new QGroupBox("좌표 정보", mContentWidget);
	QFormLayout* layout = new QFormLayout(mCoordinateGroup);
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

	// Position X
	mPosX = createValueLabel();
	layout->addRow("위치 X:", mPosX);

	// Position Y
	mPosY = createValueLabel();
	layout->addRow("위치 Y:", mPosY);

	// Position Z
	mPosZ = createValueLabel();
	layout->addRow("위치 Z:", mPosZ);

	mMainLayout->addRow(mCoordinateGroup);
}

//! Create size property group (크기 정보)
void DongArchPropertyPanel::createSizeGroup()
{
	mSizeGroup = new QGroupBox("크기 정보", mContentWidget);
	QFormLayout* layout = new QFormLayout(mSizeGroup);
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

	// Length
	mLength = createValueLabel();
	layout->addRow("길이:", mLength);

	// Width
	mWidth = createValueLabel();
	layout->addRow("폭:", mWidth);

	// Height
	mHeight = createValueLabel();
	layout->addRow("높이:", mHeight);

	// Area
	mArea = createValueLabel();
	layout->addRow("면적:", mArea);

	// Volume
	mVolume = createValueLabel();
	layout->addRow("부피:", mVolume);

	mMainLayout->addRow(mSizeGroup);
}

//! Create metadata property group (메타데이터)
void DongArchPropertyPanel::createMetadataGroup()
{
	mMetadataGroup = new QGroupBox("메타데이터", mContentWidget);
	QFormLayout* layout = new QFormLayout(mMetadataGroup);
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

	// Artifact name
	mArtifactName = createValueLabel();
	layout->addRow("유물명:", mArtifactName);

	// Site name
	mSiteName = createValueLabel();
	layout->addRow("유적명:", mSiteName);

	// Excavation date
	mExcavDate = createValueLabel();
	layout->addRow("발굴일자:", mExcavDate);

	// Grid ID
	mGridID = createValueLabel();
	layout->addRow("그리드 번호:", mGridID);

	// Layer
	mLayer = createValueLabel();
	layout->addRow("층위:", mLayer);

	mMainLayout->addRow(mMetadataGroup);
}

//! Update a single property
void DongArchPropertyPanel::updateProperty(const QString& key, const QVariant& value)
{
	QString displayValue = value.toString();

	// Map keys to labels
	if (key == "위치 X") {
		mPosX->setText(displayValue);
	} else if (key == "위치 Y") {
		mPosY->setText(displayValue);
	} else if (key == "위치 Z") {
		mPosZ->setText(displayValue);
	} else if (key == "길이") {
		mLength->setText(displayValue);
	} else if (key == "폭") {
		mWidth->setText(displayValue);
	} else if (key == "높이") {
		mHeight->setText(displayValue);
	} else if (key == "면적") {
		mArea->setText(displayValue);
	} else if (key == "부피") {
		mVolume->setText(displayValue);
	} else if (key == "유물명") {
		mArtifactName->setText(displayValue);
	} else if (key == "유적명") {
		mSiteName->setText(displayValue);
	} else if (key == "발굴일자") {
		mExcavDate->setText(displayValue);
	} else if (key == "그리드 번호") {
		mGridID->setText(displayValue);
	} else if (key == "층위") {
		mLayer->setText(displayValue);
	}
}

//! Clear all properties
void DongArchPropertyPanel::clearProperties()
{
	// Coordinate properties
	mPosX->setText("-");
	mPosY->setText("-");
	mPosZ->setText("-");

	// Size properties
	mLength->setText("-");
	mWidth->setText("-");
	mHeight->setText("-");
	mArea->setText("-");
	mVolume->setText("-");

	// Metadata
	mArtifactName->setText("-");
	mSiteName->setText("-");
	mExcavDate->setText("-");
	mGridID->setText("-");
	mLayer->setText("-");
}

//! Load sample data for testing
void DongArchPropertyPanel::loadSampleData()
{
	// Coordinate data
	updateProperty("위치 X", "12.5 mm");
	updateProperty("위치 Y", "-5.3 mm");
	updateProperty("위치 Z", "100.0 mm");

	// Size data
	updateProperty("길이", "45.2 mm");
	updateProperty("폭", "30.1 mm");
	updateProperty("높이", "15.7 mm");
	updateProperty("면적", "125.4 cm²");
	updateProperty("부피", "50.2 cm³");

	// Metadata
	updateProperty("유물명", "첨두기");
	updateProperty("유적명", "동국대학교 유적");
	updateProperty("발굴일자", "2025-11-08");
	updateProperty("그리드 번호", "C-12");
	updateProperty("층위", "3층");
}

// ============================================================================
// Private helper methods
// ============================================================================

//! Create a value label with standard formatting
QLabel* DongArchPropertyPanel::createValueLabel()
{
	QLabel* label = new QLabel("-");
	label->setTextInteractionFlags(Qt::TextSelectableByMouse);
	label->setStyleSheet("QLabel { padding: 4px; background-color: #f5f5f5; border: 1px solid #ddd; border-radius: 3px; }");
	return label;
}

//! Format double value to string
QString DongArchPropertyPanel::formatDouble(double value, int decimals) const
{
	return QString::number(value, 'f', decimals);
}
