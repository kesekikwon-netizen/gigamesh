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
//

#include "meshGLShader.h"

#include <iostream> // cerr

#include <QColor>
#include <QtOpenGL/QGLWidget>
#include <QMessageBox>
#include <QIcon>

#include "../meshwidget_params.h"

#include "glmacros.h"
#include "../QGMMacros.h"

#include "ShaderManager.h"

using namespace std;

// Vertex Array Object related -- see initializeGL()
// ----------------------------------------------------
using PglGenVertexArrays = void (*)(GLsizei, GLuint *);
using PglBindVertexArray = void (*)(GLuint);
using PglGenFramebuffers = void (*)(GLsizei, GLuint *);
using PglBindFramebuffer = void (*)(GLenum, GLuint);
using PglDeleteFramebuffers = void (*)(GLsizei, GLuint *);
using PglDrawBuffers = void (*)(GLsizei, const GLenum *);
using PglClearBufferfv = void (*)(GLenum, GLint, const GLfloat *);
using PglBlendFunci = void (*)(GLuint, GLenum, GLenum);
using PglFramebufferTexture2D = void (*)(GLenum, GLenum, GLenum, GLuint, GLint);
using PglActiveTexture = void (*)(GLenum);

//! Constructor
MeshGLShader::MeshGLShader(
                QGLContext* rGLContext,
                const filesystem::path& rFileName,
                bool& rReadSuccess
) : MeshGL( rGLContext, rFileName, rReadSuccess ) {
	// Show limits for emitting elements using the geometry shader:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	GLint maxComp;
	glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &maxComp );
	cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_MAX_GEOMETRY_OUTPUT_COMPONENTS:       " << maxComp << endl;
	glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxComp );
	cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_MAX_GEOMETRY_OUTPUT_VERTICES:         " << maxComp << endl;
	glGetIntegerv( GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &maxComp );
	cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS: " << maxComp << endl;
	cout << "[MeshGLShader::" << __FUNCTION__ << "] ----------------------------------------------------------------------------------------------" << endl;

	//======================================================================================================================================================
	// LOAD texture maps
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	//mFuncValTexMapGL = QGLWidget::convertToGLFormat( mFuncValTexMapGL );
	if( !mFuncValTexMapGL.load( ":/GMShaders/funcvalmapsquare.png" ) ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load texture map for function values!" << endl;
	} else {
		cout << "[MeshGLShader::" << __FUNCTION__ << "] Texture map for function values loaded." << endl;
	}

	if( !mImageHatchlinesNPR[0].load(":/GMShaders/NPR/hatch_0-2.png")) {
            cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load texture map for hatchlines!" << endl;
        } else {
            cout << "[MeshGLShader::" << __FUNCTION__ << "] Texture map for hatchlines loaded." << endl;
        }


	if( !mImageHatchlinesNPR[1].load(":/GMShaders/NPR/hatch_3-5.png")) {
                cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load texture map for hatchlines!" << endl;
        } else {
            cout << "[MeshGLShader::" << __FUNCTION__ << "] Texture map for hatchlines loaded." << endl;
        }

	if( !mImageHatchlinesNPR[2].load(":/GMShaders/NPR/dots_0-2.png")) {
            cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load texture map for hatchlines!" << endl;
        } else {
            cout << "[MeshGLShader::" << __FUNCTION__ << "] Texture map for hatchlines loaded." << endl;
        }


	if( !mImageHatchlinesNPR[3].load(":/GMShaders/NPR/dots_3-5.png")) {
                cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load texture map for hatchlines!" << endl;
        } else {
            cout << "[MeshGLShader::" << __FUNCTION__ << "] Texture map for hatchlines loaded." << endl;
        }

	// Load cortex cross-hatch textures (Raczynski-Henk 2017 archaeological standards)
	if( !mImageCortexCrossHatch[0].load(":/GMShaders/NPR/cortex_cross_hatch_light.png")) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load cortex cross-hatch light texture!" << endl;
	} else {
		cout << "[MeshGLShader::" << __FUNCTION__ << "] Cortex cross-hatch light texture loaded." << endl;
	}

	if( !mImageCortexCrossHatch[1].load(":/GMShaders/NPR/cortex_cross_hatch_medium.png")) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load cortex cross-hatch medium texture!" << endl;
	} else {
		cout << "[MeshGLShader::" << __FUNCTION__ << "] Cortex cross-hatch medium texture loaded." << endl;
	}

	if( !mImageCortexCrossHatch[2].load(":/GMShaders/NPR/cortex_cross_hatch_dense.png")) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Could not load cortex cross-hatch dense texture!" << endl;
	} else {
		cout << "[MeshGLShader::" << __FUNCTION__ << "] Cortex cross-hatch dense texture loaded." << endl;
	}
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Generate the texture name(s): NON-Qt
	//glGenTextures( 1, mTextureMaps );
	//.
	// Bind the texture ID
	//glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] );
	//.
	// Generate the texture
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, mFuncValTexMapGL.width(),
	//              mFuncValTexMapGL.height(),
	//              0, GL_RGBA, GL_UNSIGNED_BYTE, mFuncValTexMapGL.bits() );
	//.
	// Texture parameters -> Set thru Qt mehtods!!
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	mTextureMaps[0] = new QOpenGLTexture( mFuncValTexMapGL.mirrored() );
	cout << "[MeshGLShader::" << __FUNCTION__ << "] Texture map for function created in OpenGL." << endl;
	mTextureMaps[0]->setMinificationFilter(  QOpenGLTexture::Nearest ); // NEAREST is IMPORTANT - hard lesson learned: MipMapInterpolation will result in colors mixed from neighbouring color ramps!
	mTextureMaps[0]->setMagnificationFilter( QOpenGLTexture::Nearest ); // NEAREST is IMPORTANT - hard lesson learned: MipMapInterpolation will result in colors mixed from neighbouring color ramps!
	mTextureMaps[0]->setWrapMode(QOpenGLTexture::ClampToEdge);
	cout << "[MeshGLShader::" << __FUNCTION__ << "] Texture filters set." << endl;


	for(int i = 0; i<4; ++i)
	{
		mTextureHatchlinesNPR[i] = new QOpenGLTexture(mImageHatchlinesNPR[i].mirrored());
		mTextureHatchlinesNPR[i]->setMinificationFilter(QOpenGLTexture::Linear);
		mTextureHatchlinesNPR[i]->setMagnificationFilter(QOpenGLTexture::Linear);
	}

	// Initialize cortex cross-hatch textures
	for(int i = 0; i < 3; ++i)
	{
		mTextureCortexCrossHatch[i] = new QOpenGLTexture(mImageCortexCrossHatch[i].mirrored());
		mTextureCortexCrossHatch[i]->setMinificationFilter(QOpenGLTexture::Linear);
		mTextureCortexCrossHatch[i]->setMagnificationFilter(QOpenGLTexture::Linear);
		mTextureCortexCrossHatch[i]->setWrapMode(QOpenGLTexture::Repeat); // Repeat for tiling
	}
	cout << "[MeshGLShader::" << __FUNCTION__ << "] Cortex cross-hatch textures initialized in OpenGL." << endl;

	mShaderManager = new ShaderManager;

	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	PRINT_OPENGL_ERROR( "glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );" );
	QGLFormat::OpenGLContextProfile currentProfile = mOpenGLContext->format().profile();
	if( currentProfile != QGLFormat::CoreProfile ) {
		glEnable( GL_POINT_SPRITE );  // <- deprecated with OpenGL 3.x BUT THANKS TO A BUG WE NEED it, when the Coreprofile is NOT used!
		PRINT_OPENGL_ERROR( "glEnable( GL_POINT_SPRITE );" );
	}
	//glEnable( GL_POINT_SMOOTH ); // <- deprecated with OpenGL 3.x -- solution: http://mmmovania.blogspot.de/2010/12/circular-point-sprites-in-opengl-33.html
	//PRINT_OPENGL_ERROR( "glEnable( GL_POINT_SMOOTH );" );
	//glPointSize( 6.0 );  // never put this between glBegin and glEnd !!!
	//PRINT_OPENGL_ERROR( "glPointSize( 6.0 );" ); // <- deprecated with OpenGL 3.x
	//glVertexPointer( 3, GL_FLOAT, sizeof(grVertexStripeElment), NULL );  // <- deprecated with OpenGL 3.x
	//PRINT_OPENGL_ERROR( "glVertexPointer( 3, GL_FLOAT, sizeof(grVertexStripeElment), NULL );" ); // <- deprecated with OpenGL 3.x

    //Variables for NPR Shading
    //mFboNPR = nullptr;
    //mSobelNPR = nullptr;
    mIsFboInitialized = false;

    //Needed for transparency
    mGL4_3Functions.initializeOpenGLFunctions();
    mTransAviableFragments = 0;
    mTransIsInitialized = 0;

    mFboOverlay = nullptr;
}

//! Destructor -- deletes mShader***Programs
MeshGLShader::~MeshGLShader() {
	delete mShaderManager;

	for(QOpenGLTexture*& textureHatchlineNPR : mTextureHatchlinesNPR)
	{
		if(textureHatchlineNPR != nullptr)
		{
			delete textureHatchlineNPR;
			textureHatchlineNPR = nullptr;
		}
	}

	// Cleanup cortex cross-hatch textures
	for(QOpenGLTexture*& textureCortex : mTextureCortexCrossHatch)
	{
		if(textureCortex != nullptr)
		{
			delete textureCortex;
			textureCortex = nullptr;
		}
	}

	mIsFboInitialized = false;

	if(mTransIsInitialized && (
			(mOpenGLContext->format().majorVersion() > 4) ||
			(mOpenGLContext->format().majorVersion() == 4 && mOpenGLContext->format().minorVersion() >= 3)))
	{

		mGL4_3Functions.glDeleteBuffers(static_cast<GLsizei>(mSSBOs.size()),mSSBOs.data());
		if(mTransIsInitialized == 2)
			mGL4_3Functions.glDeleteQueries(1,&mTransFragmentQuery);
		else if(mTransIsInitialized == 1 || mTransIsInitialized == 3)
			transparencyDeleteFBO();
	}

	//transparency with OpenGL 3.2 -> can only be weighted blend (method 1)
	else if(mTransIsInitialized)
	{
		transparencyDeleteFBO();
	}

	mTransIsInitialized = 0;

	if(mFboOverlay)
		delete mFboOverlay;

	delete mTextureMaps[0];

}

//==============================================================================================================================================================

//! Paints the Mesh and all other elements using OpenGL.
void MeshGLShader::glPaint() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	MeshGL::glPaint();

	//! ----------------------------------------------------------------------------------------
	//! *) Setup clipping planes.
	//! .) Use mesh plane as clipping plane, when requested.
	bool planeForClipping;
	getParamFlagMeshGL( SHOW_MESH_PLANE_AS_CLIPLANE, &planeForClipping );
	if( planeForClipping ) {
		glEnable( GL_CLIP_PLANE0 );
		PRINT_OPENGL_ERROR( "glEnable( GL_CLIP_PLANE0 )" );
		//cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_CLIP_PLANE0 ENable." << endl;
	} else {
		glDisable( GL_CLIP_PLANE0 );
		PRINT_OPENGL_ERROR( "glDisable( GL_CLIP_PLANE0 )" );
		//cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_CLIP_PLANE0 DISable." << endl;
	}

	//! .) Use the selected primitive and the camera plane for clipping.
	bool clipUsingSelection;
	getParamFlagMeshGL( SHOW_CLIP_THRU_SEL, &clipUsingSelection );
	if( ( mPrimSelected != nullptr ) && ( clipUsingSelection ) ) {
		glEnable( GL_CLIP_PLANE2 );
		PRINT_OPENGL_ERROR( "glEnable( GL_CLIP_PLANE2 )" );
		//cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_CLIP_PLANE2 ENable." << endl;
	} else {
		glDisable( GL_CLIP_PLANE2 );
		PRINT_OPENGL_ERROR( "glDisable( GL_CLIP_PLANE2 )" );
		//cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_CLIP_PLANE2 DISable." << endl;
	}

	//! .) DongArch clip preview (Phase 4: Clip interactive preview)
	// Real-time cross-section preview using OpenGL clipping plane
	// Enables mouse-wheel based interactive cutting plane adjustment
	bool dongArchClipPreviewEnabled = false;
	getParamFlagMeshGL( DONGARCH_CLIP_PREVIEW_ENABLED, &dongArchClipPreviewEnabled );
	if( dongArchClipPreviewEnabled ) {
		// Get clip plane parameters from MeshGL float parameters
		double clipX = 0.0, clipY = 0.0, clipZ = 1.0, clipW = 0.0;
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_X, &clipX );
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_Y, &clipY );
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_Z, &clipZ );
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_W, &clipW );

		// Set up OpenGL clipping plane (GL_CLIP_PLANE1)
		GLdouble clipPlaneEquation[4] = { clipX, clipY, clipZ, clipW };
		glClipPlane( GL_CLIP_PLANE1, clipPlaneEquation );
		glEnable( GL_CLIP_PLANE1 );
		PRINT_OPENGL_ERROR( "glEnable( GL_CLIP_PLANE1 ) - DongArch Clip Preview" );
		cout << "[MeshGLShader::" << __FUNCTION__ << "] GL_CLIP_PLANE1 Enable: ("
		     << clipX << ", " << clipY << ", " << clipZ << ", " << clipW << ")" << endl;
	} else {
		glDisable( GL_CLIP_PLANE1 );
		PRINT_OPENGL_ERROR( "glDisable( GL_CLIP_PLANE1 )" );
	}

	//! ----------------------------------------------------------------------------------------
	//! *) Faces of the Mesh, its plane etc.
	int shaderChoice;
	getParamIntMeshGL( MeshGLParams::SHADER_CHOICE, &shaderChoice );
	bool drawNPR = ( MeshGLParams::SHADER_NPR == shaderChoice );
	bool drawTransparency = MeshGLParams::SHADER_TRANSPARENCY == shaderChoice;

	//! \todo the release of the framebuffer can be moved to the cases down below.
	// Do not draw normally, if NPR is active.
	if( !drawNPR && !drawTransparency )  {
		// Free NPR buffers, if no longer needed. When released the flag mIsFboInitialized prevents accidentaly re-release.
		releaseFramebuffersNPR();
	}

	//display the mesh as a reduced pointcloud when moving the camera
	bool showMeshReduced;
	mWidgetParams->getParamFlagMeshWidget(MeshWidgetParams::SHOW_MESH_REDUCED, &showMeshReduced);
	if(showMeshReduced) {
		shaderChoice = SHADER_POINTCLOUD;
	}

	switch( shaderChoice ) {
		case SHADER_POINTCLOUD:
		{
			eTexMapType texMapChoice;
			getParamIntMeshGL( TEXMAP_CHOICE_FACES, reinterpret_cast<int*>(&texMapChoice) );
			vboPaintPointcloud(texMapChoice, showMeshReduced);
			break;
		}
		case SHADER_MONOLITHIC:
			//! ----------------------------------------------------------------------------------------
			//! *) Monolithic shader with all the functionality
			vboPaintFuncVal();
			break;
		case SHADER_TRANSPARENCY:
			// Do nothing here, as the corresponding function will be called later
			// after drawing the grid otherwise it won't be shown.
			// When the shader is executed here the scene histogram will work, which
			// shows nothing for the current choice of execution order.
			// ........................................................................
			// See the call of glPaintTransparent(); in MeshWidget::paintEvent
			break;
		case SHADER_WIREFRAME:
			//! ----------------------------------------------------------------------------------------
			//! *) Wireframe of vboPaintFaces();
			vboPaintWireframe();
			break;
		case SHADER_NPR:
			//! ----------------------------------------------------------------------------------------
			//! *) NPR-Version of vboPaintFaces();
			vboPaintNPR();
			break;
		case SHADER_TEXTURED:
			//! ----------------------------------------------------------------------------------------
			//! *) textured-Version of vboPaintFaces();
			vboPaintTextured();
			break;
		default:
			cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Unknown shader choice " << shaderChoice << "!" << endl;
			break;
	}

	//! ----------------------------------------------------------------------------------------
	//! *) Faces of datum objects
	vboPaintFaces();

	//! ----------------------------------------------------------------------------------------
	//! *) Vertices.
	vboPaintVertices();

	//! ----------------------------------------------------------------------------------------
	//! *) Polygonal an other lines.
	vboPaintPolylines();

	//! ----------------------------------------------------------------------------------------
	//! *) Paint the bounding box of the Mesh.
	vboPaintBoundingBox();

	//! 3.) Draw other stuff:

	//! *) Polygonal lines - normals per label.
	//! \todo this BROKEN and has to be implemented within the shader.
	uint64_t labelNrs;
	bool mainNormalPolyLine;
	getParamFlagMeshGL( SHOW_NORMALS_POLYLINE_MAIN, &mainNormalPolyLine );
	if( false && mainNormalPolyLine && ( mPolyLines.size() > 0 ) && ( labelCount( Primitive::IS_POLYLINE, labelNrs ) ) ) {
		//cout << "[MeshGL::paintGL] SHOW_NORMALS_POLYLINE_MAIN" << endl;
		GLubyte* hsvMap; // = getColorMapBrewerPaired( labelNrs );
		glLineWidth( 4.0 );

		vector<PolyLine*>::iterator itPoly;
		for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
			uint64_t currentLabel;
			if( (*itPoly)->getLabel( currentLabel ) ) {
				glColor3ubv( &hsvMap[currentLabel*3] );
			} else {
				glColor4ub( 0, 170, 255, 255 );
			}
			glPointSize( 12.0 );  // never put this between glBegin and glEnd !!!
			Vector3D polyCOG    = (*itPoly)->getCenterOfGravity();
			Vector3D polyNormal = (*itPoly)->getNormal( false );
			glBegin( GL_POINTS );
			glVertex3f( polyCOG.getX(), polyCOG.getY(), polyCOG.getZ() );
			glEnd();
			glBegin( GL_LINES );
			glVertex3f( polyCOG.getX(), polyCOG.getY(), polyCOG.getZ() );
			glVertex3f( polyCOG.getX()+polyNormal.getX(), polyCOG.getY()+polyNormal.getY(), polyCOG.getZ()+polyNormal.getZ() );
			glEnd();
		}
		delete hsvMap;
	}

	MeshWidgetParams::eSelectionModes selectionMode;
	mWidgetParams->getParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, reinterpret_cast<int*>(&selectionMode) );
	//! *) Mesh cone \todo source revision.
	if( selectionMode == MeshWidgetParams::SELECTION_MODE_CONE ) {
		Vector3D coneAxisTop;
		Vector3D coneAxisBot;
		double coneUpperRadius;
		double coneLowerRadius;

		if( this->getConeStatus() == CONE_DEFINED_LOWER_RADIUS ) {
			if( getConeRadii( &coneUpperRadius, &coneLowerRadius ) && getConeAxis( &coneAxisTop, &coneAxisBot ) ) {
				drawTruncatedCone( coneAxisTop, coneAxisBot, coneUpperRadius, coneLowerRadius );
			}
		}
	}
}


void MeshGLShader::glPaintTransparent()
{
	bool showMeshReduced;
	mWidgetParams->getParamFlagMeshWidget(MeshWidgetParams::SHOW_MESH_REDUCED, &showMeshReduced);

	if(showMeshReduced)
		return;

	MeshGL::glPaint();

	int drawTransparency;
	getParamIntMeshGL(MeshGLParams::SHADER_CHOICE, &drawTransparency);
	//! *) Transparency-Version of vboPaintFaces();

	if((mOpenGLContext->format().majorVersion() >= 4) ||
	(mOpenGLContext->format().majorVersion() == 4 && mOpenGLContext->format().minorVersion() >= 3))
	{
		if(drawTransparency == MeshGLParams::SHADER_TRANSPARENCY)
		{
			int bufferMethod;
			getParamIntMeshGL(TRANSPARENCY_BUFFER_METHOD, &bufferMethod);

			if(mTransIsInitialized != (bufferMethod + 1) && mTransIsInitialized != 0)
			{
				mGL4_3Functions.glDeleteBuffers(static_cast<GLsizei>(mSSBOs.size()),mSSBOs.data());


				if(mTransIsInitialized == 2)
					mGL4_3Functions.glDeleteQueries(1,&mTransFragmentQuery);

				if(mTransIsInitialized == 1 || mTransIsInitialized == 3)
					transparencyDeleteFBO();

				mTransIsInitialized = 0;
			}

			if(bufferMethod == 0)
			{
				//vboPaintTransparencyKPBuffer();
				vboPaintTransparencyWAVG();
			}
			else if(bufferMethod == 1)
			{
				vboPaintTransparencyABuffer();
			}
			else if(bufferMethod == 2)
			{
				//vboPaintTransparencyWAVG();
				vboPaintTransparencyALBuffer();
			}
		}
		else
		{

			//free transparency buffers, if no longer needed
			if(mTransIsInitialized)
			{
				mGL4_3Functions.glDeleteBuffers(static_cast<GLsizei>(mSSBOs.size()),mSSBOs.data());

				if(mTransIsInitialized == 2)
					mGL4_3Functions.glDeleteQueries(1,&mTransFragmentQuery);

				if(mTransIsInitialized == 1 || mTransIsInitialized == 3)
					transparencyDeleteFBO();

				mTransIsInitialized = 0;
			}
		}
	}
	//no OpenGl4.3 -> only weighted average blending (OpenGL3.2)
	else
	{
		if(drawTransparency)
		{
			vboPaintTransparencyWAVG();
		}
		else
		{
			if(mTransIsInitialized)
			{
				transparencyDeleteFBO();
				mTransIsInitialized = 0;
			}
		}
	}

}

void MeshGLShader::glPaintOverlay()
{
	bool param;

	getParamFlagMeshGL(SHOW_BADLIT_AREAS, &param);

	if(param)
	{
		vboPaintLightingOverlay();
	}
}

//! Fills the depth buffer according to the given transformation and also sets the fragment's color components to its depth
//! @param rTransformMat transformation matrix specifiyint the transformation
void MeshGLShader::glPaintDepth( const QMatrix4x4 &rTransformMat ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	const auto mShaderDepth = mShaderManager->getShader(ShaderManager::ShaderName::DEPTH);

	if(mShaderDepth == nullptr)
		return;

	MeshGL::glPaintDepth( rTransformMat );

	glClear(GL_DEPTH_BUFFER_BIT);

	if( !mShaderDepth->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	} else {
		mShaderDepth->setUniformValue( "transformMat", rTransformMat);

		PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
		glBindVertexArray( mVAO );
		PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );
		shaderSetLocationBasicAttribs(mShaderDepth, VBUFF_VERTICES_STRIPED, static_cast<unsigned>(sizeof(grVertexStripeElment)));

		mVertBufObjs[VBUFF_FACES]->bind();
		glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );
		mVertBufObjs[VBUFF_FACES]->release();
	}
}

//! Paints per vertex light intensities (obtained by simulating frontal lighting) to individual pixels
//! @param rTransformMat transformation matrix that is applied before simulating frontal lighting
//! @param rXResolution horizontal resolution of the current framebuffer
//! @param rYResolution vertical resolution of the current framebuffer
//! @param rDepthTexture 2D texture representing the depth buffer that will be interpreted as corresponding to the given transformation
//! @param rZTolerance tolerance used tolerance that is used during occlusion checks with rDepthTexture. The values of rDepthTexture will be offset by this tolerance
//! @param rFirstVertexIdx light intensities will be calculated and painted for rXResolution * rYResolution vertices beginning at the vertex with index rFirstVertIdx
void MeshGLShader::glPaintFrontalLightPerVertex( const QMatrix4x4 &rTransformMat,
										   GLint rXResolution,
										   GLint rYResolution,
										   GLuint rDepthTexture,
										   GLfloat rZTolerance,
										   GLint rFirstVertIdx) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif

	const auto mShaderFrontalLightPerVertex = mShaderManager->getShader(ShaderManager::ShaderName::FRONTAL_LIGHT_PER_VERTEX);
	if(mShaderFrontalLightPerVertex == nullptr)
		return;


	MeshGL::glPaintFrontalLightPerVertex( rTransformMat, rXResolution, rYResolution, rDepthTexture, rZTolerance, rFirstVertIdx );

	if( !mShaderFrontalLightPerVertex->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	} else {
		mShaderFrontalLightPerVertex->setUniformValue( "transformMat", rTransformMat );
		mShaderFrontalLightPerVertex->setUniformValue( "xResolution", rXResolution);
		mShaderFrontalLightPerVertex->setUniformValue( "yResolution", rYResolution);
		mShaderFrontalLightPerVertex->setUniformValue( "zTolerance", rZTolerance);
		mShaderFrontalLightPerVertex->setUniformValue( "vertIDOffset", rFirstVertIdx);

		glBindTexture( GL_TEXTURE_2D, rDepthTexture);

		PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
		glBindVertexArray( mVAO );
		PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

		shaderSetLocationBasicAttribs(mShaderFrontalLightPerVertex, VBUFF_VERTICES_STRIPED, static_cast<unsigned>(sizeof(grVertexStripeElment)));

		mVertBufObjs[VBUFF_VERTICES_STRIPED]->bind();
		glDrawArrays( GL_POINTS, rFirstVertIdx, std::min(static_cast<GLsizei>(rXResolution * rYResolution),
		                                                 static_cast<GLsizei>(mVertBufObjs[VBUFF_VERTICES_STRIPED]->size() / sizeof(grVertexStripeElment) - rFirstVertIdx)) );
		PRINT_OPENGL_ERROR( "glDrawArrays( GL_POINTS, rFirstVertex, min((long unsigned int) rXResolution * rYResolution, mVertBufObjs[VBUFF_VERTICES_STRIPED]->size() / sizeof(grVertexStripeElment) - rFirstVertIdx) );" );
		mVertBufObjs[VBUFF_VERTICES_STRIPED]->release();
	}
}

//==============================================================================================================================================================

//! Set a shader's basic uniforms: the modelview and the projection matrix.
void MeshGLShader::shaderSetLocationBasicMatrices( QOpenGLShaderProgram* rShaderProgram ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	// Prepare the modelview and projection matrices
	QMatrix4x4 pmvMatrix( mMatModelView[0], mMatModelView[4], mMatModelView[8],  mMatModelView[12],
						  mMatModelView[1], mMatModelView[5], mMatModelView[9],  mMatModelView[13],
						  mMatModelView[2], mMatModelView[6], mMatModelView[10], mMatModelView[14],
						  mMatModelView[3], mMatModelView[7], mMatModelView[11], mMatModelView[15] );
	QMatrix4x4 ppvMatrix( mMatProjection[0], mMatProjection[4], mMatProjection[8],  mMatProjection[12],
						  mMatProjection[1], mMatProjection[5], mMatProjection[9],  mMatProjection[13],
						  mMatProjection[2], mMatProjection[6], mMatProjection[10], mMatProjection[14],
						  mMatProjection[3], mMatProjection[7], mMatProjection[11], mMatProjection[15] );
	// set matrices
	rShaderProgram->setUniformValue( "modelview", pmvMatrix );
	PRINT_OPENGL_ERROR( "SHADER operations!" );
	rShaderProgram->setUniformValue( "projection", ppvMatrix );
	PRINT_OPENGL_ERROR( "SHADER operations!" );
}

