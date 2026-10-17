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

#ifndef MESHGL_PARAMS_H
#define MESHGL_PARAMS_H

class MeshGLParams {
	public:
		MeshGLParams();
		MeshGLParams( const MeshGLParams * const rParams );
		~MeshGLParams() = default;

		// Switches controlling display of Primitives and other elements:
		enum eParamFlag{
			SHOW_UNDEFINED,                        //!< Flag with lowest ID - NOT to be used for anything other than the valid range of IDs. highest ID is determined by SHOW_FLAGS_COUNT
			SHOW_SMOOTH,                           //!< Swith for smooth/flat rendering (GL_FLAT/GL_SMOOTH)
			SHOW_VERTICES_ALL,                     //!< Show all vertices with color as set in TEXMAP_CHOICE_VERETX_SPRITES.
			SHOW_VERTICES_SOLO,                    //!< Show vertices not connected to any other Primitive, typically of type Face.
			SHOW_VERTICES_BORDER,                  //!< Show veritces along the border of the Mesh.
			SHOW_VERTICES_NON_MANIFOLD,            //!< Show non-manifold vertices.
			SHOW_VERTICES_SINGULAR,                //!< Show singular vertices.
			SHOW_VERTICES_LOCAL_MIN,               //!< Show vertices tagged as local minimum.
			SHOW_VERTICES_LOCAL_MAX,               //!< Show vertices tagged as local maximum.
			SHOW_VERTICES_SELECTION,               //!< Show selected vertices (SelMVerts)
			SHOW_VERTICES_SYNTHETIC,               //!< Show synthetic vertices. Typically added by filling holes.
			SHOW_VERTICES_CORTEX,                  //!< Show cortex vertices (archaeological lithic illustration).
			SHOW_FACES,                            //!< Show the triangles of a mesh
			SHOW_FACES_EDGES,                      //!< Show edges of the Mesh (only when SHOW_FACES is true).
			SHOW_FACES_SELECTION,                  //!< Show the selected triangle.
			SHOW_FACES_CULLED,                     //!< Show the backside of the traingles
			BACKFACE_LIGHTING,                     //!< Sets if the backside of the triangles should recieve lighting
			SHOW_FUNC_VALUES_ISOLINES,             //!< Show isolines, when function values are shown (GLSL/Shader).
			SHOW_FUNC_VALUES_ISOLINES_ONLY,        //!< Show ONLY isolines, when function values are shown (GLSL/Shader).
			SHOW_FUNC_VALUES_ISOLINES_SOLID,       //!< Show isolines using a solid color OR use the inverted color of the mesh.
			SHOW_COLMAP_INVERT,                    //!< Inverts the color map.
	                 SHOW_MESH_AXIS,                        //!< Show the rotational axis of the Mesh (cone/rollout related).
	                 SHOW_MESH_PLANE,                       //!< Shows the plane of the Mesh.
	                 SHOW_MESH_PLANE_TEMP,                  //!< Shows the plane of the Mesh temporary i.e. when the SHIFT-key is pressed.
	                 SHOW_MESH_PLANE_POSITIONS,             //!< Shows the positions defining the plane of the Mesh.
	                 SHOW_MESH_PLANE_AS_CLIPLANE,           //!< Use the plane of the mesh as clipping plane.
			SHOW_CLIP_THRU_SEL,                    //!< Use the selected primitive and the camera plane for clipping.
			DONGARCH_CLIP_PREVIEW_ENABLED,         //!< DongArch v4 Phase 4: Enable real-time clip preview with GL_CLIP_PLANE1
			SHOW_POLYLINES,                        //!< Shows polylines.
	                 SHOW_POLYLINES_CURVATURE,              //!< Polylines with curvature as quadstrip.
	                 SHOW_POLYLINES_CURVATURE_ABS,          //!< Show absolut values for polyline curvature.
			SHOW_NORMALS_VERTEX,                   //!< Normals per Vertex.
			SHOW_NORMALS_FACE,                     //!< Normals per Face.
			SHOW_NORMALS_POLYLINE,                 //!< Normals of Polyline(s).
	                 SHOW_NORMALS_POLYLINE_MAIN,            //!< (single/major/mean) normal per Polyline.
			SHOW_DATUM_SPHERES,                    //!< Show datum sphere(s)
			SHOW_DATUM_BOXES,                      //!< Show datum box(es)
			SHOW_BOUNDING_BOX,                     //!< Shows the bounding box of the Mesh as lines.
			SHOW_BOUNDING_BOX_ENCLOSED,            //!< Shows a semi-transparent bounding box of the Mesh.
			SHOW_SELECTION_SINGLE,                 //!< Shows the selected primitive (single selection).
			SHOW_LABELS_MONO_COLOR,                //!< Shows all labels with the same color.
	                 SHOW_MESH_SPHERE,                      //!< Shows sphere defined for mesh.
			SHOW_MESH_SPHERE_POSITIONS,            //!< Shows positions defining the sphere of the current mesh.
			SHOW_REPEAT_COLMAP_FUNCVAL,            //!< Show repetitive colors for the function values -- see MeshGLParams::WAVES_COLMAP_LEN
			SHOW_NPR_OUTLINES,                     //!< Enable/Disable Outlines
			SHOW_NPR_HATCHLINES,                   //!< Enable/Disable Hatchlines
			SHOW_NPR_TOON,                         //!< Enable/Disable Toon-Shading
			NPR_HATCHLINE_LIGHT_INFLUENCE,         //!< Mix in Hatchlines based on the light influence, e.g. strong light => hatchline less visible
            NPR_USE_SPECULAR,                      //!< Determine whether specular highlights should be used or not
            SHOW_BADLIT_AREAS,                     //!< Shows areas which are over- oder under-lit
            REMOVE_DANGLING_FACES,                  //!< Parameter of Mesh cleaning
            ARCHAEOLOGY_MODE_ENABLED,              //!< Enable archaeological illustration mode (Raczynski-Henk 2017 standards)
            PARAMS_FLAG_COUNT                     //!< Number of flags controlling the OpenGL display. Highest ID+1. Lowest ID-1 is determined by SHOW_UNDEFINED.

		};
		// Parameters (integer)
		enum eParamInt{
			VIEWPARAMS_INT_UNDEFINED,      //!< Parameter (integer) with lowest ID - NOT to be used for anything other than the valid range of IDs. highest ID is determined by SHOW_FLAGS_COUNT
			SHADER_CHOICE,                 //!< Choice for a set of shaders.
			TEXMAP_CHOICE_FACES,           //!< Choice for texture mapping the faces - see eTexMapType.
			TEXMAP_CHOICE_VERETX_SPRITES,  //!< Choice for texture mapping the vertices rendered as sprites - see eTexMapType.
			GLSL_COLMAP_CHOICE,            //!< Colormap used to visualize function values with shaders.
			FUNCVAL_CUTOFF_CHOICE,         //!< Limits of to the function value range e.g. quantil, user-set min/max or automatic min/max.
			COLMAP_LABEL_OFFSET,           //!< Offset used to shift the repeating label colors.
			VERTEX_SPRITE_SHAPE,           //!< Shape of the sprites used for rendering vertices. e.g. Star, Cirlce and Box. see MeshGLParams::eSpriteShapes
			NPR_HATCH_STYLE,               //!< Indicaties which texture to choose for hatching, e.g. Lines or Dots
			NPR_OUTLINE_SOURCE,            //!< Input source for the NPR outlines : 0 = geometry, 1 = FuncVals
			NPR_HATCH_SOURCE,              //!< Input source for the NPR hatches  : 0 = geometry, 1 = FuncVals
			NPR_TOON_SOURCE,               //!< Input source for the NPR toon     : 0 = geometry, 1 = FuncVals
			NPR_TOON_TYPE,                 //!< Rendering type of NPR toon shading: 0 = old, color per lightvalue; 1 = new, discreticise light and hsv-color values
			NPR_TOON_LIGHTING_STEPS,       //!< Number of discrete lighting values for alternative toon-shading
			NPR_TOON_HUE_STEPS,            //!< Number of discrete hue color values for alternative toon-shading
			NPR_TOON_SAT_STEPS,            //!< Number of saturation color values for alternative toon-shading
			NPR_TOON_VAL_STEPS,            //!< Number of value-color values for alternative toon-shading
			TRANSPARENCY_NUM_LAYERS,       //!< How many layers should be used for transparency rendering
			TRANSPARENCY_TRANS_FUNCTION,   //!< What should be used as alpha: 0 = uniform, 1 = vertex color, 2 = funcval, 3 = normal, 4 = label
			TRANSPARENCY_SEL_LABEL,        //!< Which label is rendered opaque if label is used for transparency rendering
			TRANSPARENCY_OVERFLOW_HANDLING,//!< How should a overflow in a k+-buffer be handled: 0 = discard, 1 = multiplicative blend
			TRANSPARENCY_BUFFER_METHOD,    //!< How fragments should be buffered: 0 = K+ Buffer, 1 = A-Buffer
			DEFAULT_FRAMEBUFFER_ID,        //!< The default framebuffer for rendering
            MAX_VERTICES_HOLE_FILLING,     //!< mesh Cleaning parameter, Maximum Number of vertices for hole filling
			PARAMS_INT_COUNT               //!< Number of integer paramters.
		};
		// Parameters (double)
		enum eParamFlt{
			PARAMS_FLT_UNDEFINED,        //!< Parameter (double) with lowest ID - NOT to be used for anything other than the valid range of IDs. highest ID is determined by SHOW_FLAGS_COUNT
			DATUM_SPHERE_TRANS,          //!< Transparency of the datum spheres.
			WAVES_COLMAP_LEN,            //!< Wavelength in units of function value for repeating the color ramp -- see MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL
			TEXMAP_QUANTIL_MIN,          //!< Quantil to cut-off function values for visualization.
			TEXMAP_QUANTIL_MAX,          //!< Quantil to cut-off function values for visualization.
			TEXMAP_QUANTIL_MIN_ABSOLUT,  //!< Absolut value of the Quantil to cut-off function values for visualization - this should never be changed by user interaction!
			TEXMAP_QUANTIL_MAX_ABSOLUT,  //!< Absolut value of the Quantil to cut-off function values for visualization - this should never be changed by user interaction!
			TEXMAP_FIXED_MIN,            //!< Fixed maximum for function value based visualization.
			TEXMAP_FIXED_MAX,            //!< Fixed maximum for function value based visualization.
			TEXMAP_AUTO_MIN,             //!< Automatic minimum of the function values - this should never be changed by user interaction!
			TEXMAP_AUTO_MAX,             //!< Automatic maximum of the function values - this should never be changed by user interaction!
			POLYLINE_NORMAL_SCALE,       //!< Scale factor for polyline normals.
			POLYLINE_WIDTH,              //!< Width of the polylines rendered using OpenGL.
			FUNC_VALUE_LOG_GAMMA,        //!< Gamma value for logarithmic color ramps -- 1.0 means linear mapping.
			ISOLINES_DISTANCE,           //!< Distance of isolines using the function value (GLSL/Shader).
			ISOLINES_OFFSET,             //!< Offset of the isolines (GLSL/Shader).
			ISOLINES_PIXEL_WIDTH,        //!< Pixel width of the isolines (GLSL/Shader).
			BOUNDING_BOX_LINEWIDTH,      //!< Line width of the bounding box of this mesh.
			NORMALS_LENGTH,              //!< Length of the (face/vertex) normals in world coordinates.
			NORMALS_WIDTH,               //!< Width of the (face/vertex) normals -- ratio of the length NORMALS_LENGTH.
			NPR_OUTLINE_WIDTH,           //!< NPR Line-Width of the outlines
			NPR_HATCH_ROTATION,          //!< NPR Rotation of the hatch-lines
			NPR_HATCH_SCALE,             //!< NPR Scale of the hatch-lines
			NPR_OUTLINE_THRESHOLD,       //!< NPR Threshold for sobel filter
            NPR_SPECULAR_SIZE,           //!< NPR Size of Specular highlight
            TRANSPARENCY_UNIFORM_ALPHA,  //!< Transparency, uniform alpha value
            TRANSPARENCY_ALPHA2,         //!< Second alpha for special functions, e.g. angle-based transparency
            TRANSPARENCY_GAMMA,          //!< Gamma value as exponent for interpolations between uniform alpha and alpha2
            BADLIT_UPPER_THRESHOLD,      //!< Upper Threshold for areas that are overlit (see SHOW_BADLIT_AREAS)
            BADLIT_LOWER_THRESHOLD,      //!< Lower Threshold for areas that are underlit (see SHOW_BADLIT_AREAS)
			PIN_SIZE,                    //!< Size of the renderer pins
			PIN_LINE_HEIGHT,             //!< Relative height of the lines connecting the pins
			POINTCLOUD_POINTSIZE,        //!< Pointsize for the pointcloud rendering
			LIGHTVECTOR_LENGTH,          //!< Length of the light-vectors
			DONGARCH_CLIP_PLANE_X,       //!< DongArch v4 Phase 4: Clip plane normal X
			DONGARCH_CLIP_PLANE_Y,       //!< DongArch v4 Phase 4: Clip plane normal Y
			DONGARCH_CLIP_PLANE_Z,       //!< DongArch v4 Phase 4: Clip plane normal Z
			DONGARCH_CLIP_PLANE_W,       //!< DongArch v4 Phase 4: Clip plane distance W
			PARAMS_FLT_COUNT             //!< Number of double paramters.
		};

