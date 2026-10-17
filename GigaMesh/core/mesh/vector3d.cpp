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

#include <GigaMesh/mesh/vector3d.h>
#include <GigaMesh/mesh/matrix4d.h>

#include <GigaMesh/mesh/gmcommon.h> // for windows


#define VECTOR3DINITDEFAULTS   \
	x( 0.0 ),              \
	y( 0.0 ),              \
	z( 0.0 ),              \
	h( 1.0 )

using namespace std;

// === BEGIN Constructors =========================================================================================

//! Constructs the defualt vector, which is "Nullvektor" i.e. position vector of an origin.
Vector3D::Vector3D() : VECTOR3DINITDEFAULTS {
}

//! Constructs with all elements set to scalar.
Vector3D::Vector3D( double scalar ) : VECTOR3DINITDEFAULTS {
	x = scalar;
	y = scalar;
	z = scalar;
	h = scalar;
}

//! Constructs a homogeneous (4D) vector with x, y, z and h.
Vector3D::Vector3D( double setX, double setY, double setZ, double setH ) : VECTOR3DINITDEFAULTS {
	x = setX;
	y = setY;
	z = setZ;
	h = setH;
}

//! Use homogeneous coordinates from a given Vector3D.
Vector3D::Vector3D( Vector3D* vecToCopy ) : VECTOR3DINITDEFAULTS {
	x = vecToCopy->getX();
	y = vecToCopy->getY();
	z = vecToCopy->getZ();
	h = vecToCopy->getH();
}

//! Use homogeneous coordinates from given double array, which has to be of length 4.
Vector3D::Vector3D( const double* coordXYZH ) : VECTOR3DINITDEFAULTS {
	x = coordXYZH[0];
	y = coordXYZH[1];
	z = coordXYZH[2];
	h = coordXYZH[3];
}

//! Use coordinates from given double array of length 3 and a homogenous coordinate.
Vector3D::Vector3D( const double* coordXYZ, double setH ) : VECTOR3DINITDEFAULTS {
	x = coordXYZ[0];
	y = coordXYZ[1];
	z = coordXYZ[2];
	h = setH;
}

//! Constructs a direction vector (h=0.0) of length 1.0 with two given angles in degree or radiant.
//!
//! phiDeg v thetaDeg := [ -180.0 ... +180.0 ]
//!
//! The length of the vector is normalized!
Vector3D::Vector3D( double rPhi, double rTheta, bool rIsRadiant ) : VECTOR3DINITDEFAULTS {
	if( !rIsRadiant ) {
		rPhi   *= M_PI/180.0;
		rTheta *= M_PI/180.0;
	}
	if( abs( rPhi ) > M_PI ) {
		cerr << "[Vector3D::" << __PRETTY_FUNCTION__ << "] ERROR: angle phi out of range: " << rPhi << "!" << endl;
	}
	if( abs( rTheta ) > M_PI ) {
		cerr << "[Vector3D::" << __PRETTY_FUNCTION__ << "] ERROR: angle theta out of range: " << rTheta << "!" << endl;
	}
	x = sin( rTheta ) * cos( rPhi );
	y = sin( rTheta ) * sin( rPhi );
	z = cos( rTheta );
	h = 0.0;
	normalize3();
}

// === END of Constructors =========================================================================================================

void Vector3D::set( double setX, double setY, double setZ, double setH ) {
	//! Sets x, y and z (which can also be done by accessing the public properties.
	x = setX;
	y = setY;
	z = setZ;
	h = setH;
}

void Vector3D::set( Vector3D* vecToCopy ) {
	//! Use coordinates from given Vector3D.
	x = vecToCopy->getX();
	y = vecToCopy->getY();
	z = vecToCopy->getZ();
	h = vecToCopy->getH();
}

void Vector3D::set(const Vector3D& vecToCopy ) {
	//! Use coordinates from given Vector3D.
	x = vecToCopy.getX();
	y = vecToCopy.getY();
	z = vecToCopy.getZ();
	h = vecToCopy.getH();
}