//! Set a shader's basic attributes.
void MeshGLShader::shaderSetLocationBasicAttribs( QOpenGLShaderProgram* rShaderProgram, eVertBufObjs rBufferId, int rStride ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	// Sanity checks:
	if( mVertBufObjs[rBufferId] == nullptr ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return;
	}
	if( !(mVertBufObjs[rBufferId]->isCreated()) ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: Buffer not created!" << endl;
		return;
	}

	// Strided Data -- map buffer
	mVertBufObjs[rBufferId]->bind();
	PRINT_OPENGL_ERROR( "mVertBufObjs[rBufferId]->bind();" );
	// Strided data -- first there floats are the position vectors.
	rShaderProgram->setAttributeBuffer( "position", GL_FLOAT, 0, 3, rStride ); // rShaderLocationBasic->mVertexPos
	PRINT_OPENGL_ERROR( "SHADER operations!" );
	rShaderProgram->enableAttributeArray( "position" ); // rShaderLocationBasic->mVertexPos
	PRINT_OPENGL_ERROR( "SHADER operations!" );
	// Strided data -- second set of there floats are the normal vectors.
	size_t offSetNormals = sizeof(GLfloat)*3;
	rShaderProgram->setAttributeBuffer( "vNormal", GL_FLOAT, static_cast<int>(offSetNormals), 3, rStride ); // rShaderLocationBasic->mVertexNormal
	PRINT_OPENGL_ERROR( "SHADER operations!" );
	rShaderProgram->enableAttributeArray( "vNormal" ); // rShaderLocationBasic->mVertexNormal
	PRINT_OPENGL_ERROR( "SHADER operations!" );

	size_t offSetColors = offSetNormals + sizeof(GLfloat)*3;
	rShaderProgram->setAttributeBuffer( "vColor", GL_UNSIGNED_BYTE, static_cast<int>(offSetColors), 4, rStride );
	rShaderProgram->enableAttributeArray( "vColor" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFuncVal = offSetColors+sizeof(GLubyte)*4;
	rShaderProgram->setAttributeBuffer( "vFuncVal", GL_FLOAT, static_cast<int>(offSetFuncVal), 1, rStride );
	rShaderProgram->enableAttributeArray( "vFuncVal" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
}

//! Set a shaders basic uniforms and attributes for faces.
void MeshGLShader::shaderSetLocationBasicFaces( QOpenGLShaderProgram* rShaderProgram ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	// Backfaces - culling and color
	bool boolParam;
	getParamFlagMeshGL( MeshGLParams::SHOW_FACES_CULLED, &boolParam );
	rShaderProgram->setUniformValue( "backCulling", boolParam );

	getParamFlagMeshGL( MeshGLParams::BACKFACE_LIGHTING, &boolParam );
	rShaderProgram->setUniformValue( "backLighting", boolParam);

	PRINT_OPENGL_ERROR( "SHADER operations!" );
	GLfloat colorBackface[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_BACKFACE, colorBackface );
	rShaderProgram->setUniformValue( "colorSolidBack", colorBackface[0], colorBackface[1], colorBackface[2], colorBackface[3] );
	PRINT_OPENGL_ERROR( "SHADER operations!" );

	// Flat or smooth shading?
	bool smoothShade;
	getParamFlagMeshGL( MeshGLParams::SHOW_SMOOTH, &smoothShade );
	rShaderProgram->setUniformValue( "flatShade", not( smoothShade ) ); // rShaderLocationBasic->mFlatShade
	PRINT_OPENGL_ERROR( "SHADER operations!" );
}

//! Set a shaders basic uniforms regarding lighting.
void MeshGLShader::shaderSetLocationBasicLight( QOpenGLShaderProgram* rShaderProgram ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	// Lighting turned on?
	bool lightingSet;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_ENABLED, &lightingSet );
	rShaderProgram->setUniformValue( "uLightEnabled", static_cast<GLboolean>(lightingSet) );
	if( !lightingSet ) {
		rShaderProgram->setUniformValue( "uLightVectors", 0 );
		return;
	}

	// Material related;
	double materialShininess;
	mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::MATERIAL_SHININESS, &materialShininess );
	rShaderProgram->setUniformValue( "Shininess", static_cast<GLfloat>(materialShininess) );
	double materialSpecular;
	mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::MATERIAL_SPECULAR, &materialSpecular );

	// Material AND light -- directional -- fixed to camera
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QVector4D diffuseFixCam( 1.0, 1.0, 1.0, 1.0 );
	bool lightFixedCam;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM, &lightFixedCam );
	if( lightFixedCam ) {
		double lightFixedCamPhi;
		double lightFixedCamTheta;
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM_ANGLE_PHI,   &lightFixedCamPhi   );
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM_ANGLE_THETA, &lightFixedCamTheta );
		Vector3D lightDirCamAngles( lightFixedCamPhi, lightFixedCamTheta, false );
		QVector3D lightDirCam( lightDirCamAngles.getX(), lightDirCamAngles.getY(), lightDirCamAngles.getZ() );
		rShaderProgram->setUniformValue( "uLightDirectionFixedCamera", lightDirCam );
	} else {
		diffuseFixCam = QVector4D( 0.0f, 0.0f, 0.0f, 0.0f );
		rShaderProgram->setUniformValue( "uLightDirectionFixedCamera", QVector3D( 0.0, 0.0, 0.0 ) );
	}
	double lightFixedCamBright = 0.0f;
	mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM_INTENSITY, &lightFixedCamBright );
	diffuseFixCam *= lightFixedCamBright;
	diffuseFixCam.setW( 1.0f );
	rShaderProgram->setUniformValue( "FixedCam_DiffuseProduct", diffuseFixCam );
	QVector4D specularFixCam( diffuseFixCam.toVector3D() );
	specularFixCam *= materialSpecular;
	specularFixCam.setW( 1.0f );
	rShaderProgram->setUniformValue( "FixedCam_SpecularProduct", specularFixCam );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	// Material AND light -- directional -- fixed to world/object
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	QVector4D diffuseFixWorld( 1.0, 1.0, 1.0, 1.0 );

	// Check for archaeological illustration mode first (takes priority)
	bool archaeologyMode = false;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::ARCHAEOLOGY_MODE_ENABLED, &archaeologyMode );

	bool lightFixedWorld;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD, &lightFixedWorld );

	double lightFixedWorldBright = 0.0f;
	if( archaeologyMode ) {
		// Fixed 45° upper-left lighting (Raczynski-Henk 2017 standards)
		// Archaeological mode overrides user lighting settings
		QVector3D lightDirArchaeology( -0.707f, 0.707f, 0.0f );
		rShaderProgram->setUniformValue( "uLightDirectionFixedWorld", lightDirArchaeology );
		lightFixedWorldBright = 1.0; // Full brightness for archaeological mode
	} else if( lightFixedWorld ) {
		// User-controlled lighting direction
		double lightFixedWorldPhi;
		double lightFixedWorldTheta;
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD_ANGLE_PHI,   &lightFixedWorldPhi   );
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD_ANGLE_THETA, &lightFixedWorldTheta );
		Vector3D lightDirWorldAngles( lightFixedWorldPhi, lightFixedWorldTheta, false );
		QVector3D lightDirWorld( lightDirWorldAngles.getX(), lightDirWorldAngles.getY(), lightDirWorldAngles.getZ() );
		rShaderProgram->setUniformValue( "uLightDirectionFixedWorld", lightDirWorld );
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD_INTENSITY, &lightFixedWorldBright );
	} else {
		diffuseFixWorld = QVector4D( 0.0f, 0.0f, 0.0f, 0.0f );
		rShaderProgram->setUniformValue( "uLightDirectionFixedWorld", QVector3D( 0.0, 0.0, 0.0 ) );
	}
	diffuseFixWorld *= lightFixedWorldBright;
	diffuseFixWorld.setW( 1.0f );
	rShaderProgram->setUniformValue( "FixedWorld_DiffuseProduct", diffuseFixWorld );
	QVector4D specularFixWorld( diffuseFixWorld.toVector3D() );
	specularFixWorld *= materialSpecular;
	specularFixWorld.setW( 1.0f );
	rShaderProgram->setUniformValue( "FixedWorld_SpecularProduct", specularFixWorld );
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	// Light -- ambient
	bool lightAmbient;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_AMBIENT, &lightAmbient );
	QVector4D ambient( 1.0, 1.0, 1.0, 1.0 ); // Ambient at 100% and all other lights off is equal to rendering without lights.
	if( lightAmbient ) { // Ambient on => fetch brightness.
		double lightAmbientAmount;
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::AMBIENT_LIGHT, &lightAmbientAmount );
		ambient *= lightAmbientAmount;
	} else { // Ambient off => ignore brightness.
		ambient *= 0.0f;
	}
	ambient.setW( 1.0f );
	rShaderProgram->setUniformValue( "AmbientProduct", ambient );

	// Light - show directions
	MeshWidgetParams::eMouseModes currMouseMode;
	mWidgetParams->getParamIntegerMeshWidget( MeshWidgetParams::MOUSE_MODE, reinterpret_cast<int*>(&currMouseMode) );

	int lightVectors = currMouseMode == MeshWidgetParams::MOUSE_MODE_MOVE_LIGHT_FIXED_CAM    ? 1 :
	                   currMouseMode == MeshWidgetParams::MOUSE_MODE_MOVE_LIGHT_FIXED_OBJECT ? 2 :
	                   0;

	rShaderProgram->setUniformValue( "uLightVectors", lightVectors );

	double lightVectorLength = 0.0;
	getParamFloatMeshGL( MeshGLParams::LIGHTVECTOR_LENGTH, &lightVectorLength);
	rShaderProgram->setUniformValue("uLightVeclLength", static_cast<GLfloat>(lightVectorLength));

	// Limit number of directional vectors shown.
	int maxLightVecs; // 5000 seems to be a good choice.
	mWidgetParams->getParamIntegerMeshWidget( MeshWidgetParams::LIGHT_VECTORS_SHOWN_MAX, &maxLightVecs );
	int lightVecModVal = 0;
	if( maxLightVecs > 0 ) { // otherwise we get bad values or a division by zero.
		lightVecModVal = static_cast<int>(getFaceNr()) / maxLightVecs;
	}
	rShaderProgram->setUniformValue( "uLightVecModVal", lightVecModVal );

	// DongArch v4 Phase 4: Set clip plane uniforms for shader-based clipping
	bool dongArchClipPreviewEnabled = false;
	getParamFlagMeshGL( DONGARCH_CLIP_PREVIEW_ENABLED, &dongArchClipPreviewEnabled );
	rShaderProgram->setUniformValue( "uDongArchClipEnabled", static_cast<GLboolean>(dongArchClipPreviewEnabled) );

	if( dongArchClipPreviewEnabled ) {
		// Get clip plane parameters
		double clipX = 0.0, clipY = 0.0, clipZ = 1.0, clipW = 0.0;
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_X, &clipX );
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_Y, &clipY );
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_Z, &clipZ );
		getParamFloatMeshGL( MeshGLParams::DONGARCH_CLIP_PLANE_W, &clipW );

		QVector4D clipPlane( clipX, clipY, clipZ, clipW );
		rShaderProgram->setUniformValue( "uDongArchClipPlane", clipPlane );

		cout << "[MeshGLShader::setShaderLightDir] Clip plane uniform: ("
		     << clipX << ", " << clipY << ", " << clipZ << ", " << clipW << ")" << endl;
	}

	PRINT_OPENGL_ERROR( "SHADER operations!" );
}

//! Set a shaders basic uniforms regarding fog.
//! Attention: contains hard-coded parameters.
void MeshGLShader::shaderSetLocationBasicFog( QOpenGLShaderProgram* rShaderProgram ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	// Fog present?
	bool fogPresent;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::SHOW_FOG, &fogPresent );
	rShaderProgram->setUniformValue( "fogPresent", fogPresent );
	if( !fogPresent ) {
		return;
	}
	// Fog color.
	rShaderProgram->setUniformValue( "fogParams.vFogColor", QColor( 178, 178, 178, 255 ) );
	// Linear fog parameters:
	double fogLinearStart;
	mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::FOG_LINEAR_START, &fogLinearStart );
	rShaderProgram->setUniformValue( "fogParams.fStart", static_cast<GLfloat>(fogLinearStart) );
	double fogLinearEnd;
	mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::FOG_LINEAR_END, &fogLinearEnd );
	rShaderProgram->setUniformValue( "fogParams.fEnd", static_cast<GLfloat>(fogLinearEnd) );
	// Fog parameter for exp and exp2 functions:
	rShaderProgram->setUniformValue( "fogParams.fDensity", 0.005f );
	// Fog function:
	rShaderProgram->setUniformValue( "fogParams.iEquation", GLSL_FOG_EQUATION_LINEAR );
	PRINT_OPENGL_ERROR( "SHADER operations!" );
}

//! Set a shaders basic uniforms regarding vertex SPRITES and NORMALS.
void MeshGLShader::shaderSetLocationVertexSprites( QOpenGLShaderProgram* rShaderProgram ) {
	// Set the mesh plane as clipping plane:
	double clipPlane[4];
	getPlaneHNF( clipPlane );
	rShaderProgram->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		rShaderProgram->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
	}

	// Material - solid color:
	GLfloat colorSolidMesh[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorSolidMesh );
	rShaderProgram->setUniformValue( "colorSolid", colorSolidMesh[0], colorSolidMesh[1], colorSolidMesh[2], colorSolidMesh[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Bind the 2D texture used for colorramps and labels!
	//glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] );
	//PRINT_OPENGL_ERROR( "glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] )" );
	mTextureMaps[0]->bind();
	GLint texId;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &texId);
	texId -= GL_TEXTURE0;
	PRINT_OPENGL_ERROR( "binding mTextureMaps[0]" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] texId: " << texId << endl;
	// Set the ID of the texture map:
	rShaderProgram->setUniformValue( "uLabelTexMap", texId );
	rShaderProgram->setUniformValue( "uFuncValTexMap", texId );
	PRINT_OPENGL_ERROR( "Shader setAttributeValue" );

	// Set values for rendering of the labels:
	int labelShift;
	getParamIntMeshGL( MeshGLParams::COLMAP_LABEL_OFFSET, &labelShift );
	rShaderProgram->setUniformValue( "uLabelCountOffset", static_cast<GLfloat>(labelShift) );
	bool showLabelsMonoColor;
	getParamFlagMeshGL( MeshGLParams::SHOW_LABELS_MONO_COLOR, &showLabelsMonoColor );
	rShaderProgram->setUniformValue( "uLabelSameColor", showLabelsMonoColor );
	GLfloat somemColorSetting[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_SOLID, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelSingleColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BORDER, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelBorderColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_NOT_ASSIGNED, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelNoColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BACKGROUND, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelBackgroundColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Set parameters of the color mapping.
	int colMapID = GLSL_COLMAP_GRAYSCALE;
	getParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, &colMapID );
	rShaderProgram->setUniformValue( "uFuncValColorMap", static_cast<GLfloat>(colMapID) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Set the colored range.
	double funcValMin;
	double funcValMax;
	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	rShaderProgram->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(funcValMin) );
	rShaderProgram->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(funcValMax) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] funcValMin: " << funcValMin << "  funcValMax: " << funcValMax << endl;
	// Inversion of the colormap
	bool funcValInvertMap = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_COLMAP_INVERT, &funcValInvertMap );
	rShaderProgram->setUniformValue( "uFuncValInvert", static_cast<GLboolean>(funcValInvertMap) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Repeat colormap
	bool funcValRepeat = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &funcValRepeat );
	rShaderProgram->setUniformValue( "uFuncValRepeat", static_cast<GLboolean>(funcValRepeat) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	double funcValRepeatIntervall;
	getParamFloatMeshGL( MeshGLParams::WAVES_COLMAP_LEN, &funcValRepeatIntervall );
	rShaderProgram->setUniformValue( "uFuncValIntervall", static_cast<GLfloat>(funcValRepeatIntervall) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Strided Data -- map buffer
	//----------------------------------
	mVertBufObjs[VBUFF_VERTICES_STRIPED]->bind();
	PRINT_OPENGL_ERROR( "mVertBufObjs[VBUFF_VERTICES_STRIPED].bind();" );
	// Strided data -- one function value per vertex.
	size_t offSetColors = sizeof(GLfloat)*6;
	rShaderProgram->setAttributeBuffer( "vColor", GL_UNSIGNED_BYTE, static_cast<int>(offSetColors), 4, static_cast<int>(sizeof(grVertexStripeElment)) );
	rShaderProgram->enableAttributeArray( "vColor" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFuncVal = offSetColors+sizeof(GLubyte)*4;
	rShaderProgram->setAttributeBuffer( "vFuncVal", GL_FLOAT, static_cast<int>(offSetFuncVal), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	rShaderProgram->enableAttributeArray( "vFuncVal" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetLabelID = offSetFuncVal + sizeof(GLfloat);
	rShaderProgram->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	rShaderProgram->enableAttributeArray( "vLabelID" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
	rShaderProgram->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	rShaderProgram->enableAttributeArray( "vFlags" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
}

void MeshGLShader::shaderSetMeshBasicFuncVals(QOpenGLShaderProgram *rShaderProgram)
{
	shaderSetLocationBasicMatrices( rShaderProgram );
	shaderSetLocationBasicAttribs( rShaderProgram, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );
	shaderSetLocationBasicFaces( rShaderProgram );

	// More Strided Data -- map buffer
	//----------------------------------
	size_t offSetLabelID = sizeof(GLfloat)*6 + sizeof(GLubyte)*4 + sizeof(GLfloat);
	rShaderProgram->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	rShaderProgram->enableAttributeArray( "vLabelID" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
	rShaderProgram->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	rShaderProgram->enableAttributeArray( "vFlags" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );


	// Set the basic lighting:
	shaderSetLocationBasicLight( rShaderProgram );

	// Set the basic fog:
	shaderSetLocationBasicFog( rShaderProgram );
	PRINT_OPENGL_ERROR( "Setting the Basics" );

	double clipPlane[4];
	getPlaneHNF( clipPlane );
	rShaderProgram->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue: uClipPlane0" );
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		rShaderProgram->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
		PRINT_OPENGL_ERROR( "Shader setUniformValue: uClipBefore" );
	}

	GLfloat colorSolidStored[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorSolidStored );
	rShaderProgram->setUniformValue( "colorSolid", colorSolidStored[0], colorSolidStored[1], colorSolidStored[2], colorSolidStored[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue: colorSolid" );
	// Bind the 2D texture used for colorramps and labels!
	mTextureMaps[0]->bind();

	GLint texId;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &texId);
	texId -= GL_TEXTURE0;
    PRINT_OPENGL_ERROR( "binding mTextureMaps[0]" );

	rShaderProgram->setUniformValue( "uLabelTexMap", texId );
    PRINT_OPENGL_ERROR( "Shader setAttributeValue" );
	rShaderProgram->setUniformValue( "uFuncValTexMap", texId );
    PRINT_OPENGL_ERROR( "Shader setAttributeValue" );

    // Set values for rendering of the labels:
    int labelShift;
    getParamIntMeshGL( MeshGLParams::COLMAP_LABEL_OFFSET, &labelShift );
    rShaderProgram->setUniformValue( "uLabelCountOffset", static_cast<GLfloat>(labelShift) );
    bool showLabelsMonoColor;
    getParamFlagMeshGL( MeshGLParams::SHOW_LABELS_MONO_COLOR, &showLabelsMonoColor );
    rShaderProgram->setUniformValue( "uLabelSameColor", showLabelsMonoColor );
	GLfloat somemColorSetting[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_SOLID, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelSingleColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BORDER, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelBorderColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_NOT_ASSIGNED, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelNoColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BACKGROUND, somemColorSetting );
	rShaderProgram->setUniformValue( "uLabelBackgroundColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Set parameters of the color mapping.
	int colMapID = GLSL_COLMAP_GRAYSCALE;
	getParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, &colMapID );
	rShaderProgram->setUniformValue( "uFuncValColorMap", static_cast<GLfloat>(colMapID) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Set the colored range.
	double funcValMin;
	double funcValMax;
	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	rShaderProgram->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(funcValMin) );
	rShaderProgram->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(funcValMax) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] funcValMin: " << funcValMin << "  funcValMax: " << funcValMax << endl;
	// Inversion of the colormap
	bool funcValInvertMap = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_COLMAP_INVERT, &funcValInvertMap );
	rShaderProgram->setUniformValue( "uFuncValInvert", static_cast<GLboolean>(funcValInvertMap) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Repeat colormap
	bool funcValRepeat = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &funcValRepeat );
	rShaderProgram->setUniformValue( "uFuncValRepeat", static_cast<GLboolean>(funcValRepeat) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	double funcValRepeatIntervall;
	getParamFloatMeshGL( MeshGLParams::WAVES_COLMAP_LEN, &funcValRepeatIntervall );
	rShaderProgram->setUniformValue( "uFuncValIntervall", static_cast<GLfloat>(funcValRepeatIntervall) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Logarithmic color ramp
	double funcValLogGamma;
	getParamFloatMeshGL( MeshGLParams::FUNC_VALUE_LOG_GAMMA, &funcValLogGamma );
	rShaderProgram->setUniformValue( "uFuncValLogGamma", static_cast<GLfloat>(funcValLogGamma) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );



	//skipped: Isolines
	//skipped: Normals
	//skipped: Edge/Wireframes

	eTexMapType rRenderColor;
	getParamIntMeshGL( TEXMAP_CHOICE_FACES, reinterpret_cast<int*>(& rRenderColor));
	rShaderProgram->setUniformValue( "uRenderColor", rRenderColor );
}

//==============================================================================================================================================================

//! Overloaded from Mesh, this method will re-generate the buffer for the bounding box.
bool MeshGLShader::changedBoundingBox() {
	// Currently unused.
	return MeshGL::changedBoundingBox();
}

//==============================================================================================================================================================

//! Draw the bounding box of the mesh, when the related flag is set.
void MeshGLShader::vboPaintBoundingBox() {


	bool drawBoundingBox;
	getParamFlagMeshGL( SHOW_BOUNDING_BOX, &drawBoundingBox );
	bool drawBoundingBoxEnclosed;
	getParamFlagMeshGL( SHOW_BOUNDING_BOX_ENCLOSED, &drawBoundingBoxEnclosed );
	if( !( drawBoundingBox || drawBoundingBoxEnclosed ) ) {
		return;
	}

	const auto mShaderBoundingBox = mShaderManager->getShader(ShaderManager::ShaderName::BOUNDING_BOX);
	if(mShaderBoundingBox == nullptr)
		return;

	PRINT_OPENGL_ERROR( "OLD ERROR" );

	if( !mVertBufObjs[VBUFF_CUBE]->isCreated() ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: VBUFF_CUBE was not created!" << endl;
		return;
	}

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	if( !mShaderBoundingBox->bind() ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
		return;
	}

	// Set the basics:
	shaderSetLocationBasicMatrices( mShaderBoundingBox );
	shaderSetLocationBasicAttribs( mShaderBoundingBox, VBUFF_CUBE, static_cast<int>(sizeof(grVertexElmentBasic)) );

	// Set the fog, whem present:
	shaderSetLocationBasicFog( mShaderBoundingBox );


	Vector3D bbCenter = getBoundingBoxCenter();
	Vector3D bbSize;
	getBoundingBoxSize( bbSize );

	// Linewidth >1 causing problems with CoreProfile -- see for an alternative: http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter22.html
	//glLineWidth( 2.0f );
	//PRINT_OPENGL_ERROR( "glLineWidth( 2.0f );" );
	double lineWidth = 0.25;
	getParamFloatMeshGL( BOUNDING_BOX_LINEWIDTH, &lineWidth );

	// Fetch buffer size (used multiple times)
	int vboBuffSizeCube = mVertBufObjs[VBUFF_CUBE]->size()/static_cast<int>(sizeof(grVertexElmentBasic));

	// Force the bounding box to be in one color.
	mShaderBoundingBox->setUniformValue( "uColorSolidForce", true );

	// Just an enclosing box:
	if( drawBoundingBoxEnclosed ) {
		shaderSetLocationBasicLight( mShaderBoundingBox );
		mShaderBoundingBox->setUniformValue( "uColorSolid", 0.5f, 0.5f, 0.5f, 0.5f );
		mShaderBoundingBox->setUniformValue( "uStrech", bbSize.getX(),   bbSize.getY(),   bbSize.getZ()   );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX(), bbCenter.getY(), bbCenter.getZ() );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
	}

	if( drawBoundingBox ) {
		// Black lines for the frame
		mShaderBoundingBox->setUniformValue( "uColorSolid", 0.0f, 0.0f, 0.0f, 1.0f );

		// x-lines
		mShaderBoundingBox->setUniformValue( "uStrech", bbSize.getX()+lineWidth, lineWidth, lineWidth );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX(), bbCenter.getY()+bbSize.getY()/2.0, bbCenter.getZ()+bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX(), bbCenter.getY()+bbSize.getY()/2.0, bbCenter.getZ()-bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX(), bbCenter.getY()-bbSize.getY()/2.0, bbCenter.getZ()+bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX(), bbCenter.getY()-bbSize.getY()/2.0, bbCenter.getZ()-bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		// y-lines
		mShaderBoundingBox->setUniformValue( "uStrech", lineWidth, bbSize.getY()+lineWidth, lineWidth );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()+bbSize.getX()/2.0, bbCenter.getY(), bbCenter.getZ()+bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()+bbSize.getX()/2.0, bbCenter.getY(), bbCenter.getZ()-bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()-bbSize.getX()/2.0, bbCenter.getY(), bbCenter.getZ()+bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()-bbSize.getX()/2.0, bbCenter.getY(), bbCenter.getZ()-bbSize.getZ()/2.0 );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		// z-lines
		mShaderBoundingBox->setUniformValue( "uStrech", lineWidth, lineWidth, bbSize.getZ()+lineWidth );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()+bbSize.getX()/2.0, bbCenter.getY()+bbSize.getY()/2.0, bbCenter.getZ() );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()+bbSize.getX()/2.0, bbCenter.getY()-bbSize.getY()/2.0, bbCenter.getZ() );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()-bbSize.getX()/2.0, bbCenter.getY()+bbSize.getY()/2.0, bbCenter.getZ() );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
		mShaderBoundingBox->setUniformValue( "uOffSet", bbCenter.getX()-bbSize.getX()/2.0, bbCenter.getY()-bbSize.getY()/2.0, bbCenter.getZ() );
		glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );
	}

	PRINT_OPENGL_ERROR( "glDrawArrays( GL_TRIANGLES, 0, vboBuffSizeCube );" );
	mVertBufObjs[VBUFF_CUBE]->release();

	mShaderBoundingBox->release();

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );
}

