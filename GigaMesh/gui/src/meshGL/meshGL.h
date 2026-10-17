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

#ifndef MESHGL_H
#define MESHGL_H

#include <GigaMesh/mesh/mesh.h>
#include "meshGL_params.h"
#include "meshglcolors.h"

#include "TexturedMesh.h"

// OpenGL stuff
//#define GL_GLEXT_PROTOTYPES
#ifdef MACXPORT
	#include <gl.h>
	#include <OpenGL/glu.h>
#elif defined (_MSC_VER)
    #include <gl/GL.h>

#else
	//#include <QtOpenGL>
	//#include <GL/glu.h>
#endif
#include <QtOpenGL/QGLContext>
#include <QOpenGLBuffer>

#define OPENGL_VBO_SHOW_MEMORY_USAGE 1

struct PixCoord { int x; int y; };

class MeshWidgetParams;

//!
//! \brief OpenGL extension for the Mesh class. (Layer 1)
//!
//! Extends the Mesh class with OpenGL functions for interactive display and
//! manipulation.
//!
//! Layer 1
//!
//! Vertex Buffer Objects - see: http://en.wikipedia.org/wiki/Vertex_Buffer_Object
//! White paper: http://www.opengl.org/registry/specs/ARB/vertex_buffer_object.txt

class MeshGL : public Mesh, public MeshGLParams {

	public:
		MeshGL( QGLContext* rGLContext, const std::filesystem::path& rFileName, bool& rReadSuccess );
		~MeshGL();

		// Menu interactions:
		        bool       callFunction( MeshParams::eFunctionCall rFunctionID,   bool rFlagOptional=false ) override;
		virtual bool       callFunction( MeshGLParams::eFunctionCall rFunctionID, bool rFlagOptional=false );

				bool         labelsChanged()    override;
				void         polyLinesChanged() override;

		unsigned int selectedMVertsChanged() override;
		unsigned int selectedMFacesChanged() override;

		        bool       compPolylinesIntInvRunLen( double rIIRadius, PolyLine::ePolyIntInvDirection rDirection ) override;
				bool       compPolylinesIntInvAngle( double rIIRadius ) override;
		virtual void       getPolylineExtrema();
		virtual bool       fillPolyLines( uint64_t& rFilled, uint64_t& rFail, uint64_t& rSkipped );

                bool       applyTransformationToWholeMesh( Matrix4D rTrans, bool rResetNormals = true, bool rSaveTransMat = true ) override;
                bool       applyTransformation( Matrix4D rTrans, std::set<Vertex*>* rSomeVerts, bool rResetNormals = true, bool rSaveTransMat = true ) override;
				bool       applyMeltingSphere( double rRadius, double rRel ) override;

				bool       normalsVerticesChanged() override;

				void       changedFaceFuncVal() override;
				void       changedVertFuncVal() override;
			void       cortexVerticesChanged(); //!< Invalidate VBO for cortex vertices (archaeological mode)

		// Color creation related
		        bool       normalizeFunctionValues();
		        bool       runFunctionValueToRGBTransformation();

				bool       assignImportedTexture( int rLineCount, uint64_t* rRefToPrimitves, unsigned char* rTexMap ) override;
				bool       assignImportedNormalsToVertices( const std::vector<grVector3ID>& rNormals ) override;
				bool       multiplyColorWithFuncVal() override;
				bool       multiplyColorWithFuncVal( const double rMin, const double rMax ) override;
				bool       assignAlphaToSelectedVertices(unsigned char alpha) override;

		// UI/Selection:
		        bool       selectPlaneThreePoints( int rXPixel, int rYPixel );
		        bool       selectConePoints( int rXPixel, int rYPixel );
		        bool       selectSpherePoints( int rXPixel, int rYPixel );
                bool       selectPositionAt( int rXPixel, int rYPixel, bool rLastPoint );
                bool       isMoreThanNconSelPosition( int rN);
        virtual bool       selectPrism( std::vector<PixCoord> &rTri, bool rDeselection = false );
        virtual bool       selectPoly( std::vector<PixCoord>& rPixels, bool rDeselection = false );
		virtual bool       callTriangle(std::vector<PixCoord> &p, std::vector<PixCoord> &tri);

