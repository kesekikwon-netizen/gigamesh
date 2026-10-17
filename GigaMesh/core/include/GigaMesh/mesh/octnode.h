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

#ifndef OCTNODE_H
#define OCTNODE_H

#include "vertex.h"
#include "vector3d.h"
#include "line.h"
#include "cube.h"

// Sets default values
// ----------------------------------------------------
#define OCTNODEDEFAULTS   \
	misleaf( false ), \
	mchildren()



class Octnode {

public:
	// Constructor for the root node.
    Octnode( Vector3D* center, double scale );

	// Constructor for a child node.
    Octnode( Octnode* parent, int nr );

    //copy constructor
    Octnode( Octnode &copy);

    std::vector<Octnode*> getLeafNodes();
    std::vector<Octnode*> getNodeList();
    bool isFaceInside(Face *face);
    ///is the octnode neighbor of the @param[node]?
    bool isNeighbor(Octnode* node);
	// Destructor
//	~Octnode();

	//is this node a leaf node?
	bool misleaf;
	/// pointers to child nodes
	Octnode* mchildren[8];
	/// pointer to parent node
	Octnode* mparent;
	/// the tree-depth of this node
	unsigned int mlevel;
	 /// the centerpoint of this node
	Cube mCube;
    ///position at the parent --> look at figure in cube.cpp
    int mPosNr = -1;

	/// vertices inside this node
    std::vector<Vertex*> mVertices;

    /// faces inside the node
    std::set<Face*> mFaces;

    static bool compareoctnode( Octnode* rNode1, Octnode* rNode2 );
};


#endif // OCTNODE_H
