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

#ifndef LINE_H
#define LINE_H

#include "vertex.h"

class Vertex;
class Plane;

class Line : public Primitive {

public:

	Line() = default;
	Line( const Vector3D& rPosA, const Vector3D& rPosB );
	Line( Vertex& rPosA, Vertex& rPosB );


	bool operator==(Line& l) const;


	// Information retrieval:

	void getA(Vector3D& p) const;
	void getB(Vector3D& p) const;
	Vector3D getA() const;
	Vector3D getB() const;
	void getA(Vertex& p) const;
	void getB(Vertex& p) const;
	void getData(Vector3D& a, Vector3D& b) const;
	double getLength() const;
	bool getDistanceToPoint(Vector3D &p, double &dist) const;
	bool getDistanceToPoint(Vector3D &p, double &dist, Line& shortest) const;


	// Operations:

	bool setA(Vector3D& p);
	bool setB(Vector3D& p);
	bool setA(Vertex& p);
	bool setB(Vertex& p);
	void setData(Vector3D& a, Vector3D& b);

	bool getLineIntersection(Line& other, Line& result) const;

	int getLinePlaneIntersection(Face& F, Vector3D& res);
	int getLinePlaneIntersection(Plane& P, Vector3D& res);

	void projectonPlane(Plane& tar, Line& res);


	// get t from equation p = a + t *(b-a)
	bool getParameter(Vector3D& p, double& t);


//	bool projectonPlane(Plane& tar, Line& res);

private:
	// Vertices
	Vertex* mVertA;            //!< Vertex A
	Vertex* mVertB;            //!< Vertex B

	Vector3D mVecA;            //!< Vector3D A
	Vector3D mVecB;            //!< Vector3D B

};

#endif // LINE_H
