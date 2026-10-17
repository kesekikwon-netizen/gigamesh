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

#include <GigaMesh/mesh/plane.h>

// Sets default values - to be used by all contructors!
// ----------------------------------------------------
#define PLANEINITDEFAULTS                             \
	mVertA( nullptr ),                            \
	mVertB( nullptr ),                            \
	mVertC( nullptr ),                            \
	mHNF( Vector3D( _NOT_A_NUMBER_DBL_ ) ),       \
	mCog( Vector3D( _NOT_A_NUMBER_DBL_ ) ),       \
	mWasFlipped( false ),                         \
	mDefinitionType( Plane::PLANE_UNDEFINED )

using namespace std;

//! Constructor of a plane using three given points in 3D.
Plane::Plane( Vector3D rPosA, Vector3D rPosB, Vector3D rPosC )
    : PLANEINITDEFAULTS {
	setPositions( rPosA, rPosB, rPosC );
	mDefinitionType = THREE_POSITIONS;
}

//! Constructor of a plane using a given point and a normal.
Plane::Plane( Vector3D rPosA, Vector3D rNormal )
    : PLANEINITDEFAULTS {
	// Make Normal to HNF:
	rNormal.normalize3();
	double HNFd = dot3( -rPosA, rNormal );
	rNormal.setH( HNFd );
	setPlaneHNF( &rPosA, &rNormal );
	mDefinitionType = POSITION_AND_NORMAL;
}

//! Constructor using the Hesse Normal Form -- three arbritrary points defining the plane will be computed.
Plane::Plane( Vector3D* rPlaneHNF )
    : PLANEINITDEFAULTS {
	setPlaneHNF( rPlaneHNF );
}

//! Constructor using a Face
Plane::Plane( Face& f )
    :  PLANEINITDEFAULTS {
	Vector3D posA( f.getVertA()->getPositionVector() );
	Vector3D posB( f.getVertB()->getPositionVector() );
	Vector3D posC( f.getVertC()->getPositionVector() );
	setPositions( posA, posB, posC );
}

//! Constructor for cloning / copying a plane.
//!
//! Inspired by: http://www.cplusplus.com/articles/y8hv0pDG/
Plane::Plane( const Plane& rOther )
    : PLANEINITDEFAULTS {
	Vector3D vertPosA;
	Vector3D vertPosB;
	Vector3D vertPosC;
	if( rOther.getVertPositions( vertPosA, vertPosB, vertPosC ) ) {
		mVertA = new Vertex( vertPosA );
		mVertB = new Vertex( vertPosB );
		mVertC = new Vertex( vertPosC );
	}
	if( !rOther.getVertHNFCOG( mHNF, mCog ) ) {
		cerr << "[Plane::" << __FUNCTION__ << "] ERROR: getVertHNFCOG failed unexpectedly!" << endl;
	}
	mWasFlipped     = rOther.getFlipped();
	mDefinitionType = rOther.getDefinedBy();
}

//! Constructor of a plane equaling the xy-plane using the points (0,0,0), (0,1,0) and (1,0,0).
Plane::Plane()
    : PLANEINITDEFAULTS {
	Vector3D posA( 0.0, 0.0, 0.0, 1.0 );
	Vector3D posB( 0.0, 1.0, 0.0, 1.0 );
	Vector3D posC( 1.0, 0.0, 0.0, 1.0 );
	setPositions( posA, posB, posC );
}

//! Destructor for a Plane object.
Plane::~Plane() {
	if( mVertA != nullptr ) {
		delete mVertA;
	}
	if( mVertB != nullptr ) {
		delete mVertB;
	}
	if( mVertC != nullptr ) {
		delete mVertC;
	}
}

// --- information retrival ---------------------------------------------------

double Plane::getX() const {
	//! Returns the x-coordinate of the Plane's center of gravity.
	//! Return not-a-number in case of an error.
	return mCog.getX();
}

