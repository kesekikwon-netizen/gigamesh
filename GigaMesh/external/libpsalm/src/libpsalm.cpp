/*!
*	@file	libpsalm.cpp
*	@brief	Exports a function for filling holes in meshes. Used by GigaMesh.
*	@author	Bastian Rieck <bastian.rieck@iwr.uni-heidelberg.de>
*/

#include <sstream>
#include <string>
#include <vector>

#include "libpsalm/libpsalm.h"
#include "mesh.h"

#include "SubdivisionAlgorithms/Liepa.h"
#include "TriangulationAlgorithms/MinimumWeightTriangulation.h"

/*!
*	Given a polygonal line described as a list of vertices, this function
*	triangulates the hole and subdivides it. Afterwards, the new data is
*	stored in some arrays given as parameters of the caller.
*
*	@param num_vertices	Number of vertices in polygonal line
*
*	@param vertex_IDs	List of vertex IDs, used to identify them. If
*				this parameter is nullptr, the mesh class will
*				assign sequential vertex IDs, starting with 0.
*
*	@param coordinates	Array of vertex coordinates (size: 3*num_vertices)
*
*	@param scale_attributes	Array of scale attributes (size: num_vertices).
*				The scale attribute is the average length of all
*				edges incident on the vertex. This parameter is
*				used for the subdivision algorithm. If the pointer
*				is nullptr, the scale attributes will not be initialized
*				but the algorithm will still work.
*
*	@param normals		Array of vertex normals (size: 3*num_vertices)
*
*	@param num_new_vertices	Number of new vertices created by the algorithm
*
*	@param new_coordinates	Array of vertex coordinates created by the
*				algorithm (size: 3*num_new_vertices)
*
*	@param num_new_faces	Number of new faces created by the algorithm
*
*	@param new_vertex_IDs	List of vertex IDs for the new faces (size:
*				3*num_new_faces). Negative IDs signify that the
*				vertex is _not_ new.
*
*	@param doLiepa	Sets, if subdivision sould be used
*	The nomenclature here might be confusing: new_vertex_IDs contains 3
*	vertex IDs that describe a new face created by the algorithm.
*
*	@returns true if the hole could be filled, otherwise false
*/

bool fill_hole(int num_vertices, long* vertex_IDs, double* coordinates, double* scale_attributes, double* normals,
        size_t* num_new_vertices, double** new_coordinates, int* num_new_faces, long** new_vertex_IDs, bool doLiepa)
{
	bool result = true;
	if(	num_vertices == 0		||
		coordinates == nullptr		||
		num_new_vertices == nullptr	||
		new_coordinates == nullptr		||
		num_new_faces == nullptr		||
		new_vertex_IDs == nullptr)
		return(false);

	psalm::mesh M;

	psalm::Liepa liepa_algorithm;
	psalm::MinimumWeightTriangulation triangulation_algorithm;

	result = M.load_raw_data(	num_vertices,
					vertex_IDs,
					coordinates,
					scale_attributes,
					normals);

	if(!result)
	{
		std::cerr << "libpsalm: Data processing failed for fill_hole() failed" << std::endl;
		return(false);
	}

	result = (result && triangulation_algorithm.apply_to(M));				// step 1: triangulate the hole
	if(doLiepa && result)
	{
		result = (result && liepa_algorithm.apply_to(M));					// step 2: apply Liepa's subdivision scheme with
												// default parameters
	}

	if(result)
	{
		M.save_raw_data(num_new_vertices,
				new_coordinates,
				num_new_faces,
				new_vertex_IDs);

		#ifdef DEBUG
			M.save(generate_filename());
		#endif
	}

	// signal an error for the calling function
	else
	{
		*num_new_vertices	= 0;
		*num_new_faces		= 0;
	}

	return(result);
}
