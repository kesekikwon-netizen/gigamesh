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

#ifndef SVGIMAGE_H
#define SVGIMAGE_H
#include <string>
#include "SvgElement.h"

class SvgImage : public SvgElement
{
	public:
		SvgImage();

		void setImage(const unsigned char* data, unsigned int width, unsigned int height, unsigned int numComponents, double x, double y);

		void setScale(double xScale, double yScale);
	private:
		static unsigned int imageIDs;

		std::string mImgBase64String;
		unsigned int mWidth;
		unsigned int mHeight;
		double mXOffset;
		double mYOffset;
		unsigned int mID;

		double mXScale;
		double mYScale;
		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGIMAGE_H