//! Paints vertices (sprites)
void MeshGLShader::vboPaintVertices() {
	vector<grVertexStripeElment> singlePoints; // This should hold only a few - for large lists of segments a VBO may be more suitable (for performance!).
	vector<PinRenderer::PinVertex> pinPoints;
	// Plane/Triangle selection
	bool showMeshPlanePos;
	getParamFlagMeshGL( SHOW_MESH_PLANE_POSITIONS, &showMeshPlanePos );
	if( showMeshPlanePos) {
		double planePositions[9];
		if( getPlanePositions( planePositions ) ) {
			grVertexStripeElment somePoint;
			somePoint.mColor[0] = 227;
			somePoint.mColor[1] = 147;
			somePoint.mColor[2] =  44;
			somePoint.mColor[3] = 255;
			for( int i=0; i<3; i++ ) {
				somePoint.mPosition[0] = planePositions[i*3];
				somePoint.mPosition[1] = planePositions[i*3+1];
				somePoint.mPosition[2] = planePositions[i*3+2];
				singlePoints.push_back( somePoint );
			}
		}
	}
	// Sphere selection
	MeshWidgetParams::eSelectionModes selectionMode;
	mWidgetParams->getParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, reinterpret_cast<int*>(&selectionMode) );
	if( !isUnrolledAroundSphere() && ( selectionMode == MeshWidgetParams::SELECTION_MODE_SPHERE ) ) {
		double spherePoints[12]{0.0}; // points situated _on_ the sphere; filled
								 // with 0.0 if not available (this mimics
								 // the behaviour of the plane selection feature)

		double spherePointNormals[12]{0.0};	//the normals of the shere selection points

		//! \todo SHOW_MESH_SPHERE_POSITIONS remove or implement.

		if( getSphereData(spherePoints, spherePointNormals) ) {
			PinRenderer::PinVertex somePoint;
			somePoint.color[0] = 255;
			somePoint.color[1] =   0;
			somePoint.color[2] =   0;

			for( int i=0; i<4; i++ ) {

				somePoint.position[0] = spherePoints[i*3];
				somePoint.position[1] = spherePoints[i*3+1];
				somePoint.position[2] = spherePoints[i*3+2];

				somePoint.normal[0] = spherePointNormals[i*3  ];
				somePoint.normal[1] = spherePointNormals[i*3+1];
				somePoint.normal[2] = spherePointNormals[i*3+2];

				pinPoints.push_back(somePoint);
				//singlePoints.push_back( somePoint );
			}
		}
	}
	// Positions, selected
	vector<std::tuple<Vector3D, Primitive*, bool>> somePositions;
	if( getSelectedPosition( &somePositions ) ) {
		pinPoints.reserve(somePositions.size());
		PinRenderer::PinVertex somePoint{};
		for( auto const& currPos: somePositions ) {
			// From colorbewer: 9 classs blue, element 7: 33,113,181
			somePoint.color[0] =  33;
			somePoint.color[1] = 113;
			somePoint.color[2] = 181;
			if(std::get<2>(currPos))	//change color if it is an end-position
			{
				somePoint.color[0] = 190;
				somePoint.color[1] = 113;
				somePoint.color[2] = 181;
			}
			somePoint.position[0] = std::get<0>(currPos).getX();
			somePoint.position[1] = std::get<0>(currPos).getY();
			somePoint.position[2] = std::get<0>(currPos).getZ();

			somePoint.normal[0] = std::get<1>(currPos)->getNormalX();
			somePoint.normal[1] = std::get<1>(currPos)->getNormalY();
			somePoint.normal[2] = std::get<1>(currPos)->getNormalZ();
			pinPoints.push_back( somePoint );
		}
	}

	int shaderChoice;
	MeshGLParams::getParamIntMeshGL(MeshGLParams::SHADER_CHOICE, &shaderChoice);
	bool drawSinglePoints = !singlePoints.empty();

	bool drawVerticesAll;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_ALL, &drawVerticesAll );
	bool drawVerticesSolo;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SOLO, &drawVerticesSolo );
	drawVerticesSolo = drawVerticesSolo && shaderChoice != MeshGLParams::SHADER_POINTCLOUD;
	bool drawVerticesBorder;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_BORDER, &drawVerticesBorder );
	bool drawVerticesNonManifold;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_NON_MANIFOLD, &drawVerticesNonManifold );
	bool drawVerticesSingular;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SINGULAR, &drawVerticesSingular );
	bool drawVerticesFuncValMin;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_LOCAL_MIN, &drawVerticesFuncValMin );
	bool drawVerticesFuncValMax;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_LOCAL_MAX, &drawVerticesFuncValMax );
	bool drawVerticesSelected;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SELECTION, &drawVerticesSelected );
	bool drawVerticesSynthetic;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SYNTHETIC, &drawVerticesSynthetic );
	bool drawVerticesCortex;
	getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_CORTEX, &drawVerticesCortex );
	std::cout << "[MeshGLShader::paintVerticesAsSprites] drawVerticesCortex = " << drawVerticesCortex << std::endl;
	bool drawPrimVertSel;
	getParamFlagMeshGL( MeshGLParams::SHOW_SELECTION_SINGLE, &drawPrimVertSel );
	drawPrimVertSel &= ( mPrimSelected != nullptr ) && ( mPrimSelected->getType() == Primitive::IS_VERTEX );
	bool drawVerticesNormals;
	getParamFlagMeshGL( MeshGLParams::SHOW_NORMALS_VERTEX, &drawVerticesNormals );
	if( !( drawSinglePoints || drawVerticesAll || drawVerticesSolo || drawVerticesBorder || drawVerticesNonManifold || drawVerticesSingular ||
		   drawVerticesFuncValMin || drawVerticesFuncValMax ||
		   drawVerticesSelected || drawVerticesSynthetic || drawVerticesCortex || drawVerticesNormals || !pinPoints.empty() ) ) {
		return;
	}
	// Prepare, when required
	vboPrepareVerticesStriped();

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	if( drawVerticesNormals ) {
		const auto mShaderVertexNormals = mShaderManager->getShader(ShaderManager::ShaderName::VERTEX_NORMALS);
		if(mShaderVertexNormals != nullptr)
		{
			// Lets be shady :)
			if( !mShaderVertexNormals->bind() ) {
				cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
			}
			// Set the basics:
			shaderSetLocationBasicMatrices( mShaderVertexNormals );
			shaderSetLocationBasicAttribs( mShaderVertexNormals, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );
			// Set the basic lighting:
			// We let the ligths turned off os that the color per vertex, function values or labels are shown without shading effects.
			// The default ambient light (100%) is defined within the shader.
			// shaderSetLocationBasicLight( mShaderVertexNormals );
			// Set the basic fog:
			shaderSetLocationBasicFog( mShaderVertexNormals );
			// Set up common uniforms and attributes vertex sprites and normals:
			shaderSetLocationVertexSprites( mShaderVertexNormals );

			// Geometry shader setting:
			//----------------------------------
			double normalsLength;
			getParamFloatMeshGL( MeshGLParams::NORMALS_LENGTH, &normalsLength );
			mShaderVertexNormals->setUniformValue( "uNormalLength", static_cast<GLfloat>(normalsLength) );
			double normalsWdith;
			getParamFloatMeshGL( MeshGLParams::NORMALS_WIDTH, &normalsWdith );
			mShaderVertexNormals->setUniformValue( "uNormalWidth", static_cast<GLfloat>(normalsWdith) );
			PRINT_OPENGL_ERROR( "Shader setUniformValue" );

			// Switch rendering mode solid/color_per_vertex/function_value/labels:
			int renderColor;
			getParamIntMeshGL( TEXMAP_CHOICE_VERETX_SPRITES, &renderColor );
			mShaderVertexNormals->setUniformValue( "uRenderColor", renderColor );
			PRINT_OPENGL_ERROR( "Shader setUniformValue" );

			// Material - solid color:
			GLfloat colorTmpStored[4];
			mRenderColors->getColorSettings( MeshGLColors::COLOR_VERTEX_MONO, colorTmpStored );
			mShaderVertexNormals->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
			PRINT_OPENGL_ERROR( "Shader setUniformValue" );
			// Map and draw elements
			glDrawArrays( GL_POINTS, 0, mVertBufObjs[VBUFF_VERTICES_STRIPED]->size() );
			PRINT_OPENGL_ERROR( "glDrawArrays( GL_POINTS, 0, mVBOSizes[VBUFF_VERTICES_STRIPED] );" );

			// DONE WITH NORMALS
			mShaderVertexNormals->release();
			//cout << "[MeshGLShader::" << __FUNCTION__ << "] Vertex Normals!" << endl;
		}
	}

	const auto mShaderVertexSprites = mShaderManager->getShader(ShaderManager::ShaderName::VERTEX_SPRITES);
	if(mShaderVertexSprites == nullptr)
		return;

	// Lets be shady :)
	if( !mShaderVertexSprites->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}
	// Set the basics:
	shaderSetLocationBasicMatrices( mShaderVertexSprites );
	shaderSetLocationBasicAttribs( mShaderVertexSprites, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );
	// Set the basic lighting:
	// We let the ligths turned off os that the color per vertex, function values or labels are shown without shading effects.
	// The default ambient light (100%) is defined within the shader.
	// shaderSetLocationBasicLight( mShaderVertexSprites );
	// Set the basic fog:
	shaderSetLocationBasicFog( mShaderVertexSprites );
	// Set up common uniforms and attributes vertex sprites and normals:
	shaderSetLocationVertexSprites( mShaderVertexSprites );

	// Switch sprite shape
	int spriteShape;
	getParamIntMeshGL( VERTEX_SPRITE_SHAPE, &spriteShape );
	mShaderVertexSprites->setUniformValue( "uSpriteShape", spriteShape );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Map buffer and draw with increasing size
	//-----------------------------------------
	GLfloat minSpriteSize = 5.0;
	GLfloat spriteSizeInc = 9.0;
	GLfloat spriteShiftZ  = 0.0;

	if( drawVerticesAll ) {
		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		int renderColor;
		getParamIntMeshGL( TEXMAP_CHOICE_VERETX_SPRITES, &renderColor );
		mShaderVertexSprites->setUniformValue( "uRenderColor", renderColor );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		GLfloat colorTmpStored[4];
		mRenderColors->getColorSettings( MeshGLColors::COLOR_VERTEX_MONO, colorTmpStored );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		glDrawArrays( GL_POINTS, 0, mVertBufObjs[VBUFF_VERTICES_STRIPED]->size() );
		PRINT_OPENGL_ERROR( "glDrawArrays( GL_POINTS, 0, mVBOSizes[VBUFF_VERTICES_STRIPED] );" );
	}

	// Render color := solid color FOR ALL following veritces.
	mShaderVertexSprites->setUniformValue( "uRenderColor", TEXMAP_VERT_MONO );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	if( drawVerticesSolo ) {
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// NO INCREASE, because they do not overlap with the following vertices | minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		QColor colorSolid( 0, 128, 128, 255 );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_SOLO]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_SOLO]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_SOLO]->release();
	}

	if( drawVerticesBorder ) {
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		QColor colorSolid( 255, 0, 0, 255 );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_BORDER]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_BORDER]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_BORDER]->release();
	}

	if( drawVerticesNonManifold ) {
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		QColor colorSolid( 0, 0, 255, 255 );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_NONMANIFOLD]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_NONMANIFOLD]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_NONMANIFOLD]->release();
	}

	if( drawVerticesSingular  ) {
		// Prepare, when required
		vboPrepareDoubleCone();
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		QColor colorSolid( 128, 0, 128, 255 );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_SINGULAR]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_SINGULAR]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_SINGULAR]->release();
	}

	if( drawVerticesFuncValMin ) {
		// Prepare, when required
		vboPrepareVerticesWithFlag( FLAG_LOCAL_MIN, VBUFF_VERTICES_FLAG_LOCAL_MIN );
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// NO INCREASE, because they do not overlap with the following vertices | minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		GLfloat colorTmpStored[4];
		mRenderColors->getColorSettings( MeshGLColors::COLOR_VERTEX_LOCAL_MIN, colorTmpStored );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_FLAG_LOCAL_MIN]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_FLAG_LOCAL_MIN]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_FLAG_LOCAL_MIN]->release();
	}
	if( drawVerticesFuncValMax ) {
		// Prepare, when required
		vboPrepareVerticesWithFlag( FLAG_LOCAL_MAX, VBUFF_VERTICES_FLAG_LOCAL_MAX );
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		GLfloat colorTmpStored[4];
		mRenderColors->getColorSettings( MeshGLColors::COLOR_VERTEX_LOCAL_MAX, colorTmpStored );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_FLAG_LOCAL_MAX]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_FLAG_LOCAL_MAX]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_FLAG_LOCAL_MAX]->release();
	}

	if( drawVerticesSelected ) {
		// Prepare, when required
		vboPrepareVerticesWithFlag( FLAG_SELECTED, VBUFF_VERTICES_FLAG_SELECTED );
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		GLfloat colorTmpStored[4];
		mRenderColors->getColorSettings( MeshGLColors::COLOR_VERTEX_LOCAL_MAX, colorTmpStored );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_FLAG_SELECTED]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_FLAG_SELECTED]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_FLAG_SELECTED]->release();
	}

	if( drawVerticesSynthetic ) {
		// Prepare, when required
		vboPrepareVerticesWithFlag( FLAG_SYNTHETIC, VBUFF_VERTICES_FLAG_SYNTHETIC );
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		QColor colorSolid( 64, 192, 192, 255 );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_FLAG_SYNTHETIC]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_FLAG_SYNTHETIC]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_FLAG_SYNTHETIC]->release();
	}

	if( drawVerticesCortex ) {
		std::cout << "[MeshGLShader::paintVerticesAsSprites] Drawing cortex vertices!" << std::endl;
		// Prepare, when required
		vboPrepareVerticesWithFlag( FLAG_CORTEX, VBUFF_VERTICES_FLAG_CORTEX );
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		QColor colorSolid( 0, 255, 128, 255 );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		mVertBufObjs[VBUFF_VERTICES_FLAG_CORTEX]->bind();
		glDrawElements( GL_POINTS, mVertBufObjs[VBUFF_VERTICES_FLAG_CORTEX]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_VERTICES_FLAG_CORTEX]->release();
	}

	if( drawPrimVertSel ) {
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;
		// Material - solid color:
		QColor colorSolid( 255, 128, 0, 255 );
		mShaderVertexSprites->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Map and draw elements
		GLuint primSelID[1];
		primSelID[0] = static_cast<GLuint>(mPrimSelected->getIndex());
		glDrawElements( GL_POINTS, 1, GL_UNSIGNED_INT, primSelID );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_POINT, ..., GL_UNSIGNED_INT, NULL )" );
	}

	if( drawSinglePoints ) {
		// Base size
		mShaderVertexSprites->setUniformValue( "uPointSizeBaseMinimal", minSpriteSize );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		minSpriteSize += spriteSizeInc;
		// Z-order
		mShaderVertexSprites->setUniformValue( "uPointShiftViewZ", spriteShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		spriteShiftZ += 0.00001F;

		// Disc shape (only)
		mShaderVertexSprites->setUniformValue( "uSpriteShape", SPRITE_SHAPE_DISC );
		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		mShaderVertexSprites->setUniformValue( "uRenderColor", TEXMAP_VERT_RGB );

		QOpenGLBuffer someBuffer( QOpenGLBuffer::VertexBuffer );
		someBuffer.create();
		someBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
		someBuffer.bind();
		someBuffer.allocate( singlePoints.data(), static_cast<int>(sizeof(grVertexStripeElment)*singlePoints.size()) );

		// Strided data -- first there floats are the position vectors.
		mShaderVertexSprites->setAttributeBuffer( "position", GL_FLOAT, 0, 3, sizeof(grVertexStripeElment) ); // rShaderLocationBasic->mVertexPos
		mShaderVertexSprites->enableAttributeArray( "position" ); // rShaderLocationBasic->mVertexPos
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		size_t offSetColors = sizeof(GLfloat)*6;
		mShaderVertexSprites->setAttributeBuffer( "vColor", GL_UNSIGNED_BYTE, static_cast<int>(offSetColors), 4, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderVertexSprites->enableAttributeArray( "vColor" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

		// Draw
		glDrawArrays( GL_POINTS, 0, static_cast<GLsizei>(singlePoints.size()) );
		PRINT_OPENGL_ERROR( "glDrawArrays( ... )" );

		someBuffer.release();
		someBuffer.destroy();
	}

	// End of being shady
	mShaderVertexSprites->release();

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	if( !pinPoints.empty() ) {
		vboPaintPins(pinPoints);
	}
}

//! Paints faces.
void MeshGLShader::vboPaintFaces() {
	vboPaintFacesIndexed( );
}
//==============================================================================================================================================================

//! Paint the mesh faces using a shader with color per vertex using the (generic) function value.
void MeshGLShader::vboPaintFacesIndexed() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	struct grSphere {
		Vector3D mCenter;
		Vector3D mAxis;
		double   mRadius;
		QColor   mColor;
		bool     mIsoLines;
	} aSphere;
	vector<grSphere> someSpheres;

	if( !isUnrolledAroundSphere() && getSphereData( &aSphere.mCenter, &aSphere.mRadius ) ) {
		aSphere.mColor    = QColor( 255, 0, 0, 64 );
		aSphere.mAxis     = Vector3D( 0.0, 1.0, 0.0, 0.0 );
		aSphere.mIsoLines = true;
		someSpheres.push_back( aSphere );
	}
	for( const auto& currSphere: mDatumSpheres ) {
		aSphere.mCenter = currSphere->getPositionVector();
		aSphere.mRadius = currSphere->getRadius();
		// Color:
		unsigned char colRGBA[4];
		currSphere->copyRGBTo( colRGBA );
		double datumSphereTrans;
		getParamFloatMeshGL( DATUM_SPHERE_TRANS, &datumSphereTrans );
		colRGBA[3] = static_cast<unsigned char>(floor( datumSphereTrans*255.0 ));
		aSphere.mColor = QColor( colRGBA[0], colRGBA[1], colRGBA[2], colRGBA[3] );
		aSphere.mIsoLines = false;
		someSpheres.push_back( aSphere );
	}
	bool drawSpheres = (someSpheres.size()>0);

	vector<grVertexElmentBasic> someTriangles;
	// Mesh Plane triangle fan (as single triangles):
	bool showMeshPlane, showMeshPlaneTemp;
	getParamFlagMeshGL( SHOW_MESH_PLANE, &showMeshPlane );
	getParamFlagMeshGL( SHOW_MESH_PLANE_TEMP, &showMeshPlaneTemp );
	if( showMeshPlane || showMeshPlaneTemp ) {
		Vector3D planeHNF;

		std::array<GLubyte,3> planeColor;
		if(getPlaneDefinition() == Plane::AXIS_POINTS_AND_POSITION)
		{
			planeColor = {202,178,214};
		}
		else
		{
			planeColor = {255,170,0};
		}

		if( getPlaneHNF( &planeHNF ) ) {
			vector<Vector3D> planeIntersections;
			Vector3D centralPointPlane( 0.0, 0.0, 0.0, 0.0 );
			getMeshPlaneFan( &centralPointPlane, &planeIntersections );
			if( planeIntersections.size() > 0 ) {
				// Obverse side
				grVertexStripeElment vertexBufferDataCentral;
				vertexBufferDataCentral.mColor[0]    = 255;
				vertexBufferDataCentral.mColor[1]    = 170;
				vertexBufferDataCentral.mColor[2]    =   0;
				vertexBufferDataCentral.mColor[3]    =  64;
				vertexBufferDataCentral.mPosition[0] = centralPointPlane.getX();
				vertexBufferDataCentral.mPosition[1] = centralPointPlane.getY();
				vertexBufferDataCentral.mPosition[2] = centralPointPlane.getZ();
				vertexBufferDataCentral.mNormal[0]   = planeHNF.getX();
				vertexBufferDataCentral.mNormal[1]   = planeHNF.getY();
				vertexBufferDataCentral.mNormal[2]   = planeHNF.getZ();
				grVertexStripeElment vertexBufferData;
				vertexBufferData.mColor[0]   = planeColor[0];
				vertexBufferData.mColor[1]   = planeColor[1];
				vertexBufferData.mColor[2]   = planeColor[2];
				vertexBufferData.mColor[3]   =  64;
				vertexBufferData.mNormal[0]  = planeHNF.getX();
				vertexBufferData.mNormal[1]  = planeHNF.getY();
				vertexBufferData.mNormal[2]  = planeHNF.getZ();
				Vector3D lastPos = planeIntersections.back();
				vertexBufferData.mPosition[0] = lastPos.getX();
				vertexBufferData.mPosition[1] = lastPos.getY();
				vertexBufferData.mPosition[2] = lastPos.getZ();
				for( const auto& nextPos: planeIntersections ) {
					someTriangles.push_back( vertexBufferData );
					vertexBufferData.mPosition[0] = nextPos.getX();
					vertexBufferData.mPosition[1] = nextPos.getY();
					vertexBufferData.mPosition[2] = nextPos.getZ();
					someTriangles.push_back( vertexBufferData );
					someTriangles.push_back( vertexBufferDataCentral );
				}
				// Reverse side
				planeHNF *= -1.0;
				vertexBufferDataCentral.mColor[0]    = 255;
				vertexBufferDataCentral.mColor[1]    =  92;
				vertexBufferDataCentral.mColor[2]    =  92;
				vertexBufferDataCentral.mColor[3]    =  64;
				vertexBufferDataCentral.mNormal[0]   = planeHNF.getX();
				vertexBufferDataCentral.mNormal[1]   = planeHNF.getY();
				vertexBufferDataCentral.mNormal[2]   = planeHNF.getZ();
				vertexBufferData.mColor[0]   = 255;
				vertexBufferData.mColor[1]   =  92;
				vertexBufferData.mColor[2]   =  92;
				vertexBufferData.mColor[3]   =  64;
				vertexBufferData.mNormal[0]  = planeHNF.getX();
				vertexBufferData.mNormal[1]  = planeHNF.getY();
				vertexBufferData.mNormal[2]  = planeHNF.getZ();
				vertexBufferData.mPosition[0] = lastPos.getX();
				vertexBufferData.mPosition[1] = lastPos.getY();
				vertexBufferData.mPosition[2] = lastPos.getZ();
				for( const auto& nextPos: planeIntersections ) {
					someTriangles.push_back( vertexBufferData );
					vertexBufferData.mPosition[0] = nextPos.getX();
					vertexBufferData.mPosition[1] = nextPos.getY();
					vertexBufferData.mPosition[2] = nextPos.getZ();
					someTriangles.push_back( vertexBufferDataCentral );
					someTriangles.push_back( vertexBufferData );
				}
			}
		}
	}
	bool drawSomeTriangles = (someTriangles.size()>0);

	bool drawFacesSelected;
	getParamFlagMeshGL( SHOW_FACES_SELECTION, &drawFacesSelected );
	bool drawPrimFaceSel;
	getParamFlagMeshGL( MeshGLParams::SHOW_SELECTION_SINGLE, &drawPrimFaceSel );
	drawPrimFaceSel &= ( mPrimSelected != nullptr ) && ( mPrimSelected->getType() == Primitive::IS_FACE );
	if( !( drawFacesSelected | drawPrimFaceSel | drawSpheres | drawSomeTriangles ) ) {
		return;
	}

	const auto mShaderDatumObjects = mShaderManager->getShader(ShaderManager::ShaderName::FUNC_VAL_COLOR);
	if(mShaderDatumObjects != nullptr)

	// prepare, when needed
	vboPrepareVerticesStriped();

	// Turn off culling as it is done by the shader, but store the flag for later.
	GLboolean cullFace;
	glGetBooleanv( GL_CULL_FACE, &cullFace );
	glDisable( GL_CULL_FACE );
	PRINT_OPENGL_ERROR( "glDisable( GL_CULL_FACE )" );

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	// Lets be shady :)
	if( !mShaderDatumObjects->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	// Set the basics - including strided data:
	//----------------------------------
	shaderSetLocationBasicMatrices( mShaderDatumObjects );
	shaderSetLocationBasicAttribs( mShaderDatumObjects, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );

	// More Strided Data -- map buffer
	//----------------------------------
	size_t offSetLabelID = sizeof(GLfloat)*6 + sizeof(GLubyte)*4 + sizeof(GLfloat);
	mShaderDatumObjects->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderDatumObjects->enableAttributeArray( "vLabelID" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
	mShaderDatumObjects->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderDatumObjects->enableAttributeArray( "vFlags" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

	// Set the basics for faces (culling/smooth)
	shaderSetLocationBasicFaces( mShaderDatumObjects );

	// Set the basic lighting:
	shaderSetLocationBasicLight( mShaderDatumObjects );

	// Set the basic fog:
	shaderSetLocationBasicFog( mShaderDatumObjects );

	// Set the mesh plane as clipping plane:
	double clipPlane[4];
	getPlaneHNF( clipPlane );
	mShaderDatumObjects->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		mShaderDatumObjects->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
	}

	// Material - solid color:
	GLfloat colorTmpStored[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorTmpStored );
	mShaderDatumObjects->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Bind the 2D texture used for colorramps and labels!
	//glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] );
	//PRINT_OPENGL_ERROR( "glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] )" );
	mTextureMaps[0]->bind();
	GLint texId;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &texId);
	texId -= GL_TEXTURE0;
    PRINT_OPENGL_ERROR( "binding mTextureMaps[0]" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] texId: " << texId << endl;
	// Set the ID of the texture map:
	mShaderDatumObjects->setUniformValue( "uLabelTexMap", texId );
    PRINT_OPENGL_ERROR( "Shader setAttributeValue" );
	mShaderDatumObjects->setUniformValue( "uFuncValTexMap", texId );
	PRINT_OPENGL_ERROR( "Shader setAttributeValue" );

	// Set values for rendering of the labels:
	int labelShift;
	getParamIntMeshGL( MeshGLParams::COLMAP_LABEL_OFFSET, &labelShift );
	mShaderDatumObjects->setUniformValue( "uLabelCountOffset", static_cast<GLfloat>(labelShift) );
	bool showLabelsMonoColor;
	getParamFlagMeshGL( MeshGLParams::SHOW_LABELS_MONO_COLOR, &showLabelsMonoColor );
	mShaderDatumObjects->setUniformValue( "uLabelSameColor", showLabelsMonoColor );
	GLfloat somemColorSetting[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_SOLID, somemColorSetting );
	mShaderDatumObjects->setUniformValue( "uLabelSingleColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BORDER, somemColorSetting );
	mShaderDatumObjects->setUniformValue( "uLabelBorderColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_NOT_ASSIGNED, somemColorSetting );
	mShaderDatumObjects->setUniformValue( "uLabelNoColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BACKGROUND, somemColorSetting );
	mShaderDatumObjects->setUniformValue( "uLabelBackgroundColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Set parameters of the color mapping.
	int colMapID = GLSL_COLMAP_GRAYSCALE;
	getParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, &colMapID );
	mShaderDatumObjects->setUniformValue( "uFuncValColorMap", static_cast<GLfloat>(colMapID) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Set the colored range.
	double funcValMin;
	double funcValMax;
	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	mShaderDatumObjects->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(funcValMin) );
	mShaderDatumObjects->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(funcValMax) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] funcValMin: " << funcValMin << "  funcValMax: " << funcValMax << endl;
	// Inversion of the colormap
	bool funcValInvertMap = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_COLMAP_INVERT, &funcValInvertMap );
	mShaderDatumObjects->setUniformValue( "uFuncValInvert", static_cast<GLboolean>(funcValInvertMap) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Repeat colormap
	bool funcValRepeat = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &funcValRepeat );
	mShaderDatumObjects->setUniformValue( "uFuncValRepeat", static_cast<GLboolean>(funcValRepeat) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	double funcValRepeatIntervall;
	getParamFloatMeshGL( MeshGLParams::WAVES_COLMAP_LEN, &funcValRepeatIntervall );
	mShaderDatumObjects->setUniformValue( "uFuncValIntervall", static_cast<GLfloat>(funcValRepeatIntervall) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Logarithmic color ramp
	double funcValLogGamma;
	getParamFloatMeshGL( MeshGLParams::FUNC_VALUE_LOG_GAMMA, &funcValLogGamma );
	mShaderDatumObjects->setUniformValue( "uFuncValLogGamma", static_cast<GLfloat>(funcValLogGamma) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Isoline extension:
	//----------------------------------
	bool showIsolines;
	getParamFlagMeshGL( MeshGLParams::SHOW_FUNC_VALUES_ISOLINES, &showIsolines );
	//disable isoline for datum Objects
	mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoLinesShow", static_cast<GLboolean>(false) );
	if( showIsolines ) {
		bool showIsolinesOnly;
		getParamFlagMeshGL( MeshGLParams::SHOW_FUNC_VALUES_ISOLINES_ONLY, &showIsolinesOnly );
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoLinesOnly", static_cast<GLboolean>(showIsolinesOnly) );
		double isoDistance;
		double isoOffset;
		double isoPixelWidth;
		getParamFloatMeshGL( MeshGLParams::ISOLINES_DISTANCE, &isoDistance );
		getParamFloatMeshGL( MeshGLParams::ISOLINES_OFFSET, &isoOffset );
		getParamFloatMeshGL( MeshGLParams::ISOLINES_PIXEL_WIDTH, &isoPixelWidth );
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoDist",       static_cast<GLfloat>(isoDistance)   );
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoOffset",     static_cast<GLfloat>(isoOffset)     );
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoPixelWidth", static_cast<GLfloat>(isoPixelWidth) );
		bool showIsolinesSolid;
		getParamFlagMeshGL( MeshGLParams::SHOW_FUNC_VALUES_ISOLINES_SOLID, &showIsolinesSolid );
		mShaderDatumObjects->setUniformValue( "uIsoSolidFlag", static_cast<GLboolean>(showIsolinesSolid) );
	}
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Geometry shader setting:
	//----------------------------------
	bool showFaceNormals;
	getParamFlagMeshGL( MeshGLParams::SHOW_NORMALS_FACE, &showFaceNormals );
	mShaderDatumObjects->setUniformValue( "uFaceNormals", static_cast<GLboolean>(showFaceNormals) );
	double normalsLength;
	getParamFloatMeshGL( MeshGLParams::NORMALS_LENGTH, &normalsLength );
	mShaderDatumObjects->setUniformValue( "uNormalLength", static_cast<GLfloat>(normalsLength) );
	double normalsWdith;
	getParamFloatMeshGL( MeshGLParams::NORMALS_WIDTH, &normalsWdith );
	mShaderDatumObjects->setUniformValue( "uNormalWidth", static_cast<GLfloat>(normalsWdith) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Edge rendering
	//----------------------------------
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	bool showEdge;
	getParamFlagMeshGL( MeshGLParams::SHOW_FACES_EDGES, &showEdge );
	mShaderDatumObjects->setUniformValue( "uEdgeShown", static_cast<GLboolean>(showEdge) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	mShaderDatumObjects->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Index data map buffer and draw
	//----------------------------------
	GLfloat faceShiftZ = 0.0;
	faceShiftZ -= 0.0001F;

	if( drawFacesSelected ) {
		// Prepare, when required
		vboPrepareFacesWithFlag( FLAG_SELECTED, VBUFF_FACES_FLAG_SELECTED );
		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		mShaderDatumObjects->setUniformValue( "uRenderColor", TEXMAP_VERT_MONO );
		// Material - solid color:
		QColor colorSolid( 255, 128, 0, 255 );
		mShaderDatumObjects->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Z-shift
		mShaderDatumObjects->setUniformValue( "uFaceShiftViewZ", faceShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		faceShiftZ -= 0.0001F;
		// Draw
		mVertBufObjs[VBUFF_FACES_FLAG_SELECTED]->bind();
		glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES_FLAG_SELECTED]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES_FLAG_SELECTED], GL_UNSIGNED_INT, NULL );" );
		mVertBufObjs[VBUFF_FACES_FLAG_SELECTED]->release();
	}

	if( drawPrimFaceSel ) {
		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		mShaderDatumObjects->setUniformValue( "uRenderColor", TEXMAP_VERT_MONO );
		// Material - solid color:
		QColor colorSolid( 0, 128, 255, 255 );
		mShaderDatumObjects->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Z-shift
		mShaderDatumObjects->setUniformValue( "uFaceShiftViewZ", faceShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		faceShiftZ -= 0.0001F;
		// Map and draw elements
		GLuint primSelID[3];
		Face*  selFace = static_cast<Face*>(mPrimSelected);
		primSelID[0] = selFace->getVertA()->getIndex();
		primSelID[1] = selFace->getVertB()->getIndex();
		primSelID[2] = selFace->getVertC()->getIndex();
		glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, primSelID );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, primSelID )" );
	}

	// Turn of the clipping plane. This prevents flickering of the mesh plane (currently in someTriangles).
	// Will also keep the primitives such as spheres un-clipped.
	glDisable( GL_CLIP_PLANE0 );
	glDisable( GL_CLIP_PLANE1 );  // DongArch clip preview

	if( drawSomeTriangles ) {
		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		mShaderDatumObjects->setUniformValue( "uRenderColor", TEXMAP_VERT_RGB );
		// Z-shift - None!
		mShaderDatumObjects->setUniformValue( "uFaceShiftViewZ", static_cast<GLfloat>(0.0f) );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );

		mShaderDatumObjects->setUniformValue( "backCulling", false );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );

		QOpenGLBuffer someBuffer( QOpenGLBuffer::VertexBuffer );
		someBuffer.create();
		someBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
		someBuffer.bind();
		someBuffer.allocate( someTriangles.data(), static_cast<int>(sizeof(grVertexElmentBasic)*someTriangles.size()) );

		// Strided data -- first there floats are the position vectors.
		mShaderDatumObjects->setAttributeBuffer( "position", GL_FLOAT, 0, 3, sizeof(grVertexElmentBasic) ); // rShaderLocationBasic->mVertexPos
		mShaderDatumObjects->enableAttributeArray( "position" ); // rShaderLocationBasic->mVertexPos
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		// Strided data -- second set of there floats are the normal vectors.
		size_t offSetNormals = sizeof(GLfloat)*3;
		mShaderDatumObjects->setAttributeBuffer( "vNormal", GL_FLOAT, static_cast<int>(offSetNormals), 3, sizeof(grVertexElmentBasic) ); // rShaderLocationBasic->mVertexNormal
		mShaderDatumObjects->enableAttributeArray( "vNormal" ); // rShaderLocationBasic->mVertexNormal
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		size_t offSetColors = offSetNormals + sizeof(GLfloat)*3;
		mShaderDatumObjects->setAttributeBuffer( "vColor", GL_UNSIGNED_BYTE, static_cast<int>(offSetColors), 4, static_cast<int>(sizeof(grVertexElmentBasic)) );
		mShaderDatumObjects->enableAttributeArray( "vColor" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

		// Draw
		glDrawArrays( GL_TRIANGLES, 0, static_cast<GLsizei>(someTriangles.size()) );
		PRINT_OPENGL_ERROR( "glDrawArrays( ... )" );

		someBuffer.release();
		someBuffer.destroy();
	}

	// +++ SPHEREs +++
	if( drawSpheres ) {
		// Vertex buffer
		shaderSetLocationBasicAttribs( mShaderDatumObjects, VBUFF_SPHERE_VERTS, static_cast<int>(sizeof(grVertexElmentBasic)) );

		// No shift
		mShaderDatumObjects->setUniformValue( "uFaceShiftViewZ", static_cast<GLfloat>(0.0) );
		// Set the colored range.
		mShaderDatumObjects->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(-1.0) );
		mShaderDatumObjects->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(+1.0) );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// IsoLines
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoLinesOnly",  static_cast<GLboolean>(false) );
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoDist",       static_cast<GLfloat>(0.1)     );
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoOffset",     static_cast<GLfloat>(0.0)     );
		mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoPixelWidth", static_cast<GLfloat>(1.0)     );
		mShaderDatumObjects->setUniformValue( "uIsoSolidFlag",                       static_cast<GLboolean>(true)  );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		mShaderDatumObjects->setUniformValue( "uRenderColor", TEXMAP_VERT_MONO );
		mShaderDatumObjects->setUniformValue( "backCulling", false );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );

		mVertBufObjs[VBUFF_SPHERE_FACES]->bind();
		PRINT_OPENGL_ERROR( "mVertBufObjs[VBUFF_SPHERE_FACES]->bind()" );

		for( const auto& currSphere: someSpheres ) {
			Vector3D sphereAxis( 0.0, 0.0, 1.0 );                                     // positive z-axis by default
			Vector3D axisOfRotation = sphereAxis % currSphere.mAxis;                     // we will rotate around this axis...
			double   rotationAngle  = angle( currSphere.mAxis, sphereAxis )*180.0/M_PI;  // ...by this angle

			QMatrix4x4 transMat;
			transMat.translate( currSphere.mCenter.getX(), currSphere.mCenter.getY(), currSphere.mCenter.getZ() );
			QMatrix4x4 rotMat;
			rotMat.rotate( rotationAngle, axisOfRotation.getX(), axisOfRotation.getY(), axisOfRotation.getZ() );
			QMatrix4x4 transRotMat = transMat * rotMat;

			// IsoLines
			mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoLinesShow",  static_cast<GLboolean>(currSphere.mIsoLines)  );
			// Material - solid color:
			mShaderDatumObjects->setUniformValue( "colorSolid", currSphere.mColor );
			PRINT_OPENGL_ERROR( "Shader setUniformValue" );

			// Scale - SET
			mShaderDatumObjects->setUniformValue( "uScaleHeight",       static_cast<GLfloat>(currSphere.mRadius*2.0) );
			mShaderDatumObjects->setUniformValue( "uScaleRadialBottom", static_cast<GLfloat>(currSphere.mRadius*2.0) );
			mShaderDatumObjects->setUniformValue( "uScaleRadialTop",    static_cast<GLfloat>(currSphere.mRadius*2.0) );
			PRINT_OPENGL_ERROR( "SHADER operations!" );
			// Translation and Rotation - SET
			mShaderDatumObjects->setUniformValue( "uModelViewExtra", transRotMat );
			PRINT_OPENGL_ERROR( "SHADER operations!" );

			glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_SPHERE_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
			PRINT_OPENGL_ERROR( "glDrawElements( ... )" );
		}

		mVertBufObjs[VBUFF_SPHERE_FACES]->release();
		PRINT_OPENGL_ERROR( "mVertBufObjs[VBUFF_SPHERE_FACES]->release()" );

		// Scale - set BACK TO DEFAULTS
		mShaderDatumObjects->setUniformValue( "uScaleHeight",       static_cast<GLfloat>(1.0f) );
		mShaderDatumObjects->setUniformValue( "uScaleRadialBottom", static_cast<GLfloat>(1.0f) );
		mShaderDatumObjects->setUniformValue( "uScaleRadialTop",    static_cast<GLfloat>(1.0f) );
		PRINT_OPENGL_ERROR( "SHADER operations!" );
		QMatrix4x4 identMat;
		identMat.setToIdentity();
		// Translation and Rotation - set to DEFAULTS
		mShaderDatumObjects->setUniformValue( "uModelViewExtra", identMat );
		PRINT_OPENGL_ERROR( "SHADER operations!" );
	}
	// --- SPHEREs ---

	// End of being shady
	mShaderDatumObjects->release();
	PRINT_OPENGL_ERROR( "mShaderDatumObjects->release()" );

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	// Restore culling, when necessary.
	if( cullFace ) {
		glEnable( GL_CULL_FACE );
		PRINT_OPENGL_ERROR( "glEnable( GL_CULL_FACE )" );
	}
}

void MeshGLShader::vboPaintFuncVal()
{
	bool drawFaces = false;
	getParamFlagMeshGL( SHOW_FACES, &drawFaces );

	if(!drawFaces)
		return;

	QOpenGLShaderProgram* meshShader = nullptr;
	eTexMapType texMapChoice;
	getParamIntMeshGL( TEXMAP_CHOICE_FACES, reinterpret_cast<int*>(&texMapChoice) );
	switch( texMapChoice ) {
		case TEXMAP_VERT_MONO:
		case TEXMAP_VERT_RGB:
			meshShader = mShaderManager->getShader(ShaderManager::ShaderName::FUNC_VAL_COLOR);
			break;
		case TEXMAP_VERT_FUNCVAL:
			meshShader = mShaderManager->getShader(ShaderManager::ShaderName::FUNC_VAL_FUNC_VAL);
			break;
		case TEXMAP_VERT_LABELS:
			meshShader = mShaderManager->getShader(ShaderManager::ShaderName::FUNC_VAL_LABEL);
			break;
		break;
		default:
			cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: unknown choice for texture mapping: " << texMapChoice << "!" << endl;
	}

	if(meshShader == nullptr)
		return;

	// prepare, when needed
	vboPrepareVerticesStriped();

	// Turn off culling as it is done by the shader, but store the flag for later.
	GLboolean cullFace;
	glGetBooleanv( GL_CULL_FACE, &cullFace );
	glDisable( GL_CULL_FACE );
	PRINT_OPENGL_ERROR( "glDisable( GL_CULL_FACE )" );

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	// Lets be shady :)
	if( !meshShader->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	// Set the basics - including strided data:
	//----------------------------------
	shaderSetLocationBasicMatrices( meshShader );
	shaderSetLocationBasicAttribs( meshShader, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );

	// More Strided Data -- map buffer
	//----------------------------------
	size_t offSetLabelID = sizeof(GLfloat)*6 + sizeof(GLubyte)*4 + sizeof(GLfloat);
	meshShader->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	meshShader->enableAttributeArray( "vLabelID" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
	meshShader->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	meshShader->enableAttributeArray( "vFlags" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

	// Set the basics for faces (culling/smooth)
	shaderSetLocationBasicFaces( meshShader );

	// Set the basic lighting:
	shaderSetLocationBasicLight( meshShader );

	// Set the basic fog:
	shaderSetLocationBasicFog( meshShader );

	// Set the mesh plane as clipping plane:
	double clipPlane[4];
	getPlaneHNF( clipPlane );
	meshShader->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		meshShader->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
	}

	// Material - solid color:
	GLfloat colorTmpStored[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorTmpStored );
	meshShader->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Bind the 2D texture used for colorramps and labels!
	//glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] );
	//PRINT_OPENGL_ERROR( "glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] )" );
	mTextureMaps[0]->bind();
	GLint texId;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &texId);
	texId -= GL_TEXTURE0;
	PRINT_OPENGL_ERROR( "binding mTextureMaps[0]" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] texId: " << texId << endl;
	// Set the ID of the texture map:
	meshShader->setUniformValue( "uLabelTexMap", texId );
	PRINT_OPENGL_ERROR( "Shader setAttributeValue" );
	meshShader->setUniformValue( "uFuncValTexMap", texId );
	PRINT_OPENGL_ERROR( "Shader setAttributeValue" );

	// Set values for rendering of the labels:
	int labelShift;
	getParamIntMeshGL( MeshGLParams::COLMAP_LABEL_OFFSET, &labelShift );
	meshShader->setUniformValue( "uLabelCountOffset", static_cast<GLfloat>(labelShift) );
	bool showLabelsMonoColor;
	getParamFlagMeshGL( MeshGLParams::SHOW_LABELS_MONO_COLOR, &showLabelsMonoColor );
	meshShader->setUniformValue( "uLabelSameColor", showLabelsMonoColor );
	GLfloat somemColorSetting[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_SOLID, somemColorSetting );
	meshShader->setUniformValue( "uLabelSingleColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BORDER, somemColorSetting );
	meshShader->setUniformValue( "uLabelBorderColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_NOT_ASSIGNED, somemColorSetting );
	meshShader->setUniformValue( "uLabelNoColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BACKGROUND, somemColorSetting );
	meshShader->setUniformValue( "uLabelBackgroundColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Set parameters of the color mapping.
	int colMapID = GLSL_COLMAP_GRAYSCALE;
	getParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, &colMapID );
	meshShader->setUniformValue( "uFuncValColorMap", static_cast<GLfloat>(colMapID) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Set the colored range.
	double funcValMin;
	double funcValMax;
	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	meshShader->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(funcValMin) );
	meshShader->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(funcValMax) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] funcValMin: " << funcValMin << "  funcValMax: " << funcValMax << endl;
	// Inversion of the colormap
	bool funcValInvertMap = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_COLMAP_INVERT, &funcValInvertMap );
	meshShader->setUniformValue( "uFuncValInvert", static_cast<GLboolean>(funcValInvertMap) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Repeat colormap
	bool funcValRepeat = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &funcValRepeat );
	meshShader->setUniformValue( "uFuncValRepeat", static_cast<GLboolean>(funcValRepeat) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	double funcValRepeatIntervall;
	getParamFloatMeshGL( MeshGLParams::WAVES_COLMAP_LEN, &funcValRepeatIntervall );
	meshShader->setUniformValue( "uFuncValIntervall", static_cast<GLfloat>(funcValRepeatIntervall) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Logarithmic color ramp
	double funcValLogGamma;
	getParamFloatMeshGL( MeshGLParams::FUNC_VALUE_LOG_GAMMA, &funcValLogGamma );
	meshShader->setUniformValue( "uFuncValLogGamma", static_cast<GLfloat>(funcValLogGamma) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Isoline extension:
	//----------------------------------
	bool showIsolines;
	getParamFlagMeshGL( MeshGLParams::SHOW_FUNC_VALUES_ISOLINES, &showIsolines );
	meshShader->setUniformValue( "funcValIsoLineParams.mIsoLinesShow", static_cast<GLboolean>(showIsolines) );
	if( showIsolines ) {
		bool showIsolinesOnly;
		getParamFlagMeshGL( MeshGLParams::SHOW_FUNC_VALUES_ISOLINES_ONLY, &showIsolinesOnly );
		meshShader->setUniformValue( "funcValIsoLineParams.mIsoLinesOnly", static_cast<GLboolean>(showIsolinesOnly) );
		double isoDistance;
		double isoOffset;
		double isoPixelWidth;
		getParamFloatMeshGL( MeshGLParams::ISOLINES_DISTANCE, &isoDistance );
		getParamFloatMeshGL( MeshGLParams::ISOLINES_OFFSET, &isoOffset );
		getParamFloatMeshGL( MeshGLParams::ISOLINES_PIXEL_WIDTH, &isoPixelWidth );
		meshShader->setUniformValue( "funcValIsoLineParams.mIsoDist",       static_cast<GLfloat>(isoDistance)   );
		meshShader->setUniformValue( "funcValIsoLineParams.mIsoOffset",     static_cast<GLfloat>(isoOffset)     );
		meshShader->setUniformValue( "funcValIsoLineParams.mIsoPixelWidth", static_cast<GLfloat>(isoPixelWidth) );
		bool showIsolinesSolid;
		getParamFlagMeshGL( MeshGLParams::SHOW_FUNC_VALUES_ISOLINES_SOLID, &showIsolinesSolid );
		meshShader->setUniformValue( "uIsoSolidFlag", static_cast<GLboolean>(showIsolinesSolid) );
	}
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Geometry shader setting:
	//----------------------------------
	bool showFaceNormals;
	getParamFlagMeshGL( MeshGLParams::SHOW_NORMALS_FACE, &showFaceNormals );
	meshShader->setUniformValue( "uFaceNormals", static_cast<GLboolean>(showFaceNormals) );
	double normalsLength;
	getParamFloatMeshGL( MeshGLParams::NORMALS_LENGTH, &normalsLength );
	meshShader->setUniformValue( "uNormalLength", static_cast<GLfloat>(normalsLength) );
	double normalsWdith;
	getParamFloatMeshGL( MeshGLParams::NORMALS_WIDTH, &normalsWdith );
	meshShader->setUniformValue( "uNormalWidth", static_cast<GLfloat>(normalsWdith) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Edge rendering
	//----------------------------------
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);
	bool showEdge;
	getParamFlagMeshGL( MeshGLParams::SHOW_FACES_EDGES, &showEdge );
	meshShader->setUniformValue( "uEdgeShown", static_cast<GLboolean>(showEdge) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	meshShader->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );


	// Switch rendering mode solid/color_per_vertex/function_value/labels:
	meshShader->setUniformValue( "uRenderColor", (texMapChoice == TEXMAP_VERT_MONO ? 0 : 1) );
	// Material - solid color:

	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorTmpStored );
	meshShader->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Z-shift
	meshShader->setUniformValue( "uFaceShiftViewZ", 0.0F );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Draw
	mVertBufObjs[VBUFF_FACES]->bind();
	glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] Faces drawn: " << mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint) << endl;
	mVertBufObjs[VBUFF_FACES]->release();

	// End of being shady
	meshShader->release();
	PRINT_OPENGL_ERROR( "meshShader->release()" );

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	// Restore culling, when necessary.
	if( cullFace ) {
		glEnable( GL_CULL_FACE );
		PRINT_OPENGL_ERROR( "glEnable( GL_CULL_FACE )" );
	}
}

//! Paint the mesh as wireframe.
void MeshGLShader::vboPaintWireframe() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	bool drawFacesSelected;
	getParamFlagMeshGL( SHOW_FACES_SELECTION, &drawFacesSelected );
	bool drawPrimFaceSel;
	getParamFlagMeshGL( MeshGLParams::SHOW_SELECTION_SINGLE, &drawPrimFaceSel );
	drawPrimFaceSel &= ( mPrimSelected != nullptr ) && ( mPrimSelected->getType() == Primitive::IS_FACE );
	if( !( drawFacesSelected | drawPrimFaceSel ) ) {
		return;
	}

	const auto mShaderWireframe = mShaderManager->getShader(ShaderManager::ShaderName::WIREFRAME);
	if(mShaderWireframe == nullptr)
		return;

	// prepare, when needed
	vboPrepareVerticesStriped();

	// Turn off culling as it is done by the shader, but store the flag for later.
	GLboolean cullFace;
	glGetBooleanv( GL_CULL_FACE, &cullFace );
	glDisable( GL_CULL_FACE );
	PRINT_OPENGL_ERROR( "glDisable( GL_CULL_FACE )" );

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	// Lets be shady :)
	if( !mShaderWireframe->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	// Set the basics - including strided data:
	//----------------------------------
	shaderSetLocationBasicMatrices( mShaderWireframe );
	shaderSetLocationBasicAttribs( mShaderWireframe, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );

	// More Strided Data -- map buffer
	//----------------------------------
	size_t offSetLabelID = sizeof(GLfloat)*6 + sizeof(GLubyte)*4 + sizeof(GLfloat);
	mShaderWireframe->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderWireframe->enableAttributeArray( "vLabelID" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
	mShaderWireframe->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderWireframe->enableAttributeArray( "vFlags" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

	// Set the basics for faces (culling/smooth)
	shaderSetLocationBasicFaces( mShaderWireframe );

	// Set the basic lighting:
	shaderSetLocationBasicLight( mShaderWireframe );

	// Set the basic fog:
	shaderSetLocationBasicFog( mShaderWireframe );

	// Set the mesh plane as clipping plane:
	double clipPlane[4];
	getPlaneHNF( clipPlane );
	mShaderWireframe->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		mShaderWireframe->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
	}

	// Material - solid color:
	GLfloat colorTmpStored[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorTmpStored );
	mShaderWireframe->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Set parameters of the color mapping.
	int colMapID = GLSL_COLMAP_GRAYSCALE;
	getParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, &colMapID );
	mShaderWireframe->setUniformValue( "uFuncValColorMap", static_cast<GLfloat>(colMapID) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Set the colored range.
	double funcValMin;
	double funcValMax;
	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	mShaderWireframe->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(funcValMin) );
	mShaderWireframe->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(funcValMax) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] funcValMin: " << funcValMin << "  funcValMax: " << funcValMax << endl;
	// Inversion of the colormap
	bool funcValInvertMap = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_COLMAP_INVERT, &funcValInvertMap );
	mShaderWireframe->setUniformValue( "uFuncValInvert", static_cast<GLboolean>(funcValInvertMap) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Repeat colormap
	bool funcValRepeat = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &funcValRepeat );
	mShaderWireframe->setUniformValue( "uFuncValRepeat", static_cast<GLboolean>(funcValRepeat) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	double funcValRepeatIntervall;
	getParamFloatMeshGL( MeshGLParams::WAVES_COLMAP_LEN, &funcValRepeatIntervall );
	mShaderWireframe->setUniformValue( "uFuncValIntervall", static_cast<GLfloat>(funcValRepeatIntervall) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Logarithmic color ramp
	double funcValLogGamma;
	getParamFloatMeshGL( MeshGLParams::FUNC_VALUE_LOG_GAMMA, &funcValLogGamma );
	mShaderWireframe->setUniformValue( "uFuncValLogGamma", static_cast<GLfloat>(funcValLogGamma) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Geometry shader setting:
	//----------------------------------
	bool showFaceNormals;
	getParamFlagMeshGL( MeshGLParams::SHOW_NORMALS_FACE, &showFaceNormals );
	mShaderWireframe->setUniformValue( "uFaceNormals", static_cast<GLboolean>(showFaceNormals) );
	double normalsLength;
	getParamFloatMeshGL( MeshGLParams::NORMALS_LENGTH, &normalsLength );
	mShaderWireframe->setUniformValue( "uNormalLength", static_cast<GLfloat>(normalsLength) );
	double normalsWdith;
	getParamFloatMeshGL( MeshGLParams::NORMALS_WIDTH, &normalsWdith );
	mShaderWireframe->setUniformValue( "uNormalWidth", static_cast<GLfloat>(normalsWdith) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Edge/Wireframe rendering
	//----------------------------------
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	mShaderWireframe->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Index data map buffer and draw
	//----------------------------------
	// Z-shift
	mShaderWireframe->setUniformValue( "uWireFrame", static_cast<GLboolean>(true) );
	GLfloat faceShiftZ = 0.0;
	mShaderWireframe->setUniformValue( "uFaceShiftViewZ", faceShiftZ );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	faceShiftZ -= 0.0001F;
	// Draw
	mVertBufObjs[VBUFF_FACES]->bind();
	glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] Faces drawn: " << mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint) << endl;
	mVertBufObjs[VBUFF_FACES]->release();
	// Reset -- Wireframe
	mShaderWireframe->setUniformValue( "uWireFrame", static_cast<GLboolean>(false) );

	if( drawFacesSelected ) {
		// Prepare, when required
		vboPrepareFacesWithFlag( FLAG_SELECTED, VBUFF_FACES_FLAG_SELECTED );
		// Material - solid color:
		QColor colorSolid( 255, 128, 0, 255 );
		mShaderWireframe->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Z-shift
		mShaderWireframe->setUniformValue( "uFaceShiftViewZ", faceShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		faceShiftZ -= 0.0001F;
		// Draw
		mVertBufObjs[VBUFF_FACES_FLAG_SELECTED]->bind();
		glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES_FLAG_SELECTED]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES_FLAG_SELECTED], GL_UNSIGNED_INT, NULL );" );
		mVertBufObjs[VBUFF_FACES_FLAG_SELECTED]->release();
	}

	if( drawPrimFaceSel ) {
		// Material - solid color:
		QColor colorSolid( 0, 128, 255, 255 );
		mShaderWireframe->setUniformValue( "colorSolid", colorSolid );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Z-shift
		mShaderWireframe->setUniformValue( "uFaceShiftViewZ", faceShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		faceShiftZ -= 0.0001F;
		// Map and draw elements
		GLuint primSelID[3];
		Face*  selFace = static_cast<Face*>(mPrimSelected);
		primSelID[0] = selFace->getVertA()->getIndex();
		primSelID[1] = selFace->getVertB()->getIndex();
		primSelID[2] = selFace->getVertC()->getIndex();
		glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, primSelID );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, primSelID )" );
	}

	// Turn of the clipping plane. This prevents flickering of the mesh plane (currently in someTriangles).
	// Will also keep the primitives such as spheres un-clipped.
	glDisable( GL_CLIP_PLANE0 );
	glDisable( GL_CLIP_PLANE1 );  // DongArch clip preview

	// End of being shady
	mShaderWireframe->release();
	PRINT_OPENGL_ERROR( "mShaderWireframe->release()" );

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	// Restore culling, when necessary.
	if( cullFace ) {
		glEnable( GL_CULL_FACE );
		PRINT_OPENGL_ERROR( "glEnable( GL_CULL_FACE )" );
	}
}

//! Paint the polylines.
//! ... and cone's axis and meridians, when present.
void MeshGLShader::vboPaintPolylines() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	vector<grVertexStripeElment> lineSegments; // This should hold only a few - for large lists of segments a VBO may be more suitable (for performance!).

	// Selection Mode (Cone, Sphere, etc. )
	MeshWidgetParams::eSelectionModes selectionMode;
	mWidgetParams->getParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, reinterpret_cast<int*>(&selectionMode) );

	// Axis (Used by cone, but also for other tasks)
	bool showMeshAxis = false;
	getParamFlagMeshGL( SHOW_MESH_AXIS, &showMeshAxis );
	Vector3D lineTop;
	Vector3D lineBottom;
	if( getConeAxis( &lineTop, &lineBottom ) &&
		( showMeshAxis || ( selectionMode == MeshWidgetParams::SELECTION_MODE_CONE ) ) ) {
		grVertexStripeElment vertexBufferData[2];
		vertexBufferData[0].mPosition[0] = lineBottom.getX();
		vertexBufferData[0].mPosition[1] = lineBottom.getY();
		vertexBufferData[0].mPosition[2] = lineBottom.getZ();
		vertexBufferData[0].mColor[0]    =   0;
		vertexBufferData[0].mColor[1]    = 160;
		vertexBufferData[0].mColor[2]    =   0;
		vertexBufferData[0].mColor[3]    = 255;
		vertexBufferData[1].mPosition[0] = lineTop.getX();
		vertexBufferData[1].mPosition[1] = lineTop.getY();
		vertexBufferData[1].mPosition[2] = lineTop.getZ();
		vertexBufferData[1].mColor[0]    =   0;
		vertexBufferData[1].mColor[1]    = 223;
		vertexBufferData[1].mColor[2]    =   0;
		vertexBufferData[1].mColor[3]    = 255;
		lineSegments.push_back( vertexBufferData[0] );
		lineSegments.push_back( vertexBufferData[1] );
	}

	// Cone
	if( selectionMode == MeshWidgetParams::SELECTION_MODE_CONE ) {
		if( getConeMeridianZero( &lineTop, &lineBottom, true ) ) {
			grVertexStripeElment vertexBufferData[2];
			vertexBufferData[0].mPosition[0] = lineBottom.getX();
			vertexBufferData[0].mPosition[1] = lineBottom.getY();
			vertexBufferData[0].mPosition[2] = lineBottom.getZ();
			vertexBufferData[0].mColor[0]    =   0;
			vertexBufferData[0].mColor[1]    =   0;
			vertexBufferData[0].mColor[2]    = 160;
			vertexBufferData[0].mColor[3]    = 255;
			vertexBufferData[1].mPosition[0] = lineTop.getX();
			vertexBufferData[1].mPosition[1] = lineTop.getY();
			vertexBufferData[1].mPosition[2] = lineTop.getZ();
			vertexBufferData[1].mColor[0]    =   0;
			vertexBufferData[1].mColor[1]    =   0;
			vertexBufferData[1].mColor[2]    = 223;
			vertexBufferData[1].mColor[3]    = 255;
			lineSegments.push_back( vertexBufferData[0] );
			lineSegments.push_back( vertexBufferData[1] );
		}
		if( getConeMeridianZero( &lineTop, &lineBottom, false ) ) {
			grVertexStripeElment vertexBufferData[2];
			vertexBufferData[0].mPosition[0] = lineBottom.getX();
			vertexBufferData[0].mPosition[1] = lineBottom.getY();
			vertexBufferData[0].mPosition[2] = lineBottom.getZ();
			vertexBufferData[0].mColor[0]    = 160;
			vertexBufferData[0].mColor[1]    =   0;
			vertexBufferData[0].mColor[2]    =   0;
			vertexBufferData[0].mColor[3]    = 255;
			vertexBufferData[1].mPosition[0] = lineTop.getX();
			vertexBufferData[1].mPosition[1] = lineTop.getY();
			vertexBufferData[1].mPosition[2] = lineTop.getZ();
			vertexBufferData[1].mColor[0]    = 223;
			vertexBufferData[1].mColor[1]    =   0;
			vertexBufferData[1].mColor[2]    =   0;
			vertexBufferData[1].mColor[3]    = 255;
			lineSegments.push_back( vertexBufferData[0] );
			lineSegments.push_back( vertexBufferData[1] );
		}
	}

	// Sphere
	if( !isUnrolledAroundSphere() ) {
		Vector3D sphereCenter;
		double   sphereRadius;
		Vector3D sphereAxis( 0.0, 1.0, 0.0, 0.0 );
		if( getSphereData( &sphereCenter, &sphereRadius ) ) {
			// Prepare the axis
			Vector3D lineTop    = sphereCenter + sphereAxis * ( sphereRadius * 1.2 );
			Vector3D lineBottom = sphereCenter - sphereAxis * ( sphereRadius * 1.2 );
			grVertexStripeElment vertexBufferData;
			vertexBufferData.mPosition[0] = lineBottom.getX();
			vertexBufferData.mPosition[1] = lineBottom.getY();
			vertexBufferData.mPosition[2] = lineBottom.getZ();
			vertexBufferData.mColor[0]    =   0;
			vertexBufferData.mColor[1]    = 160;
			vertexBufferData.mColor[2]    =   0;
			vertexBufferData.mColor[3]    = 255;
			lineSegments.push_back( vertexBufferData );
			vertexBufferData.mPosition[0] = lineTop.getX();
			vertexBufferData.mPosition[1] = lineTop.getY();
			vertexBufferData.mPosition[2] = lineTop.getZ();
			vertexBufferData.mColor[0]    =   0;
			vertexBufferData.mColor[1]    = 223;
			vertexBufferData.mColor[2]    =   0;
			vertexBufferData.mColor[3]    = 255;
			lineSegments.push_back( vertexBufferData );
			// Meridians:
			double phi;
			getParamFloatMesh( MeshParams::AXIS_PRIMEMERIDIAN, &phi );
			int   stipple = 33;
			// Prepare the prime and cutting meridian:
			vertexBufferData.mColor[0]    =   0;
			vertexBufferData.mColor[1]    =   0;
			vertexBufferData.mColor[2]    = 223;
			vertexBufferData.mColor[3]    = 255;
			for( int i = 0; i <= stipple; i++ ) {
				double theta = i*M_PI/double(stipple);
				vertexBufferData.mPosition[0] = sphereCenter.getX() + sphereRadius * sin(theta)*cos(phi) * 1.001;
				vertexBufferData.mPosition[1] = sphereCenter.getY() + sphereRadius * cos(theta)          * 1.001;
				vertexBufferData.mPosition[2] = sphereCenter.getZ() + sphereRadius * sin(theta)*sin(phi) * 1.001;
				lineSegments.push_back( vertexBufferData );
			}
			// .. and cutting meridian:
			vertexBufferData.mColor[0]    = 223;
			vertexBufferData.mColor[1]    =   0;
			vertexBufferData.mColor[2]    =   0;
			vertexBufferData.mColor[3]    = 255;
			for( int i = 0; i <= stipple; i++ ) {
				double theta = M_PI + i*M_PI/double(stipple);
				vertexBufferData.mPosition[0] = sphereCenter.getX() + sphereRadius * sin(theta)*cos(phi) * 1.001;
				vertexBufferData.mPosition[1] = sphereCenter.getY() + sphereRadius * cos(theta)          * 1.001;
				vertexBufferData.mPosition[2] = sphereCenter.getZ() + sphereRadius * sin(theta)*sin(phi) * 1.001;
				lineSegments.push_back( vertexBufferData );
			}
		}
	}
	// Mesh Plane outline:
	bool showMeshPlane, showMeshPlaneTemp;
	getParamFlagMeshGL( SHOW_MESH_PLANE, &showMeshPlane );
	getParamFlagMeshGL( SHOW_MESH_PLANE_TEMP, &showMeshPlaneTemp );
	if( showMeshPlane || showMeshPlaneTemp ) {
		std::array<GLubyte,3> planeColor;
		if(getPlaneDefinition() == Plane::AXIS_POINTS_AND_POSITION)
		{
			planeColor = {202,178,214};
		}
		else
		{
			planeColor = {255,170,0};
		}

		vector<Vector3D> planeIntersections;
		Vector3D centralPointPlane( 0.0, 0.0, 0.0, 0.0 );
		if( getMeshPlaneFan( &centralPointPlane, &planeIntersections ) && \
			( planeIntersections.size() > 0 ) ) {
			grVertexStripeElment vertexBufferData;
			vertexBufferData.mColor[0]    = planeColor[0];
			vertexBufferData.mColor[1]    = planeColor[1];
			vertexBufferData.mColor[2]    = planeColor[2];
			vertexBufferData.mColor[3]    = 255;
			Vector3D lastPos = planeIntersections.back();
			vertexBufferData.mPosition[0] = lastPos.getX();
			vertexBufferData.mPosition[1] = lastPos.getY();
			vertexBufferData.mPosition[2] = lastPos.getZ();
			for( const auto& nextPos: planeIntersections ) {
				lineSegments.push_back( vertexBufferData );
				vertexBufferData.mPosition[0] = nextPos.getX();
				vertexBufferData.mPosition[1] = nextPos.getY();
				vertexBufferData.mPosition[2] = nextPos.getZ();
				lineSegments.push_back( vertexBufferData );
			}
		}
	}
	// Datum Boxes
	bool showDatumBoxes;
	getParamFlagMeshGL( SHOW_DATUM_BOXES, &showDatumBoxes );
	if( showDatumBoxes ) {
		grVertexStripeElment vertexBufferData;
		vertexBufferData.mColor[0]    =   0;
		vertexBufferData.mColor[1]    =   0;
		vertexBufferData.mColor[2]    =   0;
		vertexBufferData.mColor[3]    = 255;
		for( auto const& currBox : mDatumBoxes ) {
			Vector3D vA,vB,vC,vD,vE,vF,vG,vH;
			vA = currBox->getCoordA();
			vB = currBox->getCoordB();
			vC = currBox->getCoordC();
			vD = currBox->getCoordD();
			vE = currBox->getCoordE();
			vF = currBox->getCoordF();
			vG = currBox->getCoordG();
			vH = currBox->getCoordH();
			// Front: A-B-C-D
			vA.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vB.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			lineSegments.push_back( vertexBufferData );
			vC.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			lineSegments.push_back( vertexBufferData );
			vD.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			lineSegments.push_back( vertexBufferData );
			vA.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			// Back: E-F-G-H
			vE.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vF.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			lineSegments.push_back( vertexBufferData );
			vG.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			lineSegments.push_back( vertexBufferData );
			vH.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			lineSegments.push_back( vertexBufferData );
			vE.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			// Sides: A-E  B-F  C-G  D-H
			vA.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vE.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vB.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vF.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vC.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vG.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vD.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
			vH.get3( reinterpret_cast<float*>(&vertexBufferData.mPosition) );
			lineSegments.push_back( vertexBufferData );
		}
	}
#ifdef LINE_DRAW // Deactivated, because it is (currently) only used for debuging the polygonal selection.
	// Lines used to draw e.g. the prism from polygonal selection
	if( mLines.size() > 0 ) {
		grVertexStripeElment vertexBufferData;
		vertexBufferData.mColor[0]    =   0;
		vertexBufferData.mColor[1]    =   0;
		vertexBufferData.mColor[2]    =   0;
		vertexBufferData.mColor[3]    = 255;
		for( auto const& currLine: mLines ) {
			Vector3D vA = currLine.getA();
			vA.get3( (float*)&vertexBufferData.mPosition );
			lineSegments.push_back( vertexBufferData );
			Vector3D vB = currLine.getB();
			vB.get3( (float*)&vertexBufferData.mPosition );
			lineSegments.push_back( vertexBufferData );
		}
	}
#endif
	// Positions, selected
	vector<std::tuple<Vector3D, Primitive*>> somePositions;
	if( getSelectedPositionLines( &somePositions ) ) {
		double pinSize = 1.0;
		getParamFloatMeshGL(MeshGLParams::PIN_SIZE, &pinSize);
		pinSize *= 3.73789; //see PinRenderer => should be highest point of the Pin, so this gives us the hightes offset for the line
		double lineOffset;
		getParamFloatMeshGL(MeshGLParams::PIN_LINE_HEIGHT, &lineOffset);
		lineOffset *= pinSize;

		grVertexStripeElment somePoint;
		// From colorbewer: 9 classs blue, element 7: 33,113,181
		somePoint.mColor[0] =  33;
		somePoint.mColor[1] = 113;
		somePoint.mColor[2] = 181;
		somePoint.mColor[3] = 255;

		for( auto const& currPos: somePositions ) {
			Vector3D normal = std::get<1>(currPos)->getNormal();
			somePoint.mPosition[0] = std::get<0>(currPos).getX() + normal.getX() * lineOffset;
			somePoint.mPosition[1] = std::get<0>(currPos).getY() + normal.getY() * lineOffset;
			somePoint.mPosition[2] = std::get<0>(currPos).getZ() + normal.getZ() * lineOffset;
			lineSegments.push_back( somePoint );
		}
	}
	// Check, if line segments were added before:
	bool drawLineSegments = (!lineSegments.empty());

	bool drawPolyLines;
	getParamFlagMeshGL( SHOW_POLYLINES, &drawPolyLines );
	drawPolyLines &= ( !mPolyLines.empty() );

	if( !( drawPolyLines || drawLineSegments ) ) {
		return;
	}

	// prepare, when needed
	vboPreparePolylines();

	// Turn off culling as it is done by the shader, but store the flag for later.
	GLboolean cullFace;
	glGetBooleanv( GL_CULL_FACE, &cullFace );
	glDisable( GL_CULL_FACE );
	PRINT_OPENGL_ERROR( "glDisable( GL_CULL_FACE )" );

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	const auto mShaderPolyLines = mShaderManager->getShader(ShaderManager::ShaderName::POLYLINES);
	if(mShaderPolyLines == nullptr)
		return;

	// Lets be shady :)
	if( !mShaderPolyLines->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	// Set the basics:
	shaderSetLocationBasicMatrices( mShaderPolyLines );
	if( drawPolyLines ) {
		shaderSetLocationBasicAttribs( mShaderPolyLines, VBUFF_VERTICES_POLYLINES, static_cast<int>(sizeof(grVertexStripeElment)) );
	}

	// Set the basics for faces (culling/smooth)
	shaderSetLocationBasicFaces( mShaderPolyLines );

	// Set the basic lighting:
	//shaderSetLocationBasicLight( mShaderPolyLines );

	// Set the basic fog:
	shaderSetLocationBasicFog( mShaderPolyLines );

	// Set the mesh plane as clipping plane:
	double clipPlane[4];
	getPlaneHNF( clipPlane );
	mShaderPolyLines->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		mShaderPolyLines->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
	}

	// Material - solid color:
	GLfloat colorTmpStored[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorTmpStored );
	mShaderPolyLines->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Bind the 2D texture used for colorramps and labels!
	//glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] );
	//PRINT_OPENGL_ERROR( "glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] )" );
	mTextureMaps[0]->bind();
	GLint texId;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &texId);
	texId -= GL_TEXTURE0;
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] texId: " << texId << endl;
	// Set the ID of the texture map:
	mShaderPolyLines->setUniformValue( "uLabelTexMap", texId );
	mShaderPolyLines->setUniformValue( "uFuncValTexMap", texId );
	PRINT_OPENGL_ERROR( "Shader setAttributeValue" );

	// Set values for rendering of the labels:
	int labelShift;
	getParamIntMeshGL( MeshGLParams::COLMAP_LABEL_OFFSET, &labelShift );
	mShaderPolyLines->setUniformValue( "uLabelCountOffset", static_cast<GLfloat>(labelShift) );
	bool showLabelsMonoColor;
	getParamFlagMeshGL( MeshGLParams::SHOW_LABELS_MONO_COLOR, &showLabelsMonoColor );
	mShaderPolyLines->setUniformValue( "uLabelSameColor", showLabelsMonoColor );
	GLfloat somemColorSetting[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_SOLID, somemColorSetting );
	mShaderPolyLines->setUniformValue( "uLabelSingleColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BORDER, somemColorSetting );
	mShaderPolyLines->setUniformValue( "uLabelBorderColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_NOT_ASSIGNED, somemColorSetting );
	mShaderPolyLines->setUniformValue( "uLabelNoColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BACKGROUND, somemColorSetting );
	mShaderPolyLines->setUniformValue( "uLabelBackgroundColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Set parameters of the color mapping.
	int colMapID = GLSL_COLMAP_GRAYSCALE;
	getParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, &colMapID );
	mShaderPolyLines->setUniformValue( "uFuncValColorMap", static_cast<GLfloat>(colMapID) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Set the colored range.
	double funcValMin;
	double funcValMax;
	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	mShaderPolyLines->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(funcValMin) );
	mShaderPolyLines->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(funcValMax) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] funcValMin: " << funcValMin << "  funcValMax: " << funcValMax << endl;
	// Inversion of the colormap
	bool funcValInvertMap = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_COLMAP_INVERT, &funcValInvertMap );
	mShaderPolyLines->setUniformValue( "uFuncValInvert", static_cast<GLboolean>(funcValInvertMap) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Repeat colormap
	bool funcValRepeat = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &funcValRepeat );
	mShaderPolyLines->setUniformValue( "uFuncValRepeat", static_cast<GLboolean>(funcValRepeat) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	double funcValRepeatIntervall;
	getParamFloatMeshGL( MeshGLParams::WAVES_COLMAP_LEN, &funcValRepeatIntervall );
	mShaderPolyLines->setUniformValue( "uFuncValIntervall", static_cast<GLfloat>(funcValRepeatIntervall) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Logarithmic color ramp
	double funcValLogGamma;
	getParamFloatMeshGL( MeshGLParams::FUNC_VALUE_LOG_GAMMA, &funcValLogGamma );
	mShaderPolyLines->setUniformValue( "uFuncValLogGamma", static_cast<GLfloat>(funcValLogGamma) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Line width
	double polyLineWidth;
	getParamFloatMeshGL( MeshGLParams::POLYLINE_WIDTH, &polyLineWidth );
	mShaderPolyLines->setUniformValue( "uLineWidth", static_cast<GLfloat>(polyLineWidth) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Isoline extension:
	//----------------------------------
	bool showIsolines;
	getParamFlagMeshGL( MeshGLParams::SHOW_FUNC_VALUES_ISOLINES, &showIsolines );
	mShaderPolyLines->setUniformValue( "funcValIsoLineParams.mIsoLinesShow", static_cast<GLboolean>(showIsolines) );
	if( showIsolines ) {
		double isoDistance;
		double isoOffset;
		double isoPixelWidth;
		getParamFloatMeshGL( MeshGLParams::ISOLINES_DISTANCE, &isoDistance );
		getParamFloatMeshGL( MeshGLParams::ISOLINES_OFFSET, &isoOffset );
		getParamFloatMeshGL( MeshGLParams::ISOLINES_PIXEL_WIDTH, &isoPixelWidth );
		mShaderPolyLines->setUniformValue( "funcValIsoLineParams.mIsoDist",       static_cast<GLfloat>(isoDistance)   );
		mShaderPolyLines->setUniformValue( "funcValIsoLineParams.mIsoOffset",     static_cast<GLfloat>(isoOffset)     );
		mShaderPolyLines->setUniformValue( "funcValIsoLineParams.mIsoPixelWidth", static_cast<GLfloat>(isoPixelWidth) );
	}
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Geometry shader setting:
	//----------------------------------
	bool showPolyNormals;
	getParamFlagMeshGL( MeshGLParams::SHOW_NORMALS_POLYLINE, &showPolyNormals );
	mShaderPolyLines->setUniformValue( "uPolyNormals", static_cast<GLboolean>(showPolyNormals) );
	double normalsLength;
	getParamFloatMeshGL( MeshGLParams::NORMALS_LENGTH, &normalsLength );
	mShaderPolyLines->setUniformValue( "uNormalLength", static_cast<GLfloat>(normalsLength) );
	double normalsWdith;
	getParamFloatMeshGL( MeshGLParams::NORMALS_WIDTH, &normalsWdith );
	mShaderPolyLines->setUniformValue( "uNormalWidth", static_cast<GLfloat>(normalsWdith) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Index data map buffer and draw
	//----------------------------------
	GLfloat faceShiftZ = 0.0;
	if( drawPolyLines ) {
		// More Strided Data -- map buffer
		//----------------------------------
		mVertBufObjs[VBUFF_VERTICES_POLYLINES]->bind();
		PRINT_OPENGL_ERROR( "mVertBufObjs[VBUFF_VERTICES_STRIPED].bind();" );
		// Strided data -- one function value per vertex.
		size_t offSetColors = sizeof(GLfloat)*6;
		mShaderPolyLines->setAttributeBuffer( "vColor", GL_UNSIGNED_BYTE, static_cast<int>(offSetColors), 4, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vColor" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		size_t offSetFuncVal = offSetColors+sizeof(GLubyte)*4;
		mShaderPolyLines->setAttributeBuffer( "vFuncVal", GL_FLOAT, static_cast<int>(offSetFuncVal), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vFuncVal" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		size_t offSetLabelID = offSetFuncVal + sizeof(GLfloat);
		mShaderPolyLines->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vLabelID" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
		mShaderPolyLines->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vFlags" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		mShaderPolyLines->setUniformValue( "uRenderColor", TEXMAP_VERT_FUNCVAL );
		// Material - solid color:
		GLfloat colorTmpStored[4];
		mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorTmpStored );
		mShaderPolyLines->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		// Z-shift
		mShaderPolyLines->setUniformValue( "uFaceShiftViewZ", faceShiftZ );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );
		faceShiftZ -= 0.0001F;
		// Draw
		mVertBufObjs[VBUFF_POLYLINES]->bind();
		glDrawElements( GL_LINES, mVertBufObjs[VBUFF_POLYLINES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
		PRINT_OPENGL_ERROR( "glDrawElements( GL_LINES, mVertBufObjs[VBUFF_POLYLINES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, NULL )" );
		mVertBufObjs[VBUFF_POLYLINES]->release();
	}

	if( drawLineSegments ) {
		// Switch rendering mode solid/color_per_vertex/function_value/labels:
		mShaderPolyLines->setUniformValue( "uRenderColor", TEXMAP_VERT_RGB );
		// Z-shift - None!
		mShaderPolyLines->setUniformValue( "uFaceShiftViewZ", static_cast<GLfloat>(0.0f) );
		PRINT_OPENGL_ERROR( "Shader setUniformValue" );

		QOpenGLBuffer someBuffer( QOpenGLBuffer::VertexBuffer );
		someBuffer.create();
		someBuffer.bind();
		someBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
		someBuffer.allocate( lineSegments.data(), static_cast<int>(sizeof(grVertexStripeElment)*lineSegments.size()) );

		// Strided data -- first there floats are the position vectors.
		mShaderPolyLines->setAttributeBuffer( "position", GL_FLOAT, 0, 3, sizeof(grVertexStripeElment) ); // rShaderLocationBasic->mVertexPos
		mShaderPolyLines->enableAttributeArray( "position" ); // rShaderLocationBasic->mVertexPos
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		size_t offSetColors = sizeof(GLfloat)*6;
		mShaderPolyLines->setAttributeBuffer( "vColor", GL_UNSIGNED_BYTE, static_cast<int>(offSetColors), 4, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vColor" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

		mShaderPolyLines->setAttributeBuffer( "vNormal", GL_FLOAT, sizeof(GLfloat) * 3, 3, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vNormal" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

		size_t offset = offSetColors + 4* sizeof(GLubyte);
		mShaderPolyLines->setAttributeBuffer( "vFuncVal", GL_FLOAT, static_cast<int>(offset), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vFuncVal" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

		offset = offset + sizeof(GLfloat);
		mShaderPolyLines->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offset), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vLabelID" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

		offset = offset + sizeof(GLfloat);
		mShaderPolyLines->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offset), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
		mShaderPolyLines->enableAttributeArray( "vFlags" );
		PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
		// Draw
		glDrawArrays( GL_LINES, 0, static_cast<GLsizei>(lineSegments.size()) );
		PRINT_OPENGL_ERROR( "glDrawArrays( ... )" );

		someBuffer.release();
		someBuffer.destroy();
	}

	// End of being shady
	mShaderPolyLines->release();
	PRINT_OPENGL_ERROR( "mShaderWireframe->release()" );

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	// Restore culling, when necessary.
	if( cullFace ) {
		glEnable( GL_CULL_FACE );
		PRINT_OPENGL_ERROR( "glEnable( GL_CULL_FACE )" );
	}
}

//! Prepares Framebuffers for NPR rendering
void MeshGLShader::prepareFrambuffersNPR()
{
    GLint viewPort[4];
    glGetIntegerv( GL_VIEWPORT, viewPort);

    PglGenFramebuffers genFramebuffersFunc = reinterpret_cast<PglGenFramebuffers>(mOpenGLContext->getProcAddress("glGenFramebuffers"));
    PglBindFramebuffer bindFramebufferFunc = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer"));
    PglFramebufferTexture2D framebufferTexture2DFunc = reinterpret_cast<PglFramebufferTexture2D>(mOpenGLContext->getProcAddress("glFramebufferTexture2D"));

    glGenTextures(7, mNPRFboTextures);
    genFramebuffersFunc(4, mFbosNPR);

    //Render target for Geometry Buffer normals + depth
    glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 viewPort[2], viewPort[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //Render target for Geometry Buffer lighting intensity (toon + hatches)
    glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 viewPort[2], viewPort[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //Render target for raw color buffer
    glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 viewPort[2], viewPort[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //Depth component for Geometry Buffer
    glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 viewPort[2], viewPort[3], 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

    //Output of edge-detection filter
    glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[4]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 viewPort[2], viewPort[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //Output of toon filter
    glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[5]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 viewPort[2], viewPort[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //Output of hatch filter
    glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[6]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 viewPort[2], viewPort[3], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    //create geometry buffer fbo
    bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[0]);
    framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mNPRFboTextures[0], 0);
    framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mNPRFboTextures[1], 0);
    framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mNPRFboTextures[2], 0);
    framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mNPRFboTextures[3], 0);

    PRINT_OPENGL_ERROR( "Error initializing FBO for NPR geometry buffer" );

    //create fbo for sobel filter / edge detection
    bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[1]);
    framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mNPRFboTextures[4], 0);

    PRINT_OPENGL_ERROR( "Error initializing FBO for NPR edge detection" );


    //create fbo for toon filter
    bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[2]);
    framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mNPRFboTextures[5], 0);

    PRINT_OPENGL_ERROR( "Error initializing FBO for NPR toon shading" );


    //create fbo for hatch shading
    bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[3]);
    framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mNPRFboTextures[6], 0);

    PRINT_OPENGL_ERROR( "Error initializing FBO for NPR hatch shading" );

	int defaultFramebuffer;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

	bindFramebufferFunc(GL_FRAMEBUFFER, defaultFramebuffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MeshGLShader::releaseFramebuffersNPR()
{
	// Prevent double release as this will be called quite ofen.
	if( !mIsFboInitialized ) {
		return;
	}

	PglBindFramebuffer bindFramebufferFunc = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer"));
	PglDeleteFramebuffers deleteFramebuffersFunc = reinterpret_cast<PglDeleteFramebuffers>(mOpenGLContext->getProcAddress("glDeleteFramebuffers"));

	int defaultFramebuffer;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

	bindFramebufferFunc(GL_FRAMEBUFFER,defaultFramebuffer);

	deleteFramebuffersFunc(4, mFbosNPR);
	glDeleteTextures(7, mNPRFboTextures);

	PRINT_OPENGL_ERROR( "Error deleting NPR FBO" );

	// See above - prevent double release.
	mIsFboInitialized = false;
}


void MeshGLShader::vboPaintNPR() {
	/*
		Init
	*/

	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	int shaderChoice;
	getParamIntMeshGL( MeshGLParams::SHADER_CHOICE, &shaderChoice );
	bool drawNPR = ( MeshGLParams::SHADER_NPR==shaderChoice );

	if(!drawNPR)
	{
		return;
	}

	double ortho_w = 2 / mMatProjection[0];
	//float ortho_r = 1 / mMatProjection[0] + ortho_w * mMatProjection[12] * 0.5;
	double ortho_h = 2 / mMatProjection[5];
	//float ortho_t = 1 / mMatProjection[5] + ortho_h * mMatProjection[13] * 0.5;
	double ortho_xShift = ortho_w * mMatProjection[12] * 0.5;
	double ortho_yShift = ortho_h * mMatProjection[13] * 0.5;

	//needed for tiled rendering
	int ortho_xOffset = round((ortho_xShift + (ortho_w / 2)) / (ortho_w)) - 1;
	int ortho_yOffset = round((ortho_yShift + (ortho_h / 2)) / (ortho_h)) - 1;

	static int lastWidth = 0;
	static int lastHeight = 0;

	if(!mIsFboInitialized)
	{
		lastWidth = viewPort[2];
		lastHeight = viewPort[3];
		prepareFrambuffersNPR();

		mIsFboInitialized = true;
	}

	/*-----------------------------------------------------------------------*/

	/* check if the size of the fbo is valid
	   if not, rebuild it with the correct size
	*/

	if(lastWidth != viewPort[2] || lastHeight != viewPort[3]  )
	{
		lastWidth = viewPort[2];
		lastHeight = viewPort[3];
		releaseFramebuffersNPR();
		prepareFrambuffersNPR();
	}

	bool boolParam;
	double floatParam;
	int intParam;

	int enableFlags = 0;
	getParamFlagMeshGL( SHOW_NPR_OUTLINES, &boolParam);
	if(boolParam)
		enableFlags |= 1 ;

	getParamFlagMeshGL( SHOW_NPR_HATCHLINES, &boolParam);
	enableFlags |= boolParam << 1;

	getParamFlagMeshGL(SHOW_NPR_TOON, &boolParam);
	enableFlags |= boolParam << 2;

	getParamFlagMeshGL(NPR_HATCHLINE_LIGHT_INFLUENCE, &boolParam);
	enableFlags |= boolParam << 3;



	PglBindFramebuffer bindFramebufferFunc = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer"));
	PglDrawBuffers drawBuffersFunc = reinterpret_cast<PglDrawBuffers>(mOpenGLContext->getProcAddress("glDrawBuffers"));

	PglActiveTexture activeTextureFunc = reinterpret_cast<PglActiveTexture>(mOpenGLContext->getProcAddress("glActiveTexture"));

	GLboolean b_AlphaBlend;
	glGetBooleanv( GL_BLEND, &b_AlphaBlend );

	if(b_AlphaBlend) {
		glDisable(GL_BLEND);
	}

	bool showFacesCulled;
	getParamFlagMeshGL(SHOW_FACES_CULLED,&showFacesCulled);
	if(showFacesCulled) {

		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	vboPrepareVerticesStriped();

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	const auto mShaderNPR_BuildFBO = mShaderManager->getShader(ShaderManager::ShaderName::NPR_BUILD_FBO);
	const auto mShaderNPR_BuildSobel = mShaderManager->getShader(ShaderManager::ShaderName::NPR_BUILD_SOBEL);
	const auto mShaderNPR_toonify = mShaderManager->getShader(ShaderManager::ShaderName::NPR_TOONIFY);
	const auto mShaderNPR_hatching = mShaderManager->getShader(ShaderManager::ShaderName::NPR_HATCHING);
	const auto mShaderNPR_composit = mShaderManager->getShader(ShaderManager::ShaderName::NPR_COMPOSIT);

	if(mShaderNPR_BuildFBO   == nullptr ||
	   mShaderNPR_BuildSobel == nullptr ||
	   mShaderNPR_toonify    == nullptr ||
	   mShaderNPR_hatching   == nullptr ||
	   mShaderNPR_composit   == nullptr)
		return;



	// Build initial geometry buffers
	//------------------------------------

	if( !mShaderNPR_BuildFBO->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	// Set the basics - including strided data:
	//----------------------------------
	shaderSetLocationBasicMatrices( mShaderNPR_BuildFBO );
	shaderSetLocationBasicAttribs( mShaderNPR_BuildFBO, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );
	shaderSetLocationBasicFaces( mShaderNPR_BuildFBO );

	// Set the basic lighting:
	shaderSetLocationBasicLight( mShaderNPR_BuildFBO );
	shaderSetMeshBasicFuncVals( mShaderNPR_BuildFBO );
	// Set the basic fog:
   // shaderSetLocationBasicFog( mShaderNPR_BuildFBO );
	//------------------------------------


	// Set Uniforms :
	//-------------------------------------
	getParamFlagMeshGL( NPR_USE_SPECULAR, &boolParam);
	if(boolParam)
	{
		getParamFlagMeshGL( SHOW_NPR_TOON, &boolParam);
	}
	mShaderNPR_BuildFBO->setUniformValue( "UseSpecular", boolParam ? 1.0f : 0.0f);

	getParamFloatMeshGL( NPR_SPECULAR_SIZE, &floatParam);
	mShaderNPR_BuildFBO->setUniformValue( "HighlightSize", static_cast<float>(floatParam));

	int sourceFlags = 0;
	getParamIntMeshGL( NPR_HATCH_SOURCE, &intParam);
	sourceFlags = sourceFlags | intParam;

	getParamIntMeshGL( NPR_TOON_SOURCE, &intParam);
	sourceFlags = sourceFlags << 1 | intParam;

	getParamIntMeshGL( NPR_OUTLINE_SOURCE, &intParam);
	sourceFlags = sourceFlags << 1 | intParam;

	mShaderNPR_BuildFBO->setUniformValue("flag_useFuncVal", sourceFlags);

	//-------------------------------------

	//mFboNPR->bind();
	bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[0]);
	const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	drawBuffersFunc(3,drawBuffers);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mVertBufObjs[VBUFF_FACES]->bind();
	glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );

	//mFboNPR->release();
	mShaderNPR_BuildFBO->release();
	mVertBufObjs[VBUFF_FACES]->release();
	//-------------------------------------

	//Alternative: Blur geometry-buffer before applying sobel
	//-------------------------------------
	//TODO
	//-------------------------------------

	//Build Sobel Image
	//------------------------------------
	mVertBufObjs[VBUFF_SCREEN_QUAD]->bind();
	if(!mShaderNPR_BuildSobel->bind()) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	activeTextureFunc(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, mFboNPR->texture());
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[0]);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
	mShaderNPR_BuildSobel->setAttributeBuffer("vertPosition", GL_FLOAT, 0, 2, 0);
	PRINT_OPENGL_ERROR( "setAttributeBuffer" );
	mShaderNPR_BuildSobel->enableAttributeArray( "vertPosition");
	PRINT_OPENGL_ERROR( "enableAttributeArray" );

	bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[1]);

	glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);


	mShaderNPR_BuildSobel->setUniformValue( "uFBO_Texture_ID", 0);
	mShaderNPR_BuildSobel->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );

	glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
	PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
	//mVertBufObjs[VBUFF_SCREEN_QUAD]->release();

	int defaultFramebuffer;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

	bindFramebufferFunc(GL_FRAMEBUFFER, defaultFramebuffer);
    mShaderNPR_BuildSobel->release();

	//-------------------------------------

	//Build Toon Color Buffer
	//-------------------------------------
	mShaderNPR_toonify->bind();

	//Bind lighting and color-buffer
	activeTextureFunc(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[1]);

	activeTextureFunc(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[2]);

	mShaderNPR_toonify->setAttributeBuffer("vertPosition", GL_FLOAT, 0,2,0);
	PRINT_OPENGL_ERROR("setAttributeBuffer");
	mShaderNPR_toonify->enableAttributeArray( "vertPosition" );
	PRINT_OPENGL_ERROR("enableAttributeArray");

	bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[2]);

	glClear(GL_COLOR_BUFFER_BIT);

	mShaderNPR_toonify->setUniformValue( "uLightingTexture", 0);
	mShaderNPR_toonify->setUniformValue( "uColorTexture", 1);

	mShaderNPR_toonify->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3]);

	getParamIntMeshGL(MeshGL::NPR_TOON_TYPE, &intParam);
	mShaderNPR_toonify->setUniformValue("uToonFunction", intParam);

	getParamIntMeshGL(MeshGL::NPR_TOON_LIGHTING_STEPS, &intParam);
	mShaderNPR_toonify->setUniformValue("uLightingSteps", static_cast<float>(intParam));

	getParamIntMeshGL(MeshGL::NPR_TOON_HUE_STEPS, &intParam);
	mShaderNPR_toonify->setUniformValue("uHSV_H_Steps", static_cast<float>(intParam));

	getParamIntMeshGL(MeshGL::NPR_TOON_SAT_STEPS, &intParam);
	mShaderNPR_toonify->setUniformValue("uHSV_S_Steps", static_cast<float>(intParam));

	getParamIntMeshGL(MeshGL::NPR_TOON_VAL_STEPS, &intParam);
	mShaderNPR_toonify->setUniformValue("uHSV_V_Steps", static_cast<float>(intParam));

	//Copy uniform color-values into single array to set it in shader:
	//6 * rgb colors => 18 floats
	{
		GLfloat toonColors[18];
		unsigned int currindex = 0;
		MeshGLColors::eColorSettings colorSettings[6] = { MeshGLColors::COLOR_NPR_DIFFUSE5, MeshGLColors::COLOR_NPR_DIFFUSE4, MeshGLColors::COLOR_NPR_DIFFUSE3, MeshGLColors::COLOR_NPR_DIFFUSE2, MeshGLColors::COLOR_NPR_DIFFUSE1, MeshGLColors::COLOR_NPR_SPECULAR };
		for( auto colorSetting : colorSettings ) {
			GLfloat colorTmpStored[4];
			mRenderColors->getColorSettings( colorSetting, colorTmpStored );
			for( int i=0; i<3; ++i ) {
				toonColors[currindex++] = colorTmpStored[i];
			}
		}
		mShaderNPR_toonify->setUniformValueArray( "uToonColors", toonColors, 6, 3 );
	}



	glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
	PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
	//mVertBufObjs[VBUFF_SCREEN_QUAD]->release();


	bindFramebufferFunc(GL_FRAMEBUFFER, defaultFramebuffer);

	mShaderNPR_toonify->release();

	//-------------------------------------

	//Build hatches
	//-------------------------------------


	mShaderNPR_hatching->bind();

	//Bind lighting and color-buffer
	activeTextureFunc(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[1]);

	mShaderNPR_hatching->setUniformValue( "uLightingTexture", 0);

	int hatchStyle = 0;
	getParamIntMeshGL(NPR_HATCH_STYLE, &hatchStyle);

	activeTextureFunc(GL_TEXTURE1);

	glBindTexture(GL_TEXTURE_2D, mTextureHatchlinesNPR[0 + 2* (hatchStyle % 2)]->textureId());
	PRINT_OPENGL_ERROR( "Error binding texture");

	mShaderNPR_hatching->setUniformValue( "uHatchMap0_2" , 1);
	PRINT_OPENGL_ERROR( "Error setting hatchline-sampler 0-2");


	activeTextureFunc(GL_TEXTURE2);

	glBindTexture(GL_TEXTURE_2D, mTextureHatchlinesNPR[1 + 2 * (hatchStyle % 2)]->textureId());
	PRINT_OPENGL_ERROR( "Error binding texture");

	mShaderNPR_hatching->setUniformValue( "uHatchMap3_5" , 2);
	PRINT_OPENGL_ERROR( "Error setting hatchline-sampler 3-5");

	activeTextureFunc(GL_TEXTURE0);


	mShaderNPR_hatching->setAttributeBuffer("vertPosition", GL_FLOAT, 0,2,0);
	PRINT_OPENGL_ERROR("setAttributeBuffer");
	mShaderNPR_hatching->enableAttributeArray( "vertPosition" );
	PRINT_OPENGL_ERROR("enableAttributeArray");

	bindFramebufferFunc(GL_FRAMEBUFFER, mFbosNPR[3]);

	glClear(GL_COLOR_BUFFER_BIT);


	// Check for archaeological illustration mode
	bool archaeologyMode = false;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::ARCHAEOLOGY_MODE_ENABLED, &archaeologyMode );

	if( archaeologyMode ) {
		floatParam = 45.0; // Fixed 45° hatching for archaeological standards (Raczynski-Henk 2017)
	} else {
		getParamFloatMeshGL( NPR_HATCH_ROTATION, &floatParam );
	}

	mShaderNPR_hatching->setUniformValue( "uHatchTextureRotation" , static_cast<float>(floatParam));
	PRINT_OPENGL_ERROR( "Shader set uHatchTextureRotation" );

	getParamFloatMeshGL( NPR_HATCH_SCALE, &floatParam );

	mShaderNPR_hatching->setUniformValue( "uHatchTextureScale" , static_cast<float>(floatParam));
	PRINT_OPENGL_ERROR( "Shader set uHatchTextureScale" );


	mShaderNPR_hatching->setUniformValue( "uHatchMapSize", mTextureHatchlinesNPR[0 + 2 * (hatchStyle % 2)]->width(), mTextureHatchlinesNPR[0 + 2 * (hatchStyle % 2)]->height());
	mShaderNPR_hatching->setUniformValue( "uViewPortShift", ortho_xOffset, ortho_yOffset);

	mShaderNPR_hatching->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3]);

	mShaderNPR_hatching->setUniformValue( "uHatchDitherStyle", hatchStyle);

	// Cortex cross-hatch texture binding (Raczynski-Henk 2017 Section 2.7)
	// Bind cortex texture to texture unit 3
	activeTextureFunc(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mTextureCortexCrossHatch[1]->textureId()); // Use medium density
	PRINT_OPENGL_ERROR( "Error binding cortex texture");
	mShaderNPR_hatching->setUniformValue( "uCortexPattern" , 3);
	PRINT_OPENGL_ERROR( "Error setting cortex-sampler");

	// Cortex region flag (Phase 3: Per-vertex cortex selection)
	// Check UI toggle first
	bool cortexUIToggle = false;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::CORTEX_RENDERING_ENABLED, &cortexUIToggle );

	// If UI toggle is on, use it; otherwise check if any vertices have FLAG_CORTEX set
	bool cortexEnabled = cortexUIToggle;
	if( !cortexEnabled ) {
		// Check if any vertex has cortex flag (for per-vertex selection mode)
		set<Vertex*> cortexVertices;
		if( getVertWithFlag( &cortexVertices, FLAG_CORTEX ) && !cortexVertices.empty() ) {
			cortexEnabled = true;
		}
	}

	mShaderNPR_hatching->setUniformValue( "uIsCortexRegion", cortexEnabled ? 1 : 0);
	PRINT_OPENGL_ERROR( "Shader set uIsCortexRegion");

	// Cortex pattern scale
	mShaderNPR_hatching->setUniformValue( "uCortexScale", 3.0f); // Larger scale for visibility
	PRINT_OPENGL_ERROR( "Shader set uCortexScale");

	activeTextureFunc(GL_TEXTURE0);

	// Archaeological mode already checked above (line 3575), reuse the value
	getParamFloatMeshGL( NPR_OUTLINE_WIDTH, &floatParam );

	if( archaeologyMode ) {
		floatParam *= 2.5; // 2.5x thicker ridges for archaeological standards (Raczynski-Henk 2017, p.30)
	}

	mShaderNPR_hatching->setUniformValue( "uOutlineWidth" , static_cast<float>(floatParam));
	PRINT_OPENGL_ERROR( "Shader set Outline Width" );


	getParamFlagMeshGL(NPR_HATCHLINE_LIGHT_INFLUENCE, &boolParam);
	mShaderNPR_hatching->setUniformValue("uLightInfluence", boolParam);

	GLfloat colorNPRHatchLine[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_NPR_HATCHLINE, colorNPRHatchLine );
	mShaderNPR_hatching->setUniformValue( "uHatchColor", colorNPRHatchLine[0], colorNPRHatchLine[1],
														 colorNPRHatchLine[2], colorNPRHatchLine[3] );

	//set missing uniforms



	glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
	PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );

	bindFramebufferFunc(GL_FRAMEBUFFER, defaultFramebuffer);

	mShaderNPR_hatching->release();


	//-------------------------------------
	//Composit the single buffers and draw them to the screen
	//-------------------------------------

	//bind the output-buffers
	mShaderNPR_composit->bind();
	activeTextureFunc(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[4]);
	mShaderNPR_composit->setUniformValue("uEdgeTexture", 0);

	activeTextureFunc(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[5]);
	mShaderNPR_composit->setUniformValue("uColorTexture", 1);

	activeTextureFunc(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[6]);
	mShaderNPR_composit->setUniformValue("uHatchTexture", 2);

	activeTextureFunc(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mNPRFboTextures[3]);
	mShaderNPR_composit->setUniformValue("uDepthTexture", 3);


	mShaderNPR_composit->setUniformValue("uViewPortSize", viewPort[2], viewPort[3]);

	GLfloat colorNPROutLine[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_NPR_OUTLINE, colorNPROutLine );
	mShaderNPR_composit->setUniformValue( "uOutlineColor", colorNPROutLine[0], colorNPROutLine[1],
														   colorNPROutLine[2] ); // , mColorSetting[COLOR_NPR_OUTLINE][3] / 255.0);

	getParamFloatMeshGL( NPR_OUTLINE_THRESHOLD, &floatParam);
	mShaderNPR_composit->setUniformValue("uOutlineThreshold", static_cast<float>(floatParam));



	// Archaeological mode already checked above (line 3575), reuse the value
	getParamFloatMeshGL( NPR_OUTLINE_WIDTH, &floatParam );

	if( archaeologyMode ) {
		floatParam *= 2.5; // 2.5x thicker ridges for archaeological standards (Raczynski-Henk 2017, p.30)
	}

	mShaderNPR_composit->setUniformValue( "uOutlineWidth" , static_cast<float>(floatParam));

	mShaderNPR_composit->setUniformValue("uEnableFlags", enableFlags);


	mShaderNPR_composit->setAttributeBuffer("vertPosition", GL_FLOAT, 0,2,0);
	PRINT_OPENGL_ERROR("setAttributeBuffer");
	mShaderNPR_composit->enableAttributeArray( "vertPosition" );
	PRINT_OPENGL_ERROR("enableAttributeArray");

	glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
	PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
	mVertBufObjs[VBUFF_SCREEN_QUAD]->release();

	mShaderNPR_composit->release();
	//-------------------------------------


	//release textures and reset original render-settings
	activeTextureFunc(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);

	activeTextureFunc(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	activeTextureFunc(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	activeTextureFunc(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if(showFacesCulled) {
	   glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	if(b_AlphaBlend) {
		glEnable(GL_BLEND);
	}
}

void MeshGLShader::vboPaintTextured()
{

	if(!mTexturedMeshRenderer.isInitialized())
	{
		if(getModelMetaDataRef().getTexturefilesRef().empty()) //menu entry should be disabled in this case.
			return;

		mTexturedMeshRenderer.init(getModelMetaDataRef().getTexturefilesRef());
	}

	if(mMeshTextured == nullptr)
	{
		PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
		glBindVertexArray( mVAO );
		mMeshTextured = new TexturedMesh();
		mMeshTextured->establishStructure(this);
	}

	QMatrix4x4 pmvMatrix( mMatModelView[0], mMatModelView[4], mMatModelView[8],  mMatModelView[12],
						  mMatModelView[1], mMatModelView[5], mMatModelView[9],  mMatModelView[13],
						  mMatModelView[2], mMatModelView[6], mMatModelView[10], mMatModelView[14],
						  mMatModelView[3], mMatModelView[7], mMatModelView[11], mMatModelView[15] );
	QMatrix4x4 ppvMatrix( mMatProjection[0], mMatProjection[4], mMatProjection[8],  mMatProjection[12],
						  mMatProjection[1], mMatProjection[5], mMatProjection[9],  mMatProjection[13],
						  mMatProjection[2], mMatProjection[6], mMatProjection[10], mMatProjection[14],
						  mMatProjection[3], mMatProjection[7], mMatProjection[11], mMatProjection[15] );

	TexturedMeshRenderer::LightInfo lightInfo;
	mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_ENABLED, &lightInfo.lightEnabled);

	//Copy and paste from shaderSetLocationBasicLight
	if(lightInfo.lightEnabled)
	{
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::MATERIAL_SHININESS, &lightInfo.shininess);

		double materialSpecular;
		mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::MATERIAL_SPECULAR, &materialSpecular);

		// Material AND light -- directional -- fixed to camera
		//------------------------------------------------------------------------------------------------------------------------------------------------------
		bool lightFixedCam;
		mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM, &lightFixedCam );
		if( lightFixedCam ) {
			double lightFixedCamPhi;
			double lightFixedCamTheta;
			mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM_ANGLE_PHI,   &lightFixedCamPhi   );
			mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM_ANGLE_THETA, &lightFixedCamTheta );
			Vector3D lightDirCamAngles( lightFixedCamPhi, lightFixedCamTheta, false );
			QVector3D lightDirCam( lightDirCamAngles.getX(), lightDirCamAngles.getY(), lightDirCamAngles.getZ() );
			lightInfo.lightDirFixedCam = lightDirCam;

			double lightFixedCamBright = 0.0F;
			mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_CAM_INTENSITY, &lightFixedCamBright );
			lightInfo.fixedCamDiffuse = QVector4D(1.0F,1.0F,1.0F,1.0F) * lightFixedCamBright;
			lightInfo.fixedCamSpecular = lightInfo.fixedCamDiffuse * materialSpecular;
		}
		lightInfo.fixedCamDiffuse.setW( 1.0F );
		lightInfo.fixedCamSpecular.setW( 1.0F );
		//------------------------------------------------------------------------------------------------------------------------------------------------------

		// Material AND light -- directional -- fixed to world/object
		//------------------------------------------------------------------------------------------------------------------------------------------------------
		bool lightFixedWorld;
		mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD, &lightFixedWorld );
		if( lightFixedWorld ) {
			double lightFixedWorldPhi;
			double lightFixedWorldTheta;
			mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD_ANGLE_PHI,   &lightFixedWorldPhi   );
			mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD_ANGLE_THETA, &lightFixedWorldTheta );
			Vector3D lightDirWorldAngles( lightFixedWorldPhi, lightFixedWorldTheta, false );
			QVector3D lightDirWorld( lightDirWorldAngles.getX(), lightDirWorldAngles.getY(), lightDirWorldAngles.getZ() );
			lightInfo.lightDirFixedWorld = lightDirWorld;

			double lightFixedWorldBright = 0.0f;
			mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::LIGHT_FIXED_WORLD_INTENSITY, &lightFixedWorldBright );
			lightInfo.fixedWorldDiffuse = QVector4D(1.0F,1.0F,1.0F,1.0F) * lightFixedWorldBright;
			lightInfo.fixedWorldSpecular = lightInfo.fixedWorldDiffuse * materialSpecular;
		}
		lightInfo.fixedWorldDiffuse.setW(1.0F);
		lightInfo.fixedWorldSpecular.setW(1.0F);
		//------------------------------------------------------------------------------------------------------------------------------------------------------

		// Light -- ambient
		bool lightAmbient;
		mWidgetParams->getParamFlagMeshWidget( MeshWidgetParams::LIGHT_AMBIENT, &lightAmbient );
		if( lightAmbient ) { // Ambient on => fetch brightness.
			double lightAmbientAmount;
			mWidgetParams->getParamFloatMeshWidget( MeshWidgetParams::AMBIENT_LIGHT, &lightAmbientAmount );
			lightInfo.ambient = QVector4D(1.0F,1.0F,1.0F,1.0F) * lightAmbientAmount;
		}
		lightInfo.ambient.setW( 1.0f );
	}

	// better: do this section only when changes are made:
	// --------------------------------------------------
	bool showBackfaces;
	MeshGLParams::getParamFlagMeshGL(SHOW_FACES_CULLED, &showBackfaces);

	mTexturedMeshRenderer.setCullBackfaces(!showBackfaces);
	if(showBackfaces)
	{
		float backFaceColor[4];
		mRenderColors->getColorSettings(MeshGLColors::COLOR_MESH_BACKFACE, backFaceColor);
		mTexturedMeshRenderer.setBackFaceColor(QVector3D(backFaceColor[0], backFaceColor[1], backFaceColor[2]));
	}
	// ----------------------------------------------------
	PRINT_OPENGL_ERROR("unknown error");

	// Set the mesh plane as clipping plane:
	double clipPlane[4];
	getPlaneHNF( clipPlane );
	lightInfo.clipPlane = QVector4D(clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3]);
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		lightInfo.clipVertexPos = QVector3D( selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
	}
	else
	{
		lightInfo.clipVertexPos = QVector3D(0.0,0.0,0.0);
	}

	mTexturedMeshRenderer.render(ppvMatrix, pmvMatrix, *mMeshTextured, lightInfo);

}

