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

#include "qgmdocksections.h"
#include <QLabel>
#include <QMessageBox>

QGMDockSections::QGMDockSections(QWidget* parent)
    : QDockWidget(parent)
    , mSectionManager(nullptr)
    , mEditingSection(nullptr)
{
	setWindowTitle(QString::fromUtf8("단면 목록 (Sections)"));
	setObjectName("DockSections");

	setupUI();
}

QGMDockSections::~QGMDockSections()
{
}

void QGMDockSections::setupUI()
{
	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
	mainLayout->setSpacing(10);

	// 🎯 큰 아이콘 툴바 - 고고학 실측 전용
	QGroupBox* toolbarGroup = new QGroupBox(QString::fromUtf8("🎯 빠른 실행"), mainWidget);
	toolbarGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; }");
	QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbarGroup);
	toolbarLayout->setSpacing(15);

	// 큰 아이콘 버튼들 (72x72 픽셀) - 텍스트로 표시
	mBtnAddTop = new QPushButton(QString::fromUtf8("📐\n상면"), toolbarGroup);
	mBtnAddTop->setObjectName("iconButton");
	mBtnAddTop->setToolTip(QString::fromUtf8("상면 단면 (Top View)\n위에서 아래로 보는 평면"));
	mBtnAddTop->setMinimumSize(72, 72);
	mBtnAddTop->setStyleSheet("QPushButton { font-size: 14px; font-weight: bold; }");

	mBtnAddFront = new QPushButton(QString::fromUtf8("📐\n정면"), toolbarGroup);
	mBtnAddFront->setObjectName("iconButton");
	mBtnAddFront->setToolTip(QString::fromUtf8("정면 단면 (Front View)\n앞에서 뒤로 보는 평면"));
	mBtnAddFront->setMinimumSize(72, 72);
	mBtnAddFront->setStyleSheet("QPushButton { font-size: 14px; font-weight: bold; }");

	mBtnAddSide = new QPushButton(QString::fromUtf8("📐\n측면"), toolbarGroup);
	mBtnAddSide->setObjectName("iconButton");
	mBtnAddSide->setToolTip(QString::fromUtf8("측면 단면 (Side View)\n옆에서 보는 평면"));
	mBtnAddSide->setMinimumSize(72, 72);
	mBtnAddSide->setStyleSheet("QPushButton { font-size: 14px; font-weight: bold; }");

	QPushButton* btnCut = new QPushButton(QString::fromUtf8("✂️\n자르기"), toolbarGroup);
	btnCut->setObjectName("iconButton");
	btnCut->setToolTip(QString::fromUtf8("평면 자르기\n설정한 평면으로 메시 절단"));
	btnCut->setMinimumSize(72, 72);
	btnCut->setStyleSheet("QPushButton { font-size: 14px; font-weight: bold; }");

	QPushButton* btnExportQuick = new QPushButton(QString::fromUtf8("💾\nSVG"), toolbarGroup);
	btnExportQuick->setObjectName("iconButton");
	btnExportQuick->setToolTip(QString::fromUtf8("SVG 내보내기\n도면으로 저장 (방안선 포함)"));
	btnExportQuick->setMinimumSize(72, 72);
	btnExportQuick->setStyleSheet("QPushButton { font-size: 14px; font-weight: bold; }");

	toolbarLayout->addWidget(mBtnAddTop);
	toolbarLayout->addWidget(mBtnAddFront);
	toolbarLayout->addWidget(mBtnAddSide);
	toolbarLayout->addWidget(btnCut);
	toolbarLayout->addWidget(btnExportQuick);
	toolbarLayout->addStretch();

	mainLayout->addWidget(toolbarGroup);

	// 사용자 정의 버튼은 작게 별도로
	mBtnAddCustom = new QPushButton(QString::fromUtf8("+ 사용자 정의 평면"), mainWidget);
	mBtnAddCustom->setMaximumWidth(150);
	mainLayout->addWidget(mBtnAddCustom);

	// Section list (간소화)
	QLabel* listLabel = new QLabel(QString::fromUtf8("📋 활성 단면:"), mainWidget);
	listLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
	mainLayout->addWidget(listLabel);

	mSectionList = new QListWidget(mainWidget);
	mSectionList->setSelectionMode(QAbstractItemView::SingleSelection);
	mSectionList->setMaximumHeight(120);
	mainLayout->addWidget(mSectionList);

	// Action buttons
	QGroupBox* actionGroup = new QGroupBox(QString::fromUtf8("작업"), mainWidget);
	QVBoxLayout* actionLayout = new QVBoxLayout(actionGroup);

	mBtnRemove = new QPushButton(QString::fromUtf8("선택 삭제"), actionGroup);
	mBtnExport = new QPushButton(QString::fromUtf8("선택 내보내기 (SVG)"), actionGroup);
	mBtnExportAll = new QPushButton(QString::fromUtf8("전체 내보내기 (SVG)"), actionGroup);

	actionLayout->addWidget(mBtnRemove);
	actionLayout->addWidget(mBtnExport);
	actionLayout->addWidget(mBtnExportAll);

	mainLayout->addWidget(actionGroup);

	// Display options
	QGroupBox* displayGroup = new QGroupBox(QString::fromUtf8("표시 옵션"), mainWidget);
	QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);

	mBtnToggleVisible = new QPushButton(QString::fromUtf8("표시/숨김"), displayGroup);
	mBtnToggleVisible->setCheckable(true);
	mBtnToggleVisible->setChecked(true);

	mBtnToggleGrid = new QPushButton(QString::fromUtf8("방안선 ON/OFF"), displayGroup);
	mBtnToggleGrid->setCheckable(true);
	mBtnToggleGrid->setChecked(false);

	displayLayout->addWidget(mBtnToggleVisible);
	displayLayout->addWidget(mBtnToggleGrid);

	mainLayout->addWidget(displayGroup);

	// 🎨 시각적 슬라이더 다이얼 - 평면 조정
	// 초기에는 숨김 (편집 모드 진입 시 표시)
	mEditGroup = new QGroupBox(QString::fromUtf8("🎨 평면 미세 조정"), mainWidget);
	mEditGroup->setVisible(false);  // 초기에는 숨김
	// 간단한 스타일로 변경 (겹침 방지)
	mEditGroup->setStyleSheet(
		"QGroupBox { font-size: 13px; font-weight: bold; "
		"border: 1px solid #0078d4; border-radius: 4px; padding: 10px; margin-top: 5px; }"
	);
	QVBoxLayout* editLayout = new QVBoxLayout(mEditGroup);

	// 📏 평면 이동 다이얼 (큰 시각적 슬라이더)
	QWidget* translationWidget = new QWidget(mEditGroup);
	translationWidget->setStyleSheet("background: white; border-radius: 6px; padding: 8px;");
	QVBoxLayout* transLayout = new QVBoxLayout(translationWidget);

	QLabel* translationTitle = new QLabel(QString::fromUtf8("📏 평면 이동 (법선 방향)"), mEditGroup);
	translationTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #0078d4;");
	transLayout->addWidget(translationTitle);

	// 눈금 라벨
	QHBoxLayout* tickLayout = new QHBoxLayout();
	QLabel* minLabel = new QLabel("-100mm", mEditGroup);
	QLabel* centerLabel = new QLabel("0", mEditGroup);
	QLabel* maxLabel = new QLabel("+100mm", mEditGroup);
	minLabel->setStyleSheet("font-size: 10px; color: #666;");
	centerLabel->setStyleSheet("font-size: 10px; color: #000; font-weight: bold;");
	maxLabel->setStyleSheet("font-size: 10px; color: #666;");
	tickLayout->addWidget(minLabel);
	tickLayout->addStretch();
	tickLayout->addWidget(centerLabel);
	tickLayout->addStretch();
	tickLayout->addWidget(maxLabel);
	transLayout->addLayout(tickLayout);

	mTranslationSlider = new QSlider(Qt::Horizontal, mEditGroup);
	mTranslationSlider->setRange(-1000, 1000);
	mTranslationSlider->setValue(0);
	mTranslationSlider->setTickPosition(QSlider::TicksBelow);
	mTranslationSlider->setTickInterval(100);
	mTranslationSlider->setMinimumHeight(35);
	mTranslationSlider->setStyleSheet(
		"QSlider::groove:horizontal { height: 12px; background: #e0e0e0; border-radius: 6px; }"
		"QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4fc3f7, stop:1 #0078d4); "
		"width: 28px; height: 28px; border-radius: 14px; margin: -8px 0; border: 2px solid white; }"
		"QSlider::sub-page:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #42a5f5, stop:1 #0078d4); border-radius: 6px; }"
	);
	transLayout->addWidget(mTranslationSlider);

	mTranslationLabel = new QLabel("0.0 mm", mEditGroup);
	mTranslationLabel->setAlignment(Qt::AlignCenter);
	mTranslationLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #0078d4; background: #e3f2fd; padding: 4px; border-radius: 4px;");
	transLayout->addWidget(mTranslationLabel);

	editLayout->addWidget(translationWidget);
	editLayout->addSpacing(15);

	// 🔄 평면 회전 다이얼 (큰 시각적 슬라이더)
	QWidget* rotationWidget = new QWidget(mEditGroup);
	rotationWidget->setStyleSheet("background: white; border-radius: 6px; padding: 8px;");
	QVBoxLayout* rotLayout = new QVBoxLayout(rotationWidget);

	QLabel* rotationTitle = new QLabel(QString::fromUtf8("🔄 평면 회전"), mEditGroup);
	rotationTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #ff5722;");
	rotLayout->addWidget(rotationTitle);

	// 회전 눈금
	QHBoxLayout* rotTickLayout = new QHBoxLayout();
	QLabel* rot180L = new QLabel("-180°", mEditGroup);
	QLabel* rot90L = new QLabel("-90°", mEditGroup);
	QLabel* rot0 = new QLabel("0°", mEditGroup);
	QLabel* rot90R = new QLabel("+90°", mEditGroup);
	QLabel* rot180R = new QLabel("+180°", mEditGroup);
	rot180L->setStyleSheet("font-size: 10px; color: #666;");
	rot90L->setStyleSheet("font-size: 10px; color: #666;");
	rot0->setStyleSheet("font-size: 10px; color: #000; font-weight: bold;");
	rot90R->setStyleSheet("font-size: 10px; color: #666;");
	rot180R->setStyleSheet("font-size: 10px; color: #666;");
	rotTickLayout->addWidget(rot180L);
	rotTickLayout->addStretch();
	rotTickLayout->addWidget(rot90L);
	rotTickLayout->addStretch();
	rotTickLayout->addWidget(rot0);
	rotTickLayout->addStretch();
	rotTickLayout->addWidget(rot90R);
	rotTickLayout->addStretch();
	rotTickLayout->addWidget(rot180R);
	rotLayout->addLayout(rotTickLayout);

	mRotationSlider = new QSlider(Qt::Horizontal, mEditGroup);
	mRotationSlider->setRange(-1800, 1800);
	mRotationSlider->setValue(0);
	mRotationSlider->setTickPosition(QSlider::TicksBelow);
	mRotationSlider->setTickInterval(150);
	mRotationSlider->setMinimumHeight(35);
	mRotationSlider->setStyleSheet(
		"QSlider::groove:horizontal { height: 12px; background: #e0e0e0; border-radius: 6px; }"
		"QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff9800, stop:1 #ff5722); "
		"width: 28px; height: 28px; border-radius: 14px; margin: -8px 0; border: 2px solid white; }"
		"QSlider::sub-page:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff7043, stop:1 #ff5722); border-radius: 6px; }"
	);
	rotLayout->addWidget(mRotationSlider);

	mRotationLabel = new QLabel(QString::fromUtf8("0.0°"), mEditGroup);
	mRotationLabel->setAlignment(Qt::AlignCenter);
	mRotationLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ff5722; background: #fff3e0; padding: 4px; border-radius: 4px;");
	rotLayout->addWidget(mRotationLabel);

	editLayout->addWidget(rotationWidget);

	editLayout->addSpacing(15);

	// Action buttons
	QHBoxLayout* editActionLayout = new QHBoxLayout();

	mBtnFinalize = new QPushButton(QString::fromUtf8("✓ 단면 생성"), mEditGroup);
	mBtnFinalize->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; }");
	editActionLayout->addWidget(mBtnFinalize);

	mBtnCancel = new QPushButton(QString::fromUtf8("✗ 취소"), mEditGroup);
	mBtnCancel->setStyleSheet("QPushButton { background-color: #757575; color: white; padding: 10px; }");
	editActionLayout->addWidget(mBtnCancel);

	editLayout->addLayout(editActionLayout);

	// Initially hidden (only shown when editing)
	mEditGroup->setEnabled(false);
	mEditGroup->setVisible(false);

	mainLayout->addWidget(mEditGroup);

	mainLayout->addStretch();

	setWidget(mainWidget);

	// Connect signals
	connect(mSectionList, &QListWidget::itemClicked, this, &QGMDockSections::onSectionItemClicked);
	connect(mSectionList, &QListWidget::itemDoubleClicked, this, &QGMDockSections::onSectionItemDoubleClicked);

	connect(mBtnAddTop, &QPushButton::clicked, this, &QGMDockSections::onAddTopSection);
	connect(mBtnAddFront, &QPushButton::clicked, this, &QGMDockSections::onAddFrontSection);
	connect(mBtnAddSide, &QPushButton::clicked, this, &QGMDockSections::onAddSideSection);
	connect(mBtnAddCustom, &QPushButton::clicked, this, &QGMDockSections::onAddCustomSection);

	connect(mBtnRemove, &QPushButton::clicked, this, &QGMDockSections::onRemoveSection);
	connect(mBtnExport, &QPushButton::clicked, this, &QGMDockSections::onExportSection);
	connect(mBtnExportAll, &QPushButton::clicked, this, &QGMDockSections::onExportAll);

	connect(mBtnToggleVisible, &QPushButton::clicked, this, &QGMDockSections::onToggleVisibility);
	connect(mBtnToggleGrid, &QPushButton::clicked, this, &QGMDockSections::onToggleGrid);

	// Interactive editing signals
	connect(mTranslationSlider, &QSlider::valueChanged, this, &QGMDockSections::onTranslationSliderChanged);
	connect(mRotationSlider, &QSlider::valueChanged, this, &QGMDockSections::onRotationSliderChanged);
	connect(mBtnFinalize, &QPushButton::clicked, this, &QGMDockSections::onFinalizeClicked);
	connect(mBtnCancel, &QPushButton::clicked, this, &QGMDockSections::onCancelClicked);

	updateButtons();
}

