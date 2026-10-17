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

#ifndef MESHWIDGET_PARAMS_H
#define MESHWIDGET_PARAMS_H

// generic C++ includes:
#include <iostream>
#include <string>
#include <cmath>    // pow, sqrt, floor, exp, ...

// C++ includes:
// none

// Default values
#define MATERIAL_SPECULAR_DEFAULT   0.3
#define MATERIAL_SHININESS_DEFAULT  2.65  // exp(.)
#define LIGHTX_ANGLE_PHI_DEFAULT    140.0 // Degree
#define LIGHTX_ANGLE_THETA_DEFAULT   40.0 // Degree
#define LIGHTX_INTENSITY_DEFAULT      1.0 // Intervall [0...1]
#define AMBIENT_LIGHT_DEFAULT         0.10

//!
//! \brief Parameter class for the MeshWidhet. (Layer 2)
//!
//! This class handles the parameters for the MeshWidget class.
//! Therefore it contains: flags, ints, doubles and strings.
//!
//! Layer 2
//!

class MeshWidgetParams {
//	friend class MeshWidget;

	public:
		// Constructor and deconstructor:
		MeshWidgetParams();
		MeshWidgetParams( const MeshWidgetParams* const rParams );
		~MeshWidgetParams() = default;

		// Parameters:
		enum eParamFlag{ PARAMS_FLAG_UNDEFINED,     //!< Flag with lowest ID - NOT to be used for anything other than the valid range of IDs. highest ID is determined by SHOW_FLAGS_COUNT
		                 ORTHO_MODE,                //!< Orthographic instead of perspective projection.
		                 SHOW_GRID_RECTANGULAR,     //!< Rectangular grid, best to be used with orthographic projection.
		                 SHOW_GRID_HIGHLIGHTCENTER, //!< Highlight the central pixel.
		                 SHOW_GRID_POLAR_LINES,     //!< Lines of a polar grid.
						 SHOW_GRID_POLAR_CIRCLES,   //!< Concentric circles of a polar grid, best to be used with orthographic projection.
						 SHOW_GRID_HIGHLIGHTCENTER_FRONT, //! If enabled, the whole crosshair of the center highlight will be rendered on top of the mesh
		                 SHOW_HISTOGRAM,            //!< Turn the Mesh Histogram on/off.
		                 SHOW_HISTOGRAM_LOG,        //!< Apply log(arithm) to the Mesh Histogram.
		                 SHOW_HISTOGRAM_SCENE,      //!< Turn Histogram for the OpenGL scene on/off.
		                 SHOW_HISTOGRAM_SCENE_LOG,  //!< Apply log(arithm) to the OpenGL scene histogram.
		                 SHOW_GIGAMESH_LOGO_FORCED, //!< GigaMesh logo shown on the OpenGL canvas i.e. the widget displaying the mesh (ENFORCED).
		                 SHOW_GIGAMESH_LOGO_CANVAS, //!< GigaMesh logo shown on the OpenGL canvas i.e. the widget displaying the mesh.
		                 SHOW_KEYBOARD_CAMERA,      //!< Shown the keyboard layout for camera movement on the OpenGL canvas i.e. the widget displaying the mesh.
		                 SHOW_FOG,                  //!< Switch for OpenGL fog.
		                 SPHERICAL_VERTICAL,        //!< Switch for axis of rotation for spherical images.
		                 LIGHT_ENABLED,             //!< Turns ALL lights off and selected lights on.
		                 LIGHT_FIXED_WORLD,         //!< Switch for light with fixed position in world coordinates - corresponds to GL_LIGHT0
		                 LIGHT_FIXED_CAM,           //!< Switch for light with fixed position in camera coordinats - corresponds to GL_LIGHT1
		                 LIGHT_AMBIENT,             //!< Ambient light on/off - corresponds to GL_LIGHT_MODEL_AMBIENT
		                 ARCHAEOLOGY_MODE_ENABLED,  //!< Enable archaeological illustration mode (Raczynski-Henk 2017 standards)
		                 CORTEX_RENDERING_ENABLED,  //!< Enable cortex cross-hatch rendering (Raczynski-Henk 2017 Section 2.7)
		                 DONGARCH_CLIP_PREVIEW_ENABLED, //!< Enable DongArch clip preview with OpenGL clipping plane
		                 CROP_SCREENSHOTS,          //!< Crop screenshots using the z-buffer.
		                 VIDEO_FRAME_FIXED,         //!< Switch between resizeable and fixed window size.
		                 EXPORT_SVG_AXIS_DASHED,    //!< Export the rotational axis with(out) dashes to a SVG.
		                 EXPORT_SIDE_VIEWS_SIX,     //!< Toggle between six and eigth side-views. The latter is typically only interesting for cuneiform tablets.
			             SCREENSHOT_FILENAME_WITH_DPI,    //!< Add the DPI for ortho images to the filename.
			             SCREENSHOT_PNG_BACKGROUND_OPAQUE, //!< Toggle between transparent background (false) and background color.
		                 SHOW_MESH_REDUCED,         //!< Is set when the mesh is moved and the option is enabled the mesh should be shown as a reduced pointcloud or not
		                 ENABLE_SHOW_MESH_REDUCED,  //!< Toggle if the mesh should be shown as a reduced pointcloud or not
                         EXPORT_TTL_WITH_PNG,  //!< Turn the metadata as ttl during the png export on/off
		                 PARAMS_FLAG_COUNT               //!< Number of flags available
		};
		enum eParamInt{ PARAMS_INT_UNDEFINED,       //!< ID for undefined paramaters.
		                MOUSE_MODE,                 //!< Modes for using the mouse to interact with the scene -- see eMouseModes.
		                SELECTION_MODE,             //!< State to handle selections.
		                HISTOGRAM_TYPE,             //!< Type of the histogram -- see Mesh::eHistogramType
		                HISTOGRAM_POSX,             //!< x-coordinate of the overlayed Histogram
		                HISTOGRAM_POSY,             //!< y-coordinate of the overlayed Histogram
		                HISTOGRAM_WIDTH,            //!< width of the overlayed Histogram
		                HISTOGRAM_HEIGHT,           //!< height of the overlayed Histogram
		                VIDEO_FRAME_WIDTH,          //!< Fixed width for rendring video/flash.
		                VIDEO_FRAME_HEIGHT,         //!< Fixed height for rendring video/flash.
		                STATE_NUMBER,               //!< State number for VR export.
		                IMAGE_SPLIT_SIZE,           //!< Maximum width and height in pixel for a tiled image. Used to split larger images into severall sub-images.
		                LIGHT_VECTORS_SHOWN_MAX,    //!< Used in th geometry shader to limit the number of vectors used to show the light direction.
		                GRID_CENTER_POSITION,       //!< Sets where the center of the rendered grid is located. See dialogGridCenterSelect class for the values
		                PARAMS_INT_COUNT            //!< Number of integer parameters available
		};
		enum eParamFlt{ PARAMS_FLT_UNDEFINED,       //!< ID for undefined paramaters.
		                VIEW_DIST_DECREMENT,        //!< Zoom, perspective: Decrement the distance betweeb eyeX/Y/Z and centerViewX/Y/Z
		                ORTHO_ZOOM,                 //!< Zoom for orthographic mode (linked to mousewheel).
		                ORTHO_SHIFT_HORI,           //!< Horizontal shift in pixels in orthographic mode.
		                ORTHO_SHIFT_VERT,           //!< Vertical shift in pixels in orthographic mode.
		                GRID_SHIFT_DEPTH,           //!< Shift the grid(s) forward/backward along the view direction of the camera.
		                FOV_ANGLE,                  //!< cameras field of view, angle in degree (not radiant!!)
		                ROTATION_STEP,              //!< Angle (in degree) used to increment rotations (e.g. by user interaction).
		                AMBIENT_LIGHT,              //!< Amount of ambient light [0.0 ... 1.0]
		                MATERIAL_SHININESS,         //!< Material shininess [0.0 .... 128.0] regarding OpenGL specs
		                MATERIAL_SPECULAR,          //!< Material specularity
		                LIGHT_STEPPING,             //!< sin( angle ) for moving the light source [-1.0 ... +1.0]
		                LIGHT_FIXED_CAM_ANGLE_PHI,      //!< Angle phi of the light in fixed position to the camera coordinate system.
		                LIGHT_FIXED_CAM_ANGLE_THETA,    //!< Angle theta of the light in fixed position to the camera coordinate system.
		                LIGHT_FIXED_CAM_INTENSITY,      //!< i.e Brightness of the light source fixed to the camera coordinate system.
		                LIGHT_FIXED_WORLD_ANGLE_PHI,    //!< Angle phi of the light in fixed position to the world/object coordinates.
		                LIGHT_FIXED_WORLD_ANGLE_THETA,  //!< Angle theta of the light in fixed position to the world/object coordinates.
		                LIGHT_FIXED_WORLD_INTENSITY,    //!< i.e Brightness of the light source fixed to the world/object coordinate system.
		                FOG_LINEAR_START,           //!< Distance to the camera, where linear fogs starts to appear.
		                FOG_LINEAR_END,             //!< Distance to the camera, where the linear fogs reaches its maximum.
		                VIDEO_DURATION,             //!< Length of a video sequence in seconds => number of still images.
		                VIDEO_FRAMES_PER_SEC,       //!< Number of frames per seconds for video computation => number of still images.
		                VIDEO_SLOW_STARTSTOP,       //!< Seconds to accelrate/decelerate the movement within the video frame sequence.
		                SPHERICAL_STEPPING,         //!< Angle stepping (in degree) for sperical images.
		                RULER_HEIGHT,               //!< Ruler width in world units (typically: mm or meter).
		                RULER_WIDTH,                //!< Ruler length in world units (typically: mm or meter).
		                RULER_UNIT,                 //!< Ruler unit in world units (typically: mm or meter).
		                RULER_UNIT_TICKS,           //!< Ruler unit tickmarks in world units (typically: mm or meter).
		                SVG_SCALE,                  //!< DPI assumed for SVG file. e.g. Inkscape expects 72 DPI as default.
		                HIGHDPI_ZOOM_FACTOR,        //!< For HighDPI displays with magnification enabled. (Linux only?)
		                DONGARCH_CLIP_PLANE_X,      //!< DongArch clip plane X coefficient (normal X)
		                DONGARCH_CLIP_PLANE_Y,      //!< DongArch clip plane Y coefficient (normal Y)
		                DONGARCH_CLIP_PLANE_Z,      //!< DongArch clip plane Z coefficient (normal Z)
		                DONGARCH_CLIP_PLANE_W,      //!< DongArch clip plane W coefficient (distance)
		                PARAMS_FLT_COUNT            //!< Number of float paramters.
		};

