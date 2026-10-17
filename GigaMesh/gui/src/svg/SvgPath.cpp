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

#include "SvgPath.h"
#include <cmath>

SvgPath::SvgPath() : mLineWidth(1.0),
	mCapStyle(LineCap::CAP_UNSET),
	mJoinStyle(LineJoin::JOIN_UNSET),
	mDashOffset(0.0),
	mR(0.0),
	mG(0.0),
	mB(0.0),
	mA(1.0),
	mXScale(1.0),
	mYScale(1.0),
	mIsFilled(false)
{

}

void SvgPath::moveTo(double x, double y)
{
	mElementsStream << "M" << x << " " << y << " ";
}

void SvgPath::moveToRel(double x, double y)
{
	mElementsStream << "m" << x << " " << y << " ";
}

void SvgPath::lineTo(double x, double y)
{
	mElementsStream << "L" << x << " " << y << " ";
}

void SvgPath::lineToRel(double x, double y)
{
	mElementsStream << "l" << x << " " << y << " ";
}

void SvgPath::closePath()
{
	mElementsStream << "Z" << " ";
}

void SvgPath::setLineWidth(float width)
{
	mLineWidth = width;
}

void SvgPath::setLineCap(SvgPath::LineCap cap)
{
	mCapStyle = cap;
}

void SvgPath::setLineJoin(SvgPath::LineJoin join)
{
	mJoinStyle = join;
}

void SvgPath::setLineDash(const double* dashes, int num_dashes, double offset)
{
	mDashes.resize(num_dashes);
	for(int i = 0; i<num_dashes; ++i)
		mDashes[i] = dashes[i];

	mDashOffset = offset;
}

void SvgPath::setLineDash(const std::vector<double>& dashes, double offset)
{
	mDashes = dashes;
	mDashOffset = offset;
}

void SvgPath::setColor(double r, double g, double b, double a)
{
	mR = r; mG = g; mB = b; mA = a;
}

void SvgPath::setScale(double xScale, double yScale)
{
	mXScale = xScale;
	mYScale = yScale;
}

void SvgPath::setFilled(bool fill)
{
	mIsFilled = fill;
}

std::string SvgPath::getString() const
{
	if(mElementsStream.rdbuf()->in_avail() == 0)
		return "";

	std::string lineJoin = "miter";
	switch (mJoinStyle) {
		case JOIN_BEVEL:
			lineJoin = "bevel";
			break;
		case JOIN_ROUND:
			lineJoin = "round";
			break;
		case JOIN_MITER:
		case JOIN_UNSET:
			//default value
			break;
	}

	std::string lineCap = "butt";
	switch (mCapStyle) {
		case CAP_ROUND:
			lineCap = "round";
			break;
		case CAP_SQUARE:
			lineCap = "square";
			break;
		case CAP_BUTT:
		case CAP_UNSET:
		default:
			//default value
			break;
	}

	std::string fillText = "none";

	if(mIsFilled)
	{
		std::stringstream rgbText;
		rgbText << "rgb(" << mR * 100.0 << "%," << mG * 100.0 << "%," << mB*100.0 << "%)";
		fillText = rgbText.str();
	}
	std::stringstream sStream;
	sStream << "<path style=\"fill:" << fillText << ";stroke-width:" << mLineWidth << ";stroke-linecap:"<<lineCap<<";stroke-linejoin:"<< lineJoin <<";stroke:rgb(" << mR * 100.0 << "%," << mG * 100.0 << "%," << mB*100.0 << "%);stroke-opacity:" << mA << "1;stroke-miterlimit:10;\" d=\"";

	sStream << mElementsStream.str();

	sStream << "\"";

	if(!mDashes.empty())
	{
		sStream << " stroke-dasharray=\"";
		for(const auto dash : mDashes)
			sStream << dash << " ";
		sStream << "\" stroke-dashoffset=\"" << mDashOffset << "\"";
	}

	if(mXScale != 0.0 || mYScale != 0.0)
	{
		sStream << " transform=\"scale(" << mXScale << "," << mYScale << ")\"";
	}

	sStream << " />";
	return sStream.str();

}