double Plane::getY() const {
	//! Returns the y-coordinate of the Plane's center of gravity.
	//! Return not-a-number in case of an error.
	return mCog.getY();
}

double Plane::getZ() const {
	//! Returns the z-coordinate of the Plane's center of gravity.
	//! Return not-a-number in case of an error.
	return mCog.getZ();
}

//! Returns X coordinate of plane normal
double Plane::getNormalX() {
	return(mHNF.getX());
}

//! Returns Y coordinate of plane normal
double Plane::getNormalY() {
	return(mHNF.getY());
}

//! Returns Z coordinate of plane normal
double Plane::getNormalZ() {
	return(mHNF.getZ());
}

//! HNF Paramater 1 of 4
//! @returns value of the first of four HNF paramters.
double Plane::getHNFA() const {
	return( mHNF.getX() );
}

//! HNF Paramater 2 of 4
//! @returns value of the first of four HNF paramters.
double Plane::getHNFB() const {
	return( mHNF.getY() );
}

//! HNF Paramater 3 of 4
//! @returns value of the first of four HNF paramters.
double Plane::getHNFC() const {
	return( mHNF.getY() );
}

//! HNF Paramater 4 of 4
//! @returns value of the first of four HNF paramters.
double Plane::getHNFD() const {
	return( mHNF.getH() );
}

//! Sets new data for the plane. This updates all relevant information.
void Plane::setPositions( const Vector3D& rPosA, const Vector3D& rPosB, const Vector3D& rPosC ) {
	delete( mVertA );
	delete( mVertB );
	delete( mVertC );

	mVertA = new Vertex( rPosA );
	mVertB = new Vertex( rPosB );
	mVertC = new Vertex( rPosC );

	updateHNFCOG();
}


Vector3D Plane::getHNF() const {
	return mHNF;
}

double Plane::calcNormalX() {
	//! Calculates the x-coordinate of the Primitive's normal.
	//! Return not-a-number in case of an error.

    double /*vBAx,*/ vBAy, vBAz; // Vector AB
    double /*vCAx,*/ vCAy, vCAz; // Vector CA

    //vBAx = mVertB->getX() - mVertA->getX();
	vBAy = mVertB->getY() - mVertA->getY();
	vBAz = mVertB->getZ() - mVertA->getZ();

    //vCAx = mVertC->getX() - mVertA->getX();
	vCAy = mVertC->getY() - mVertA->getY();
	vCAz = mVertC->getZ() - mVertA->getZ();

	// cross product using xyzzy - see: http://en.wikipedia.org/wiki/Cross_product#Mnemonic
	// divided by half => | NORMAL | = sqrt( x^2 + y^2 + z^2 ) == area
	double normalX = ( ( vBAy * vCAz ) - ( vBAz * vCAy ) );
	//double normalY = ( ( vBAz * vCAx ) - ( vBAx * vCAz ) );
	//double normalZ = ( ( vBAx * vCAy ) - ( vBAy * vCAx ) );
	return normalX;
}

double Plane::calcNormalY() {
	//! Calculates the y-coordinate of the Primitive's normal.
	//! Return not-a-number in case of an error.
    double vBAx, /*vBAy,*/ vBAz; // Vector AB
    double vCAx, /*vCAy,*/ vCAz; // Vector CA

	vBAx = mVertB->getX() - mVertA->getX();
    //vBAy = mVertB->getY() - mVertA->getY();
	vBAz = mVertB->getZ() - mVertA->getZ();

	vCAx = mVertC->getX() - mVertA->getX();
    //vCAy = mVertC->getY() - mVertA->getY();
	vCAz = mVertC->getZ() - mVertA->getZ();

	// cross product using xyzzy - see: http://en.wikipedia.org/wiki/Cross_product#Mnemonic
	// divided by half => | NORMAL | = sqrt( x^2 + y^2 + z^2 ) == area
	//double normalX = ( ( vBAy * vCAz ) - ( vBAz * vCAy ) );
	double normalY = ( ( vBAz * vCAx ) - ( vBAx * vCAz ) );
	//double normalZ = ( ( vBAx * vCAy ) - ( vBAy * vCAx ) );

	return normalY;
}

