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

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include "sys/types.h"

#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <climits>

#include <GigaMesh/mesh/vertex.h>

#include <GigaMesh/mesh/face.h>
#include <GigaMesh/mesh/edgegeodesic.h>
#include <GigaMesh/logging/Logging.h>

// proper access to the center of gravity / position:
#define POS_X           mPosition[0]
#define POS_Y           mPosition[1]
#define POS_Z           mPosition[2]
// proper access to the normal:
#define NORMAL_X        mNormalXYZ[0]
#define NORMAL_Y        mNormalXYZ[1]
#define NORMAL_Z        mNormalXYZ[2]

// proper access to the texture:
#define TEX_RED         mTexRGBA[0]
#define TEX_GREEN       mTexRGBA[1]
#define TEX_BLUE        mTexRGBA[2]
#define TEX_ALPHA       mTexRGBA[3]

// Sets default values - to be used by all contructors!
// ----------------------------------------------------
#define VERTEXINITDEFAULTS \
	mFuncValue( 0.0 ),                  \
	mIdx( _PRIMITIVE_NOT_INDEXED_ ),    \
	mIdxOri( _PRIMITIVE_NOT_INDEXED_ ), \
	mLabelNr( 0 ),                      \
	mFeatureVecLen( 0 ),                \
	mFeatureVec( NULL )                 \

using namespace std;

Vertex::Vertex()
    : VERTEXINITDEFAULTS  {
	//! Constructor with no arguments - will construct a vertex with the origin (0,0,0) as position.
	POS_X     = 0.0;
	POS_Y     = 0.0;
	POS_Z     = 0.0;
	// --- SPECIFIC ---
	TEX_RED   =   0;
	TEX_GREEN =   0;
	TEX_BLUE  =   0;
	TEX_ALPHA = 255;
}

Vertex::Vertex( const int rSetIdx, const sVertexProperties& rSetProps )
    : VERTEXINITDEFAULTS  {
	//! Constructor e.g. to be used when a Vertex list is read from a file.
	// defaults:
	NORMAL_X = _NOT_A_NUMBER_DBL_;
	NORMAL_Y = _NOT_A_NUMBER_DBL_;
	NORMAL_Z = _NOT_A_NUMBER_DBL_;

	POS_X = rSetProps.mCoordX;
	POS_Y = rSetProps.mCoordY;
	POS_Z = rSetProps.mCoordZ;
	// Function value
	mFuncValue = rSetProps.mFuncVal;
	// Indexing:
	mIdx     = rSetIdx;
	mIdxOri  = rSetIdx;
	// Color and transparency
	TEX_RED   = rSetProps.mColorRed;
	TEX_GREEN = rSetProps.mColorGrn;
	TEX_BLUE  = rSetProps.mColorBle;
	TEX_ALPHA = rSetProps.mColorAlp;
	// Labels
	mLabelNr  =  rSetProps.mLabelId;
	// Flags - BEFORE NORMALS!
	setFlagAll( rSetProps.mFlags );
	// Normals
	clearFlag( FLAG_NORMAL_SET );
	setNormal( rSetProps.mNormalX, rSetProps.mNormalY, rSetProps.mNormalZ );
}

//! Constructor
Vertex::Vertex( unsigned int rSetIdx, double rPosX, double rPosY, double rPosZ  )
    : VERTEXINITDEFAULTS  {
	// defaults:
	NORMAL_X = _NOT_A_NUMBER_DBL_;
	NORMAL_Y = _NOT_A_NUMBER_DBL_;
	NORMAL_Z = _NOT_A_NUMBER_DBL_;

	POS_X = rPosX;
	POS_Y = rPosY;
	POS_Z = rPosZ;
	// indexing:
	mIdx     = rSetIdx;
	mIdxOri  = rSetIdx;
	// --- SPECIFIC ---
	TEX_RED   = 187;
	TEX_GREEN = 187;
	TEX_BLUE  = 187;
	TEX_ALPHA = 255;
}

//! Constuctor using a position Vector3D.
Vertex::Vertex( Vector3D vertPos )
    : VERTEXINITDEFAULTS {
	// defaults:
	NORMAL_X  = _NOT_A_NUMBER_DBL_;
	NORMAL_Y  = _NOT_A_NUMBER_DBL_;
	NORMAL_Z  = _NOT_A_NUMBER_DBL_;
	TEX_RED   = 187;
	TEX_GREEN = 187;
	TEX_BLUE  = 187;
	TEX_ALPHA = 255;

	POS_X    = vertPos.getX();
	POS_Y    = vertPos.getY();
	POS_Z    = vertPos.getZ();
}

Vertex::Vertex( Vector3D vertPos, Vector3D vertNormal )
    : VERTEXINITDEFAULTS {
	//! Constuctor using a position Vector3D and a direction Vector3D.
	// defaults:
	NORMAL_X  = _NOT_A_NUMBER_DBL_;
	NORMAL_Y  = _NOT_A_NUMBER_DBL_;
	NORMAL_Z  = _NOT_A_NUMBER_DBL_;
	TEX_RED   = 187;
	TEX_GREEN = 187;
	TEX_BLUE  = 187;
	TEX_ALPHA = 255;

	POS_X          = vertPos.getX();
	POS_Y          = vertPos.getY();
	POS_Z          = vertPos.getZ();
	NORMAL_X       = vertNormal.getX();
	NORMAL_Y       = vertNormal.getY();
	NORMAL_Z       = vertNormal.getZ();
	setFlag( FLAG_NORMAL_SET );
}

Vertex::Vertex(Vector3D vertPos, Vector3D vertNormal, uint64_t setLabelTo )
    : VERTEXINITDEFAULTS {
	//! Constuctor using a position Vector3D, a direction Vector3D and a valid label no.
	// defaults:
	NORMAL_X  = _NOT_A_NUMBER_DBL_;
	NORMAL_Y  = _NOT_A_NUMBER_DBL_;
	NORMAL_Z  = _NOT_A_NUMBER_DBL_;
	TEX_RED   = 187;
	TEX_GREEN = 187;
	TEX_BLUE  = 187;
	TEX_ALPHA = 255;

	setLabel( setLabelTo );
	POS_X          = vertPos.getX();
	POS_Y          = vertPos.getY();
	POS_Z          = vertPos.getZ();
	NORMAL_X       = vertNormal.getX();
	NORMAL_Y       = vertNormal.getY();
	NORMAL_Z       = vertNormal.getZ();
	setFlag( FLAG_NORMAL_SET );
}

Vertex::Vertex( Vertex* vertA, Vertex* vertB, double weightA, double weightB, double weightPos )
    : VERTEXINITDEFAULTS {
	//! Creates a new vertex interpolated geometrically between the given weighted vertices.
	// defaults:
	NORMAL_X  = _NOT_A_NUMBER_DBL_;
	NORMAL_Y  = _NOT_A_NUMBER_DBL_;
	NORMAL_Z  = _NOT_A_NUMBER_DBL_;
	TEX_RED   = 187;
	TEX_GREEN = 187;
	TEX_BLUE  = 187;
	TEX_ALPHA = 255;

	if( weightB < weightA ) {
		// when numbers are reversed, we have to compensate:
		weightA   = -weightA;
		weightB   = -weightB;
		weightPos = -weightPos;
	}
	double weightA0 = ( weightPos - weightA ) / ( weightB - weightA );
	double weightB0 = ( weightB - weightPos ) / ( weightB - weightA );
	Vector3D vertDir = vertA->getPositionVector() - vertB->getPositionVector();
	vertDir *= weightB0;
	Vector3D vertPos = vertB->getPositionVector() + vertDir;
	Vector3D vertNormA = vertA->getNormal();
	Vector3D vertNormB = vertB->getNormal();
	// --- SPECIFIC ---
	POS_X     = vertPos.getX();
	POS_Y     = vertPos.getY();
	POS_Z     = vertPos.getZ();
	NORMAL_X  = vertNormA.getX() * weightA0 + vertNormB.getX() * weightB0;
	NORMAL_Y  = vertNormA.getY() * weightA0 + vertNormB.getY() * weightB0;
	NORMAL_Z  = vertNormA.getX() * weightA0 + vertNormB.getZ() * weightB0;
	setFlag( FLAG_NORMAL_SET );
	mFuncValue = weightPos;
	// --- DEBUG ---
	//vertA->dumpInfo();
	//vertB->dumpInfo();
	//dumpInfo();
	//cout << "[Vertex::Vertex] weightA: " << weightA << " weightB: " << weightB << " weightPos: " << weightPos << endl;
}

Vertex::~Vertex() {
	//! Destructor sets properties to not a number or NULL.
	//!
	//! This is done just in case we referer to an object still in the memory,
	//! which is already destroyed.
	if( mFeatureVec != nullptr ) {
		delete[] mFeatureVec;
		mFeatureVecLen = 0;
		mFeatureVec    = nullptr;
	}
}

// Indexing -------------------------------------------------------------------

bool Vertex::setIndex( int setIdx ) {
	//! Setes the index of a Primitive. Additionally it preserves the very first
	//! index set. Returns false in case the index has not been set.
	mIdx = setIdx;
	return true;
}

int Vertex::getIndex() const {
	//! Retrieves the primitives index. Typcally used to write a mesh to a file.
	//!
	//! Attention: the returned index might be wrong due to alterations to the 
	//! the meshs structure or not set at all.
	return mIdx;
}

int Vertex::getIndexOriginal() {
	//! Retrieves the primitives original index, when a Mesh file was read.
	return mIdxOri;
}

void Vertex::getIndexOffsetBit( int* bitOffset, int* bitNr ) {
	//! Returns the index of the vertex for a bit array using an array of 64-bit long integers.
	//! relates to Mesh::fetchSphereBitArray
	*bitOffset = mIdx / ( 8*sizeof( uint64_t ) );
	*bitNr     = mIdx - (*bitOffset)*8*sizeof( uint64_t );
}

//! Marks this vertex as visited by setting the bit in the bit array.
//! Returns the old state of the bit;
bool Vertex::markVisited( uint64_t* rVertBitArrayVisited ) {
	int bitOffset = mIdx / ( 8*sizeof( uint64_t ) );
	int bitNr     = mIdx - bitOffset*8*sizeof( uint64_t );
	uint64_t bitPattern = static_cast<uint64_t>(1)<<bitNr;
	bool retVal = rVertBitArrayVisited[bitOffset] & bitPattern;
	rVertBitArrayVisited[bitOffset] |= bitPattern;
	return retVal;
}

//! Unmarks this vertex as visited by setting the bit in the bit array.
//! Returns the old state of the bit;
bool Vertex::unmarkVisited( uint64_t* rVertBitArrayVisited ) {
	int bitOffset = mIdx / ( 8*sizeof( uint64_t ) );
	int bitNr     = mIdx - bitOffset*8*sizeof( uint64_t );
	uint64_t bitPattern = static_cast<uint64_t>(1)<<bitNr;
	bool retVal = rVertBitArrayVisited[bitOffset] & bitPattern;
	rVertBitArrayVisited[bitOffset] &= ~bitPattern;
	return retVal;
}

//! Returns the bit of the bit array correspond to this vertices' index.
bool Vertex::isMarked( const uint64_t* rVertBitArrayVisited ) {
	int bitOffset = mIdx / ( 8*sizeof( uint64_t ) );
	int bitNr     = mIdx - bitOffset*8*sizeof( uint64_t );
	uint64_t bitPattern = static_cast<uint64_t>(1)<<bitNr;
	return ( rVertBitArrayVisited[bitOffset] & bitPattern ) != 0;
}