		virtual Primitive* selectPrimitiveAt( int primitiveTypeToSelect, int xPixel, int yPixel, bool addToList );
	private:
		        Vertex*    getVertexAt( int xPixel, int yPixel );
		        Face*      getFaceAt( int rPixelX, int rPixelY, Vector3D* rPointIntersect=nullptr );
		        Face*      getFaceAt( const Vector3D& rRayTop, const Vector3D& rRayBot, const Vector3D& vecPointSel, Vector3D* rPointIntersect=nullptr );
	public:
		        bool       getRayWorld( int xPixel, int yPixel, Vector3D* rayTop, Vector3D* rayBot );
		        bool       getWorldPoint( int rPixelX, int rPixelY, float rDepth, Vector3D* rPosVec );
				bool       getWorldPointOnMesh( int rPixelX, int rPixelY, Vector3D* rPosVec);

		// Function Values - Generic:
		virtual bool       setVertexFuncValues(Vertex** vertices, double* values, int verticesNr, const std::string& setName );
		        bool       funcVertAmbientOcclusion();
		        bool       funcVertAmbientOcclusion( int rResolution, unsigned int rNumberOfDirections, float rZTolerance );
		        bool       funcVertAmbientOcclusionHW( int rDepthBufferRes, int rMaxValueBufferRes, unsigned int rNumberOfDirections, float rZTolerance );
		        bool       funcVertAmbientOcclusionHWPass( const QMatrix4x4 &rTransformToClipspace, QOpenGLFramebufferObject &rDepthFBO, QOpenGLFramebufferObject &rValueFBO, unsigned int rNumberOfDirections, float rZTolerance, unsigned int rPassNr, unsigned int rNrOfPasses, unsigned int rStartVertIndex );
		        //! \todo The following methods and related Params could be moved down to the Mesh layer.
		        bool       getFuncValMinMaxUser( double* rMin, double* rMax );
		        bool       getFuncValIsoStepping( unsigned int rNumberOfIsoLines , double* rIsoDistance );
		        bool       setFuncValIsoSteppingByNumberUI();
		        bool       setFuncValIsoSteppingByNumber( unsigned int rNumberOfLines, bool rSetOffsetToZero );

		// OpenGL related:
		        void       glPrepare();
		virtual void       glPaint();
		        void       glRemove();
		virtual void       glPaintDepth( const QMatrix4x4 &rTransformMat );
		virtual void       glPaintFrontalLightPerVertex( const QMatrix4x4 &rTransformMat, GLint rXResolution, GLint rYResolution, GLuint rDepthTexture, GLfloat rZTolerance, GLint rFirstVertIdx );

		// Handling of new primitives:
				bool       removeVertices( std::set<Vertex*>* verticesToRemove ) override;
				bool       insertVerticesCoordTriplets( std::vector<double>* rCoordTriplets ) override;
				bool       insertVertices( std::vector<Vertex*>* rNewVertices ) override;

		// Parameters:
				bool       setParamFlagMeshGL(  MeshGLParams::eParamFlag rParamID, bool   rState ) override;
				bool       setParamIntMeshGL(   MeshGLParams::eParamInt  rParamID, int    rValue ) override;
				bool       setParamFloatMeshGL( MeshGLParams::eParamFlt  rParamID, double rValue ) override;