void QGMDockSections::setSectionManager(SectionManager* manager)
{
	// Disconnect old manager
	if (mSectionManager != nullptr) {
		disconnect(mSectionManager, nullptr, this, nullptr);
	}

	mSectionManager = manager;

	// Connect new manager signals
	if (mSectionManager != nullptr) {
		connect(mSectionManager, &SectionManager::sectionAdded, this, &QGMDockSections::onSectionAdded);
		connect(mSectionManager, &SectionManager::sectionRemoved, this, &QGMDockSections::onSectionRemoved);
		connect(mSectionManager, &SectionManager::activeSectionChanged, this, &QGMDockSections::onActiveSectionChanged);
		connect(mSectionManager, &SectionManager::sectionsCleared, this, &QGMDockSections::refreshSectionList);
	}

	refreshSectionList();
}

void QGMDockSections::refreshSectionList()
{
	std::cout << "[QGMDockSections::refreshSectionList] Refreshing list, count: "
	          << (mSectionManager ? mSectionManager->getSectionCount() : 0) << std::endl;

	std::cout << "[QGMDockSections::refreshSectionList] DEBUG: mSectionList pointer = "
	          << (void*)mSectionList << std::endl;

	if (mSectionList == nullptr) {
		std::cout << "[QGMDockSections::refreshSectionList] CRITICAL ERROR: mSectionList is nullptr!" << std::endl;
		return;
	}

	std::cout << "[QGMDockSections::refreshSectionList] About to clear mSectionList..." << std::endl;
	mSectionList->clear();
	std::cout << "[QGMDockSections::refreshSectionList] mSectionList cleared successfully" << std::endl;

	if (mSectionManager == nullptr) {
		std::cout << "[QGMDockSections::refreshSectionList] ERROR: mSectionManager is nullptr!" << std::endl;
		return;
	}

	for (int i = 0; i < mSectionManager->getSectionCount(); ++i) {
		Section* section = mSectionManager->getSection(i);
		if (section != nullptr) {
			std::cout << "[QGMDockSections::refreshSectionList] Adding item " << i
			          << ": " << section->getName().toStdString() << std::endl;
			QListWidgetItem* item = createSectionItem(section, i);
			mSectionList->addItem(item);

			// Highlight active section
			if (section == mSectionManager->getActiveSection()) {
				mSectionList->setCurrentItem(item);
			}
		}
	}

	std::cout << "[QGMDockSections::refreshSectionList] About to call updateButtons()..." << std::endl;
	updateButtons();
	std::cout << "[QGMDockSections::refreshSectionList] updateButtons() completed successfully" << std::endl;
}