// Color managment -------------------------------------------------------------

bool Vertex::setRGB( unsigned char setTexR, unsigned char setTexG, unsigned char setTexB ) {
	//! Sets R,G and B of a primitves texture color. 
	//!
	//! Remark: will return false and show an error message, when the primitives
	//! pointers to the texture map array are NULL.
	TEX_RED   = setTexR;
	TEX_GREEN = setTexG;
	TEX_BLUE  = setTexB;
	return true;
}

bool Vertex::setAlpha(unsigned char alpha)
{
	if(alpha > 255)
	{
		return false;
	}
	TEX_ALPHA = alpha;
	return true;
}

bool Vertex::setAlpha( double rVal ) {
	//! Set alpha/transparency for this vertex - range: 0.0 to 1.0
	//! Will return false in case of an error.
	if( ( rVal < 0.0 ) || ( rVal > 1.0 ) ) {
		return false;
	}
	TEX_ALPHA = static_cast<unsigned char>(rVal * 255.0);
	return true;
}

// Information retrival ---------------------------------------------------------------------------

//! Returns the x-coordinate of the position vector of the Vertex
//! Return not-a-number in case of an error regarding to Primitive::getX()
double Vertex::getX() const {
	return POS_X;
}

//! Returns the x-coordinate of the position vector of the Vertex
//! Return not-a-number in case of an error regarding to Primitive::getY()
double Vertex::getY() const {
	return POS_Y;
}

//! Returns the x-coordinate of the position vector of the Vertex
//! Return not-a-number in case of an error regarding to Primitive::getZ()
double Vertex::getZ() const {
	return POS_Z;
}

double Vertex::getNormalX() {
	//! Returns the x-coordinate of the vertex normal, when set.
	//! Return not-a-number in case of an error.
	return NORMAL_X;
}

double Vertex::getNormalY() {
	//! Returns the y-coordinate of the vertex normal, when set.
	//! Return not-a-number in case of an error.
	return NORMAL_Y;
}

double Vertex::getNormalZ() {
	//! Returns the z-coordinate of the vertex normal, when set.
	//! Return not-a-number in case of an error.
	return NORMAL_Z;
}

//! Returns the length of the normal vector, which is typically 1.0.
double Vertex::getNormalLen() const {
	double retVal = sqrt( pow( NORMAL_X, 2.0 ) +
	                      pow( NORMAL_Y, 2.0 ) +
	                      pow( NORMAL_Z, 2.0 ) );
	return( retVal );
}

Vector3D Vertex::getCenterOfGravity() {
	//! Returns the center of gravity, which is stored in an external array.
	//! When the reference to this array is not set Vector3D( nan, nan, nan, 1.0 )
	//! will be returned!
	return Vector3D( POS_X, POS_Y, POS_Z, 1.0 );
}

Vector3D Vertex::getNormal( bool normalized ) {
	//! Returns the normal vector.
	//! Returns (nan,nan,nan,0.0) in case of an error.
	if( !getFlag( FLAG_NORMAL_SET ) ) {
		estNormalAvgAdjacentFaces();
	}
	return Primitive::getNormal( normalized );
}

bool Vertex::copyNormalXYZTo( float* rNormalXYZ, bool rNormalized ) {
	//! Copies the x-, y- and z-elements of the normal vector to a given array, which has to be at least of size 3.
	//! Normal vector is either taken from an array, when set or from the 1-ring neighbourhood.
	if( !getFlag( FLAG_NORMAL_SET ) ) {
		estNormalAvgAdjacentFaces();
	}
	return Primitive::copyNormalXYZTo( rNormalXYZ, rNormalized );
}

bool Vertex::copyNormalXYZTo( double* rNormalXYZ, bool rNormalized ) {
	//! Copies the x-, y- and z-elements of the normal vector to a given array and its length set to setLen.
	//! The given array has to be at least of size 3.
	//! Returns false in case of an error.
	if( !getFlag( FLAG_NORMAL_SET ) ) {
		estNormalAvgAdjacentFaces();
	}
	return Primitive::copyNormalXYZTo( rNormalXYZ, rNormalized );
}

bool Vertex::copyNormalXYZTo( double* rNormalXYZ, double rSetLength ) {
	//! Copies the x-, y- and z-elements of the normal vector to a given array and its length set to setLen.
	//! The given array has to be at least of size 3.
	//! Returns false in case of an error.
	if( !getFlag( FLAG_NORMAL_SET ) ) {
		estNormalAvgAdjacentFaces();
	}
	return Primitive::copyNormalXYZTo( rNormalXYZ, rSetLength );
}

bool Vertex::copyRGBTo( unsigned char* rColor ) {
	//! Copies the red, blue and green value to a given array, which has to be at least of size 3.
	//! Returns false in case of an error.
	if( rColor == nullptr ) {
		return false;
	}
	memcpy( rColor, mTexRGBA, 3*sizeof(unsigned char) );
	return true;
}

bool Vertex::copyRGBATo( unsigned char* rColor ) {
	//! Copies the red, blue, green and alpha value to a given array, which has to be at least of size 4.
	//! Returns false in case of an error.
	if( rColor == nullptr ) {
		return false;
	}
	memcpy( rColor, mTexRGBA, 4*sizeof(unsigned char) );
	return true;
}

//! Red value of the vertex color in RGBA.
//! @returns the amount of red the color per Vertex.
unsigned char Vertex::getR() {
	return mTexRGBA[0];
}

//! Green value of the vertex color in RGBA.
//! @returns the amount of green the color per Vertex.
unsigned char Vertex::getG() {
	return mTexRGBA[1];
}

//! Blue value of the vertex color in RGBA.
//! @returns the amount of blue the color per Vertex.
unsigned char Vertex::getB() {
	return mTexRGBA[2];
}

//! Alpha value of the vertex color in RGBA
//! @returns the amount of transparency of the color per Vertex.
unsigned char Vertex::getA() {
	return mTexRGBA[3];
}

//! Return the graylevel of the vertices color.
//! Average of Red, Green and Blue.
//! Alpha value is ignored.
//! Note that there are other conversion from RGB to gray.
//! Inspired by: http://home.arcor.de/ulile/node54.html
bool Vertex::getGraylevel( double* rGraylevel, eGrayLevelConversion rGrayLevelConvMethod ) {
	switch( rGrayLevelConvMethod ) {
		case RGB_TO_GRAY_AVERAGE:
			*rGraylevel = ( static_cast<double>(mTexRGBA[0]) + static_cast<double>(mTexRGBA[1]) + static_cast<double>(mTexRGBA[2]) ) / ( 255.0 * 3.0 );
			break;
		case RGB_TO_GRAY_AVERAGE_WEIGHTED: // GIMP inspired
			*rGraylevel = ( 0.3*static_cast<double>(mTexRGBA[0]) + 0.59*static_cast<double>(mTexRGBA[1]) + 0.11*static_cast<double>(mTexRGBA[2]) ) / 255.0;
			break;
		case RGB_TO_GRAY_SATURATION_REMOVAL:
			*rGraylevel = ( static_cast<double>(max( max( mTexRGBA[0], mTexRGBA[1] ), mTexRGBA[2] )) + static_cast<double>(min( min( mTexRGBA[0], mTexRGBA[1] ), mTexRGBA[2] )) ) / ( 255.0 * 2.0 );
			break;
		case RGB_TO_GRAY_HSV_DECOMPOSITION:
			*rGraylevel = ( static_cast<double>(max( max( mTexRGBA[0], mTexRGBA[1] ), mTexRGBA[2] )) ) / 255.0;
			break;
		default:
			LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Unknown conversion method: " << rGrayLevelConvMethod << "\n";
			break;
	}
	return true;
}

// Value access -----------------------------------------------------------------------------------

//! Retrieve postion vector  ("Ortsvektor").
//!
//! @returns a homogenous position vector.
bool Vertex::getPositionVector( Vector3D* rPosVec ) const {
	rPosVec->set( POS_X, POS_Y, POS_Z, 1.0 );
	return( true );
}

Vector3D Vertex::getPositionVector() const {
	//! Returns a homogenous position vector ("Ortsvektor").
	Vector3D posVec( POS_X, POS_Y, POS_Z, 1.0 );
	return posVec;
}

double Vertex::distanceToHNF( Vector3D HNF_Position, Vector3D HNF_Normal ) {
	//! Estimates the Hessian normal form (HNF) of a plane defined by a position vector and an orientation vector
	//! and returns the distance between the plane and this Vertex.
	// HNF_Normal is expected to have length == 1 !!!
	double distance = 0.0;
	// HNF: x_norm + y_norm + z_norm + lambda --> lambda given by point HNF_Position
	double lambda = HNF_Normal.getX()*HNF_Position.getX() + HNF_Normal.getY()*HNF_Position.getY() + HNF_Normal.getZ()*HNF_Position.getZ();

	distance = HNF_Normal.getX()*getX()+HNF_Normal.getY()*getY()+HNF_Normal.getZ()*getZ()-lambda;
	return distance;
}

//! Compute the distance of this Vertex (x0) to line defined by x1 and x2.
//! See Vector3D::distanceToLine
//!
//! See: http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
//!
//! @returns distance of this vertex to a given line. NaN in case of an error.
double Vertex::distanceToLine( const Vector3D* rPos1, const Vector3D* rPos2 ) {
	if( ( rPos1 == nullptr ) || ( rPos2 == nullptr ) ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] ERROR: Null pointer given!\n";
		return _NOT_A_NUMBER_DBL_;
	}
	Vector3D x0 = getPositionVector();
	double dist = x0.distanceToLine( rPos1, rPos2 ); // abs3( ( x0 - x1 ) % ( x0 - x2 ) ) / abs3( x2 - x1 );
	return dist;
}

//! Compute the angle within a cylindrical coordinate system using the given line i.e. axis.
//! Note that a rotation of the line into the y-axis as expected by Vertex::unrollAroundCylinderRadius.
//! Therefore Vector3D::getAngleToXinXY,
//! which will result in an other angle than for the other axis.
//! Also see Mesh::setVertFuncValAngleBasedOnAxis which is an optimized version for all vertices.
//! @returns angle in radiant. Attn: Not-A-Number can be returend for vertices on the line or in case of other errors.
double Vertex::angleInLineCoord( const Vector3D* rPosTop, const Vector3D* rPosBottom ) {
	Vector3D currPosition = this->getPositionVector();
	return( currPosition.angleInLineCoord( rPosTop, rPosBottom ) );
}

double Vertex::distanceToCoord( const double* someXYZ ) {
	//! Returns simple euklidian distance.
	//! someXYZ has to be of length 3.
	//! Does not do any checks - so Vertex has to be initalized carefully.
	double dX = POS_X - someXYZ[0];
	double dY = POS_Y - someXYZ[1];
	double dZ = POS_Z - someXYZ[2];
	return sqrt( ( dX * dX ) + ( dY * dY ) + ( dZ * dZ ) );
}

//! Copies the x-, y- an z-coordinate to a given array of type float - which has to be of size 3.
bool Vertex::copyCoordsTo( float* rCoordArr ) {
	rCoordArr[0] = static_cast<float>(POS_X);
	rCoordArr[1] = static_cast<float>(POS_Y);
	rCoordArr[2] = static_cast<float>(POS_Z);
	return true;
}

