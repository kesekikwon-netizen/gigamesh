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

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include <stdlib.h>
#include <float.h>

#include "vertexofface.h"
#include "face.h"

using namespace std;

//! Main routine for testing surface area integral invariant
//==========================================================================
int main(void) {

	Vector3D aa(0.0, 0.0, 0.0);
	Vector3D bb(1.0, 0.0, 0.0);
	Vector3D cc(0.0, 1.0, 0.0);
	VertexOfFace a(aa);
	VertexOfFace b(bb);
	VertexOfFace c(cc);

	Face face1(1000000000, &a, &b, &c );
	Vector3D mm(0.0, 0.0, 0.0);
	VertexOfFace m(mm);
	double r=1.0;
	double area=0.0;
	//a.dumpInfo();
	//face1.dumpFaceInfo();
	face1.surfaceintegralinvariant(1, &r, &area, &m);


	cout<<"Area of intersection: "<<area<<endl;

	return 0;
}
