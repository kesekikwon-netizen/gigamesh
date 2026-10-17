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

#ifndef MESH_PARAMS_H
#define MESH_PARAMS_H

//!
//! \brief Parameter class for the allmighty mesh. (Layer 0)
//!
//! This class handles the parameters for the mesh class.
//! Therefore it contains: flags, ints, doubles and strings.
//!
//! Layer 0
//!

class MeshParams {
	public:
		// Constructor and deconstructor:
		MeshParams();
		~MeshParams() = default;

		// Parameters:
		enum eParamFlag {
			FILE_TRANSFORMATION_APPLIED,     //!< Has to be set, when a transformation was applied, which is a strong indicator for orientation by the user.
			GEODESIC_STORE_DIRECTION, //!< If true the direction (angle) of the geodesic paths is stored. Otherwise store the geodesic distance.
			GEODESIC_USE_FUNCVAL_AS_WEIGHT,  //!< If true use the function values as weigths (EXPERIMENTAL, not properly working).
			LABELING_USE_STEP_AS_FUNCVAL,    //!< If true set the iteration step of the labeling as function value.
			FILLPOLYLINES_COLOR_AVG,  //!< If true the average color of the polyline is ued as color for the new (synthetic) vertices.
			PARAMS_FLAG_COUNT         //!< Number of flags available
		};
		enum eParamInt {
			HISTOGRAM_SHOW_FEATURE_ELEMENT_VERTEX_DIM, //!< User selection for rendering the vertices' feature vector histogram.
			PARAMS_INT_COUNT                           //!< Number of integer parameters available
		};
		enum eParamFlt {
			PARAMS_FLT_UNDEFINED, //!< Parameter (double) with lowest ID - NOT to be used for anything other than the valid range of IDs. highest ID is determined by SHOW_FLAGS_COUNT
			FUNC_VALUE_THRES,            //!< Function value threshold - also used for ISOLines
			AXIS_PRIMEMERIDIAN,   //!< Offset in radiant to set the prime meridian for rollouts (cone, cylinder and sphere).
			CYLINDER_RADIUS,      //!< Radius of a cylinder for cylindric rollouts.
			SMOOTH_LENGTH,   //!< Absolut length defining the size of e.g. a gaussian filter e.g. for smoothing a polyline.
			PARAMS_FLT_COUNT //!< Number of floating point parameters available
		};
		enum eParamStr {
			PARAMS_STR_COUNT //!< Number of string parameters available
		};

		// Values for eParamInt:
		enum eHistogramType {
			HISTOGRAM_EDGE_LENGTH,                  //!< Length of the triangles' edges.
			HISTOGRAM_FEATURE_ELEMENTS_VERTEX,      //!< All feature elements' values of all feature vectors.
			HISTOGRAM_FEATURE_ELEMENTS_VERTEX_DIM,  //!< All feature elements' values of a certain dimension (nr) of all feature vectors.
			HISTOGRAM_FUNCTION_VALUES_VERTEX,       //!< Function values of the vertices.
			HISTOGRAM_FUNCTION_VALUES_VERTEX_LOCAL_MINIMA,   //!< Function values of the vertices being a local mininma.
			HISTOGRAM_FUNCTION_VALUES_VERTEX_LOCAL_MAXIMA,   //!< Function values of the vertices being a local maximum.
			HISTOGRAM_ANGLES_FACES_MINIMUM,         //!< Smallest angles of all faces.
			HISTOGRAM_ANGLES_FACES_MAXIMUM,         //!< Largest angles of all faces.
			HISTOGRAM_POLYLINE_RUNLENGHTS,          //!< Runlengths of the polyline
			HISTOGRAM_TYPE_COUNT                    //!< Number of available options for a mesh's histogram.
		}; //! \todo: histogram for auto-correlation and polyline length