void MeshGLShader::vboPaintLightingOverlay()
{
	const auto mShaderLightToFBO            = mShaderManager->getShader(ShaderManager::ShaderName::LIGHTING_LIGHT_TO_FBO);
	const auto mShaderPaintLightningOverlay = mShaderManager->getShader(ShaderManager::ShaderName::LIGHTING_PAINT_OVERLAY);

	if(mShaderLightToFBO == nullptr || mShaderPaintLightningOverlay)
		return;

	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	//c&p of NPR...
	if(mFboOverlay == nullptr)
	{
		QOpenGLFramebufferObjectFormat fboFormat;
		fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
		mFboOverlay = new QOpenGLFramebufferObject(viewPort[2], viewPort[3],fboFormat);
	}

	if(mFboOverlay->width() != viewPort[2] || mFboOverlay->height() != viewPort[3])
	{
		QOpenGLFramebufferObjectFormat fboFormat;
		fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);

		delete mFboOverlay;
		mFboOverlay = new QOpenGLFramebufferObject(viewPort[2], viewPort[3], fboFormat);
	}

	bool showFacesCulled;
	getParamFlagMeshGL(SHOW_FACES_CULLED,&showFacesCulled);
	if(showFacesCulled) {

		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	vboPrepareVerticesStriped();

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	PglActiveTexture activeTextureFunc = reinterpret_cast<PglActiveTexture>(mOpenGLContext->getProcAddress("glActiveTexture"));
	glBindVertexArray( mVAO );


	mFboOverlay->bind();
	if(!mShaderLightToFBO->bind())
	{
			   cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	shaderSetLocationBasicMatrices(mShaderLightToFBO);
	shaderSetLocationBasicAttribs(mShaderLightToFBO, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );
	shaderSetLocationBasicFaces(mShaderLightToFBO);
	shaderSetLocationBasicLight(mShaderLightToFBO);

	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mVertBufObjs[VBUFF_FACES]->bind();

	glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );

	mFboOverlay->release();

	PglBindFramebuffer bindFrameBuffer = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer"));

	int defaultFramebuffer = 0;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);
	bindFrameBuffer(GL_FRAMEBUFFER, defaultFramebuffer);

	mShaderLightToFBO->release();
	mVertBufObjs[VBUFF_FACES]->release();

	//------------------------------------------------------------------------

	//Draw the overlay:
	mVertBufObjs[VBUFF_SCREEN_QUAD]->bind();

	if(!mShaderPaintLightningOverlay->bind())
	{
		 cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	activeTextureFunc(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mFboOverlay->texture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	mShaderPaintLightningOverlay->setAttributeBuffer("vertPosition", GL_FLOAT, 0, 2, 0);
	PRINT_OPENGL_ERROR( "setAttributeBuffer" );
	mShaderPaintLightningOverlay->enableAttributeArray( "vertPosition");
	PRINT_OPENGL_ERROR( "enableAttributeArray" );

	mShaderPaintLightningOverlay->setUniformValue( "uFBO_Texture_ID", 0);
	mShaderPaintLightningOverlay->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );

	double floatParam;

	getParamFloatMeshGL(BADLIT_UPPER_THRESHOLD, &floatParam);
	mShaderPaintLightningOverlay->setUniformValue( "upperThreshold", static_cast<float>(floatParam));
	getParamFloatMeshGL(BADLIT_LOWER_THRESHOLD, &floatParam);
	mShaderPaintLightningOverlay->setUniformValue( "lowerThreshold", static_cast<float>(floatParam));

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_CULL_FACE);

	glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));

	glDisable(GL_CULL_FACE);
	mShaderPaintLightningOverlay->release();
	glBindTexture(GL_TEXTURE_2D,0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(1.0,1.0,1.0,1.0);
}