double Plane::calcNormalZ() {
	//! Calculates the z-coordinate of the Primitive's normal.
	//! Return not-a-number in case of an error.
    double vBAx, vBAy/*, vBAz*/; // Vector AB
    double vCAx, vCAy/*, vCAz*/; // Vector CA

	vBAx = mVertB->getX() - mVertA->getX();
	vBAy = mVertB->getY() - mVertA->getY();
    //vBAz = mVertB->getZ() - mVertA->getZ();

	vCAx = mVertC->getX() - mVertA->getX();
	vCAy = mVertC->getY() - mVertA->getY();
    //vCAz = mVertC->getZ() - mVertA->getZ();

	// cross product using xyzzy - see: http://en.wikipedia.org/wiki/Cross_product#Mnemonic
	// divided by half => | NORMAL | = sqrt( x^2 + y^2 + z^2 ) == area
	//double normalX = ( ( vBAy * vCAz ) - ( vBAz * vCAy ) );
	//double normalY = ( ( vBAz * vCAx ) - ( vBAx * vCAz ) );
	double normalZ = ( ( vBAx * vCAy ) - ( vBAy * vCAx ) );

	return normalZ;
}

//! Pre-compute HNF and COG from vertex coordinates.
//! Has to be called after each manipulation, like Plane::set
bool Plane::updateHNFCOG() {
	// Center of gravity
	mCog = ( mVertA->getPositionVector() +
	         mVertB->getPositionVector() +
	         mVertC->getPositionVector() ) / 3.0;

	// Set parameters for Hessian normal form
	mHNF.set( calcNormalX(), calcNormalY(), calcNormalZ() ); // normal vector should be initialized; will be used by getPlaneHNF
	getPlaneHNF( &this->mHNF );

	return true;
}

//! Return the type (=class) of this object as an id.
int Plane::getType() {
	return Primitive::IS_PLANE;
}

//! Check if the plane is properly defined.
//!
//! @returns false in case the plane was not properly defined.
bool Plane::isValid() {
	// The first three HNF paramters define the orientation.
	// So the length has to be a normal value (i.e. not zero!, not inf, etc.)
	if( !std::isnormal( mHNF.getLength3() ) ) {
		return( false );
	}
	// The fourth HNF paramters can be zero, but has to be finite.
	if( !std::isfinite( mHNF.getH() ) ) {
		return( false );
	}
	return( true );
}

//! Applies a transformation matrix, when all three vertices are defined.
bool Plane::applyTransfrom( Matrix4D* rTransMat ) {
	if( ( mVertA == nullptr ) || ( mVertB == nullptr ) || ( mVertC == nullptr ) ) {
		return false;
	}
	//! Will return false, when a NULL pointer is given.
	if( rTransMat == nullptr ) {
		return false;
	}
	//! Will return false in case of any other error.
	if( !mVertA->applyTransfrom( rTransMat ) ) {
		cerr << "[Plane::" << __FUNCTION__ << "] ERROR: applyTransfrom failed for vertex A!";
		return false;
	}
	if( !mVertB->applyTransfrom( rTransMat ) ) {
		cerr << "[Plane::" << __FUNCTION__ << "] ERROR: applyTransfrom failed for vertex B!";
		return false;
	}
	if( !mVertC->applyTransfrom( rTransMat ) ) {
		cerr << "[Plane::" << __FUNCTION__ << "] ERROR: applyTransfrom failed for vertex C!";
		return false;
	}
	return updateHNFCOG();
}

bool Plane::getVertCoords( double* rCoords ) const {
	//! Returns the coordinates of the vertices.
	//! rCoords has to be of size 9 (3x3).

	//! Will return false, when the plane is not properly defined.
	if( ( mVertA == nullptr ) || ( mVertB == nullptr ) || ( mVertC == nullptr ) ) {
		return false;
	}
	//! Will return false, when a NULL pointer is given.
	if( rCoords == nullptr ) {
		return false;
	}
	mVertA->copyXYZTo( &rCoords[0] );
	mVertB->copyXYZTo( &rCoords[3] );
	mVertC->copyXYZTo( &rCoords[6] );
	return true;
}

