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

#ifndef PINRENDERER_H
#define PINRENDERER_H

#include "meshGL.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix>

class PinRenderer
{
public:

	struct PinVertex {
		GLfloat  position[3]; //!< xyz-coordinate.
		GLfloat  normal[3];   //!< Normal vector.
		GLubyte  color[3];    //!< Color information (RGBA).
	};

	PinRenderer();
	~PinRenderer();

	//prevent copy and moving
	PinRenderer(const PinRenderer& other) = delete;
	PinRenderer& operator=(const PinRenderer& other) = delete;

	PinRenderer(const PinRenderer&& other) = delete;
	PinRenderer& operator=(const PinRenderer&& other) = delete;

	//call these with valid OpenGL context
	bool init();
	void destroy();
	void render(std::vector<PinVertex> &pinPoints, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelViewMatrix, float pinSize);


	bool isInitialized();
private:

	bool initializeShader();
	bool initializePinGeometry();

	bool resizePinPointsBuffer(unsigned int size);

	bool mIsInitialized;
	unsigned int mAllocatedBufferSize;

	unsigned int mPinFaceIndicesSize;

	QOpenGLFunctions_3_3_Core mGL;
	QOpenGLShaderProgram* mShaderPins = nullptr;

	QOpenGLBuffer mPinPositionBuffer;
	QOpenGLBuffer mPinNormalBuffer;
	QOpenGLBuffer mPinHeadFlags;
	QOpenGLBuffer mPinFaceIndexBuffer;
	QOpenGLBuffer mInstancedPinBuffer;
	QOpenGLVertexArrayObject mVAO;
};

#endif // PINRENDERER_H
