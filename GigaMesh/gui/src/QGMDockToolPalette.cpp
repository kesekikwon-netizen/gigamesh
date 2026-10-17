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

#include "QGMDockToolPalette.h"
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QEvent>
#include <iostream>

const QStringList QGMDockToolPalette::mCategoryNames = {
	QT_TRANSLATE_NOOP("QGMDockToolPalette", "단면 도구"),
	QT_TRANSLATE_NOOP("QGMDockToolPalette", "석기 도구"),
	QT_TRANSLATE_NOOP("QGMDockToolPalette", "토기 도구"),
	QT_TRANSLATE_NOOP("QGMDockToolPalette", "측정 도구")
};

QGMDockToolPalette::QGMDockToolPalette(QWidget *parent)
	: QDockWidget(parent)
	, mTabWidget(nullptr)
	, mCurrentTool(TOOL_SECTION_TOP)
{
	std::cout << "[QGMDockToolPalette] Constructor started" << std::endl;
	setWindowTitle(tr("도구 팔레트"));
	setObjectName("QGMDockToolPalette");

	std::cout << "[QGMDockToolPalette] About to call setupToolDefinitions()..." << std::endl;
	setupToolDefinitions();
	std::cout << "[QGMDockToolPalette] About to call initializeUI()..." << std::endl;
	initializeUI();
	std::cout << "[QGMDockToolPalette] Constructor completed" << std::endl;
}

QGMDockToolPalette::~QGMDockToolPalette()
{
	// Qt parent-child relationship handles cleanup
}

void QGMDockToolPalette::setupToolDefinitions()
{
	mToolDefinitions.clear();

	// 단면 도구 (Category 0)
	mToolDefinitions.append({TOOL_SECTION_TOP,
		tr("상단 단면"),
		":/icons/archaeology/section_top.svg",
		tr("수평 (상면) 단면 평면 생성"),
		0});

	mToolDefinitions.append({TOOL_SECTION_FRONT,
		tr("정면 단면"),
		":/icons/archaeology/section_front.svg",
		tr("수직 정면 단면 평면 생성"),
		0});

	mToolDefinitions.append({TOOL_SECTION_SIDE,
		tr("측면 단면"),
		":/icons/archaeology/section_side.svg",
		tr("수직 측면 단면 평면 생성"),
		0});

	mToolDefinitions.append({TOOL_SECTION_FREE,
		tr("자유 단면"),
		":/icons/archaeology/section_free.svg",
		tr("임의 방향 단면 평면 생성"),
		0});

	mToolDefinitions.append({TOOL_SECTION_EDIT,
		tr("단면 편집"),
		":/icons/archaeology/section_edit.svg",
		tr("기존 단면 평면 위치 및 방향 수정"),
		0});

	mToolDefinitions.append({TOOL_SECTION_PROFILE,
		tr("프로파일 추출"),
		":/icons/archaeology/section_profile.svg",
		tr("단면 평면에서 2D 프로파일 추출"),
		0});

	// 석기 도구 (Category 1)
	mToolDefinitions.append({TOOL_LITHIC_MODE,
		tr("석기 실측 모드"),
		":/icons/archaeology/lithic_tool.svg",
		tr("석기 유물 실측 모드 활성화 (Raczynski-Henk 2017 표준)"),
		1});

	mToolDefinitions.append({TOOL_LITHIC_CORTEX,
		tr("격지면 선택"),
		":/icons/archaeology/lithic_cortex.svg",
		tr("교차 해칭 패턴 렌더링을 위한 격지면 영역 선택"),
		1});

	mToolDefinitions.append({TOOL_LITHIC_ORIENTATION,
		tr("타격 방향"),
		":/icons/archaeology/lithic_orient.svg",
		tr("표준 뷰를 위한 타격면 방향 설정"),
		1});

	mToolDefinitions.append({TOOL_LITHIC_RIDGE,
		tr("능선 강조"),
		":/icons/archaeology/lithic_ridge.svg",
		tr("능선을 굵은 선으로 강조 (표준의 2-3배)"),
		1});

	mToolDefinitions.append({TOOL_LITHIC_PROPERTIES,
		tr("석기 속성"),
		":/icons/archaeology/lithic_annotate.svg",
		tr("석기 유물 메타데이터 및 측정값 보기/편집"),
		1});

	// 토기 도구 (Category 2)
	mToolDefinitions.append({TOOL_CERAMIC_MODE,
		tr("토기 실측 모드"),
		":/icons/archaeology/ceramic_profile.svg",
		tr("토기 용기 실측 모드 활성화"),
		2});

	mToolDefinitions.append({TOOL_CERAMIC_ROLLOUT,
		tr("회전 전개도"),
		":/icons/archaeology/ceramic_reconstruct.svg",
		tr("용기 동체 원통형 전개도 생성"),
		2});

	mToolDefinitions.append({TOOL_CERAMIC_RIM,
		tr("구연부 분석"),
		":/icons/archaeology/ceramic_rim.svg",
		tr("구연부 직경 및 방향 분석"),
		2});

	mToolDefinitions.append({TOOL_CERAMIC_PATTERN,
		tr("문양 추출"),
		":/icons/archaeology/ceramic_pattern.svg",
		tr("표면 장식 문양 추출"),
		2});

	mToolDefinitions.append({TOOL_CERAMIC_PROPERTIES,
		tr("토기 속성"),
		":/icons/archaeology/ceramic_annotate.svg",
		tr("토기 용기 메타데이터 보기/편집"),
		2});

	// 측정 도구 (Category 3)
	mToolDefinitions.append({TOOL_MEASURE_DISTANCE,
		tr("거리 측정"),
		":/icons/archaeology/measure_distance.svg",
		tr("메시 표면의 두 점 사이 거리 측정"),
		3});

	mToolDefinitions.append({TOOL_MEASURE_ANGLE,
		tr("각도 측정"),
		":/icons/archaeology/measure_angle.svg",
		tr("모서리 또는 표면 간 각도 측정"),
		3});

	mToolDefinitions.append({TOOL_MEASURE_AREA,
		tr("면적 측정"),
		":/icons/archaeology/measure_area.svg",
		tr("선택한 영역의 표면적 계산"),
		3});

	mToolDefinitions.append({TOOL_MEASURE_CURVATURE,
		tr("곡률 분석"),
		":/icons/archaeology/measure_curvature.svg",
		tr("표면 곡률 분석 및 곡률 맵 표시"),
		3});
}