//! Copies the x-, y- an z-coordinate to a given array of type double - which has to be of size 3.
bool Vertex::copyCoordsTo( double* rCoordArr ) {
	memcpy( rCoordArr, mPosition, 3*sizeof(double) );
	return true;
}

//! Copies the vertex properties.
bool Vertex::copyVertexPropsTo( sVertexProperties& rVertexProps ) const {
	rVertexProps.mCoordX   = mPosition[0];
	rVertexProps.mCoordY   = mPosition[1];
	rVertexProps.mCoordZ   = mPosition[2];
	rVertexProps.mNormalX  = NORMAL_X;
	rVertexProps.mNormalY  = NORMAL_Y;
	rVertexProps.mNormalZ  = NORMAL_Z;
	rVertexProps.mFuncVal  = mFuncValue;
	rVertexProps.mColorRed = TEX_RED;
	rVertexProps.mColorGrn = TEX_GREEN;
	rVertexProps.mColorBle = TEX_BLUE;
	rVertexProps.mColorAlp = TEX_ALPHA;
	rVertexProps.mLabelId  = mLabelNr;
	// Flags
	uint64_t vertFlag;
	if( !getFlagAll( &vertFlag ) ) {
		return( false );
	}
	rVertexProps.mFlags = vertFlag;
	return( true );
}

//! Estimates the average normal of the adjacent faces and stores them into
//! the vertices normal vector.
//!
//! See VertexOfFace::estNormalAvgAdjacentFaces
//!
//! @returns false in case of an error - e.g. no adjacent faces. True otherwise.
bool Vertex::estNormalAvgAdjacentFaces() {
	return( false );
}

//! Set normal vector for this vertex.
//! Checks if the given vector is valid i.e. a normal number.
//! In case of an abnormal number the normal will not be altered.
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::setNormal( double rNormX, double rNormY, double rNormZ ) {
	if( isnormal( sqrt( rNormX*rNormX + rNormY*rNormY + rNormZ*rNormZ ) ) ) {
		mNormalXYZ[0] = rNormX;
		mNormalXYZ[1] = rNormY;
		mNormalXYZ[2] = rNormZ;
		setFlag( FLAG_NORMAL_SET );
		return( true );
	}
	return( false );
}

//! Set normal vector for this vertex.
//! Checks if the vector is valid (length > 0)
//! @returns false in case of an error. True otherwise.
bool Vertex::setNormal( Vector3D* rNormal ) {
	if( rNormal == nullptr ) {
		return false;
	}
	double normX = rNormal->getX();
	double normY = rNormal->getY();
	double normZ = rNormal->getZ();
	return setNormal( normX, normY, normZ );
}

//! Unset the normal vector:
//!    .) Elements are set to Not-A-Number (NaN)
//!    .) The FLAG_NORMAL_SET will be cleared.
//!
//! @returns false in case of an error.
bool Vertex::unsetNormal() {
	mNormalXYZ[0] = _NOT_A_NUMBER_DBL_;
	mNormalXYZ[1] = _NOT_A_NUMBER_DBL_;
	mNormalXYZ[2] = _NOT_A_NUMBER_DBL_;
	clearFlag( FLAG_NORMAL_SET );
	return( true );
}

//! Estimates the total area of the adjacent faces.
double Vertex::get1RingArea() {
	return 0.0f;
}

double Vertex::get1RingEdgeLenAvg() {
	//! Estimates the average distance to the 1-ring neighbour vertices.
	//! Returns not-a-number, when the Vertex has no neighbours.
	set<Vertex*> ringNeigh;
	getNeighbourVertices( &ringNeigh );
	if( ringNeigh.size() == 0 ) {
		return _NOT_A_NUMBER_DBL_;
	}
	double sumLen = 0.0;
	set<Vertex*>::iterator itVertex;
	for( itVertex=ringNeigh.begin(); itVertex!=ringNeigh.end(); itVertex++ ) {
		sumLen += ( getPositionVector() - (*itVertex)->getPositionVector() ).getLength3();
	}
	return sumLen/ringNeigh.size();
}

//! Estimates the minimum distance to the 1-ring neighbour vertices.
//! Returns not-a-number, when the Vertex has no neighbours.
double Vertex::get1RingEdgeLenMin() {
	set<Vertex*> ringNeigh;
	getNeighbourVertices( &ringNeigh );
	if( ringNeigh.size() == 0 ) {
		return _NOT_A_NUMBER_DBL_;
	}
	double minLen = +DBL_MAX;
	set<Vertex*>::iterator itVertex;
	for( itVertex=ringNeigh.begin(); itVertex!=ringNeigh.end(); itVertex++ ) {
		double currLen = ( getPositionVector() - (*itVertex)->getPositionVector() ).getLength3();
		if( currLen < minLen ) {
			minLen = currLen;
		}
	}
	return minLen;
}

double Vertex::get1RingEdgeLenMax() {
	//! Estimates the minimum distance to the 1-ring neighbour vertices.
	//! Returns not-a-number, when the Vertex has no neighbours.
	set<Vertex*> ringNeigh;
	getNeighbourVertices( &ringNeigh );
	if( ringNeigh.size() == 0 ) {
		return _NOT_A_NUMBER_DBL_;
	}
	double maxLen = 0.0;
	set<Vertex*>::iterator itVertex;
	for( itVertex=ringNeigh.begin(); itVertex!=ringNeigh.end(); itVertex++ ) {
		double currLen = ( getPositionVector() - (*itVertex)->getPositionVector() ).getLength3();
		if( currLen > maxLen ) {
			maxLen = currLen;
		}
	}
	return maxLen;
}

//! Number of adjacent Faces, which have to be pre-determined!
//!
//! @returns the number of adjacent Faces (when pre-determined).
uint64_t Vertex::get1RingFaceCount() const {
	return( 0 );
}

//! Returns the sum of the angles of the adjacent faces enclosing this Vertex.
double Vertex::get1RingSumAngles() {
	return 0.0f;
}

//! Compute (one-dimensional) index of this vertex within the smallest cube of an octree.
unsigned int Vertex::getOctreeIndex( Vector3D rCubeTopLeft, double rEdgeLen, unsigned int rXyzCubes ) {
	if( rXyzCubes & 0x1 ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] ERROR: expects an even number!\n";
		return 0;
	}
	Vector3D distToCenter = ( getPositionVector() - rCubeTopLeft ) / rEdgeLen;
	unsigned int octInd1D = static_cast<unsigned int>( ceil( distToCenter.getX() ) +
	                                                   ceil( distToCenter.getY() ) * rXyzCubes +
	                                                   ceil( distToCenter.getZ() ) * rXyzCubes * rXyzCubes );
	return octInd1D;
}

int Vertex::getType() {
	//! Return the type (=class) of this object as an id.
	return Primitive::IS_VERTEX;
}

//! Relates to Mesh::fetchSphereBitArray passes thru to neighbourfaces to reach the vertices in 1-ring distance.
Vertex* Vertex::advanceInSphere( [[maybe_unused]] double*        sphereCenter,        //!< X, y and z-coordinate of the sphere's center
                                 [[maybe_unused]] double         radius,              //!< Radius of the sphere.
                                 [[maybe_unused]] uint64_t* vertBitArrayVisited, //!< Bit array tagging visited vertices.
                                 [[maybe_unused]] set<Vertex*>*  nextArray,           //!< Vertices along the marching front.
                                 [[maybe_unused]] uint64_t* faceBitArrayVisited, //!< Bit array tagging visited faces.
                                 [[maybe_unused]] bool           rOrderToFuncVal,     //!< Flag for visual debuging: will set the faces function value to the sequenze nr.
                                 [[maybe_unused]] double*        rSeqNr               //!< Sequenze number - only used when rOrderToFuncVal is set.
	) {
	return nullptr;
}

//! Relates to Mesh::fetchSphereBitArray1R adds vertices and faces in 1-ring distance to the bit arrays.
bool Vertex::mark1RingVisited( [[maybe_unused]] uint64_t* rVertBitArrayVisited, //!< Bit array tagging visited vertices.
                               [[maybe_unused]] uint64_t* rFaceBitArrayVisited, //!< Bit array tagging visited faces.
                               [[maybe_unused]] bool           rOrderToFuncVal,      //!< Flag for visual debuging: will set the faces function value to the sequenze nr.
                               [[maybe_unused]] double*        rSeqNr                 //!< Sequenze number - only used when rOrderToFuncVal is set.
	) {
	return true;
}

// --- Function Values --------------------------------------------------------

bool Vertex::setFuncValue( double setVal ) {
	//! Method to set the function value.
	//! Returns false in case of an error (or not implemented).
	mFuncValue = setVal;
	return true;
}

bool Vertex::getFuncValue( double* rGetVal ) const {
	//! Method to retrieve the function value.
	//! Returns false in case of an error (or not implemented).
	*rGetVal = mFuncValue;
	return true;
}

//! Method to check the finite condition of the function value.
//!
//! @returns true, when the function value is normal, subnormal or zero, but not infinite or NaN.
bool Vertex::isFuncValFinite() const {
	return( isfinite( mFuncValue ) );
}

//! Checks if this Vertex is a local Minimum using the 1-ring
//! stored in mAdjacentFaces.
//! 
//! The according flag will be set.
bool Vertex::isFuncValLocalMinimum() {
	// This function requires faces. Therefore it is implemented in VertexOfFace.
	this->clearFlag( FLAG_LOCAL_MIN );
	return( false );
}

//! Checks if this Vertex is a local Maximumg using the 1-ring
//! stored in mAdjacentFaces.
//! 
//! The according flag will be set.
bool Vertex::isFuncValLocalMaximum() {
	// This function requires faces. Therefore it is implemented in VertexOfFace.
	this->clearFlag( FLAG_LOCAL_MAX );
	return( false );
}

//! Applies an 1-ring median filter to the function value.
//!
//! Implemented in VertexOfFace::funcValMedianOneRing
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::funcValMedianOneRing( [[maybe_unused]] double* rMedianValue , [[maybe_unused]] double rMinDist  ) {
	// This function requires faces. Therefore it is implemented in VertexOfFace.
	return false;
}

//! Applies an 1-ring mean filter to the function value.
//!
//! Implemented in VertexOfFace::funcValMeanOneRing
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::funcValMeanOneRing( [[maybe_unused]] double* rMeanValue , [[maybe_unused]] double rMinDist  ) {
	// This function requires faces. Therefore it is implemented in VertexOfFace.
	return false;
}


// --- Comparison of function value - Heap/sorting related -------------------

//! Used for sorting e.g. a heap.
//! @returns false, when one of the vertices has no function value.
bool Vertex::funcValDescendingLabel( Vertex* vert1, Vertex* vert2 ) {
	uint64_t labelID1=0;
	uint64_t labelID2=0;
	vert1->getLabel( labelID1 );
	vert2->getLabel( labelID2 );
	if( labelID1 > labelID2 ) {
		return false;
	}
	if( labelID1 < labelID2 ) {
		return true;
	}

	double funcVal1;
	if( !vert1->getFuncValue( &funcVal1 ) ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
		return false;
	}
	double funcVal2;
	if( !vert2->getFuncValue( &funcVal2 ) ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
		return false;
	}

	if( funcVal1 > funcVal2 ) {
		// when both short distances are of equal length, we have to sort by the longer distance:
		return true;
	}
	return false;
}