		// Types of texture mapping
		enum eTexMapType { // Values are hard-coded as they are passed to the shaders.
			TEXMAP_VERT_MONO    = 0,     //!< One color for all (vertex based).
			TEXMAP_VERT_RGB     = 1,     //!< RGB Texturemap (per Vertex) typically supplied within a file.
			TEXMAP_VERT_FUNCVAL = 2,     //!< Visualization of the vertices function values.
			TEXMAP_VERT_LABELS  = 3,     //!< Visualization of the vertices labels.
		};

		// Swith between sets of shaders.
		enum eShaderChoice {
			SHADER_MONOLITHIC,      //!< Use the largest shader.
			SHADER_TRANSPARENCY,    //!< Use the tranparency shader, which is similar in functions to the monolitihic shader.
			SHADER_WIREFRAME,       //!< Use the shader for the wireframe.
			SHADER_NPR,             //!< Use the NPR shader.
			SHADER_POINTCLOUD,      //!< Use Monolithic shader, but render as pointcloud
			SHADER_TEXTURED         //!< Show mesh with texture
		};

		// Colormaps (NEW for shader)
		// for hypsomatic tint see: http://en.wikipedia.org/wiki/Cartographic_relief_depiction#Hypsometric_tints
		// values adopted from: http://www.mathworks.de/matlabcentral/newsreader/view_thread/2727 (Nico Sneeuw,  TU Munich, IAPG, Germany)
		enum eFuncValColorMaps { // Vales are hard-coded, because they actually refer to texture coordinates of shaders/funcvalmapsquare.png
			GLSL_COLMAP_GRAYSCALE       = 0,  //!< Simple grayscale colormap.
			GLSL_COLMAP_HOT             = 1,  //!< Colormap with hot colors: white - yellow - red - black. IMPROVED as suggested by http://cresspahl.blogspot.de/2012/03/expanded-control-of-octaves-colormap.html 
			GLSL_COLMAP_COLD            = 2,  //!< Colormap cold hot colors: white - blue - black. IMPROVED as suggested by http://cresspahl.blogspot.de/2012/03/expanded-control-of-octaves-colormap.html 
			GLSL_COLMAP_HSV             = 3,  //!< Colormap f(hue) - full range [0.0 ... 1.0]
			GLSL_COLMAP_HSV_PART        = 4,  //!< Colormap f(hue) - partial range avoiding red for maximim as it is used for the minimum already.
			GLSL_COLMAP_BREWER_RDGY     = 5,  //!< RdGy diverging colormap for 11 classes from http://colorbrewer2.org
			GLSL_COLMAP_BREWER_SPECTRAL = 6,  //!< Diverging colormap red-yellow-blue for 11 classes from http://colorbrewer2.org
			GLSL_COLMAP_BREWER_RDYLGN   = 7,  //!< Diverging colormap red-yellow-green for 11 classes from http://colorbrewer2.org
			GLSL_COLMAP_HYPSO           = 8,  //!< Hypsometric tint - at least a try.
			GLSL_COLMAP_OCTAVE_JET      = 9, //!< Colormap Jet as used by GNU Octave (similar to HSV).
			GLSL_COLMAP_MORGENSTEMNING  = 10, //!< As suggest by Matthias Geissbuehler and Theo Lasser -- http://dx.doi.org/10.1364/OE.21.009862
			GLSL_COLMAP_HYPSO_HIRISE1   = 11, //!< Extracted from the Hypsometric colorramp similar to those shown at http://www.uahirise.org
			GLSL_COLMAP_HYPSO_HIRISE2   = 12, //!< Extracted from the Hypsometric colorramp similar to those used by Zack Moratto - http://lunokhod.org/?tag=mars
			GLSL_COLMAP_PARULA          = 13, //!< Similar to the new default colorramp used in Matlab replacing Jet.
			GLSL_COLMAP_BREWER_YLORBR   = 14, //!< Sequential colormap - yellow-orange-brown for 9 classes from http://colorbrewer2.org
			GLSL_COLMAP_OCTAVE_COPPER   = 15, //!< Colormap Copper as used by GNU Octave.
			GLSL_COLMAP_RUSTTONES       = 16, //!< Colormap Rust(Tones) similar to Mathematica -- see: http://root.cern.ch/root/html/src/TColor.cxx.html#BMkfdC
			GLSL_COLMAP_SIENNATONES     = 17, //!< Colormap Sienna(Tones) similar to Mathematica -- see: http://root.cern.ch/root/html/src/TColor.cxx.html#BMkfdC
			GLSL_COLMAP_HYPSO_ARID      = 18, //!< Hypsometric tint Arid for land. The Development and Rationale of Cross-blended Hypsometric Tints, Tom Patterson, Bernhard Jenny, Cartographic Perspectives, Number 69, 2011 - See: http://cartographicperspectives.org/index.php/journal/article/viewFile/20/70
		};
		// To get Hex-Codes for Inkscape colorramps of GNU Octave maps use, e.g:
		// [ dec2hex(uint8([jet(16)(:,1)]*255)), dec2hex(uint8([jet(16)(:,2)]*255)), dec2hex(uint8([jet(16)(:,3)]*255)), dec2hex(ones(16,1)*255) ]