		// Enumerator for string-type parameters
		enum eParamStr{
			FILENAME_EXPORT_VR,               //!< Filename - typically with sprintf expression - for VR export.
			FILENAME_EXPORT_VIEWS,            //!< Filename (pattern) to export the 4 side, top and bottom view.
			FILENAME_EXPORT_VIEWS_LATEX,      //!< Filename (pattern) to export the 4 side, top and bottom view.
			FILENAME_EXPORT_RULER,            //!< Filename (pattern) to export a true to scale ruler (ortho mode!)
			RULER_WIDTH_UNIT,                 //!< Unit of Ruler
			INKSCAPE_COMMAND,                 //!< path to run inkscape
			PDF_LATEX_COMMAND,                //!< path to run pdflatex
            PDF_VIEWER_COMMAND,               //!< program to view pdf's (evince)
            PDF_VIEWER_COMMAND_ALT1,          //!< program to view pdf's, first alternative (okular)
            PDF_VIEWER_COMMAND_ALT2,          //!< program to view pdf's, second alternative (atril)
			PYTHON3_COMMAND,                  //!< path to python3
			PARAMS_STR_COUNT                  //!< Number of integer paramters.
		};

		// Enumeraturs for integer related parameters:
		enum eMouseModes {
			MOUSE_MODE_MOVE_CAMERA,             //!< Move the camera
			MOUSE_MODE_MOVE_PLANE,              //!< Move the plane.
			MOUSE_MODE_MOVE_PLANE_AXIS,         //!< Move plane bound by axis
			MOUSE_MODE_ROTATE_PLANE_AXIS,       //!< Rotate plane bound by axis
			MOUSE_MODE_MOVE_LIGHT_FIXED_CAM,    //!< Move the OpenGL light fixed to the camera coordinate system.
			MOUSE_MODE_MOVE_LIGHT_FIXED_OBJECT,  //!< Move the OpenGL light fixed in the object coordinate system.
			MOUSE_MODE_SELECT,                  //!< Use the mouse for selection.
			MOUSE_MODE_COUNT                    //!< Number of choices for mouse modes.
		};