void MeshGLShader::vboPaintPins(std::vector<PinRenderer::PinVertex> &singlePoints)
{
	double pinSize = 1.0;
	getParamFloatMeshGL(MeshGLParams::PIN_SIZE, &pinSize);

	if(!mPinRenderer.isInitialized())
	{
		mPinRenderer.init();
	}

	QMatrix4x4 pmvMatrix( mMatModelView[0], mMatModelView[4], mMatModelView[8],  mMatModelView[12],
						  mMatModelView[1], mMatModelView[5], mMatModelView[9],  mMatModelView[13],
						  mMatModelView[2], mMatModelView[6], mMatModelView[10], mMatModelView[14],
						  mMatModelView[3], mMatModelView[7], mMatModelView[11], mMatModelView[15] );
	QMatrix4x4 ppvMatrix( mMatProjection[0], mMatProjection[4], mMatProjection[8],  mMatProjection[12],
						  mMatProjection[1], mMatProjection[5], mMatProjection[9],  mMatProjection[13],
						  mMatProjection[2], mMatProjection[6], mMatProjection[10], mMatProjection[14],
						  mMatProjection[3], mMatProjection[7], mMatProjection[11], mMatProjection[15] );

	mPinRenderer.render(singlePoints, ppvMatrix, pmvMatrix, pinSize);
}