		enum eFuncValCutoff {
			FUNCVAL_CUTOFF_QUANTIL,     //!< Cutoff value computed using user-defined quantile.
			FUNCVAL_CUTOFF_MINMAX_USER, //!< Cutoff value (absolut) user-defined.
			FUNCVAL_CUTOFF_MINMAX_AUTO, //!< Cutoff value absolut minimum and maximum (automatically choosen).
		};

		// Shape choices for stripes e.g. rendering vertices.
		enum eSpriteShapes { // Values are hard-coded as they are passed to the shaders.
			SPRITE_SHAPE_BOX          = 0,  //!< A simple sqaure.
			SPRITE_SHAPE_DISC         = 1,  //!< A simple circular disc.
			SPRITE_SHAPE_POLAR_ROSE   = 2,  //!< Polar rose "*".
			SPRITE_SHAPE_STAR_ROUNDED = 3   //!< Rounded star.
		};

		// Infos to be emitted by the mesh e.g. for display.
		enum eInfoMesh {
			MGLINFO_SELECTED_PRIMITIVE, //!< Type of (single) selected primitive (SelPrim).
			MGLINFO_SELECTED_VERTICES,  //!< Number of vertices selected (SelMVerts).
			MGLINFO_SELECTED_FACES,     //!< Number of faces selected (SelMFaces).
			MGLINFO_SELECTED_POLYLINES, //!< Number of polylines selected (SelMPolys/SelMPolylines).
			MGLINFO_SELECTED_POSITIONS  //!< Number of positions selected (SelMPrims.
		};

