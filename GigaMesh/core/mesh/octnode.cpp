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

#include <GigaMesh/mesh/octnode.h>
#include <algorithm>

// Constructor for the root node.
Octnode::Octnode( Vector3D* center, double scale ) : \
        OCTNODEDEFAULTS,                    \
        mparent( nullptr ),                    \
        mlevel( 0 ),                        \
        mCube( *center, scale ) {
    // do nothing
}

// Constructor for a child node.
Octnode::Octnode( Octnode* parent, int nr ) : \
       OCTNODEDEFAULTS,              \
       mparent( parent ),            \
       mlevel( parent->mlevel+1 ),   \
       mPosNr(nr), \
       mCube( Cube(mparent->mCube.mcenter, 0.5*mparent->mCube.mscale).getVertex(nr), 0.5*mparent->mCube.mscale ) {
    // do nothing
}

Octnode::Octnode(Octnode &copy) :\
    OCTNODEDEFAULTS,              \
    mlevel(copy.mlevel),       \
    mCube(copy.mCube),            \
    mVertices(copy.mVertices)
{
   // do nothing
}



//!get all the leaf nodes of the octnode
//! traverse all children recursive until the leafs were found
std::vector<Octnode*> Octnode::getLeafNodes(){
    std::vector<Octnode*> leafNodes;
    //check all children of the node
    for( unsigned int i=0; i < 8; i++){
        if(mchildren[i]->misleaf == true){
            //nodes without vertices or faces are not interesting
            if(mchildren[i]->mVertices.size() > 0 || mchildren[i]->mFaces.size() > 0){
                leafNodes.push_back(mchildren[i]);
            }
        }
        else{
            //search in the children
            std::vector<Octnode*> tmpNodes;
            tmpNodes = mchildren[i]->getLeafNodes();
            leafNodes.insert(leafNodes.end(), tmpNodes.begin(), tmpNodes.end());
        }
    }

    return leafNodes;
}

//! traverse all children recursive until all nodes were added
std::vector<Octnode*> Octnode::getNodeList(){
    std::vector<Octnode*> nodes;
    //check all children of the node
    for( unsigned int i=0; i < 8; i++){
        if(mchildren[i]->misleaf == false){
            //search in the child
            std::vector<Octnode*> tmpNodes;
            tmpNodes = mchildren[i]->getNodeList();
            nodes.insert(nodes.end(), tmpNodes.begin(), tmpNodes.end());
        }
        nodes.push_back(mchildren[i]);
    }

    return nodes;
}

bool Octnode::isFaceInside(Face *face){
    if(mFaces.find(face) != mFaces.end()) {
        return true;
    } else {
        return false;
    }
}

bool Octnode::isNeighbor(Octnode *node)
{
    //neighbors must have the same parent
    if (node->mparent != mparent){
        return false;
    }
    switch (mPosNr)
    {
    case 0:
        if (node->mPosNr == 1 || node->mPosNr == 2 || node->mPosNr == 4 ){
            return true;
        }
        return false;
    case 1:
        if (node->mPosNr == 0 || node->mPosNr == 3 || node->mPosNr == 6 ){
            return true;
        }
        return false;
    case 2:
        if (node->mPosNr == 0 || node->mPosNr == 3 || node->mPosNr == 5 ){
            return true;
        }
        return false;
    case 3:
        if (node->mPosNr == 1 || node->mPosNr == 7 || node->mPosNr == 2 ){
            return true;
        }
        return false;
    case 4:
        if (node->mPosNr == 0 || node->mPosNr == 6 || node->mPosNr == 5 ){
            return true;
        }
        return false;
    case 5:
        if (node->mPosNr == 4 || node->mPosNr == 7 || node->mPosNr == 2 ){
            return true;
        }
        return false;
    case 6:
        if (node->mPosNr == 4 || node->mPosNr == 7 || node->mPosNr == 1 ){
            return true;
        }
        return false;
    case 7:
        if (node->mPosNr == 3 || node->mPosNr == 5 || node->mPosNr == 6 ){
            return true;
        }
        return false;

    default:
        return false;
    }
    return false;
}

/// comparison function to sort nodes by level
bool Octnode::compareoctnode( Octnode* rNode1, Octnode* rNode2 ) {
    return ( rNode1->mlevel < rNode2->mlevel );
}