//! Used for sorting e.g. a heap.
//! @returns false, when one of the vertices has no function value.
bool Vertex::funcValDescending( Vertex* vert1, Vertex* vert2 ) {
	double funcVal1;
	if( !vert1->getFuncValue( &funcVal1 ) ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
		return false;
	}
	double funcVal2;
	if( !vert2->getFuncValue( &funcVal2 ) ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
		return false;
	}

	if( funcVal1 > funcVal2 ) {
		// when both short distances are of equal length, we have to sort by the longer distance:
		return true;
	}
	return false;
}

//! Used for sorting e.g. a heap.
//! @returns false, when one of the vertices has no function value.
bool Vertex::funcValLower( Vertex* vert1, Vertex* vert2 ) {
	double funcVal1;
	if( !vert1->getFuncValue( &funcVal1 ) ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
		return false;
	}
	double funcVal2;
	if( !vert2->getFuncValue( &funcVal2 ) ) {
		LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
		return false;
	}

	if( funcVal1 < funcVal2 ) {
		// when both short distances are of equal length, we have to sort by the longer distance:
		return true;
	}
	return false;
}

//! Used for sorting e.g. a heap.
//! Sorts only labled.
//! @returns true, when one of the vertices has no function value.
bool Vertex::funcValLowerLabled( Vertex* vert1, Vertex* vert2 ) {
	if( (vert1->isLabled()) && (!vert2->isLabled()) ){
		return true;
	}

	double funcVal1;
	if( !vert1->getFuncValue( &funcVal1 ) ) {
		LOG::debug() << "[Vertex::funcValLower] no function value!\n";
		return false;
	}
	double funcVal2;
	if( !vert2->getFuncValue( &funcVal2 ) ) {
		LOG::debug() << "[Vertex::funcValLower] no function value!\n";
		return false;
	}

	if( funcVal1 < funcVal2 ) {
		// when both short distances are of equal length, we have to sort by the longer distance:
		return true;
	}
	return false;
}

bool Vertex::sortLabelsFirst( Vertex* vert1, Vertex* vert2 ) {
	//! Used for sorting e.g. a heap.
	if( ( vert1->isLabled() ) && ( !(vert2->isLabled()) ) ) {
		return true;
	}
	return false;
}

bool Vertex::sortByIndex( Vertex* vert1, Vertex* vert2 ) {
	//! Used for sorting by Index.
	if( vert1->getIndex() < vert2->getIndex() ) {
		return true;
	}
	return false;
}

//! Used for sorting e.g. a heap.
//! @returns false, when one of the vertices has no rgb values.
bool Vertex::RGBLower( Vertex* vert1, Vertex* vert2 ) {
    unsigned char RGB1[3];
    if( !vert1->copyRGBTo( RGB1 ) ) {
        LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
        return false;
    }
    unsigned char RGB2[3];
    if( !vert2->copyRGBTo( RGB2 ) ) {
        LOG::debug() << "[Vertex::" << __FUNCTION__ << "] no function value!\n";
        return false;
    }

    if( RGB1[0] < RGB2[0] ) {
        // red value smaller
        return true;
    }
    else if( RGB1[0] == RGB2[0] ) {
        if( RGB1[1] < RGB2[1] ) {
            // red value equal, green value smaller
            return true;
        }
        else if( RGB1[1] == RGB2[1] ) {
            if( RGB1[2] < RGB2[2] ) {
                // red and green value equal, blue smaller
                return true;
            }
        }
    }
    // red or green is larger, or red and green are equal and blue is equal or larger
    return false;
}

// --- Transformation --------------------------------------------------------

//! Applies (multiplies) a given homogenous transformation matrix.
//! Returns false, when the application fails.
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::applyTransfrom( Matrix4D* transMat ) {
	Vector3D posVecNew = getPositionVector() * (*transMat);
	POS_X = posVecNew.getX();
	POS_Y = posVecNew.getY();
	POS_Z = posVecNew.getZ();
	Vector3D normalNew = getNormal( false )  * (*transMat);
	NORMAL_X = normalNew.getX();
	NORMAL_Y = normalNew.getY();
	NORMAL_Z = normalNew.getZ();
	return( true );
}

bool Vertex::applyMeltingSphere( double rRadius, double rRel ) {
	//! Applies melting with sqrt(r^2-x^2-y^2).
	//! Returns false, when the application fails, when
	//! the Vertex is outside the radius.
	double deltaZ = sqrt( rRadius*rRadius - POS_X*POS_X - POS_Y*POS_Y ) * rRel;
	if( std::isnan( deltaZ ) ) {
		POS_Z = -rRadius;
		return false;
	}
	POS_Z = POS_Z - rRadius + deltaZ;
	return true;
}

//! Move the vertex by the offset value
Vertex *Vertex::applyNormalShift(float offsetDistance, [[maybe_unused]] int index) {
    Vector3D normal = this->getNormal(true);
    Vector3D pos = this->getPositionVector();
    Vector3D newPos = pos + offsetDistance * normal;
    Vertex* ret = new Vertex(newPos);
    ret->setNormal(normal.getX(),normal.getY(),normal.getZ());
    return ret;
}

// --- Labeling ---------------------------------------------------------------------------------------------

//! Sets a label number of a primitve.
//!
//! Preserves the very first label set.
//!
//! @returns false, when the primitive could not be labled.
bool Vertex::setLabel( uint64_t rSetLabelNr ) {
	if( Primitive::setLabel( rSetLabelNr ) ) {
		mLabelNr = rSetLabelNr;
		return( true );
	}
	return( false );
}

//! Returns the label nr [1...
//! Note label nr. 0 means background!
//! When false, the returned labelNr is not changed.
//!
//! @returns false, when no label is set or in case of an error (or not implemented).
bool Vertex::getLabel( uint64_t& rGetLabelNr ) const {
	if( !isLabled() ) { // Checks flags for background and 'not labled'.
		return( false );
	}
	if( mLabelNr == 0 ) { // Check if the number corresponds to background - in case the appropriate flag is not set!
		return( false );
	}
	rGetLabelNr = mLabelNr;
	return( true );
}

// neighbourhood - related to labeling ------------------------------------------------------------

//! Retrieves all Vertices in 1-ring distance and adds them to the given list.
//! Will exclude the calling Vertex.
void Vertex::getNeighbourVertices( [[maybe_unused]] set<Vertex*>* someVertList  ) {
}

bool Vertex::getNeighbourVerticesExcluding( set<Vertex*>* neighboursSelected, set<Vertex*>* verticesToExclude ) {
	//! Returns all neighbouring Vertices not within verticesToExclude.
	//! \return True when a Vertex tagged as _PRIMITIVE_NO_LABEL_ was added. 
	//! This is an identifier for label border (not Mesh borders!)

	bool hitNoLabel = false;

	set<Vertex*> neighbourVertices;
	getNeighbourVertices( &neighbourVertices );

	set<Vertex*>::iterator itVertex;
	set<Vertex*>::iterator itVertexFound;

	for ( itVertex=neighbourVertices.begin(); itVertex!=neighbourVertices.end(); itVertex++ ) {
		if( verticesToExclude->find( (*itVertex) ) != verticesToExclude->end() ) {
			continue;
		}
		if( !(*itVertex)->isLabled() ) {
			hitNoLabel = true;
		}
		neighboursSelected->insert( (*itVertex) );
	}
	return hitNoLabel;
}

//! Adds the adjacent vertices of the 1-ring to rSomeVerts, when they have not been tagged in rVertBitArrayVisited.
//! Will also tag the vertices added.
bool Vertex::getAdjacentVerticesExcluding( [[maybe_unused]] vector<Vertex*>* rSomeVerts, [[maybe_unused]] uint64_t* rVertBitArrayVisited ) {
	return true;
}

//! Return the reference to next vertex on the border within the 1-ring neighbourhood.
//! Considers the bit-array for vertices tagged unvisited.
Vertex* Vertex::getAdjacentNextBorderVertex( [[maybe_unused]] uint64_t* rVertBitArrayUnVisited ) {
	return nullptr;
}

//! When one of the next Vertex within orientated 1-ring neighbourhood
//! is part of the list, the Vertex is returned - NULL otherwise.
Vertex* Vertex::getConnection( [[maybe_unused]] set<Vertex*>* someVertList, [[maybe_unused]] bool reverse ) {
	return nullptr;
}

// MESH-SETUP ------------------------------------------------------------------
/*
//! Add an adjacent Face.
void Vertex::connectToFace( Face* someFace ) {
}

//! Remove an adjacent Face (e.g. when removed).
void Vertex::disconnectFace( Face* someFace ) {
}
*/
//! Test if a given face is adjacent to this Vertex.
bool Vertex::isAdjacent( [[maybe_unused]] Face* someFace ) {
	return false;
}

//! Typically called when a Face is initalized. Adds all Faces
//! having this Vertex and the otherVert, but is not the
//! calling Face.
void Vertex::getFaces( [[maybe_unused]] Vertex* otherVert, [[maybe_unused]] set<Face*>* neighbourFaces, [[maybe_unused]] Face* callingFace ) {
}

//! Adds all adjacent Faces of this Vertex to the given list/set of Faces.
//! Because we use a set, there will be NO duplicate faces.
//!
//! Typically called by Mesh::removeVertices()
void Vertex::getFaces( [[maybe_unused]] set<Face*>* someFaceList ) {
}

//! Adds all adjacent Faces of this Vertex to the given list/set of Faces.
//! Because we use a vector, there will can be duplicate faces, which is a rather odd case.
void Vertex::getFaces( [[maybe_unused]] vector<Face*>* someFaceList ) {
}

// Mesh checking ----------------------------------------------------------------------------

//! For sanity checks
bool Vertex::belongsToFace() {
	return false;
}

//! Returns the number of faces connected/related to this Vertex.
int Vertex::connectedToFacesCount() {
	return 0;
}

//! Checks if any of the coordinates is not a number.
//! @returns true, when NaN. False otherwise.
bool Vertex::isNotANumber() {
	bool retVal = false;
	retVal |= std::isnan( mPosition[0] );
	retVal |= std::isnan( mPosition[1] );
	retVal |= std::isnan( mPosition[2] );
	return retVal;
}

//! Returns true, when related/connected to ZERO faces and ZERO edges.
bool Vertex::isSolo() {
	return true;
}

//! Returns true, when the Vertex is part of one or more border edges and
//! therefore on the border of a mesh.
//!
//! Solo vertices are NOT considered as BORDER vertices.
bool Vertex::isBorder() {
	return false;
}

//! Returns true, when the Vertex is part of manifold faces (only).
bool Vertex::isManifold() {
	return not( isNonManifold() );
}

//! Returns true, when the Vertex is part of one or more non-manifold
//! edges/faces.
bool Vertex::isNonManifold() {
	return false;
}

//! Determines if this vertex is the only connection between different parts
//! of the mesh. This (rare) connection is kind of generic double cone (in
//! respec to the meshs graph not actual geometry).
//! It is kind of a non-manifold construct with a single Vertex instead of
//! and Edge.
bool Vertex::isDoubleCone() {
	return false;
}

//! Determines if this vertex belongs to at least one face
//! having an area of zero.
bool Vertex::isPartOfZeroFace() {
	return( false );
}