QListWidgetItem* QGMDockSections::createSectionItem(Section* section, int index)
{
	QString itemText = QString("%1. %2").arg(index + 1).arg(section->getName());

	if (!section->isVisible()) {
		itemText += QString::fromUtf8(" [숨김]");
	}
	if (section->isGridEnabled()) {
		itemText += QString::fromUtf8(" [방안선]");
	}
	if (section->hasPolyLine()) {
		int vertCount = section->getPolyLine()->length();
		itemText += QString(" (%1 pts)").arg(vertCount);
	}

	QListWidgetItem* item = new QListWidgetItem(itemText);
	item->setData(Qt::UserRole, index);

	// Color indicator
	QColor color = section->getColor();
	QPixmap pixmap(16, 16);
	pixmap.fill(color);
	item->setIcon(QIcon(pixmap));

	return item;
}

void QGMDockSections::updateButtons()
{
	bool hasSelection = mSectionList->currentRow() >= 0;
	bool hasSections = (mSectionManager != nullptr && mSectionManager->getSectionCount() > 0);

	mBtnRemove->setEnabled(hasSelection);
	mBtnExport->setEnabled(hasSelection);
	mBtnExportAll->setEnabled(hasSections);
	mBtnToggleVisible->setEnabled(hasSelection);
	mBtnToggleGrid->setEnabled(hasSelection);

	// Update toggle button states
	if (hasSelection && mSectionManager != nullptr) {
		Section* section = mSectionManager->getSection(mSectionList->currentRow());
		if (section != nullptr) {
			mBtnToggleVisible->setChecked(section->isVisible());
			mBtnToggleGrid->setChecked(section->isGridEnabled());
		}
	}
}