		// ENumerator for methods/functions to be called from else-where.
		enum eFunctionCall {
			FILE_SAVE_AS,                                 //!< Call writeFile related methods.
            EXPORT_AS_LEGACY,                             //!< Disable the export of Features, Flags and Labels in PLY. Then, call writeFile related methods.
			EXPORT_CONNECTED_COMPONENTS,                  //!< Export the labels i.e. connected components as file per component.
			EXPORT_COORDINATES_OF_VERTICES,               //!< Export coordinates of all vertices
			EXPORT_COORDINATES_OF_SELECTED_VERTICES,      //!< Export coordinates of selected vertices
			EXPORT_SELPRIMS_POSITIONS,                    //!< Export positions of selected primitives
			EXPORT_METADATA_HTML,                         //!< Export Meta-Data as HTML page.
			EXPORT_METADATA_JSON,                         //!< Export Meta-Data as JSON.
			EXPORT_METADATA_TTL,                          //!< Export Meta-Data as TTL - Turtle, Terse RDF Triple Language.
			EXPORT_METADATA_XML,                          //!< Export Meta-Data as XML.
			EXPORT_METADATA_ALL,                          //!< Export Meta-Data in all available formats (HTML, JSON, TTL and XML).
			PLANE_FLIP,                                //!< Flip the mesh plane.
			SELPRIM_VERTEX_BY_INDEX,                   //!< Select a single vertex using an index given by the user.
			SELMVERTS_FLAG_SYNTHETIC,                  //!< Call selection of vertices tagged synthetic.
			SELMVERTS_FLAG_CIRCLE_CENTER,              //!< Call selection of vertices tagged as circle center.
			SELMVERTS_FLAG_CORTEX,                     //!< Call selection of vertices tagged as cortex (archaeological lithic illustration).
			SELMVERTS_INVERT,                          //!< Call to invert the selction of (multiple) vertices (SelMVert).
			SELMVERTS_LABEL_IDS,                       //!< Select vertices using label ids.
			SELMVERTS_LABEL_BACKGROUND,                //!< Select vertices being labeled as background.
			SELMVERTS_FROMSELMFACES,                   //!< Call selection of vertices belonging to selected faces (SelMFaces).
			SELMVERTS_RIDGES,                          //!< Select ridge points along Faces edges.
			SELMVERTS_SHOW_INDICES,                    //!< Show the indices of the selected vertices.
			SELMVERTS_SELECT_INDICES,                  //!< Select vertices using indicies given by the user.
			SELMVERTS_RANDOM,                          //!< Randomly select vertices. Amount given by user.
			SELMVERTS_SET_ALPHA,                       //!< Set the color-alpha value of selected vertices.
			SELMFACES_WITH_SYNTHETIC_VERTICES,         //!< Select faces having synthetic vertices.
			SELMFACES_WITH_THREE_BORDER_VERTICES,      //!< Select faces having three border vertices e.g. for erosion.
			SELMFACES_WITH_THREE_SELECTED_VERTICES,    //!< Select faces having three vertices selected (SelMVerts).
			SELMFACES_BORDER_BRIDGE_TRICONN,           //!< Select faces along the border connecting three bridges i.e. 0 border edge and three border vertices.
			SELMFACES_BORDER_BRIDGE,                   //!< Select faces along the border of a bridge i.e. 1 border edge and three border vertices.
			SELMFACES_BORDER_DANGLING,                 //!< Select faces dangling at the border i.e. 2 border edges and three border vertices.
			SELMFACES_LABEL_CORNER,                    //!< Select faces having three vertices with different label id.
			SELMPOLY_BY_VERTEX_COUNT,                  //!< Call selection of (multiple) Polylines (SelMPoly) by given number of vertices.
			SELECT_MESH_PLANE_AXIS_SELPRIM,            //!< Define the mesh plane using the axis and the selected primitive.
			SELECT_MESH_PLANE_AXIS_SELPOS,             //!< Define the mesh plane using the axis and the last selected position.
			ORIENT_MESH_PLANE_TO_AXIS,                 //!< Orients the mesh plane by the cone axis
			FEATUREVEC_MEAN_ONE_RING_REPEAT,           //!< Apply a mean filter repeatedly to the feature vectors' values within a 1-ring neighbourhood.
			FEATUREVEC_MEDIAN_ONE_RING_REPEAT,         //!< Apply a median filter repeatedly to the feature vectors' values within a 1-ring neighbourhood.
			FEATUREVEC_UNLOAD_ALL,                     //!< Unload i.e. remove all the feature vectors.
			FUNCVAL_FEATUREVECTOR_STDDEV_ELEMENTS,     //!< Compute the standard deviation of the feature vector's elements.
			FUNCVAL_FEATUREVECTOR_CORRELATE_WITH,      //!< Compute correlation between vertices' feature vectors and a given reference vector.
			FUNCVAL_FEATUREVECTOR_APPLY_PNORM,         //!< Compute the p-Norm distance or length.
			FUNCVAL_FEATUREVECTOR_APPLY_MAHALANOBIS,   //!< Compute a Mahalanobis inspired distance between feature vectors.
			FUNCVAL_FEATUREVECTOR_MIN_ELEMENT,         //!< Fetch the smallest element of the feature vector for each vertex.
			FUNCVAL_FEATUREVECTOR_MAX_ELEMENT,         //!< Fetch the largest element of the feature vector for each vertex - this is different from the Maximum Norm.
			FUNCVAL_FEATUREVECTOR_MIN_ELEMENT_SIGNED,  //!< Fetch the smallest positive or negative element of the feature vector for each vertex.
			FUNCVAL_FEATUREVECTOR_MAX_ELEMENT_SIGNED,  //!< Fetch the largest positive or negative element of the feature vector for each vertex - this is different from the Maximum Norm.
			FUNCVAL_FEATUREVECTOR_ELEMENT_BY_INDEX,    //!< Fetch one element of a feature vector by its index.
			FUNCVAL_DISTANCE_TO_SELPRIM,               //!< Compute the euclidean distance to the center of gravity of a single primitive (SelPrim).
			FUNCVAL_PLANE_ANGLE,                       //!< Use the mesh plane to compute a slope angle.
			FUNCVAL_ANGLE_TO_RADIAL,                   //!< Use the cone axis to compute an angle between the radial vector and the normal vector.
			FUNCVAL_AXIS_ANGLE_TO_RADIAL,              //!< Use the cone axis to compute an angle between the radial vector and the normals projection onto the cone axis.
			FUNCVAL_ORTHOGONAL_AXIS_ANGLE_TO_RADIAL,   //!< Use the cone axis to compute an angle between the radial vector and the normals projection onto an orthogonal plane around the cone axis. The radial vector is coplanar to this orthogonal plane.
			FUNCVAL_SET_GRAY_RGB_AVERAGE,              //!< Call to set the average of the vertices' RGB Values as function value.
			FUNCVAL_SET_GRAY_RGB_AVERAGE_WEIGHTED,     //!< Call to set the weigthed average of the vertices' RGB Values as function value. (inspired by GIMP)
			FUNCVAL_SET_GRAY_SATURATION_REMOVAL,       //!< Call to remove the saturation and set the gray values as function value.
			FUNCVAL_SET_GRAY_HSV_DECOMPOSITION,        //!< Call to set the HSV component as function value.
			FUNCVAL_SET_DISTANCE_TO_LINE,              //!< Compute distance to a line given a position vector and a directional vector.
			FUNCVAL_SET_DISTANCE_TO_SPHERE,            //!< Compute distance to a sphere given a radius and a center.
			FUNCVAL_SET_DISTANCE_TO_AXIS,              //!< Compute distance to the cone's axis or a user given line.
			FUNCVAL_SET_ANGLE_USING_AXIS,              //!< Use the axis (from the cone) as cylindrical coordinate system and store the angle as function value.
			FUNCVAL_MULTIPLY_SCALAR,                   //!< Multiply the function value with a scalar e.g. to invert the range using -1.0.
			FUNCVAL_TO_ORDER,                          //!< Order the vertices by function value and use the corresponding indicies as function value.
			FUNCVAL_MEAN_ONE_RING_REPEAT,              //!< Apply a mean filter repeatedly to the function value within a 1-ring neighbourhood.
			FUNCVAL_MEDIAN_ONE_RING_REPEAT,            //!< Apply a median filter repeatedly to the function value within a 1-ring neighbourhood.
			FUNCVAL_ADJACENT_FACES,                    //!< Count the number of adjacent faces and store this value as function value for each vertex.
			FUNCVAL_DISTANCE_TO_SEED_MARCHING,         //!< Compute the euclidean distance to a SelPrim/SelVert within a sphere for faces reached by a marching front.
			FUNCVAL_VERT_ONE_RING_AREA,                //!< Compute the 1-ring areas of all vertices.
			FUNCVAL_VERT_ONE_RING_ANGLE_SUM,           //!< Compute the sum of all adjacent face angles within a 1-ring for all vertices.
			FUNCVAL_VERT_MAX_DISTANCE,                 //!< tba
			FUNCVAL_FACE_SORT_INDEX,                   //!< Sort index to face function value.
			FUNCVAL_SPHERE_SURFACE_LENGTH,             //!< Compute a normalized arc length of the intersection of a local part of the mesh surface with a sphere
			FUNCVAL_SPHERE_VOLUME_AREA,                //!< Compute a normalized area of the intersection of a local part of the mesh volume with a sphere
			FUNCVAL_SPHERE_SURFACE_NUMBER_OF_COMPONENTS,  //!< Compute the number of components of the intersection of a local part of the mesh surface with a sphere
			EDIT_REMOVE_SELMFACES,                     //!< Remove selected faces (SelMFace).
			EDIT_REMOVE_FACESZERO,                     //!< Call to remove face with zero area.
			EDIT_REMOVE_FACES_BORDER_EROSION,          //!< Call to remove faces having three border vertices iterativly.
			EDIT_AUTOMATIC_POLISHING,                  //!< Call to polish the mesh i.e. clean and fill.
			EDIT_REMOVE_SEEDED_SYNTHETIC_COMPONENTS,   //!< Call to remove synthetic components using SelMVerts as seeds.
			EDIT_VERTICES_RECOMPUTE_NORMALS,           //!< Recompute the vertex normals.
			EDIT_VERTICES_ADD,                         //!< Manually add vertices by giving triplets of coordinates.
			EDIT_SPLIT_BY_PLANE,                       //!< Split the mesh using its plane.
			EDIT_FACES_INVERT_ORIENTATION,             //!< Change the orientation of the faces. Either for all or for a selection.
			APPLY_TRANSMAT_ALL,						   //!< Call to apply a 4x4 transformation matrix to all vertices.
			APPLY_TRANSMAT_ALL_SCALE,				   //!< Call to apply a 4x4 transformation matrix to all vertices for scaling/skewing.
			APPLY_TRANSMAT_SELMVERT,				   //!< Call to apply a 4x4 transformation matrix to the selected vertices (SelMVert).
			SELMPRIMS_POS_DESELECT_ALL,				   //!< Call to de-select all positions.
			SELMPRIMS_POS_DISTANCES,				   //!< Show distances of the selected positions.
			SELMPRIMS_POS_CIRCLE_CENTERS,              //!< Compute circle centers of selected positions.
			COMPUTE_FEATUREVECTORS_QUICK,              //!< Apply volume integral invariant filtering (MSII) with default options
			GEODESIC_DISTANCE_TO_SELPRIM,              //!< Estimate the geodesic distance to a given primitive (SelPrim).
			POLYLINES_FROM_MULTIPLE_FUNCTION_VALUES,   //!< Compute multiple isolines using the function values.
			POLYLINES_FROM_FUNCTION_VALUE,             //!< Compute isolines using the function values.
			POLYLINES_FROM_PLANE_INTERSECTIONS,        //!< Compute intersections with the mesh plane.
			POLYLINES_FROM_AXIS_AND_POSTIONS,          //!< Compute intersections using the planes defined by the axis and the selected positions (SelPrims)
			POLYLINES_REMOVE_SELECTED,                 //!< Remove selected polylines.
			POLYLINES_REMOVE_ALL,                      //!< Remove all polylines.
			AXIS_ENTER_PRIMEMERIDIAN_ROTATION,         //!< Ask the user to set an angle for positioning the prime meridan.
			AXIS_SET_PRIMEMERIDIAN_SELPRIM,            //!< Use the selected primitive (SelPrim) to set the prime meridian.
			AXIS_SET_CUTTINGMERIDIAN_SELPRIM,          //!< Use the selected primitive (SelPrim) to set the cutting meridian (180°).
			AXIS_FROM_CIRCLE_CENTERS,                  //!< Compute an axis using the circle centers.
			UNROLL_AROUND_CONE,                        //!< Unroll the mesh using the defined cone.
			UNROLL_AROUNG_CYLINDER,                    //!< Unroll the mesh using the defined axis as this is the special case of the cone.
			CONE_COVER_MESH,                           //!< Extend the cone to cover the whole mesh.
			EXTRUDE_POLYLINES,                         //!< Extrude the polylines using the axis.
			LABELING_LABEL_ALL,                        //!< Label all connected components of the mesh.
			LABELING_LABEL_SELMVERTS,                  //!< Label all selected vertices of the mesh. Vertices not selected will become background.
            LABELING_KMEANS_VERT_POS,                  //!< Label all vertices with a k-means clustering with the vertex position
            REFRESH_SELECTION_DISPLAY,   //!< Pretend to have all selections changed.
			SHOW_INFO_MESH,              //!< Display information about the mesh e.g. number of vertices.
			SHOW_INFO_SELECTION,         //!< Display information about the selected primitve.
			SHOW_INFO_FUNCVAL,           //!< Display information about the function values of the vertices.
			SHOW_INFO_LABEL_PROPS,                     //!< Display information about the label properties.
			SHOW_INFO_AXIS,              //!< Display information about the (cone) axis.
			LATEX_TEMPLATE,              //!< Call menu for LaTeX template.
			METADATA_EDIT_MODEL_ID,                    //!< Edit meta-data: model id.
			METADATA_EDIT_MODEL_MATERIAL,              //!< Edit meta-data: object material.
			METADATA_EDIT_REFERENCE_WEB,               //!< Edit meta-data: web reference.
			ELLIPSENFIT_EXPERIMENTAL,
			DRAW_SELF_INTERSECTIONS      //!< Select faces of the mesh that intersect with other faces
		};

		// Parameters -- Boolean / Flags
		virtual bool getParamFlagMesh( MeshParams::eParamFlag rParamID, bool* rValue );
		virtual bool setParamFlagMesh( MeshParams::eParamFlag rParamID, bool rState );

		// Parameters -- Integer / Discrete
		virtual bool setParamIntMesh( MeshParams::eParamInt rParam, int rValue );
		virtual bool getParamIntMesh( MeshParams::eParamInt rParam, int* rValue );

		// Parameters -- Floating point
		virtual bool setParamFloatMesh( MeshParams::eParamFlt rParam, double rValue );
		virtual bool getParamFloatMesh( MeshParams::eParamFlt rParam, double* rValue );

	private:
		bool   mParamFlag[PARAMS_FLAG_COUNT];  //!< Parameters: boolean i.e. flags for processing options.
		int    mParamInt[PARAMS_INT_COUNT];    //!< Parameters: discrete values e.g. choice of selection.
		double mParamFlt[PARAMS_FLT_COUNT];    //!< Parameters: floating point for smootinh etc.
};

#endif
