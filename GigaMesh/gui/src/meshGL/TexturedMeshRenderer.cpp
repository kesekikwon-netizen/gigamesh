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

#include "TexturedMeshRenderer.h"
#include <GigaMesh/mesh/mesh.h>
#include <array>
#include <list>
#include <string>
#include <QFileInfo>
#include "glmacros.h"

#include <GigaMesh/logging/Logging.h>

TexturedMeshRenderer::~TexturedMeshRenderer()
{
	if(mIsInitialized)
	{
		destroy();
	}
}

bool TexturedMeshRenderer::init(const std::vector<std::filesystem::path>& textureNames)
{
	if(mIsInitialized)
		return true;

	mGL.initializeOpenGLFunctions();

	GLint prevVAO;
	mGL.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
	//init shader

	if(!initShader())
		return false;

	mVAO.create();
	mVAO.bind();

	mVAO.release();

	mGL.glBindVertexArray(prevVAO);

	mTextures.resize(textureNames.size());

	for(size_t i = 0; i<textureNames.size(); ++i)
	{
		QString textureName = textureNames[i].string().c_str();

		if(QFileInfo::exists(textureName))
		{
			QImage texImage(textureName);
			mTextures[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
			mTextures[i]->setData(texImage.mirrored(), QOpenGLTexture::GenerateMipMaps);
		}
		else
		{
			LOG::warn() << "[TexturedMeshRenderer::" << __FUNCTION__ << "] Missing texture: " << textureNames[i] << "\n";
			QImage texImage(1,1, QImage::Format_RGB16);
			texImage.fill(0);
			mTextures[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
			mTextures[i]->setData(texImage.mirrored(), QOpenGLTexture::GenerateMipMaps);
		}
	}

	mIsInitialized = true;
	return true;
}

//assumes that vao and shader of the TexturedMesh Renderer is bound
//should only be called from render
void TexturedMeshRenderer::setUpVertexBuffer(QOpenGLBuffer& vertexBuffer)
{
	vertexBuffer.bind();

	mShader->setAttributeBuffer( "vPosition", GL_FLOAT, offsetof(TexturedMeshVertex, pos), 3, sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vPosition" );

	mShader->setAttributeBuffer( "vNormal", GL_FLOAT, offsetof(TexturedMeshVertex, normal), 3, sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vNormal" );

	mShader->setAttributeBuffer( "vUV", GL_FLOAT, offsetof(TexturedMeshVertex, uv), 2 , sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vUV" );
}

void TexturedMeshRenderer::render(const QMatrix4x4& projectionMatrix, const QMatrix4x4& modelViewMatrix, TexturedMesh& texturedMesh, const LightInfo& lightInfo)
{
	if(!mIsInitialized)
		return;

	PRINT_OPENGL_ERROR("start of textured mesh renderer");

	GLint prevVAO;
	mGL.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);

	if(!mCullBackfaces)
	{
		mGL.glDisable(GL_CULL_FACE);
	}

	mShader->bind();
	mVAO.bind();

	//set uniforms
	mShader->setUniformValue("modelViewMat", modelViewMatrix);
	mShader->setUniformValue("projectionMat", projectionMatrix);

	mShader->setUniformValue("uTexture", 0);
	mShader->setUniformValue("uLightDirectionFixedCamera", lightInfo.lightDirFixedCam  );
	mShader->setUniformValue("uLightDirectionFixedWorld" , lightInfo.lightDirFixedWorld);
	mShader->setUniformValue("uLightEnabled"             , lightInfo.lightEnabled      );
	mShader->setUniformValue("FixedCam_DiffuseProduct"   , lightInfo.fixedCamDiffuse   );
	mShader->setUniformValue("FixedCam_SpecularProduct"  , lightInfo.fixedCamSpecular  );
	mShader->setUniformValue("FixedWorld_DiffuseProduct" , lightInfo.fixedWorldDiffuse );
	mShader->setUniformValue("FixedWorld_SpecularProduct", lightInfo.fixedWorldSpecular);
	mShader->setUniformValue("AmbientProduct"            , lightInfo.ambient           );
	mShader->setUniformValue("Shininess"                 , static_cast<GLfloat>(lightInfo.shininess));
	mShader->setUniformValue("uBackFaceColor"            , mBackFaceColor              );
	mShader->setUniformValue("uClipPlane0"               , lightInfo.clipPlane         );
	mShader->setUniformValue("uClipBefore"               , lightInfo.clipVertexPos     );

	mGL.glActiveTexture(GL_TEXTURE0);
	PRINT_OPENGL_ERROR("active texture");

	unsigned int elementSize = TexturedMesh::getVertexElementSize();

	for(auto& bufferPair : texturedMesh.getVertexBuffers())
	{
		if(bufferPair.first > mTextures.size())
			continue;

		//bind textureid of bufferPair 1
		mGL.glBindTexture(GL_TEXTURE_2D, mTextures[bufferPair.first]->textureId());

		for(auto& buffer : bufferPair.second)
		{
			setUpVertexBuffer(buffer);
			mGL.glDrawArrays(GL_TRIANGLES, 0, buffer.size() / elementSize);
		}

		mTextures[bufferPair.first]->release();
	}

	//mGL.glDrawArrays(GL_TRIANGLES, 0, numVertices);

	PRINT_OPENGL_ERROR("draw Arrays");
	mVAO.release();
	mShader->release();

	mGL.glEnable(GL_CULL_FACE);
	mGL.glBindVertexArray(prevVAO);
	mGL.glBindTexture(GL_TEXTURE_2D, 0);
	PRINT_OPENGL_ERROR("release texture");
}

void TexturedMeshRenderer::destroy()
{
	mShader->removeAllShaders();
	delete mShader;
	mShader = nullptr;

	mVAO.destroy();

	for(auto& texture : mTextures)
	{
		texture->destroy();
		delete texture;
	}

	mTextures.clear();
	mIsInitialized = false;
}

bool TexturedMeshRenderer::initShader()
{

	mShader = new QOpenGLShaderProgram;

	if(!mShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/GMShaders/textureMesh/texturemesh.vert"))
	{
		QString errMsg = mShader->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[TexturedMeshRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "TEXMESH" << "/vert): " << errMsg.toStdString() << std::endl;
		mShader->removeAllShaders();
		return false;
	}

	if(!mShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GMShaders/textureMesh/texturemesh.frag"))
	{
		QString errMsg = mShader->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[TexturedMeshRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "TEXMESH" << "/frag): " << errMsg.toStdString() << std::endl;
		mShader->removeAllShaders();
		return false;
	}

	if(!mShader->link())
	{
		QString errMsg = mShader->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[TexturedMeshRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "TEXMESH" << "/program): " << errMsg.toStdString() << std::endl;
		mShader->removeAllShaders();
		return false;
	}
	return true;
}

void TexturedMeshRenderer::setCullBackfaces(bool cullBackfaces)
{
	mCullBackfaces = cullBackfaces;
}

void TexturedMeshRenderer::setBackFaceColor(const QVector3D& backFaceColor)
{
	mBackFaceColor = backFaceColor;
}

bool TexturedMeshRenderer::isInitialized() const
{
	return mIsInitialized;
}
