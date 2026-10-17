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

#include "meshGL.h"
#include "../meshwidget_params.h"

#include <thread>
#include <future>
#include <QTime>
#include <QPixmap>

#include "glmacros.h"
#include <GigaMesh/mesh/octree.h>
#include <GigaMesh/mesh/vertexofface.h>

#include <GigaMesh/mesh/ellipsedisc.h>

extern "C"
{
    #include <triangle/triangle.h>
}
#include <triangle/triangleTriangleIntersection.h>

//#include "../MeshQtCSVImportExport.h"

// #define DEBUG_SHOW_ALL_METHOD_CALLS

using namespace std;

// Vertex Array Object related -- see initializeGL()
// ----------------------------------------------------
using PglGenVertexArrays = void (*)(GLsizei, GLuint *);
using PglBindVertexArray = void (*)(GLuint);


//! Constructor setting some defaults (e.g. colors).
MeshGL::MeshGL(QGLContext* rGLContext,
                const std::filesystem::path& rFileName,
                bool& rReadSuccess
) : Mesh( rFileName, rReadSuccess ), mVBOPrepared( false ), mOpenGLContext( rGLContext ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
		mVboMemoryUsage = 0;
#endif
		// Initalize pointers to buffer objects:
		for(QOpenGLBuffer*& mVertBufObj : mVertBufObjs) {
                mVertBufObj = nullptr;
		}

        cout << "[MeshGL::" << __FUNCTION__ << "] ------------------------------------------------------------------------" << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] " << glGetString( GL_VERSION ) << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] " << glGetString( GL_RENDERER ) << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] " << glGetString( GL_VENDOR ) << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] ------------------------------------------------------------------------" << endl;
}

//! Destructor.
MeshGL::~MeshGL() {
		// nothing to do :)
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		glRemove();
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << __PRETTY_FUNCTION__ << " Done. " << endl;
#endif
}

//! Passing thru function calls from/to Mesh.
//! Typically related GUI interaction.
bool MeshGL::callFunction( MeshParams::eFunctionCall rFunctionID, bool rFlagOptional ) {
	// Ellipsenfit
    /**
    if( rFunctionID == MeshParams::ELLIPSENFIT_EXPERIMENTAL ) {
        vector<Vertex*> testPlanePoints; //Test
        vector<pair<double,double> > ellipseCandidatePoints;
        for( auto const& selVertex : mSelectedMVerts ) {
            testPlanePoints.push_back(selVertex);
            double xPos = selVertex->getPositionVector().getX();
            double yPos = selVertex->getPositionVector().getY();
            ellipseCandidatePoints.emplace_back( std::make_pair(xPos,yPos) );
            std::cout << "[MeshGL::" << __FUNCTION__ << "] EPC: " << xPos << " " << yPos << std::endl;
        }

        EllipseDisc bar;
        bar.findEllipseParams( EllipseDisc::CONIC, ellipseCandidatePoints );
        bar.dumpInfo();

        //---------------------------------------------------------
        //test

        //create TestPlane
        Plane plane = Plane(testPlanePoints[0]->getPositionVector(),testPlanePoints[1]->getPositionVector(),testPlanePoints[2]->getPositionVector());

        //projection onto plane
        vector<pair<double,double> > ellipseCandidatePoints2;
        for( auto const& selVertex : mSelectedMVerts ) {
            Vector3D selPoint = selVertex->getPositionVector();
            selPoint.projectOntoPlane(plane.getHNF());
            double xPos2 = selPoint.getX();
            double yPos2 = selPoint.getZ(); //changed to Z because the Y axis points upwards
            ellipseCandidatePoints2.emplace_back( std::make_pair(xPos2,yPos2) );
        }
        EllipseDisc bar2;
        bar2.findEllipseParams( EllipseDisc::CONIC, ellipseCandidatePoints2 );
        bar2.dumpInfo();

        double planeY = plane.getY();
        double r1Quad = pow(bar2.mRadius1,2.0);
        double r2Quad = pow(bar2.mRadius2,2.0);
        for(Vertex* vert : mVertices){
            if(abs((vert->getY()) - planeY) < 0.5){
                vert->setLabel(1);
            }
            else{
                vert->setLabel(2);
            }
            Vector3D projVert = vert->getPositionVector();
            projVert.projectOntoPlane(plane.getHNF());
            double xTerm = pow((vert->getX()-bar2.mCenterX),2.0)/r1Quad;
            double yTerm = pow((vert->getZ()-bar2.mCenterY),2.0)/r2Quad; //useZ because for the ellipse constructions is used too
            //double xTerm = pow((projVert.getX()-bar2.mCenterX),2.0)/r1Quad;
            //double yTerm = pow((projVert.getY()-bar2.mCenterY),2.0)/r2Quad;
            double mainForm = xTerm+yTerm;

            if(abs(1-(mainForm)) < 0.01 ){
                vert->setLabel(3);
            }
            if(mainForm < 0.99 ){
                vert->setLabel(4);
            }

        }
        labelsChanged();

        //calculate center vertex
        bool error;
        //Vector3D normalPlane = plane.getNormal( true );
        Vector3D normalPlane = plane.getHNF();
        Vector3D ellipseCenter2(bar2.mCenterX,planeY,bar2.mCenterY); //Y and Z changed before
        vector<Vertex*> circleCenterVertices;
        Vertex* centerVertex = new Vertex( ellipseCenter2 );
        centerVertex->setFlag( FLAG_SYNTHETIC | FLAG_CIRCLE_CENTER);
        centerVertex->setNormal( &normalPlane );
        circleCenterVertices.push_back(centerVertex);
        error = insertVertices( &circleCenterVertices );

        Vector3D topPoint;
        Vector3D bottomPoint;
        error  = getAxisFromCircleCenters( topPoint, bottomPoint );
        error &= setConeAxis( &topPoint, &bottomPoint );

        //------------------------------------------------------------



        bar.findEllipseParams( EllipseDisc::FPF, ellipseCandidatePoints );
        bar.dumpInfo();
        bar.findEllipseParams( EllipseDisc::BOOKSTEIN, ellipseCandidatePoints );
        bar.dumpInfo();
    }
    **/
	// Show labels after they have been determined.
	if( rFunctionID == LABELING_LABEL_ALL ) {
		setParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_MONOLITHIC );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_LABELS );
	}

	return Mesh::callFunction( rFunctionID, rFlagOptional );
}

//! Processing function calls typically from a GUI interaction.
//!
//! See MeshGLParams::eFunctionCall
//!
//! @returns false in case of an error. True otherwise.
bool MeshGL::callFunction( MeshGLParams::eFunctionCall rFunctionID, bool rFlagOptional ) {
    std::cout << "[MeshGL::" << __FUNCTION__ << "] with MeshGLParam: "   << rFunctionID   << std::endl;
    std::cout << "[MeshGL::" << __FUNCTION__ << "] with optional flag: " << rFlagOptional << std::endl;
	bool retVal = false;
	switch( rFunctionID ) {
        case TEXMAP_FIXED_SET_NORMALIZED:
			setParamIntMeshGL( MeshGLParams::FUNCVAL_CUTOFF_CHOICE, MeshGLParams::FUNCVAL_CUTOFF_MINMAX_USER );
			setParamFloatMeshGL( MeshGLParams::TEXMAP_FIXED_MIN, 0.0 );
			setParamFloatMeshGL( MeshGLParams::TEXMAP_FIXED_MAX, 1.0 );
			break;
        case SET_SHOW_VERTICES_NONE:
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_ALL,              false );
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SOLO,             false );
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_BORDER,           false );
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SINGULAR,         false );
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_NON_MANIFOLD,     false );
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_LOCAL_MIN,        false );
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_LOCAL_MAX,        false );
			setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SYNTHETIC,        false );
			break;
        case ISOLINES_SET_BY_NUMBER:
			retVal = setFuncValIsoSteppingByNumberUI();
			break;
        case ISOLINES_SET_TEN_ZEROED:
			retVal = setFuncValIsoSteppingByNumber( 10, true );
			break;
		case FUNCVAL_AMBIENT_OCCLUSION:
			retVal = funcVertAmbientOcclusion();
			break;
		case TRANSFORM_FUNCTION_VALUES_TO_RGB: {
				QMessageBox colorOverwriteWarningMessageBox;
				colorOverwriteWarningMessageBox.setIcon( QMessageBox::Warning );
				colorOverwriteWarningMessageBox.setText( "Vertex color overwrite warning" );
				colorOverwriteWarningMessageBox.setInformativeText(
				                        "This will irrevertably overwrite "
				                        "the color per vertex.<br /><br />"
				                        "Do you want to continue?" );
				colorOverwriteWarningMessageBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
				colorOverwriteWarningMessageBox.setDefaultButton( QMessageBox::No );
				if( colorOverwriteWarningMessageBox.exec() == QMessageBox::Yes ) {
					retVal = runFunctionValueToRGBTransformation();
				} else {
					retVal = false;
				}
			}
			break;
		case MULTIPLY_COLORVALS_WITH_FUNCVALS: {
				QMessageBox colorOverwriteWarningMessageBox;
				colorOverwriteWarningMessageBox.setIcon( QMessageBox::Warning );
				colorOverwriteWarningMessageBox.setText( "Vertex color overwrite warning" );
				colorOverwriteWarningMessageBox.setInformativeText(
				                        "This will irrevertably overwrite "
				                        "the color per vertex.<br /><br />"
				                        "Note: Function values should be normalized "
				                        "before performing this operation.<br /><br />"
				                        "Do you want to continue?" );
				colorOverwriteWarningMessageBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
				colorOverwriteWarningMessageBox.setDefaultButton( QMessageBox::No );
				if( colorOverwriteWarningMessageBox.exec() == QMessageBox::Yes ) {
					retVal = multiplyColorWithFuncVal();
				} else {
					retVal = false;
				}
			}
			break;
		case NORMALIZE_FUNCTION_VALUES: {
				QMessageBox colorOverwriteWarningMessageBox;
				colorOverwriteWarningMessageBox.setIcon( QMessageBox::Warning );
				colorOverwriteWarningMessageBox.setText( "Function value overwrite warning" );
				colorOverwriteWarningMessageBox.setInformativeText(
				                        "This will irrevertably overwrite "
				                        "the vertex function values.<br /><br />"
				                        "Do you want to continue?" );
				colorOverwriteWarningMessageBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
				colorOverwriteWarningMessageBox.setDefaultButton( QMessageBox::No );
				if( colorOverwriteWarningMessageBox.exec() == QMessageBox::Yes ) {
					retVal = normalizeFunctionValues();
				} else {
					retVal = false;
				}
			}
			break;
		default:
            std::cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Unknown rFunctionID "<< rFunctionID << " !" << std::endl;
			return( false );
	}
	// Done:
	return( retVal );
}

//! Remove OpenGL lists and VBOs related to labels.
//! Typically called, whem labels and/or their colormap were changed.
bool MeshGL::labelsChanged() {
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
		// Polylines, which are often colored like the labels have to be reset too:
		polyLinesChanged();
		vboRemoveBuffer( VBUFF_VERTICES_STRIPED,  __FUNCTION__ );
		// Pass method call to father class:
		return Mesh::labelsChanged();
}

//! Removes OpenGL lists, so that they are regenerated.
void MeshGL::polyLinesChanged() {
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
		Mesh::polyLinesChanged();
		// In case there are (new) polylines - show them:
		bool showPolylines = ( getPolyLineNr() > 0 );
		setParamFlagMeshGL( MeshGLParams::SHOW_POLYLINES, showPolylines );
		// Polylines - remove buffers, which will re-create those, when needed:
		vboRemoveBuffer( VBUFF_VERTICES_POLYLINES, __FUNCTION__ );
		vboRemoveBuffer( VBUFF_POLYLINES, __FUNCTION__ );
}