void Vector3D::setX( double setX ) {
	//! Sets the x-coordinate.
	x = setX;
}

void Vector3D::setY( double setY ) {
	//! Sets the x-coordinate.
	y = setY;
}

void Vector3D::setZ( double setZ ) {
	//! Sets the x-coordinate.
	z = setZ;
}

void Vector3D::setH( double setH ) {
	//! Sets the homogenous coordinate.
	h = setH;
}

// Access ----------------------------------------------------------------------

double Vector3D::getX() const {
	//! Returns the x-coordinate.
	return x;
}

double Vector3D::getY() const {
	//! Returns the y-coordinate.
	return y;
}

double Vector3D::getZ() const {
	//! Returns the z-coordinate.
	return z;
}

double Vector3D::getH() const {
	//! Returns the homogene coordinate.
	return h;
}

double Vector3D::getLength3() const {
	//! Returns the length of the inhomogenous Vector3D.
	return sqrt( pow( x, 2 ) + pow( y, 2 ) + pow( z, 2 ) );
}

double Vector3D::getLength3Squared() const
{
	//! Returns the squared length of the inhomogenous Vector3D.
	return x * x + y * y + z * z;
}

double Vector3D::getLength4() const {
	//! Returns the length of the homogenous Vector3D.
	return sqrt( pow( x, 2 ) + pow( y, 2 ) + pow( z, 2 ) + pow( h, 2 ) );
}

double Vector3D::getAngleToXinXY() {
	//! Returns the angle (in radiant) to the x-axis of the vectors parallel
	//! projection along to z-axis into the xy-plane.

	// we will a nice not-a-number if we don't handle x == nominator == 0.0
	if( x == 0.0 ) {
		if( y > 0.0 ) { // +90°
			return M_PI/2.0;
		} else if( y < 0.0 ) { // -90°
			return -M_PI/2.0;
		} else { // 0°
			return 0.0;
		}
	}

	double alpha = atan( y/abs(x) );
	//cout << "[Vector3D::getAngleToXinXY] y: " << y << " x: " << x << endl;
	if( x < 0.0 ) {
		if( alpha < 0.0 ) {
			alpha = -M_PI - alpha;
		} else {
			alpha = M_PI - alpha;
		}
	}
	// for positive angles.
	//if( alpha < 0.0 ) {
	//	alpha += 2.0*M_PI;
	//}
	return alpha;
}

double Vector3D::getAngleToZinYZ() {
	//! Returns the angle (in radiant) to the z-axis of the vectors parallel
	//! projection along to x-axis into the yz-plane.
	//! \todo test this method!!!
	double alpha = atan( y/abs(z) );
	if( z < 0.0 ) {
		if( alpha < 0.0 ) {
			alpha += M_PI;
		} else {
			alpha -= M_PI;
		}
	}
	return alpha;
}

double Vector3D::getAngleToZ() {
	//! Returns the angle (in radiant) to the z-axis.
	return angle( this, Vector3D( 0.0f, 0.0f, 1.0f ) );
}

bool Vector3D::get( double* getX, double* getY, double* getZ, double* getH ) {
	//! Writes the vectors elements to given addresses (typically from an array).
	//! Returns false in case of failure.
	if( ( getX == nullptr ) || ( getY == nullptr ) || ( getZ == nullptr ) ) {
		cerr << "[Vector3D] get( ... ) got some illegal NULL pointers!" << endl;
		return false;
	}

	*getX = x;
	*getY = y;
	*getZ = z;
	if( getH != nullptr ) {
		*getH = h;
	}

	return true;
}

bool Vector3D::get3( double* getXYZ ) {
	//! Writes the vectors x-, y- and z-components to a given array of size 3.
	//! Returns false in case of failure.
	if( getXYZ == nullptr ) {
		cerr << "[Vector3D] get3( ... ) got an illegal NULL pointer!" << endl;
		return false;
	}

	getXYZ[0] = x;
	getXYZ[1] = y;
	getXYZ[2] = z;

	return true;
}