void MeshGLShader::vboPaintTransparencyABuffer()
{
#ifdef GL_SHADER_STORAGE_BUFFER
	int drawTransparency;
	getParamIntMeshGL(MeshGLParams::SHADER_CHOICE, &drawTransparency);
	static int lastWidth = 0;
	static int lastHeight = 0;
	 if(drawTransparency != MeshGLParams::SHADER_TRANSPARENCY)
	 {
		 return;
	 }

	 if(!(mOpenGLContext->format().majorVersion() >= 4) || (mOpenGLContext->format().majorVersion() == 4 && mOpenGLContext->format().minorVersion() < 3))
	 {
		 cerr << "[MeshGLShader::" << __FUNCTION__ << "] Error: Opengl 4.3 or higher required for tranparency rendering: need ssbo support" << std::endl;
		 return;
	 }

	 const auto mShaderTransparencyABFill = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AB_FILL);
	 const auto mShaderTransparencyABClear = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AB_CLEAR);
	 const auto mShaderTransparencyABRender = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AB_RENDER);
	 const auto mShaderTransparencyCountFrags = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AB_FRAGCOUNT);
	 if( mShaderTransparencyABFill     == nullptr ||
	     mShaderTransparencyABClear    == nullptr ||
	     mShaderTransparencyABRender   == nullptr ||
	     mShaderTransparencyCountFrags == nullptr)
		return;

	 //Disable multisampling, as it creates multiple fragments at triangle seams, currently not handled correctly on shader
	 glDisable(GL_MULTISAMPLE);

	 if(!mTransIsInitialized)
	 {
			 mGL4_3Functions.glGenQueries(1, &mTransFragmentQuery);
			 PRINT_OPENGL_ERROR( "Problem initializing Query" );
			 mTransAviableFragments = 0;
			 mTransIsInitialized = 2;
			 lastWidth = 0;
			 lastHeight = 0;
			 //fragCounts is used to count the number of generated fragments for insertion in the list
			 //SSBOs: 0 = fragment-counts, 1 = fragment-data, 2 = listheads
			 mGL4_3Functions.glGenBuffers(3, mSSBOs.data());
			 mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSSBOs[0]);
			 mGL4_3Functions.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLint), nullptr, GL_DYNAMIC_COPY);

			 mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			 PRINT_OPENGL_ERROR( "Problem initializing FragCounts" );
	 }

	 /*
	  * Add minimal render pass to count fragments
	  *
	  */

	 PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));

	 glBindVertexArray( mVAO);

	 bool showFacesCulled;
	 getParamFlagMeshGL(SHOW_FACES_CULLED,&showFacesCulled);
	 if(showFacesCulled) {

		 glDisable(GL_CULL_FACE);
	 }
	 else {
		 glEnable(GL_CULL_FACE);
	 }
	//disable color writing, and writing to the depth-buffer
	//we just want to count the fragments, that pass the depth-test
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	 //draw object with minimal shader

	 vboPrepareVerticesStriped();

	 mVertBufObjs[VBUFF_FACES]->bind();

	 if( !mShaderTransparencyCountFrags->bind() ) {
		 cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	 }

	 shaderSetLocationBasicMatrices( mShaderTransparencyCountFrags );
	 shaderSetLocationBasicAttribs( mShaderTransparencyCountFrags, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );

	 mGL4_3Functions.glBeginQuery(GL_SAMPLES_PASSED, mTransFragmentQuery);

	 glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	 PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );

	 mGL4_3Functions.glEndQuery(GL_SAMPLES_PASSED);

	 GLuint QueryResult = GL_FALSE;

	 while(QueryResult == GL_FALSE)
	 {
		 mGL4_3Functions.glGetQueryObjectuiv(mTransFragmentQuery, GL_QUERY_RESULT_AVAILABLE, &QueryResult);
	 }

	 GLuint fragCount;
	 mGL4_3Functions.glGetQueryObjectuiv(mTransFragmentQuery, GL_QUERY_RESULT, &fragCount);
	fragCount /= mOpenGLContext->format().samples(); // required, because the query still uses for some reason QT's multisampling...

   // glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
   // glDepthMask(GL_TRUE);


	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	 //height or width has changed --> reinitialize list heads
	 if(viewPort[2] != lastWidth || viewPort[3] != lastHeight)
	 {
		 lastWidth = viewPort[2];
		 lastHeight =  viewPort[3];

		 mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSSBOs[2]);
		 mGL4_3Functions.glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(GLint) * viewPort[2] * viewPort[3], nullptr, GL_DYNAMIC_COPY);
		 mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		 PRINT_OPENGL_ERROR( "Problem changing Locks size" );


		 //-----------------------------------------------------------------------
		 //init transparency buffers
		 glEnable(GL_CULL_FACE);
		 //bind the quad data

		 if(!mVertBufObjs[VBUFF_SCREEN_QUAD]->bind())
		 {
			 PRINT_OPENGL_ERROR( "glBindVertexArray( VBUFF_SCREEN_QUAD )" );
		 }

		 glDisable(GL_DEPTH_TEST); //no depth-testing for clearing

		 if( !mShaderTransparencyABClear->bind() ) {
			 cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
		 }

		// mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, mSSBOFragCounts );
		 //mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mSSBOFragBuffer );
		 mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSSBOs[2] );

		 mShaderTransparencyABClear->setAttributeBuffer("vertPosition", GL_FLOAT, 0, 2, 0);
		 PRINT_OPENGL_ERROR( "setAttributeBuffer" );
		 mShaderTransparencyABClear->enableAttributeArray( "vertPosition");
		 PRINT_OPENGL_ERROR( "enableAttributeArray" );

		 mShaderTransparencyABClear->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );

		 glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
		 PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
		 mShaderTransparencyABClear->release();
		 mVertBufObjs[VBUFF_SCREEN_QUAD]->release();
		 //-----------------------------------------------------------------------------------------------------
		mGL4_3Functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	 }
	 //listSize too small --> resize fragment list
	 if(fragCount > mTransAviableFragments)
	 {
			mTransAviableFragments = fragCount;
			 mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSSBOs[1]);

			 //std430 rules pad structures to align to its biggest structure member. since we have a vec4 and a float it is padded to 2*vec4 = sizeof(GLfloat) * 8
			 mGL4_3Functions.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 3 * mTransAviableFragments, nullptr, GL_DYNAMIC_COPY);

			 std::cout << "Transparency (AB) memory used: " << (sizeof(GLfloat) * 3 * mTransAviableFragments + 2 * sizeof(GLint) * viewPort[2] * viewPort[3]) / (1024 * 1024)  << " MB used." << std::endl;
	 }



	 //---------------------------------------------------------------------------------------------
	 //Fill Transparency Buffers with data

	 //reset counter
	 mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSSBOs[0]);
	 //GLuint zero = 0;
	 mGL4_3Functions.glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R32UI,GL_RED,GL_UNSIGNED_INT, nullptr);
	 mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	 glEnable(GL_DEPTH_TEST); //cull faces behind opaque ones
	 if(showFacesCulled) {

		 glDisable(GL_CULL_FACE);
	 }
	 else {
		 glEnable(GL_CULL_FACE);
	 }

	 vboPrepareVerticesStriped();

	 mVertBufObjs[VBUFF_FACES]->bind();

	 if(!mShaderTransparencyABFill->bind())
	 {
		 cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	 }

	 //add the default shader stuff, exluding the geometry-shader stuff -> no normals rendering, no wireframes...
	 //largely copy and paste from the "vboPaintFacesIndexed()" function
	 //-------------------------------------------


	 // Set the basics - including strided data:
	 //----------------------------------
	 shaderSetMeshBasicFuncVals(mShaderTransparencyABFill);


	 //-------------------------------------------

	 //add transparancy and ssbo stuff
	 //------------------------------------
	 mShaderTransparencyABFill->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );
	 double floatParam;
	 getParamFloatMeshGL(TRANSPARENCY_UNIFORM_ALPHA, &floatParam);
	 mShaderTransparencyABFill->setUniformValue( "uUniformAlpha", static_cast<float>(floatParam));

	 getParamFloatMeshGL(TRANSPARENCY_ALPHA2, &floatParam);
	 mShaderTransparencyABFill->setUniformValue( "uAlpha2", static_cast<float>(floatParam));

	 getParamFloatMeshGL(TRANSPARENCY_GAMMA, &floatParam);
	 mShaderTransparencyABFill->setUniformValue("uGamma", static_cast<float>(floatParam));

	 //mShaderTransparencyABFill->setUniformValue("NUM_LAYERS", numLayers);
	mShaderTransparencyABFill->setUniformValue("uMaxFragments", mTransAviableFragments);
	 int intParam;
	 getParamIntMeshGL(TRANSPARENCY_TRANS_FUNCTION, &intParam);
	 mShaderTransparencyABFill->setUniformValue("uTransparencyType", intParam);

	 getParamIntMeshGL(TRANSPARENCY_SEL_LABEL, &intParam);
	 mShaderTransparencyABFill->setUniformValue("uTransLabelNr", static_cast<float>(intParam));

	 /*getParamIntMeshGL(TRANSPARENCY_OVERFLOW_HANDLING, &intParam);
	 mShaderTransparencyABFill->setUniformValue("uOverflowHandling", intParam);*/

	 mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSSBOs[0] );
	 mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSSBOs[1] );
	 mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSSBOs[2] );
	 //------------------------------------
	 //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	 //mVertBufObjs[VBUFF_FACES]->bind();
	 glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	 PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );

	 mShaderTransparencyABFill->release();
	 mVertBufObjs[VBUFF_FACES]->release();
	 glEnable(GL_DEPTH_TEST);
	mGL4_3Functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	 //---------------------------------------------------------------------------------------------

	 //draw other stuff for transparency...


	 //---------------------------------------------------------------------------------------------
	 //Render Transparency Buffers
	 glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	 glDepthMask(GL_TRUE);
	 glEnable(GL_CULL_FACE); // we need only the front faces of the fullscreen-quad
	 //glDisable(GL_DEPTH_TEST); //need to disable it to draw the screenquad
	 glDepthFunc(GL_ALWAYS); //see vboPaintTransparencyKPBuffer()
	 glEnable(GL_BLEND);     //blend with opaque objects in the scene
	 if(!mVertBufObjs[VBUFF_SCREEN_QUAD]->bind())
	 {
		 PRINT_OPENGL_ERROR( "glBindVertexArray( VBUFF_SCREEN_QUAD )" );
	 }

	 if(!mShaderTransparencyABRender->bind())
	 {
		 cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	 }

	 //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	 mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSSBOs[1] );
	 mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSSBOs[2] );

	 mShaderTransparencyABRender->setAttributeBuffer("vertPosition", GL_FLOAT, 0, 2, 0);
	 PRINT_OPENGL_ERROR( "setAttributeBuffer" );
	 mShaderTransparencyABRender->enableAttributeArray( "vertPosition");
	 PRINT_OPENGL_ERROR( "enableAttributeArray" );

	 mShaderTransparencyABRender->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );
	 //mShaderTransparencyABRender->setUniformValue("NUM_LAYERS", numLayers);

	 glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
	 PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
	 mShaderTransparencyABRender->release();
	 //----------------------------------------------------------------------------------------------
	mGL4_3Functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	 //CleanUp

	 mVertBufObjs[VBUFF_SCREEN_QUAD]->release();
	 glBindVertexArray(0);
	 PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	 if(showFacesCulled) {

		 glDisable(GL_CULL_FACE);
	 }
	 else {
		 glEnable(GL_CULL_FACE);
	 }

	 glEnable(GL_MULTISAMPLE);
	 glDepthFunc(GL_LESS);