//! Remove VBO related to selection and therefore trigger the generation of a new one.
//! @returns the number of selected vertices.
unsigned int MeshGL::selectedMVertsChanged() {
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
		vboRemoveBuffer( VBUFF_VERTICES_FLAG_SELECTED, __FUNCTION__ );
		// Function value range will change too, when vertices go missing:
        setParamFloatMeshGL( TEXMAP_AUTO_MIN, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
        setParamFloatMeshGL( TEXMAP_AUTO_MAX, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
        setParamFloatMeshGL( TEXMAP_QUANTIL_MIN_ABSOLUT, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
        setParamFloatMeshGL( TEXMAP_QUANTIL_MAX_ABSOLUT, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
		return Mesh::selectedMVertsChanged();
}

//! Remove VBO related to selection and therefore trigger the generation of a new one.
//! @returns the number of selected vertices.
unsigned int MeshGL::selectedMFacesChanged() {
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
		vboRemoveBuffer( VBUFF_FACES_FLAG_SELECTED, __FUNCTION__ );
		return Mesh::selectedMFacesChanged();
}

//! Clears the polyline curvature/integral invariant visualization before its estimated.
bool MeshGL::compPolylinesIntInvRunLen( double rIIRadius, PolyLine::ePolyIntInvDirection rDirection ) {
		bool retVal = Mesh::compPolylinesIntInvRunLen( rIIRadius, rDirection );
		vboRemoveBuffer( VBUFF_VERTICES_POLYLINES, __FUNCTION__ );
		return retVal;
}

//! Clears the polyline curvature/integral invariant visualization before its estimated.
bool MeshGL::compPolylinesIntInvAngle(  double rIIRadius ) {
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
		bool retVal = Mesh::compPolylinesIntInvAngle( rIIRadius );
		vboRemoveBuffer( VBUFF_VERTICES_POLYLINES, __FUNCTION__ );
		return retVal;
}

//! Clears the polyline curvature/integral invariant visualization before its estimated.
void MeshGL::getPolylineExtrema() {
		bool absoluteCurvature;
		getParamFlagMeshGL( SHOW_POLYLINES_CURVATURE_ABS, &absoluteCurvature );
		Mesh::getPolylineExtrema( absoluteCurvature );
		vboRemoveBuffer( VBUFF_VERTICES_POLYLINES, __FUNCTION__ );
}

//! Takes care about the mesh, when new faces and vertices were add by filling holes.
bool MeshGL::fillPolyLines(
                uint64_t& rFilled,
                uint64_t& rFail,
                uint64_t& rSkipped
) {
		bool retVal = Mesh::fillPolyLines( 0, rFilled, rFail, rSkipped );
		if( !retVal ) {
				return false;
		}
		glRemove();
		glPrepare();
		return true;
}

bool MeshGL::applyTransformationToWholeMesh(Matrix4D rTrans, bool rResetNormals, bool rSaveTransMat)
{
    bool retVal = Mesh::applyTransformationToWholeMesh(rTrans, rResetNormals, rSaveTransMat);

	if(retVal)
	{
		double minScalePins = 1.0; //smallest downscale value
		double scalePins = 1.0; //highest upscale value
		for(uint8_t i = 0; i<3; ++i)
		{
			const Vector3D columnVec(rTrans.get(0,i), rTrans.get(1,i), rTrans.get(2,i));
			const auto length = columnVec.getLength3();

			minScalePins = std::min(length, minScalePins);
			scalePins    = std::max(length, scalePins);
		}

		if(1.0 / minScalePins > scalePins)
		{
			scalePins = minScalePins;
		}

		if(scalePins != 1.0 && scalePins > 0.0)
		{
			double pinSize = 0.0;
			getParamFloatMeshGL(MeshGLParams::PIN_SIZE, &pinSize);
			pinSize *= scalePins;
			setParamFloatMeshGL(MeshGLParams::PIN_SIZE, pinSize);
		}
	}

	return retVal;
}

//! Takes care about OpenGL stuff, when the Mesh is transformed.
bool MeshGL::applyTransformation( Matrix4D rTrans, set<Vertex*>* rSomeVerts, bool rResetNormals, bool rSaveTransMat ) {
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
        bool retVal = Mesh::applyTransformation( rTrans, rSomeVerts, rResetNormals, rSaveTransMat );
		if( !retVal ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: applyTransformation failed!" << endl;
				return false;
		}
		glRemove();
		glPrepare();
		return true;
}

//! Takes care about OpenGL stuff, when the Mesh is melted.
bool MeshGL::applyMeltingSphere( double rRadius, double rRel ) {
		bool retVal = Mesh::applyMeltingSphere( rRadius, rRel );
		if( !retVal ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: applyMeltingSphere failed!" << endl;
				return false;
		}
		glRemove();
		glPrepare();
		return true;
}

//! Refresh recomputed vertex normals.
bool MeshGL::normalsVerticesChanged() {
	vboRemoveBuffer( VBUFF_VERTICES_STRIPED, __FUNCTION__ );
	if( mMeshTextured != nullptr ) {
		delete mMeshTextured;
		mMeshTextured = nullptr;
	}
	return( true );
}

//! Takes care about related VBOs, when the face's function values were changed.
void MeshGL::changedFaceFuncVal() {
		Mesh::changedFaceFuncVal();
		setParamIntMeshGL( TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
}

//! Takes care about related VBOs, when the face's function values were changed.
void MeshGL::changedVertFuncVal() {
		Mesh::changedVertFuncVal();
		vboRemoveBuffer( VBUFF_VERTICES_STRIPED,         __FUNCTION__ );
		vboRemoveBuffer( VBUFF_VERTICES_FLAG_LOCAL_MIN,  __FUNCTION__ );
		vboRemoveBuffer( VBUFF_VERTICES_FLAG_LOCAL_MAX,  __FUNCTION__ );
        setParamFloatMeshGL( TEXMAP_AUTO_MIN, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
        setParamFloatMeshGL( TEXMAP_AUTO_MAX, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
        setParamFloatMeshGL( TEXMAP_QUANTIL_MIN_ABSOLUT, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
        setParamFloatMeshGL( TEXMAP_QUANTIL_MAX_ABSOLUT, _NOT_A_NUMBER_DBL_ ); // Nan means not set.
}

//! Takes care about related VBOs, when cortex vertex flags change (archaeological mode).
void MeshGL::cortexVerticesChanged() {
	vboRemoveBuffer( VBUFF_VERTICES_FLAG_CORTEX, __FUNCTION__ );
}

//! Handles changes of texture map, when a new one is imported.
bool MeshGL::assignImportedTexture( int              rLineCount,
                                    uint64_t*   rRefToPrimitves,
									unsigned char*   rTexMap
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		bool retVal = Mesh::assignImportedTexture( rLineCount, rRefToPrimitves, rTexMap );
		vboRemoveBuffer( VBUFF_VERTICES_STRIPED, __FUNCTION__ );

		if(mMeshTextured != nullptr)
		{
			delete mMeshTextured;
			mMeshTextured = nullptr;
		}

		return retVal;
}

//! Handles changes of normals per vertex, when new ones were imported.
bool MeshGL::assignImportedNormalsToVertices(
                const vector<MeshIO::grVector3ID>& rNormals
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		bool retVal = Mesh::assignImportedNormalsToVertices( rNormals );
		vboRemoveBuffer( VBUFF_VERTICES_STRIPED, __FUNCTION__ );

		if(mMeshTextured != nullptr)
		{
			delete mMeshTextured;
			mMeshTextured = nullptr;
		}

		return retVal;
}


//! \brief
//! \return
bool MeshGL::multiplyColorWithFuncVal() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		bool retVal = Mesh::multiplyColorWithFuncVal();
		vboRemoveBuffer( VBUFF_VERTICES_STRIPED, __FUNCTION__ );
		setParamIntMeshGL(MeshGLParams::TEXMAP_CHOICE_FACES,
								MeshGLParams::TEXMAP_VERT_RGB);
		return retVal;
}

bool MeshGL::multiplyColorWithFuncVal( const double rMin, double rMax ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		bool retVal = Mesh::multiplyColorWithFuncVal( rMin, rMax );
		vboRemoveBuffer( VBUFF_VERTICES_STRIPED, __FUNCTION__ );
		return retVal;
}

bool MeshGL::assignAlphaToSelectedVertices(unsigned char alpha) {
	bool retVal = Mesh::assignAlphaToSelectedVertices(alpha);
	vboRemoveBuffer( VBUFF_VERTICES_STRIPED, __FUNCTION__);
	return retVal;
}

//! Select a point of the plane by pixel coordinates in screen space.
//!
//! @return false in case of an error. True otherwise.
bool MeshGL::selectPlaneThreePoints( int rXPixel, int rYPixel ) {
	Vector3D pointIntersect;
	Face* currFace;
	currFace = getFaceAt( rXPixel, rYPixel, &pointIntersect );
	if( currFace != nullptr ) {
		// Only when a face was selected there is a pointIntersect
		pointIntersect.dumpInfo();
		setPlanePos( &pointIntersect );
	}
	return( true );
}

//! Select a point of the cone by pixel coordinates in screen space.
//!
//! @return false in case of an error. True otherwise.
bool MeshGL::selectConePoints( int rXPixel, int rYPixel ) {
	// new cone (either old data is overwritten or it is
	// indeed the first cone the user selected)
	if(   this->getConeStatus() == CONE_UNDEFINED
	   || this->getConeStatus() == CONE_DEFINED_LOWER_RADIUS )
	{
		// Get _general_ cone axis first
		Vector3D upper;
		Vector3D lower;
		getRayWorld( rXPixel, rYPixel, &upper, &lower);
		setConeAxis( &upper, &lower );
	}

	// axis is defined, but no radius yet or first radius has been defined
	else if(    this->getConeStatus() == CONE_DEFINED_AXIS
	         || this->getConeStatus() == CONE_DEFINED_UPPER_RADIUS )
	{
        // Set next radius of cone. This _also_ changes the previously defined
		// axis. Namely, the upper/lower point of the axis is changed to the
		// appropriat point on the axis where the user selected the radius
		// (perpendicular foot)
		Vector3D pointIntersect;
		Face*    currFace;
		currFace = getFaceAt( rXPixel, rYPixel, &pointIntersect );
		if( currFace != nullptr ) {
			// Only when a face was selected there is a pointIntersect
			setConeRadius(pointIntersect);
		}
	}
	return( true );
}

//! Select a point of the sphere by pixel coordinates in screen space.
//!
//! @return false in case of an error. True otherwise.
bool MeshGL::selectSpherePoints( int rXPixel, int rYPixel ) {
	// Select next point in row; once four points have been determined,
	// sphere data is calculated automatically
	Vector3D pointIntersect;
	Face*    currFace;
	currFace = getFaceAt( rXPixel, rYPixel, &pointIntersect );
	if( currFace != nullptr ) {
		// Only when a face was selected there is a pointIntersect
		float faceNormal[3]{0};
		currFace->copyNormalXYZTo( faceNormal, true );
		Vector3D normalVec( faceNormal[0], faceNormal[1], faceNormal[2] );
		setSpherePoint( pointIntersect, normalVec );
	}
	return( true );
}

//! Checks the number of connected selected positions (pins)
//! This Function is used to select 3 positions as one function
//! The selections stops after the third pin
//!
//! At the future developer: Feel free to rename the method.
//!
//! @return true if more than N positions are in connection selected otherwise false
bool MeshGL::isMoreThanNconSelPosition( int rN){
     vector<std::tuple<Vector3D, Primitive*, bool>> currentSelectedPositions;
     getSelectedPosition(&currentSelectedPositions);
     //reverse loop --> check the last elements
     unsigned int count = 0;
     for (auto it = currentSelectedPositions.rbegin(); it != currentSelectedPositions.rend(); ++it)
     {
         if (std::next(it) == currentSelectedPositions.rend()){
             //count the last element too
             count++;
         }
         if(std::get<2>(*it) == true or (std::next(it) == currentSelectedPositions.rend())){
             //connection is interrupted
            if(count > rN){
                return true;
            }
            else{
                return false;
            }
         }
         count++;
     }
     return false;
}

//! Select a position by pixel coordinates in screen space.
//!
//! @return false in case of an error. True otherwise.
bool MeshGL::selectPositionAt( int rXPixel, int rYPixel, bool rLastPoint ) {
	//! \todo extend to solo vertices and polylines.
    Vector3D pointIntersect;
	Face*    currFace;
	currFace = getFaceAt( rXPixel, rYPixel, &pointIntersect );
	if( currFace != nullptr ) {
		addSelectedPosition( pointIntersect, currFace, rLastPoint );
	}
	if( rLastPoint ) {
		// Check the presence of an axis:
		if( !getConeAxisDefined() ) {
			return( true );
		}
		// If there is an axis, ask the user to compute profile lines.
		bool rUserChoice;
		string rHead = "Compute Profile lines";
		string rMsg = "Do you want to compute profile lines using the axis and the selected positions";
		if( !showQuestion( &rUserChoice, rHead, rMsg ) || !rUserChoice ) {
			// User cancel or NO.
			return( true );
		}
		// User choice YES:
		callFunction( POLYLINES_FROM_AXIS_AND_POSTIONS );
	}
	// Done
	return( true );
}


//! \brief											This function is used by the threads
//!													spawned by the function
//!													runFunctionValueToRGBTransformation to
//!													perform the actual function value to RGB
//!													transformation in parallel. The algorithm
//!													is derived from
//!													shaders/funcval.vert -> main().
//! \param colorMapImage							A QImage containing all currently available
//!													colormaps used to represent function values
//! \param meshGLColorMapChoiceValue				The currently used colormap
//! \param showRepeatColorMapValue					The currently used colormap repeat setting
//! \param invertFunctionValueColorValue			The currently used color invertion setting
//! \param functionValueMin							The minimum function value currently set by
//!													the end-user, or if none is set, the minumum
//!													function value available in the mesh
//! \param functionValueMax							The maximum function value currently set by
//!													the end-user, or if none is set, the maximum
//!													function value available in the mesh
//! \param functionValueRepetitionIntervalValue		The currently set colormap repetition
//!													interval value
//! \param functionValueLogarithmGammaValue			The currently set logarithm gamma value
//! \param offset									The offset of the dataset of the current
//!													relative to the beginning of the Vertex
//!													object vector
//! \param range									The size of the dataset to be processed by
//!													The current thread
//! \return											False of an error occured while retrieving
//!													the function value or setting the RGB value
//!													of any Vertex object in the dataset of
//!													the thread, true of all Vertex objects were
//!													processed without error
bool MeshGL::runFunctionValueToRGBTransformation_impl(
                const QImage& colorMapImage,
                const int meshGLColorMapChoiceValue,
                const bool showRepeatColorMapValue,
                const bool invertFunctionValueColorValue,
                const double functionValueMin,
                const double functionValueMax,
                const double functionValueRepetitionIntervalValue,
                const double functionValueLogarithmGammaValue,
                const uint64_t offset,
                const uint64_t range
) {
	constexpr double functionValueToTextureCoordinateXScalingFactor = 511.0/512.0;
	constexpr double functionValueToTextureCoordinateXOffset = 0.5/512.0;

	const double imageWidth = colorMapImage.width();
	const double imageHeight = colorMapImage.height();

	Vertex* currVertex = nullptr;

	QColor currentPixelColor;

	double currentFunctionValue = 0.0;

	double textureCoordinateXDouble = 0.0;
	double textureCoordinateYDouble = 0.0;

	double functionValueForTextureCoordinates = 0.0;
	double normalizedRange = 0.0;

	int64_t textureCoordinateX = 0;
	int64_t textureCoordinateY = 0;

	bool allSet = true;

	for(uint64_t vertIdx = 0; vertIdx < range; vertIdx++)
	{
		currVertex = getVertexPos(vertIdx + offset);

		if(!currVertex->getFuncValue(&currentFunctionValue))
		{
			allSet = false;
		}

		textureCoordinateYDouble = (1.0 - (20.0*static_cast<double>(
		                               meshGLColorMapChoiceValue) + 11.0)/1024.0);

		if(showRepeatColorMapValue)
		{
			functionValueForTextureCoordinates =
			        std::fabs(2.0*std::fmod(currentFunctionValue,
			                                functionValueRepetitionIntervalValue))/
			                                functionValueRepetitionIntervalValue;

			if(functionValueForTextureCoordinates > 1.0)
			{
				functionValueForTextureCoordinates = 2.0 - functionValueForTextureCoordinates;
			}

			textureCoordinateXDouble = functionValueToTextureCoordinateXScalingFactor*
			                           functionValueForTextureCoordinates +
			                           functionValueToTextureCoordinateXOffset;
		}

		else
		{
			normalizedRange = (currentFunctionValue - functionValueMin)/
			                  (functionValueMax - functionValueMin);

			if(normalizedRange < 0.0)
			{
				normalizedRange = 0.0;
			}

			else if(normalizedRange > 1.0)
			{
				normalizedRange = 1.0;
			}

			normalizedRange = std::pow(normalizedRange, functionValueLogarithmGammaValue);

			textureCoordinateXDouble = functionValueToTextureCoordinateXScalingFactor*
			                           normalizedRange +
			                           functionValueToTextureCoordinateXOffset;
		}

		if(invertFunctionValueColorValue)
		{
			textureCoordinateXDouble = 1.0 - textureCoordinateXDouble;
		}

		textureCoordinateX = std::floor(imageWidth*textureCoordinateXDouble);
		textureCoordinateY = std::floor(imageHeight*textureCoordinateYDouble);

		currentPixelColor = QColor(colorMapImage.pixel(
		                               textureCoordinateX, textureCoordinateY));

		if(!currVertex->setRGB(static_cast<unsigned char>(currentPixelColor.red()),
		                       static_cast<unsigned char>(currentPixelColor.green()),
		                       static_cast<unsigned char>(currentPixelColor.blue())))
		{
			allSet = false;
		}
	}

	return allSet;

}

//! \brief											Identical to
//!													runFunctionValueToRGBTransformation_impl,
//!													except that it updates the progress bar of
//!													the GigaMesh GUI. Used by the parent thread
//!													in runFunctionValueToRGBTransformation. The
//!													algorithm is derived from
//!													shaders/funcval.vert -> main().
//! \param colorMapImage							A QImage containing all currently available
//!													colormaps used to represent function values
//! \param meshGLColorMapChoiceValue				The currently used colormap
//! \param showRepeatColorMapValue					The currently used colormap repeat setting
//! \param invertFunctionValueColorValue			The currently used color invertion setting
//! \param functionValueMin							The minimum function value currently set by
//!													the end-user, or if none is set, the minumum
//!													function value available in the mesh
//! \param functionValueMax							The maximum function value currently set by
//!													the end-user, or if none is set, the maximum
//!													function value available in the mesh
//! \param functionValueRepetitionIntervalValue		The currently set colormap repetition
//!													interval value
//! \param functionValueLogarithmGammaValue			The currently set logarithm gamma value
//! \param offset									The offset of the dataset of the current
//!													relative to the beginning of the Vertex
//!													object vector
//! \param range									The size of the dataset to be processed by
//!													The current thread
//! \return											False of an error occured while retrieving
//!													the function value or setting the RGB value
//!													of any Vertex object in the dataset of
//!													the thread, true of all Vertex objects were
//!													processed without error
bool MeshGL::runFunctionValueToRGBTransformationUpdateProgress_impl(
                const QImage& colorMapImage,
                const int meshGLColorMapChoiceValue,
                const bool showRepeatColorMapValue,
                const bool invertFunctionValueColorValue,
                const double functionValueMin,
                const double functionValueMax,
                const double functionValueRepetitionIntervalValue,
                const double functionValueLogarithmGammaValue,
                const uint64_t offset,
                const uint64_t range
) {
	constexpr double functionValueToTextureCoordinateXScalingFactor = 511.0/512.0;
	constexpr double functionValueToTextureCoordinateXOffset = 0.5/512.0;

	const double imageWidth = colorMapImage.width();
	const double imageHeight = colorMapImage.height();

	Vertex* currVertex = nullptr;

	QColor currentPixelColor;

	double currentFunctionValue = 0.0;

	double textureCoordinateXDouble = 0.0;
	double textureCoordinateYDouble = 0.0;

	double functionValueForTextureCoordinates = 0.0;
	double normalizedRange = 0.0;

	int64_t textureCoordinateX = 0;
	int64_t textureCoordinateY = 0;

	bool allSet = true;

	for(uint64_t vertIdx = 0; vertIdx < range; vertIdx++)
	{
		currVertex = getVertexPos(vertIdx + offset);

		if(!currVertex->getFuncValue(&currentFunctionValue))
		{
			allSet = false;
		}

		textureCoordinateYDouble = (1.0 - (20.0*static_cast<double>(
		                                       meshGLColorMapChoiceValue) + 11.0)/1024.0);

		if(showRepeatColorMapValue)
		{
			functionValueForTextureCoordinates =
			        std::fabs(2.0*std::fmod(currentFunctionValue,
			                                functionValueRepetitionIntervalValue))/
			        functionValueRepetitionIntervalValue;

			if(functionValueForTextureCoordinates > 1.0)
			{
				functionValueForTextureCoordinates = 2.0 - functionValueForTextureCoordinates;
			}

			textureCoordinateXDouble = functionValueToTextureCoordinateXScalingFactor*
			                           functionValueForTextureCoordinates +
			                           functionValueToTextureCoordinateXOffset;
		}

		else
		{
			normalizedRange = (currentFunctionValue - functionValueMin)/
			                  (functionValueMax - functionValueMin);

			if(normalizedRange < 0.0)
			{
				normalizedRange = 0.0;
			}

			else if(normalizedRange > 1.0)
			{
				normalizedRange = 1.0;
			}

			normalizedRange = std::pow(normalizedRange, functionValueLogarithmGammaValue);

			textureCoordinateXDouble = functionValueToTextureCoordinateXScalingFactor*
			                           normalizedRange +
			                           functionValueToTextureCoordinateXOffset;
		}

		if(invertFunctionValueColorValue)
		{
			textureCoordinateXDouble = 1.0 - textureCoordinateXDouble;
		}

		textureCoordinateX = std::floor(imageWidth*textureCoordinateXDouble);
		textureCoordinateY = std::floor(imageHeight*textureCoordinateYDouble);

		currentPixelColor = QColor(colorMapImage.pixel(
		                               textureCoordinateX, textureCoordinateY));

		if(!currVertex->setRGB(static_cast<unsigned char>(currentPixelColor.red()),
		                       static_cast<unsigned char>(currentPixelColor.green()),
		                       static_cast<unsigned char>(currentPixelColor.blue())))
		{
			allSet = false;
		}

		showProgress(static_cast<double>(vertIdx)/
		                        static_cast<double>(range),
		                        "Vertex function value to "
		                        "vertex RGB value transformation");

	}

	return allSet;

}


//! \brief		This function sets the Vertex color values of
//!				the current mesh to the color representation of its Vertex
//!				function values with the current function value color
//!				representation options, using all available logical processors,
//!				if more than one is available
//! \details	This function retrieves the currently used function value color
//!				representation options, queries the available logical processors
//!				available in the system. It then sets the Vertex color values of
//!				the current mesh to the color representation of its Vertex
//!				function values with the retrieved options, using all available
//!				logical processors, if more than one is available.
//!				The spawned threads call the function
//!				runFunctionValueToRGBTransformation_impl, the thread calling this
//!				function calls
//!				runFunctionValueToRGBTransformationUpdateProgress_impl, which in
//!				addition to performing the function color representation to color
//!				value transformation updates the GigaMesh GUI progress bar.
//! \return		True if all threads performed the transformation without error,
//!				false if an error occured while retrieving a Vertex object
//!				function value or setting a Vertex object color value in any of
//!				the threads performing the transformation
bool MeshGL::runFunctionValueToRGBTransformation()
{

	int meshGLColorMapChoiceValue = 0;

	bool showRepeatColorMapValue = false;
	bool invertFunctionValueColorValue = false;

	const QImage colorMapImage(":/GMShaders/funcvalmapsquare.png");

	double functionValueMin = 0.0;
	double functionValueMax = 0.0;
	double functionValueRepetitionIntervalValue = 0.0;
	double functionValueLogarithmGammaValue = 0.0;

	getParamIntMeshGL(MeshGLParams::GLSL_COLMAP_CHOICE, &meshGLColorMapChoiceValue);

	// Inversion of the colormap

	getParamFlagMeshGL(MeshGLParams::SHOW_COLMAP_INVERT, &invertFunctionValueColorValue);

	getFuncValMinMaxUser(&functionValueMin, &functionValueMax);

	// Repeat colormap

	getParamFlagMeshGL(MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &showRepeatColorMapValue);


	getParamFloatMeshGL(MeshGLParams::WAVES_COLMAP_LEN,
	                    &functionValueRepetitionIntervalValue);

	getParamFloatMeshGL(MeshGLParams::FUNC_VALUE_LOG_GAMMA,
	                    &functionValueLogarithmGammaValue);

	const unsigned int availableConcurrentThreads = std::thread::hardware_concurrency();

	bool allSet = true;

	showProgressStart("Vertex function value to vertex RGB value transformation");

	if(availableConcurrentThreads < 2)
	{
		allSet = runFunctionValueToRGBTransformationUpdateProgress_impl(colorMapImage,
		                                                                  meshGLColorMapChoiceValue,
		                                                                  showRepeatColorMapValue,
		                                                                  invertFunctionValueColorValue,
		                                                                  functionValueMin,
		                                                                  functionValueMax,
		                                                                  functionValueRepetitionIntervalValue,
		                                                                  functionValueLogarithmGammaValue,
		                                                                  0, getVertexNr());
	}

	else
	{

		const uint64_t offsetPerThread = getVertexNr()/availableConcurrentThreads;
		const uint64_t verticesPerThread = offsetPerThread;
		const uint64_t verticesForThisThread = offsetPerThread +
		                                            getVertexNr() % availableConcurrentThreads;

		std::vector<future<bool>> threadFutureHandlesVector(availableConcurrentThreads - 1);

		for(unsigned int threadCount = 0;
		    threadCount < (availableConcurrentThreads - 1); threadCount++)
		{

			auto functionCall = std::bind(&MeshGL::runFunctionValueToRGBTransformation_impl,
			                              this,
			                              colorMapImage,
			                              meshGLColorMapChoiceValue,
			                              showRepeatColorMapValue,
			                              invertFunctionValueColorValue,
			                              functionValueMin,
			                              functionValueMax,
			                              functionValueRepetitionIntervalValue,
			                              functionValueLogarithmGammaValue,
			                              offsetPerThread*threadCount,
			                              verticesPerThread);

			threadFutureHandlesVector.at(threadCount) =
			        std::async(std::launch::async, functionCall);
		}

		allSet = runFunctionValueToRGBTransformationUpdateProgress_impl(colorMapImage,
		                                                  meshGLColorMapChoiceValue,
		                                                  showRepeatColorMapValue,
		                                                  invertFunctionValueColorValue,
		                                                  functionValueMin,
		                                                  functionValueMax,
		                                                  functionValueRepetitionIntervalValue,
		                                                  functionValueLogarithmGammaValue,
		                                                  offsetPerThread*(availableConcurrentThreads - 1),
		                                                  verticesForThisThread);

		for(std::future<bool>& threadFutureHandle : threadFutureHandlesVector)
		{
			allSet &= threadFutureHandle.get();
		}

	}

	showProgressStop("Vertex function value to vertex RGB value transformation");

	MeshGL::changedVertFuncVal();
	setParamIntMeshGL(MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_RGB);

	return allSet;

}


//! \brief											This function is used by the threads
//!													spawned by the function
//!													normalizeFunctionValues to perform the
//!													actual Vertex function value normalization
//!													in parallel. The algorithm is derived from
//!													shaders/funcval.vert -> main().
//! \param showRepeatColorMapValue					The currently used colormap repeat setting
//! \param invertFunctionValueColorValue			The currently used color invertion setting
//! \param functionValueMin							The minimum function value currently set by
//!													the end-user, or if none is set, the minumum
//!													function value available in the mesh
//! \param functionValueMax							The maximum function value currently set by
//!													the end-user, or if none is set, the maximum
//!													function value available in the mesh
//! \param functionValueRepetitionIntervalValue		The currently set colormap repetition
//!													interval value
//! \param functionValueLogarithmGammaValue			The currently set logarithm gamma value
//! \param offset									The offset of the dataset of the current
//!													relative to the beginning of the Vertex
//!													object vector
//! \param range									The size of the dataset to be processed by
//!													The current thread
//! \return											False of an error occured while retrieving
//!													or setting the function value of any Vertex
//!													object in the dataset of the thread, true
//!													if all Vertex objects were processed without
//!													error
bool MeshGL::runFunctionValueNormalization_impl(const bool showRepeatColorMapValue,
													  const bool invertFunctionValueColorValue,
													  const double functionValueMin,
													  const double functionValueMax,
													  const double functionValueRepetitionIntervalValue,
													  const double functionValueLogarithmGammaValue,
                                                      const uint64_t offset,
                                                      const uint64_t range)
{

	constexpr double functionValueToTextureCoordinateXScalingFactor = 511.0/512.0;
	constexpr double functionValueToTextureCoordinateXOffset = 0.5/512.0;

	Vertex* currVertex = nullptr;

	double currentFunctionValue = 0.0;

	double normalizedFunctionValue = 0.0;

	double functionValueForTextureCoordinates = 0.0;
	double normalizedRange = 0.0;

	bool allSet = true;

	for(uint64_t vertIdx = 0; vertIdx < range; vertIdx++)
	{
		currVertex = getVertexPos(vertIdx + offset);

		if(!currVertex->getFuncValue(&currentFunctionValue))
		{
			allSet = false;
		}

		if(showRepeatColorMapValue)
		{
			functionValueForTextureCoordinates =
			        std::fabs(2.0*std::fmod(currentFunctionValue,
			                                functionValueRepetitionIntervalValue))/
			        functionValueRepetitionIntervalValue;

			if(functionValueForTextureCoordinates > 1.0)
			{
				functionValueForTextureCoordinates = 2.0 - functionValueForTextureCoordinates;
			}

			normalizedFunctionValue = functionValueToTextureCoordinateXScalingFactor*
			                           functionValueForTextureCoordinates +
			                           functionValueToTextureCoordinateXOffset;
		}

		else
		{
			normalizedRange = (currentFunctionValue - functionValueMin)/
			                  (functionValueMax - functionValueMin);

			if(normalizedRange < 0.0)
			{
				normalizedRange = 0.0;
			}

			else if(normalizedRange > 1.0)
			{
				normalizedRange = 1.0;
			}

			normalizedRange = std::pow(normalizedRange, functionValueLogarithmGammaValue);

			normalizedFunctionValue = functionValueToTextureCoordinateXScalingFactor*
			                           normalizedRange +
			                           functionValueToTextureCoordinateXOffset;
		}

		if(invertFunctionValueColorValue)
		{
			normalizedFunctionValue = 1.0 - normalizedFunctionValue;
		}


		if(!currVertex->setFuncValue(normalizedFunctionValue))
		{
			allSet = false;
		}
	}

	return allSet;

}

//! \brief											Identical to
//!													runFunctionValueNormalization_impl,
//!													except that it updates the progress bar of
//!													the GigaMesh GUI. Used by the parent thread
//!													in normalizeFunctionValues.
//! \param showRepeatColorMapValue					The currently used colormap repeat setting
//! \param invertFunctionValueColorValue			The currently used color invertion setting
//! \param functionValueMin							The minimum function value currently set by
//!													the end-user, or if none is set, the minumum
//!													function value available in the mesh
//! \param functionValueMax							The maximum function value currently set by
//!													the end-user, or if none is set, the maximum
//!													function value available in the mesh
//! \param functionValueRepetitionIntervalValue		The currently set colormap repetition
//!													interval value
//! \param functionValueLogarithmGammaValue			The currently set logarithm gamma value
//! \param offset									The offset of the dataset of the current
//!													relative to the beginning of the Vertex
//!													object vector
//! \param range									The size of the dataset to be processed by
//!													The current thread
//! \return											False of an error occured while retrieving
//!													or setting the function value of any Vertex
//!													object in the dataset of the thread, true
//!													if all Vertex objects were processed without
//!													error
bool MeshGL::runFunctionValueNormalizationUpdateProgress_impl(const bool showRepeatColorMapValue,
													  const bool invertFunctionValueColorValue,
													  const double functionValueMin,
													  const double functionValueMax,
													  const double functionValueRepetitionIntervalValue,
													  const double functionValueLogarithmGammaValue,
                                                      const uint64_t offset,
                                                      const uint64_t range)
{

	constexpr double functionValueToTextureCoordinateXScalingFactor = 511.0/512.0;
	constexpr double functionValueToTextureCoordinateXOffset = 0.5/512.0;

	Vertex* currVertex = nullptr;

	double currentFunctionValue = 0.0;

	double normalizedFunctionValue = 0.0;

	double functionValueForTextureCoordinates = 0.0;
	double normalizedRange = 0.0;

	bool allSet = true;

	for(uint64_t vertIdx = 0; vertIdx < range; vertIdx++)
	{
		currVertex = getVertexPos(vertIdx + offset);

		if(!currVertex->getFuncValue(&currentFunctionValue))
		{
			allSet = false;
		}

		if(showRepeatColorMapValue)
		{
			functionValueForTextureCoordinates =
			        std::fabs(2.0*std::fmod(currentFunctionValue,
			                                functionValueRepetitionIntervalValue))/
			                                functionValueRepetitionIntervalValue;

			if(functionValueForTextureCoordinates > 1.0)
			{
				functionValueForTextureCoordinates = 2.0 - functionValueForTextureCoordinates;
			}

			normalizedFunctionValue = functionValueToTextureCoordinateXScalingFactor*
			                           functionValueForTextureCoordinates +
			                           functionValueToTextureCoordinateXOffset;
		}

		else
		{
			normalizedRange = (currentFunctionValue - functionValueMin)/
			                  (functionValueMax - functionValueMin);

			if(normalizedRange < 0.0)
			{
				normalizedRange = 0.0;
			}

			else if(normalizedRange > 1.0)
			{
				normalizedRange = 1.0;
			}

			normalizedRange = std::pow(normalizedRange, functionValueLogarithmGammaValue);

			normalizedFunctionValue = functionValueToTextureCoordinateXScalingFactor*
			                           normalizedRange +
			                           functionValueToTextureCoordinateXOffset;
		}

		if(invertFunctionValueColorValue)
		{
			normalizedFunctionValue = 1.0 - normalizedFunctionValue;
		}

		if(!currVertex->setFuncValue(normalizedFunctionValue))
		{
			allSet = false;
		}

		showProgress(static_cast<double>(vertIdx)/
		                        static_cast<double>(range),
		                        "Vertex function value to "
		                        "vertex RGB value transformation");

	}

	return allSet;

}

//! \brief		This function normalizes the function values of all Vertex
//!				objects of the mesh, using all available logical processors,
//!				if more than one is available
//! \return		True if all threads performed the transformation without error,
//!				false if an error occured while retrieving a Vertex object
//!				function value or setting a Vertex object function value in any
//!				of the threads performing the transformation
bool MeshGL::normalizeFunctionValues()
{

	bool showRepeatColorMapValue = false;
	bool invertFunctionValueColorValue = false;

	double functionValueMin = 0.0;
	double functionValueMax = 0.0;
	double functionValueRepetitionIntervalValue = 0.0;
	double functionValueLogarithmGammaValue = 0.0;

	// Inversion of the colormap

	getParamFlagMeshGL(MeshGLParams::SHOW_COLMAP_INVERT, &invertFunctionValueColorValue);

	getFuncValMinMaxUser(&functionValueMin, &functionValueMax);

	// Repeat colormap

	getParamFlagMeshGL(MeshGLParams::SHOW_REPEAT_COLMAP_FUNCVAL, &showRepeatColorMapValue);


	getParamFloatMeshGL(MeshGLParams::WAVES_COLMAP_LEN,
	                    &functionValueRepetitionIntervalValue);

	getParamFloatMeshGL(MeshGLParams::FUNC_VALUE_LOG_GAMMA,
	                    &functionValueLogarithmGammaValue);

	const unsigned int availableConcurrentThreads = std::thread::hardware_concurrency();

	bool allSet = true;

	showProgressStart("Vertex function value normalization");

	if(availableConcurrentThreads < 2)
	{
		allSet = runFunctionValueNormalizationUpdateProgress_impl(showRepeatColorMapValue,
		                                                                  invertFunctionValueColorValue,
		                                                                  functionValueMin,
		                                                                  functionValueMax,
		                                                                  functionValueRepetitionIntervalValue,
		                                                                  functionValueLogarithmGammaValue,
		                                                                  0, getVertexNr());
	}

	else
	{

		const uint64_t offsetPerThread = getVertexNr()/availableConcurrentThreads;
		const uint64_t verticesPerThread = offsetPerThread;
		const uint64_t verticesForThisThread = offsetPerThread +
		                                            getVertexNr() % availableConcurrentThreads;

		std::vector<future<bool>> threadFutureHandlesVector(availableConcurrentThreads - 1);

		for(unsigned int threadCount = 0;
		    threadCount < (availableConcurrentThreads - 1); threadCount++)
		{

			auto functionCall = std::bind(&MeshGL::runFunctionValueNormalization_impl,
			                              this,
			                              showRepeatColorMapValue,
			                              invertFunctionValueColorValue,
			                              functionValueMin,
			                              functionValueMax,
			                              functionValueRepetitionIntervalValue,
			                              functionValueLogarithmGammaValue,
			                              offsetPerThread*threadCount,
			                              verticesPerThread);

			threadFutureHandlesVector.at(threadCount) =
			        std::async(std::launch::async, functionCall);
		}

		allSet = runFunctionValueNormalizationUpdateProgress_impl(showRepeatColorMapValue,
		                                                  invertFunctionValueColorValue,
		                                                  functionValueMin,
		                                                  functionValueMax,
		                                                  functionValueRepetitionIntervalValue,
		                                                  functionValueLogarithmGammaValue,
		                                                  offsetPerThread*(availableConcurrentThreads - 1),
		                                                  verticesForThisThread);

		for(std::future<bool>& threadFutureHandle : threadFutureHandlesVector)
		{
			allSet &= threadFutureHandle.get();
		}

	}

	showProgressStop( "Vertex function value normalization" );

	changedVertFuncVal();
    callFunction( MeshGLParams::TEXMAP_FIXED_SET_NORMALIZED );

	return( allSet );
}


//call triangle lib, to triangulate a Planar straight-line graph (PLSG) given in p
//result is returned to tri
bool MeshGL::callTriangle(vector<PixCoord> &p, vector<PixCoord> &tri) {
		struct triangulateio in, mid;

		// Define input points.

		in.numberofpoints = p.size();
		in.numberofpointattributes = 0;

		in.pointlist = static_cast<REAL *>(malloc(in.numberofpoints * 2 * sizeof(REAL)));
		for(unsigned int i=0; i<p.size(); ++i) {
				in.pointlist[2*i] = p[i].x;
				in.pointlist[2*i+1] = p[i].y;
		 }

		in.numberofsegments = p.size();
		in.numberofholes = 0;
		in.numberofregions = 0;

		in.pointmarkerlist = nullptr;
		in.segmentmarkerlist = nullptr;

		//initialize line segments
		in.segmentlist = static_cast<int *>(malloc(in.numberofsegments*2*sizeof(int)));
		for(unsigned int i=0; i<p.size()-1; ++i) {
				in.segmentlist[2*i] = i+1;
				in.segmentlist[2*i+1] = i+2;
		}
		//last point is connected with first point
		in.segmentlist[2*(p.size()-1)] = p.size();
		in.segmentlist[2*p.size() - 1] = 1;


		// Make necessary initializations so that Triangle can return a
		//   triangulation in `mid' and a voronoi diagram in `vorout'.
		mid.pointlist = nullptr;            // Not needed if -N switch used.
		// Not needed if -N switch used or number of point attributes is zero:
		mid.pointattributelist = nullptr;
		mid.pointmarkerlist = nullptr; // Not needed if -N or -B switch used.
		mid.trianglelist = nullptr;          // Not needed if -E switch used.
		// Not needed if -E switch used or number of triangle attributes is zero:
		mid.triangleattributelist = nullptr;

		// Needed only if segments are output (-p or -c) and -P not used:
		mid.segmentlist = nullptr;
		// Needed only if segments are output (-p or -c) and -P and -B not used:
		mid.segmentmarkerlist = nullptr;

		triangulate("pD", &in, &mid, nullptr);

		p.clear();
		PixCoord tmp;
		// create any new points that are necessary
		for (int i=0; i< mid.numberofpoints; i++) {
						tmp.x = mid.pointlist[i*2];
						tmp.y = mid.pointlist[i*2+1];
						p.push_back( tmp );
						mid.pointmarkerlist[i]= p.size()-1;

		}

		// create the triangles
		for (int i = 0; i < mid.numberoftriangles; i++) {
				printf("Triangle %4d points:", i);
				for (int j = 0; j < mid.numberofcorners; j++) {
						printf("  %4d", mid.trianglelist[i * mid.numberofcorners + j]);
						tri.push_back( p[mid.trianglelist[i * mid.numberofcorners + j] - 1] );
				}
				printf("\n");
		}

		free(in.pointlist);
		free(in.pointmarkerlist);
		free(in.segmentlist);
		free(in.segmentmarkerlist);
		free(mid.pointlist);
		free(mid.pointattributelist);
		free(mid.pointmarkerlist);
		free(mid.trianglelist);
		free(mid.triangleattributelist);
		free(mid.segmentlist);
		free(mid.segmentmarkerlist);

		return true;
}

//! Select vertices using a polygon.
//!
//! \todo Improve performance.
//!
//! @arg pixel coordinates defining a polygon, which is the base of a prism enclosing the vertices to be selected.
bool MeshGL::selectPrism(
                vector<PixCoord>& rTri, bool rDeselection
) {

	// Determine processing time - START
	using namespace std::chrono;
	high_resolution_clock::time_point tStart      = high_resolution_clock::now();
	high_resolution_clock::time_point tInterStart = high_resolution_clock::now();
	high_resolution_clock::time_point tInterEnd   = high_resolution_clock::now();
	duration<double> time_span;
	unsigned int segmentNr = 1;

	// Octree required - also time consuming
	if( mOctree == nullptr ) {
        generateOctree( 0.05*getVertexNr());
	}

	// Determine processing time - INTERMEDIATE
	tInterEnd = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>( tInterEnd - tInterStart );
    cout << "[Mesh::" << __FUNCTION__ << "] TIME segmemt " << segmentNr++ << ": " << time_span.count() << " seconds." << std::endl;
	tInterStart = high_resolution_clock::now();

	//vertices in cvertexlist are contained completely, no further check necessary
    vector<Octnode *> ilist;  // Vertices of a node, which is partially enclosed by the prism
    vector<Octnode *> cilist; // Vertices of a node, which is fully enclosed by the prism
	Vector3D selectBeam[6];

	for( unsigned int j=0; j<rTri.size(); j+=3 ) {
        cout << "[Mesh::" << __FUNCTION__ << "] ---------------------------------------------" << endl;
        cout << "[Mesh::" << __FUNCTION__ << "] THIS PRISM"<<endl;
		for( int i=0; i<3; ++i ) {
			getRayWorld( rTri[j+i].x, rTri[j+i].y, &selectBeam[2*i], &selectBeam[2*i+1] );
            cout<<selectBeam[2*i]<<"\t"<<selectBeam[2*i+1]<<endl;
		}
		TriangularPrism tritmp(selectBeam);

        mOctree->gettriangleintersection( ilist, cilist, mLines, tritmp );
	}

	// CILIST is often empty. However, it has to be collected.
    cout << "[Mesh::" << __FUNCTION__ << "] CILIST size: " << cilist.size() << endl;

	// Determine processing time - INTERMEDIATE
	tInterEnd = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>( tInterEnd - tInterStart );
    cout << "[Mesh::" << __FUNCTION__ << "] TIME segmemt " << segmentNr++ << ": " << time_span.count() << " seconds." << std::endl;
	tInterStart = high_resolution_clock::now();

	// vertexlist needs to be checked later
    set<Vertex*> vertexlist;
    set<Vertex*> cvertexlist;
	//copy Vertex* from Octnode* ilist to vertexlist
    for(Octnode*& octnode : ilist) {
		vertexlist.insert( octnode->mVertices.begin(), octnode->mVertices.end());
	}
	//copy Vertex* from Octnode* cilist to cvertexlist
    for(Octnode*& octnode : cilist) {
		cvertexlist.insert( octnode->mVertices.begin(), octnode->mVertices.end());
	}

	// Determine processing time - INTERMEDIATE
	tInterEnd = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>( tInterEnd - tInterStart );
    cout << "[Mesh::" << __FUNCTION__ << "] TIME segmemt " << segmentNr++ << ": " << time_span.count() << " seconds." << std::endl;
	tInterStart = high_resolution_clock::now();

	// This has to be done faster
	vector<Vertex*> realvertexlist;
	// Check if vertex is really in triangular prism
	// This is needed because it is possible there is an intersection with octree cube
	// but the vertex is not within triangular prism
	for( unsigned int j = 0; j<rTri.size(); j+=3 ) {
		for( int i=0; i<3; ++i ) {
			getRayWorld( rTri[i+j].x, rTri[i+j].y, &selectBeam[2*i], &selectBeam[2*i+1] );
		}

		TriangularPrism tritmp(selectBeam);
		// Pre-fetch planes, which is way faster than calling TriangularPrism::PointisintringularPrism.
		std::unique_ptr<Plane> prismSide1 = tritmp.gettriangularPrismPlane( 0 );
		std::unique_ptr<Plane> prismSide2 = tritmp.gettriangularPrismPlane( 1 );
		std::unique_ptr<Plane> prismSide3 = tritmp.gettriangularPrismPlane( 2 );

		for(Vertex* vertex : vertexlist) {
			Vector3D tmp = vertex->getPositionVector();
			double distance;
			prismSide1->getDistanceToPoint( &tmp, &distance );
			if( distance < 0.0 ) {
				continue;
			}
			prismSide2->getDistanceToPoint( &tmp, &distance );
			if( distance < 0.0 ) {
				continue;
			}
			prismSide3->getDistanceToPoint( &tmp, &distance );
			if( distance < 0.0 ) {
				continue;
			}
			//if( tritmp.PointisintringularPrism(tmp) ) { // <- MIGHTY SLOW
			realvertexlist.push_back(vertex);
			//}
		}
	}

	// Determine processing time - INTERMEDIATE
	tInterEnd = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>( tInterEnd - tInterStart );
    cout << "[Mesh::" << __FUNCTION__ << "] TIME segmemt " << segmentNr++ << ": " << time_span.count() << " seconds." << std::endl;
	tInterStart = high_resolution_clock::now();

	// Add to selection
    if (!rDeselection){
        Mesh::addToSelection( &realvertexlist );
        Mesh::addToSelection( cvertexlist );
    }
    else{
        Mesh::removeFromSelection( &realvertexlist );
        Mesh::removeFromSelection( cvertexlist );
    }
	// Determine processing time - INTERMEDIATE
	tInterEnd = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>( tInterEnd - tInterStart );
    cout << "[Mesh::" << __FUNCTION__ << "] TIME segmemt " << segmentNr++ << ": " << time_span.count() << " seconds." << std::endl;
	tInterStart = high_resolution_clock::now();

	// Determine processing time - STOP
	high_resolution_clock::time_point tEnd = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>( tEnd - tStart );
    cout << "[Mesh::" << __FUNCTION__ << "] TIME Total: " << time_span.count() << " seconds." << std::endl;
    cout << "[Mesh::" << __FUNCTION__ << "] ======================================================" << std::endl;

	return( true );
}

//! Polygonal/prismatic selection of vertices.
bool MeshGL::selectPoly( vector<PixCoord> &rPixels, bool rDeselection ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		//do triangulation of Planar straight-line graph (PSLG) using TRIANGLE lib
		//be careful TRIANGLE LIB crash if p.size <= 2
		if( rPixels.size()>2 ) {
				vector<PixCoord> tri;
        //	cout<<"SIZE OF TRIANGULATION VECTOR p  "<<p.size()<<endl<<endl;

				callTriangle( rPixels, tri );
        //	cout<<"SIZE OF TRIANGULATION VECTOR tri  "<<tri.size()<<endl<<endl;
                return selectPrism( tri, rDeselection );
		}

		return false;
}

//! Select a Primitve of a certain type at a given position within the Viewport.
Primitive* MeshGL::selectPrimitiveAt( int primitiveTypeToSelect, int xPixel, int yPixel, bool addToList ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		mPrimSelected = nullptr;
		if( primitiveTypeToSelect == Primitive::IS_VERTEX ) {
			    mPrimSelected = static_cast<Primitive*>( getVertexAt( xPixel, yPixel ) );
				if( ( addToList ) && ( mPrimSelected != nullptr ) ) {
					    addToSelection( static_cast<Vertex*>( mPrimSelected ) );
				}
		}
		if( primitiveTypeToSelect == Primitive::IS_FACE ) {
			    mPrimSelected = static_cast<Primitive*>( getFaceAt( xPixel, yPixel ) );
				if( ( addToList ) && ( mPrimSelected != nullptr ) ) {
						//! \todo implement addToSelection for multiple faces.
					    addToSelection( static_cast<Face*>( mPrimSelected ) );
				}
		}
		return mPrimSelected;
}

//! Select a visible Vertex at the given position within the Viewport.
Vertex* MeshGL::getVertexAt( int xPixel, int yPixel ) {
		// Fetch value from Z-Buffer entry:
		GLfloat zDepth;
		glReadPixels( xPixel, yPixel, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zDepth );
        //cout << "[MeshGL::" << __FUNCTION__ << "] Pixel ( " << xPixel << ", " << yPixel << " ) has depth " << zDepth << endl;

		Vector3D vecPointSel;
		getWorldPoint( xPixel, yPixel, zDepth, &vecPointSel );

		Vertex* vertSelected = nullptr;
		getVertexNextTo( vecPointSel, &vertSelected );
		return vertSelected;
}

//! Select a visible Face at the given position within the Viewport.
Face* MeshGL::getFaceAt( int rPixelX, int rPixelY, Vector3D* rPointIntersect ) {
		float timeStart = clock();
		// Fetch value from Z-Buffer entry:
		GLfloat zDepth;
		glReadPixels( rPixelX, rPixelY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zDepth );
        //cout << "[MeshGL] Pixel ( " << xPixel << ", " << yPixel << " ) has depth " << zDepth << endl;

		Vector3D vecPointSel;
		getWorldPoint( rPixelX, rPixelY, zDepth, &vecPointSel );

		Vector3D selectBeamTop;
		Vector3D selectBeamBot;
		getRayWorld( rPixelX, rPixelY, &selectBeamTop, &selectBeamBot );
        cout << "[MeshGL::" << __FUNCTION__ << "] time using depth buffer: " << ( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		return getFaceAt( selectBeamTop, selectBeamBot, vecPointSel, rPointIntersect );
}

//! Select a visible Face at the given position within the model space.
Face* MeshGL::getFaceAt(
                const Vector3D& rRayTop,
                const Vector3D& rRayBot,
                const Vector3D& vecPointSel,
                Vector3D* rPointIntersect
) {
	// Fetch candidate faces
	Vertex* vertNearby = nullptr;
	getVertexNextTo( vecPointSel, &vertNearby );
	vector<Face*> faceCandidates;
	vertNearby->getFaces( &faceCandidates );

	Vector3D pointIntersectionTemp;
	Face*    faceSelected = nullptr;
	unsigned int faceSelectedCtr = 0;
	for( auto const& currFace: faceCandidates ) {
		currFace->getIntersectionFacePlaneLinePos( rRayTop, rRayBot, pointIntersectionTemp );
		if( currFace->pointintriangle( &pointIntersectionTemp ) ) {
            //cout << "[MeshGL::" << __FUNCTION__ << "] Face found:" << currFace->getIndex() << endl;
			faceSelected = currFace;
			if( rPointIntersect != nullptr ) {
                rPointIntersect->set( &pointIntersectionTemp );
			}
			faceSelectedCtr++;
		}
	}
	if( faceSelectedCtr == 0 ) {
        cout << "[MeshGL::" << __FUNCTION__ << "] No face selected."  << endl;
	} else if( faceSelectedCtr > 1 ) {
        cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: More than one faces selected (" << faceSelectedCtr << ") -- the last was choosen!"  << endl;
	}

	return faceSelected;
}

//! Gets the line of view in world-coordinates for a given pixel (-> raytracing).
bool MeshGL::getRayWorld( int xPixel, int yPixel, Vector3D* rayTop, Vector3D* rayBot ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
        //cout << "[MeshGL::" << __FUNCTION__ << "] x: " << xPixel << " y: " << yPixel << endl;
		//OPENGL_UNPROJECT( (*rayTop), xPixel, yPixel, 0.0 ); // Schaft
		//OPENGL_UNPROJECT( (*rayBot), xPixel, yPixel, 1.0 ); // Spitze

		float projInv[16];
		float modvInv[16];
		invert( mMatProjection, projInv );
		invert( mMatModelView, modvInv );
		Matrix4D projMatInv( projInv );
		Matrix4D modvMatInv( modvInv );

		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
        //cout << "[MeshGL::" << __FUNCTION__ << "] Viewport: " << viewport[0] << " " << viewport[1] << " " << viewport[2] << " " << viewport[3] << endl;

		Vector3D vecTop( (static_cast<double>(xPixel*2)/static_cast<double>(viewport[2]))-1.0, (static_cast<double>(yPixel*2)/static_cast<double>(viewport[3]))-1.0, -1.0, 1.0 );
		Vector3D vecBot( (static_cast<double>(xPixel*2)/static_cast<double>(viewport[2]))-1.0, (static_cast<double>(yPixel*2)/static_cast<double>(viewport[3]))-1.0, +1.0, 1.0 );
        rayTop->set( modvMatInv * ( projMatInv * vecTop ) );
        rayBot->set( modvMatInv * ( projMatInv * vecBot ) );
		rayTop->normalizeW();
		rayBot->normalizeW();

		return true;
}

//! Converts projected screencoordiantes plus z-buffer depth value into world coordinates.
bool MeshGL::getWorldPoint( int rPixelX,       //!< Horizontal screencoordinate.
							int rPixelY,       //!< Vertical screencoordinate.
							float rDepth,      //!< Depth value of the z-buffer (retrieved by glReadPixels( x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth );
							Vector3D* rPosVec  //!< Corresponding position vector in real world coordinates.
						  ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		// Sanity check
		if( rPosVec == nullptr ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
				return false;
		}
		if( ( rDepth < 0.0f ) || ( rDepth > 1.0f ) ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: rDepth out of range " << rDepth << " != [0..1]!" << endl;
		}
		float projInv[16];
		float modvInv[16];
		invert( mMatProjection, projInv );
		invert( mMatModelView, modvInv );
		Matrix4D projMatInv( projInv );
		Matrix4D modvMatInv( modvInv );

		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
        //cout << "[MeshGL::" << __FUNCTION__ << "] Viewport: " << viewport[0] << " " << viewport[1] << " " << viewport[2] << " " << viewport[3] << endl;

		rDepth = (rDepth-0.5f) * 2.0f;

		Vector3D posVec( (static_cast<double>(rPixelX*2)/static_cast<double>(viewport[2]))-1.0, (static_cast<double>(rPixelY*2)/static_cast<double>(viewport[3]))-1.0, rDepth, 1.0 );
        rPosVec->set( modvMatInv * ( projMatInv * posVec ) );
		rPosVec->normalizeW();

		return true;
}

//! Get a point on the mesh from screencoordinates
bool MeshGL::getWorldPointOnMesh(int rPixelX, int rPixelY, Vector3D* rPosVec)
{
	return getFaceAt(rPixelX, rPixelY, rPosVec) != nullptr;
}

bool MeshGL::setVertexFuncValues( Vertex** vertices, double* values, int verticesNr, const string& setName ) {
        //! Set a value array to be visualized with a colormap.
		//! values will be copied and normalized to [0.0...1.0] - be aware of numeric errors!
		//! Returns false in case of an error.
		//! See Mesh::setVertexFuncValues

	for( int i=0; i<verticesNr; i++ ) {
		if( vertices[i] == nullptr ) {
            cerr << "[MeshGL::" << __FUNCTION__ << "] Bad vertex reference (NULL) at pos " << i << "!" << endl;
			continue;
		}
		vertices[i]->setFunctionValue( values[i] );
	}
	changedVertFuncVal();

		return true;
}

namespace {
		//! converts spherical coordinates of a point on the unis sphere to cartesian coordinates
		//! @param rPhi azimuth angle
		//! @param rTheta polar angle
		//! @returns a vector describing the point on the unit sphere specified by the angles phi and theta
		QVector3D sphericalToCartesian( double rPhi, double rTheta ) {
				return QVector3D( cos(rPhi) * sin(rTheta), sin(rPhi) * sin(rTheta), cos(rTheta) );
		}

		//! calculates almost uniformly distributed points on the unit sphere
		//! @param rSize total number of points on the unit sphere
		//! @param rIndex index of the requested point (0 <= rIndex <= rSize)
		//! @returns the requested point
		QVector3D sphericalFibonacci( unsigned int rSize, unsigned int rIndex ) {
				double phi = 2.0 * M_PI * (rIndex / (0.5 * (sqrt(5.0) + 1.0))) ;
				double z = 1 - ((2.0 * rIndex + 1.0) / (rSize));
				return sphericalToCartesian( phi, acos(z) );
		}

		//! Calculates a rotation matrix that rotates the given vector onto the non-positive part of the z-axis.
		//! Rotation is performed an axis perpendicular to the given vector and the z-axis.
		//! @param rVec given vector
		//! @returns a rotation matrix that rotates the given vector onto the non-positive part of the z-axis.
		QMatrix4x4 rotateVecOntoZ( const QVector3D &rVec ){
				QVector3D zAxis(0,0,-1);
				QVector3D rotationAxis = QVector3D::crossProduct(rVec, zAxis);
				float rotationAngle = acos(QVector3D::dotProduct(rVec, zAxis) / rVec.length()) * 180.0 / M_PI;
				QMatrix4x4 rotationMat;
				rotationMat.rotate(rotationAngle, rotationAxis);
				return rotationMat;
		}
}

//! Sets function values to values representing local brightness using ambient occlusion and sets parameters for suitable visualization
//! Asks for parameters for internal usage via ui dialog
//! Asks for combination with vertex colors after computation of ambient occlusion
//! @returns False in case of an error. True otherwise.
bool MeshGL::funcVertAmbientOcclusion() {
		vector<double> parameters(4,0);
		parameters[0] = 512;
		parameters[1] = 1000;
		parameters[2] = 512;
		parameters[3] = 0.0;

		if( !showEnterText( parameters, "Depth buffer resolution, number of directions, maximum value buffer resolution and tolerance (4 values)" ) ) {
				return false;
		}

		if( parameters.size() != 4 ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] Wrong number of elements (" << parameters.size() << ") given 4 expected!" << endl;
				return false;
		}

		if ( parameters[0] < 1 ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] Depth buffer resolution has to be >= 1 (given depth buffer resolution: " << parameters[0] << ")" << endl;
				return false;
		}

		if ( parameters[1] < 0 ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] Number of directions has to be >= 0 (given number of directions: " << parameters[1] << ")" << endl;
				return false;
		}

		if ( parameters[2] < 0 ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] Maximum value buffer resolution hast to be >= 0 (given maximum texture resolution: " << parameters[2] << ")" << endl;
				return false;
		}

		if ( parameters[3] < 0 || parameters[3] > 1) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] Tolerance has to be >= 0 and <= 1 (given tolerance: " << parameters[3] << ")" << endl;
				return false;
		}

		// Fetch values
		int depthBuffeRes = static_cast<int>(parameters[0]);
		unsigned int numberOfDirections = static_cast<unsigned int>(parameters[1]);
		int maxValueBufferRes = static_cast<int>(parameters[2]);
		float zTolerance = static_cast<float>(parameters[3]);

		if (maxValueBufferRes <= 0){
				if( !funcVertAmbientOcclusion( depthBuffeRes, numberOfDirections, zTolerance ) ) {
						return false;
				}
		} else {
				if( !funcVertAmbientOcclusionHW( depthBuffeRes, maxValueBufferRes, numberOfDirections, zTolerance ) ) {
						return false;
				}
		}

        //set parameters for suitable visualization
		float minAOValue = 0.0f;
		float maxAOValue = numberOfDirections / 4.0f; // normalized surface integral over hemisphere: 1/(4*pi) int_0^{2*pi} int_0^{pi/2} cos(x) sin(x) dx dy = 1/4
		setParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE, MeshGLParams::GLSL_COLMAP_GRAYSCALE );
		setParamFloatMeshGL( MeshGLParams::TEXMAP_AUTO_MIN, minAOValue );
		setParamFloatMeshGL( MeshGLParams::TEXMAP_AUTO_MAX, maxAOValue );
		// setParamIntMeshGL( MeshGLParams::FUNCVAL_CUTOFF_CHOICE, MeshGLParams::FUNCVAL_CUTOFF_MINMAX_AUTO ); // Optimal, but Quantil provides a nicer impression.
		setParamIntMeshGL( MeshGLParams::FUNCVAL_CUTOFF_CHOICE, MeshGLParams::FUNCVAL_CUTOFF_QUANTIL );
		mWidgetParams->setParamFlagMeshWidget( MeshWidgetParams::LIGHT_ENABLED, false);

		//combination with vertex colors depending on user input
		bool combineWithColor;
		if ( !showQuestion( &combineWithColor, "Combine Color with AO", "Combine vertex color with ambient occlusion?") ) {
				return false;
		}

		if ( combineWithColor ) {
				multiplyColorWithFuncVal( minAOValue, maxAOValue );
				setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_RGB );
		}

		return true;
}

