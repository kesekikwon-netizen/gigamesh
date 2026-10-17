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

#ifndef SVGCIRCLE_H
#define SVGCIRCLE_H

#include <array>

#include "SvgElement.h"

class SvgCircle : public SvgElement
{
	public:
		SvgCircle();
		SvgCircle(double posX, double posY, double radius);

		void setFilled(bool filled);
		void setStroke(bool hasStroke);

		void setPosition(double posX, double posY);
		void setRadius(double radius);
		void setFillColor(double r, double g, double b);
		void setStrokeColor(double r, double g, double b);
	private:
		double mX;
		double mY;
		double mRadius;

		std::array<double, 3> mFillRGB;
		std::array<double, 3> mStrokeRGB;

		bool mIsFilled;
		bool mHasStroke;
		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGCIRCLE_H