//! Determines if this vertex belongs to an edge connecting
//! faces with improper orientation.
bool Vertex::isInverse() {
	return( false );
}

//! Returns the state of a vertex.
int Vertex::getState() {
	if( isSolo() ) {
		return  _PRIMITIVE_STATE_SOLO_;
	}
	if( isBorder() ) {
		return  _PRIMITIVE_STATE_BORDER_;
	}
	if( isManifold() ) {
		return  _PRIMITIVE_STATE_MANIFOLD_;
	}
	if( isNonManifold() ) {
		return  _PRIMITIVE_STATE_NON_MANIFOLD_;
	}
	// we should never reach this point:
// 	LOG::debug() << "[Edge] ERROR - negative value for size (" << faceList.size() << ").\n";
	return _PRIMITIVE_STATE_ERROR_;
}

// Distances  -------------------------------------------------------------------------------------------

double Vertex::estDistanceTo( Vertex* vert ) {
	//! Estimate the distance to a given vertex.
	//! \todo test this function.

	double dX, dY, dZ, len;

	dX = POS_X - vert->getX();
	dY = POS_Y - vert->getY();
	dZ = POS_Z - vert->getZ();
	len = sqrt( dX*dX + dY*dY + dZ*dZ );

	return len;
}

double Vertex::estDistanceTo( double x, double y, double z ) {
	//! Estimate the distance to the given coordinates.

	double dX, dY, dZ, len;

	dX = POS_X - x;
	dY = POS_Y - y;
	dZ = POS_Z - z;
	len = sqrt( dX*dX + dY*dY + dZ*dZ );

	return len;
}

//! Compute the distance of this Vertex to a line given by two other vertices.
//! @returns distance to the given line.
double Vertex::estDistanceToLine(
	const Vertex* rPosTip,     //!< Top vertex    ("Spitze")
	const Vertex* rPosBottom   //!< Bottom vertex ("Schaft")
) {
	Vector3D posVec;
	Vector3D dirVec;
	rPosBottom->getPositionVector( &posVec );  // "Schaft"
	rPosTip->getPositionVector( &dirVec );     // "Spitze"
	dirVec -= posVec;                          // "Spitze minus Schaft."
	return( this->estDistanceToLineDir( &posVec, &dirVec ) );
}

//! Compute the distance of this Vertex to a line given by a position rPos and a direction rDir.
//! @returns distance to the given line.
double Vertex::estDistanceToLineDir(
	const Vector3D* rPos,  //!< Position vector.
	const Vector3D* rDir   //!< Direction vector.
) {
	Vector3D posVert = getPositionVector();
	double lengthDirVec = rDir->getLength3();
	double dist = ( ( posVert - (*rPos) ) % (*rDir) ).getLength3() / lengthDirVec;
	return dist;
}

double Vertex::estDistanceToPlane( double* planeHNF, bool absDist ) {
	//! Estimate the distance to the given plane in Hesse form as double[4].
	//! \todo check for NULL pointer and |(a,b,c)^T|>0
	//! \todo eventually make more performant using a normalized normal |(a,b,c)^T|=1.0
	double nom,denom,dist;

	nom   = planeHNF[0]*POS_X + planeHNF[1]*POS_Y + planeHNF[2]*POS_Z + planeHNF[3];
	denom = sqrt( pow(planeHNF[0],2) + pow(planeHNF[1],2) + pow(planeHNF[2],2) );
	if( absDist ) {
		dist = abs( nom/denom );
	} else {
		dist = nom/denom;
	}

	return dist;
}

//! Estimates distance of vertex to a plane given in Hessian normal form (the last component
//! of the HNF is contained within the homogenous coordinate of the vector).
double Vertex::estDistanceToPlane(
		const Vector3D& rPlaneHNF,
		bool            rAbsDist
) const {
	double dist = dot3( rPlaneHNF, this->getPositionVector() )
	              + rPlaneHNF.getH();

	if( rAbsDist ) {
		return( abs( dist ) );
	}
	// Otherwise:
	return( dist );
}

//! Estimates distance to a cone given by an axis and two radii -- an upper radius
//! and a lower radius.
//! @warning This function assumes that the direction vector (axisTop - axisBot)
//! is pointing _away_ from the cone tip. The calling function has to ensure that
//! this is the case.
double Vertex::estDistanceToCone(Vector3D* axisTop,
                                 Vector3D* axisBot,
                                 Vector3D* coneTip,
                                 const double& semiAngle,
                                 const double& coneHeight,
                                 const double& upperRadius,
                                 const double& lowerRadius,
                                 bool absDist) {

	if(!axisTop || !axisBot || !coneTip) {
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] Called without axis parameters -- unable to continue\n";
		return(0.0);
	}

	if(upperRadius == lowerRadius) {
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] Unable to handle degenerate cone (upperRadius == lowerRadius)\n";
		return(0.0);
	}

	Vector3D axisVector = *axisTop - *axisBot;

	// Check angle of both connection vectors between the current vertex and the
	// lower/upper point of the _truncated_ cone with respect to the cone axis.
	// The lower angle must not be larger than M_PI/2, whereas the upper angle
	// must not be smaller than M_PI/2. Else, the vertex is situated above/below
	// the truncated cone and needs to be handled differently

	Vector3D topToVertex    = this->getPositionVector() - axisTop;
	Vector3D bottomToVertex = this->getPositionVector() - axisBot;
	Vector3D tipToVertex   = this->getPositionVector() - coneTip;

	double upperAngle = angle(topToVertex, axisVector);
	double lowerAngle = angle(bottomToVertex, axisVector);
	double tipAngle   = angle(tipToVertex, axisVector);

	if(upperAngle >= M_PI/2 && lowerAngle <= M_PI/2) {
		double distSigned = coneHeight*sin(tipAngle - semiAngle);
		return(absDist ? fabs(distSigned) : distSigned);
	}
	else {
		return(std::numeric_limits<double>::quiet_NaN());
	}
}

//! Estimates distance to a sphere given by a center and a radius.
double Vertex::estDistanceToSphere(Vector3D* center, const double& radius, bool absDist) {

	if(!center) {
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] Called without sphere center -- unable to continue\n";
		return(0.0);
	}

	if(radius == 0) { // TODO: entferne Nullckeck !!!!!!!!
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] Unable to handle degenerate sphere (radius == 0)\n";
		return(0.0);
	}

	double centerToVertex = (this->getPositionVector() - center).getLength3();

	double distSigned = centerToVertex - radius;
	 return(absDist ? fabs(distSigned) : distSigned);

}

// Angles ---------------------------------------------------------------------

//! Computes the angle between vertex normal and projection vector of vertex onto cone axis.
//! @returns unsigned angle in radian.
double Vertex::getAngleToRadial( const Vector3D &rAxisTop, const Vector3D &rAxisBottom ) {
	Vector3D point( getX(), getY(), getZ() );
	Vector3D pointProjected = rAxisTop + ( point - rAxisTop ).projectOnto( rAxisBottom - rAxisTop );
	Vector3D normalVector( getNormalX(), getNormalY(), getNormalZ() );
	double angleResult = angle( point - pointProjected, normalVector );
	return angleResult;
}

//! Computes the angle between vertex normal and projection vector of vertex onto cone axis with vertex normal projected onto cone axis.
//! @returns unsigned angle in radian.
double Vertex::getAxisAngleToRadial( const Vector3D &rAxisTop, const Vector3D &rAxisBottom ) {
	Vector3D point( getX(), getY(), getZ() );
	Vector3D pointProjected = rAxisTop + ( point - rAxisTop ).projectOnto( rAxisBottom - rAxisTop );
	Vector3D normalVector( getNormalX(), getNormalY(), getNormalZ() );
	Vector3D normalAlongAxis = normalVector.projectOnto( rAxisBottom - rAxisTop );
	Vector3D normalAlongProjection = normalVector.projectOnto( point - pointProjected );
	double angleResult = angle( normalAlongAxis + normalAlongProjection, point - pointProjected );
	return angleResult;
}

//! Computes the angle between vertex normal and projection vector of vertex onto cone axis with vertex normal projected onto plane orthogonal to cone axis.
//! @returns unsigned angle in radian.
double Vertex::getOrthogonalAxisAngleToRadial( const Vector3D &rAxisTop, const Vector3D &rAxisBottom ) {
	Vector3D point( getX(), getY(), getZ() );
	Vector3D pointProjected = rAxisTop + ( point - rAxisTop ).projectOnto( rAxisBottom - rAxisTop );
	Vector3D normalVector( getNormalX(), getNormalY(), getNormalZ() );
	Vector3D normalAlongAxis = normalVector.projectOnto( rAxisBottom - rAxisTop );
	Vector3D normalInPlane = normalVector - normalAlongAxis;
	double angleResult = angle( point - pointProjected, normalInPlane );
	return angleResult;
}

// Geodesics ------------------------------------------------------------------

