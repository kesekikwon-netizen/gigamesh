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

#ifndef QGMDOCKTOOLPALETTE_H
#define QGMDOCKTOOLPALETTE_H

#include <QDockWidget>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QButtonGroup>
#include <QMap>

//! Tool IDs for archaeological illustration tools
enum ArchaeologyToolID {
	// Section Tools (0-5)
	TOOL_SECTION_TOP = 0,
	TOOL_SECTION_FRONT,
	TOOL_SECTION_SIDE,
	TOOL_SECTION_FREE,
	TOOL_SECTION_EDIT,
	TOOL_SECTION_PROFILE,

	// Lithic Tools (6-10)
	TOOL_LITHIC_MODE,
	TOOL_LITHIC_CORTEX,
	TOOL_LITHIC_ORIENTATION,
	TOOL_LITHIC_RIDGE,
	TOOL_LITHIC_PROPERTIES,

	// Ceramic Tools (11-15)
	TOOL_CERAMIC_MODE,
	TOOL_CERAMIC_ROLLOUT,
	TOOL_CERAMIC_RIM,
	TOOL_CERAMIC_PATTERN,
	TOOL_CERAMIC_PROPERTIES,

	// Measurement Tools (16-19)
	TOOL_MEASURE_DISTANCE,
	TOOL_MEASURE_ANGLE,
	TOOL_MEASURE_AREA,
	TOOL_MEASURE_CURVATURE,

	TOOL_COUNT = 20
};

//! Structure to hold tool information
struct ToolInfo {
	ArchaeologyToolID id;
	QString name;
	QString iconPath;
	QString tooltip;
	int category;  // 0=Section, 1=Lithic, 2=Ceramic, 3=Measure
};

//! QGMDockToolPalette - Archaeological Tool Palette Dock Widget
//!
//! Provides a categorized tool palette for archaeological illustration tasks.
//! Organizes 20 tools into 4 categories: Section Tools, Lithic Tools,
//! Ceramic Tools, and Measurement Tools.
//!
class QGMDockToolPalette : public QDockWidget
{
	Q_OBJECT

public:
	explicit QGMDockToolPalette(QWidget *parent = nullptr);
	~QGMDockToolPalette();

	//! Get currently selected tool ID
	ArchaeologyToolID getCurrentTool() const { return mCurrentTool; }

	//! Set current tool programmatically
	void setCurrentTool(ArchaeologyToolID toolId);

public slots:
	//! Enable/disable the tool palette
	void setEnabled(bool enabled);

	//! Update tool availability based on mesh state
	void updateToolAvailability(bool hasMesh, bool hasSelection);

signals:
	//! Emitted when a tool is activated
	void toolActivated(ArchaeologyToolID toolId);

	//! Emitted when tool selection changes
	void toolSelectionChanged(ArchaeologyToolID toolId);

private slots:
	//! Handle tool button clicks
	void onToolButtonClicked(int buttonId);

	//! Handle category tab changes
	void onCategoryChanged(int index);

private:
	//! Initialize the tool palette UI
	void initializeUI();

	//! Create tool buttons for a category
	QWidget* createCategoryWidget(int category);

	//! Create a single tool button
	QToolButton* createToolButton(const ToolInfo& toolInfo);

	//! Get tool information by ID
	const ToolInfo& getToolInfo(ArchaeologyToolID toolId) const;

	//! Setup all tool definitions
	void setupToolDefinitions();

	// UI Components
	QTabWidget* mTabWidget;
	QMap<int, QButtonGroup*> mButtonGroups;  // One group per category
	QMap<ArchaeologyToolID, QToolButton*> mToolButtons;

	// Tool Data
	QVector<ToolInfo> mToolDefinitions;
	ArchaeologyToolID mCurrentTool;

	// Category names
	static const QStringList mCategoryNames;

	// QWidget interface
protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKTOOLPALETTE_H