#endif
}

void MeshGLShader::vboPaintTransparencyALBuffer()
{
#ifdef GL_SHADER_STORAGE_BUFFER
	int drawTransparency;
	getParamIntMeshGL(MeshGLParams::SHADER_CHOICE, &drawTransparency);

	if(drawTransparency != MeshGLParams::SHADER_TRANSPARENCY)
	{
		return;
	}

	int numLayers = 10;
	getParamIntMeshGL(TRANSPARENCY_NUM_LAYERS, &numLayers);

	int overflowHandling;
	getParamIntMeshGL(TRANSPARENCY_OVERFLOW_HANDLING, &overflowHandling);
   static int lastWidth = 0;
   static int lastHeight = 0;
   static int lastoverflowHandling = 0;


	if(!(mOpenGLContext->format().majorVersion() >= 4) || (mOpenGLContext->format().majorVersion() == 4 && mOpenGLContext->format().minorVersion() < 3))
	{
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] Error: Opengl 4.3 or higher required for tranparency rendering: need ssbo support" << std::endl;
		return;
	}

	const auto mShaderTransparencyALFill = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AL_FILL);
	const auto mShaderTransparencyALClear = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AL_CLEAR);
	const auto mShaderTransparencyALRender = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AL_RENDER);
	const auto mShaderTransparencyALDepthCollect = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_AL_DEPTH_COLLECT);
	if( mShaderTransparencyALFill         == nullptr ||
	    mShaderTransparencyALClear        == nullptr ||
	    mShaderTransparencyALRender       == nullptr ||
	    mShaderTransparencyALDepthCollect == nullptr)
	   return;

	//Disable multisampling, as it creates multiple fragments at triangle seams, currently not handled correctly on shader
	glDisable(GL_MULTISAMPLE);

	if(!mTransIsInitialized)
	{
			mCurrentNumLayers = 0;
			mTransIsInitialized = 3;
			lastWidth = 0;
			lastHeight = 0;
			//SSBOs: 0 = depth data, 1 = fragment data
			mGL4_3Functions.glGenBuffers(2, mSSBOs.data());
			PRINT_OPENGL_ERROR( "Problem initializing SSBOs" );

			transparencyPrepareFBO();
	}

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));

	glBindVertexArray( mVAO);

	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	//height or width has changed --> reinitialize buffers
	if(viewPort[2] != lastWidth || viewPort[3] != lastHeight || numLayers != mCurrentNumLayers
			|| lastoverflowHandling != overflowHandling)
	{
		//viewport size change, update tail fbo
		if(numLayers == mCurrentNumLayers)
		{
			transparencyDeleteFBO();
			transparencyPrepareFBO();
		}

		mCurrentNumLayers = numLayers;
		lastWidth = viewPort[2];
		lastHeight = viewPort[3];
		lastoverflowHandling = overflowHandling;

		mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSSBOs[0]);
		mGL4_3Functions.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * viewPort[2] * viewPort[3] * numLayers, nullptr, GL_DYNAMIC_COPY);

		mGL4_3Functions.glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSSBOs[1]);
		//packed
		mGL4_3Functions.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * viewPort[2] * viewPort[3] * numLayers, nullptr, GL_DYNAMIC_COPY);
		mGL4_3Functions.glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_R32UI,GL_RED,GL_UNSIGNED_INT,nullptr);
		//not packed
		//mGL4_3Functions.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * 4 * viewPort[2] * viewPort[3] * numLayers, NULL, GL_DYNAMIC_COPY);
		//mGL4_3Functions.glClearBufferData(GL_SHADER_STORAGE_BUFFER,GL_RGBA32F,GL_RGBA,GL_FLOAT,NULL);

		PRINT_OPENGL_ERROR( "Error Clearing Buffer" );
		//-----------------------------------------------------------------------
		//init transparency buffers
		glEnable(GL_CULL_FACE);
		//bind the quad data


		if(!mVertBufObjs[VBUFF_SCREEN_QUAD]->bind())
		{
			PRINT_OPENGL_ERROR( "glBindVertexArray( VBUFF_SCREEN_QUAD )" );
		}

		glDisable(GL_DEPTH_TEST); //no depth-testing for clearing

		if( !mShaderTransparencyALClear->bind() ) {
			cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
		}

		mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSSBOs[0] );

		mShaderTransparencyALClear->setAttributeBuffer("vertPosition", GL_FLOAT, 0, 2, 0);
		PRINT_OPENGL_ERROR( "setAttributeBuffer" );
		mShaderTransparencyALClear->enableAttributeArray( "vertPosition");
		PRINT_OPENGL_ERROR( "enableAttributeArray" );

		mShaderTransparencyALClear->setUniformValue( "NUM_LAYERS" , numLayers);
		mShaderTransparencyALClear->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );

		glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
		PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
		mShaderTransparencyALClear->release();
		mVertBufObjs[VBUFF_SCREEN_QUAD]->release();

		std::cout << "Transparency (Atomic Loop) memory used: " << ((sizeof(GLuint) + sizeof(GLuint))  * numLayers * viewPort[2] * viewPort[3]) / (1024 * 1024) << " MB used." << std::endl;
		mGL4_3Functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glEnable(GL_DEPTH_TEST);
		//-----------------------------------------------------------------------------------------------------

	}

	/*
	 * Add minimal render pass to get depth-values
	 *
	 */
	//----------------------------------------------------------------------------


	//bind fbo for tail-blending
	mGL4_3Functions.glBindFramebuffer(GL_FRAMEBUFFER, mTransFbo);

	const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	mGL4_3Functions.glDrawBuffers(2, drawBuffers);

	float clearColorZero[4] = { 0.f, 0.f, 0.f, 0.f };
	float clearColorOne[4]  = { 1.f, 1.f, 1.f, 1.f };

	mGL4_3Functions.glClearBufferfv(GL_COLOR, 0, clearColorZero);
	mGL4_3Functions.glClearBufferfv(GL_COLOR, 1, clearColorOne);
	mGL4_3Functions.glClear(GL_DEPTH_BUFFER_BIT);
	mGL4_3Functions.glEnable(GL_BLEND);
	//mGL4_3Functions.glBlendEquation(GL_FUNC_ADD);
	mGL4_3Functions.glBlendFunci(0, GL_ONE, GL_ONE);
	mGL4_3Functions.glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

	bool showFacesCulled;
	getParamFlagMeshGL(SHOW_FACES_CULLED,&showFacesCulled);
	if(showFacesCulled) {

		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	//disable color writing, and writing to the depth-buffer
	//we just want to count the fragments, that pass the depth-test
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glDepthMask(GL_FALSE);

	vboPrepareVerticesStriped();

	mVertBufObjs[VBUFF_FACES]->bind();

	if( !mShaderTransparencyALDepthCollect->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	shaderSetLocationBasicMatrices( mShaderTransparencyALDepthCollect );
	shaderSetLocationBasicAttribs( mShaderTransparencyALDepthCollect, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)) );

	mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSSBOs[0] );

	mShaderTransparencyALDepthCollect->setUniformValue( "NUM_LAYERS" , numLayers);
	mShaderTransparencyALDepthCollect->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );

	double clipPlane[4];
	getPlaneHNF( clipPlane );
	mShaderTransparencyALDepthCollect->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue: uClipPlane0" );

	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		mShaderTransparencyALDepthCollect->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
		PRINT_OPENGL_ERROR( "Shader setUniformValue: uClipBefore" );
	}

	glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );
	mGL4_3Functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	mShaderTransparencyALDepthCollect->release();
   glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
   glDepthMask(GL_TRUE);
	//-----------------------------------------------------------------------------------


	//---------------------------------------------------------------------------------------------
	//Fill Transparency Buffers with data
	//glEnable(GL_DEPTH_TEST); //cull faces behind opaque ones
	/*if(showFacesCulled) {
		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}
	*/
	vboPrepareVerticesStriped();

	mVertBufObjs[VBUFF_FACES]->bind();

	if(!mShaderTransparencyALFill->bind())
	{
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	//add the default shader stuff, exluding the geometry-shader stuff -> no normals rendering, no wireframes...
	//largely copy and paste from the "vboPaintFacesIndexed()" function
	//-------------------------------------------


	// Set the basics - including strided data:
	//----------------------------------
	shaderSetMeshBasicFuncVals(mShaderTransparencyALFill);


	//-------------------------------------------

	//add transparancy and ssbo stuff
	//------------------------------------
	mShaderTransparencyALFill->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );

	double floatParam;
	getParamFloatMeshGL(TRANSPARENCY_UNIFORM_ALPHA, &floatParam);
	mShaderTransparencyALFill->setUniformValue( "uUniformAlpha", static_cast<float>(floatParam));

	getParamFloatMeshGL(TRANSPARENCY_ALPHA2, &floatParam);
	mShaderTransparencyALFill->setUniformValue( "uAlpha2", static_cast<float>(floatParam));

	getParamFloatMeshGL(TRANSPARENCY_GAMMA, &floatParam);
	mShaderTransparencyALFill->setUniformValue("uGamma", static_cast<float>(floatParam));


	mShaderTransparencyALFill->setUniformValue("NUM_LAYERS", numLayers);
	int intParam;
	getParamIntMeshGL(TRANSPARENCY_TRANS_FUNCTION, &intParam);
	mShaderTransparencyALFill->setUniformValue("uTransparencyType", intParam);

	getParamIntMeshGL(TRANSPARENCY_SEL_LABEL, &intParam);
	mShaderTransparencyALFill->setUniformValue("uTransLabelNr", static_cast<float>(intParam));

	mShaderTransparencyALFill->setUniformValue("uOverflowHandling", overflowHandling);

	mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSSBOs[0] );
	mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSSBOs[1] );
	//mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mSSBOFragLocks );
	//------------------------------------
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//mVertBufObjs[VBUFF_FACES]->bind();
	glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );

	mShaderTransparencyALFill->release();
	mVertBufObjs[VBUFF_FACES]->release();
	mGL4_3Functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glEnable(GL_DEPTH_TEST);


	//---------------------------------------------------------------------------------------------

	//draw other stuff for transparency...


	//---------------------------------------------------------------------------------------------
	//Render Transparency Buffers
	glEnable(GL_CULL_FACE); // we need only the front faces of the fullscreen-quad
	//glDisable(GL_DEPTH_TEST); //need to disable it to draw the screenquad
	glDepthFunc(GL_ALWAYS); //see vboPaintTransparencyKPBuffer()
	glEnable(GL_BLEND);     //blend with opaque objects in the scene
	if(!mVertBufObjs[VBUFF_SCREEN_QUAD]->bind())
	{
		PRINT_OPENGL_ERROR( "glBindVertexArray( VBUFF_SCREEN_QUAD )" );
	}

	if(!mShaderTransparencyALRender->bind())
	{
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	//reset to normal draw buffers
	mGL4_3Functions.glBlendFunci(1, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	mGL4_3Functions.glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int defaultFramebuffer;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

	mGL4_3Functions.glBindFramebuffer(GL_FRAMEBUFFER,defaultFramebuffer);
	PRINT_OPENGL_ERROR( "Error reverting back to default buffer");
	//mGL4_3Functions.glDrawBuffer(GL_BACK);
	PRINT_OPENGL_ERROR( "Error setting draw buffer to GL_BACK");



	mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSSBOs[0] );
	mGL4_3Functions.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSSBOs[1] );
	mShaderTransparencyALRender->setAttributeBuffer("vertPosition", GL_FLOAT, 0, 2, 0);
	PRINT_OPENGL_ERROR( "setAttributeBuffer" );
	mShaderTransparencyALRender->enableAttributeArray( "vertPosition");
	PRINT_OPENGL_ERROR( "enableAttributeArray" );

	mShaderTransparencyALRender->setUniformValue( "uViewPortSize", viewPort[2], viewPort[3] );
	mShaderTransparencyALRender->setUniformValue("NUM_LAYERS", numLayers);
	mShaderTransparencyALRender->setUniformValue("uOverflowHandling", overflowHandling);


	mGL4_3Functions.glActiveTexture(GL_TEXTURE0);
	mGL4_3Functions.glBindTexture(GL_TEXTURE_RECTANGLE,mTransFboTextures[0]);

	mGL4_3Functions.glActiveTexture(GL_TEXTURE1);
	mGL4_3Functions.glBindTexture(GL_TEXTURE_RECTANGLE,mTransFboTextures[1]);

	mShaderTransparencyALRender->setUniformValue( "ColorTex0", 0 );
	mShaderTransparencyALRender->setUniformValue("ColorTex1",1);

	glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
	PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
	mShaderTransparencyALRender->release();
	//----------------------------------------------------------------------------------------------

	//CleanUp
	mGL4_3Functions.glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	mVertBufObjs[VBUFF_SCREEN_QUAD]->release();
	glBindVertexArray(0);
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	if(showFacesCulled) {

		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LESS);

	mGL4_3Functions.glBindTexture(GL_TEXTURE_2D, 0);
	mGL4_3Functions.glActiveTexture(GL_TEXTURE0);
	mGL4_3Functions.glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