//! Initalize a marching front and visited faces for geodesic distance estimation with this Vertex as seed.
//! Hint: 1-ring
//! @returns false in case of an error.
bool Vertex::getGeodesicEdgesSeed( deque<EdgeGeodesic*>* frontEdges, map<Vertex*,GeodEntry*>* geoDistList, uint64_t* faceBitArray, [[maybe_unused]] int faceNrBlocks, bool weightFuncVal ) {

	// Check for invalid settings:
	if( ( frontEdges == nullptr ) || ( geoDistList == nullptr ) ) {
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] frontEdges and/or geoDistList is/are NULL!\n";
		return false;
	}

	// Adjacent faces:
	set<Face*> adjacentFaces;
	getFaces( &adjacentFaces );

	if( adjacentFaces.size() <= 0 ) {
		// Solo vertex. Therefore have noting to do and
		return true;
	}

	// Insert this vertex, with distance zero:
	geoDistList->insert( pair<Vertex*,GeodEntry*>( this, new GeodEntry( 0.0, _NOT_A_NUMBER_DBL_, this ) ) );

	// Reference vector for the geodesic angle
	Vector3D refVecAngle;
	Vector3D refNormal = getNormal();
	{
		Face* currFace = (*(adjacentFaces.begin()));
		Vertex* vertA;
		Vertex* vertB;
		Face::eEdgeNames edgeIndex;
		if( !currFace->getOposingEdgeAndVertices( this, &edgeIndex, &vertA, &vertB ) ) {
			LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Unexpected error #1!\n";
			return false;
		}
		refVecAngle = ( vertA->getPositionVector() - getPositionVector() ) + ( vertB->getPositionVector() - getPositionVector() );
		if( refVecAngle.getLength3() <= 0.0 ) {
			LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Unexpected error #2!\n";
			return false;
		}
	}

	// Adjacent vertices + mark faces visited:
	uint64_t bitOffset;
	uint64_t bitNr;
	set<Face*>::iterator itFace;
	for( itFace=adjacentFaces.begin(); itFace!=adjacentFaces.end(); itFace++ ) {
		Face* currFace = (*itFace);
		// Fetch the 1-ring vertices and their edge from the face:
		Vertex* vertA;
		Vertex* vertB;
		Face::eEdgeNames edgeIndex;
		if( !currFace->getOposingEdgeAndVertices( this, &edgeIndex, &vertA, &vertB ) ) {
			LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Unexpected error #3!\n";
			return false;
		}

		// Check and eventually insert Vertex A
		map<Vertex*,GeodEntry*>::iterator itVertex = geoDistList->find( vertA );
		GeodEntry* geodARef( nullptr );
		if( itVertex == geoDistList->end() ) {
			// Vertex not in list
			Vector3D geodVecA = vertA->getPositionVector() - getPositionVector();
			double geodDistA = geodVecA.getLength3();
			double geodAngleA =  angle( geodVecA, refVecAngle, refNormal );
			if( weightFuncVal ) {
				double funcValA;
				if( !vertA->getFuncValue( &funcValA ) ) {
					LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: fetching function value for A!\n";
					return false;
				}
				Vector3D posC = getPositionVector() + getNormal( true ) * mFuncValue;
				Vector3D posA = vertA->getPositionVector() + vertA->getNormal( true ) * funcValA;
				geodDistA = abs3( posC-posA );
				// remove offset:
				//funcValA -= mFuncValue;
				//funcValA *= 1.0;
				// weight distance:
				//geodDistA = sqrt( geodDistA*geodDistA + funcValA*funcValA );
			}
			geodARef = new GeodEntry( geodDistA, geodAngleA, this );
			//if( (*geodARef) <= 0.0 ) {
			//	cout << "[Vertex::" << __FUNCTION__ << "] Warning: bad geodARef " << *geodARef << endl;
			//}
			geoDistList->insert( pair<Vertex*,GeodEntry*>( vertA, geodARef ) );
			//cout << "[Vertex::" << __FUNCTION__ << "] geodDistA " << geodDistA << endl;
		} else {
			// Vertex already in list
			geodARef = (*itVertex).second;
		}

		// Check and eventually insert Vertex B
		itVertex = geoDistList->find( vertB );
		GeodEntry* geodBRef( nullptr );
		if( itVertex == geoDistList->end() ) {
			// Vertex not in list
			Vector3D geodVecB = vertB->getPositionVector() - getPositionVector();
			double geodDistB = geodVecB.getLength3();
			double geodAngleB = angle( geodVecB, refVecAngle, refNormal );
			if( weightFuncVal ) {
				double funcValB;
				if( !vertB->getFuncValue( &funcValB ) ) {
					LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: fetching function value for B!\n";
					return false;
				}
				Vector3D posC = getPositionVector() + getNormal( true ) * mFuncValue;
				Vector3D posB = vertB->getPositionVector() + vertB->getNormal( true ) * funcValB;
				geodDistB = abs3( posC-posB );
				// remove offset:
				//funcValB -= mFuncValue;
				//funcValB *= 1.0;
				// weight distance:
				//geodDistB = sqrt( geodDistB*geodDistB + funcValB*funcValB );
			}
			geodBRef = new GeodEntry( geodDistB, geodAngleB, this );
			//if( (*geodBRef) <= 0.0 ) {
			//	cout << "[Vertex::" << __FUNCTION__ << "] Warning: bad geodBRef " << *geodBRef << endl;
			//}
			geoDistList->insert( pair<Vertex*,GeodEntry*>( vertB, geodBRef ) );
			//cout << "[Vertex::" << __FUNCTION__ << "] geodDistB " << geodDistB << endl;
		} else {
			// Vertex already in list
			geodBRef = (*itVertex).second;
		}

		// Create new edge ...
		EdgeGeodesic* edgeAB = new EdgeGeodesic( currFace, edgeIndex, geodARef, geodBRef );
		// .. add to the heap:
		frontEdges->push_back( edgeAB );
		push_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );

		// Mark visited:
		currFace->getIndexOffsetBit( &bitOffset, &bitNr );
		faceBitArray[bitOffset] |= (static_cast<uint64_t>(1) << bitNr);
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Feature vector functions --------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Assign a feature vectore stored in an external table.
//! @returns false in case of an error (e.g. another vector already attached). True otherwise.
bool Vertex::assignFeatureVec(const double*         rAttachFeatureVec,
        unsigned int rSetFeatureVecLen
) {
	if( ( rAttachFeatureVec == nullptr ) && ( rSetFeatureVecLen > 0 ) ) {
		// No pointer => problem.
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: NULL pointer given, while length > 0!\n";
		return false;
	}
	// Erase existing vector:
	if( mFeatureVec != nullptr ) {
		delete[] mFeatureVec;
		mFeatureVec = nullptr;
		mFeatureVecLen = 0;
	}
	// Nothing to do - empty vector.
	if( rSetFeatureVecLen <= 0 ) {
		return true;
	}
	// Create and copy vector:
	mFeatureVecLen = rSetFeatureVecLen;
	mFeatureVec    = new double[mFeatureVecLen];
	// cout << "[Vertex::" << __FUNCTION__ << "] assign FV:";
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		mFeatureVec[i] = rAttachFeatureVec[i];
		// cout << " " << mFeatureVec[i];
	}
	// cout << " to: " << mIdxOri << endl;
	return true;
}


bool Vertex::assignFeatureVec(const std::initializer_list<double> featureVectorValues)
{

	// Erase existing vector:

	if(mFeatureVec != nullptr)
	{
		delete[] mFeatureVec;
		mFeatureVec = nullptr;
		mFeatureVecLen = 0;
	}

	// Nothing to do - empty vector.

	if(featureVectorValues.size() == 0)
	{
		return true;
	}

	// Create and copy vector:

	mFeatureVecLen = static_cast<unsigned int>(featureVectorValues.size());
	mFeatureVec    = new double[mFeatureVecLen];

	if(mFeatureVec != nullptr)
	{
		size_t featureVectorElementCount = 0;
		
		for(const double featureVectorValue : featureVectorValues)
		{
			mFeatureVec[featureVectorElementCount++] = featureVectorValue;
		}
	}

	return true;
}

//! Copies the feature vector to a given array, which has to be of proper length!
//! @returns false in case of an error. True otherwise.
bool Vertex::copyFeatureVecTo( double* rFetchFeatureVec ) const {
	if(rFetchFeatureVec && mFeatureVec)
	{
		memcpy( rFetchFeatureVec, mFeatureVec, mFeatureVecLen*sizeof(double) );
		return true;
	}
	
	else
	{
		return false;
	}
}

//! Write new values for the feature vector's elements.
//!
//! The number of values has to be the same for the given/new vector
//! and the old/existing vector.
//!
//! A feature vector has to be attached (previously).
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::assignFeatureVecValues( const vector<double>& newFeatureVec ) {
	if( mFeatureVec == nullptr ) {
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Can not write to feature vector - no memory resereved!\n";
		return( false );
	}
	if( newFeatureVec.size() != mFeatureVecLen ) {
		LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Feature vector length do not match!\n";
		return( false );
	}
	for( uint64_t i=0; i<mFeatureVecLen; i++ ) {
		mFeatureVec[i] = newFeatureVec.at( i );
	}
	return( true );
}

//! Appends the feature vector's elements to a given vector!
//! @returns false in case of an error. True otherwise.
bool Vertex::getFeatureVectorElements( vector<double>& rFeatVec ) const {
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		rFeatVec.push_back( mFeatureVec[i] );
	}
	return( true );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Norm of the feature vector ------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Compute the mean value and the standard deviation of the 
//! feature vector's elements.
//! 
//! @returns false in case of an error e.g. no feature vector.
//!          True otherwise
bool Vertex::getFeatureVecMeanStdDev( 
        double* rFeatureVecMean, 
        double* rFeatureVecStdDev 
) const {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) ) {
		return( false );
	}

	// Compute mean
	double elementValueMean = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		elementValueMean += mFeatureVec[i];
	}
	elementValueMean /= mFeatureVecLen;

	// Compute standard deviation
	double elementValueStd = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		elementValueStd += pow( mFeatureVec[i] - elementValueMean, 2.0 );
	}
	elementValueMean = sqrt( elementValueMean / ( mFeatureVecLen-1 ) );

	// Done - return values.
	(*rFeatureVecMean)   = elementValueMean;
	(*rFeatureVecStdDev) = elementValueMean;
	return( true );
}

//! Compute the length of the feature vector using the manhatten norm.
//! I.e. 1-Norm
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecLenMan( double* rFeatureVecLenMan ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) ) {
		return false;
	}
	(*rFeatureVecLenMan) = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		(*rFeatureVecLenMan) += mFeatureVec[i];
	}
	return true;
}

//! Compute the length of the feature vector using the euclidean norm.
//! I.e. 2-Norm
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecLenEuc( double* rFeatureVecLenEuc ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) ) {
		return false;
	}
	(*rFeatureVecLenEuc) = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		(*rFeatureVecLenEuc) += mFeatureVec[i] * mFeatureVec[i];
	}
	(*rFeatureVecLenEuc) = sqrt( (*rFeatureVecLenEuc) );
	return true;
}

//! Compute the minimum element of the feature vector.
//!
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecMin( double* rFeatureVecMin ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) || ( rFeatureVecMin == nullptr ) ) {
		return( false );
	}
	double minValue = +_INFINITE_DBL_;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		if( minValue > mFeatureVec[i] ) {
			minValue = mFeatureVec[i];
		}
	}
	(*rFeatureVecMin) = minValue;
	return( true );
}

//! Compute the maximum element of the feature vector.
//! This is not the infinity-Norm or Maximum Norm as this requires the use of absoulut values.
//!
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecMax( double* rFeatureVecMax ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) || ( rFeatureVecMax == nullptr ) ) {
		return( false );
	}
	double maxValue = -_INFINITE_DBL_;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		if( maxValue < mFeatureVec[i] ) {
			maxValue = mFeatureVec[i];
		}
	}
	(*rFeatureVecMax) = maxValue;
	return( true );
}

//! Compute the minimal positive or negative element of the feature vector.
//!
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecMinSigned( double* rFeatureVecMinSigned ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) || ( rFeatureVecMinSigned == nullptr ) ) {
		return( false );
	}
	double minValue = +_INFINITE_DBL_;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		if( abs( minValue ) > abs( mFeatureVec[i] ) ) {
			minValue = mFeatureVec[i];
		}
	}
	(*rFeatureVecMinSigned) = minValue;
	return( true );
}

//! Compute the maximum positive or negative element of the feature vector.
//!
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecMaxSigned( double* rFeatureVecMaxSigned ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) || ( rFeatureVecMaxSigned == nullptr ) ) {
		return( false );
	}
	double maxValue = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		if( abs( maxValue ) < abs( mFeatureVec[i] ) ) {
			maxValue = mFeatureVec[i];
		}
	}
	(*rFeatureVecMaxSigned) = maxValue;
	return true;
}

//! Compute Bounded variation (BV) of a function with one variable.
//! See: https://en.wikipedia.org/wiki/Bounded_variation#BV_functions_of_one_variable
//!      https://de.wikipedia.org/wiki/Beschr%C3%A4nkte_Variation#Definition
//! Attention: this function is modified!
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecBVFunc( double* rFeatureVecBVFunc ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) ) {
		return false;
	}

	double maxVal = 0.0;
	for( int i=mFeatureVecLen-1; i>0; i-- ) {
		maxVal = max( abs( mFeatureVec[i] - mFeatureVec[i-1] ), maxVal );
	}
	//(*rFeatureVecBVFunc) = maxVal; // ORIGINAL BV - not used as the following presents more interesting results in combintaion with volume MSII feature vectors.
	(*rFeatureVecBVFunc) = maxVal + mFeatureVec[mFeatureVecLen-1]; // MODIFIED BV !!!
	//(*rFeatureVecBVFunc) = maxVal + abs( maxVal + mFeatureVec[mFeatureVecLen-1] ); // Modified BV with less interesting results

	return true;
}

