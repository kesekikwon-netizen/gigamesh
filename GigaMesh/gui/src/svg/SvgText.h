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

#ifndef SVGTEXT_H
#define SVGTEXT_H

#include <string>
#include "SvgElement.h"

class SvgText : public SvgElement
{
	public:

		enum TextAnchor {
			ANCHOR_START,
			ANCHOR_MIDDLE,
			ANCHOR_END
		};

		SvgText();

		void setText(const std::string& text);
		void setText(const std::string& text, int x, int y);

		void setTextAnchor(TextAnchor anchor);

		void setPosition(double x, double y);
		void setSize(double fontSize);
		void setFont(const std::string& font);
		void setRotation(double angle);
	private:
		double mX;
		double mY;
		double mFontSize;
		double mRotation;

		TextAnchor mAnchor;

		std::string mText;
		std::string mFont;

		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGTEXT_H
