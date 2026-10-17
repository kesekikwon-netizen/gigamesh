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

#ifndef DONGARCHTOOLPALETTE_H
#define DONGARCHTOOLPALETTE_H

#include <QWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QVector>
#include <QStyle>

/**
 * @brief Tool IDs for DongArch3D measurement and manipulation tools
 *
 * Enumeration of 20 archaeological measurement tools organized in a 4x5 grid.
 * These tools provide general-purpose manipulation and specialized measurement
 * capabilities for 3D archaeological artifact analysis.
 */
enum DongArchToolID {
	TOOL_SELECT = 0,           // 선택 도구
	TOOL_MOVE,                 // 이동 도구
	TOOL_ROTATE,               // 회전 도구
	TOOL_DISTANCE,             // 거리 측정
	TOOL_ANGLE,                // 각도 측정
	TOOL_AREA,                 // 면적 측정
	TOOL_SECTION,              // 단면 생성
	TOOL_PROFILE,              // 프로파일 추출
	TOOL_LITHIC,               // 석기 모드
	TOOL_CERAMIC,              // 토기 모드
	TOOL_METAL,                // 금속기 모드
	TOOL_CORTEX,               // 표피 선택
	TOOL_RIDGE,                // 능선 강조
	TOOL_CONTOUR,              // 등고선 생성
	TOOL_GRID,                 // 그리드 표시
	TOOL_SCALE_BAR,            // 스케일 바
	TOOL_ANNOTATE,             // 주석 추가
	TOOL_SCREENSHOT,           // 스크린샷
	TOOL_COMPARE,              // 비교 모드
	TOOL_SETTINGS,             // 설정

	DONGARCH_TOOL_COUNT = 20
};

/**
 * @brief Structure holding tool metadata
 *
 * Contains display name, icon path, and tooltip for each tool button.
 */
struct DongArchToolInfo {
	DongArchToolID id;
	QString name;          // Korean display name
	QString iconPath;      // Qt resource path to icon
	QString tooltip;       // Korean tooltip text
};

/**
 * @brief DongArchToolPalette - General-purpose archaeological tool palette
 *
 * Provides a 4x5 grid of 20 tool buttons for common archaeological measurement
 * and manipulation tasks. This palette complements the specialized QGMDockToolPalette
 * by offering frequently-used general tools in a compact layout.
 *
 * Features:
 * - 4 columns x 5 rows grid layout
 * - 32x32 pixel icons with Korean tooltips
 * - Exclusive tool selection (radio button behavior)
 * - Signal emission on tool change
 * - Fallback to Qt standard icons when custom icons unavailable
 *
 * Usage:
 * @code
 * DongArchToolPalette* palette = new DongArchToolPalette(parent);
 * connect(palette, &DongArchToolPalette::toolSelected,
 *         this, &MyClass::handleToolSelection);
 * @endcode
 */
class DongArchToolPalette : public QWidget
{
	Q_OBJECT

public:
	/**
	 * @brief Construct a new DongArchToolPalette
	 * @param parent Parent widget (optional)
	 */
	explicit DongArchToolPalette(QWidget *parent = nullptr);

	/**
	 * @brief Destructor
	 */
	~DongArchToolPalette();

	/**
	 * @brief Get currently selected tool ID
	 * @return Current tool ID (0-19)
	 */
	DongArchToolID getCurrentTool() const { return mCurrentTool; }

	/**
	 * @brief Set current tool programmatically
	 * @param toolId Tool ID to activate (0-19)
	 */
	void setCurrentTool(DongArchToolID toolId);

signals:
	/**
	 * @brief Emitted when user selects a different tool
	 * @param toolId ID of newly selected tool (0-19)
	 */
	void toolSelected(int toolId);

private slots:
	/**
	 * @brief Handle tool button clicks
	 * @param buttonId Button ID from button group (matches tool ID)
	 */
	void onToolButtonClicked(int buttonId);

private:
	/**
	 * @brief Initialize the UI grid layout and tool buttons
	 */
	void initializeUI();

	/**
	 * @brief Create a single tool button with icon and tooltip
	 * @param toolInfo Tool metadata
	 * @return Configured QToolButton
	 */
	QToolButton* createToolButton(const DongArchToolInfo& toolInfo);

	/**
	 * @brief Setup tool definitions (names, icons, tooltips)
	 */
	void setupToolDefinitions();

	/**
	 * @brief Load icon with fallback strategy
	 * @param iconPath Primary Qt resource path
	 * @param fallbackStyle Qt standard icon style (if primary fails)
	 * @return QIcon (may be null if all fallbacks fail)
	 */
	QIcon loadIconWithFallback(const QString& iconPath,
	                           QStyle::StandardPixmap fallbackStyle = QStyle::SP_CustomBase);

	// UI Components
	QGridLayout* mGridLayout;
	QButtonGroup* mButtonGroup;
	QVector<QToolButton*> mToolButtons;

	// Tool Data
	QVector<DongArchToolInfo> mToolDefinitions;
	DongArchToolID mCurrentTool;
};

#endif // DONGARCHTOOLPALETTE_H
