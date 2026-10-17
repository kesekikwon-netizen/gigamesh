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

#ifndef TEXTUREDMESH_H
#define TEXTUREDMESH_H

#include <vector>
#include <map>
#include <list>
#include <QOpenGLBuffer>

class Mesh;
class Face;
class Vertex;

class TexturedMesh
{
	public:
		TexturedMesh() = default;

		//The destructor needs to be called with a valid OpenGLContext. If this is not possible (e.g. allocated on stack), call destroy with a valid context
		~TexturedMesh();

		TexturedMesh(const TexturedMesh&  other) = delete;
		TexturedMesh(      TexturedMesh&& other) = delete;

		TexturedMesh& operator=(const TexturedMesh&  other) = delete;
		TexturedMesh& operator=(      TexturedMesh&& other) = delete;

		//Needs to be called with a valid OpenGLContext
		void establishStructure(Mesh* mesh);

		//Needs to be called with a valid OpenGLContext
		void destroy();

		[[nodiscard]] bool isCreated() const;

		std::map<unsigned char, std::list<QOpenGLBuffer>>& getVertexBuffers();

		static unsigned int getVertexElementSize();

	private:
		void generateBuffers(const std::map<unsigned char, std::list<Face*> >& faces);
		std::map<unsigned char, std::list<QOpenGLBuffer>> mVertexBuffers;
};

#endif // TEXTUREDMESH_H