		// Values for interactive selection modes.
		enum eSelectionModes{
			SELECTION_MODE_NONE,       //!< No selection.
			SELECTION_MODE_VERTEX,     //!< Select a (single!) Vertex.
			SELECTION_MODE_FACE,       //!< Select a (single!) Face.
			SELECTION_MODE_VERTICES,   //!< Select multiple Vertices by pinpointing (SelMVerts).
			SELECTION_MODE_VERTICES_LASSO,   //!< Selects all/multiple vertices in a polygonal area (SelMVerts).
            DESELECTION_MODE_VERTICES_LASSO,   //!< Deselects all/multiple vertices in a polygonal area (SelMVerts).
			SELECTION_MODE_MULTI_FACES,      //!< Select multiple Vertices by pinpointing (SelMFaces).
			SELECTION_MODE_PLANE_3FP,  //!< Select 3 point for a plane.
			SELECTION_MODE_CONE,       //!< Selects points for a cone.
			SELECTION_MODE_SPHERE,     //!< Selects points for a sphere.
			SELECTION_MODE_POSITIONS,  //!< Select positions from faces or  (solo) vertices.
            SELECTION_MODE_THREE_POSITIONS,  //!< Select 3 positions from faces or  (solo) vertices. Special modus of Profile Lines
			SELECTION_MODE_SECTION_DRAG,  //!< Create section plane by dragging (2 points). For archaeological illustration.
			SELECTION_MODE_COUNT       //!< Number of selection choices.
		};