void QGMDockToolPalette::initializeUI()
{
	std::cout << "[QGMDockToolPalette::initializeUI] Creating main widget..." << std::endl;
	// Create main widget and layout
	QWidget* mainWidget = new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
	mainLayout->setContentsMargins(4, 4, 4, 4);
	mainLayout->setSpacing(4);

	std::cout << "[QGMDockToolPalette::initializeUI] Creating tab widget..." << std::endl;
	// Create tab widget for categories
	mTabWidget = new QTabWidget(mainWidget);
	mTabWidget->setTabPosition(QTabWidget::North);

	std::cout << "[QGMDockToolPalette::initializeUI] Creating category widgets..." << std::endl;
	// Create a tab for each category
	for (int i = 0; i < 4; ++i) {
		std::cout << "[QGMDockToolPalette::initializeUI] Creating category " << i << "..." << std::endl;
		QWidget* categoryWidget = createCategoryWidget(i);
		std::cout << "[QGMDockToolPalette::initializeUI] Adding tab for category " << i << "..." << std::endl;
		mTabWidget->addTab(categoryWidget, tr(mCategoryNames[i].toUtf8().constData()));
	}

	std::cout << "[QGMDockToolPalette::initializeUI] Setting widget..." << std::endl;
	mainLayout->addWidget(mTabWidget);
	setWidget(mainWidget);

	std::cout << "[QGMDockToolPalette::initializeUI] Connecting signals..." << std::endl;
	// Connect tab change signal
	connect(mTabWidget, &QTabWidget::currentChanged,
	        this, &QGMDockToolPalette::onCategoryChanged);

	// Set minimum size
	setMinimumWidth(200);
	std::cout << "[QGMDockToolPalette::initializeUI] initializeUI completed" << std::endl;
}

QWidget* QGMDockToolPalette::createCategoryWidget(int category)
{
	// Create scroll area for tools
	QScrollArea* scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame);

	// Create container widget
	QWidget* container = new QWidget();
	QGridLayout* gridLayout = new QGridLayout(container);
	gridLayout->setContentsMargins(4, 4, 4, 4);
	gridLayout->setSpacing(4);

	// Create button group for this category (for exclusive selection)
	QButtonGroup* buttonGroup = new QButtonGroup(this);
	buttonGroup->setExclusive(true);
	mButtonGroups[category] = buttonGroup;

	// Add tools for this category
	int row = 0;
	int col = 0;
	const int columns = 2;  // 2 columns of buttons

	for (const ToolInfo& toolInfo : mToolDefinitions) {
		if (toolInfo.category == category) {
			QToolButton* button = createToolButton(toolInfo);
			buttonGroup->addButton(button, toolInfo.id);
			mToolButtons[toolInfo.id] = button;

			gridLayout->addWidget(button, row, col);

			col++;
			if (col >= columns) {
				col = 0;
				row++;
			}
		}
	}

	// Add stretch at the bottom
	gridLayout->setRowStretch(row + 1, 1);

	container->setLayout(gridLayout);
	scrollArea->setWidget(container);

	// Connect button group signal
	connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
	        this, &QGMDockToolPalette::onToolButtonClicked);

	return scrollArea;
}

QToolButton* QGMDockToolPalette::createToolButton(const ToolInfo& toolInfo)
{
	QToolButton* button = new QToolButton();
	button->setCheckable(true);
	button->setAutoRaise(false);
	button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	button->setText(toolInfo.name);
	button->setToolTip(toolInfo.tooltip);
	button->setIconSize(QSize(32, 32));
	button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	button->setMinimumHeight(64);

	// Load icon
	QIcon icon(toolInfo.iconPath);
	if (!icon.isNull()) {
		button->setIcon(icon);
	} else {
		// Fallback: use a standard icon if custom icon not found
		button->setText(toolInfo.name);
	}

	return button;
}

