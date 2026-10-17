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

#ifndef SVGPATH_H
#define SVGPATH_H

#include "SvgElement.h"
#include <vector>
#include <list>
#include <sstream>

class SvgPath : public SvgElement
{
	public:
		enum LineCap {
			CAP_UNSET,
			CAP_BUTT,
			CAP_ROUND,
			CAP_SQUARE,
		};

		enum LineJoin {
			JOIN_UNSET,
			JOIN_MITER,
			JOIN_ROUND,
			JOIN_BEVEL
		};

		SvgPath();
		~SvgPath() override = default;

		void moveTo(double x, double y);
		void moveToRel(double x, double y);
		void lineTo(double x, double y);
		void lineToRel(double x, double y);
		void closePath();

		void setLineWidth(float width);
		void setLineCap(LineCap cap);
		void setLineJoin(LineJoin join);
		void setLineDash(const double* dashes, int num_dashes, double offset);
		void setLineDash(const std::vector<double>& dashes, double offset);

		void setColor(double r, double g, double b, double a = 1.0);
		void setScale(double xScale, double yScale);

		void setFilled(bool fill);
	private:
		std::stringstream mElementsStream;

		float mLineWidth;
		LineCap mCapStyle;
		LineJoin mJoinStyle;

		std::vector<double> mDashes;
		double mDashOffset;
		double mR;
		double mG;
		double mB;
		double mA;

		double mXScale;
		double mYScale;

		bool mIsFilled;
		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGPATH_H
