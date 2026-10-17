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

#include <GigaMesh/icoSphereTree/IcoSphereTree.h>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <limits>
#include <cassert>
#include <unordered_set>

//based on
//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

//possible optimisations here:
//https://github.com/erich666/jgt-code/blob/master/Volume_02/Number_1/Moller1997a/raytri.c

//calculates intersection between ray starting at origin and rayDir
//and triangle described by the vertices t0,t1,t2 . Returns true if the ray intersects the triangle and returns the distance in t
//note, t can be negative. This means we have a line-intersection, not a ray-intersection
bool intersectTriangle(const Vector3D& rayOrigin, const Vector3D& rayDir,
					   const Vector3D& t0, const Vector3D& t1, const Vector3D& t2,
					   double& t)
{
	const double eps = 0.000001;
	/* find vectors for two edges sharing vert0 */
	const Vector3D edge1 = t1 - t0;
	const Vector3D edge2 = t2 - t0;

	/* begin calculating determinant - also used to calculate U parameter */
	const Vector3D pvec = rayDir % edge2;

	/* if determinant is near zero, ray lies in plane of triangle */
	const double det = dot3(edge1, pvec);

	if(det > -eps && det < eps)
		return false;

	const double inv_det = 1.0 / det;

	/* calculate distance from vert0 to ray origin */
	const Vector3D tvec = rayOrigin - t0;

	/* calculate U parameter and test bounds */
	const double u = dot3(tvec, pvec) * inv_det;
	if(u < 0.0 || u > 1.0)
		return false;

	/* prepare to test V parameter */
	const Vector3D qvec = tvec % edge1;

	/* calculate V parameter and test bounds */
	const double v = dot3(rayDir, qvec) * inv_det;
	if( v < 0.0 || u + v > 1.0)
		return false;

	/* calculate t, ray intersects triangle */
	t = dot3(edge2, qvec) * inv_det;

	return true;
}

IcoSphereTree::IcoSphereTree(unsigned int subdivision)
{
	mVertices.resize(12);
	//initialise root icosahedron, t == golden ratio
	const double t = (1.0 + sqrt(5.0)) / 2.0;

	// create 12 vertices of a icosahedron
	mVertices[ 0] = normalize3(Vector3D(-1.0,  t, 0.0));
	mVertices[ 1] = normalize3(Vector3D( 1.0,  t, 0.0));
	mVertices[ 2] = normalize3(Vector3D(-1.0, -t, 0.0));
	mVertices[ 3] = normalize3(Vector3D( 1.0, -t, 0.0));

	mVertices[ 4] = normalize3(Vector3D( 0.0, -1.0,  t));
	mVertices[ 5] = normalize3(Vector3D( 0.0,  1.0,  t));
	mVertices[ 6] = normalize3(Vector3D( 0.0, -1.0, -t));
	mVertices[ 7] = normalize3(Vector3D( 0.0,  1.0, -t));

	mVertices[ 8] = normalize3(Vector3D( t, 0.0, -1.0));
	mVertices[ 9] = normalize3(Vector3D( t, 0.0,  1.0));
	mVertices[10] = normalize3(Vector3D(-t, 0.0, -1.0));
	mVertices[11] = normalize3(Vector3D(-t, 0.0,  1.0));

	// 5 faces around point 0
	mRootFaces[0].setVertIndices(0, 11,  5);
	mRootFaces[1].setVertIndices(0,  5,  1);
	mRootFaces[2].setVertIndices(0,  1,  7);
	mRootFaces[3].setVertIndices(0,  7, 10);
	mRootFaces[4].setVertIndices(0, 10, 11);

	// 5 adjacent faces
	mRootFaces[5].setVertIndices( 1,  5, 9);
	mRootFaces[6].setVertIndices( 5, 11, 4);
	mRootFaces[7].setVertIndices(11, 10, 2);
	mRootFaces[8].setVertIndices(10,  7, 6);
	mRootFaces[9].setVertIndices( 7,  1, 8);

	// 5 faces around point 3
	mRootFaces[10].setVertIndices(3, 9, 4);
	mRootFaces[11].setVertIndices(3, 4, 2);
	mRootFaces[12].setVertIndices(3, 2, 6);
	mRootFaces[13].setVertIndices(3, 6, 8);
	mRootFaces[14].setVertIndices(3, 8, 9);

	// 5 adjacent faces
	mRootFaces[15].setVertIndices(4, 9,  5);
	mRootFaces[16].setVertIndices(2, 4, 11);
	mRootFaces[17].setVertIndices(6, 2, 10);
	mRootFaces[18].setVertIndices(8, 6,  7);
	mRootFaces[19].setVertIndices(9, 8,  1);

	subdivide(subdivision);
}