		// ENumerator for methods/functions to be called from else-where.
		enum eFunctionCall {
			IMPORT_COORDINATES_OF_VERTICES,   //!< Import new coordinates for all vertices and update mesh
			RUN_TPS_RPM_TRANSFORMATION,       //!< Run TPS-RPM transformation
			TEXMAP_FIXED_SET_NORMALIZED,      //!< Set TEXMAP_FIXED_MIN = 0.0 and TEXMAP_FIXED_MAX = 1.0
			SET_SHOW_VERTICES_NONE,           //!< Turn off single vertex rendering except the selected (SelMVerts).
			ISOLINES_SET_BY_NUMBER,           //!< Compute the isoline distance by a given number.
			ISOLINES_SET_TEN_ZEROED,          //!< Compute the isoline distance for 10 lines and set their offset to zero.
			FUNCVAL_AMBIENT_OCCLUSION,        //!< Set function values to values representing local brightness using ambient occlusion
			TRANSFORM_FUNCTION_VALUES_TO_RGB, //!< Trigger transformation of function values to RGB
			MULTIPLY_COLORVALS_WITH_FUNCVALS, //!< Trigger multiplication of color values with function values
			NORMALIZE_FUNCTION_VALUES         //!< Trigger function value normalization
		};

		// Parameters -- Boolean / Flags
		virtual bool       getParamFlagMeshGL( MeshGLParams::eParamFlag rParamID, bool* rValue ) const;
		virtual bool       setParamFlagMeshGL( MeshGLParams::eParamFlag rParamID, bool rState );

		// Parameters -- Integer / Discrete
		virtual bool       getParamIntMeshGL( MeshGLParams::eParamInt rParamID, int* rValue ) const;
		virtual bool       setParamIntMeshGL( MeshGLParams::eParamInt rParamID, int rValue );

		// Parameters -- Floating point
		virtual bool       getParamFloatMeshGL( MeshGLParams::eParamFlt rParamID, double* rValue ) const;
		virtual bool       setParamFloatMeshGL( MeshGLParams::eParamFlt rParamID, double rValue );

		// Parameters -- ALL
		        bool       setParamAllMeshWidget( const MeshGLParams& rParams );

	private:
		// Switches and parameters controlling display of Primitives and other elements:
        bool     mParamFlag[PARAMS_FLAG_COUNT  ];  //!< Array handling the flags to show/hide Primitives and other objects.
		int      mParamInt[PARAMS_INT_COUNT];    //!< Parameters (int).
		double   mParamFlt[PARAMS_FLT_COUNT];    //!< Datum transparency, etc.
};

#endif // MESHGL_PARAMS_H
