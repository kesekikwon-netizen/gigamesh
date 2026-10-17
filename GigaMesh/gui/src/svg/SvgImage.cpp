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

#include "SvgImage.h"
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <sstream>

unsigned int SvgImage::imageIDs = 0;

SvgImage::SvgImage() : mWidth(0), mHeight(0), mXOffset(0.0), mYOffset(0.0), mID(imageIDs++), mXScale(1.0), mYScale(1.0)
{

}

void SvgImage::setImage(const unsigned char* data, unsigned int width, unsigned int height, unsigned int numComponents, double x, double y)
{
	QImage img(data, width, height, numComponents * width * sizeof (unsigned char), QImage::Format_RGBA8888);

	QByteArray ba;
	QBuffer bu(&ba);
	bu.open(QIODevice::WriteOnly);
	img.save(&bu, "PNG");
	mImgBase64String = ba.toBase64().toStdString();

	mWidth = width;
	mHeight = height;
	mXOffset = x;
	mYOffset = y;
}

void SvgImage::setScale(double xScale, double yScale)
{
	mXScale = xScale; mYScale = yScale;
}


std::string SvgImage::getString() const
{
	std::stringstream sStream;

	sStream << "<image id=\"image" << mID
	        << "\" width=\"" << mWidth
	        << "\" height=\"" << mHeight
			<< "\" xlink:href=\"data:image/png;base64," << mImgBase64String << "\" ";

	if(mXScale != 1.0 || mYScale != 1.0)
	{
		sStream << " transform=\"scale(" << mXScale << "," << mYScale <<")\"";
	}

	sStream << "/>" ;

	return sStream.str();
}
