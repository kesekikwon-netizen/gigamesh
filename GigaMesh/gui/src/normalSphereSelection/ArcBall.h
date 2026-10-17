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

#ifndef ARCBALL_H
#define ARCBALL_H

#include <QQuaternion>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

class ArcBall
{
	public:
		ArcBall(const QVector3D& center = QVector3D(0.0,0.0,0.0), float radius = 2.0);

		~ArcBall() = default;

		void beginDrag(const QVector2D& screenCoords);

		void drag(const QVector2D& screenCoords);

		[[nodiscard]] QQuaternion getTransformationQuat() const;
		void setTransformationQuat(const QQuaternion& quat);
	private:

		QVector3D mouseOnSphere(const QVector2D& mousePos);
		static QQuaternion quatFromUnitSphere(const QVector3D& from, const QVector3D& to);

		QVector3D mCenter;
		float mRadius;

		QQuaternion mQNow;
		QQuaternion mQDown;

		QVector3D mMouseStart;
};

#endif // ARCBALL_H
