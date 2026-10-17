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

#include "triangulation.h"

#ifdef LIBPSALM
#include<libpsalm/libpsalm.h>
#endif

#include <numeric>

std::vector<size_t> triangulateQuad(const std::vector<Vector3D>& vertices)
{
	if( (vertices[0] - vertices[2]).getLength3Squared() < (vertices[1] - vertices[3]).getLength3Squared() )
	{
		return {0,1,2,0,2,3};
	}

	return {0,1,3,1,2,3};
}
//! triangulates a ring of vertices
//! @param vertices input vertices (non closed polyline)
//! @return triangulated indices of the vertices
std::vector<size_t> GigaMesh::Util::triangulateNgon(const std::vector<Vector3D>& vertices)
{
	if(vertices.empty())
	{
		return {};
	}
	if(vertices.size() == 1)
	{
		return {0};
	}
	if(vertices.size() == 2)
	{
		return {0,1};
	}
	if(vertices.size() == 3)
	{
		return {0,1,2};
	}

	if(vertices.size() == 4)
	{
		return triangulateQuad(vertices);
	}

#ifdef LIBPSALM
	std::vector<double> coordinates;
	coordinates.reserve(vertices.size() * 3);

	for(const auto& vertex : vertices)
	{
		coordinates.push_back(vertex.getX());
		coordinates.push_back(vertex.getY());
		coordinates.push_back(vertex.getZ());
	}

	size_t numNewVertices = 0;
	int numNewFaces = 0;
	double* newCoordinates = nullptr;
	long* newCoordinatesIndices = nullptr;
	fill_hole(vertices.size(), nullptr, coordinates.data(), nullptr, nullptr,&numNewVertices, &newCoordinates,&numNewFaces,&newCoordinatesIndices, true);

	std::vector<size_t> retIndices(numNewFaces * 3);

	for(size_t i = 0; i< numNewFaces * 3; ++i)
	{
		retIndices[i] = static_cast<size_t>(std::abs(newCoordinatesIndices[i]));
	}

	return retIndices;
#else
	return std::vector<size_t>();
#endif
}
