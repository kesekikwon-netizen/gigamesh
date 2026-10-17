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

#ifndef GLMACROS_H
#define GLMACROS_H

#define PRINT_OPENGL_ERROR( functionName ) { \
	GLenum errorNr = glGetError(); \
	if( errorNr != GL_NO_ERROR ) { \
		std::string errString; \
		switch( errorNr ) { \
			case GL_INVALID_ENUM: \
				errString = "invalid ENUM"; \
				break; \
			case GL_INVALID_VALUE: \
				errString = "invalid Value"; \
				break; \
			case GL_INVALID_OPERATION: \
				errString = "invalid Operation"; \
				break; \
			case GL_INVALID_FRAMEBUFFER_OPERATION: \
				errString = "invalid Framebuffer Operation"; \
				break; \
			case GL_OUT_OF_MEMORY: \
				errString = "out of Memory"; \
				break; \
			default: \
				errString = "UNKNOWN ERROR"; \
		} \
		std::cerr << __PRETTY_FUNCTION__ << " Line: " << __LINE__ << " OpenGL Error(" << errorNr << "): " << errString << " in " << (functionName) << "!" << std::endl; \
	} \
}
// DEPRECATED use of gluErrorString: cerr << __PRETTY_FUNCTION__ << " Line: " << __LINE__ << " OpenGL Error(" << errorNr << "): " << gluErrorString( errorNr ) << " in " << functionName << "!" << endl;

/*
	// DEPRECATED sometime in spring 2013 - to be removed in the future.
	// gluUnProject inspired by: http://nehe.gamedev.net/data/articles/article.asp?article=13
	#define OPENGL_UNPROJECT( someVector3D, viewPortX, viewPortY, viewPortZ ) { \
		GLint    viewport[4]; \
		GLdouble modelview[16]; \
		GLdouble projection[16]; \
		GLdouble posX, posY, posZ; \
		glGetDoublev( GL_MODELVIEW_MATRIX, modelview ); \
		glGetDoublev( GL_PROJECTION_MATRIX, projection ); \
		glGetIntegerv( GL_VIEWPORT, viewport ); \
		gluUnProject( viewPortX, viewPortY, viewPortZ, modelview, projection, viewport, &posX, &posY, &posZ ); \
		someVector3D.set( posX, posY, posZ, 1.0 ); \
	}
*/

#endif
