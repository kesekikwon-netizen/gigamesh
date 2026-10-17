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

#include "ArcBall.h"

#include <QtMath>

ArcBall::ArcBall(const QVector3D& center, float radius) : mCenter(center), mRadius(radius), mMouseStart(0.0F,0.0F,0.0F)
{

}

void ArcBall::beginDrag(const QVector2D& screenCoords)
{
	mQDown = mQNow;
	mMouseStart = mouseOnSphere(screenCoords);
}

void ArcBall::drag(const QVector2D& screenCoords)
{
	QVector3D currentMousePos = mouseOnSphere(screenCoords);

	QQuaternion qDrag = quatFromUnitSphere(mMouseStart, currentMousePos);

	mQNow = qDrag * mQDown;
}

QQuaternion ArcBall::getTransformationQuat() const
{
	return mQNow;
}

void ArcBall::setTransformationQuat(const QQuaternion& quat)
{
	mQNow = quat;
}

QVector3D ArcBall::mouseOnSphere(const QVector2D& mousePos)
{
	QVector3D ballMouse;

	ballMouse.setX( (mousePos.x() - mCenter.x()) / mRadius );
	ballMouse.setY( (mousePos.y() - mCenter.y()) / mRadius );

	float mag = QVector3D::dotProduct(ballMouse, ballMouse);	//mag == squared length == dot(x,x)

	if(mag > 1.0)
	{
		ballMouse *= 1.0F / sqrtf(mag);
		ballMouse.setZ(0.0);
	}

	else {
		ballMouse.setZ(sqrtf(1.0F - mag));
	}

	return ballMouse;
}

QQuaternion ArcBall::quatFromUnitSphere(const QVector3D& from, const QVector3D& to)
{
	QQuaternion q;
	q.setVector(QVector3D::crossProduct(from, to));
	q.setScalar(QVector3D::dotProduct(from, to));
	return q;
}