//! Sets function values to values representing local brightness using ambient occlusion
//! Internally uses the given parameters
//! @param rResolution resolution of the depth buffer that is internally used to check occlusion
//! @param rNumberOfDirections number of uniformly distributed directions that are used for casting parallel light rays from the given direction onto the mesh
//! @param rZTolerance tolerance that is used during occlusion checks. The values are relative to 2r where r denotes the bojects bounding box radius
//! @returns False in case of an error. True otherwise.
bool MeshGL::funcVertAmbientOcclusion( int rResolution, unsigned int rNumberOfDirections, float rZTolerance ) {
        cout << "[MeshGL::" << __FUNCTION__ << "] Calculating Ambient Occlusion..." << endl;
		showProgressStart( "Ambient Occlusion" );

		setVertFuncVal(0.0);

		Vector3D center = getBoundingBoxCenter();

		double radius = getBoundingBoxRadius();

		double scaleFactor = 1.0 / radius;

		QMatrix4x4 transformToClipspace;
		transformToClipspace.setRow(2,-transformToClipspace.row(2));
		//these transformations are done by multiplying the given matrix with a matrix performing the transformation from the right.
		transformToClipspace.scale(scaleFactor);
		transformToClipspace.translate(-center.getX(), -center.getY(), -center.getZ());

		//save viewport, polygon offset and depth test to restore them later
		GLboolean previousDepthTestState = glIsEnabled(GL_DEPTH_TEST);

		GLboolean previousPolygonOffsetFillState = glIsEnabled(GL_POLYGON_OFFSET_FILL);

		GLfloat previousPolygonOffsetFactor;
		glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &previousPolygonOffsetFactor);

		GLfloat previousPolygonOffsetUnits;
		glGetFloatv(GL_POLYGON_OFFSET_UNITS, &previousPolygonOffsetUnits);

		GLint previousViewport[4];
		glGetIntegerv(GL_VIEWPORT, previousViewport);

		//setup
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		glViewport(0, 0, rResolution, rResolution);

		QOpenGLFramebufferObject fbo( rResolution, rResolution, QOpenGLFramebufferObject::Depth, GL_TEXTURE_2D, GL_R8 );
		if( !fbo.bind() ) {
				return false;
		}

		vector<float> depths( rResolution * rResolution, 0 );

		for (unsigned int i = 0; i < rNumberOfDirections; i++) {
				showProgress( double(i) / double(rNumberOfDirections), "Ambient Occlusion" );

				QMatrix4x4 transform = rotateVecOntoZ(sphericalFibonacci( rNumberOfDirections, i )) * transformToClipspace;

				glPaintDepth( transform );
				glReadPixels(0, 0, rResolution, rResolution, GL_DEPTH_COMPONENT, GL_FLOAT, &depths[0]);

				Matrix4D transform2(transform.data());

				if ( !funcVertAddLight(transform2, rResolution, rResolution, depths, rZTolerance) ) {
						return false;
				}
		}

		if( !fbo.release() ) {
				return false;
		}

		//restore previous viewport, polygon offset and depth test
		glViewport(previousViewport[0],previousViewport[1],previousViewport[2],previousViewport[3]);
		glPolygonOffset(previousPolygonOffsetFactor, previousPolygonOffsetUnits);
		if(!previousPolygonOffsetFillState){
				glDisable(GL_POLYGON_OFFSET_FILL);
		}
		if(!previousDepthTestState){
				glDisable(GL_DEPTH_TEST);
		}

		int defaultFramebuffer = 0;
		getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

		using PglBindFramebuffer = void (*)(GLenum, GLuint);
		PglBindFramebuffer bindFramebuffer = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer") );

		bindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

		changedVertFuncVal();

		showProgressStop( "Ambient Occlusion" );
        cout << "[MeshGL::" << __FUNCTION__ << "] Done." << endl;

		return true;
}