//! Retrieve the three (optional) co-planar vertex positions.
//! @returns false in case of an error or undefined positions. True otherwise.
bool Plane::getVertPositions( Vector3D& rPosA, Vector3D& rPosB, Vector3D& rPosC ) const {
	if( ( mVertA == nullptr ) || ( mVertB == nullptr ) || ( mVertC == nullptr ) ) {
		return( false );
	}
	bool retVal = true;
	retVal |= mVertA->getPositionVector( &rPosA );
	retVal |= mVertB->getPositionVector( &rPosB );
	retVal |= mVertC->getPositionVector( &rPosC );
	return( retVal );
}

//! Retrieve the Hesse normal form (HNF) and the Center of Gravity (COG).
//! @returns false in case of an error or undefined positions. True otherwise.
bool Plane::getVertHNFCOG( Vector3D& rPosHNF, Vector3D& rPosCOG ) const {
	bool retVal = true;
	rPosHNF.set( mHNF ); // retVal |=
	rPosCOG.set( mCog ); // retVal |=
	return( retVal );
}

//! Retrieve flipped flag.
//! @returns the current state of the flag.
bool Plane::getFlipped() const {
	return( mWasFlipped );
}

//! Retrieve the plane definition type.
Plane::ePlaneDefinedBy Plane::getDefinedBy() const {
	return( mDefinitionType );
}


// Operations -------------------------------------------------------------------------------------

//! Classifies a point with respect to the plane. Returns the distance of the point
//! from the plane.
double Plane::classifyPoint(Vertex* point) {
	double distance = dot3(mHNF, point->getPositionVector()) + mHNF.getH();
	return(distance);
}

//! Set one of the three vertices vertices with rPosIdx (0=A, 1=B and 2=C).
bool Plane::setData( ePlaneVerts rPosIdx, Vector3D* rPos ) {
	switch( rPosIdx ) {
		case Plane::PLANE_VERT_A:
			delete( mVertA );
			mVertA = new Vertex( rPos );
			break;
		case Plane::PLANE_VERT_B:
			delete( mVertB );
			mVertB = new Vertex( rPos );
			break;
		case Plane::PLANE_VERT_C:
			delete( mVertC );
			mVertC = new Vertex( rPos );
			mDefinitionType = THREE_POSITIONS;
			break;
		default:
			cerr << "[Plane::" << __FUNCTION__ << "] ERROR: Invalid mPlanePosIdx!" << endl;
			return false;
	}

	// Abort pre-computing of mCog and mHNF, vertices are not set.
	if( ( mVertA == nullptr ) || ( mVertB == nullptr ) || ( mVertC == nullptr ) ) {
		return true;
	}
	return updateHNFCOG();
}

//! Set HNF and pre-compute mVertA, mVertB and mVertC and COG from vertex coordinates.
//! Has to be called after each manipulation, like Plane::set
bool Plane::setPlaneHNF( Vector3D* rPlaneHNF ) {
	// Closest point to origin ("Fusspunkt"):
	Vector3D posA( 0.0, 0.0, 0.0, 1.0 );
	if( rPlaneHNF->getZ() != 0.0 ) {
		posA.setZ( -rPlaneHNF->getH() / rPlaneHNF->getZ() );
	} else {
		if( rPlaneHNF->getY() != 0.0 ) {
            posA.setY( -rPlaneHNF->getH() / rPlaneHNF->getY() );
		} else {
			posA.setX( -rPlaneHNF->getH() / rPlaneHNF->getX() );
		}
	}
	mDefinitionType = HESSE_NORMAL_FORM;
	//cout << "rPlaneHNF: " << (*rPlaneHNF) << endl;
	//cout << "posA:      " << posA << endl;
	// Two other positions within the plane:
	return setPlaneHNF( &posA, rPlaneHNF );
}