const ToolInfo& QGMDockToolPalette::getToolInfo(ArchaeologyToolID toolId) const
{
	for (const ToolInfo& info : mToolDefinitions) {
		if (info.id == toolId) {
			return info;
		}
	}
	// Return first tool as fallback (should never happen)
	return mToolDefinitions[0];
}

void QGMDockToolPalette::setCurrentTool(ArchaeologyToolID toolId)
{
	if (toolId >= TOOL_COUNT) {
		return;
	}

	mCurrentTool = toolId;

	// Update button state
	if (mToolButtons.contains(toolId)) {
		QToolButton* button = mToolButtons[toolId];
		button->setChecked(true);

		// Switch to the correct tab
		const ToolInfo& info = getToolInfo(toolId);
		if (mTabWidget->currentIndex() != info.category) {
			mTabWidget->setCurrentIndex(info.category);
		}
	}

	emit toolSelectionChanged(toolId);
}

void QGMDockToolPalette::setEnabled(bool enabled)
{
	QDockWidget::setEnabled(enabled);
	if (mTabWidget) {
		mTabWidget->setEnabled(enabled);
	}
}

void QGMDockToolPalette::updateToolAvailability(bool hasMesh, bool hasSelection)
{
	// Enable/disable tools based on mesh state

	// Section tools always available if mesh loaded
	mToolButtons[TOOL_SECTION_TOP]->setEnabled(hasMesh);
	mToolButtons[TOOL_SECTION_FRONT]->setEnabled(hasMesh);
	mToolButtons[TOOL_SECTION_SIDE]->setEnabled(hasMesh);
	mToolButtons[TOOL_SECTION_FREE]->setEnabled(hasMesh);
	mToolButtons[TOOL_SECTION_EDIT]->setEnabled(hasMesh);
	mToolButtons[TOOL_SECTION_PROFILE]->setEnabled(hasMesh);

	// Lithic tools
	mToolButtons[TOOL_LITHIC_MODE]->setEnabled(hasMesh);
	mToolButtons[TOOL_LITHIC_CORTEX]->setEnabled(hasMesh);
	mToolButtons[TOOL_LITHIC_ORIENTATION]->setEnabled(hasMesh);
	mToolButtons[TOOL_LITHIC_RIDGE]->setEnabled(hasMesh);
	mToolButtons[TOOL_LITHIC_PROPERTIES]->setEnabled(hasMesh);

	// Ceramic tools
	mToolButtons[TOOL_CERAMIC_MODE]->setEnabled(hasMesh);
	mToolButtons[TOOL_CERAMIC_ROLLOUT]->setEnabled(hasMesh);
	mToolButtons[TOOL_CERAMIC_RIM]->setEnabled(hasMesh);
	mToolButtons[TOOL_CERAMIC_PATTERN]->setEnabled(hasMesh);
	mToolButtons[TOOL_CERAMIC_PROPERTIES]->setEnabled(hasMesh);

	// Measurement tools
	mToolButtons[TOOL_MEASURE_DISTANCE]->setEnabled(hasMesh);
	mToolButtons[TOOL_MEASURE_ANGLE]->setEnabled(hasMesh);
	mToolButtons[TOOL_MEASURE_AREA]->setEnabled(hasMesh && hasSelection);
	mToolButtons[TOOL_MEASURE_CURVATURE]->setEnabled(hasMesh && hasSelection);
}

void QGMDockToolPalette::onToolButtonClicked(int buttonId)
{
	ArchaeologyToolID toolId = static_cast<ArchaeologyToolID>(buttonId);

	if (toolId >= TOOL_COUNT) {
		return;
	}

	mCurrentTool = toolId;
	emit toolActivated(toolId);
	emit toolSelectionChanged(toolId);
}

void QGMDockToolPalette::onCategoryChanged(int index)
{
	// Category changed - could be used for additional logic
	// Currently just ensures the UI is updated
	Q_UNUSED(index);
}

void QGMDockToolPalette::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate UI when language changes
		setWindowTitle(tr("Tool Palette"));

		// Update tab labels
		for (int i = 0; i < 4 && i < mTabWidget->count(); ++i) {
			mTabWidget->setTabText(i, tr(mCategoryNames[i].toUtf8().constData()));
		}

		// Update tool button texts and tooltips
		for (int i = 0; i < mToolDefinitions.size(); ++i) {
			ArchaeologyToolID toolId = mToolDefinitions[i].id;
			if (mToolButtons.contains(toolId)) {
				QToolButton* button = mToolButtons[toolId];
				button->setText(mToolDefinitions[i].name);
				button->setToolTip(mToolDefinitions[i].tooltip);
			}
		}
	}

	QDockWidget::changeEvent(event);
}