void IcoSphereTreeFaceNode::setVertIndices(size_t v0, size_t v1, size_t v2)
{
	vertexIndices[0] = v0;
	vertexIndices[1] = v1;
	vertexIndices[2] = v2;
}

size_t getMidPoint(size_t p1, size_t p2, std::vector<Vector3D>& vertices, std::unordered_map<uint64_t, size_t>& cache)
{
	const uint64_t minIndex = std::min(p1, p2);
	const uint64_t maxIndex = std::max(p1, p2);
	const uint64_t key = (minIndex << 32) + maxIndex;

	const auto item = cache.find(key);
	if(item != cache.end())
	{
		return item->second;
	}

	Vector3D middle = (vertices[p1] + vertices[p2]) * 0.5F;
	const unsigned int retVal = vertices.size();

	vertices.emplace_back(normalize3(middle));
	cache.emplace(std::make_pair(key, retVal));

	return retVal;
}

void subdivideFaces(std::unordered_map<uint64_t, size_t>& newNodeCache, std::queue<IcoSphereTreeFaceNode*>& faces, std::vector<Vector3D>& vertices)
{
	while(!faces.empty())
	{
		auto face = faces.front();
		faces.pop();

		//check if current face is a leaf-node
		//if not, push children to the queue and skip further processing
		if(!face->childNodes.empty())
		{
			for(auto& child : face->childNodes)
			{
				faces.push(child.get());
			}
			continue;
		}

		face->childNodes.reserve(4);

		unsigned int a = getMidPoint(face->vertexIndices[0], face->vertexIndices[1], vertices, newNodeCache);
		unsigned int b = getMidPoint(face->vertexIndices[1], face->vertexIndices[2], vertices, newNodeCache);
		unsigned int c = getMidPoint(face->vertexIndices[2], face->vertexIndices[0], vertices, newNodeCache);

		auto newFace = std::make_unique<IcoSphereTreeFaceNode>();
		newFace->setVertIndices(face->vertexIndices[0], a, c);
		face->childNodes.emplace_back(std::move(newFace));

		newFace = std::make_unique<IcoSphereTreeFaceNode>();
		newFace->setVertIndices(face->vertexIndices[1], b, a);
		face->childNodes.emplace_back(std::move(newFace));

		newFace = std::make_unique<IcoSphereTreeFaceNode>();
		newFace->setVertIndices(face->vertexIndices[2], c, b);
		face->childNodes.emplace_back(std::move(newFace));

		newFace = std::make_unique<IcoSphereTreeFaceNode>();
		newFace->setVertIndices(a, b, c);
		face->childNodes.emplace_back(std::move(newFace));
	}
}
//smoothes the sphere, resets the data
void IcoSphereTree::subdivide(unsigned int subdivisions)
{
	std::unordered_map<uint64_t, size_t> newVertexCache;
	for(unsigned int i = 0; i< subdivisions; ++i)
	{
		std::queue<IcoSphereTreeFaceNode*> faces;
		for(auto& face : mRootFaces)
		{
			faces.push(&face);
		}

		subdivideFaces(newVertexCache, faces, mVertices);
	}

	mVertexData = std::vector<double>(mVertices.size(), 0);

}