void QGMDockSections::onSectionItemClicked(QListWidgetItem* item)
{
	if (item == nullptr || mSectionManager == nullptr) {
		return;
	}

	int index = item->data(Qt::UserRole).toInt();
	mSectionManager->setActiveSection(index);
	updateButtons();
}

void QGMDockSections::onSectionItemDoubleClicked(QListWidgetItem* item)
{
	if (item == nullptr || mSectionManager == nullptr) {
		return;
	}

	int index = item->data(Qt::UserRole).toInt();
	Section* section = mSectionManager->getSection(index);
	if (section != nullptr) {
		// Toggle visibility on double-click
		section->setVisible(!section->isVisible());
		refreshSectionList();
	}
}

void QGMDockSections::onAddTopSection()
{
	std::cout << "[DEBUG] onAddTopSection() called" << std::endl;

	// 상면 섹션 생성
	emit createSectionRequested(Section::SECTION_TOP);
	std::cout << "[DEBUG] createSectionRequested signal emitted for SECTION_TOP" << std::endl;

	// 🎯 자동으로 편집 모드 진입 - 사용자가 바로 조정 가능
	if (mSectionManager && mSectionManager->getSectionCount() > 0) {
		Section* newSection = mSectionManager->getSection(mSectionManager->getSectionCount() - 1);
		std::cout << "[DEBUG] Entering edit mode for new section" << std::endl;
		enterEditMode(newSection);
	}
}