//! Set mPosA using the given rPosA and the compute a valid mPosB and mPosC.
bool Plane::setPlaneHNF( Vector3D* rPosA, Vector3D* rPlaneHNF ) {
	// Two other positions within the plane:
	Vector3D posB( 0.0, 0.0, 0.0, 1.0 );
	Vector3D posC( 0.0, 0.0, 0.0, 1.0 );

	if( rPlaneHNF->getZ() != 0.0 ) {
		posB.setX( 1.0 );
		posB.setZ( -( rPlaneHNF->getH() + rPlaneHNF->getX() ) / rPlaneHNF->getZ() );
		posC.setY( 1.0 );
		posC.setZ( -( rPlaneHNF->getH() + rPlaneHNF->getY() ) / rPlaneHNF->getZ() );
	} else {
		if( rPlaneHNF->getY() != 0.0 ) {
            posB.setZ(1.0);
            posB.setY(  -(rPlaneHNF->getH() + rPlaneHNF->getZ()) / rPlaneHNF->getY() );

            posC.setX(1.0);
            posC.setY(  -(rPlaneHNF->getH() + rPlaneHNF->getX()) / rPlaneHNF->getY() );
		} else {
			posB.setY( 1.0 );
            posB.setX( -(rPlaneHNF->getH() + rPlaneHNF->getY()) / rPlaneHNF->getX() );
			posC.setZ( 1.0 );
            posC.setX( -(rPlaneHNF->getH() + rPlaneHNF->getZ()) / rPlaneHNF->getX() );
		}
	}

	delete( mVertA );
	delete( mVertB );
	delete( mVertC );

	mVertA = new Vertex( rPosA );
	mVertB = new Vertex( posB );
	mVertC = new Vertex( posC );

	updateHNFCOG();
	return true;
}

//! Set the plane using the two points of an axis and the center of gravity of a primitive.
//!
//! The positions are stored as follows:
//!     Vertex A is the bottom of the axis.
//!     Vertex B is the tip of the axis.
//!     Vertex C is the center of gravity of the primitive.
//!
//! @returns false in case of an error. True otherwise.
bool Plane::setPlaneByAxisAndPosition(
                const Vector3D& rAxisTop,
                const Vector3D& rAxisBottom,
                const Vector3D& rPos
) {
	if(!isValid())
		return false;

	setPositions( rAxisTop, rAxisBottom, rPos );
	mDefinitionType = AXIS_POINTS_AND_POSITION;

	return( true );
}

//! Change the orientation of the underlying vertices, to change the orientation of the plane's normal vector.
//! This is used e.g. for an OpenGL clipping plane.
bool Plane::flipPlane() {
	Vertex* tmpVert = mVertA;
	mVertA = mVertB;
	mVertB = tmpVert;
	mWasFlipped = not( mWasFlipped );
	updateHNFCOG();
	return true;
}

bool Plane::getPointProjected( Vector3D rPoint, Vector3D* rProjection ) {
	//! Returns the projection of the origin onto the plane.
	//! Returns false in case of an error, e.g. plane not properly defined.
	if( !rPoint.projectOntoPlane( mHNF ) ) {
		return false;
	}
	rProjection->set( &rPoint );
	return true;
}

