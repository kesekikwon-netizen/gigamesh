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

#include "DongArchToolPalette.h"
#include <QApplication>
#include <QStyle>
#include <QDebug>

DongArchToolPalette::DongArchToolPalette(QWidget *parent)
	: QWidget(parent)
	, mGridLayout(nullptr)
	, mButtonGroup(nullptr)
	, mCurrentTool(TOOL_SELECT)
{
	setupToolDefinitions();
	initializeUI();
}

DongArchToolPalette::~DongArchToolPalette()
{
	// Qt parent-child relationship handles cleanup
}

void DongArchToolPalette::setupToolDefinitions()
{
	mToolDefinitions.clear();

	// Row 1 (Tools 0-3)
	mToolDefinitions.append({TOOL_SELECT,
		tr("선택 도구"),
		"",  // Use Qt standard icon
		tr("메시 요소를 선택합니다 (정점, 면, 영역)")});

	mToolDefinitions.append({TOOL_MOVE,
		tr("이동 도구"),
		"",  // Use Qt standard icon
		tr("선택된 요소나 뷰를 이동합니다")});

	mToolDefinitions.append({TOOL_ROTATE,
		tr("회전 도구"),
		"",  // Use Qt standard icon
		tr("뷰를 회전하거나 객체의 방향을 조정합니다")});

	mToolDefinitions.append({TOOL_DISTANCE,
		tr("거리 측정"),
		":/icons/archaeology/measure_distance.svg",
		tr("메시 표면의 두 점 사이 거리를 측정합니다")});

	// Row 2 (Tools 4-7)
	mToolDefinitions.append({TOOL_ANGLE,
		tr("각도 측정"),
		":/icons/archaeology/measure_angle.svg",
		tr("모서리나 면 사이의 각도를 측정합니다")});

	mToolDefinitions.append({TOOL_AREA,
		tr("면적 측정"),
		":/icons/archaeology/measure_area.svg",
		tr("선택된 영역의 표면적을 계산합니다")});

	mToolDefinitions.append({TOOL_SECTION,
		tr("단면 생성"),
		":/icons/archaeology/section_top.svg",
		tr("단면 평면을 생성하여 내부 구조를 분석합니다")});

	mToolDefinitions.append({TOOL_PROFILE,
		tr("프로파일 추출"),
		":/icons/archaeology/section_profile.svg",
		tr("단면에서 2D 프로파일을 추출합니다")});

	// Row 3 (Tools 8-11)
	mToolDefinitions.append({TOOL_LITHIC,
		tr("석기 모드"),
		":/icons/archaeology/lithic_tool.svg",
		tr("석기 유물 도해 모드 활성화 (Raczynski-Henk 2017 표준)")});

	mToolDefinitions.append({TOOL_CERAMIC,
		tr("토기 모드"),
		":/icons/archaeology/ceramic_profile.svg",
		tr("토기 용기 도해 모드 활성화")});

	mToolDefinitions.append({TOOL_METAL,
		tr("금속기 모드"),
		"",  // No specific icon, will use fallback
		tr("금속 유물 도해 모드 활성화")});

	mToolDefinitions.append({TOOL_CORTEX,
		tr("표피 선택"),
		":/icons/archaeology/lithic_cortex.svg",
		tr("석기의 표피(cortex) 영역을 선택하여 교차 해칭 적용")});

	// Row 4 (Tools 12-15)
	mToolDefinitions.append({TOOL_RIDGE,
		tr("능선 강조"),
		":/icons/archaeology/lithic_ridge.svg",
		tr("능선을 굵은 선으로 강조합니다 (2-3배 두께)")});

	mToolDefinitions.append({TOOL_CONTOUR,
		tr("등고선 생성"),
		"",  // No specific icon, will use fallback
		tr("표면 높이 등고선을 생성합니다")});

	mToolDefinitions.append({TOOL_GRID,
		tr("그리드 표시"),
		":/icons/archaeology/grid_show.svg",
		tr("측정용 참조 그리드를 표시합니다")});

	mToolDefinitions.append({TOOL_SCALE_BAR,
		tr("스케일 바"),
		":/icons/archaeology/scale_bar.svg",
		tr("축척 막대를 추가하거나 편집합니다")});

	// Row 5 (Tools 16-19)
	mToolDefinitions.append({TOOL_ANNOTATE,
		tr("주석 추가"),
		":/icons/archaeology/lithic_annotate.svg",
		tr("텍스트 주석과 라벨을 추가합니다")});

	mToolDefinitions.append({TOOL_SCREENSHOT,
		tr("스크린샷"),
		"",  // Use Qt standard icon
		tr("현재 뷰를 고해상도 이미지로 저장합니다")});

	mToolDefinitions.append({TOOL_COMPARE,
		tr("비교 모드"),
		"",  // No specific icon, will use fallback
		tr("두 개의 메시를 나란히 비교합니다")});

	mToolDefinitions.append({TOOL_SETTINGS,
		tr("설정"),
		"",  // Use Qt standard icon
		tr("도구 팔레트 및 애플리케이션 설정")});
}

