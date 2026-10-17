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

#include "SvgText.h"
#include <sstream>

SvgText::SvgText() : mX(0), mY(0), mFontSize(-1.0), mRotation(0.0), mAnchor(TextAnchor::ANCHOR_START)
{

}

void SvgText::setText(const std::string& text)
{
	mText = text;
}

void SvgText::setText(const std::string& text, int x, int y)
{
	mText = text;
	mX = x;
	mY = y;
}

void SvgText::setTextAnchor(SvgText::TextAnchor anchor)
{
	mAnchor = anchor;
}

void SvgText::setPosition(double x, double y)
{
	mX = x;
	mY = y;
}

void SvgText::setSize(double fontSize)
{
	mFontSize = fontSize;
}

void SvgText::setFont(const std::string& font)
{
	mFont = font;
}

void SvgText::setRotation(double angle)
{
	mRotation = angle;
}

std::string SvgText::getString() const
{
	std::stringstream sStream;

	sStream << "<text x=\"" << mX
	        << "\" y=\"" << mY
			<< "\" ";

	if(!mFont.empty() || mFontSize >= 0.0)
	{
		sStream << "style=\"";
		if(!mFont.empty())
		{
			sStream << "font-family:" << mFont << ";";
		}
		if(mFontSize >= 0.0)
		{
			sStream << "font-size:" << mFontSize << "px;";
		}
		sStream << "\"";
	}

	if(mAnchor != TextAnchor::ANCHOR_START)
	{
		switch (mAnchor) {
			case TextAnchor::ANCHOR_MIDDLE:
				sStream << R"( text-anchor="middle")";
				break;
			case TextAnchor::ANCHOR_END:
				sStream << R"( text-anchor="end")";
				break;
			default:
				break;
		}

	}

	if(mRotation != 0.0)
	{
		sStream << R"( transform="rotate( )" << mRotation << " " << mX << " " << mY << ")\"";
	}

	sStream << ">" << mText << "</text>";
	return sStream.str();
}
