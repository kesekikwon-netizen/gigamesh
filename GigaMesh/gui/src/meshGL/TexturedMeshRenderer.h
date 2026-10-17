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

#ifndef TEXTUREDMESHRENDERER_H
#define TEXTUREDMESHRENDERER_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <string>
#include "TexturedMesh.h"
#include <filesystem>

class TexturedMeshRenderer
{
	public:
		TexturedMeshRenderer() = default;
		~TexturedMeshRenderer();

		TexturedMeshRenderer(const TexturedMeshRenderer& other) = delete;
		TexturedMeshRenderer(const TexturedMeshRenderer&& other) = delete;

		TexturedMeshRenderer& operator=(const TexturedMeshRenderer& other) = delete;
		TexturedMeshRenderer& operator=(const TexturedMeshRenderer&& other) = delete;

		struct LightInfo
		{
				QVector4D fixedCamDiffuse;
				QVector4D fixedCamSpecular;
				QVector4D fixedWorldDiffuse;
				QVector4D fixedWorldSpecular;
				QVector4D ambient;
				QVector4D clipPlane;
				QVector3D lightDirFixedCam;
				QVector3D lightDirFixedWorld;
				QVector3D clipVertexPos;
				double shininess = 1.0;
				bool lightEnabled = false;
		};

		bool init(const std::vector<std::filesystem::path>& textureNames);
		void render(const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelViewMatrix, TexturedMesh& texturedMesh, const LightInfo& lightInfo);

		void destroy();

		struct TexturedMeshVertex {
			GLfloat pos[3];
			GLfloat normal[3];
			GLfloat uv[2];
		};

		bool isInitialized() const;

		void setBackFaceColor(const QVector3D& backFaceColor);

		void setCullBackfaces(bool cullBackfaces);

	private:

		void setUpVertexBuffer(QOpenGLBuffer& vertexBuffer);
		bool initShader();
		bool mIsInitialized = false;

		QOpenGLVertexArrayObject mVAO;
		QOpenGLShaderProgram* mShader = nullptr;
		std::vector<QOpenGLTexture*> mTextures;
		QOpenGLFunctions_3_3_Core mGL;

		QVector3D mBackFaceColor = QVector3D(128.0F / 255.0F, 92.0F / 255.0F, 92.0F / 255.0F);
		bool mCullBackfaces = false;
};

#endif // TEXTUREDMESHRENDERER_H