//! Sets function values to values representing local brightness using hardware accelerated ambient occlusion
//! Internally uses the given parameters
//! @param rDepthBufferRes resolution of the depth buffer that is internally used to check occlusion
//! @param rMaxValueBufferRes maximum horizontal and vertical resolution of a framebuffer that will be used for buffering ambient occlusion values
//! @param rNumberOfDirections number of uniformly distributed directions that are used for casting parallel light rays from the given direction onto the mesh
//! @param rZTolerance tolerance that is used during occlusion checks. The values are relative to 2r where r denotes the bojects bounding box radius
//! @returns False in case of an error. True otherwise.
bool MeshGL::funcVertAmbientOcclusionHW( int rDepthBufferRes, int rMaxValueBufferRes, unsigned int rNumberOfDirections, float rZTolerance ) {
        cout << "[MeshGL::" << __FUNCTION__ << "] Calculating hardware accelerated Ambient Occlusion..." << endl;

		Vector3D center = getBoundingBoxCenter();

		double radius = getBoundingBoxRadius();

		double scaleFactor = 1.0 / radius;

		QMatrix4x4 transformToClipspace;
		transformToClipspace.setRow(2,-transformToClipspace.row(2));
		//these transformations are done by multiplying the given matrix with a matrix performing the transformation from the right.
		transformToClipspace.scale(scaleFactor);
		transformToClipspace.translate(-center.getX(), -center.getY(), -center.getZ());

		//save OpenGL parameters to restore them later
		GLboolean previousDepthTestState = glIsEnabled(GL_DEPTH_TEST);

		GLboolean previousPolygonOffsetFillState = glIsEnabled(GL_POLYGON_OFFSET_FILL);

		GLboolean previousBlendState = glIsEnabled(GL_BLEND);

		GLint previousBlendSrc;
		glGetIntegerv(GL_BLEND_SRC, &previousBlendSrc);

		GLint previousBlendDst;
		glGetIntegerv(GL_BLEND_DST, &previousBlendDst);

		GLint previousBlendEquation;
		glGetIntegerv(GL_BLEND_EQUATION, &previousBlendEquation);

		GLint previousDepthRange[2];
		glGetIntegerv(GL_DEPTH_RANGE, previousDepthRange);

		GLint previousViewport[4];
		glGetIntegerv(GL_VIEWPORT, previousViewport);

		GLfloat previousPolygonOffsetFactor;
		glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &previousPolygonOffsetFactor);

		GLfloat previousPolygonOffsetUnits;
		glGetFloatv(GL_POLYGON_OFFSET_UNITS, &previousPolygonOffsetUnits);

		GLfloat previousClearColor[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, previousClearColor);

		//setup
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		glBlendFunc(GL_ONE, GL_ONE);

		//required for windows...
		using PglBlendEquation = void (*)(GLenum);
		PglBlendEquation blendEquation = reinterpret_cast<PglBlendEquation>(mOpenGLContext->getProcAddress("glBlendEquation") );

		blendEquation(GL_FUNC_ADD);
		glDepthRange(0.0,1.0);

		//calculate resolution of value buffer
		int valueBufferRes = 1;
		if( static_cast<unsigned int>(rMaxValueBufferRes * rMaxValueBufferRes) <= getVertexNr()) {
				valueBufferRes = rMaxValueBufferRes;
		} else {
				valueBufferRes = pow( 2, ceil(0.5 * log(getVertexNr()) / log(2)));
		}

		//create and prepare framebuffer objects
		QOpenGLFramebufferObject depthFBO( rDepthBufferRes, rDepthBufferRes, QOpenGLFramebufferObject::Depth, GL_TEXTURE_2D, GL_R32F );

		QOpenGLFramebufferObject valueFBO( valueBufferRes, valueBufferRes, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D, GL_R32F );

		unsigned int nrOfPasses = (getVertexNr() / (valueBufferRes * valueBufferRes)) + 1;
		showProgressStart( "Ambient occlusion to Function Value" );
		for (unsigned int passNr = 0; passNr < nrOfPasses; passNr++) {
                cout << "[MeshGL::" << __FUNCTION__ << "] Hardware accelerated ambient occlusion pass " << (passNr + 1) << " of " << nrOfPasses << endl;
				if( !valueFBO.bind() ) {
						return false;
				}
				glClearColor(0.0, 0.0, 0.0, 1.0);
				glDisable(GL_BLEND);
				glClear(GL_COLOR_BUFFER_BIT);

				//choose correct clear color for clearing depthFBO
				glClearColor(1.0, 1.0, 1.0, 1.0);

				funcVertAmbientOcclusionHWPass( transformToClipspace, depthFBO, valueFBO, rNumberOfDirections,
												rZTolerance, passNr, nrOfPasses, passNr * valueBufferRes * valueBufferRes );
		}
		showProgressStop( "Ambient occlusion to Function Value" );

		if( !valueFBO.bindDefault() ) {
				return false;
		}

		//restore previous OpenGL parameters
		int defaultFramebuffer = 0;
		getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);

		using PglBindFramebuffer = void (*)(GLenum, GLuint);
		PglBindFramebuffer bindFramebuffer = reinterpret_cast<PglBindFramebuffer>(mOpenGLContext->getProcAddress("glBindFramebuffer") );

		bindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);


		previousDepthTestState ? glEnable( GL_DEPTH_TEST ) : glDisable( GL_DEPTH_TEST );
		previousPolygonOffsetFillState ? glEnable( GL_POLYGON_OFFSET_FILL ) : glDisable( GL_POLYGON_OFFSET_FILL );
		previousBlendState ? glEnable( GL_BLEND ) : glDisable( GL_BLEND ) ;
		glBlendFunc( previousBlendSrc, previousBlendDst );
		blendEquation( previousBlendEquation );
		glDepthRange( previousDepthRange[0], previousDepthRange[1] );
		glViewport( previousViewport[0],previousViewport[1],previousViewport[2],previousViewport[3] );
		glPolygonOffset( previousPolygonOffsetFactor, previousPolygonOffsetUnits );
		glClearColor( previousClearColor[0], previousClearColor[1], previousClearColor[2], previousClearColor[3] );

		changedVertFuncVal();

        cout << "[MeshGL::" << __FUNCTION__ << "] Done." << endl;

		return true;
}