//! Writes the vectors x-, y- and z-components to a given array of size 3.
//! Returns false in case of failure.
bool  Vector3D::get3( float* rPositionXYZ ) {
	if( rPositionXYZ == nullptr ) {
		cerr << "[Vector3D::" << __FUNCTION__ << "] ERROR: NULL pointer!" << endl;
		return false;
	}
	rPositionXYZ[0] = x;
	rPositionXYZ[1] = y;
	rPositionXYZ[2] = z;
	return true;
}

bool Vector3D::getString3( string* rStr ) {
	//! Returns a string for the x, y and z-coordinate.
	if( rStr == nullptr ) {
		cerr << "[Vector3D] get3( ... ) got an illegal NULL pointer!" << endl;
		return false;
	}
	std::ostringstream s;
	s << x << " " << y << " " << z;
	(*rStr) = s.str();
	return true;
}

// Spherical coordinates -------------------------------------------------------

//! Returns the spherical coordinate Phi in degreee.
double Vector3D::getSphPhiDeg() {
	double phiInDegree = atan2( y, x ) * 180.0/M_PI;
	return phiInDegree;
}

//! Returns the spherical coordinate Theta in degreee.
double Vector3D::getSphThetaDeg() {
	double thetaInDegree = atan2( sqrt( x*x + y*y ), z ) * 180.0/M_PI;
	return thetaInDegree;
}

//! Returns the Radius of the spherical coordinate system.
double Vector3D::getSphRadius() {
	double radius = sqrt( x*x + y*y + z*z );
	return radius;
}

// Manipulation ----------------------------------------------------------------

double Vector3D::normalize3() {
	//! Normalizes the inhomogenous Vector3D and returns its original length.
	//!
	//! Only appropriate for direction vectors.
	//! Equal to Vector3D::setLength( 1.0 ), but faster.
	double len = getLength3();
	x /= len;
	y /= len;
	z /= len;
	return len;
}

double Vector3D::setLength3( double newLen ) {
	//! Scales the inhomogenous Vector3D to the new lenght 'newLen'.
	//!
	//! Only appropriate for direction vectors.
	double len = getLength3();
	x *= newLen / len;
	y *= newLen / len;
	z *= newLen / len;
	return len;
}

double Vector3D::normalizeW() {
	//! Normalizes the 4D Vector by its 4. element and returns the (old) value of the 4. element.
	//! x = x / h
	//! y = y / h
	//! z = z / h
	//! h = 1.0
	double oldH = h;
	x /= h;
	y /= h;
	z /= h;
	h = 1.0;
	return oldH;
}

void Vector3D::pow3( double exponent ) {
	//! "Powers" the inhomogenous vectors element by the given exponent
	//!
	//! Actually the difference between pow3 and pow4 should not matter
	//! as h is either 1.0 or 0.0 in most of our cases.
	x = pow( x, exponent );
	y = pow( y, exponent );
	z = pow( z, exponent );
}

void Vector3D::pow4( double exponent ) {
	//! "Powers" the homogenous vectors element by the given exponent
	//!
	//! Actually the difference between pow3 and pow4 should not matter
	//! as h is either 1.0 or 0.0 in most of our cases.
	x = pow( x, exponent );
	y = pow( y, exponent );
	z = pow( z, exponent );
	h = pow( h, exponent );
}

double Vector3D::sum3() {
	//! Sum of the inhomogenous vectors elements (ignoring the 4. element).
	return ( x + y + z );
}

double Vector3D::sum4() {
	//! Sum of the homogenous vectors elements.
	return ( x + y + z + h );
}

bool Vector3D::projectOntoPlane( Vector3D rPlaneHNF ) {
	//! Project the homogenous vector to a plane.
	//! Ignores homogenous element, which will be set to 1.0 as the result is a position on the plane.
	//! Returns false in case of an error, e.g. plane not properly defined.
	double nom   = rPlaneHNF.getX() * x + rPlaneHNF.getY() * y + rPlaneHNF.getZ() * z + rPlaneHNF.getH();
	double denom = sqrt( pow( rPlaneHNF.getX(), 2 ) + pow( rPlaneHNF.getY(), 2 ) + pow( rPlaneHNF.getZ(), 2 ) );
	if( denom == 0.0 ) {
		return false;
	}
	double distance = nom/denom;
	Vector3D shiftVec( rPlaneHNF );
	shiftVec.normalize3();
	shiftVec *= distance;
	x -= shiftVec.getX();
	y -= shiftVec.getY();
	z -= shiftVec.getZ();
	h = 1.0;
	return true;
}