std::vector<unsigned int> IcoSphereTree::getFaceIndices(int subdivisionLevel) const
{
	int level = 0;

	std::queue<const IcoSphereTreeFaceNode*> currLevelFaces;

	for(auto& face : mRootFaces)
	{
		currLevelFaces.push(&face);
	}

	while((level < subdivisionLevel || subdivisionLevel < 0) && !currLevelFaces.front()->childNodes.empty())
	{
		std::queue<const IcoSphereTreeFaceNode*> nextLevelFaces;

		while(!currLevelFaces.empty())
		{
			auto face = currLevelFaces.front();
			currLevelFaces.pop();

			for(auto& child : face->childNodes)
			{
				nextLevelFaces.push(child.get());
			}
		}

		std::swap(currLevelFaces, nextLevelFaces);
		++level;
	}


	std::vector<unsigned int> retVec;
	retVec.reserve(currLevelFaces.size() * 3);

	while(!currLevelFaces.empty())
	{
		auto face = currLevelFaces.front();
		currLevelFaces.pop();

		retVec.push_back(face->vertexIndices[0]);
		retVec.push_back(face->vertexIndices[1]);
		retVec.push_back(face->vertexIndices[2]);
	}

	return retVec;
}

std::vector<float> IcoSphereTree::getVertices() const
{
	std::vector<float> retVec(mVertices.size() * 3);

	auto it = mVertices.begin();
	for(size_t i = 0; i< retVec.size(); i += 3, ++it)
	{
		retVec[i    ] = it->getX();
		retVec[i + 1] = it->getY();
		retVec[i + 2] = it->getZ();
	}

	return retVec;
}

double pointRayDistanceSquared(const Vector3D& origin, const Vector3D& direction, const Vector3D& p)
{
	return ((p - origin) % direction).getLength3Squared();
}

size_t getClosestFaceVertexIndexToRay(const Vector3D& rayOrigin, const Vector3D& rayDirection, const IcoSphereTreeFaceNode* face, const std::vector<Vector3D>& vertices)
{
	if(face == nullptr)
		return 0;

	//find closest vertex of the face to the intersection-point
	int retIndex = face->vertexIndices[0];
	double minDistance = pointRayDistanceSquared(rayOrigin, rayDirection, vertices[face->vertexIndices[0]]);

	double distance = pointRayDistanceSquared(rayOrigin, rayDirection, vertices[face->vertexIndices[1]]);

	if(distance < minDistance)
	{
		minDistance = distance;
		retIndex = face->vertexIndices[1];
	}

	distance = pointRayDistanceSquared(rayOrigin, rayDirection, vertices[face->vertexIndices[2]]);

	if(distance < minDistance)
	{
		retIndex = face->vertexIndices[2];
	}
	return retIndex;
}

inline Vector3D getFaceCenter(const IcoSphereTreeFaceNode* face, const std::vector<Vector3D>& vertices)
{
	return (vertices[face->vertexIndices[0]] + vertices[face->vertexIndices[1]] + vertices[face->vertexIndices[2]]) / 3.0;
}

