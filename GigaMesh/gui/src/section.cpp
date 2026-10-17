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

#include "section.h"
#include <cmath>

Section::Section()
    : mName("New Section")
    , mPlaneHNF(0.0, 0.0, 1.0, 0.0)  // Default: XY plane at z=0
    , mType(SECTION_CUSTOM)
    , mPolyLine(nullptr)
    , mVisible(true)
    , mIsEditing(false)
    , mColor(Qt::blue)
    , mLineWidth(2.0)
    , mGridEnabled(false)
    , mGridSpacing(10.0)  // 10mm = 1cm default
    , mGridColor(Qt::gray)
    , mGridLabelsEnabled(true)
    , mRulerEnabled(false)
    , mRulerUnit("mm")
    , mScaleBarEnabled(true)
    , mScaleBarLength(50.0)  // 50mm = 5cm default
{
}

Section::Section(const QString& name, const Vector3D& planeHNF, SectionType type)
    : mName(name)
    , mPlaneHNF(planeHNF)
    , mType(type)
    , mPolyLine(nullptr)
    , mVisible(true)
    , mIsEditing(false)
    , mColor(Qt::blue)
    , mLineWidth(2.0)
    , mGridEnabled(false)
    , mGridSpacing(10.0)
    , mGridColor(Qt::gray)
    , mGridLabelsEnabled(true)
    , mRulerEnabled(false)
    , mRulerUnit("mm")
    , mScaleBarEnabled(true)
    , mScaleBarLength(50.0)
{
}

Section::~Section()
{
	// Clean up polyline if owned
	if (mPolyLine != nullptr) {
		delete mPolyLine;
		mPolyLine = nullptr;
	}
}

void Section::setPlaneHNF(const Vector3D& planeHNF)
{
	mPlaneHNF = planeHNF;
	// When plane changes, invalidate the polyline (it needs to be recalculated)
	if (mPolyLine != nullptr) {
		delete mPolyLine;
		mPolyLine = nullptr;
	}
}

void Section::setPolyLine(PolyLine* polyLine)
{
	// Delete old polyline if exists
	if (mPolyLine != nullptr) {
		delete mPolyLine;
	}
	mPolyLine = polyLine;
}

//! Factory method to create preset sections
//! @param type The type of preset section to create
//! @param centerPoint The center point for positioning the plane
//! @returns A new Section object with preset plane orientation
Section* Section::createPreset(SectionType type, const Vector3D& centerPoint)
{
	Section* section = nullptr;
	Vector3D planeHNF;
	QString name;

	double cx = centerPoint.getX();
	double cy = centerPoint.getY();
	double cz = centerPoint.getZ();

	switch (type) {
		case SECTION_TOP:
			// Top view: looking down Z axis, XY plane
			// Normal: (0, 0, 1), d = -cz
			planeHNF = Vector3D(0.0, 0.0, 1.0, -cz);
			name = QString::fromUtf8("상면 (Top)");
			break;

		case SECTION_FRONT:
			// Front view: looking along -Y axis, XZ plane
			// Normal: (0, 1, 0), d = -cy
			planeHNF = Vector3D(0.0, 1.0, 0.0, -cy);
			name = QString::fromUtf8("정면 (Front)");
			break;

		case SECTION_SIDE_RIGHT:
			// Right side view: looking along +X axis, YZ plane
			// Normal: (1, 0, 0), d = -cx
			planeHNF = Vector3D(1.0, 0.0, 0.0, -cx);
			name = QString::fromUtf8("우측면 (Right)");
			break;

		case SECTION_SIDE_LEFT:
			// Left side view: looking along -X axis, YZ plane
			// Normal: (-1, 0, 0), d = cx
			planeHNF = Vector3D(-1.0, 0.0, 0.0, cx);
			name = QString::fromUtf8("좌측면 (Left)");
			break;

		case SECTION_OBLIQUE:
			// Oblique view: 45° angle between XY plane
			// Normal: (1, 1, 0) normalized, d calculated from center
			{
				double nx = 1.0 / std::sqrt(2.0);
				double ny = 1.0 / std::sqrt(2.0);
				double nz = 0.0;
				double d = -(nx * cx + ny * cy + nz * cz);
				planeHNF = Vector3D(nx, ny, nz, d);
				name = QString::fromUtf8("사투영 (Oblique)");
			}
			break;

		case SECTION_CUSTOM:
		default:
			// Custom section with default plane
			planeHNF = Vector3D(0.0, 0.0, 1.0, -cz);
			name = QString::fromUtf8("사용자 정의");
			break;
	}

	section = new Section(name, planeHNF, type);

	// Set default colors based on type
	switch (type) {
		case SECTION_TOP:
			section->setColor(QColor(0, 0, 255));  // Blue
			break;
		case SECTION_FRONT:
			section->setColor(QColor(255, 0, 0));  // Red
			break;
		case SECTION_SIDE_RIGHT:
		case SECTION_SIDE_LEFT:
			section->setColor(QColor(0, 128, 0));  // Green
			break;
		case SECTION_OBLIQUE:
			section->setColor(QColor(128, 0, 128));  // Purple
			break;
		default:
			section->setColor(QColor(0, 0, 0));  // Black
			break;
	}

	return section;
}