//! Compute Total Variation (TV) of sequences of bounded variation.
//! See: https://en.wikipedia.org/wiki/Bounded_variation#bv_sequences
//!      https://de.wikipedia.org/wiki/Norm_%28Mathematik%29#BV-Norm_2
//! Attention: this function is modified!
//! @returns false in case of an error i.e. no feature vector present. True otherwise.
bool Vertex::getFeatureVecTVSeqn(double* rFeatureVecTVSeqn ) {
	// Sanity check
	if( ( mFeatureVecLen == 0 ) || ( mFeatureVec == nullptr ) ) {
		return false;
	}

	//(*rFeatureVecTVSeqn) = 0.0; // Variant without the first element.
	//(*rFeatureVecTVSeqn) = abs( mFeatureVec[0] ); // |a_1| - assuming that the feature vectors first element is the first of a seqeunce
	//(*rFeatureVecTVSeqn) = abs( mFeatureVec[mFeatureVecLen-1] ); // |a_1| - as the feature vector typically contains large scales first (inverted sequence)
	(*rFeatureVecTVSeqn) =  mFeatureVec[mFeatureVecLen-1]; // MODIFICATION: a_1 - as the feature vector typically contains large scales first (inverted sequence)
	for( int i=mFeatureVecLen-1; i>0; i-- ) {
		(*rFeatureVecTVSeqn) += abs( mFeatureVec[i] - mFeatureVec[i-1] );
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Norm of the feature vector with a given reference vector ------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Manhattan distance between this and the given feature vector.
//! I.e. 1-norm
//! @returns Manhattan distance
double Vertex::getFeatureDistManTo( double* rSomeFeatureVec ) {
	double dist = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		dist += abs( rSomeFeatureVec[i] - mFeatureVec[i] );
	}
	return dist;
}

//! Compute the euclidean distance between the vertex' feature vector and the given feature vector.
//! I.e. 2-norm
//! @returns Euclidean distance.
double Vertex::getFeatureDistEucTo( double* rSomeFeatureVec ) {
	double dist = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		dist += pow( rSomeFeatureVec[i] - mFeatureVec[i], 2.0 );
	}
	dist = sqrt( dist );
	return dist;
}

//! Compute the normalized euclidean distance between the vertex' feature vector and the given feature vector.
//! See https://en.wikipedia.org/wiki/Mahalanobis_distance#Definition_and_properties
//! I.e. Mahalanobis distance with a diagonal covariance matrix.
//! @returns Normalized euclidean distance.
double Vertex::getFeatureDistEucNormTo( double* rSomeFeatureVec, vector<double>* rFeatureVecStdDev ) {
	double dist = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		dist += pow( rSomeFeatureVec[i] - mFeatureVec[i], 2.0 ) / pow( (*rFeatureVecStdDev)[i], 2.0 );
	}
	dist = sqrt( dist );
	return dist;
}

//! Compute Cosine Similarity to a given feature vector.
//! See: https://en.wikipedia.org/wiki/Cosine_similarity
//! @returns either the cosine or the arccosine depending on rApplyACos.
//! \todo Add check or warning for vectors having a euclidean length of ZERO!
double Vertex::getFeatureVecCosSim( double* rSomeFeatureVec, //!< Reference feature vector.
                                    bool    rApplyACos       //!< If true the arccosine is returned. Otherwise the cosine is returned.
                                  ) {
	double nom    = 0.0;
	double denomA = 0.0;
	double denomB = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		nom    += rSomeFeatureVec[i] * mFeatureVec[i];
		denomA += rSomeFeatureVec[i] * rSomeFeatureVec[i];
		denomB += mFeatureVec[i] * mFeatureVec[i];
	}
	double dist = nom / ( sqrt( denomA ) * sqrt( denomB ) );
	if( rApplyACos ) {
		dist = acos( dist );
	}
	//cout << "[Mesh::" << __FUNCTION__ << "] Returns: " << dist << endl;
	return dist;
}

//! Compute Cosine Similarity to a given feature vector.
//! See: https://en.wikipedia.org/wiki/Cosine_similarity#Confusion_with_.22Tanimoto.22_coefficient
//! @returns the Tanimoto distance.
//! \todo Add check or warning for vectors having a euclidean length of ZERO!
double Vertex::getFeatureVecTanimotoDist( double* rSomeFeatureVec //!< Reference feature vector.
                                        ) {
	double nom    = 0.0;
	double denomA = 0.0;
	double denomB = 0.0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		nom    += rSomeFeatureVec[i] * mFeatureVec[i];
		denomA += rSomeFeatureVec[i] * rSomeFeatureVec[i];
		denomB += mFeatureVec[i] * mFeatureVec[i];
	}
	double dist = nom / ( sqrt( denomA ) + sqrt( denomB ) - nom );
	//cout << "[Mesh::" << __FUNCTION__ << "] Returns: " << dist << endl;
	return dist;
}

//! Compute p-Norm to given reference (feature) vector.
//!
//! Optional: reference, weight and offset.
//! Allowed optional vectors can be set to NULL, size()==0 and size()=='length of the feature vector'.
//! Only in the latter case the vector's element values are taken into account.
//!
//! rPNorm also accepts infinite as value and will compute the maximum-norm.
//! See: https://en.wikipedia.org/wiki/Lp_space#The_p-norm_in_finite_dimensions
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::getFeatureVecPNorm(
                double* rResult,                     //!< reference to the result
                double  rPNorm,                      //!< p-Norm, infinity and zero allowed. Negative values will throw an error.
                const vector<double>* rReferenceVec, //!< optional given reference vector - when NULL is given the length of the feature vector is computed
                const vector<double>* rOffsetVec,    //!< optional offset vector
                const vector<double>* rWeightNum,    //!< optional weight vector as numerator (Zähler)
                const vector<double>* rWeightDenom   //!< optional weight vector as denominator (Nenner)
) {
	// Sanity and range checks
	if( rResult == nullptr ) {
		return false;
	}
	if( rPNorm < 0.0 ) {
		return false;
	}
	if( ( rReferenceVec != nullptr )
	    && ( rReferenceVec->size() != 0 )
	    && ( rReferenceVec->size() != mFeatureVecLen ) ) {
		return false;
	}
	if( ( rOffsetVec != nullptr )
	    && ( rOffsetVec->size() != 0 )
	    && ( rOffsetVec->size() != mFeatureVecLen ) ) {
		return false;
	}
	if( ( rWeightNum != nullptr )
	    && ( rWeightNum->size() != 0 )
	    && ( rWeightNum->size() != mFeatureVecLen ) ) {
		return false;
	}
	if( ( rWeightDenom != nullptr )
	    && ( rWeightDenom->size() != 0 )
	    && ( rWeightDenom->size() != mFeatureVecLen ) ) {
		return false;
	}

	bool retVal = true;
	bool maxNorm = ( rPNorm == std::numeric_limits<double>::infinity() ); // maximum norm
	bool nonZeroCount = ( rPNorm == 0.0 );                                // non-zero counting "norm"
	double sumElements = 0.0;

	// Compute p-Norm including the non-zero counting "norm"
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		double currElement = mFeatureVec[i];
		if( ( rReferenceVec != nullptr ) && ( rReferenceVec->size() == mFeatureVecLen ) ){
			currElement -= rReferenceVec->at( i );
		}
		if( ( rOffsetVec != nullptr ) && ( rOffsetVec->size() == mFeatureVecLen ) ) {
			currElement -= rOffsetVec->at( i );
		}
		if( ( rWeightNum != nullptr ) && ( rWeightNum->size() == mFeatureVecLen ) ) {
			currElement *= rWeightNum->at( i );
		}
		if( ( rWeightDenom != nullptr ) && ( rWeightDenom->size() == mFeatureVecLen ) ) {
			currElement /= rWeightDenom->at( i );
		}
		currElement = abs( currElement );
		if( maxNorm ) {
			sumElements = max( sumElements, currElement );
		} else if( nonZeroCount && ( currElement != 0.0 ) ) {
			sumElements += 1.0;
		} else {
			sumElements += pow( currElement, rPNorm );
		}
	}

	// Return
	if( maxNorm || nonZeroCount ) {
		(*rResult) = sumElements;
	} else {
		(*rResult) = pow( sumElements, 1.0/rPNorm );
	}
	return retVal;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Other feature vector related functions ------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Retrieves an elemment of the feature vector.
//! Index here starts at 0 (ZERO!)
//! Maximum is stored in Vertex::mFeatureVecLen
//! In case of an error the returned value is set to NaN.
//!
//! @returns false in case of an error e.g. elementNr > featureVecLen. True otherwise.
bool Vertex::getFeatureElement( unsigned int rElementNr, double* rElementValue ) {
	// Sanity checks

	if( rElementValue == nullptr )
	{
		return false;
	}

	if(  mFeatureVec == nullptr  ) {

		(*rElementValue) = _NOT_A_NUMBER_DBL_;
		return false;
	}
	if( rElementNr >= mFeatureVecLen ) {
		(*rElementValue) = _NOT_A_NUMBER_DBL_;
		return false;
	}
	// Return the requested element.
	(*rElementValue) = mFeatureVec[rElementNr];
	return true;
}

//! Sets an element of the feature vector to value.
//! @param elementNr the element to be set
//! @param value the value to set the element to
//! @returns false, if elementNr is out of range
bool Vertex::setFeatureElement(unsigned int elementNr, double value)
{
	if(elementNr >= mFeatureVecLen)
		return false;

	mFeatureVec[elementNr] = value;
	return true;
}

//! @returns the length of the feature vector.
unsigned int Vertex::getFeatureVectorLen() {
	return mFeatureVecLen;
}

int Vertex::cutOffFeatureElements( double rMinVal, double rMaxVal, bool rSetToNotANumber ) {
	//! Removes feature elements. This is done either by setting values
	//! larger than maxVal to maxVal or to not-a-number. Analog: minVal.
	//! Purpose: to remove/discard outliers.
	//!
	//! Returns negative value in case of an error.
	//! Otherwise: returns the number of elements changed.
	int elementsChanged = 0;
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		if( mFeatureVec[i] < rMinVal ) {
			if( rSetToNotANumber ) {
				mFeatureVec[i] = _NOT_A_NUMBER_DBL_;
			} else {
				mFeatureVec[i] = rMinVal;
			}
			elementsChanged++;
		}
		if( mFeatureVec[i] > rMaxVal ) {
			if( rSetToNotANumber ) {
				mFeatureVec[i] = _NOT_A_NUMBER_DBL_;
			} else {
				mFeatureVec[i] = rMaxVal;
			}
			elementsChanged++;
		}
	}
	return elementsChanged;
}