size_t getVertexIndexClosestToRay(const IcoSphereTreeFaceNode* faceToRefine, const Vector3D& rayOrigin, const Vector3D& rayDirection, const std::vector<Vector3D>& vertices)
{
	const IcoSphereTreeFaceNode* vertexFace = faceToRefine;
	double t;


	//descent until we have the triangle on the lowest level
	while(vertexFace != nullptr && !vertexFace->childNodes.empty())
	{
		bool found = false;
		//select triangle that intersects
		for(const auto& child : vertexFace->childNodes)
		{
			if(intersectTriangle(rayOrigin, rayDirection, vertices[child->vertexIndices[0]], vertices[child->vertexIndices[1]], vertices[child->vertexIndices[2]], t))
			{
				vertexFace = child.get();
				found = true;
				break;
			}
		}
		//no face collision detected. this may happen, if the ray is on the edge of two faces. in this case, we have to search through the vertices manually by distance...
		if(!found)
		{
			double minDist = std::numeric_limits<double>::max();
			IcoSphereTreeFaceNode* candidate = nullptr;

			//check which child shares the closes vertex
			for(const auto& child : vertexFace->childNodes)
			{
				Vector3D center = getFaceCenter(child.get(), vertices);

				double distance = pointRayDistanceSquared(rayOrigin, rayDirection, center);
				if(distance < minDist)
				{
					minDist = distance;
					candidate = child.get();
				}
			}
			vertexFace = candidate;
		}
	}

	return getClosestFaceVertexIndexToRay(rayOrigin, rayDirection, vertexFace, vertices);
}

size_t IcoSphereTree::getNearestVertexIndexAt(const Vector3D& position) const
{
	size_t index = 0;

	//no check if it hits, because the ray is directed into the ico-spheres origin
	getNearestVertexFromRay(position, normalize3(-position) , index);
	return index;
}

bool IcoSphereTree::getNearestVertexFromRay(const Vector3D& rayOrigin, const Vector3D& rayDirection, size_t& index, bool rayIsLine) const
{
	//check if ray hits the sphere:
	//origin of sphere is 0,0,0 , so direction from rayOrigin to sphereOrigin is -rayOrigin

	double distance2 = (-rayOrigin % rayDirection).getLength3Squared();

	//sphere has unit-radius
	if(distance2 > 1.0)
		return false;

	//get closest point from ray. then test intersection from that point emitting a ray to the origin
	//we do this, because the ray might intersect the sphere, but miss the lowest resolution of the icosphere
	Vector3D newOrigin = dot3(rayOrigin, rayDirection) * rayDirection; //(dot(a-p, n) * n) ; p == 0,0,0
	Vector3D newDir = normalize3(-newOrigin);
	newOrigin -= newDir; //we push the origin out of the sphere. Otherwise it might be inside...

	//check root faces and recusivly refine the closest triangle intersection

	double minDist = std::numeric_limits<double>::max();
	const IcoSphereTreeFaceNode* refineFace = nullptr;
	for(const auto& faceCandidate : mRootFaces)
	{
		double distance = 0.0;
		if(intersectTriangle(newOrigin, newDir, mVertices[faceCandidate.vertexIndices[0]], mVertices[faceCandidate.vertexIndices[1]], mVertices[faceCandidate.vertexIndices[2]],distance))
		{
			if(rayIsLine && distance < 0.0)
				distance = -distance;

			if(distance < minDist)
			{
				refineFace = &faceCandidate;
				minDist = std::abs(distance);
			}
		}
	}

	if(refineFace == nullptr)
		return false;

	index = getVertexIndexClosestToRay(refineFace, rayOrigin, rayDirection, mVertices);
	return true;
}

void IcoSphereTree::selectVertex(size_t index)
{
	mSelectedVertices.insert(index);
}

void IcoSphereTree::deselectVertex(size_t index)
{
	auto itemIt = mSelectedVertices.find(index);

	if(itemIt != mSelectedVertices.end())
		mSelectedVertices.erase(itemIt);
}

void IcoSphereTree::clearSelection()
{
	mSelectedVertices.clear();
}

bool IcoSphereTree::isSelected(size_t index) const
{
	return mSelectedVertices.find(index) != mSelectedVertices.end();
}

void IcoSphereTree::incData(size_t index, double value)
{
	if(index > mVertexData.size())
		return;

	mVertexData[index] += value;
	mMaxData = std::max(mMaxData, mVertexData[index]);	//increment mVertexData, update maxData
}

double IcoSphereTree::getMaxData() const
{
	return mMaxData;
}