//! Computes the homogenous matrix to change the basis
//! of points to the plane coordinate system.
//!
//! Optional: an axis, when present, can be used for alignment.
//!
//! @returns false in case of an error. True otherwise.
bool Plane::getChangeOfBasisTrans(
                Matrix4D* rTransMat,     //!< The requested transformation matrix.
                bool      rUseAxisToY    //!< Use the (optional) axis for alignment to Y. Used to export SVG drawings.
) {
	// Old version - to be removed. Outdated: 02/2018
	//-----------------------------------------------
	// Vector3D shiftVec;
	// if( !getPointProjected( Vector3D( 0.0, 0.0, 0.0, 1.0 ), &shiftVec ) ) {
	// 	cerr << "[Plane::" << __FUNCTION__ << "] ERROR: plane getPointProjected failed!" << endl;
	// 	return false;
	// }
	// Vector3D rotAxis = mHNF % Vector3D( 0.0, 0.0, 1.0, 0.0 );
	// double   rotAngle = angle( mHNF, Vector3D( 0.0, 0.0, 1.0, 0.0 ) );
	// Matrix4D transMat( shiftVec );
	// transMat *= Matrix4D( Vector3D( 0.0, 0.0, 0.0, 1.0 ), rotAxis, rotAngle );
	// transMat.dumpInfo( false, "old" );
	// rTransMat->set( transMat );

	// Sanity checks
	if( rTransMat == nullptr ) {
		return( false );
	}
	if( !isValid() ) {
		return( false );
	}

	bool axisPresent = ( mDefinitionType == AXIS_POINTS_AND_POSITION );

	if( axisPresent && rUseAxisToY ) { // Use the axis defined by Vertex A (Top) and B (Bottom)
		// 1.) The axis is aligned with y-axis
		Vector3D axisDir = mVertA->getCenterOfGravity() - mVertB->getCenterOfGravity();
		vector<double> baseChangeAxis { mVertB->getX(), mVertB->getY(), mVertB->getZ(), \
		                                axisDir.getX(), axisDir.getY(), axisDir.getZ() };
		Matrix4D baseChangeMatAxis( Matrix4D::INIT_BASE_CHANGE_TO_Y, &baseChangeAxis );
		// 2.) Align the plane normal with the z-axis by rotation and NOT translation.
		Vector3D baseDir( mHNF );
		baseDir.setH( 0.0 );
		baseDir.normalize3();
		baseDir.applyTransformation( baseChangeMatAxis );
		vector<double> baseChangePosDir { 0.0,            0.0,            0.0, \
		                                  baseDir.getX(), baseDir.getY(), baseDir.getZ()    };
		Matrix4D baseChangeMat( Matrix4D::INIT_BASE_CHANGE_TO_Z, &baseChangePosDir );
		baseChangeMatAxis *= baseChangeMat;
		rTransMat->set( baseChangeMatAxis );
	} else { // Alternative, newer Version for a base change using the HNF.
		Vector3D basePoint( mHNF );
		basePoint *= -basePoint.getH(); //! \todo check if there is somewhere a sign error or a wrong normal orientation.
		basePoint.setH( 1.0 );
		Vector3D baseDir( mHNF );
		baseDir.setH( 0.0 );
		baseDir.normalize3();
		vector<double> baseChangePosDir { basePoint.getX(), basePoint.getY(), basePoint.getZ(), \
		                                  baseDir.getX(),   baseDir.getY(),   baseDir.getZ()    };
		Matrix4D baseChangeMat( Matrix4D::INIT_BASE_CHANGE_TO_Z, &baseChangePosDir );
		//baseChangeMat.dumpInfo( false, "new" );
		rTransMat->set( baseChangeMat );
	}

	// Done:
	return( true );
}


//!@return the line of a plane-plane intersection by solving the linear equations of the HNF of plane and other plane and plane perpendicular to both.
//!@param other: other Plane to intersect with this
//!@param rLinePosA: line of intersection, 1st position
//!@param rLinePosB: line of intersection, 2nd position
bool Plane::getPlaneIntersection(
                Plane& rOther,
                Vector3D& rLinePosA,
                Vector3D& rLinePosB
) {
	Vector3D HNF1;
	Vector3D HNF2;
	getPlaneHNF( &HNF1 );
	rOther.getPlaneHNF( &HNF2 );
	
	// Fix LEGACY ERROR in getPlaneHNF
	HNF1.setH( - HNF1.getH() );
	HNF2.setH( - HNF2.getH() );
	
	Vector3D n = HNF1 % HNF2;
	
	Vector3D HNF3;
	HNF3.set( n );
	HNF3.setH( 0 );
	
	double rX = 0;
	double rY = 0;
	double rZ = 0;
	
	if( !solve3dimLinearSystem( HNF1, HNF2, HNF3, rX, rY, rZ ) ) {
		return( false );
	}
	
	rLinePosA.setX( rX );
	rLinePosA.setY( rY );
	rLinePosA.setZ( rZ );
	rLinePosA.setH( 1.0 );
	rLinePosB = rLinePosA + n;
	rLinePosB.setH( 1.0 );

	return( true );
}