//! Compute the distance to a line defined by two position vectors.
//!
//! See: http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
//!
//! @returns the distance to the line.
double Vector3D::distanceToLine( const Vector3D* rPos1, const Vector3D* rPos2 ) {
	double dist = abs3( ( (*this) - (*rPos1) ) % ( (*this) - (*rPos2) ) ) / abs3( (*rPos2) - (*rPos1) );
    return( dist );
}

double Vector3D::distanceToVectorWithouSqrt(const Vector3D *rPos1, const Vector3D *rPos2)
{
    double distance = 0.0;
    distance += (rPos1->getX()-rPos2->getX())*(rPos1->getX()-rPos2->getX());
    distance += (rPos1->getY()-rPos2->getY())*(rPos1->getY()-rPos2->getY());
    distance += (rPos1->getZ()-rPos2->getZ())*(rPos1->getZ()-rPos2->getZ());
    return distance;
}


//! Compute the angle within a cylindrical coordinate system using the given line i.e. axis.
//! Note that a rotation of the line into the y-axis as expected by Vertex::unrollAroundCylinderRadius.
//! Therefore Vector3D::getAngleToXinXY,
//! which will result in an other angle than for the other axis.
//! Also see Mesh::setVertFuncValAngleBasedOnAxis which is an optimized version for all vertices.
//! @returns angle in radiant. Attn: Not-A-Number can be returend for vertices on the line or in case of other errors.
double Vector3D::angleInLineCoord( const Vector3D* rPosTop, const Vector3D* rPosBottom ) {
	double angle = _NOT_A_NUMBER_DBL_;

	Vector3D orientVec = (*rPosTop) - (*rPosBottom); // direction of axis
	Matrix4D originTrans;
	originTrans.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Y, rPosBottom, &orientVec );

	Vector3D currPosition( this );
	currPosition.applyTransformation( originTrans );
	double x = currPosition.getX();
	double z = currPosition.getZ();
	angle = atan2( z, x ); //currPosition.getAngleToXinXY();

	return angle;
}

// Operators -------------------------------------------------------------------

void Vector3D::operator+= (const Vector3D& addVec) {
	//! Adds another Vector3D to this one.
	x += addVec.x;
	y += addVec.y;
	z += addVec.z;
	h += addVec.h;
}

void Vector3D::operator-= (const Vector3D& addVec) {
	//! Subtracts another Vector3D to this one.
	x -= addVec.x;
	y -= addVec.y;
	z -= addVec.z;
	h -= addVec.h;
}

void Vector3D::operator/= (double divVec) {
	//! Divides this Vector3D by a scalar.
	x /= divVec;
	y /= divVec;
	z /= divVec;
	h /= divVec;
}

void Vector3D::operator*= (double multVec) {
	//! Multiplies this Vector3D by a scalar.
	x *= multVec;
	y *= multVec;
	z *= multVec;
	h *= multVec;
}

