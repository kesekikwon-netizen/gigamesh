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

#ifndef OCTREE_H
#define OCTREE_H

#include <utility>
#include <thread>
#include <array>
#include <chrono>
#include <mutex>
#include "vector3d.h"
#include "octnode.h"
#include "rectbox.h"
#include "face.h"
#include "showprogress.h"


class Octree {

public:
    //! @param vertexlist  @param  center @param maxnr represents the maximum number of vertices per cube
    //! @param edgelen contains the length of edge of the largest cube
    //! Octree is primarily a octree of vertices. A Faceoctree can be created on the base of the vertex octree.
    Octree(std::vector<Vertex*> &vertexlist,std::vector<Face*> &facelist, Vector3D* center, unsigned int maxnr, double edgelen, double EdgeLenMax, bool copyelements=true);

	//! destruct
    ~Octree();

    void getnodesinlevel(std::vector<Octnode*>& nodelist, unsigned int i);

    enum eTreeType {
        VERTEX_OCTREE, //!< Octree of Vertices (Vertices inside the Leafnodes)
        FACE_OCTREE //!< Octree of Faces (Faces inside the Leafnodes)
    };

    void getleafnodes(std::vector<Octnode*>& nodelist, eTreeType treeType);
    void getnodelist(std::vector<Octnode*>& nodelist,  eTreeType treeType);

    //! get all nodes of the faces octree with the face inside
    //! return false if the face is not in any node
    bool getNodesOfFace(std::vector<Octnode*>& nodelist, Face* face);
    //! @param[allNodes] list of nodes where the face will be searched in --> no tree traverse needed
    //! @param[nodelist] list of nodes where the face is inside
    bool getNodesOfFace(std::vector<Octnode*>& nodelist, Face* face,std::vector<Octnode*>allNodes);


	bool twovert(std::vector<Face*>::iterator& kt, std::vector<Face*>::iterator& jt, std::vector<Face*>& sif,
                 Line& l1, Line& l2, Line& l3);


	//! @param[out] sif contains pointers to self intersectiing faces
    void detectselfintersections(std::vector<Face*>& sif);

    //!get triangle intersection of the Triangular prism tri
    void gettriangleintersection(std::vector<Octnode*>& nodelist, std::vector<Octnode*>& cnodelist,
                                     std::vector<Line> &drawlines, TriangularPrism& tri);
	//! prints number of levels and number of nodes in each level and total number of nodes
    bool dumpInfo();
	//! prints all centers of octnodes
    bool dumpInfov();
	//! prints all centers of octnodes and corresponding vertices
    bool dumpInfovlong();


private:
    ///all faces with more than one node
    struct StrucIncompleteFace{
        Face* face; // face not completly in aligned node
        Octnode* node; // aligned node
        bool operator==(const StrucIncompleteFace& a) const
        {
            return (face == a.face);
        }
        bool operator<(const StrucIncompleteFace& a) const
        {
            return (face < a.face);
        }
    };
    void oointers(Vertex* object, Octnode* cnode);
    void oointers(Face* object, Octnode* cnode);

	//helper function for recursive construction (we do not want to call the constructor recursively)
    bool initialize(Octnode* cnode, unsigned int clevel);

    //! generate a faces octree with the base of the vertex octree
    //! each assigned face of a vertex inside a node becomes part of the same node in the face octree
    //! face octree has the same nodes like the vertex octree
    //! @param[incompleteFaces] are all faces which are not completely inside a node
    void generateFacesOctreeHypothesis(Octnode* parentNodeFace,Octnode* parentNodeVertex, unsigned int treeLevel, std::vector<StrucIncompleteFace> *incompletFaces);
    //! check if each face is only inside the initial assigned node/cube
    //! otherwise search all nodes/cube that are intersected by the face
    void correctFacesOctree(std::vector<StrucIncompleteFace> &facelist);
    void addFaceToAllIntersectedChildren(Octnode* parentNode, Face* face);
    //!method for parallel compution of correctFacesOctree
    void correctFace(StrucIncompleteFace IncompFaceStruct);
    //! delete all faces from mIncompleteFaces if they are neighbors and all vertices are inside this two cubes
    void checkNeighborhoodOfIncompleteFaces();

    //!contains the implementation of Tomas Moeller A Fast Triangle-Triangle Intersection Test
    //! obsolete and not tested
    bool areFacesIntersected(Face* faceA, Face* faceB);
    //!helper function of detectselfintersections for parallel threads
    //! check for one face if it has an intersection with all following faces
    //! the function only checks the following faces inside the vector because the predecessors are already checked
    //! @param[nodeFace] all faces of a octree node
    void checkIntersectionOfFaceToOtherFaces(std::vector<Face*> nodeFaces, unsigned int faceIterator);
    //! for each face inside the check if it intersects a other node
    void checkSelfIntersectionInsideNode(Octnode* node);
    /// pointer to the root node of the vertex octree
    Octnode* mRootVertices;
    /// pointer to the root node of the face octree
    Octnode* mRootFaces;
    ///all faces with more than one node
    std::vector<StrucIncompleteFace> mIncompleteFaces;
    /// mutex to lock critical memory access
    std::mutex mLock;

    ///all faces which are intersected by an other face
    std::vector<Face*> mSelfIntersectedFaces;

	/// maximum depth of octree
	unsigned int mmaxlevel;
	// mmaxnr represents the maximum number of vertices per cube
	unsigned int mmaxnr;

	//at this level no further subdivision is permitted
	static constexpr unsigned int mmaxlevelpermitted = 15 ;

	//! @param copyelements ==true => elements on the border of a cube are copied to all adjacent cubes
	//! if copyelements ==false => elements on the border of a cube are copied to a single cube
	//! should be true for correct collision detection etc.
	bool mcopyelements;

	double mEdgeLenMax;


};


#endif // OCTREE_H
