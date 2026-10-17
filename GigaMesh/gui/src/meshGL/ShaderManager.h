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

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <array>

class QOpenGLShaderProgram;

class ShaderManager
{
	public:
		ShaderManager() = default;
		~ShaderManager();

		enum ShaderName : std::size_t {
			BOUNDING_BOX,					//!< Shader program rendering the bounding box of the mesh.
			VERTEX_SPRITES,					//!< Shader program rendering only normals of vertices.
			VERTEX_NORMALS,					//!< Shader program rendering only normals of vertices.
			FUNC_VAL_COLOR,					//!< Shader program rendering a mesh with vertexColors.
			FUNC_VAL_FUNC_VAL,				//!< Shader program rendering a mesh with colors mapped to the (generic) function value.
			FUNC_VAL_LABEL,					//!< Shader program rendering a mesh with colors mapped Labels.
			WIREFRAME,						//!< Shader program rendering a mesh as wireframe.
			POLYLINES,						//!< Shader program rendering cones using two vertices with radii as input.

			NPR_BUILD_FBO,					//!< Shader program generating a FBO texture for NPR-sketch-shading
			NPR_BUILD_SOBEL,				//!< Shader program generating the sobel image
			NPR_GAUSSIAN_BLUR,				//!< Shader program to blur an image using a 3x3 gausian kernel
			NPR_HATCHING,					//!< Shader program to generate hatch-shading
			NPR_TOONIFY,					//!< Shader program to generate toon-like images
			NPR_COMPOSIT,					//!< Shader program tom composit multiple NPR textures into a final image

			TRANSPARENCY_AB_CLEAR,			//!< Shader program to clear Buffers for transparency (ABuffer variant)
			TRANSPARENCY_AB_FILL,			//!< Shader program to fill fragment buffers for transparency (ABuffer variant)
			TRANSPARENCY_AB_RENDER,			//!< Shader program to render fragment buffer for transparency (ABuffer variant)
			TRANSPARENCY_AB_FRAGCOUNT,		//!< Shader program to count the maximal number of fragments generated of an object

			TRANSPARENCY_AL_CLEAR,			//!< Shader program to clear Buffers for transparency (Atomic Loop variant)
			TRANSPARENCY_AL_FILL,			//!< Shader program to fill fragment buffers for transparency (Atomic Loop variant)
			TRANSPARENCY_AL_RENDER,			//!< Shader program to render fragment buffer for transparency (Atomic Loop variant)
			TRANSPARENCY_AL_DEPTH_COLLECT,	//!< Shader program to collect depth-values (Atomic Loop)

			TRANSPARENCY_WOIT_GEOM,			//!< Geometry pass of Weighted OIT
			TRANSPARENCY_WOIT_BLEND,		//!< Blending pass of Weighted OIT

			LIGHTING_LIGHT_TO_FBO,			//!< Geometry pass to create Geometry-Buffer with lighting information
			LIGHTING_PAINT_OVERLAY,			//!< Shader program to paint lightning information of over-/under-lit areas

			DEPTH,							//!< Shader program for just filling the depth buffer
			FRONTAL_LIGHT_PER_VERTEX,		//!< Shader program for painting per vertex light intensities (obtained by simulating frontal lighting) to individual pixels

			POINT_CLOUD,					//!< Shader program for painting point-clouds
			SHADER_COUNT					//!< Number of existing shaders
		};

		QOpenGLShaderProgram* getShader(ShaderName shaderName);

	    private:

		QOpenGLShaderProgram* initShader(ShaderName shaderName);

		std::array<QOpenGLShaderProgram*, static_cast<std::size_t>(ShaderName::SHADER_COUNT)> mShaderPrograms = {nullptr};
		std::array<bool, static_cast<std::size_t>(ShaderName::SHADER_COUNT)>                  mBrokenShader = {false};
};

#endif // SHADERMANAGER_H