		friend class MeshQtCSVImportExport;

protected:
	// Vertex Buffer Objects (VBOs) ------------------------------------------------------------------------------------------------------------------------
	enum eVertBufObjs {
		VBUFF_VERTICES_STRIPED,         //!< Vertex data striped: position, color, normal.                                                (VertexBuffer)
		VBUFF_VERTICES_SOLO,            //!< Index buffer for solo vertices (not belonging to e.g. a face).                              (ELementbuffer)
		VBUFF_VERTICES_BORDER,          //!< Index buffer for vertices along the border of a mesh.                                       (ELementbuffer)
		VBUFF_VERTICES_NONMANIFOLD,     //!< Index buffer for vertices along non-manifold edges.                                         (ELementbuffer)
		VBUFF_VERTICES_SINGULAR,        //!< Index buffer for singular vertices ("double cones").                                        (ELementbuffer)
		VBUFF_VERTICES_FLAG_LOCAL_MIN,  //!< Index buffer for vertices with a specific flag (LOCAL_MIN).                                 (ELementbuffer)
		VBUFF_VERTICES_FLAG_LOCAL_MAX,  //!< Index buffer for vertices with a specific flag (LOCAL_MAX).                                 (ELementbuffer)
		VBUFF_VERTICES_FLAG_SELECTED,   //!< Index buffer for vertices with a specific flag (SELECTED).                                  (ELementbuffer)
		VBUFF_VERTICES_FLAG_SYNTHETIC,  //!< Index buffer for vertices with a specific flag (SYNTHETIC).                                 (ELementbuffer)
		VBUFF_VERTICES_FLAG_CORTEX,     //!< Index buffer for vertices with cortex flag (archaeological lithic illustration).            (ELementbuffer)
		VBUFF_FACES,                    //!< Index buffer for triangles with a specific flag (SELECTED).                                 (ELementbuffer)
		VBUFF_FACES_FLAG_SELECTED,      //!< Index buffer for triangles.                                                                 (ELementbuffer)
        VBUFF_CUBE,                     //!< Simple cube e.g. for bounding box lines.                                                     (VertexBuffer)
		VBUFF_CYLINDER,                 //!< Simple cylinder e.g. to render cones for unwrapping.                                         (VertexBuffer)
		VBUFF_SPHERE_VERTS,             //!< Simple sphere, e.g. to render spheres for unwrapping.                                        (VertexBuffer)
		VBUFF_SPHERE_FACES,             //!< Simple sphere, e.g. to render spheres for unwrapping.                                       (ELementbuffer)
		VBUFF_VERTICES_POLYLINES,       //!< Vertex data to be used to render polylines with a geometry shader.                           (VertexBuffer)
        VBUFF_POLYLINES,                //!< Index buffer for polylines.                                                                 (ELementbuffer)
        VBUFF_SCREEN_QUAD,              //!< Simple Quad to render full-screen buffers, e.g. NPR or transparency                          (VertexBuffer)
		VBUFF_COUNT                     //!< Number of VBO arrays.                                                                    (NO buffer at all)
	};
	GLuint          mVAO;                      //!< Vertex Array Object - has to be created!
	QOpenGLBuffer*  mVertBufObjs[VBUFF_COUNT]; //!< Array of Vertex Buffer Objects (within the VAO).

	TexturedMesh* mMeshTextured = nullptr;               //!< Class holding the vertex-buffers for a mesh with multiple textures
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
	long   mVboMemoryUsage; //!< Memory usage for resources analysis and debugging.
#endif

	//! Simple structure used to stripe vertex data for VBOs -- see eVBOs::VBO_VERTICES_STRIPED
	struct grVertexElmentBasic {
		GLfloat  mPosition[3]; //!< xyz-coordinate.
		GLfloat  mNormal[3];   //!< Normal vector.
		GLubyte  mColor[4];    //!< Color information (RGBA).
		GLfloat  mFuncVal;     //!< Function value.
	} ;
	struct grVertexStripeElment : grVertexElmentBasic {
		GLfloat  mLabelID;     //!< Label number. The reason for using a float is the fact that it will be interpolated for faces and a non-integer value indicates a face along a label border.
		GLfloat  mFlags;       //!< State flags, such as border, selected, solo, etc. This should be UINT, but all buffer data is NORMALIZED: http://qt-project.org/forums/viewthread/38929
	};