void QGMDockSections::onAddFrontSection()
{
	// 정면 섹션 생성
	emit createSectionRequested(Section::SECTION_FRONT);

	// 🎯 자동으로 편집 모드 진입
	if (mSectionManager && mSectionManager->getSectionCount() > 0) {
		Section* newSection = mSectionManager->getSection(mSectionManager->getSectionCount() - 1);
		enterEditMode(newSection);
	}
}

void QGMDockSections::onAddSideSection()
{
	// 측면 섹션 생성
	emit createSectionRequested(Section::SECTION_SIDE_RIGHT);

	// 🎯 자동으로 편집 모드 진입
	if (mSectionManager && mSectionManager->getSectionCount() > 0) {
		Section* newSection = mSectionManager->getSection(mSectionManager->getSectionCount() - 1);
		enterEditMode(newSection);
	}
}

void QGMDockSections::onAddCustomSection()
{
	// 사용자 정의 섹션 생성
	emit createSectionRequested(Section::SECTION_CUSTOM);

	// 🎯 자동으로 편집 모드 진입
	if (mSectionManager && mSectionManager->getSectionCount() > 0) {
		Section* newSection = mSectionManager->getSection(mSectionManager->getSectionCount() - 1);
		enterEditMode(newSection);
	}
}

void QGMDockSections::onRemoveSection()
{
	int index = mSectionList->currentRow();
	if (index >= 0) {
		emit removeSectionRequested(index);
	}
}