//! @returns true if equation system is solvable.
//! Parameters are three vectors containing the equations (in the form: X * result1 + Y * result2 + Z * result3 = H. X, Y, Z and H are the components of the vectors) and the three results.
bool solve3dimLinearSystem( const Vector3D lg1, const Vector3D lg2, const Vector3D lg3, double& result1, double& result2, double& result3 ) {
	bool check = false;
	
	if( ( fabs( lg1.getX() ) < DBL_EPSILON ) and ( fabs( lg2.getX() ) < DBL_EPSILON ) and ( fabs( lg3.getX() ) < DBL_EPSILON ) ) {
		return false;
	}
	
	if( ( fabs( lg1.getX() ) < DBL_EPSILON ) and ( fabs( lg2.getX() ) < DBL_EPSILON ) ) {
		check = solve2dimLinearSystem( lg1, lg2, result2, result3 );
		result1 = ( lg3.getH() - lg3.getY() * result2 - lg3.getZ() * result3 ) / lg3.getX();
	}
	else if( ( fabs( lg1.getX() ) < DBL_EPSILON ) and ( fabs( lg3.getX() ) < DBL_EPSILON ) ) {
		check = solve2dimLinearSystem( lg1, lg3, result2, result3 );
		result1 = ( lg2.getH() - lg2.getY() * result2 - lg2.getZ() * result3 ) / lg2.getX();
	}
	else if( ( fabs( lg2.getX() ) < DBL_EPSILON ) and ( fabs( lg3.getX() ) < DBL_EPSILON ) ) {
		check = solve2dimLinearSystem( lg2, lg3, result2, result3 );
		result1 = ( lg1.getH() - lg1.getY() * result2 - lg1.getZ() * result3 ) / lg1.getX();
	}
	else if( fabs( lg1.getX() ) < DBL_EPSILON ) {
		Vector3D interim4 = lg1;
		Vector3D interim5 = lg2 - lg3 / lg3.getX() * lg2.getX();
		check = solve2dimLinearSystem( interim4, interim5, result2, result3 );
		result1 = ( lg3.getH() - lg3.getY() * result2 - lg3.getZ() * result3 ) / lg3.getX();
	}
	else if( fabs( lg2.getX() ) < DBL_EPSILON ) {
		Vector3D interim4 = lg1 - lg3 / lg3.getX() * lg1.getX();
		Vector3D interim5 = lg2;
		check = solve2dimLinearSystem( interim4, interim5, result2, result3 );
		result1 = ( lg3.getH() - lg3.getY() * result2 - lg3.getZ() * result3 ) / lg3.getX();
	}
	else if( fabs( lg3.getX() ) < DBL_EPSILON ) {
		Vector3D interim4 = lg1 - lg2 / lg2.getX() * lg1.getX();
		Vector3D interim5 = lg3;
		check = solve2dimLinearSystem( interim4, interim5, result2, result3 );
		result1 = ( lg1.getH() - lg1.getY() * result2 - lg1.getZ() * result3 ) / lg1.getX();
	}
	else {
		Vector3D interim4 = lg1 - lg3 / lg3.getX() * lg1.getX();
		Vector3D interim5 = lg2 - lg3 / lg3.getX() * lg2.getX();
		check = solve2dimLinearSystem( interim4, interim5, result2, result3 );
		result1 = ( lg1.getH() - lg1.getY() * result2 - lg1.getZ() * result3 ) / lg1.getX();
	}
	
	return check;
}