		// Infos to be emitted by the viewport e.g. for display.
		enum eViewPortInfo {
			VPINFO_FRAMES_PER_SEC, //!< Frames per Second
			VPINFO_DPI,            //!< Dots per Inch
			VPINFO_LIGHT_CAM,      //!< Orientation of the light fixed to the camera-
			VPINFO_LIGHT_WORLD,    //!< Orientation of the light fixed to the world/object.
			VPINFO_FUNCTION_VALUE, //!< Function value of the selected primiitve (SelPrim).
			VPINFO_LABEL_ID        //!< Label ID of the selected primiitve (SelPrim).
		};

		// ENumerator for methods/functions to be called from else-where.
		enum eFunctionCall {
			EXPORT_POLYLINES_INTERSECT_PLANE,   //!< Export the polylines computed using an intersecting plane.
			SCREENSHOT_CURRENT_VIEW_SINGLE,     //!< Write a single image with the current view.
			SCREENSHOT_CURRENT_VIEW_SINGLE_PDF, //!< Write a single image with the current view embedded into a PDF.
			SCREENSHOT_VIEWS_IMAGES,            //!< Side-views of the mesh as PNGs or TIFFs.
			SCREENSHOT_VIEWS_PDF,               //!< Side-views of the mesh as PNGs embedded into a PDF.
			SCREENSHOT_VIEWS_DIRECTORY,         //!< Side-views or front-view of all meshes in a given directory as PNGs with or without embeding via LaTeX into PDFs (one per mesh).
			DIRECTORY_FUNCVAL_TO_RGB,           //!< Apply function value rendering settings to multiple files and store the coloring as RGB.
			EDIT_SET_CONEAXIS_CENTRALPIXEL,     //!< Use the central pixel of the viewport to set the axis of the cone. See Mesh::setConeAxis
			SET_CURRENT_VIEW_TO_DEFAULT,        //!< Use the current view (matrix) to set the default view of the object.
			SET_ORTHO_DPI,                      //!< Set the screen resolution for the orthographic rendering.
			SET_RENDER_DEFAULT,                 //!< Use predefined default settings for objects i.e. high contrast for cuneiform tablets.
			SET_RENDER_MATTED,                  //!< Use predefined settings for objects made from wood or clay.
			SET_RENDER_METALLIC,                //!< Use predefined settings for objects made metal or having a shiny surface.
			SET_RENDER_LIGHT_SHADING,           //!< Use predefined settings for landscape or in combination with ambient occlusion.
			SET_RENDER_FLAT_AND_EDGES,          //!< Use predefined settings for inspection of mesh details.
			SET_GRID_NONE,                      //!< Turn off grid.
			SET_GRID_RASTER,                    //!< Switch to mm-grid - see SHOW_GRID_RECTANGULAR
			SET_GRID_POLAR,                     //!< Switch to polar grid - see SHOW_GRID_POLAR_LINES and SHOW_GRID_POLAR_CIRCLES
			SET_VIEW_AXIS_UP,                   //!< Set the view aligned to the mesh axis.
			SET_VIEW_PARAMETERS,                //!< Set the view parameters.
			SHOW_VIEW_PARAMETERS,               //!< Show the view parameters.
			SHOW_VIEW_2D_BOUNDING_BOX,          //!< Show the 2D bounding box of the (projected) mesh. Not to be confused with the mesh's bounding box in 3D!
		};