void DongArchToolPalette::initializeUI()
{
	// Create main grid layout (4 columns x 5 rows)
	mGridLayout = new QGridLayout(this);
	mGridLayout->setContentsMargins(4, 4, 4, 4);
	mGridLayout->setSpacing(4);

	// Create button group for exclusive selection
	mButtonGroup = new QButtonGroup(this);
	mButtonGroup->setExclusive(true);

	// Add 20 tool buttons in 4x5 grid
	const int columns = 4;
	int row = 0;
	int col = 0;

	for (const DongArchToolInfo& toolInfo : mToolDefinitions) {
		QToolButton* button = createToolButton(toolInfo);
		mButtonGroup->addButton(button, toolInfo.id);
		mToolButtons.append(button);

		mGridLayout->addWidget(button, row, col);

		col++;
		if (col >= columns) {
			col = 0;
			row++;
		}
	}

	// Set first tool as default
	if (!mToolButtons.isEmpty()) {
		mToolButtons[TOOL_SELECT]->setChecked(true);
	}

	// Connect button group signal
	connect(mButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
	        this, &DongArchToolPalette::onToolButtonClicked);

	setLayout(mGridLayout);
}

QToolButton* DongArchToolPalette::createToolButton(const DongArchToolInfo& toolInfo)
{
	QToolButton* button = new QToolButton(this);
	button->setCheckable(true);
	button->setAutoRaise(false);
	button->setToolButtonStyle(Qt::ToolButtonIconOnly);
	button->setToolTip(toolInfo.tooltip);
	button->setIconSize(QSize(32, 32));
	button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	button->setMinimumSize(48, 48);

	// Load icon with fallback strategy
	QIcon icon;
	QStyle::StandardPixmap fallbackIcon = QStyle::SP_CustomBase;

	// Determine fallback icon based on tool type
	switch (toolInfo.id) {
		case TOOL_SELECT:
			fallbackIcon = QStyle::SP_ArrowForward;
			break;
		case TOOL_MOVE:
			fallbackIcon = QStyle::SP_ArrowUp;
			break;
		case TOOL_ROTATE:
			fallbackIcon = QStyle::SP_BrowserReload;
			break;
		case TOOL_SCREENSHOT:
			fallbackIcon = QStyle::SP_DesktopIcon;
			break;
		case TOOL_SETTINGS:
			fallbackIcon = QStyle::SP_FileDialogDetailedView;
			break;
		default:
			fallbackIcon = QStyle::SP_FileIcon;
			break;
	}

	icon = loadIconWithFallback(toolInfo.iconPath, fallbackIcon);

	if (!icon.isNull()) {
		button->setIcon(icon);
	}

	return button;
}

QIcon DongArchToolPalette::loadIconWithFallback(const QString& iconPath,
                                                 QStyle::StandardPixmap fallbackStyle)
{
	QIcon icon;

	// Try loading custom icon from resources
	if (!iconPath.isEmpty()) {
		icon = QIcon(iconPath);
	}

	// If custom icon failed or not specified, use Qt standard icon
	if (icon.isNull() && fallbackStyle != QStyle::SP_CustomBase) {
		icon = QApplication::style()->standardIcon(fallbackStyle);
	}

	return icon;
}

void DongArchToolPalette::setCurrentTool(DongArchToolID toolId)
{
	if (toolId >= DONGARCH_TOOL_COUNT) {
		return;
	}

	mCurrentTool = toolId;

	// Update button state
	if (toolId < mToolButtons.size()) {
		mToolButtons[toolId]->setChecked(true);
	}

	emit toolSelected(static_cast<int>(toolId));
}

void DongArchToolPalette::onToolButtonClicked(int buttonId)
{
	DongArchToolID toolId = static_cast<DongArchToolID>(buttonId);

	if (toolId >= DONGARCH_TOOL_COUNT) {
		return;
	}

	mCurrentTool = toolId;

	// Debug output for verification
	qDebug() << "[DongArchToolPalette] Tool selected:" << buttonId
	         << "(" << mToolDefinitions[buttonId].name << ")";

	emit toolSelected(buttonId);
}
