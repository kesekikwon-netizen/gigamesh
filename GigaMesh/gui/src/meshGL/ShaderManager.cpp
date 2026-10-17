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

#include "ShaderManager.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QString>

#include <GigaMesh/logging/Logging.h>
#include "../QGMMacros.h"

QOpenGLShaderProgram* shaderLink(const QString& vertSrc,
                          const QString& geomSrc,
                          const QString& fragSrc,
                          const QString& name);

ShaderManager::~ShaderManager()
{
	for(auto shaderProgram : mShaderPrograms)
	{
		delete shaderProgram;
	}
}

//! gets shaderprogram assiciated with ShaderName enum
//! @returns pointer to shaderprogram if successfull, nullptr otherwise
QOpenGLShaderProgram* ShaderManager::getShader(ShaderManager::ShaderName shaderName)
{
	if(mBrokenShader[static_cast<size_t>(shaderName)] == true)
	{
		return nullptr;
	}

	auto& shaderPtr = mShaderPrograms[static_cast<size_t>(shaderName)];

	if(shaderPtr == nullptr)
	{
		shaderPtr = initShader(shaderName);
		if(shaderPtr == nullptr)
		{
			mBrokenShader[static_cast<size_t>(shaderName)] = true;
		}
	}

	return shaderPtr;
}

//! creates a new shader based on enum passed
//! @returns ShaderProgram pointer if successfull, nullptr otherwise
QOpenGLShaderProgram* ShaderManager::initShader(ShaderManager::ShaderName shaderName)
{
	switch(shaderName)
	{
		case ShaderName::BOUNDING_BOX:
			return shaderLink(":/GMShaders/boundingbox.vert", "", ":/GMShaders/boundingbox.frag", "BB" );
		case ShaderName::VERTEX_SPRITES:
			return shaderLink(":/GMShaders/vertex_sprite.vert", "", ":/GMShaders/vertex_sprite.frag", "VS");
		case ShaderName::VERTEX_NORMALS:
			return shaderLink(":/GMShaders/vertex_sprite.vert", ":/GMShaders/vertex_sprite.geom", ":/GMShaders/funcval.frag", "VN" );
		case ShaderName::FUNC_VAL_COLOR:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/funcval.geom", ":/GMShaders/funcval/vertexColor.frag", "FVCOLOR" );
		case ShaderName::FUNC_VAL_FUNC_VAL:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/funcval.geom", ":/GMShaders/funcval/funcVal.frag", "FVFUNCVAL" );
		case ShaderName::FUNC_VAL_LABEL:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/funcval.geom", ":/GMShaders/funcval/label.frag", "FFLABEL" );
		case ShaderName::WIREFRAME:
			return shaderLink(":/GMShaders/wireframe/wireframe.vert", ":/GMShaders/wireframe/wireframe.geom", ":/GMShaders/wireframe/wireframe.frag", "WF" );
		case ShaderName::POLYLINES:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/polyline.geom", ":/GMShaders/funcval.frag", "PL" );

		case ShaderName::NPR_BUILD_FBO:
			return shaderLink(":/GMShaders/NPR/NPR_buildFBO.vert", "", ":/GMShaders/NPR/NPR_buildFBO.frag", "BFBO");
		case ShaderName::NPR_BUILD_SOBEL:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/NPR/NPR_ApplySobel.frag", "SOBELFBO");
		case ShaderName::NPR_GAUSSIAN_BLUR:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/NPR/NPR_gaussianblur.frag", "NPR_GAUSSIAN");
		case ShaderName::NPR_HATCHING:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/NPR/NPR_hatches.frag", "NPR_HATCHES");
		case ShaderName::NPR_TOONIFY:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/NPR/NPR_toonify.frag", "NPR_TOONIFY");
		case ShaderName::NPR_COMPOSIT:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/NPR/NPR_composit.frag", "NPR_COMPOSIT");

		case ShaderName::TRANSPARENCY_AB_CLEAR:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/transparency/ABuffer/transparency_clearABuffers.frag", "TCLAB");
		case ShaderName::TRANSPARENCY_AB_FILL:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/funcval.geom", ":/GMShaders/transparency/ABuffer/transparency_createABuffers.frag", "TFILLAB");
		case ShaderName::TRANSPARENCY_AB_RENDER:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/transparency/ABuffer/transparency_renderABuffers.frag", "TRENDAB");
		case ShaderName::TRANSPARENCY_AB_FRAGCOUNT:
			return shaderLink(":/GMShaders/transparency/ABuffer/transparency_minGeom.vert", "", ":/GMShaders/transparency/ABuffer/transparency_minGeom.frag", "TMINGEOMAB");

		case ShaderName::TRANSPARENCY_AL_CLEAR:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/transparency/atomicLoop/transparency_clearALBuffers.frag", "TCLAL");
		case ShaderName::TRANSPARENCY_AL_FILL:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/funcval.geom", ":/GMShaders/transparency/atomicLoop/transparency_createALBuffers.frag", "TFILLAL");
		case ShaderName::TRANSPARENCY_AL_RENDER:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/transparency/atomicLoop/transparency_renderALBuffers.frag", "TRENDAL");
		case ShaderName::TRANSPARENCY_AL_DEPTH_COLLECT:
			return shaderLink(":/GMShaders/transparency/ABuffer/transparency_minGeom.vert", "", ":/GMShaders/transparency/atomicLoop/transparency_ALcollectDepth.frag", "TDCAL");

		case ShaderName::TRANSPARENCY_WOIT_GEOM:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/funcval.geom", ":/GMShaders/transparency/WeightedOIT/transparency_geomWOIT.frag", "TGEOMWOIT");
		case ShaderName::TRANSPARENCY_WOIT_BLEND:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/transparency/WeightedOIT/transparency_blendWOIT.frag", "TBLENDWOIT");

		case ShaderName::LIGHTING_LIGHT_TO_FBO:
			return shaderLink(":/GMShaders/lightingOverlay/overlayLighting_geometrypass.vert", "", ":/GMShaders/lightingOverlay/overlayLighting_geometrypass.frag", "OVBFBO");
		case ShaderName::LIGHTING_PAINT_OVERLAY:
			return shaderLink(":/GMShaders/fullscreenQuad_passthrough.vert", "", ":/GMShaders/lightingOverlay/overlayLighting_fbopass.frag", "OVDFBO");

		case ShaderName::DEPTH:
			return shaderLink(":/GMShaders/depth.vert", "", ":/GMShaders/depth.frag", "DEPTH");
		case ShaderName::FRONTAL_LIGHT_PER_VERTEX:
			return shaderLink(":/GMShaders/frontal_light_per_vertex.vert", "", ":/GMShaders/frontal_light_per_vertex.frag", "FRONTAL_PER_VERTEX");

		case ShaderName::POINT_CLOUD:
			return shaderLink(":/GMShaders/funcval.vert", ":/GMShaders/pointcloud/pointcloud.geom", ":/GMShaders/pointcloud/pointcloud.frag", "POINTCLOUD");
		case ShaderName::SHADER_COUNT:
			[[fallthrough]];
		default:
			return nullptr;
	}

	return nullptr;
}