		// ENumerator for user guidance in selection mode:
		enum eGuideIDSelection{
			GUIDE_SELECT_NONE,                //!< No user guide shown
			// Single primitive SelPrim
			GUIDE_SELECT_SELPRIM_VERTEX,      //!< Select a single vertex (SelPrim/SelVert).
			GUIDE_SELECT_SELPRIM_FACE,        //!< Select a single face   (SelPrim/SelFace).
			// Multiple selections:
			GUIDE_SELECT_SELMVERTS,           //!< Select multiple vertices by pinpointing (SelMVerts).
			GUIDE_SELECT_SELMVERTS_LASSO,     //!< Select multiple vertices using a polygonal area (SelMVerts).
            GUIDE_DESELECT_SELMVERTS_LASSO,     //!< Deselect multiple vertices using a polygonal area (SelMVerts).
			GUIDE_SELECT_SELMFACES,           //!< Select multiple faces by pinpointing (SelMVerts).
			// Positions:
			GUIDE_SELECT_POSITIONS,           //! Select multiple positions on the mesh.
            GUIDE_SELECT_THREE_POSITIONS,     //! Select three positions on the mesh. Used with the profile lines computation
			// Plane by three points:
			GUIDE_SELECT_PLANE_3FP_A,  //!< Define plane by three vertices - select vertex A
			GUIDE_SELECT_PLANE_3FP_B,  //!< Define plane by three vertices - select vertex B
			GUIDE_SELECT_PLANE_3FP_C,  //!< Define plane by three vertices - select vertex C
			// Cone by an axis and two points:
			GUIDE_SELECT_CONE_AXIS,    //!< Define a cone by an axis and two veritces - select axis
			GUIDE_SELECT_CONE_PR1,     //!< Define a cone by an axis and two veritces - select p_r_1
			GUIDE_SELECT_CONE_PR2,     //!< Define a cone by an axis and two veritces - select p_r_2
			// Sphere by four points:
			GUIDE_SELECT_SPHERE_P0,    //!< Define a sphere using four veritces - 0/4
			GUIDE_SELECT_SPHERE_P1,    //!< Define a sphere using four veritces - 1/4
			GUIDE_SELECT_SPHERE_P2,    //!< Define a sphere using four veritces - 2/4
			GUIDE_SELECT_SPHERE_P3,    //!< Define a sphere using four veritces - 3/4
			GUIDE_SELECT_SPHERE_P4,    //!< Define a sphere using four veritces - 4/4
			// Worst-case:
			GUIDE_SELECT_ERROR         //!< Error
		};

		// ENumerator for user guidance:
		enum eGuideIDCommon{
			GUIDE_COMMON_NONE,                //!< No user guide shown
			// Cone by an axis and two points:
			GUIDE_COMMON_CONE_DEFINED,        //!< Define a cone by an axis and two veritces - defined!
			// Worst-case:
			GUIDE_COMMON_ERROR                //!< Error i.e. undefined
		};

		// Flags:
		virtual bool getParamFlagMeshWidget( eParamFlag rFlagNr, bool* rState ) const;
		virtual bool setParamFlagMeshWidget( eParamFlag rFlagNr, bool rState );
		virtual bool toggleShowFlag( eParamFlag rFlagNr );
		// Integer:
		virtual bool getParamIntegerMeshWidget( eParamInt rParam, int* rValue ) const;
		virtual bool setParamIntegerMeshWidget( eParamInt rParam, int rValue );
		// Floating point:
		virtual bool getParamFloatMeshWidget( eParamFlt rParam, double* rValue ) const;
		virtual bool setParamFloatMeshWidget( eParamFlt rParam, double rValue );
		// Strings:
		virtual bool getParamStringMeshWidget( eParamStr rParamID, std::string* rString ) const;
		virtual bool setParamStringMeshWidget( eParamStr rParamID, const std::string& rString );
		// ALL:
		        bool setParamAllMeshWidget( const MeshWidgetParams& rParams );

		// Extra Helper functions (to prevent copied code)
		bool getGridCenterPosOffsets( double& rXOffset, double& rYOffset );

	protected: //! \todo make private
		bool     mParamFlag[PARAMS_FLAG_COUNT];  //!< Parameters: boolean AKA flags.
		int      mParamInt[PARAMS_INT_COUNT];    //!< Parameters: integer.
		double   mParamFlt[PARAMS_FLT_COUNT];    //!< Parameters: floating point for light, material, video duration, etc.
	private:
		std::string   mParamStr[PARAMS_STR_COUNT];    //!< Parameters: strings.
};

#endif // MESHWIDGET_PARAMS_H