//! Sets function values of specified vertices to values representing local brightness using hardware accelerated ambient occlusion
//! Internally uses the given parameters and expects correct setup of OpenGL parameters (e.g. poylgon offset)
//! @param rTransformToClipspace after transforming the mesh into clipspace using the transformation specified by this matrix it is rotated to determine occlusion from different directions.
//! @param rDepthFBO framebuffer object for repeatadly filling its depth buffer and using these depth values as a texture
//! @param rValueFBO framebuffer object for accumulating light intensities per vertex
//! @param rNumberOfDirections number of uniformly distributed directions that are used for casting parallel light rays from the given direction onto the mesh
//! @param rZTolerance tolerance that is used during occlusion checks. The values are relative to 2r where r denotes the bojects bounding box radius
//! @param rPassNr number of this pass in a chain of ambient occlusion passes (only for progress notification)
//! @param rNrOfPasses total number of passes in the corresponding chain of ambient occlusion passes (only for progress notification)
//! @param rStartVertIndex function values representing ambient occlusion will be calculated for rValueFBO.width() * rValueFBO.height() many vertices beginning at the vertex with index rStartVertIndex
//! @returns False in case of an error. True otherwise.
bool MeshGL::funcVertAmbientOcclusionHWPass(
				const QMatrix4x4 &rTransformToClipspace,
				QOpenGLFramebufferObject &rDepthFBO,
				QOpenGLFramebufferObject &rValueFBO,
				unsigned int rNumberOfDirections,
				float rZTolerance,
				unsigned int rPassNr,
				unsigned int rNrOfPasses,
				unsigned int rStartVertIndex
) {
		for( unsigned int i=0; i<rNumberOfDirections; i++ ) {
				showProgress( double(rPassNr)/double(rNrOfPasses) + double(i)/double(rNumberOfDirections*rNrOfPasses),
							  "Ambient occlusion to Function Value" );

				QMatrix4x4 transform = rotateVecOntoZ(sphericalFibonacci( rNumberOfDirections, i )) * rTransformToClipspace;

				// generate depth information
				if( !rDepthFBO.bind() ) {
						return false;
				}
				glViewport(0, 0, rDepthFBO.width(), rDepthFBO.height());
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_BLEND);//! @todo restore default
				glPaintDepth( transform );

				// use depth information to simulate additive frontal lighting
				if( !rValueFBO.bind() ) {
						return false;
				}
				glViewport(0, 0, rValueFBO.width(), rValueFBO.height());
				glDisable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				glPaintFrontalLightPerVertex( transform, rValueFBO.width(), rValueFBO.height(), rDepthFBO.texture(), rZTolerance, rStartVertIndex );
		}

		if( !rValueFBO.bind() ) {
				return false;
		}

		vector<float> values(rValueFBO.width() * rValueFBO.height(), 0);
		glReadPixels(0, 0, rValueFBO.width(), rValueFBO.height(), GL_RED, GL_FLOAT, &values[0]);

		unsigned int nrOfVertices = getVertexNr();

		for( unsigned int vertIdx = rStartVertIndex; vertIdx < min(rStartVertIndex + rValueFBO.width() * rValueFBO.height(), nrOfVertices); vertIdx++ ) {
				Vertex* vertex = getVertexPos( vertIdx );
				vertex->setFuncValue( values[vertIdx - rStartVertIndex]);
		}

		return true;
}

//! This method wraps some of the flags and parameters to return the minimum and maximum value for visualizing the function value as color map.
//! Relates to:
//! SHOW_FUNC_VALUES_QUANTIL
//! SHOW_FUNC_VALUES_FIXED_MINMAX
//! TEXMAP_QUANTIL_MIN
//! TEXMAP_QUANTIL_MAX
//! TEXMAP_FIXED_MIN
//! TEXMAP_FIXED_MAX
//! --- newly introduced, for performance reasons:
//! TEXMAP_AUTO_MIN
//! TEXMAP_AUTO_MAX
//! TEXMAP_QUANTIL_MIN_ABSOLUT
//! TEXMAP_QUANTIL_MAX_ABSOLUT
bool MeshGL::getFuncValMinMaxUser( double* rMin, double* rMax ) {
	// Compute the minimum and maximum of all function values, if not pre-computed.
	double texmapAutoMin = 0.0;
	double texmapAutoMax = 0.0;
	getParamFloatMeshGL( TEXMAP_AUTO_MIN, &texmapAutoMin );
	getParamFloatMeshGL( TEXMAP_AUTO_MAX, &texmapAutoMax );
	if( ( !std::isfinite( texmapAutoMin ) ) || ( !std::isfinite( texmapAutoMax ) ) ) {
		double valMin = +DBL_MAX;
		double valMax = -DBL_MAX;
		//! \todo here exists a performance issue, when all function values are not finite! getFuncValuesMinMax will ALWAYS step thru all vertices.
		if( !getFuncValuesMinMax( valMin, valMax ) ) {
			// Suppress error for meshes having no finite function value.
			if( !std::isfinite( valMin ) || !std::isfinite( valMax ) ) {
                cout << "[MeshGL::" << __FUNCTION__ << "] ERROR: getFuncValuesMinMax failed!" << endl;
			}
			return( false );
		}
        cout << "[MeshGL::" << __FUNCTION__ << "] min Value:        " << valMin << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] max Value:        " << valMax << endl;
		setParamFloatMeshGL( TEXMAP_AUTO_MIN, valMin );
		setParamFloatMeshGL( TEXMAP_AUTO_MAX, valMax );
	}
	// Reload values in case they changed above.
	getParamFloatMeshGL( TEXMAP_AUTO_MIN, &texmapAutoMin );
	getParamFloatMeshGL( TEXMAP_AUTO_MAX, &texmapAutoMax );
	// Fetch user-defined minimum and maximum.
	double texmapFixedMin;
	double texmapFixedMax;
	getParamFloatMeshGL( TEXMAP_FIXED_MIN, &texmapFixedMin );
	getParamFloatMeshGL( TEXMAP_FIXED_MAX, &texmapFixedMax );
    // Set a default for manual/fixed function value range.
	if( ( !std::isfinite( texmapFixedMin ) ) || ( !std::isfinite( texmapFixedMax ) ) ) {
        // Set only if there is a valid range:
		if( isnormal( texmapAutoMax - texmapAutoMin ) ) {
			texmapFixedMin = texmapAutoMin;
			texmapFixedMax = texmapAutoMax;
			setParamFloatMeshGL( TEXMAP_FIXED_MIN, texmapAutoMin );
			setParamFloatMeshGL( TEXMAP_FIXED_MAX, texmapAutoMax );
		}
	}

	// Find absolute values for the Quantile range.
	double texmapQuantilMinAbsolut;
	double texmapQuantilMaxAbsolut;
	getParamFloatMeshGL( TEXMAP_QUANTIL_MIN_ABSOLUT, &texmapQuantilMinAbsolut );
	getParamFloatMeshGL( TEXMAP_QUANTIL_MAX_ABSOLUT, &texmapQuantilMaxAbsolut );
	if( ( std::isnan( texmapQuantilMinAbsolut ) ) || ( std::isnan( texmapQuantilMaxAbsolut ) ) ) {
		    // Fallbacks in case of an error:
		    double valMin;
			double valMax;
			getParamFloatMeshGL( TEXMAP_AUTO_MIN, &valMin );
			getParamFloatMeshGL( TEXMAP_AUTO_MAX, &valMax );
			// Fetch quantil values in percent.
			double texmapQuantilMin;
			double texmapQuantilMax;
			getParamFloatMeshGL( TEXMAP_QUANTIL_MIN, &texmapQuantilMin );
			getParamFloatMeshGL( TEXMAP_QUANTIL_MAX, &texmapQuantilMax );

			if( !getFuncValuesMinMaxQuantil( texmapQuantilMin, texmapQuantilMax, valMin, valMax ) ) {
                    cout << "[MeshGL::" << __FUNCTION__ << "] ERROR: getFuncValuesMinMax failed!" << endl;
					return( false );
			}

            cout << "[MeshGL::" << __FUNCTION__ << "] min Value:    " << valMin << " (Quantile)" << endl;
            cout << "[MeshGL::" << __FUNCTION__ << "] max Value:    " << valMax << " (Quantile)" << endl;
			setParamFloatMeshGL( TEXMAP_QUANTIL_MIN_ABSOLUT, valMin );
			setParamFloatMeshGL( TEXMAP_QUANTIL_MAX_ABSOLUT, valMax );
	}

    // Set cutoff value on the users choice for the shader.
	eFuncValCutoff cutoffChoice;
	getParamIntMeshGL( FUNCVAL_CUTOFF_CHOICE, reinterpret_cast<int*>(&cutoffChoice) );
	switch( cutoffChoice ) {
		    case FUNCVAL_CUTOFF_QUANTIL:
			        getParamFloatMeshGL( TEXMAP_QUANTIL_MIN_ABSOLUT, rMin );
					getParamFloatMeshGL( TEXMAP_QUANTIL_MAX_ABSOLUT, rMax );
			        break;
		    case FUNCVAL_CUTOFF_MINMAX_USER:
			        getParamFloatMeshGL( TEXMAP_FIXED_MIN, rMin );
					getParamFloatMeshGL( TEXMAP_FIXED_MAX, rMax );
			        break;
		    case FUNCVAL_CUTOFF_MINMAX_AUTO:
			        getParamFloatMeshGL( TEXMAP_AUTO_MIN, rMin );
					getParamFloatMeshGL( TEXMAP_AUTO_MAX, rMax );
			        break;
		    default:
                    cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Invalid choice for cutoff value." << endl;
			        return( false );
	}

	return( true );
}

