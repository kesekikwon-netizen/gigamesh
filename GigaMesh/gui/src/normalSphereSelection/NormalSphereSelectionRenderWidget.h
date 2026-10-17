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

#ifndef NormalSphereSELECTIONRenderWIDGET_H
#define NormalSphereSELECTIONRenderWIDGET_H

#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <vector>

#include "ArcBall.h"
#include <GigaMesh/icoSphereTree/IcoSphereTree.h>

class NormalSphereSelectionRenderWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
		Q_OBJECT
	public:
		explicit NormalSphereSelectionRenderWidget(QWidget* parent = nullptr);
		~NormalSphereSelectionRenderWidget() override;

		//moves the data from normals to internal data => normals is afterward empty!
		void setRenderNormals(std::vector<float>& normals);

		void setSelected(double nx, double ny, double nz);
		void clearSelected();
		bool isNormalSelected(double nx, double ny, double nz);
		// QWidget interface

		void setColorMapIndex(unsigned int index);
		float selectionRadius() const;
		void setSelectionRadius(float selectionRadius);

		GLubyte selectionMask() const;
		void setSelectionMask(const GLubyte& selectionMask);

		void setRotation(const QQuaternion& rotQuat);
		QQuaternion getRotation();

		void setScaleNormals(bool enable);
		void setInvertFuncVal(bool invertFuncVal);

		void setUpperQuantil(float upperQuantil);

	signals:
		void rotationChanged(QQuaternion quat);

	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

		// QOpenGLWidget interface
	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;

	private:
		void initializeUnitSphereBuffer();
		void refreshNormals();
		void selectAt(int xCoord, int yCoord);

		QMatrix4x4 mViewMatrix;
		QMatrix4x4 mProjectionMatrix;

		QOpenGLTexture mFuncValTexture;
		QOpenGLTexture mSelectionTexture;

		QOpenGLBuffer mIcosphereBuffer;
		QOpenGLBuffer mIcosphereDataBuffer;
		QOpenGLBuffer mIcosphereIndices;
		QOpenGLVertexArrayObject mIcoSphereVAO;

		//create QOpenGLShaderprogram on heap, because there is no destroy function and we have to rely on a valid context on destruction
		QOpenGLShaderProgram* mIcoSphereShader = nullptr;

		std::vector<float> mNormalUpload;
		std::vector<GLubyte> mSelectionBuffer;

		IcoSphereTree mIcoSphereTree;
		ArcBall mArcBall;


		int mScreenWidth = 0;
		int mScreenHeight = 0;
		unsigned int mColorMapIndex = 0;

		float mSelectionRadius = 1.0F;
		float mUpperQuantil = 1.0F;

		bool mScaleNormals = false;
		bool mInvertFuncVal = false;
		bool mUpdateSelectionTexture = false;
		GLubyte mSelectionMask = 255;
};

#endif // NormalSphereSELECTIONRenderWIDGET_H
