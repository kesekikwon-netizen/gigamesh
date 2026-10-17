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

#include "PinRenderer.h"
#include <vector>
#include "glmacros.h"
#include "PinGeometry.h"

PinRenderer::PinRenderer() : mIsInitialized(false),
							 mAllocatedBufferSize(0),
							 mPinPositionBuffer(QOpenGLBuffer::VertexBuffer),
							 mPinNormalBuffer(QOpenGLBuffer::VertexBuffer),
							 mPinHeadFlags(QOpenGLBuffer::VertexBuffer),
							 mPinFaceIndexBuffer(QOpenGLBuffer::IndexBuffer),
							 mInstancedPinBuffer(QOpenGLBuffer::VertexBuffer)
{

}

PinRenderer::~PinRenderer()
{
	if(mIsInitialized)
		destroy();
}

bool PinRenderer::init()
{
	if(mIsInitialized)
		return true;

	mGL.initializeOpenGLFunctions();

	if(!mShaderPins)
	{
		if(!initializeShader())
		{
			return false;
		}
	}

	if(!initializePinGeometry())
		return false;

	mIsInitialized = true;
	return true;
}

void PinRenderer::destroy()
{
	if(!mIsInitialized)
		return;

	mPinPositionBuffer.destroy();
	mPinNormalBuffer.destroy();
	mPinHeadFlags.destroy();
	mPinFaceIndexBuffer.destroy();
	mInstancedPinBuffer.destroy();
	mVAO.destroy();

	mIsInitialized = false;
	mAllocatedBufferSize = 0;

	delete mShaderPins;
	mShaderPins = nullptr;
}

void PinRenderer::render(std::vector<PinVertex> &pinPoints, const QMatrix4x4& projectionMatrix, const QMatrix4x4& modelViewMatrix, float pinSize)
{
	if(!mIsInitialized || pinPoints.empty())
	{
		return;
	}
	GLint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);

	mShaderPins->bind();
	mVAO.bind();

	if(mAllocatedBufferSize < pinPoints.size())
	{
		resizePinPointsBuffer(pinPoints.size());
	}

	mInstancedPinBuffer.bind();
	mInstancedPinBuffer.write(0,pinPoints.data(), sizeof(PinVertex) * pinPoints.size());
	PRINT_OPENGL_ERROR( "Writing pin instenced data" );


	mGL.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Draw
	mShaderPins->setUniformValue("modelview", modelViewMatrix);
	mShaderPins->setUniformValue("projection", projectionMatrix);

	mShaderPins->setUniformValue("scaleFactor", pinSize );
	mGL.glDrawElementsInstanced(GL_TRIANGLES, mPinFaceIndicesSize, GL_UNSIGNED_INT,  nullptr, pinPoints.size());
	PRINT_OPENGL_ERROR( "glDrawArrays( ... )" );

	mVAO.release();
	mShaderPins->release();

	mGL.glBindVertexArray(prevVAO);
}

bool PinRenderer::isInitialized()
{
	return mIsInitialized;
}