//! Compute the distance for steps of isolines.
//!
//! @returns false in case of an error. True otherwise.
bool MeshGL::getFuncValIsoStepping( unsigned int rNumberOfIsoLines, double* rIsoDistance ) {
		if( rIsoDistance == nullptr ) {
				return( false );
		}
		double funcValMin = 0.0;
		double funcValMax = 0.0;
		getFuncValMinMaxUser( &funcValMin, &funcValMax );
		double isoDistance = ( funcValMax-funcValMin )/static_cast<double>(rNumberOfIsoLines);
		(*rIsoDistance) = isoDistance;
		return( true );
}

//! User Interactive: set isoline distance and offset.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool MeshGL::setFuncValIsoSteppingByNumberUI() {
	    uint64_t numberOfIsoLines = 5;
		if( !showEnterText( numberOfIsoLines, "Number of Isolines" ) ) {
				return( false );
		}
		bool setIsoOffsetZero = true;
        if( !showQuestion( &setIsoOffsetZero, string( "Reset Isovalue Offset" ), string( "Set the offset of the isolines to zero?" ) ) ) {
				return( false );
		}
		// Execute, when no user cancel:
		bool retVal = setFuncValIsoSteppingByNumber( numberOfIsoLines, setIsoOffsetZero );
		return( retVal );
}

//! Set isoline distance by number and optionally set their offset to zero.
//!
//! @returns false in case of an error. True otherwise.
bool MeshGL::setFuncValIsoSteppingByNumber(
				unsigned int rNumberOfLines,
				bool         rSetOffsetToZero
) {
		double isoDistance;
		getFuncValIsoStepping( rNumberOfLines, &isoDistance );
		bool retVal  = setParamFloatMeshGL( MeshGLParams::ISOLINES_DISTANCE, isoDistance );
		if( rSetOffsetToZero ) {
				retVal |= setParamFloatMeshGL( MeshGLParams::ISOLINES_OFFSET, 0.0 );
		}
		return( retVal );
}

//! Prepares OpenGL for Mesh display.
//! Load some initial Vertex Buffer Objects (VBOs).
void MeshGL::glPrepare() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		PRINT_OPENGL_ERROR( "OLD_ERROR" );

		if( mVBOPrepared ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Buffers were already prepared!" << endl;
				return;
		}


		const auto timeStart    = clock(); // for performance mesurement
		const auto timeStartSub = clock(); // for performance mesurement

		PglGenVertexArrays glGenVertexArrays = reinterpret_cast<PglGenVertexArrays>(mOpenGLContext->getProcAddress( "glGenVertexArrays" ));
		glGenVertexArrays( 1, &mVAO );
        cout << "[MeshGL::" << __FUNCTION__ << "] glGenVertexArrays --- mVAO = " << mVAO << endl;
		for(QOpenGLBuffer*& mVertBufObj : mVertBufObjs) {
			    mVertBufObj = nullptr;
		}

		// --- Unit sized objects ------------------------------------------------------------------------------------------------------------------------------
		vboPrepareBox();
		vboPrepareCylinder();
		vboPrepareSphere();
		vboPrepareQuad();

		// especially as double cone estimation takes quite some time, we store indices to "intersting" vertices in a vector array:
		vector<GLuint> vertexSoloTmp;
		vector<GLuint> vertexBorderTmp;
		vector<GLuint> vertexNonManifoldTmp;

		//for( const auto& curVertex : mVertices ) {
		Vertex* curVertex;
		for( uint64_t vertexCtr=0; vertexCtr<getVertexNr(); vertexCtr++ ) {
				curVertex = getVertexPos( vertexCtr );
				curVertex->setIndex( vertexCtr );
				switch( curVertex->getState() ) {
						case _PRIMITIVE_STATE_SOLO_:
								vertexSoloTmp.push_back( static_cast<GLuint>(vertexCtr) );
						break;
						case _PRIMITIVE_STATE_BORDER_:
								vertexBorderTmp.push_back( static_cast<GLuint>(vertexCtr) );
						break;
						case _PRIMITIVE_STATE_MANIFOLD_:
								// nothing to add - better for memory to use monochrome vertices and draw all others larger in different color
						break;
						case _PRIMITIVE_STATE_NON_MANIFOLD_:
								vertexNonManifoldTmp.push_back( static_cast<GLuint>(vertexCtr) );
						break;
				default:
                        cerr << "[MeshGL::" << __FUNCTION__ << "] unknown Vertex state at index " << curVertex->getIndexOriginal() << "!" << endl;
				}
		}
        cout << "[MeshGL::" << __FUNCTION__ << "] Time Vertices: " << static_cast<float>( clock() - timeStartSub ) / CLOCKS_PER_SEC << " seconds."  << endl;

		// --- Extra infos -------------------------------------------------------------------------------------------------------------------------------------
		mVertBufObjs[VBUFF_VERTICES_SOLO]        = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );
		mVertBufObjs[VBUFF_VERTICES_BORDER]      = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );
		mVertBufObjs[VBUFF_VERTICES_NONMANIFOLD] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );

		// --- Vertex references Solo ----------------------------------------------
		vboAddBuffer( sizeof(GLuint)*vertexSoloTmp.size(), vertexSoloTmp.data(), QOpenGLBuffer::StaticDraw, VBUFF_VERTICES_SOLO, __FUNCTION__ );
		// --- Vertex references Border --------------------------------------------
		vboAddBuffer( sizeof(GLuint)*vertexBorderTmp.size(), vertexBorderTmp.data(), QOpenGLBuffer::StaticDraw, VBUFF_VERTICES_BORDER, __FUNCTION__ );
		// --- Vertex references Non-Manifold --------------------------------------
		vboAddBuffer( sizeof(GLuint)*vertexNonManifoldTmp.size(), vertexNonManifoldTmp.data(), QOpenGLBuffer::StaticDraw, VBUFF_VERTICES_NONMANIFOLD, __FUNCTION__ );

		// --- Vertex references Double Cones --------------------------------------
		//vboPrepareDoubleCone(); // Skip this here, makes startup faster.

		// === FACES (texture per Vertex) ===================================================================
		vector<GLuint> faceIndices;
		faceIndices.reserve( getFaceNr()*3 );

		//for( const auto& currFace : mFaces ) {
		Face* currFace;
		for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
				currFace = getFacePos( faceIdx );
				faceIndices.push_back( static_cast<GLuint>(currFace->getVertA()->getIndex()) );
				faceIndices.push_back( static_cast<GLuint>(currFace->getVertB()->getIndex()) );
				faceIndices.push_back( static_cast<GLuint>(currFace->getVertC()->getIndex()) );
                //cout << "[MeshGL::prepareVBOs] Face: " << faces[faceCtr*3] << " - " << faces[faceCtr*3+1] << " - " << faces[faceCtr*3+2] << endl;
		}

		mVertBufObjs[VBUFF_FACES] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );
		vboAddBuffer( sizeof(GLuint)*faceIndices.size(), faceIndices.data(), QOpenGLBuffer::StaticDraw, VBUFF_FACES, __FUNCTION__ );

		const auto timeStop = clock();
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
        cout << "[MeshGL::" << __FUNCTION__ << "] Memory: " << mVboMemoryUsage << " Bytes = " << mVboMemoryUsage/(1024*1024) << " MB" << endl;
#endif
        cout << "[MeshGL::" << __FUNCTION__ << "] Time: " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		mVBOPrepared = true;
}

//! Paints the Mesh using OpenGL.
void MeshGL::glPaint() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		//! *) Prepare VBOs if necessary.
		if( !mVBOPrepared ) {
				glPrepare();
		}
}

//! Removes Mesh data from the graphic card.
void MeshGL::glRemove() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
        // no general free as not all arrays might not be set!: glDeleteBuffers( VBO_ARRAY_COUNT, vboId );
		for( int i=0; i<VBUFF_COUNT; i++ ) {
				vboRemoveBuffer( static_cast<eVertBufObjs>(i), __FUNCTION__ );
		}

		if(mMeshTextured != nullptr)
		{
			delete mMeshTextured;
			mMeshTextured = nullptr;
		}
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
		mVboMemoryUsage = 0;
        cout << "[MeshGL::" << __FUNCTION__ << "] VBOs deleted." << endl;
#endif
		mVBOPrepared = false;
}

//! Preparation for painting the mesh
void MeshGL::glPaintDepth( const QMatrix4x4 &rTransformMat  ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		//! *) Prepare VBOs if necessary.
		if( !mVBOPrepared ) {
				glPrepare();
		}
}

//! Preparation for painting the mesh
void MeshGL::glPaintFrontalLightPerVertex( const QMatrix4x4 &rTransformMat ,
                                                                                   GLint rXResolution ,
                                                                                   GLint rYResolution ,
                                                                                   GLuint rDepthTexture ,
                                                                                   GLfloat rZTolerance ,
                                                                                   GLint rFirstVertIdx  ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		//! *) Prepare VBOs if necessary.
		if( !mVBOPrepared ) {
				glPrepare();
		}
		if ( !vboPrepareVerticesStriped() ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Preparing Vertices and related data as VBO stripe failed!" << endl;
		}
}

//! Removes vertices and removes OpenGL VBOs and lists.
//! See Mesh::removeVertices for further details.
bool MeshGL::removeVertices( set<Vertex*>* verticesToRemove ) {
		bool retVal = Mesh::removeVertices( verticesToRemove );
		glRemove();
		glPrepare();
		return retVal;
}

//! Manually insert vertices by entering triplets of coordinates.
//! OpenGL method for updating buffers ... see GUI for usage.
//! @returns true in case of an error. False otherwise.
bool MeshGL::insertVerticesCoordTriplets( vector<double>* rCoordTriplets ) {
		bool retVal = Mesh::insertVerticesCoordTriplets( rCoordTriplets );
		glRemove();
		glPrepare();
		return retVal;
}

//! Insert new vertices given by a vector of references.
//! OpenGL method for updating buffers.
//! @returns true in case of an error. False otherwise.
bool MeshGL::insertVertices( vector<Vertex*>* rNewVertices ) {
		// Sanity check
		if( rNewVertices == nullptr ) {
				return false;
		}
		// Prevent unnessary updates:
		if( rNewVertices->size() == 0 ) {
				return true;
		}
		bool retVal = Mesh::insertVertices( rNewVertices );
		glRemove();
		glPrepare();
		return retVal;
}

// Parameters --------------------------------------------------------------------------------------------------------------------------------------------------

//! Set flag controlling the display of Primitives, etc.
bool MeshGL::setParamFlagMeshGL( MeshGLParams::eParamFlag rParamID, bool rState ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
	if( !MeshGLParams::setParamFlagMeshGL( rParamID, rState ) ) {
		// Prevent infinite loops and unnecessary changes.
		return( false );
	}
	// Done.
	return( true );
}

//! Set an integer parameter.
//! @returns true when a change was made -- false otherwise.
bool MeshGL::setParamIntMeshGL( MeshGLParams::eParamInt rParamID, int rValue ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "] rParamID: " << rParamID << " rValue: " << rValue << endl;
#endif
		// Prevent infinite loops and unnecessary changes.
		if( !MeshGLParams::setParamIntMeshGL( rParamID, rValue ) ) {
				return false;
		}

		//remove textured buffer if not needed
		if(rParamID == MeshGLParams::SHADER_CHOICE)
		{
			if(rValue != SHADER_TEXTURED)
			{
				if(mMeshTextured != nullptr)
				{
					delete mMeshTextured;
					mMeshTextured = nullptr;
				}
			}
		}

		return true;
}

//! Set the parameters floating point value
//!
//! Sets the parameters for ambient light, materials, etc.
//! Ignores values out of range.
//!
//! @returns true when the value was changed. false otherwise.
bool MeshGL::setParamFloatMeshGL( MeshGLParams::eParamFlt rParamID, double rValue ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		// Prevent infinite loops:
		if( !MeshGLParams::setParamFloatMeshGL( rParamID, rValue ) ) {
				return false;
		}
		switch( rParamID ) {
				case DATUM_SPHERE_TRANS:
				case WAVES_COLMAP_LEN:
						break;
				case TEXMAP_QUANTIL_MIN:
						setParamFloatMeshGL( TEXMAP_QUANTIL_MIN_ABSOLUT, _NOT_A_NUMBER_DBL_ );
						break;
				case TEXMAP_QUANTIL_MAX:
						setParamFloatMeshGL( TEXMAP_QUANTIL_MAX_ABSOLUT, _NOT_A_NUMBER_DBL_ );
						break;
				case TEXMAP_QUANTIL_MIN_ABSOLUT:
				case TEXMAP_QUANTIL_MAX_ABSOLUT:
				case TEXMAP_FIXED_MIN:
				case TEXMAP_FIXED_MAX:
				case TEXMAP_AUTO_MIN:
				case TEXMAP_AUTO_MAX:
						break;
				case POLYLINE_NORMAL_SCALE:
						polyLinesChanged();
						break;
				case POLYLINE_WIDTH:
						break;
				case FUNC_VALUE_LOG_GAMMA:
                        //cout << "[MeshGLParams::" << __FUNCTION__ << "] : " << rValue << endl;
						break;
				case ISOLINES_DISTANCE: {
						double funcValMin;
						double funcValMax;
						getFuncValMinMaxUser( &funcValMin, &funcValMax );
						int numberOfIsoLines = floor( (funcValMax-funcValMin) / rValue );
                        cout << "[MeshGLParams::" << __FUNCTION__ << "] Number of Isolines: " << numberOfIsoLines << endl;
						} break;
				case ISOLINES_OFFSET: // Should be lower than ISOLINES_DISTANCE. However, nothing happens when not. => freedom of choice.
						break;
				case ISOLINES_PIXEL_WIDTH:
						break;
				case BOUNDING_BOX_LINEWIDTH:
						break;
				case NORMALS_LENGTH:
				case NORMALS_WIDTH:
						break;
				// Shader:
		case NPR_OUTLINE_WIDTH:
		case NPR_OUTLINE_THRESHOLD:
		case NPR_HATCH_ROTATION:
		case NPR_HATCH_SCALE:
		case NPR_SPECULAR_SIZE:
		case TRANSPARENCY_UNIFORM_ALPHA:
		case TRANSPARENCY_ALPHA2:
		case TRANSPARENCY_GAMMA:
		case BADLIT_LOWER_THRESHOLD:
		case BADLIT_UPPER_THRESHOLD:
		case PIN_SIZE:
		case PIN_LINE_HEIGHT:
		case POINTCLOUD_POINTSIZE:
			break;
				// Not defined = error:
				default:
                        cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Unknown parameter ID: " << rParamID << endl;
						break;
		}
		return true;
}



// Vertex Buffer Objects ---------------------------------------------------------------------------------------------------------