void Vector3D::operator *= (const Matrix4D& multMat) {
	//! Multiplies the (homogenous!) Vector3D with a Matrix4D.
	//!
	//! see: http://www.mathworks.com/access/helpdesk/help/techdoc/ref/mtimes.html

	double vecX = x;
	double vecY = y;
	double vecZ = z;
	double vecH = h;

	x = vecX * multMat.getX(0) + vecY * multMat.getY(0) + vecZ * multMat.getZ(0) + vecH * multMat.getH(0);
	y = vecX * multMat.getX(1) + vecY * multMat.getY(1) + vecZ * multMat.getZ(1) + vecH * multMat.getH(1);
	z = vecX * multMat.getX(2) + vecY * multMat.getY(2) + vecZ * multMat.getZ(2) + vecH * multMat.getH(2);
	h = vecX * multMat.getX(3) + vecY * multMat.getY(3) + vecZ * multMat.getZ(3) + vecH * multMat.getH(3);
/*
	// A is an m-by-p matrix => m=1, p=4
	// B is an p-by-n matrix => p=4, n=4
	// => C is m-by-n => 1x4

	double matA[4] = { x, y, z, h };
	double matB[4][4];
	double matC[4] = { 0.0, 0.0, 0.0, 0.0 };

	multMat.getArray( (double*)&matB );

	// the loop i = [0...m] can be left out as m=1 so we can simplify:
	for( int j=0; j<4; j++ ) {
		for( int k=0; k<4; k++ ) {
			matC[j] += matA[k] * matB[k][j];
		}
	}

	x = matC[0];
	y = matC[1];
	z = matC[2];
	h = matC[3];
*/
}

bool Vector3D::operator!= (const Vector3D& compVec) {
	//! Compares another Vector3D with this one.
	return (compVec.getX() != x || compVec.getY() != y || compVec.getZ() != z);
}

bool Vector3D::operator== (const Vector3D& compVec) {
	//! Compares another Vector3D with this one.
	return (compVec.getX() == x && compVec.getY() == y && compVec.getZ() == z);
}

// Debuging --------------------------------------------------------------------

void Vector3D::dumpInfo( bool forMatlab, char* someNameTag ) {
	//! Prints the vectors coordinates to stdout.

	if( forMatlab ) {
		cout << someNameTag << " = [ " << x << ", " << y << ", " << z << ", " << h << " ]; " << endl;
		return;
	}

	cout << "[Vector3D] " << x << ", " << y << ", " << z << ", " << h << " Length3: " << getLength3() << " Length4: " << getLength4() << endl;
}

// GLOBAL Operators ------------------------------------------------------------

Vector3D operator% ( const Vector3D& crossVec1, const Vector3D& crossVec2 ) {
	//! Cross product between two vectors.
	Vector3D crossProd;
	crossProd.setX( crossVec1.getY() * crossVec2.getZ() - crossVec1.getZ() * crossVec2.getY() );
	crossProd.setY( crossVec1.getZ() * crossVec2.getX() - crossVec1.getX() * crossVec2.getZ() );
	crossProd.setZ( crossVec1.getX() * crossVec2.getY() - crossVec1.getY() * crossVec2.getX() );
	crossProd.setH( 0.0 );
	return crossProd;
}

Vector3D  operator+ ( const Vector3D& someVec1, const Vector3D& someVec2 ) {
	//! Adds two vectors.
	Vector3D vecAdd;
	vecAdd.setX( someVec1.getX()+someVec2.getX() );
	vecAdd.setY( someVec1.getY()+someVec2.getY() );
	vecAdd.setZ( someVec1.getZ()+someVec2.getZ() );
	vecAdd.setH( someVec1.getH()+someVec2.getH() );
	return vecAdd;
}

Vector3D  operator- ( const Vector3D& someVec ) {
	//! Reverse vector.
	Vector3D vecNegative( someVec );
	vecNegative.setX( -vecNegative.getX() );
	vecNegative.setY( -vecNegative.getY() );
	vecNegative.setZ( -vecNegative.getZ() );
	vecNegative.setH( -vecNegative.getH() );
	return vecNegative;
}

Vector3D  operator- ( const Vector3D& someVec1, const Vector3D& someVec2 ) {
	//! Substracts two vectors.
	Vector3D vecSubstract;
	vecSubstract.setX( someVec1.getX()-someVec2.getX() );
	vecSubstract.setY( someVec1.getY()-someVec2.getY() );
	vecSubstract.setZ( someVec1.getZ()-someVec2.getZ() );
	vecSubstract.setH( someVec1.getH()-someVec2.getH() );
	return vecSubstract;
}