void MeshGLShader::vboPaintTransparencyWAVG()
{

	const auto mShaderTransparencyGeomWOIT  = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_WOIT_GEOM);
	const auto mShaderTransparencyBlendWOIT = mShaderManager->getShader(ShaderManager::ShaderName::TRANSPARENCY_WOIT_BLEND);

	if( mShaderTransparencyGeomWOIT  == nullptr ||
	    mShaderTransparencyBlendWOIT == nullptr )
	   return;

	static int lastWidth = 0;
	static int lastHeight = 0;

	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);

	if(!mTransIsInitialized)
	{
			mTransIsInitialized = 1;
			lastWidth = viewPort[2];
			lastHeight = viewPort[3];
			transparencyPrepareFBO();
	}

	if(lastWidth != viewPort[2] || lastHeight != viewPort[3])
	{
		lastWidth = viewPort[2];
		lastHeight = viewPort[3];

		transparencyDeleteFBO();
		transparencyPrepareFBO();
	}

	//draw geometry
	glDisable(GL_DEPTH_TEST);
	bool showFacesCulled;
	getParamFlagMeshGL(SHOW_FACES_CULLED,&showFacesCulled);
	if(showFacesCulled) {

		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
	}

	PglBindFramebuffer bindFramebufferFunc = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer"));
	PglDrawBuffers drawBuffersFunc = reinterpret_cast<PglDrawBuffers>(mOpenGLContext->getProcAddress("glDrawBuffers"));
	PglClearBufferfv clearBufferfvFunc = reinterpret_cast<PglClearBufferfv>(mOpenGLContext->getProcAddress("glClearBufferfv"));
	PglBlendFunci blendFunciFunc = reinterpret_cast<PglBlendFunci>(mOpenGLContext->getProcAddress("glBlendFunci"));

	bindFramebufferFunc(GL_FRAMEBUFFER, mTransFbo);
	const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	drawBuffersFunc(2,drawBuffers);

	float clearColorZero[4] = { 0.f, 0.f, 0.f, 0.f };
	float clearColorOne[4]  = { 1.f, 1.f, 1.f, 1.f };

	clearBufferfvFunc(GL_COLOR, 0, clearColorZero);
	clearBufferfvFunc(GL_COLOR, 1, clearColorOne);

	glEnable(GL_BLEND);
	blendFunciFunc(0, GL_ONE, GL_ONE);
	blendFunciFunc(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	PglActiveTexture activeTextureFunc = reinterpret_cast<PglActiveTexture>(mOpenGLContext->getProcAddress("glActiveTexture"));

	glBindVertexArray( mVAO);

	vboPrepareVerticesStriped();

	mVertBufObjs[VBUFF_FACES]->bind();

	if(!mShaderTransparencyGeomWOIT->bind())
	{
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	//add the default shader stuff, exluding the geometry-shader stuff -> no normals rendering, no wireframes...
	//largely copy and paste from the "vboPaintFacesIndexed()" function
	//-------------------------------------------


	// Set the basics - including strided data:
	//----------------------------------
	shaderSetMeshBasicFuncVals(mShaderTransparencyGeomWOIT);


	//-------------------------------------------

	//add transparancy and ssbo stuff
	//------------------------------------

	double floatParam;
	getParamFloatMeshGL(TRANSPARENCY_UNIFORM_ALPHA, &floatParam);
	mShaderTransparencyGeomWOIT->setUniformValue( "uUniformAlpha", static_cast<float>(floatParam));

	getParamFloatMeshGL(TRANSPARENCY_ALPHA2, &floatParam);
	mShaderTransparencyGeomWOIT->setUniformValue( "uAlpha2", static_cast<float>(floatParam));

	getParamFloatMeshGL(TRANSPARENCY_GAMMA, &floatParam);
	mShaderTransparencyGeomWOIT->setUniformValue("uGamma", static_cast<float>(floatParam));

	int intParam;
	getParamIntMeshGL(TRANSPARENCY_TRANS_FUNCTION, &intParam);
	mShaderTransparencyGeomWOIT->setUniformValue("uTransparencyType", intParam);

	getParamIntMeshGL(TRANSPARENCY_SEL_LABEL, &intParam);
	mShaderTransparencyGeomWOIT->setUniformValue("uTransLabelNr", static_cast<float>(intParam));

	//------------------------------------
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//mVertBufObjs[VBUFF_FACES]->bind();
	glDrawElements( GL_TRIANGLES, mVertBufObjs[VBUFF_FACES]->size()/sizeof(GLuint), GL_UNSIGNED_INT, nullptr );
	PRINT_OPENGL_ERROR( "glDrawElements( GL_TRIANGLES, mVBOSizes[VBUFF_FACES], GL_UNSIGNED_INT, NULL );" );
	mShaderTransparencyGeomWOIT->release();


	//blend result
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_ALWAYS);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );


    //blend to background

	int defaultFramebuffer;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);
	bindFramebufferFunc(GL_FRAMEBUFFER,defaultFramebuffer);

	glEnable(GL_DEPTH_TEST);
	//Render Transparency Buffers
	//glEnable(GL_CULL_FACE); // we need only the front faces of the fullscreen-quad
	//glDisable(GL_DEPTH_TEST); //need to disable it to draw the screenquad
	glDepthFunc(GL_ALWAYS); //see vboPaintTransparencyKPBuffer()

	if(!mVertBufObjs[VBUFF_SCREEN_QUAD]->bind())
	{
		PRINT_OPENGL_ERROR( "glBindVertexArray( VBUFF_SCREEN_QUAD )" );
	}

	if(!mShaderTransparencyBlendWOIT->bind())
	{
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShaderTransparencyBlendWOIT->setAttributeBuffer("vertPosition", GL_FLOAT, 0, 2, 0);
	PRINT_OPENGL_ERROR( "setAttributeBuffer" );
	mShaderTransparencyBlendWOIT->enableAttributeArray( "vertPosition");
	PRINT_OPENGL_ERROR( "enableAttributeArray" );

	activeTextureFunc(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE,mTransFboTextures[0]);

	activeTextureFunc(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE,mTransFboTextures[1]);

	mShaderTransparencyBlendWOIT->setUniformValue( "ColorTex0", 0 );
	mShaderTransparencyBlendWOIT->setUniformValue("ColorTex1",1);

	glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/static_cast<int>(sizeof(GLfloat)));
	PRINT_OPENGL_ERROR( "glDrawArrays(GL_TRIANGLE_FAN, 0, mVertBufObjs[VBUFF_SCREEN_QUAD]->size()/(int)sizeof(GLfloat));" );
	mShaderTransparencyBlendWOIT->release();

	mVertBufObjs[VBUFF_SCREEN_QUAD]->release();
	glBindVertexArray(0);
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	glBindTexture(GL_TEXTURE_2D,0);
	activeTextureFunc(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,0);

	glEnable(GL_DEPTH_TEST);

	if(showFacesCulled) {

		glDisable(GL_CULL_FACE);
	}
	glEnable(GL_BLEND);
	glDepthFunc(GL_LESS);
}

void MeshGLShader::transparencyPrepareFBO()
{
	GLint viewPort[4];
	glGetIntegerv(GL_VIEWPORT, viewPort);


	glGenTextures(2, mTransFboTextures);
	glBindTexture(GL_TEXTURE_RECTANGLE, mTransFboTextures[0]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F,
				 viewPort[2], viewPort[3], 0, GL_RGBA, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_RECTANGLE, mTransFboTextures[1]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8,
				 viewPort[2], viewPort[3], 0, GL_RED, GL_FLOAT, nullptr);

	PglGenFramebuffers genFramebuffersFunc = reinterpret_cast<PglGenFramebuffers>(mOpenGLContext->getProcAddress("glGenFramebuffers"));
	PglBindFramebuffer bindFramebufferFunc = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer"));
	PglFramebufferTexture2D framebufferTexture2DFunc = reinterpret_cast<PglFramebufferTexture2D>(mOpenGLContext->getProcAddress("glFramebufferTexture2D"));

	genFramebuffersFunc(1, &mTransFbo);

	bindFramebufferFunc(GL_FRAMEBUFFER, mTransFbo);

	framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							 GL_TEXTURE_RECTANGLE, mTransFboTextures[0], 0);
	framebufferTexture2DFunc(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
							   GL_TEXTURE_RECTANGLE, mTransFboTextures[1], 0);

	PRINT_OPENGL_ERROR( "Error initializing WeightedBlended OIT FBO" );

	int defaultFramebuffer;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

	bindFramebufferFunc(GL_FRAMEBUFFER, defaultFramebuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void MeshGLShader::transparencyDeleteFBO()
{
	PglBindFramebuffer bindFramebufferFunc = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer"));
	PglDeleteFramebuffers deleteFramebuffersFunc = reinterpret_cast<PglDeleteFramebuffers>(mOpenGLContext->getProcAddress("glDeleteFramebuffers"));

	int defaultFramebuffer;

	getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

	bindFramebufferFunc(GL_FRAMEBUFFER,defaultFramebuffer);
    deleteFramebuffersFunc(1, &mTransFbo);
    glDeleteTextures(2, mTransFboTextures);

	PRINT_OPENGL_ERROR( "Error deleting WeightedBlended OIT FBO" );
}

void MeshGLShader::vboPaintPointcloud(eTexMapType rRenderColor, bool limitPoints)
{
	const auto mShaderPointcloud  = mShaderManager->getShader(ShaderManager::ShaderName::POINT_CLOUD);

	if( mShaderPointcloud  == nullptr )
	   return;

	mShaderPointcloud->bind();

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray(mVAO);

	vboPrepareVerticesStriped();

	GLboolean cullFace;
	glGetBooleanv( GL_CULL_FACE, &cullFace );
	glDisable( GL_CULL_FACE );

	glDisable(GL_PROGRAM_POINT_SIZE);

	double pointSize = 1.0;
	getParamFloatMeshGL( MeshGLParams::POINTCLOUD_POINTSIZE, &pointSize);
	glPointSize(pointSize);

	//---- set uniforms ----
	shaderSetLocationBasicMatrices( mShaderPointcloud);
	// Set the basics for faces (culling/smooth)
	shaderSetLocationBasicFaces( mShaderPointcloud );

	// Set the basic lighting:
	shaderSetLocationBasicLight( mShaderPointcloud );

	// Set the basic fog:
	shaderSetLocationBasicFog( mShaderPointcloud );

	// Set the mesh plane as clipping plane:
	double clipPlane[4];
	getPlaneHNF( clipPlane );
	mShaderPointcloud->setUniformValue( "uClipPlane0", clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3] );
	// Set the COG of the selected primitve as clip plane (implictly using the camera's view direction):
	if( mPrimSelected != nullptr ) {
		Vector3D selPrimCog = mPrimSelected->getCenterOfGravity();
		mShaderPointcloud->setUniformValue( "uClipBefore", selPrimCog.getX(), selPrimCog.getY(), selPrimCog.getZ() );
	}

	// Bind the 2D texture used for colorramps and labels!
	//glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] );
	//PRINT_OPENGL_ERROR( "glBindTexture( GL_TEXTURE_2D, mTextureMaps[0] )" );
	mTextureMaps[0]->bind();
	PRINT_OPENGL_ERROR( "binding mTextureMaps[0]" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] texId: " << texId << endl;
	// Set the ID of the texture map:
	GLint currTextureSlot;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &currTextureSlot);
	currTextureSlot -= GL_TEXTURE0;
	mShaderPointcloud->setUniformValue( "uLabelTexMap", currTextureSlot );
	PRINT_OPENGL_ERROR( "Shader setAttributeValue" );
	mShaderPointcloud->setUniformValue( "uFuncValTexMap", currTextureSlot );
	PRINT_OPENGL_ERROR( "Shader setAttributeValue" );


	// Set values for rendering of the labels:
	int labelShift;
	getParamIntMeshGL( MeshGLParams::COLMAP_LABEL_OFFSET, &labelShift );
	mShaderPointcloud->setUniformValue( "uLabelCountOffset", static_cast<GLfloat>(labelShift) );
	bool showLabelsMonoColor;
	getParamFlagMeshGL( MeshGLParams::SHOW_LABELS_MONO_COLOR, &showLabelsMonoColor );
	mShaderPointcloud->setUniformValue( "uLabelSameColor", showLabelsMonoColor );
	GLfloat somemColorSetting[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_SOLID, somemColorSetting );
	mShaderPointcloud->setUniformValue( "uLabelSingleColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BORDER, somemColorSetting );
	mShaderPointcloud->setUniformValue( "uLabelBorderColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_NOT_ASSIGNED, somemColorSetting );
	mShaderPointcloud->setUniformValue( "uLabelNoColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	mRenderColors->getColorSettings( MeshGLColors::COLOR_LABEL_BACKGROUND, somemColorSetting );
	mShaderPointcloud->setUniformValue( "uLabelBackgroundColor", somemColorSetting[0], somemColorSetting[1], somemColorSetting[2], somemColorSetting[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Set parameters of the color mapping.
	int colMapID = GLSL_COLMAP_GRAYSCALE;
	getParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, &colMapID );
	mShaderPointcloud->setUniformValue( "uFuncValColorMap", static_cast<GLfloat>(colMapID) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Set the colored range.
	double funcValMin;
	double funcValMax;
	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	mShaderPointcloud->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(funcValMin) );
	mShaderPointcloud->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(funcValMax) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	//cout << "[MeshGLShader::" << __FUNCTION__ << "] funcValMin: " << funcValMin << "  funcValMax: " << funcValMax << endl;
	// Inversion of the colormap
	bool funcValInvertMap = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_COLMAP_INVERT, &funcValInvertMap );
	mShaderPointcloud->setUniformValue( "uFuncValInvert", static_cast<GLboolean>(funcValInvertMap) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Repeat colormap
	bool funcValRepeat = false;
	getParamFlagMeshGL( MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &funcValRepeat );
	mShaderPointcloud->setUniformValue( "uFuncValRepeat", static_cast<GLboolean>(funcValRepeat) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	double funcValRepeatIntervall;
	getParamFloatMeshGL( MeshGLParams::WAVES_COLMAP_LEN, &funcValRepeatIntervall );
	mShaderPointcloud->setUniformValue( "uFuncValIntervall", static_cast<GLfloat>(funcValRepeatIntervall) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	// Logarithmic color ramp
	double funcValLogGamma;
	getParamFloatMeshGL( MeshGLParams::FUNC_VALUE_LOG_GAMMA, &funcValLogGamma );
	mShaderPointcloud->setUniformValue( "uFuncValLogGamma", static_cast<GLfloat>(funcValLogGamma) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Switch rendering mode solid/color_per_vertex/function_value/labels:
	mShaderPointcloud->setUniformValue( "uRenderColor", rRenderColor );
	GLfloat colorTmpStored[4];
	mRenderColors->getColorSettings( MeshGLColors::COLOR_MESH_SOLID, colorTmpStored );
	mShaderPointcloud->setUniformValue( "colorSolid", colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );

	// Z-shift
	mShaderPointcloud->setUniformValue( "uFaceShiftViewZ", 0.0f );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );


	double reductionFactor = 1.0;

	if(limitPoints)
	{
		double totalSize = 0.0;
		double targetSize = 200000;
		mVertBufObjs[VBUFF_VERTICES_SOLO]->bind();
		totalSize +=  mVertBufObjs[VBUFF_VERTICES_SOLO]->size() / static_cast<int>(sizeof(grVertexStripeElment));

		mVertBufObjs[VBUFF_VERTICES_STRIPED]->bind();
		totalSize += mVertBufObjs[VBUFF_VERTICES_STRIPED]->size() / static_cast<int>(sizeof(grVertexStripeElment));
		reductionFactor = std::max(totalSize / targetSize, 1.0);
	}

	mShaderPointcloud->setUniformValue("reductionFactor", static_cast<int>(reductionFactor));
	// --- set attributes ---


	shaderSetLocationBasicAttribs( mShaderPointcloud, VBUFF_VERTICES_STRIPED, static_cast<int>(sizeof(grVertexStripeElment)));
	// More Strided Data -- map buffer
	//----------------------------------
	size_t offSetLabelID = sizeof(GLfloat)*6 + sizeof(GLubyte)*4 + sizeof(GLfloat);
	mShaderPointcloud->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderPointcloud->enableAttributeArray( "vLabelID" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
	mShaderPointcloud->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderPointcloud->enableAttributeArray( "vFlags" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

	glDrawArrays(GL_POINTS, 0, mVertBufObjs[VBUFF_VERTICES_STRIPED]->size() / static_cast<int>(sizeof(grVertexStripeElment)));

	mVertBufObjs[VBUFF_VERTICES_SOLO]->bind();
	shaderSetLocationBasicAttribs( mShaderPointcloud, VBUFF_VERTICES_SOLO, static_cast<int>(sizeof(grVertexStripeElment)));
	glDrawArrays(GL_POINTS, 0, mVertBufObjs[VBUFF_VERTICES_SOLO]->size() / static_cast<int>(sizeof(grVertexStripeElment)));

	mShaderPointcloud->release();

	if(cullFace)
	{
		glEnable(GL_CULL_FACE);
	}

	glEnable(GL_PROGRAM_POINT_SIZE);
}

//! Draws a truncated cone using an axis and two radii.
void MeshGLShader::drawTruncatedCone( Vector3D rAxisTop, Vector3D rAxisBottom, double rUpperRadius, double rLowerRadius ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGLShader::" << __FUNCTION__ << "]" << endl;
#endif
	//MeshGL::drawTruncatedCone( axisTop, axisBottom, upperRadius, lowerRadius, slices, stacks );

	// No clipping of a cone
	glDisable( GL_CLIP_PLANE0 );
	glDisable( GL_CLIP_PLANE1 );  // DongArch clip preview
	glDisable( GL_CLIP_PLANE2 );
	PRINT_OPENGL_ERROR( "glDisable( GL_CLIP_PLANE... )" );

	const auto mShaderDatumObjects  = mShaderManager->getShader(ShaderManager::ShaderName::FUNC_VAL_COLOR);

	if( mShaderDatumObjects  == nullptr )
	   return;

	double coneHeight = ( rAxisTop - rAxisBottom ).getLength3();
	Vector3D coneAxis( 0.0, 0.0, coneHeight );                      // positive z-axis by default
	Vector3D userAxis       = rAxisTop - rAxisBottom;                 // desired rotaion axis
	Vector3D axisOfRotation = coneAxis % userAxis;                  // we will rotate around this axis...
	double   rotationAngle  = angle(userAxis, coneAxis)*180.0/M_PI; // ...by this angle

	QMatrix4x4 transMat;
	transMat.translate( (rAxisBottom.getX()+rAxisTop.getX())/2.0, (rAxisBottom.getY()+rAxisTop.getY())/2.0, (rAxisBottom.getZ()+rAxisTop.getZ())/2.0 );
	QMatrix4x4 rotMat;
	rotMat.rotate( rotationAngle, axisOfRotation.getX(), axisOfRotation.getY(), axisOfRotation.getZ() );
	QMatrix4x4 transRotMat = transMat * rotMat;

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(mOpenGLContext->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	// Lets be shady :)
	if( !mShaderDatumObjects->bind() ) {
		cerr << "[MeshGLShader::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
	}

	// Would be nice to have: http://en.wikipedia.org/wiki/Order-independent_transparency#cite_note-lfb-6
	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	// CONE/CYLINDER Set the basics:
	shaderSetLocationBasicMatrices( mShaderDatumObjects );
	shaderSetLocationBasicAttribs( mShaderDatumObjects, VBUFF_CYLINDER, static_cast<int>(sizeof(grVertexElmentBasic)) );

	mShaderDatumObjects->setUniformValue( "backCulling", false );
	//mShaderDatumObjects->setUniformValue( "colorSolidBack", QColor( 255, 0, 0, 64 ) );
	// Switch rendering mode solid/color_per_vertex/function_value/labels:
	mShaderDatumObjects->setUniformValue( "uRenderColor", TEXMAP_VERT_RGB );

	mShaderDatumObjects->setUniformValue( "uFuncValMin",  static_cast<GLfloat>(-0.5) );
	mShaderDatumObjects->setUniformValue( "uFuncValMax",  static_cast<GLfloat>(+0.5) );
	PRINT_OPENGL_ERROR( "Shader setUniformValue" );
	mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoLinesShow",  static_cast<GLboolean>(true)           );
	mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoDist",       static_cast<GLfloat>(2.0/coneHeight) );
	mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoOffset",     static_cast<GLfloat>(0.0f)             );
	mShaderDatumObjects->setUniformValue( "funcValIsoLineParams.mIsoPixelWidth", static_cast<GLfloat>(1.0f)             );
	mShaderDatumObjects->setUniformValue( "uIsoSolidFlag",                       static_cast<GLboolean>(true)           );
	PRINT_OPENGL_ERROR( "SHADER operations!" );

	// Scale - SET
	mShaderDatumObjects->setUniformValue( "uScaleHeight",       static_cast<GLfloat>(coneHeight)        );
	mShaderDatumObjects->setUniformValue( "uScaleRadialBottom", static_cast<GLfloat>(rLowerRadius*2.0) );
	mShaderDatumObjects->setUniformValue( "uScaleRadialTop",    static_cast<GLfloat>(rUpperRadius*2.0) );
	PRINT_OPENGL_ERROR( "SHADER operations!" );
	// Translation and Rotation - SET
	mShaderDatumObjects->setUniformValue( "uModelViewExtra", transRotMat );
	PRINT_OPENGL_ERROR( "SHADER operations!" );

	GLsizei triangleCount = mVertBufObjs[VBUFF_CYLINDER]->size()/sizeof(grVertexElmentBasic);
	shaderSetLocationBasicMatrices( mShaderDatumObjects );
	shaderSetLocationBasicAttribs( mShaderDatumObjects, VBUFF_CYLINDER, static_cast<int>(sizeof(grVertexElmentBasic)) );

	//set Label and Flag attributes
	size_t offSetLabelID = sizeof(GLfloat)*6 + sizeof(GLubyte)*4 + sizeof(GLfloat);
	mShaderDatumObjects->setAttributeBuffer( "vLabelID", GL_FLOAT, static_cast<int>(offSetLabelID), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderDatumObjects->enableAttributeArray( "vLabelID" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );
	size_t offSetFlags = offSetLabelID + sizeof(GLfloat);
	mShaderDatumObjects->setAttributeBuffer( "vFlags", GL_FLOAT, static_cast<int>(offSetFlags), 1, static_cast<int>(sizeof(grVertexStripeElment)) );
	mShaderDatumObjects->enableAttributeArray( "vFlags" );
	PRINT_OPENGL_ERROR( "Shader enableAttributeArray" );

	glDrawArrays( GL_TRIANGLES, 0, triangleCount );
	PRINT_OPENGL_ERROR( "glDrawArrays( ... );" );


	// Scale - set BACK TO DEFAULTS
	mShaderDatumObjects->setUniformValue( "uScaleHeight",       static_cast<GLfloat>(1.0f) );
	mShaderDatumObjects->setUniformValue( "uScaleRadialBottom", static_cast<GLfloat>(1.0f) );
	mShaderDatumObjects->setUniformValue( "uScaleRadialTop",    static_cast<GLfloat>(1.0f) );
	PRINT_OPENGL_ERROR( "SHADER operations!" );
	transRotMat.setToIdentity();
	// Translation and Rotation - set to DEFAULTS
	mShaderDatumObjects->setUniformValue( "uModelViewExtra", transRotMat );
	PRINT_OPENGL_ERROR( "SHADER operations!" );


	// End of being shady
	mShaderDatumObjects->release();
	PRINT_OPENGL_ERROR( "mShaderPolyLines->release()" );

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );
}
