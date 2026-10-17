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

#ifndef SVGRECT_H
#define SVGRECT_H

#include "SvgElement.h"

class SvgRect : public SvgElement
{
	public:
		SvgRect();
		SvgRect(double x, double y, double width, double height);

		void setSize(double width, double height);
		void setPosition(double x, double y);
		void setFill(bool fill);
		void setStrokeWidth(double width);
	private:
		double mX;
		double mY;
		double mWidth;
		double mHeight;
		double mStrokeWidth;

		bool mIsFilled;

		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGRECT_H