	struct grVertexTextured {
			GLfloat position[3];
			GLfloat normal[3];
			GLfloat uv[2];
	};

private:
	        bool mVBOPrepared;   //!< Has to be true, when VBOs were actually prepared.

	        void vboPrepareBox();
	        void vboPrepareCylinder();
            void vboPrepareSphere();
            void vboPrepareQuad();
protected:
	        bool vboPreparePolylines();
	        bool vboPrepareVerticesStriped();
	        bool vboPrepareVerticesStripedFetchVertex( Vertex* rVertex, grVertexStripeElment* rWriteTo );
	        bool vboPrepareDoubleCone();
	        bool vboPrepareVerticesWithFlag( unsigned int rFlagNr, eVertBufObjs rBufferID );
	        bool vboPrepareFacesWithFlag(    unsigned int rFlagNr, eVertBufObjs rBufferID );
private:
	// Generic functions to add ander remove Vertex Buffer Objects:
			void vboRemoveBuffer(eVertBufObjs rBufferID, const std::string& rCallingFunc );
			void vboAddBuffer(GLsizeiptr rTotalSize, GLvoid* rData,
			                  QOpenGLBuffer::UsagePattern rUsage, eVertBufObjs rBufferID, const std::string& rCallingFunc );

			bool runFunctionValueToRGBTransformation_impl(
			                const QImage& colorMapImage,
			                const int meshGLColorMapChoiceValue,
			                const bool showRepeatColorMapValue,
			                const bool invertFunctionValueColorValue,
			                const double functionValueMin,
			                const double functionValueMax,
			                const double functionValueRepetitionIntervalValue,
			                const double functionValueLogarithmGammaValue,
			                const uint64_t offset,
			                const uint64_t range );

			bool runFunctionValueToRGBTransformationUpdateProgress_impl(
			                const QImage& colorMapImage,
			                const int meshGLColorMapChoiceValue,
			                const bool showRepeatColorMapValue,
			                const bool invertFunctionValueColorValue,
			                const double functionValueMin,
			                const double functionValueMax,
			                const double functionValueRepetitionIntervalValue,
			                const double functionValueLogarithmGammaValue,
			                const uint64_t offset,
			                const uint64_t range );

			bool runFunctionValueNormalization_impl(const bool showRepeatColorMapValue,
													  const bool invertFunctionValueColorValue,
													  const double functionValueMin,
													  const double functionValueMax,
													  const double functionValueRepetitionIntervalValue,
													  const double functionValueLogarithmGammaValue,
			                                          const uint64_t offset,
			                                          const uint64_t range);

			bool runFunctionValueNormalizationUpdateProgress_impl(const bool showRepeatColorMapValue,
																  const bool invertFunctionValueColorValue,
																  const double functionValueMin,
																  const double functionValueMax,
																  const double functionValueRepetitionIntervalValue,
																  const double functionValueLogarithmGammaValue,
			                                                      const uint64_t offset,
			                                                      const uint64_t range);


protected:
	// Simple Line segments_
	std::vector<Line> mLines; //!< used to render the lines of the Octree. \todo rename and refactor.

	// Projection:
	const float*       mMatModelView = nullptr;   //!< Pointer to float[16] matrix in column-major format with the model view.
	const float*       mMatProjection = nullptr;  //!< Pointer to float[16] matrix in column-major format for projection.
	MeshWidgetParams*  mWidgetParams = nullptr;   //!< reference to settings of the meshwidget, which hold properties used for visualization e.g. light properties.
	MeshGLColors*      mRenderColors = nullptr;   //!< reference to settings of the meshwidget, which holds the colors used for rendering.
	const QGLContext*  mOpenGLContext = nullptr;  //!< reference to the (parents) widget's OpenGL context -- see QGLWidget::context()
};

#endif