//! Translate the plane along its normal direction
//! @param distance Distance to move (positive = along normal, negative = opposite)
void Section::translateAlongNormal(double distance)
{
	// In Hesse Normal Form: (nx, ny, nz, d)
	// Moving along normal means: d_new = d_old - distance
	// (because d is the signed distance from origin)

	double nx = mPlaneHNF.getX();
	double ny = mPlaneHNF.getY();
	double nz = mPlaneHNF.getZ();
	double d = mPlaneHNF.getH();

	double d_new = d - distance;

	mPlaneHNF = Vector3D(nx, ny, nz, d_new);

	// Invalidate polyline (needs recalculation)
	if (mPolyLine != nullptr) {
		delete mPolyLine;
		mPolyLine = nullptr;
	}
}

//! Rotate the plane around an arbitrary axis
//! @param axis Rotation axis (will be normalized)
//! @param angleDegrees Rotation angle in degrees
void Section::rotateAroundAxis(const Vector3D& axis, double angleDegrees)
{
	// Get plane normal
	double nx = mPlaneHNF.getX();
	double ny = mPlaneHNF.getY();
	double nz = mPlaneHNF.getZ();
	double d = mPlaneHNF.getH();

	// Normalize rotation axis
	Vector3D axisNorm = ::normalize3(axis);
	double ax = axisNorm.getX();
	double ay = axisNorm.getY();
	double az = axisNorm.getZ();

	// Convert angle to radians
	double angleRad = angleDegrees * M_PI / 180.0;
	double c = std::cos(angleRad);
	double s = std::sin(angleRad);
	double t = 1.0 - c;

	// Rodrigues' rotation formula (rotation matrix)
	// R = I + sin(θ)K + (1-cos(θ))K²
	// where K is the skew-symmetric matrix of axis

	double r00 = c + ax*ax*t;
	double r01 = ax*ay*t - az*s;
	double r02 = ax*az*t + ay*s;

	double r10 = ay*ax*t + az*s;
	double r11 = c + ay*ay*t;
	double r12 = ay*az*t - ax*s;

	double r20 = az*ax*t - ay*s;
	double r21 = az*ay*t + ax*s;
	double r22 = c + az*az*t;

	// Rotate normal vector
	double nx_new = r00*nx + r01*ny + r02*nz;
	double ny_new = r10*nx + r11*ny + r12*nz;
	double nz_new = r20*nx + r21*ny + r22*nz;

	// d remains the same (rotation around axis through origin)
	// If we want to rotate around a different point, we'd need translation

	mPlaneHNF = Vector3D(nx_new, ny_new, nz_new, d);

	// Invalidate polyline
	if (mPolyLine != nullptr) {
		delete mPolyLine;
		mPolyLine = nullptr;
	}
}