//! Prepate VBO for a simple 1x1x1 box centered at the origin.
void MeshGL::vboPrepareBox() {
		// VBUFF_CUBE - fill with unit box
		mVertBufObjs[VBUFF_CUBE]             = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
		grVertexElmentBasic boxVertices[] = {
				{ { -0.5, -0.5, -0.5 }, // A-z
				  { 0.0, 0.0, -1.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, -0.5 }, // B-z
				  { 0.0, 0.0, -1.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, -0.5 }, // C-z
				  { 0.0, 0.0, -1.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, -0.5 }, // C-z
				  { 0.0, 0.0, -1.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, -0.5 }, // B-z
				  { 0.0, 0.0, -1.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, -0.5 }, // D-z
				  { 0.0, 0.0, -1.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 }, // ----------------------------------------------
				{ { -0.5, +0.5, +0.5 }, // B+z
				  { 0.0, 0.0, +1.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, -0.5, +0.5 }, // A+z
				  { 0.0, 0.0, +1.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, +0.5 }, // C+z
				  { 0.0, 0.0, +1.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, +0.5 }, // B+z
				  { 0.0, 0.0, +1.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, +0.5 }, // C+z
				  { 0.0, 0.0, +1.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, +0.5 }, // D+z
				  { 0.0, 0.0, +1.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 }, // ----------------------------------------------
				{ { -0.5, +0.5,-0.5 }, // A+y
				  { 0.0, +1.0, 0.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, +0.5 }, // B+y
				  { 0.0, +1.0, 0.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, -0.5 }, // C+y
				  { 0.0, +1.0, 0.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, -0.5 }, // C+y
				  { 0.0, +1.0, 0.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, +0.5 }, // B+y
				  { 0.0, +1.0, 0.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, +0.5 }, // D+y
				  { 0.0, +1.0, 0.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 }, // ----------------------------------------------
				{ { -0.5, -0.5, +0.5 }, // B-y
				  { 0.0, -1.0, 0.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, -0.5, -0.5 }, // A-y
				  { 0.0, -1.0, 0.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, -0.5 }, // C-y
				  { 0.0, -1.0, 0.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, -0.5, +0.5 }, // B-y
				  { 0.0, -1.0, 0.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, -0.5 }, // C-y
				  { 0.0, -1.0, 0.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, +0.5 }, // D-y
				  { 0.0, -1.0, 0.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 }, // ----------------------------------------------
				{ { -0.5,-0.5, -0.5 }, // A-x
				  { -1.0, 0.0, 0.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { -0.5,-0.5, +0.5 }, // B-x
				  { -1.0, 0.0, 0.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, -0.5 }, // C-x
				  { -1.0, 0.0, 0.0 },
				  { 0xFF, 0x0, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, -0.5 }, // C-x
				  { -1.0, 0.0, 0.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, -0.5, +0.5 }, // B-x
				  { -1.0, 0.0, 0.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { -0.5, +0.5, +0.5 }, // D-x
				  { -1.0, 0.0, 0.0 },
				  { 0x00, 0xFF, 0x0, 0xFF },
				  0.0 }, // ----------------------------------------------
				{ { +0.5, -0.5, +0.5 }, // B+x
				  { +1.0, 0.0, 0.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, -0.5 }, // A+x
				  { +1.0, 0.0, 0.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, -0.5 }, // C+x
				  { +1.0, 0.0, 0.0 },
				  { 0xFF, 0xAA, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, -0.5, +0.5 }, // B+x
				  { +1.0, 0.0, 0.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, -0.5 }, // C+x
				  { +1.0, 0.0, 0.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 },
				{ { +0.5, +0.5, +0.5 }, // D+x
				  { +1.0, 0.0, 0.0 },
				  { 0xAA, 0xFF, 0x0, 0xFF },
				  0.0 }, // ----------------------------------------------
		};

		// Vertex data -- 2x6 triangles with 36 points
		vboAddBuffer( sizeof(grVertexElmentBasic)*36, boxVertices, QOpenGLBuffer::StaticDraw, VBUFF_CUBE, __FUNCTION__ );
}

void MeshGL::vboPrepareQuad()
{
	mVertBufObjs[VBUFF_SCREEN_QUAD]             = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );

	GLfloat vertices[] =  { -1.0, -1.0,
			+1.0, -1.0,
			+1.0, +1.0,
			-1.0, +1.0
		  };

	vboAddBuffer( 4*2*sizeof(GLfloat), vertices, QOpenGLBuffer::StaticDraw, VBUFF_SCREEN_QUAD , __FUNCTION__ );
}

//! Prepare a simple cylinder of height 1 and diameter 1 (1x1x1) centered at the origin. The axis is aligned to Z.
void MeshGL::vboPrepareCylinder() {
	// VBUFF_CYLINDER fill with unit cylinder
	//----------------------------------------------------------------------------------------------------------------------
	mVertBufObjs[VBUFF_CYLINDER] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
	constexpr unsigned int cylSegments = 32;
	grVertexElmentBasic cylinderVertices[cylSegments*6*2];
	for(grVertexElmentBasic& cylinderVertex : cylinderVertices) {
		cylinderVertex.mColor[0] = 255;
		cylinderVertex.mColor[1] =   0;
		cylinderVertex.mColor[2] =   0;
		cylinderVertex.mColor[3] =  64;
	}
	// Outer Cylinder
	for( unsigned int i=0; i<cylSegments; i++ ) {
		double sinAlphaA = sin( static_cast<double>(i)/static_cast<double>(cylSegments)     * 2*M_PI )*0.5;
		double sinAlphaB = sin( static_cast<double>(i+1)/static_cast<double>(cylSegments) * 2*M_PI )*0.5;
		double cosAlphaA = cos( static_cast<double>(i)/static_cast<double>(cylSegments)     * 2*M_PI )*0.5;
		double cosAlphaB = cos( static_cast<double>(i+1)/static_cast<double>(cylSegments) * 2*M_PI )*0.5;
		cylinderVertices[i*6].mPosition[0]   = static_cast<GLfloat>(sinAlphaA);
		cylinderVertices[i*6].mPosition[1]   = static_cast<GLfloat>(cosAlphaA);
		cylinderVertices[i*6].mPosition[2]   = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6].mNormal[0]     = static_cast<GLfloat>(sinAlphaA)*2.0;
		cylinderVertices[i*6].mNormal[1]     = static_cast<GLfloat>(cosAlphaA)*2.0;
		cylinderVertices[i*6].mNormal[2]     = static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6].mFuncVal       = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+1].mPosition[0] = static_cast<GLfloat>(sinAlphaA);
		cylinderVertices[i*6+1].mPosition[1] = static_cast<GLfloat>(cosAlphaA);
		cylinderVertices[i*6+1].mPosition[2] = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+1].mNormal[0]   = static_cast<GLfloat>(sinAlphaA)*2.0;
		cylinderVertices[i*6+1].mNormal[1]   = static_cast<GLfloat>(cosAlphaA)*2.0;
		cylinderVertices[i*6+1].mNormal[2]   = static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+1].mFuncVal     = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+2].mPosition[0] = static_cast<GLfloat>(sinAlphaB);
		cylinderVertices[i*6+2].mPosition[1] = static_cast<GLfloat>(cosAlphaB);
		cylinderVertices[i*6+2].mPosition[2] = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+2].mNormal[0]   = static_cast<GLfloat>(sinAlphaB)*2.0;
		cylinderVertices[i*6+2].mNormal[1]   = static_cast<GLfloat>(cosAlphaB)*2.0;
		cylinderVertices[i*6+2].mNormal[2]   = static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+2].mFuncVal     = static_cast<GLfloat>(-0.5f);

		cylinderVertices[i*6+3].mPosition[0] = static_cast<GLfloat>(sinAlphaB);
		cylinderVertices[i*6+3].mPosition[1] = static_cast<GLfloat>(cosAlphaB);
		cylinderVertices[i*6+3].mPosition[2] = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+3].mNormal[0]   = static_cast<GLfloat>(sinAlphaB)*2.0;
		cylinderVertices[i*6+3].mNormal[1]   = static_cast<GLfloat>(cosAlphaB)*2.0;
		cylinderVertices[i*6+3].mNormal[2]   = static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+3].mFuncVal     = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+4].mPosition[0] = static_cast<GLfloat>(sinAlphaA);
		cylinderVertices[i*6+4].mPosition[1] = static_cast<GLfloat>(cosAlphaA);
		cylinderVertices[i*6+4].mPosition[2] = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+4].mNormal[0]   = static_cast<GLfloat>(sinAlphaA)*2.0;
		cylinderVertices[i*6+4].mNormal[1]   = static_cast<GLfloat>(cosAlphaA)*2.0;
		cylinderVertices[i*6+4].mNormal[2]   = static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+4].mFuncVal     = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+5].mPosition[0] = static_cast<GLfloat>(sinAlphaB);
		cylinderVertices[i*6+5].mPosition[1] = static_cast<GLfloat>(cosAlphaB);
		cylinderVertices[i*6+5].mPosition[2] = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+5].mNormal[0]   = static_cast<GLfloat>(sinAlphaB)*2.0;
		cylinderVertices[i*6+5].mNormal[1]   = static_cast<GLfloat>(cosAlphaB)*2.0;
		cylinderVertices[i*6+5].mNormal[2]   = static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+5].mFuncVal     = static_cast<GLfloat>(+0.5f);
	}
	// Inner Cylinder i.e. "Backfaces"
	for( unsigned int i=cylSegments; i<2*cylSegments; i++ ) {
		double sinAlphaA = sin( static_cast<double>(i)/static_cast<double>(cylSegments)     * 2*M_PI )*0.5;
		double sinAlphaB = sin( static_cast<double>(i+1)/static_cast<double>(cylSegments) * 2*M_PI )*0.5;
		double cosAlphaA = cos( static_cast<double>(i)/static_cast<double>(cylSegments)     * 2*M_PI )*0.5;
		double cosAlphaB = cos( static_cast<double>(i+1)/static_cast<double>(cylSegments) * 2*M_PI )*0.5;
		cylinderVertices[i*6].mPosition[0]   = static_cast<GLfloat>(sinAlphaA);
		cylinderVertices[i*6].mPosition[1]   = static_cast<GLfloat>(cosAlphaA);
		cylinderVertices[i*6].mPosition[2]   = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6].mNormal[0]     = -static_cast<GLfloat>(sinAlphaA)*2.0;
		cylinderVertices[i*6].mNormal[1]     = -static_cast<GLfloat>(cosAlphaA)*2.0;
		cylinderVertices[i*6].mNormal[2]     = -static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6].mFuncVal       = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+2].mPosition[0] = static_cast<GLfloat>(sinAlphaA);
		cylinderVertices[i*6+2].mPosition[1] = static_cast<GLfloat>(cosAlphaA);
		cylinderVertices[i*6+2].mPosition[2] = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+2].mNormal[0]   = -static_cast<GLfloat>(sinAlphaA)*2.0;
		cylinderVertices[i*6+2].mNormal[1]   = -static_cast<GLfloat>(cosAlphaA)*2.0;
		cylinderVertices[i*6+2].mNormal[2]   = -static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+2].mFuncVal     = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+1].mPosition[0] = static_cast<GLfloat>(sinAlphaB);
		cylinderVertices[i*6+1].mPosition[1] = static_cast<GLfloat>(cosAlphaB);
		cylinderVertices[i*6+1].mPosition[2] = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+1].mNormal[0]   = -static_cast<GLfloat>(sinAlphaB)*2.0;
		cylinderVertices[i*6+1].mNormal[1]   = -static_cast<GLfloat>(cosAlphaB)*2.0;
		cylinderVertices[i*6+1].mNormal[2]   = -static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+1].mFuncVal     = static_cast<GLfloat>(-0.5f);

		cylinderVertices[i*6+3].mPosition[0] = static_cast<GLfloat>(sinAlphaB);
		cylinderVertices[i*6+3].mPosition[1] = static_cast<GLfloat>(cosAlphaB);
		cylinderVertices[i*6+3].mPosition[2] = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+3].mNormal[0]   = -static_cast<GLfloat>(sinAlphaB)*2.0;
		cylinderVertices[i*6+3].mNormal[1]   = -static_cast<GLfloat>(cosAlphaB)*2.0;
		cylinderVertices[i*6+3].mNormal[2]   = -static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+3].mFuncVal     = static_cast<GLfloat>(-0.5f);
		cylinderVertices[i*6+5].mPosition[0] = static_cast<GLfloat>(sinAlphaA);
		cylinderVertices[i*6+5].mPosition[1] = static_cast<GLfloat>(cosAlphaA);
		cylinderVertices[i*6+5].mPosition[2] = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+5].mNormal[0]   = -static_cast<GLfloat>(sinAlphaA)*2.0;
		cylinderVertices[i*6+5].mNormal[1]   = -static_cast<GLfloat>(cosAlphaA)*2.0;
		cylinderVertices[i*6+5].mNormal[2]   = -static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+5].mFuncVal     = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+4].mPosition[0] = static_cast<GLfloat>(sinAlphaB);
		cylinderVertices[i*6+4].mPosition[1] = static_cast<GLfloat>(cosAlphaB);
		cylinderVertices[i*6+4].mPosition[2] = static_cast<GLfloat>(+0.5f);
		cylinderVertices[i*6+4].mNormal[0]   = -static_cast<GLfloat>(sinAlphaB)*2.0;
		cylinderVertices[i*6+4].mNormal[1]   = -static_cast<GLfloat>(cosAlphaB)*2.0;
		cylinderVertices[i*6+4].mNormal[2]   = -static_cast<GLfloat>(0.0f);
		cylinderVertices[i*6+4].mFuncVal     = static_cast<GLfloat>(+0.5f);
	}
	vboAddBuffer( sizeof(grVertexElmentBasic)*cylSegments*6*2, cylinderVertices, QOpenGLBuffer::StaticDraw, VBUFF_CYLINDER, __FUNCTION__ );
}

//! Prepare a simple unit sphere with diameter 1 (1x1x1) centered at the origin.
//! The axis is aligend to Z. Yes this sphere has an axis to e.g. determine the poles or a meridian.
void MeshGL::vboPrepareSphere() {
		// --- Unit Sphere -------------------------------------------------------------------------------------------------------------------------------------
		// Inspired by: http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
		vector<VertexOfFace*> sphereVertices;
		vector<Face*>         sphereFaces;

		// create 12 vertices of a icosahedron
		double t = ( 1.0 + sqrt(5.0) ) / 2.0;
		unsigned int indexVert = 0;
		sphereVertices.push_back( new VertexOfFace( indexVert++, -1.0,  t,  0.0 ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++,  1.0,  t,  0.0 ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++, -1.0, -t,  0.0 ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++,  1.0, -t,  0.0 ) );

		sphereVertices.push_back( new VertexOfFace( indexVert++, 0.0, -1.0,  t ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++, 0.0,  1.0,  t ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++, 0.0, -1.0, -t ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++, 0.0,  1.0, -t ) );

		sphereVertices.push_back( new VertexOfFace( indexVert++,  t,  0.0, -1.0 ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++,  t,  0.0,  1.0 ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++, -t,  0.0, -1.0 ) );
		sphereVertices.push_back( new VertexOfFace( indexVert++, -t,  0.0,  1.0 ) );

		// create 20 triangles of the icosahedron
		unsigned int index = 0;
		// 5 faces around point 0
		sphereFaces.push_back( new Face( index++, sphereVertices[0], sphereVertices[11], sphereVertices[5]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[0], sphereVertices[5],  sphereVertices[1]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[0], sphereVertices[1],  sphereVertices[7]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[0], sphereVertices[7],  sphereVertices[10] ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[0], sphereVertices[10], sphereVertices[11] ) );
		// 5 adjacent faces
		sphereFaces.push_back( new Face( index++, sphereVertices[1],  sphereVertices[5],  sphereVertices[9]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[5],  sphereVertices[11], sphereVertices[4]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[11], sphereVertices[10], sphereVertices[2]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[10], sphereVertices[7],  sphereVertices[6]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[7],  sphereVertices[1],  sphereVertices[8]  ) );
		// 5 faces around point 3
		sphereFaces.push_back( new Face( index++, sphereVertices[3],  sphereVertices[9],  sphereVertices[4]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[3],  sphereVertices[4],  sphereVertices[2]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[3],  sphereVertices[2],  sphereVertices[6]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[3],  sphereVertices[6],  sphereVertices[8]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[3],  sphereVertices[8],  sphereVertices[9]  ) );
		// 5 adjacent faces
		sphereFaces.push_back( new Face( index++, sphereVertices[4],  sphereVertices[9],  sphereVertices[5]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[2],  sphereVertices[4],  sphereVertices[11] ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[6],  sphereVertices[2],  sphereVertices[10] ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[8],  sphereVertices[6],  sphereVertices[7]  ) );
		sphereFaces.push_back( new Face( index++, sphereVertices[9],  sphereVertices[8],  sphereVertices[1]  ) );

		// Interpolation of each face into four new faces:
		for( unsigned int i=0; i<3; i++ ) {
				index = 0;
				vector<Face*> sphereFacesInterpolated;
				for( auto const& currFace: sphereFaces ) {
						// Vertex A-B
						currFace->getMidPoint( Face::EDGE_AB, &sphereVertices );
						VertexOfFace* vertAB = sphereVertices.back();
						vertAB->setIndex( indexVert++ );
						// Vertex B-C
						currFace->getMidPoint( Face::EDGE_BC, &sphereVertices );
						VertexOfFace* vertBC = sphereVertices.back();
						vertBC->setIndex( indexVert++ );
						// Vertex C-A
						currFace->getMidPoint( Face::EDGE_CA, &sphereVertices );
						VertexOfFace* vertCA = sphereVertices.back();
						vertCA->setIndex( indexVert++ );
						// New faces:
						sphereFacesInterpolated.push_back( new Face( index++, currFace->getVertAoF(), vertAB, vertCA) );
						sphereFacesInterpolated.push_back( new Face( index++, currFace->getVertBoF(), vertBC, vertAB) );
						sphereFacesInterpolated.push_back( new Face( index++, currFace->getVertCoF(), vertCA, vertBC) );
						sphereFacesInterpolated.push_back( new Face( index++, vertAB, vertBC, vertCA) );
				}
				
				for(Face*& facePtr : sphereFaces)
				{
					delete facePtr;
				}
				
				sphereFaces.clear();
				sphereFaces.swap( sphereFacesInterpolated );
		}

		// Convert to buffer arrays
		vector<grVertexElmentBasic> vertexElements;
		for( auto const& currVertex: sphereVertices ) {
				// Move position and normal to those of a sphere:
				Vector3D pos;
				pos = currVertex->getPositionVector();
				pos.setLength3( 0.5 ); // Diameter = 1
				currVertex->setPosition( pos.getX(), pos.getY(), pos.getZ() );
				pos.setLength3( 1.0 );
				currVertex->setNormal( pos.getX(), pos.getY(), pos.getZ() );
				// Now copy to an array for buffering:
				grVertexElmentBasic currElement;
				currVertex->copyCoordsTo( currElement.mPosition );
				// The normal vector is equal to the position vector as the sphere is centered at the origin.
				currElement.mNormal[0] = currElement.mPosition[0];
				currElement.mNormal[1] = currElement.mPosition[1];
				currElement.mNormal[2] = currElement.mPosition[2];
				currElement.mFuncVal = pos.getZ();
				vertexElements.push_back( currElement );
		}
		// Inner sphere:
		for( auto const& currVertex: sphereVertices ) {
				// Move position and normal to those of a sphere:
				Vector3D pos;
				pos = currVertex->getPositionVector();
				pos.setLength3( 0.5 ); // Diameter = 1
				currVertex->setPosition( pos.getX(), pos.getY(), pos.getZ() );
				pos.setLength3( 1.0 );
				currVertex->setNormal( pos.getX(), pos.getY(), pos.getZ() );
				// Now copy to an array for buffering:
				grVertexElmentBasic currElement;
				currVertex->copyCoordsTo( currElement.mPosition );
				// The normal vector is equal to the position vector as the sphere is centered at the origin.
				currElement.mNormal[0] = -currElement.mPosition[0];
				currElement.mNormal[1] = -currElement.mPosition[1];
				currElement.mNormal[2] = -currElement.mPosition[2];
				currElement.mFuncVal = pos.getZ();
				vertexElements.push_back( currElement );
		}
		vector<GLuint> vertexIndices;
		for( auto const& currFace: sphereFaces ) {
				vertexIndices.push_back( static_cast<GLuint>(currFace->getVertAIndex()) );
				vertexIndices.push_back( static_cast<GLuint>(currFace->getVertBIndex()) );
				vertexIndices.push_back( static_cast<GLuint>(currFace->getVertCIndex()) );
		}
		// Inner sphere:
		for( auto const& currFace: sphereFaces ) {
				vertexIndices.push_back( static_cast<GLuint>(currFace->getVertAIndex()) );
				vertexIndices.push_back( static_cast<GLuint>(currFace->getVertCIndex()) );
				vertexIndices.push_back( static_cast<GLuint>(currFace->getVertBIndex()) );
		}
		// Create buffers
		mVertBufObjs[VBUFF_SPHERE_VERTS] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
		mVertBufObjs[VBUFF_SPHERE_FACES] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );
		// Add buffers
		vboAddBuffer( sizeof(grVertexElmentBasic)*vertexElements.size(), vertexElements.data(), QOpenGLBuffer::StaticDraw, VBUFF_SPHERE_VERTS, __FUNCTION__ );
		vboAddBuffer( sizeof(GLuint)*vertexIndices.size(),               vertexIndices.data(),  QOpenGLBuffer::StaticDraw, VBUFF_SPHERE_FACES, __FUNCTION__ );
		// Clear/remove temporary elements
		
		for(Face*& facePtr : sphereFaces)
		{
			delete facePtr;
		}
		
		sphereFaces.clear();
		
		for(VertexOfFace*& vertexOfFacePtr : sphereVertices)
		{
			delete vertexOfFacePtr;
		}
		
		sphereVertices.clear();
		
		vertexElements.clear();
		vertexIndices.clear();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Prepares the VBO arrays for singular vertices.