//! @returns true if equation system is solvable.
//! Parameters are two vectors containing the equations (in the form: Y * result1 + Z * result2 = H. X, Y, Z and H are the components of the vectors, X has to be 0) and the two results.
bool solve2dimLinearSystem( const Vector3D lg1, const Vector3D lg2, double& result1, double& result2 ) {
	if( ( lg1.getX() != 0 ) or ( lg2.getX() != 0 ) ) {
		return false;
	}
	if( ( fabs( lg1.getY() ) < DBL_EPSILON ) and ( fabs( lg1.getZ() ) < DBL_EPSILON ) ) {
		return false;
	}
	if( ( fabs( lg2.getY() ) < DBL_EPSILON ) and ( fabs( lg2.getZ() ) < DBL_EPSILON ) ) {
		return false;
	}
	if( ( fabs( lg1.getY() ) < DBL_EPSILON ) and ( fabs( lg2.getY() ) < DBL_EPSILON ) ) {
		return false;
	}
	if( ( fabs( lg1.getZ() ) < DBL_EPSILON ) and ( fabs( lg2.getZ() ) < DBL_EPSILON ) ) {
		return false;
	}
	
	if( fabs( lg1.getY() ) < DBL_EPSILON ) {
		Vector3D interim3;
		Vector3D interim2;
		
		if( fabs( lg2.getZ() ) < DBL_EPSILON ) {
			result1 = lg2.getH() / lg2.getY();
			result2 = lg1.getH() / lg1.getZ();
		}
		else {
			interim2 = lg2;
			interim3 = lg1;
			result2 = interim3.getH() / interim3.getZ();
			result1 = ( interim2.getH() - interim2.getZ() * result2 ) / interim2.getY();
		}
	}
	else if( fabs( lg1.getZ() ) < DBL_EPSILON ) {
		Vector3D interim2;
		Vector3D interim3;
		
		if( fabs( lg2.getY() ) < DBL_EPSILON ) {
			result1 = lg1.getH() / lg1.getY();
			result2 = lg2.getH() / lg2.getZ();
		}
		else {
			interim2 = Vector3D{ 0, lg2.getZ(), lg2.getY(), lg2.getH() };
			interim3 = Vector3D{ 0, 0, lg1.getY(), lg1.getH() };
			result1 = interim3.getH() / interim3.getZ();
			result2 = ( interim2.getH() - interim2.getZ() * result1 ) / interim2.getY();
		}
	}
	else {
		bool resort = false;
		Vector3D interim3;
		Vector3D interim1;
		
		if( fabs( lg2.getY() ) < DBL_EPSILON ) {
			interim1 = lg1;
			interim3 = lg2;
		}
		else if( fabs( lg2.getZ() ) < DBL_EPSILON ) {
			interim1 = Vector3D{ 0, lg1.getZ(), lg1.getY(), lg1.getH() };
			interim3 = Vector3D{ 0, 0, lg2.getY(), lg2.getH() };
			resort = true;
		}
		else {
			interim1 = lg1;
			interim3 = lg1 - lg2 / lg2.getY() * lg1.getY();
		}
		
		if( resort ) {
			result1 = interim3.getH() / interim3.getZ();
			result2 = ( interim1.getH() - interim1.getZ() * result1 ) / interim1.getY();
		}
		else {
			result2 = interim3.getH() / interim3.getZ();
			result1 = ( interim1.getH() - interim1.getZ() * result2 ) / interim1.getY();
		}
	}
	return true;
}

//! Writes data of current plane to the output stream.
//! The values of the HNF are separated by spaces.
std::ostream& operator<<( std::ostream& o, const Plane& p ) {
	Vector3D v;
	return( o << std::fixed << std::setprecision( 8 )
	          << p.getHNFA() << " "
	          << p.getHNFB() << " "
	          << p.getHNFC() << " "
	          << p.getHNFD() );
}