void QGMDockSections::onExportSection()
{
	int index = mSectionList->currentRow();
	if (index >= 0) {
		emit exportSectionRequested(index);
	}
}

void QGMDockSections::onExportAll()
{
	emit exportAllSectionsRequested();
}

void QGMDockSections::onToggleVisibility()
{
	int index = mSectionList->currentRow();
	if (index >= 0 && mSectionManager != nullptr) {
		Section* section = mSectionManager->getSection(index);
		if (section != nullptr) {
			section->setVisible(!section->isVisible());
			refreshSectionList();
		}
	}
}

void QGMDockSections::onToggleGrid()
{
	int index = mSectionList->currentRow();
	if (index >= 0 && mSectionManager != nullptr) {
		Section* section = mSectionManager->getSection(index);
		if (section != nullptr) {
			section->setGridEnabled(!section->isGridEnabled());
			refreshSectionList();
		}
	}
}

void QGMDockSections::onSectionAdded(Section* section)
{
	std::cout << "[QGMDockSections::onSectionAdded] Section added: "
	          << (section ? section->getName().toStdString() : "nullptr") << std::endl;
	Q_UNUSED(section);
	refreshSectionList();
}

void QGMDockSections::onSectionRemoved(int index)
{
	Q_UNUSED(index);
	refreshSectionList();
}

void QGMDockSections::onActiveSectionChanged(Section* section)
{
	Q_UNUSED(section);
	refreshSectionList();
}