//! Scalar product (or inner product) of the homogenous vector.
//! Attention: for the 3D inner product the 4th coordinate ("h" or "w") has to be zero. See Vector3D::setH().
//! @returns scalar.
double operator* ( const Vector3D& someVec1, const Vector3D& someVec2 ) {
	return someVec1.getX() * someVec2.getX() +
		   someVec1.getY() * someVec2.getY() +
		   someVec1.getZ() * someVec2.getZ() +
	       someVec1.getH() * someVec2.getH();
}

Vector3D operator* ( const Vector3D& someVec, const Matrix4D& someMat ) {
	//! Apply with Matrix4D, which is typically a transformation matrix.
	//! \todo this is not enterly correct as it works only for a transposed vector and than the result is different as this operation is not commutative 
	Vector3D vecToMult( someVec );
	vecToMult *= someMat;
	return vecToMult;
}

Vector3D operator* ( const Matrix4D& someMat, const Vector3D& someVec ) {
	//! Apply with Matrix4D, which is typically a transformation matrix.
	Vector3D vecToMult( someVec );
	vecToMult *= someMat;
	return vecToMult;
}

Vector3D  operator* ( const Vector3D& someVec, double scalar ) {
	//! Multiplication with a scalar.
	Vector3D vecToMult( someVec );
	vecToMult *= scalar;
	return vecToMult;
}

Vector3D  operator* ( double scalar, const Vector3D& someVec ) {
	//! Multiplication with a scalar.
	return someVec * scalar;
}

Vector3D  operator/ ( const Vector3D& someVec, double scalar ) {
	//! Division by a scalar.
	Vector3D vecToMult( someVec );
	vecToMult /= scalar;
	return vecToMult;
}

Vector3D  operator/ ( double scalar, const Vector3D& someVec ) {
	//! Division by a scalar.
	return someVec / scalar;
}

Vector3D normalize3(const Vector3D& someVec ) {
	//! Normalizes of the homogenous vector using Vector3D::getLength3.
	//! The 4. element stay unmodified.
	Vector3D normVec;
	double    len3 = someVec.getLength3();
	normVec.setX( someVec.getX() / len3 );
	normVec.setY( someVec.getY() / len3 );
	normVec.setZ( someVec.getZ() / len3 );
	normVec.setH( someVec.getH() );
	return normVec;
}

Vector3D normalize4(const Vector3D& someVec ) {
	//! Normalizes of the homogenous vector using Vector3D::getLength4.
	Vector3D normVec;
	double    len4 = someVec.getLength4();
	normVec.setX( someVec.getX() / len4 );
	normVec.setY( someVec.getY() / len4 );
	normVec.setZ( someVec.getZ() / len4 );
	normVec.setH( someVec.getH() / len4 );
	return normVec;
}

Vector3D compMult( const Vector3D& someVec1, const Vector3D& someVec2 ) {
	//! Multiplication of components (Matlab equivalent to .* )
	Vector3D resVec;
	resVec.setX( someVec1.getX() * someVec2.getX() );
	resVec.setY( someVec1.getY() * someVec2.getY() );
	resVec.setZ( someVec1.getZ() * someVec2.getZ() );
	resVec.setH( someVec1.getH() * someVec2.getH() );
	return resVec;	
}

double dot3( const Vector3D& someVec1, const Vector3D& someVec2 ) {
	//! dot-product for the inhomogenous vector - see: http://en.wikipedia.org/wiki/Dot_product
	double dotProd = 0.0;
	dotProd += someVec1.getX() * someVec2.getX();
	dotProd += someVec1.getY() * someVec2.getY();
	dotProd += someVec1.getZ() * someVec2.getZ();
	return dotProd;
}

double  dot4( const Vector3D& someVec1, const Vector3D& someVec2 ) {
	//! dot-product for the homogenous vector - see: http://en.wikipedia.org/wiki/Dot_product
	double dotProd = 0.0;
	dotProd += someVec1.getX() * someVec2.getX();
	dotProd += someVec1.getY() * someVec2.getY();
	dotProd += someVec1.getZ() * someVec2.getZ();
	dotProd += someVec1.getH() * someVec2.getH();
	return dotProd;
}