//! Adds shaders from source and links them.
//! In case of an error a message is shown and
//! @returns nullptr in case of an error. a new shaderprogram otherwise.
QOpenGLShaderProgram* shaderLink(const QString& vertSrc,
                          const QString& geomSrc,
                          const QString& fragSrc,
                          const QString& name)
{
	auto shaderProgram = new QOpenGLShaderProgram;

	auto printErrMsg = [shaderProgram] (const QString& src) {
		auto msg = shaderProgram->log();
		msg = msg.left(msg.indexOf( "***" ) );
		LOG::error() << "[ShaderManager::" << __FUNCTION__ << "] error in shader: " << src.toStdString() << " : " << msg.toStdString() << "\n";
		SHOW_MSGBOX_CRIT( QString( "GLSL Error (" + src) , msg );
	};

	if(!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vertSrc))
	{
		printErrMsg(vertSrc);
		delete shaderProgram;
		return nullptr;
	}

	if(!geomSrc.isEmpty())
	{
		if(!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Geometry, geomSrc))
		{
			printErrMsg(geomSrc);
			delete shaderProgram;
			return nullptr;
		}
	}

	if(!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fragSrc))
	{
		printErrMsg(fragSrc);
		delete shaderProgram;
		return nullptr;
	}

	if(!shaderProgram->link() )
	{
		QString msg = shaderProgram->log();
		msg = msg.left(msg.indexOf( "***" ) );

		LOG::error() << "[ShaderManager::" << __FUNCTION__ << "] error linking: " << name.toStdString() << " : " << msg.toStdString() << "\n";
		SHOW_MSGBOX_CRIT( QString( "GLSL Error linking Shader: " + name ), msg );

		delete shaderProgram;
		return nullptr;
	}

	LOG::debug() << "Linked shader: " << name.toStdString() << "\n";
	return shaderProgram;
}