bool PinRenderer::initializeShader()
{
	mShaderPins = new QOpenGLShaderProgram;

	if(!mShaderPins->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/GMShaders/vertex_pin.vert"))
	{
		QString errMsg = mShaderPins->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[PinRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "PIN" << "/vert): " << errMsg.toStdString() << std::endl;
		mShaderPins->removeAllShaders();
		return false;
	}

	if(!mShaderPins->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GMShaders/vertex_pin.frag"))
	{
		QString errMsg = mShaderPins->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[PinRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "PIN" << "/frag): " << errMsg.toStdString() << std::endl;
		mShaderPins->removeAllShaders();
		return false;
	}

	if(!mShaderPins->link())
	{
		QString errMsg = mShaderPins->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[PinRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "PIN" << "/program): " << errMsg.toStdString() << std::endl;
		mShaderPins->removeAllShaders();
		return false;
	}
	return true;
}

bool PinRenderer::initializePinGeometry()
{

	GLint prevVAO;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);

	mVAO.create();
	mVAO.bind();
	std::vector<float> positions = PinGeometry::getPositions();
	std::vector<float> normals = PinGeometry::getNormals();
	std::vector<float> headVertexFlag = PinGeometry::getHeadVertexFlag();

	std::vector<unsigned int> faceIndices = PinGeometry::getFaceIndices();

	mPinFaceIndicesSize = faceIndices.size();

	mPinPositionBuffer.create();
	mPinPositionBuffer.bind();
	mPinPositionBuffer.allocate( positions.data(), sizeof(float) * positions.size());
	mShaderPins->setAttributeBuffer( "position", GL_FLOAT, 0, 3, 0 );
	mShaderPins->enableAttributeArray( "position" );


	mPinNormalBuffer.create();
	mPinNormalBuffer.bind();
	mPinNormalBuffer.allocate( normals.data(), sizeof(float) * normals.size());
	mShaderPins->setAttributeBuffer( "vNormal", GL_FLOAT, 0, 3, 0 );
	mShaderPins->enableAttributeArray( "vNormal" );

	mPinHeadFlags.create();
	mPinHeadFlags.bind();
	mPinHeadFlags.allocate( headVertexFlag.data(), sizeof(float) * headVertexFlag.size());
	mShaderPins->setAttributeBuffer( "vHeadFlag", GL_FLOAT, 0, 1, 0 );
	mShaderPins->enableAttributeArray( "vHeadFlag" );


	mPinFaceIndexBuffer.create();
	mPinFaceIndexBuffer.bind();
	mPinFaceIndexBuffer.allocate( faceIndices.data(), sizeof(unsigned int) * faceIndices.size());

	mInstancedPinBuffer.create();
	mInstancedPinBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	mInstancedPinBuffer.bind();
	mInstancedPinBuffer.allocate(0);
	PRINT_OPENGL_ERROR( "enabling attribute Array vHeadFlag" );


	mShaderPins->setAttributeBuffer( "offsetInstanced", GL_FLOAT, offsetof(PinVertex, position), 3, static_cast<int>(sizeof(PinVertex)) );
	PRINT_OPENGL_ERROR( "Shader setting Attribute buffer offsetInstanced" );
	mShaderPins->enableAttributeArray( "offsetInstanced" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray offsetInstanced" );

	mShaderPins->setAttributeBuffer( "colorInstanced", GL_UNSIGNED_BYTE, offsetof(PinVertex, color), 3, static_cast<int>(sizeof(PinVertex)));
	mShaderPins->enableAttributeArray("colorInstanced");
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray colorInstanced" );


	mShaderPins->setAttributeBuffer( "pinDirectionInstanced", GL_FLOAT, offsetof(PinVertex, normal), 3, static_cast<int>(sizeof(PinVertex)));
	mShaderPins->enableAttributeArray( "pinDirectionInstanced");
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray pinDirectionInstanced" );

	mGL.glVertexAttribDivisor(mShaderPins->attributeLocation("offsetInstanced"),1);
	mGL.glVertexAttribDivisor(mShaderPins->attributeLocation("colorInstanced"),1);
	//PRINT_OPENGL_ERROR( "Shader attribdivisor colorInstanced" );

	mGL.glVertexAttribDivisor(mShaderPins->attributeLocation("pinDirectionInstanced"),1);
	PRINT_OPENGL_ERROR( "Shader attribdivisor pinDirectionInstanced" );

	mVAO.release();

	mGL.glBindVertexArray(prevVAO);
	return true;
}

bool PinRenderer::resizePinPointsBuffer(unsigned int size)
{

	mAllocatedBufferSize = size;
	mInstancedPinBuffer.bind();
	mInstancedPinBuffer.allocate(size * sizeof (PinVertex));
	return true;
}
