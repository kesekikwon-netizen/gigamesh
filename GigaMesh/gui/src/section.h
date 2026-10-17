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

#ifndef SECTION_H
#define SECTION_H

#include <QString>
#include <QColor>
#include <GigaMesh/mesh/polyline.h>
#include <GigaMesh/mesh/vector3d.h>

//!
//! \brief Archaeological section (cross-section) for a mesh
//!
//! Represents a single cross-section plane with its settings and intersection polyline.
//! Used for archaeological illustration and measurement.
//!
class Section
{
public:
	//! Section type presets for quick creation
	enum SectionType {
		SECTION_CUSTOM,      //!< Custom user-defined section
		SECTION_TOP,         //!< Top view (XY plane, looking down)
		SECTION_FRONT,       //!< Front view (XZ plane, looking from front)
		SECTION_SIDE_RIGHT,  //!< Right side view (YZ plane, looking from right)
		SECTION_SIDE_LEFT,   //!< Left side view (YZ plane, looking from left)
		SECTION_OBLIQUE      //!< Oblique view (45° angle)
	};

	Section();
	Section(const QString& name, const Vector3D& planeHNF, SectionType type = SECTION_CUSTOM);
	~Section();

	// Basic properties
	QString getName() const { return mName; }
	void setName(const QString& name) { mName = name; }

	Vector3D getPlaneHNF() const { return mPlaneHNF; }
	void setPlaneHNF(const Vector3D& planeHNF);

	SectionType getType() const { return mType; }
	void setType(SectionType type) { mType = type; }

	// Intersection polyline
	PolyLine* getPolyLine() const { return mPolyLine; }
	void setPolyLine(PolyLine* polyLine);
	bool hasPolyLine() const { return mPolyLine != nullptr; }

	// Visibility
	bool isVisible() const { return mVisible; }
	void setVisible(bool visible) { mVisible = visible; }

	QColor getColor() const { return mColor; }
	void setColor(const QColor& color) { mColor = color; }

	double getLineWidth() const { return mLineWidth; }
	void setLineWidth(double width) { mLineWidth = width; }

	// Archaeological grid settings (방안선)
	bool isGridEnabled() const { return mGridEnabled; }
	void setGridEnabled(bool enabled) { mGridEnabled = enabled; }

	double getGridSpacing() const { return mGridSpacing; }
	void setGridSpacing(double spacing) { mGridSpacing = spacing; }

	QColor getGridColor() const { return mGridColor; }
	void setGridColor(const QColor& color) { mGridColor = color; }

	bool isGridLabelsEnabled() const { return mGridLabelsEnabled; }
	void setGridLabelsEnabled(bool enabled) { mGridLabelsEnabled = enabled; }

	// Ruler/measurement settings (치수)
	bool isRulerEnabled() const { return mRulerEnabled; }
	void setRulerEnabled(bool enabled) { mRulerEnabled = enabled; }

	QString getRulerUnit() const { return mRulerUnit; }
	void setRulerUnit(const QString& unit) { mRulerUnit = unit; }

	// Scale bar settings (스케일 바)
	bool isScaleBarEnabled() const { return mScaleBarEnabled; }
	void setScaleBarEnabled(bool enabled) { mScaleBarEnabled = enabled; }

	double getScaleBarLength() const { return mScaleBarLength; }
	void setScaleBarLength(double length) { mScaleBarLength = length; }

	// Editing mode for interactive plane manipulation
	bool isEditing() const { return mIsEditing; }
	void setEditing(bool editing) { mIsEditing = editing; }

	// Plane manipulation methods
	void translateAlongNormal(double distance);
	void rotateAroundAxis(const Vector3D& axis, double angleDegrees);

	// Factory method for preset sections
	static Section* createPreset(SectionType type, const Vector3D& centerPoint);

private:
	QString mName;                //!< Section name (e.g., "상면", "측면")
	Vector3D mPlaneHNF;           //!< Plane definition in Hesse Normal Form
	SectionType mType;            //!< Section type (preset or custom)
	PolyLine* mPolyLine;          //!< Intersection polyline (owned by this Section)

	// Display settings
	bool mVisible;                //!< Show/hide this section
	bool mIsEditing;              //!< Currently being edited (interactive mode)
	QColor mColor;                //!< Section line color
	double mLineWidth;            //!< Section line width in pixels

	// Grid settings (방안선)
	bool mGridEnabled;            //!< Show archaeological grid
	double mGridSpacing;          //!< Grid spacing in mesh units (typically cm)
	QColor mGridColor;            //!< Grid line color
	bool mGridLabelsEnabled;      //!< Show grid labels (A, B, C... / 1, 2, 3...)

	// Ruler settings (치수)
	bool mRulerEnabled;           //!< Show measurements
	QString mRulerUnit;           //!< Unit for measurements (mm, cm, m)

	// Scale bar settings (스케일 바)
	bool mScaleBarEnabled;        //!< Show scale bar in export
	double mScaleBarLength;       //!< Scale bar length in mesh units
};

#endif // SECTION_H