//! Reason: time consuming
//! Returns false, when no buffer was prepared.
bool MeshGL::vboPrepareDoubleCone() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		bool drawVerticesSingular;
		getParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SINGULAR, &drawVerticesSingular );
		if( !drawVerticesSingular ) {
                // flag not set - so there is nothing to do.
				return false;
		}

		if( mVertBufObjs[VBUFF_VERTICES_SINGULAR] == nullptr ) {
				mVertBufObjs[VBUFF_VERTICES_SINGULAR] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );
		}

		if( mVertBufObjs[VBUFF_VERTICES_SINGULAR]->isCreated() ) {
				// Already prepared => nothing to do.
				return true;
		}

		int timeStartSub = clock(); // for performance mesurement
		vector<int> vertexDoubleConeTmp;
		Vertex* curVertex;
		for( uint64_t vertexCtr=0; vertexCtr<getVertexNr(); vertexCtr++ ) {
				curVertex = getVertexPos( vertexCtr );
				if( curVertex->isDoubleCone() ) {
						vertexDoubleConeTmp.push_back( vertexCtr );
				}
		}
		unsigned int vboSize = vertexDoubleConeTmp.size();
		GLuint* vertexDoubleCone =  new GLuint[vboSize];
		int vertexCtr = 0;
		vector<int>::iterator itInt;
		for ( itInt=vertexDoubleConeTmp.begin(); itInt != vertexDoubleConeTmp.end(); itInt++ ) {
				vertexDoubleCone[vertexCtr] = (*itInt);
				vertexCtr++;
		}

		// --- Vertex references Double Cones --------------------------------------
		vboAddBuffer( sizeof(GLuint)*vboSize, vertexDoubleCone, QOpenGLBuffer::StaticDraw, VBUFF_VERTICES_SINGULAR, __FUNCTION__ );
		delete[] vertexDoubleCone;

        cout << "[MeshGL::" << __FUNCTION__ << "] Time: " << static_cast<float>( clock() - timeStartSub ) / CLOCKS_PER_SEC << " seconds."  << endl;
		return true;
}

//! Prepares a VBO array with vertices having specific flags set.
//! Reason: time consuming
//! Returns false, when no buffer was prepared.
bool MeshGL::vboPrepareVerticesWithFlag( unsigned int rFlagNr,   //!< See Primitive::BitFlagArray
										 eVertBufObjs rBufferID  //!< ID of a Vertex Buffer Object
									   ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		if( mVertBufObjs[rBufferID] == nullptr ) {
				mVertBufObjs[rBufferID] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );
		}

		if( mVertBufObjs[rBufferID]->isCreated() ) {
				// Already prepared => nothing to do.
				return true;
		}

		int timeStartSub = clock(); // for performance mesurement
		vector<int> vertexListTmp;
		unsigned int vertexCtr = 0;
		for( auto const& currVertex: (*getPrimitiveListVertices()) ) { // C++11 style
				if( currVertex->getFlag( rFlagNr ) ) {
						vertexListTmp.push_back( vertexCtr );
				}
				vertexCtr++;
		}
		unsigned int vboSize = vertexListTmp.size();
		GLuint* vertexIndices = new GLuint[vboSize];
		vertexCtr = 0;
		vector<int>::iterator itInt;
		for ( itInt=vertexListTmp.begin(); itInt != vertexListTmp.end(); itInt++ ) {
				vertexIndices[vertexCtr] = (*itInt);
				vertexCtr++;
		}

		// --- Vertex references Double Cones --------------------------------------
		vboAddBuffer( sizeof(GLuint)*vboSize, vertexIndices, QOpenGLBuffer::StaticDraw, rBufferID, __FUNCTION__ );
		delete[] vertexIndices;

        cout << "[MeshGL::" << __FUNCTION__ << "] Time: " << static_cast<float>( clock() - timeStartSub ) / CLOCKS_PER_SEC << " seconds."  << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] Elements: " << vboSize << endl;
		return true;
}

//! Prepares a VBO array with vertices having specific flags set.
//! Reason: time consuming
//! Returns false, when no buffer was prepared.
bool MeshGL::vboPrepareFacesWithFlag( unsigned int rFlagNr,   //!< See Primitive::BitFlagArray
									  eVertBufObjs rBufferID  //!< ID of a Vertex Buffer Object
									) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		if( mVertBufObjs[rBufferID] == nullptr ) {
				mVertBufObjs[rBufferID] = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );
		}

		if( mVertBufObjs[rBufferID]->isCreated() ) {
				// Already prepared => nothing to do.
				return true;
		}

		int timeStartSub = clock(); // for performance mesurement
		vector<int> faceListTmp;
		for( auto const& currFace: (*getPrimitiveListFaces()) ) { // C++11 style
				if( currFace->getFlag( rFlagNr ) ) {
						faceListTmp.push_back( currFace->getVertA()->getIndex() );
						faceListTmp.push_back( currFace->getVertB()->getIndex() );
						faceListTmp.push_back( currFace->getVertC()->getIndex() );
				}
		}
		unsigned int vboSize = faceListTmp.size();
		GLuint* vertexIndices = new GLuint[vboSize];
		unsigned int faceCtr = 0;
		vector<int>::iterator itInt;
		for ( itInt=faceListTmp.begin(); itInt != faceListTmp.end(); itInt++ ) {
				vertexIndices[faceCtr] = (*itInt);
				faceCtr++;
		}

		// --- Vertex references Double Cones --------------------------------------
		vboAddBuffer( sizeof(GLuint)*vboSize, vertexIndices, QOpenGLBuffer::StaticDraw, rBufferID, __FUNCTION__ );
		delete[] vertexIndices;

        cout << "[MeshGL::" << __FUNCTION__ << "] Time: " << static_cast<float>( clock() - timeStartSub ) / CLOCKS_PER_SEC << " seconds."  << endl;
        cout << "[MeshGL::" << __FUNCTION__ << "] Elements: " << vboSize << endl;
		return true;
}

//! Prepare OpenGL VBOs for polygonal lines.
bool MeshGL::vboPreparePolylines() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		PRINT_OPENGL_ERROR( "OLD_ERROR" );

		if( mPolyLines.size() <= 0 ) {
				return false;
		}

		//------------------------------------------------------------------------------------------------------------------------------------------------------
		// NEW VBuff
		if( ( mVertBufObjs[VBUFF_VERTICES_POLYLINES] != nullptr )   &&
			( mVertBufObjs[VBUFF_POLYLINES] != nullptr )            &&
			( mVertBufObjs[VBUFF_VERTICES_POLYLINES]->isCreated() ) &&
			( mVertBufObjs[VBUFF_POLYLINES]->isCreated() )
		  ) {
				// Buffers already created.
				return false;
		}

		bool noError = true;

		vboRemoveBuffer( VBUFF_VERTICES_POLYLINES, __FUNCTION__ );
		vboRemoveBuffer( VBUFF_POLYLINES, __FUNCTION__ );
		mVertBufObjs[VBUFF_VERTICES_POLYLINES] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
		mVertBufObjs[VBUFF_POLYLINES]          = new QOpenGLBuffer( QOpenGLBuffer::IndexBuffer );

		unsigned int vboSize    = 0;
		for( auto const& currPoly: mPolyLines ) {
                vboSize    += currPoly->length();
        }
		unsigned int vboSizeIdx = (vboSize-mPolyLines.size())*2;

		// Vertex data
		vector<grVertexStripeElment> vertexBufferData ( vboSize );
		// Indices of vertex pairs
		vector<GLuint> vertexIndices;
		vertexIndices.reserve( vboSizeIdx );
		// Counter
		unsigned int vertexCtr = 0;
		for( auto const& currPoly: mPolyLines ) {
				unsigned int polyLength = currPoly->length();
				for( unsigned int i=0; i<polyLength; i++ ) {
						Vertex* currVertex = currPoly->getVertexRef( i );
						if( !vboPrepareVerticesStripedFetchVertex( currVertex, &(vertexBufferData[vertexCtr]) ) ) {
								noError = false;
						}
						if( i>0 ) { // Prevent connecting two different polylines.
								vertexIndices.push_back( vertexCtr-1 );   // previous index
								vertexIndices.push_back( vertexCtr );     // current index
						}
						vertexCtr++;
				}
        }
        vboAddBuffer( sizeof(grVertexStripeElment)*vboSize, vertexBufferData.data(), QOpenGLBuffer::StaticDraw, VBUFF_VERTICES_POLYLINES, __FUNCTION__ );
		vboAddBuffer( sizeof(GLuint)*vboSizeIdx,            vertexIndices.data(),    QOpenGLBuffer::StaticDraw, VBUFF_POLYLINES,          __FUNCTION__ );
		return noError;
}

//! Prepare Vertices and related data as VBO stripe, i.e. position, color, normal.
//! See also MeshGL::grVertexStripeElment
//! @returns false in case of an error.
bool MeshGL::vboPrepareVerticesStriped() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		bool noError = true;

		if( mVertBufObjs[VBUFF_VERTICES_STRIPED] == nullptr ) {
				mVertBufObjs[VBUFF_VERTICES_STRIPED] = new QOpenGLBuffer( QOpenGLBuffer::VertexBuffer );
		}

		if( mVertBufObjs[VBUFF_VERTICES_STRIPED]->isCreated() ) {
				// Already prepared => nothing to do.
				return true;
		}
        cout << "[MeshGL::" << __FUNCTION__ << "] sizeof( grVertexStripeElment ): " << sizeof( grVertexStripeElment ) << " bytes." << endl;

		// Prepare data for the buffer
		int timeStart = clock(); // for performance mesurement

		unsigned int vboSize = getVertexNr();
		vector<grVertexStripeElment> bufferData;
		bufferData.reserve( vboSize );

		for( unsigned int vertexCtr=0; vertexCtr<vboSize; vertexCtr++ ) {
				Vertex* currVertex = getVertexPos( vertexCtr );
				grVertexStripeElment currElement;
				if( !vboPrepareVerticesStripedFetchVertex( currVertex, &currElement ) ) {
						noError = false;
				}
				bufferData.push_back( currElement );
		}

		vboAddBuffer( sizeof(grVertexStripeElment)*bufferData.size(), bufferData.data(), QOpenGLBuffer::StaticDraw, VBUFF_VERTICES_STRIPED, __FUNCTION__ );
        cout << "[MeshGL::" << __FUNCTION__ << "] Time Vertices: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

		return noError;
}

//! Fetch one vertex into a strided array.
bool MeshGL::vboPrepareVerticesStripedFetchVertex( Vertex* rVertex, grVertexStripeElment* rWriteTo ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		bool noError = true;

		if( !rVertex->copyCoordsTo( rWriteTo->mPosition ) ) {
				noError = false;
		}
		if( !rVertex->copyNormalXYZTo( rWriteTo->mNormal ) ) {
				noError = false;
		}
		if( !rVertex->copyRGBATo( rWriteTo->mColor ) ) {
				noError = false;
		}
		double funcVal;
		if( !rVertex->getFuncValue( &funcVal ) ) {
				noError = false;
		}
		rWriteTo->mFuncVal = funcVal;  //! \todo change from GLfloat to GLdouble.
		uint64_t labelID;
        rVertex->getLabel( labelID ); // No ERROR check, here as getLabels also returns false, when no label is set!
		rWriteTo->mLabelID = static_cast<GLfloat>(labelID);  // This is INTENTIONAL - to determine faces along the border of a label!
		// Flags -- these should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
		rWriteTo->mFlags = 0.0f;
		if( rVertex->isBorder() ) {
				rWriteTo->mFlags += 1.0f;
		}
		if( rVertex->isNonManifold() ) {
				rWriteTo->mFlags += 2.0f;
		}
		if( rVertex->isDoubleCone() ) {
				rWriteTo->mFlags += 4.0f;
		}
		if( !rVertex->isLabled() ) {
				rWriteTo->mFlags += 8.0f;
		}

		return noError;
}

// VBO common --------------------------------------------------------------------------------------------------------------------------------------------------

//! Removes a specific Vertex Buffer Object.
//!
//! IMPORTANT: This should be the only function calling mVertBufObjs[rBufferID]->destroy() !!!
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
void MeshGL::vboRemoveBuffer( const eVertBufObjs rBufferID, const string& rCallingFunc ) {
#else
void MeshGL::vboRemoveBuffer( eVertBufObjs rBufferID, string rCallingFunc  ) {
#endif
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		if( mVertBufObjs[rBufferID] == nullptr ) {
                cout << "[MeshGL::" << __FUNCTION__ << "] Buffer " << rBufferID << " already removed - called by " << rCallingFunc << endl;
				return;
		}
		if( !mVertBufObjs[rBufferID]->isCreated() ) {
                cout << "[MeshGL::" << __FUNCTION__ << "] Buffer " << rBufferID << " exist, but is not created - called by " << rCallingFunc << endl;
				delete mVertBufObjs[rBufferID];
				mVertBufObjs[rBufferID] = nullptr;
				return;
		}
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
		mVertBufObjs[rBufferID]->bind();
		int bufferSizeDeleted = mVertBufObjs[rBufferID]->size();
		mVertBufObjs[rBufferID]->release();
        cout << "[MeshGL::" << __FUNCTION__ << "] will delete " << bufferSizeDeleted << " bytes (" << bufferSizeDeleted/(static_cast<double>(1024)*1024) << " MBytes) ";
        cout << "by " << rCallingFunc << endl;
		mVboMemoryUsage -= bufferSizeDeleted;
#endif
		delete mVertBufObjs[rBufferID];
		mVertBufObjs[rBufferID] = nullptr;
}

//! Adds a specific Vertex Buffer Object.
//!
//! IMPORTANT: Except, vboAddElementBuffer, this should be the only function calling glBufferData!!!
void MeshGL::vboAddBuffer( GLsizeiptr                   rTotalSize,   //!< Size of a all elements of the array.
						   GLvoid*                      rData,        //!< Pointer to the array holding the data to be mapped.
						   QOpenGLBuffer::UsagePattern  rUsage,       //!< e.g. GL_STATIC_DRAW - see glBufferData documentation.
						   eVertBufObjs                 rBufferID,    //!< Our internal buffer No - see enum vboArrays.
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
                           const string&                rCallingFunc  //!< Calling function - typically set to __FUNCTION__ - for debuging.
#else
                           string                       rCallingFunc  //!< Calling function - typically set to __FUNCTION__ - for debuging.
#endif
		) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
        cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
		GLint currentVAO;
		glGetIntegerv( GL_VERTEX_ARRAY_BINDING, &currentVAO );
        //cout << "[MeshGL::" << __FUNCTION__ << "] currentVAO: " << currentVAO << endl;

		PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>( mOpenGLContext->getProcAddress( "glBindVertexArray" ) );
		if( static_cast<GLuint>(currentVAO) != mVAO ) {
				// Bind buffer and fill with data
				glBindVertexArray( mVAO );
				PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );
		}

		// Vertex data: one quadtriangle
		mVertBufObjs[rBufferID]->create();
		mVertBufObjs[rBufferID]->setUsagePattern( rUsage );
		if( !mVertBufObjs[rBufferID]->bind() ) {
                cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Could not bind vertex buffer VBO_VERTICES_STRIPED to the context!" << endl;
		}
		mVertBufObjs[rBufferID]->allocate( rData, rTotalSize );
#ifdef OPENGL_VBO_SHOW_MEMORY_USAGE
		int bufferSizeAdded = mVertBufObjs[rBufferID]->size();
        cout << "[MeshGL::" << __FUNCTION__ << "] add " << bufferSizeAdded << " bytes - " << bufferSizeAdded/(1024*1024) << " MBytes by " << rCallingFunc << endl; \
		mVboMemoryUsage += bufferSizeAdded;
#endif
		mVertBufObjs[rBufferID]->release();

        // Bind previous buffer.
		glBindVertexArray( static_cast<GLuint>(currentVAO) );
		PRINT_OPENGL_ERROR( "glBindVertexArray( currentVAO )" );
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