double angle( const Vector3D& vec1, const Vector3D& vec2 ) {
	//! Unsigned Angle (in radiant) between two inhomogenous vectors.
	double numerator   = vec1.getX() * vec2.getX() + vec1.getY() * vec2.getY() + vec1.getZ() * vec2.getZ();
	double denominator = vec1.getLength3() * vec2.getLength3();
	double lambda      = acos( numerator / denominator );
	if( std::isnan( lambda ) ) {
		//cerr << "[Vector3D] angle: acos( " << numerator << " / " << denominator << " )!" << endl;
		//cerr << "[Vector3D] angle: " << numerator-denominator << " FLT_EPSILON " << FLT_EPSILON << " !" << endl;
		// happens, when vectors have almost the same length - so we set:
		if( numerator < 0.0 ) {
			lambda = M_PI;
		} else {
			lambda = 0.0;
		}
	}
	return lambda;
}

double angle( const Vector3D& vec1, const Vector3D& vec2, const Vector3D& vecNormal ) {
	//! Signed Angle (in radiant) between two inhomogenous vectors.
	//! \todo: test if the sign is correct (or has to be inverted!)
	double lambda = angle( vec1, vec2 );
	double theta = angle( ( vec2 % vec1 ), vecNormal );
	if( theta > M_PI/2.0 ) {
		lambda = -lambda;
	}
	return lambda;
}

double angle3( double* vec1, double* vec2 ) {
	//! Angle (in radiant) between two inhomogenous vectors, given as double[3].
	//! \todo test angle3.
	double numerator   = vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
	double denominator = sqrt( pow( vec1[0], 2 ) + pow( vec1[1], 2 ) +pow( vec1[2], 2 ) ) * sqrt( pow( vec2[0], 2 ) + pow( vec2[1], 2 ) +pow( vec2[2], 2 ) );
	double lambda      = acos( numerator / denominator );
	return lambda;
}

double angle3ToZ( double* vec ) {
	//! Unsigned Angle (in radiant) between an inhomogenous vector given as double[3] and the z-axis ( 0.0, 0.0, 1.0 )^T.
	return acos( vec[2] / sqrt( pow( vec[0], 2 ) + pow( vec[1], 2 ) + pow( vec[2], 2 ) ) );
}

double abs3( const Vector3D& someVec ) {
	//! Length of the inhomogenous vector (ignoring the 4. element).
	//! Returns the same value as Vector3D::getLength3.
	return someVec.getLength3();
}

double abs4( const Vector3D& someVec ) {
	//! Length of the homogenous vector.
	//! Returns the same value as Vector3D::getLength4.
	return someVec.getLength4();
}

//! Projects the current vector onto the other vector and returns the result.
//! The current vector is _not_ changed by this operation.
Vector3D Vector3D::projectOnto(const Vector3D& someVec)
{
	double someVecLenSquared = someVec.getLength3() * someVec.getLength3();
	return(dot3(*this, someVec)*someVec/someVecLenSquared);
}

//! Perpendicular projection of a position (vector) onto a line
//! defined by two position vectors.
//!
//! @returns the project position (vector(.
Vector3D Vector3D::projectOntoLine( const Vector3D& rVertA, const Vector3D& rVertB ) {
	//line point a
	double a1 = rVertA.getX();
	double a2 = rVertA.getY();
	double a3 = rVertA.getZ();
	
	//line point b
	double b1 = rVertB.getX();
	double b2 = rVertB.getY();
	double b3 = rVertB.getZ();
	
	//line direction vector d
	double d1 = b1 - a1;
	double d2 = b2 - a2;
	double d3 = b3 - a3;
	
	//point p to project
	double p1 = getX();
	double p2 = getY();
	double p3 = getZ();
	
	//Q is projected P
	//Q is on line -> Q = A + u * D
	//Dot product (P - Q)*(B - A) = 0 if Q is Point closest to P on line
	//(P - (A + u * D))*(B - A) solved to u:
	double u = ( -p1*b1 + p1*a1 + a1*b1 - a1*a1 - p2*b2 + p2*a2 + a2*b2 - a2*a2 - p3*b3 + p3*a3 + a3*b3 - a3*a3 ) / ( d1*a1 - d1*b1 + d2*a2 - d2*b2 + d3*a3 - d3*b3 );
	Vector3D A{ a1, a2, a3, 1. };
	Vector3D D{ d1, d2, d3, 1. };
	Vector3D Q{ a1 + u * d1, a2 + u * d2, a3 + u * d3, 1. };
	return Q;
}