//! Enter interactive editing mode for a section
void QGMDockSections::enterEditMode(Section* section)
{
	if (section == nullptr) {
		return;
	}

	mEditingSection = section;
	
	// Reset sliders to initial position
	mTranslationSlider->setValue(0);
	mRotationSlider->setValue(0);
	mTranslationLabel->setText("0.0 mm");
	mRotationLabel->setText(QString::fromUtf8("0.0°"));

	// Show and enable editing UI
	mEditGroup->setVisible(true);
	mEditGroup->setEnabled(true);

	// Disable other controls during editing
	mBtnAddTop->setEnabled(false);
	mBtnAddFront->setEnabled(false);
	mBtnAddSide->setEnabled(false);
	mBtnAddCustom->setEnabled(false);
	mSectionList->setEnabled(false);
}

//! Exit interactive editing mode
void QGMDockSections::exitEditMode()
{
	mEditingSection = nullptr;

	// Hide editing UI
	mEditGroup->setVisible(false);
	mEditGroup->setEnabled(false);

	// Re-enable other controls
	mBtnAddTop->setEnabled(true);
	mBtnAddFront->setEnabled(true);
	mBtnAddSide->setEnabled(true);
	mBtnAddCustom->setEnabled(true);
	mSectionList->setEnabled(true);

	updateButtons();
}

//! Handle translation slider changes - 실시간 평면 이동
void QGMDockSections::onTranslationSliderChanged(int value)
{
	if (mEditingSection == nullptr) {
		return;
	}

	// 슬라이더 값을 거리로 변환 (1 unit = 0.1mm)
	double distance = value * 0.1;  // mm

	// 라벨 업데이트
	mTranslationLabel->setText(QString("%1 mm").arg(distance, 0, 'f', 1));

	// 🎯 평면을 법선 방향으로 이동
	mEditingSection->translateAlongNormal(distance);

	// 메시와의 교선 다시 계산
	if (mSectionManager) {
		mSectionManager->calculateIntersection(nullptr, mEditingSection);
	}

	// 3D 뷰 실시간 업데이트
	emit sectionSelected(mEditingSection);
}

//! Handle rotation slider changes - 실시간 평면 회전
void QGMDockSections::onRotationSliderChanged(int value)
{
	if (mEditingSection == nullptr) {
		return;
	}

	// 슬라이더 값을 각도로 변환 (0.1° per unit)
	double angle = value * 0.1;

	// 라벨 업데이트
	mRotationLabel->setText(QString::fromUtf8("%1°").arg(angle, 0, 'f', 1));

	// 🎯 평면 회전 - 섹션 타입에 따라 회전축 결정
	Vector3D rotationAxis;
	switch(mEditingSection->getType()) {
		case Section::SECTION_TOP:
			rotationAxis = Vector3D(0.0, 0.0, 1.0, 0.0);  // Z축 중심 회전
			break;
		case Section::SECTION_FRONT:
			rotationAxis = Vector3D(0.0, 1.0, 0.0, 0.0);  // Y축 중심 회전
			break;
		case Section::SECTION_SIDE_LEFT:
		case Section::SECTION_SIDE_RIGHT:
			rotationAxis = Vector3D(1.0, 0.0, 0.0, 0.0);  // X축 중심 회전
			break;
		default:
			rotationAxis = Vector3D(0.0, 0.0, 1.0, 0.0);  // 기본: Z축
			break;
	}

	// 평면 회전 적용
	mEditingSection->rotateAroundAxis(rotationAxis, angle);

	// 메시와의 교선 다시 계산
	if (mSectionManager) {
		mSectionManager->calculateIntersection(nullptr, mEditingSection);
	}

	// 3D 뷰 실시간 업데이트
	emit sectionSelected(mEditingSection);
}

//! Finalize the section and exit edit mode
void QGMDockSections::onFinalizeClicked()
{
	if (mEditingSection == nullptr) {
		return;
	}

	// Emit signal to inform main window to calculate intersection
	emit sectionFinalized(mEditingSection);

	// Exit edit mode
	exitEditMode();
}

//! Cancel editing and exit edit mode
void QGMDockSections::onCancelClicked()
{
	if (mEditingSection == nullptr) {
		return;
	}

	// Emit signal to inform main window
	emit sectionEditCanceled();

	// Exit edit mode
	exitEditMode();
}