//! resizes the feature-vector of the vertex to fit size
//! @param size the new size. If size is smaller than the current size, elements get cut. Otherwise, the vector is padded with zeros
void Vertex::resizeFeatureVector(unsigned int size)
{
	if(size == mFeatureVecLen)
		return;

	double* oldVec = mFeatureVec;
	unsigned int oldlen = mFeatureVecLen;
	mFeatureVec = new double[size];
	mFeatureVecLen = size;

	//copy data
	for(unsigned int i = 0; i<mFeatureVecLen; ++i)
	{
		mFeatureVec[i] = i < oldlen ? oldVec[i] : _NOT_A_NUMBER_DBL_;
	}

	delete[] oldVec;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Feature vector smoothing --------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Median value for each element of the feature vector's elements.
//!
//! For a solo vertex the feature vector is returned as is, because
//! there are no neighbours to be used for smoothing.
//!
//! See VertexOfFace::getFeatureVecMedianOneRing
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::getFeatureVecMedianOneRing( vector<double>& rMedianValues, [[maybe_unused]] double rMinDist   ) {
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		try {
			rMedianValues.at( i ) = mFeatureVec[i];
		} catch (...) {
			LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Exception!\n";
			LOG::error() << "[Vertex::" << __FUNCTION__ << "]        Probably some missmatch of array/vector-size.\n";
			return( false );
		}
	}
	return( true );
}

//! Mean value for each element of the feature vector's elements.
//!
//! For a solo vertex the feature vector is returned as is, because
//! there are no neighbours to be used for smoothing.
//!
//! See VertexOfFace::getFeatureVecMeanOneRing
//!
//! @returns false in case of an error. True otherwise.
bool Vertex::getFeatureVecMeanOneRing( vector<double>& rMeanValues, [[maybe_unused]] double rMinDist   ) {
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		try {
			rMeanValues.at( i ) = mFeatureVec[i];
		} catch (...) {
			LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Exception!\n";
			LOG::error() << "[Vertex::" << __FUNCTION__ << "]        Probably some missmatch of array/vector-size.\n";
			return( false );
		}
	}
	return( true );
}

bool Vertex::getFeatureVecMeanOneRing(
                [[maybe_unused]] const double& rMinDist ,
                [[maybe_unused]] const vector<s1RingSectorPrecomp>& r1RingSecPrecomp ,
                vector<double>& rMeanValues
) {
	for( unsigned int i=0; i<mFeatureVecLen; i++ ) {
		try {
			rMeanValues.at( i ) = mFeatureVec[i];
		} catch (...) {
			LOG::error() << "[Vertex::" << __FUNCTION__ << "] ERROR: Exception!\n";
			LOG::error() << "[Vertex::" << __FUNCTION__ << "]        Probably some missmatch of array/vector-size.\n";
			return( false );
		}
	}
	return( true );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Rollouts ------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Unrolls a vertex that is assumed to be a part of a truncated cone situated
//! around the y-axis. The cone is specified by two radii and two positions. y0
//! is the "upper" y-position, y1 is the "lower" y-position. Returns true if
//! the vertex could be unrolled, else false.
bool Vertex::unrollAroundCone( double rCutHeight1,   //!< Lower clipping plane defined by the height of the cone (y-coordinate)
                               double rCutHeight2,   //!< Upper clipping plane defined by the height of the cone (y-coordinate)
                               double rConeAngle,    //!< Angle of the cone.
                               double rPrimeMeridian //!< Offset of the prime Meridian. Defines the cutting position of the cone.
) {
	// Skip vertices that lie _outside_ the cone
	if( this->getY() < std::min( rCutHeight1, rCutHeight2 ) || this->getY() > std::max( rCutHeight1, rCutHeight2 ) ) {
		return false;
	}

	double phi   = atan2(this->getZ(), this->getX()); // vertex angle in cylindrical coordinates
	double r     = sqrt(this->getX()*this->getX()+this->getY()*this->getY()+this->getZ()*this->getZ()); // vertex distance to origin i.e. radius within the spherical coordinate system
	double beta  = asin( getY()/r ); // vertex angle of the spherical coordinate system
	double delta = M_PI/2.0 - rConeAngle - beta; // 'offset' angle between the cone and the vertex-

	phi += rPrimeMeridian; // Offset for the cut

	if( phi > M_PI ) {
		phi -= 2.0*M_PI;
	} else if( phi < -M_PI ) {
		phi += 2.0*M_PI;
	}

	double s = r * cos( delta ); // Distance to cone tip along the cone's surface.
	double d = r * sin( delta ); // Orthogonal (minimal) distance of the point to cone's surface.

	double sinConeAngle = sin( rConeAngle );

	// Degenerated case: cone becomes cylinder
	if( sinConeAngle == 0.0 ) {
		double cylinderArcPos = d * phi;
		this->setPosition( cylinderArcPos, getY(), -d );
		return true;
	}

	double coneCoordAngle = phi * sinConeAngle;

	double conePosX = s * cos( coneCoordAngle );
	double conePosY = s * sin( coneCoordAngle );
	this->setPosition( conePosX, conePosY, d );

	return true;
}


//! Unrolls a vertex that is assumed to be a part of an infinite truncated cylinder
//!  situated around the y-axis. The cone is specified by an radius.
bool Vertex::unrollAroundCylinderRadius( double rRadius,       //!< Radius of the cylinder.
                                         double rPrimeMeridian //!< Offset of the prime Meridian. Defines the cutting position of the cone.
) {
	double phi = atan2( this->getX(), this->getZ()) ; // vertex angle in cylindrical coordinates
	double d   = sqrt( this->getX()*this->getX() + this->getZ()*this->getZ() ); // vertex distance to the axis

	phi += rPrimeMeridian; // Offset for the cut

	if( phi > M_PI ) {
		phi -= 2.0*M_PI;
	} else if( phi < -M_PI ) {
		phi += 2.0*M_PI;
	}

	double cylinderArcPos = rRadius * phi;
	this->setPosition( cylinderArcPos, getY(), d );
	return true;
}

//! Projects a vertex using the equirectangular projection. The function
//! name is not entirely correct.
//! @param primeMeridian Meridian onto which projection is centered
//! @param sphereRadius  Radius of fitted sphere. The radial distance of
//!                      each vertex is _not_ used because this would result
//!                      in heavy distortions when the vertices are not
//!                      situated along a perfectly spherical object.
bool Vertex::unrollAroundSphere(double primeMeridian, double sphereRadius)
{
	double r     = sqrt(this->getX()*this->getX() +
	                    this->getY()*this->getY() +
	                    this->getZ()*this->getZ());

	double theta   = atan2(this->getZ(), this->getX());
	double phi     = acos (this->getY() / r);

	theta -= primeMeridian;

	if(theta > M_PI) {
		theta -= 2*M_PI;
	}
	else if(theta < -M_PI) {
		theta += 2*M_PI;
	}

	this->setPosition(sphereRadius*theta, sphereRadius*phi, r);
	return(true);
}

// DEBUGING --------------------------------------------------------------------

//! Dumps information about the Vertex to stdout.
void Vertex::dumpInfo() {
	Primitive::dumpInfo();
	cout << "[Vertex] " << getIndex() << endl;
	cout << "         " << getX() << ", " << getY() << ", " << getZ() << endl;
}

//! Dumps information about the Vertex and its neighbourhood as an image
//! of a graph using the DOT-language and GraphViz.
void Vertex::dumpInfoAsDOT( string fileSuffix ) {
	stringstream dotGraph (stringstream::in | stringstream::out);

	dotGraph << "digraph vertex_" << getIndex() << "_edges {" << endl;
// 	set<Edge*>::iterator itEdge;
	// first we get the edges connected to our vertex
// 	for( itEdge=edgeList.begin(); itEdge!=edgeList.end(); itEdge++ ) {
// 		Face* belongsToFace = (*itEdge)->getFace( this );
// 		if( belongsToFace == NULL ) {
// 			cerr << "[Vertex] dumpInfoAsDOT no Face found for edge." << endl;
// 		}
// 		dotGraph << "  ";
// 		dotGraph << (*itEdge)->getVertA()->getIndex() << " -> ";
// 		dotGraph << (*itEdge)->getVertB()->getIndex();
// 		if( (*itEdge)->isBorder() ) {
// 			dotGraph << " [label=\"" << (*itEdge)->getIndex() << "\",group=" << belongsToFace->getIndex() << ",color=red] ";
// 		} else {
// 			dotGraph << " [label=\"" << (*itEdge)->getIndex() << "\",group=" << belongsToFace->getIndex() << "] ";
// 		}
// 		dotGraph << endl;
// 	}
	// secondly we fetch the opposite edges of the triangle
//	set<Face*>::iterator itFace;
//	for( itFace=faceList.begin(); itFace!=faceList.end(); itFace++ ) {
//		Edge* edgeOposite = (*itFace)->getOposingEdge( this );
//		dotGraph << "  ";
//		dotGraph << edgeOposite->getVertA()->getIndex() << " -> ";
//		dotGraph << edgeOposite->getVertB()->getIndex();
// 		if( edgeOposite->isBorder() ) {
// 			dotGraph << " [label=\"" << edgeOposite->getIndex() << "\",style=dotted,group=" << (*itFace)->getIndex() << ",color=red] ";
// 		} else {
//			dotGraph << " [label=\"" << edgeOposite->getIndex() << "\",group=" << (*itFace)->getIndex() << ",style=dotted] ";
// 		}
//		dotGraph << endl;
//	}
//	dotGraph << "}" << endl;
	//cout << dotGraph.str();

	writeDotFile( &dotGraph, &fileSuffix );
}

// Position management ---------------------------------------------------------

//! Sets a new position vector for the vertex
bool Vertex::setPosition( double rPosX, double rPosY, double rPosZ ) {
	mPosition[0] = rPosX;
	mPosition[1] = rPosY;
	mPosition[2] = rPosZ;
	return true;
}

// GLOBAL Operators ------------------------------------------------------------
//! \todo Source revision
//! \todo Proper testing
//! Correlation, r = result requires size m+n-1
void corrr1d(const double* signal, const int n, const double* pattern, const int m, double* r) {
        int i;
        int j;
        double v;
		double* s = new double[m+n-1];
	for(i=0; i<n; i++) {
		s[i] = signal[i];
	}
	for(i=n; i<=m+n-2; i++)
        {
            s[i] = 0;
        }

        for(i=0; i<=n-1; i++)
        {
            v = 0;
            for(j=0; j<=m-1; j++)
            {
                if( i+j>=n )
                {
                    break;
                }
                v += pattern[j] * s[i+j];
            }
            r[i] = v;
        }
        for(i=1; i<=m-1; i++)
        {
            v = 0;
            for(j=i; j<=m-1; j++)
            {
               v += pattern[j] * s[-i+j];
            }
            r[m+n-1-i] = v;
        }

		delete[] s;
}

//! Compute the distance between to vertices.
double distanceVV( const Vertex* rVertA, const Vertex* rVertB ) {
	double dist;
	double posB[3];
	rVertB->copyXYZTo( posB );
	rVertA->getDistanceFromCenterOfGravityTo( posB, &dist );
	return( dist );
	// Faster than: return abs3( rVertA->getPositionVector() - rVertB->getPositionVector() );
}

//! Compute the distance between a vertex and a (position) vector
double distanceVV( Vertex* rVert, const Vector3D& rPos ) {
	return abs3( rVert->getPositionVector() - rPos );
}

//! Compute the distance between a (position) vector and a vertex
double distanceVV( Vector3D* rPos, Vertex* rVert ) {
	return abs3( (*rPos) - rVert->getPositionVector() );
}
//! Cimpute distance between two Vector3D
double distanceVV( Vector3D* rPos1, Vector3D* rPos2 ) {
	return abs3( (*rPos1) - (*rPos2) );
}