//! Applies a transformation matrix to the current vector. The vector data
//! is irreversibly changed by this operation.
Vector3D& Vector3D::applyTransformation(const Matrix4D &transformationMatrix) {
	(*this) = (*this) * transformationMatrix;
	return(*this);
}

//! Intersection i.e. overlap of two lines given with a total of
//! four position vectors.
//!
//! See: http://paulbourke.net/geometry/pointlineplane/lineline.c
//!
//! Calculate the line segment PaPb that is the shortest route between
//! two lines P1P2 and P3P4. Calculate also the values of mua and mub where
//!     Pa = P1 + mua (P2 - P1)
//!     Pb = P3 + mub (P4 - P3)
//! @returns false if no solution exists.
bool lineLineIntersect( const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, const Vector3D& p4, Vector3D& pa, Vector3D& pb ) {
	double mua, mub;

	Vector3D p13,p43,p21;
	double d1343,d4321,d1321,d4343,d2121;
	double numer,denom;

	p13.setX( p1.getX() - p3.getX() );
	p13.setY( p1.getY() - p3.getY() );
	p13.setZ( p1.getZ() - p3.getZ() );
	p43.setX( p4.getX() - p3.getX() );
	p43.setY( p4.getY() - p3.getY() );
	p43.setZ( p4.getZ() - p3.getZ() );

	if( fabs(p43.getX()) < DBL_EPSILON && (p43.getY()) < DBL_EPSILON && fabs(p43.getZ()) < DBL_EPSILON ) {
		return( false );
	}

	p21.setX( p2.getX() - p1.getX() );
	p21.setY( p2.getY() - p1.getY() );
	p21.setZ( p2.getZ() - p1.getZ() );

	if( fabs(p21.getX()) < DBL_EPSILON && fabs(p21.getY()) < DBL_EPSILON && fabs(p21.getZ()) < DBL_EPSILON ) {
		return( false );
	}

	d1343 = p13.getX() * p43.getX() + p13.getY() * p43.getY() + p13.getZ() * p43.getZ();
	d4321 = p43.getX() * p21.getX() + p43.getY() * p21.getY() + p43.getZ() * p21.getZ();
	d1321 = p13.getX() * p21.getX() + p13.getY() * p21.getY() + p13.getZ() * p21.getZ();
	d4343 = p43.getX() * p43.getX() + p43.getY() * p43.getY() + p43.getZ() * p43.getZ();
	d2121 = p21.getX() * p21.getX() + p21.getY() * p21.getY() + p21.getZ() * p21.getZ();

	denom = d2121 * d4343 - d4321 * d4321;
	if( fabs(denom) < DBL_EPSILON ) {
		return( false );
	}
	numer = d1343 * d4321 - d1321 * d4343;

	mua = numer / denom;
	mub = (d1343 + d4321 * mua) / d4343;

	pa.setX( p1.getX() + mua * p21.getX() );
	pa.setY( p1.getY() + mua * p21.getY() );
	pa.setZ( p1.getZ() + mua * p21.getZ() );
	pb.setX( p3.getX() + mub * p43.getX() );
	pb.setY( p3.getY() + mub * p43.getY() );
	pb.setZ( p3.getZ() + mub * p43.getZ() );

	return( true );
}

//! Writes data of current vector to the output stream. The coordinates are
//! separated by spaces.
std::ostream& operator<<(std::ostream& o, const Vector3D& v) {
	return(o << std::fixed << std::setprecision(8)
	         << v.getX() << " "
	         << v.getY() << " "
	         << v.getZ() << " "
	         << v.getH() );
}
