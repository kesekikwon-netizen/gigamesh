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

#include <GigaMesh/mesh/polyline.h>
#include <GigaMesh/mesh/plane.h>

#define POLYLINEINITDEFAULTS             \
	mPlaneUsed( nullptr )

using namespace std;

//! Constructor - sets position and normal to not-a-number.
PolyLine::PolyLine()
	: Vertex( Vector3D( _NOT_A_NUMBER_ ), Vector3D( _NOT_A_NUMBER_ ) ), POLYLINEINITDEFAULTS {
	//vertPos.dumpInfo();
	//vertNormal.dumpInfo();
	//cout << "[PolyLine::" << __FUNCTION__ << "] setLabelNr: " << setLabelNr << endl;
}

//! Constructor accepting a position vector and a normal vector
PolyLine::PolyLine( Vector3D vertPos, Vector3D vertNormal )
	: Vertex( vertPos, vertNormal ), POLYLINEINITDEFAULTS  {
	// Nothing else to do here.
}

//! Constructor accepting a position vector, a normal vector and a label no.
PolyLine::PolyLine( Vector3D vertPos, Vector3D vertNormal, uint64_t setLabelNr )
	: Vertex( vertPos, vertNormal, setLabelNr ), POLYLINEINITDEFAULTS  {
	// Nothing else to do here.
}

//! Constructor for polygonal lines computed using
//! a plane intersecting the mesh i.e. profile line.
PolyLine::PolyLine( const Plane& rPlaneIntersecting )
        : POLYLINEINITDEFAULTS {
	mPlaneUsed = new Plane( rPlaneIntersecting );
}

//! Destructor
PolyLine::~PolyLine() {
	vector<PolyEdge*>::iterator itEdge;
	for ( itEdge=mEdgeList.begin(); itEdge != mEdgeList.end(); itEdge++ ) {
		delete (*itEdge);
	}
	mEdgeList.clear();
}

// Information retrival, Common ---------------------------------------------------- ---------------------------------------------------------------------------

double PolyLine::getX() const {
	//! Returns the x-coordinate of the polylines's center of gravity.
	double localCogX = 0.0;
	for( const PolyEdge* polyEdge : mEdgeList ) {
		localCogX += polyEdge->mVertPoly->getX();
	}
	localCogX /= mEdgeList.size();
	return localCogX;
}

double PolyLine::getY() const {
	//! Returns the y-coordinate of the polylines's center of gravity.
	double localCogY = 0.0;
	vector<Vertex*>::iterator itVertex;
	for( const PolyEdge* polyEdge : mEdgeList ) {
		localCogY += polyEdge->mVertPoly->getY();
	}
	localCogY /= mEdgeList.size();
	return localCogY;
}

double PolyLine::getZ() const {
	//! Returns the z-coordinate of the polylines's center of gravity.
	double localCogZ = 0.0;
	for( const PolyEdge* polyEdge : mEdgeList ) {
		localCogZ += polyEdge->mVertPoly->getZ();
	}
	localCogZ /= mEdgeList.size();
	return localCogZ;
}

// Information retrival, Specific --------------------------------------------------- ---------------------------------------------------------------------------

//! Returns the absoult length of the polyline.
//! Returns false in case of an error.
bool PolyLine::getLengthAbs( double* rLength ) {
	if( rLength == nullptr ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	if( mEdgeList.size() == 0 ) {
		(*rLength) = _NOT_A_NUMBER_DBL_;
		return true;
	}
	if( mEdgeList.size() == 1 ) {
		(*rLength) = 0.0;
		return true;
	}
	double lenAbs = 0.0;
	Vector3D lastPos = mEdgeList.at( 0 )->mVertPoly->getPositionVector();
	for( unsigned int vertIdx=1; vertIdx<mEdgeList.size(); vertIdx++ ) {
		Vector3D currPos = mEdgeList.at( vertIdx )->mVertPoly->getPositionVector();
		double edgleLen = ( currPos - lastPos ).getLength3();
		lenAbs += edgleLen;
		lastPos = currPos;
	}
	(*rLength) = lenAbs;
	return true;
}

//! Returns the length of each edge of the polyline.
bool PolyLine::getEdgeLengths( vector<double>* rEdgeLengths ) {
	if( rEdgeLengths == nullptr ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	Vector3D lastPos = mEdgeList.at( 0 )->mVertPoly->getPositionVector();
	for( unsigned int vertIdx=1; vertIdx<mEdgeList.size(); vertIdx++ ) {
		Vector3D currPos = mEdgeList.at( vertIdx )->mVertPoly->getPositionVector();
		double edgleLen = ( currPos - lastPos ).getLength3();
		rEdgeLengths->push_back( edgleLen );
		lastPos = currPos;
	}
	return true;
}

//! Returns the function values of each of the polyline's vertices.
bool PolyLine::getEdgeFuncVals( vector<double>* rFuncVals ) {
	if( rFuncVals == nullptr ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	for( const PolyEdge* polyEdge : mEdgeList ) {
		double funcVal;
		polyEdge->mVertPoly->getFuncValue( &funcVal );
		rFuncVals->push_back( funcVal );
	}
	return true;
}

//! Retrieve the optional intersecting plane.
//!
//! @returns false in case no plane was defined or an error occured. True otherwise.
bool PolyLine::getIntersectPlane( Plane** rIntersectPlane ) {
	if( mPlaneUsed == nullptr ) {
		return( false );
	}
	if( !mPlaneUsed->isValid() ) {
		return( false );
	}
	(*rIntersectPlane) = new Plane( *mPlaneUsed );
	return true ;
}

/*
//! Returns true, when all vertices of the PolyLine belong to the same label.
//! In this case the label id will be written to getLabelNr.
//! Returns false otherwise - also when all vertices are tagged as "no labled".
bool PolyLine::vertLabelGet( int* getLabelNr ) {
	// A Polyline has to have at least one edge.
	if( mEdgeList.size() == 0 ) {
		return false;
	}

	vector<PolyEdge*>::iterator itEdge;
	itEdge = mEdgeList.begin();

	Vertex* currVert = (*itEdge)->mVertPoly;
	int     labelFirst;
	int     labelCurr;
	// Fetch the first vertex label in advance and bail out, when this fails.
	if( !currVert->getLabel( &labelFirst ) ) {
		return false;
	}
	// Check all vertices:
	do {
		currVert = (*itEdge)->mVertPoly;
		if( !currVert->getLabel( &labelCurr ) ) {
			return false;
		}
		// Test if the current label matches the first one.
		if( labelFirst != labelCurr ) {
			// If not we know that not all vertices have the same label and we bail out.
			return false;
		}
		itEdge++;
	} while( itEdge != mEdgeList.end() );
	// if we reach this point, we know that all vertices belong to the same label:
	(*getLabelNr) = labelFirst;
	return true;
}
*/

bool PolyLine::isClosed() {
	if(mEdgeList.empty())
		return false;
	//! Returns true, when the last vertex of the polyline is identical to the first vertex.
	Vector3D firstLastVert = mEdgeList.front()->mVertPoly->getPositionVector() - mEdgeList.back()->mVertPoly->getPositionVector();
	if( firstLastVert.getLength3() <= DBL_EPSILON ) {
		return true;
	}
	return false;
}

//! Returns the maximum of the smoothed curvature.
bool PolyLine::getCurvatureSmoothMax( double* rMaxCurv ) {
	(*rMaxCurv) = -DBL_MAX;
	for( const PolyEdge* polyEdge : mEdgeList ) {
		double currCurv;
		polyEdge->getFuncValue( &currCurv );
		if( currCurv > (*rMaxCurv) ) {
			(*rMaxCurv) = currCurv;
		}
	}
	return true;
}

//! Returns the minimum of the smoothed curvature.
bool PolyLine::getCurvatureSmoothMin( double* rMinCurv ) {
	(*rMinCurv) = +DBL_MAX;
	for( const PolyEdge* polyEdge : mEdgeList ) {
		double currCurv;
		polyEdge->getFuncValue( &currCurv );
		if( currCurv < (*rMinCurv) ) {
			(*rMinCurv) = currCurv;
		}
	}
	return true;
}

//! Returns the minimum and the maximum of the smoothed curvature.
bool PolyLine::getCurvatureSmoothMinMax( double* rMinCurv, double* rMaxCurv ) {
	(*rMinCurv) = +DBL_MAX;
	(*rMaxCurv) = -DBL_MAX;
	for( const PolyEdge* polyEdge : mEdgeList ){
		double currCurv;
		polyEdge->getFuncValue( &currCurv );
		if( currCurv < (*rMinCurv) ) {
			(*rMinCurv) = currCurv;
		}
		if( currCurv > (*rMaxCurv) ) {
			(*rMaxCurv) = currCurv;
		}
	}
	return true;
}

bool PolyLine::closeLine() {
	//! Closes the line (by adding the first vertex to the end).
	//! Returns false, when the PolyLine is already closed.
	if( isClosed() ) {
		return false;
	}
	addBackNoDupes( mEdgeList.front()->mVertPoly, mEdgeList.front()->mFromFace, mEdgeList.front()->mFromEdge );
	return true;
}

bool PolyLine::estCurvature( bool absolut ) {
	//! Returns the signed list of curvature along the polylines segements.
	//! Related to getAngles, while the curvature is 1/R of the circumscribed circle:
	//! Absolut (bad results in many cases): 1/R = 4A/abc with area A=|n|/2, and edge lengths a, b and c.
	//! Correct (unitless!): 1/R = 4A/ac with area A=|n|/2, and edge lengths a, b and c.
	//!
	//! Attention: curvatureSmooth is set equal to curvature - smoothing hast to be applied e.g. by PolyLine::estCurvatureSmooth
	if( length() < 3 ) {
		cerr << "[PolyLine::estCurvature] too short: " << length() <<  " < 3! Curvature set to zero." << endl;
		for( PolyEdge* polyEdge : mEdgeList ) {
			polyEdge->mCurvature = 0.0;
			polyEdge->setFuncValue( 0.0 );
		}
		return false;
	}
	for( int i=0; i<static_cast<int>(mEdgeList.size()); i++ ) {
		int lastVertIdx = getSafeIndex( i-1 );
		if( lastVertIdx < 0 ) {
			//cout << "[PolyLine::estCurvature] open end (left)" << endl;
			//cout << "[PolyLine::getCurvature] getSafeIndex( " << i << "-1 ): " << getSafeIndex(i-1) <<  "" << endl;
			mEdgeList.at( i )->mCurvature = 0.0;
			continue;
		}
		int nextVertIdx = getSafeIndex( i+1 );
		if( nextVertIdx < 0 ) {
			//cout << "[PolyLine::estCurvature] open end (right)" << endl;
			//cout << "[PolyLine::getCurvature] getSafeIndex( " << i << "+1 ): " << getSafeIndex(i+1) <<  "" << endl;
			mEdgeList.at( i )->mCurvature = 0.0;
			continue;
		}
		Vector3D lastVert = mEdgeList.at( lastVertIdx )->mVertPoly->getPositionVector();
		Vector3D currVert = mEdgeList.at( i )->mVertPoly->getPositionVector();
		Vector3D nextVert = mEdgeList.at( getSafeIndex( nextVertIdx ) )->mVertPoly->getPositionVector();
		Vector3D currNorm = mEdgeList.at( i )->mVertPoly->getNormal();
		Vector3D vAB = currVert - lastVert;
		Vector3D vBC = nextVert - currVert;
		double curvNumer;
		double curvDenom;
		if( absolut ) {
			curvNumer = 2.0*( vAB % vBC ).getLength3();
			// scale by 1/50 for visualization purposes:
			curvDenom = ( (vAB.getLength3()*vBC.getLength3()*(lastVert-nextVert).getLength3()) * 50.0);
		} else {
			// we can also skip x2.0:
			curvNumer = (vAB.getLength3()+vBC.getLength3())*( vAB % vBC ).getLength3();
			curvDenom = (vAB.getLength3()*vBC.getLength3()*(lastVert-nextVert).getLength3());
		}
		double curvature = curvNumer / curvDenom;
		if( std::isnan( curvature ) ) {
			cerr << "[PolyLine::estCurvature] Bad polyline, because the curvature isnan ( " << curvNumer << " / " << curvDenom << " ), Vertex/Edge index: " << i << ". ";
			if( curvNumer == 0.0 ) {
				cerr << "Curvature set to 0.0!" << endl;
				curvature = 0.0;
			} else {
				cerr << "Curvature set to DBL_MAX!" << endl;
				curvature = DBL_MAX;
			}
		}
		double theta = angle( vAB % vBC, currNorm );
		if( theta < M_PI/2.0 ) {
			curvature *= -1.0;
		}
		//cout << "[PolyLine::estCurvature] curvature: " << curvature << " theta: " << theta * 180.0/M_PI << " Vertex/Edge index: " << i << endl;
		mEdgeList.at( i )->mCurvature = static_cast<float>(curvature);
		mEdgeList.at( i )->setFuncValue( curvature );
	}
	return true;
}

bool PolyLine::estCurvatureSmooth( double gaussWidth ) {
	//! Smooths the given curvatureList with a gaussian filter of gaussWidth.
	//! The result is added to curvatureListSmooth.
	vector<int>    neighVertsIdx;
	vector<double> dists;
	vector<double> weights;
	for( int i=0; i<static_cast<int>(mEdgeList.size()); i++ ) {
		getNeighboursRunLen( i, gaussWidth, &neighVertsIdx, &dists, &weights );
		double smoothCurv = 0.0;
		//cout << "filtVal = [ ";
		double filtValSum = 0.0;
		for( int j=0; j<static_cast<int>(dists.size()); j++ ) {
			double filtVal = sqrt( 1.0/(2.0*M_PI) ) * exp( -pow( 3.0*dists.at( j )/gaussWidth, 2.0 ) / 2.0 );
			//cout << filtVal << " ";
			smoothCurv += filtVal * mEdgeList.at( neighVertsIdx.at( j ) )->mCurvature;// * weights.at( j );
			filtValSum += filtVal;
		}
		//cout << "];" << endl;
		mEdgeList.at( i )->setFuncValue( smoothCurv/filtValSum );
		neighVertsIdx.clear();
		dists.clear();
		weights.clear();
	}
	return true;
}

//! Compute the integral invariants using the radius rIIRadius for all PolyEdge elements.
//! @returns false in case of an error.
bool PolyLine::compIntInv( double rIIRadius, ePolyIntInvDirection rDirection ) {
	bool noError = true;
	for( unsigned int i=0; i<mEdgeList.size(); i++ ) {
		if( !compIntInv( i, rIIRadius, rDirection ) ) {
			noError = false;
		}
	}
	cerr << "[PolyLine::" << __FUNCTION__ << "] Warning or errors occured!" << endl;
	return noError;
}

//! Compute the integral invariants using the radius rIIRadius for all a PolyEdge elements with the given index.
//! @returns false in case of an error.
bool PolyLine::compIntInv( int rVertNr, double rIIRadius, ePolyIntInvDirection rDirection ) {
	int      iCurrSafeStart = getSafeIndex( rVertNr );
	int      iCurrSafe      = iCurrSafeStart;
	int      iCurrSafeNext  = getSafeIndex( iCurrSafeStart+1 );
	Vertex*  vertStart      = mEdgeList.at( iCurrSafeStart )->mVertPoly;
	Vector3D sphereCenter   = vertStart->getPositionVector();
	bool     stopBorder     = false;
	bool     stopLoop       = false;
	double   distIntegral   = 0.0;

	// Compute in FORWARD direction:
	if( rDirection != POLY_INTEGRAL_INV_BACKWARD ) {
		while( (!stopBorder) && (!stopLoop) ) {
			Vertex* currVert = mEdgeList.at( iCurrSafe )->mVertPoly;
			Vertex* nextVert = mEdgeList.at( iCurrSafeNext )->mVertPoly;
			double distSpherical = distanceVV( vertStart,  nextVert );
			if( distSpherical > rIIRadius ) {
				// Sphere boundary crossed:
				Vector3D currPos = currVert->getPositionVector();
				Vector3D nextPos = nextVert->getPositionVector();
				Vector3D interSect1;
				Vector3D interSect2;
				eLineSphereCases intersectCase = lineSphereIntersect( rIIRadius, sphereCenter, currPos, nextPos, &interSect1, &interSect2 );
				if ( ( intersectCase != LSI_ONE_INTERSECT_P1 ) && ( intersectCase != LSI_ONE_INTERSECT_P2 ) ) {
					cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: Unexpected intersection!" << endl;
				}
				if( intersectCase == LSI_ONE_INTERSECT_P1 ) {
					//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (1). Adding: " << abs3( interSect1 - currPos ) << endl;
					distIntegral += abs3( interSect1 - currPos );
				} else {
					//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (2). Adding: " << abs3( interSect2 - currPos ) << endl;
					distIntegral += abs3( interSect2 - currPos );
				}
				stopBorder = true;
				continue;
			}
			//cout << "[PolyLine::" << __FUNCTION__ << "] Adding: " << distanceVV( currVert, nextVert ) << endl;
			distIntegral += distanceVV( currVert, nextVert );
			// Set the next index:
			iCurrSafe = iCurrSafeNext;
			iCurrSafeNext = getSafeIndex( iCurrSafeNext+1 );
			// Prevent infinite loop, when rIIRadius defines a sphere containing the whole polyline:
			if( iCurrSafeNext == iCurrSafeStart ) {
				stopLoop = true;
				continue;
			}
		}
		if( stopLoop ) {
			// We have a problem and do not need to continue any further:
			cerr << "[PolyLine::" << __FUNCTION__ << "] Warning: Radius for Integral Invariant appears to be to large!" << endl;
			return false;
		}
	}

	// Set values for BACKWARD direction:
	if( rDirection != POLY_INTEGRAL_INV_FORWARD ) {
		stopBorder    = false;
		stopLoop      = false;
		iCurrSafe     = iCurrSafeStart;
		iCurrSafeNext = getSafeIndex( iCurrSafeStart-1 );
		// Compute in BACKWARD direction:
		while( (!stopBorder) && (!stopLoop) ) {
			Vertex* currVert = mEdgeList.at( iCurrSafe )->mVertPoly;
			Vertex* nextVert = mEdgeList.at( iCurrSafeNext )->mVertPoly;
			double distSpherical = distanceVV( vertStart,  nextVert );
			if( distSpherical > rIIRadius ) {
				// Sphere boundary crossed:
				Vector3D currPos = currVert->getPositionVector();
				Vector3D nextPos = nextVert->getPositionVector();
				Vector3D interSect1;
				Vector3D interSect2;
				eLineSphereCases intersectCase = lineSphereIntersect( rIIRadius, sphereCenter, currPos, nextPos, &interSect1, &interSect2 );
				if ( ( intersectCase != LSI_ONE_INTERSECT_P1 ) && ( intersectCase != LSI_ONE_INTERSECT_P2 ) ) {
					cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: Unexpected intersection!" << endl;
				}
				if( intersectCase == LSI_ONE_INTERSECT_P1 ) {
					//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (1). Adding: " << abs3( interSect1 - currPos ) << endl;
					distIntegral += abs3( interSect1 - currPos );
				} else {
					//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (2). Adding: " << abs3( interSect2 - currPos ) << endl;
					distIntegral += abs3( interSect2 - currPos );
				}
				stopBorder = true;
				continue;
			}
			//cout << "[PolyLine::" << __FUNCTION__ << "] Adding: " << distanceVV( currVert, nextVert ) << endl;
			distIntegral += distanceVV( currVert, nextVert );
			// Set the next index:
			iCurrSafe = iCurrSafeNext;
			iCurrSafeNext = getSafeIndex( iCurrSafeNext-1 );
			// Prevent infinite loop, when rIIRadius defines a sphere containing the whole polyline:
			if( iCurrSafeNext == iCurrSafeStart ) {
				stopLoop = true;
				continue;
			}
		}
	}

	mEdgeList.at( iCurrSafeStart )->mCurvature = static_cast<float>(distIntegral / rIIRadius);
	mEdgeList.at( iCurrSafeStart )->setFuncValue( distIntegral / rIIRadius );

	return true;
}

//! Compute the integral invariants using the radius rIIRadius for all PolyEdge elements.
//! @returns false in case of an error.
bool PolyLine::compIntInvAngle( double rIIRadius ) {
	bool noError = true;
	for( unsigned int i=0; i<mEdgeList.size(); i++ ) {
		if( !compIntInvAngle( i, rIIRadius ) ) {
			noError = false;
		}
	}
	cerr << "[PolyLine::" << __FUNCTION__ << "] Warning or errors occured!" << endl;
	return noError;
}

//! Compute the integral invariants using the radius rIIRadius for all a PolyEdge elements with the given index.
//! @returns false in case of an error.
bool PolyLine::compIntInvAngle( int rVertNr, double rIIRadius ) {
	int      iCurrSafeStart = getSafeIndex( rVertNr );
	int      iCurrSafe      = iCurrSafeStart;
	int      iCurrSafeNext  = getSafeIndex( iCurrSafeStart+1 );
	Vertex*  vertStart      = mEdgeList.at( iCurrSafeStart )->mVertPoly;
	Vector3D sphereCenter   = vertStart->getPositionVector();
	bool     stopBorder     = false;
	bool     stopLoop       = false;
	double   distIntegral   = 0.0;
	Vector3D borderPointA;
	Vector3D borderPointB;
	// Compute in forward direction:
	while( (!stopBorder) && (!stopLoop) ) {
		Vertex* currVert = mEdgeList.at( iCurrSafe )->mVertPoly;
		Vertex* nextVert = mEdgeList.at( iCurrSafeNext )->mVertPoly;
		double distSpherical = distanceVV( vertStart,  nextVert );
		if( distSpherical > rIIRadius ) {
			// Sphere boundary crossed:
			Vector3D currPos = currVert->getPositionVector();
			Vector3D nextPos = nextVert->getPositionVector();
			Vector3D interSect1;
			Vector3D interSect2;
			eLineSphereCases intersectCase = lineSphereIntersect( rIIRadius, sphereCenter, currPos, nextPos, &interSect1, &interSect2 );
			if ( ( intersectCase != LSI_ONE_INTERSECT_P1 ) && ( intersectCase != LSI_ONE_INTERSECT_P2 ) ) {
				cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: Unexpected intersection!" << endl;
			}
			if( intersectCase == LSI_ONE_INTERSECT_P1 ) {
				//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (1). Adding: " << abs3( interSect1 - currPos ) << endl;
				borderPointA = interSect1;
				distIntegral += abs3( interSect1 - currPos );
			} else {
				//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (2). Adding: " << abs3( interSect2 - currPos ) << endl;
				borderPointA = interSect2;
				distIntegral += abs3( interSect2 - currPos );
			}
			stopBorder = true;
			continue;
		}
		//cout << "[PolyLine::" << __FUNCTION__ << "] Adding: " << distanceVV( currVert, nextVert ) << endl;
		distIntegral += distanceVV( currVert, nextVert );
		// Set the next index:
		iCurrSafe = iCurrSafeNext;
		iCurrSafeNext = getSafeIndex( iCurrSafeNext+1 );
		// Prevent infinite loop, when rIIRadius defines a sphere containing the whole polyline:
		if( iCurrSafeNext == iCurrSafeStart ) {
			stopLoop = true;
			continue;
		}
	}
	if( stopLoop ) {
		// We have a problem and do not need to continue any further:
		cerr << "[PolyLine::" << __FUNCTION__ << "] Warning: Radius for Integral Invariant appears to be to large!" << endl;
		return false;
	}

	// Set values for BACKWARD direction:
	stopBorder    = false;
	stopLoop      = false;
	iCurrSafe     = iCurrSafeStart;
	iCurrSafeNext = getSafeIndex( iCurrSafeStart-1 );
	// Compute in BACKWARD direction:
	while( (!stopBorder) && (!stopLoop) ) {
		Vertex* currVert = mEdgeList.at( iCurrSafe )->mVertPoly;
		Vertex* nextVert = mEdgeList.at( iCurrSafeNext )->mVertPoly;
		double distSpherical = distanceVV( vertStart,  nextVert );
		if( distSpherical > rIIRadius ) {
			// Sphere boundary crossed:
			Vector3D currPos = currVert->getPositionVector();
			Vector3D nextPos = nextVert->getPositionVector();
			Vector3D interSect1;
			Vector3D interSect2;
			eLineSphereCases intersectCase = lineSphereIntersect( rIIRadius, sphereCenter, currPos, nextPos, &interSect1, &interSect2 );
			if ( ( intersectCase != LSI_ONE_INTERSECT_P1 ) && ( intersectCase != LSI_ONE_INTERSECT_P2 ) ) {
				cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: Unexpected intersection!" << endl;
			}
			if( intersectCase == LSI_ONE_INTERSECT_P1 ) {
				//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (1). Adding: " << abs3( interSect1 - currPos ) << endl;
				borderPointB = interSect1;
				distIntegral += abs3( interSect1 - currPos );
			} else {
				//cout << "[PolyLine::" << __FUNCTION__ << "] Spherical boundary crossed (2). Adding: " << abs3( interSect2 - currPos ) << endl;
				borderPointB = interSect2;
				distIntegral += abs3( interSect2 - currPos );
			}
			stopBorder = true;
			continue;
		}
		//cout << "[PolyLine::" << __FUNCTION__ << "] Adding: " << distanceVV( currVert, nextVert ) << endl;
		distIntegral += distanceVV( currVert, nextVert );
		// Set the next index:
		iCurrSafe = iCurrSafeNext;
		iCurrSafeNext = getSafeIndex( iCurrSafeNext-1 );
		// Prevent infinite loop, when rIIRadius defines a sphere containing the whole polyline:
		if( iCurrSafeNext == iCurrSafeStart ) {
			stopLoop = true;
			continue;
		}
	}

	Vector3D surfNorm = vertStart->getNormal();
	double intAngle;
	if( abs3( surfNorm ) > 0.0 ) {
		intAngle = -angle( (borderPointA-sphereCenter), (borderPointB-sphereCenter) );
		//intAngle = angle( (borderPointA-sphereCenter), (borderPointB-sphereCenter), surfNorm );
	} else {
		intAngle = angle( (borderPointA-sphereCenter), (borderPointB-sphereCenter) );
	}

	mEdgeList.at( iCurrSafeStart )->mCurvature = static_cast<float>(intAngle);
	mEdgeList.at( iCurrSafeStart )->setFuncValue( intAngle );

	return true;
}

bool PolyLine::getExtrema( set<Vertex*>* someVerts, double gaussWidth, bool absolut ) {
	//! Returns the polylines extrema.
	if( !estCurvature( absolut ) ) {
		cerr << "[PolyLine::getExtrema] problem estimating curvature!" << endl;
		return false;
	}
	if( ( gaussWidth > 0.0 ) && ( !estCurvatureSmooth( gaussWidth ) ) ) {
		cerr << "[PolyLine::getExtrema] problem smoothing curvature, gaussWidth: " << gaussWidth << "!" << endl;
		return false;
	}
	// Debuging:
	//----------
	//dumpCurvMat();
	//dumpCurvSmoothMat();
	//dumpRunLenMat();

	uint64_t myLabel;
	bool hasLabel = getLabel( myLabel );
	if( hasLabel ) {
		cout << "[PolyLine::" << __FUNCTION__ << "] Label " << myLabel << " ";
	} else {
		cout << "[PolyLine::" << __FUNCTION__ << "] ";
	}

	for( int i=1; i<static_cast<int>(mEdgeList.size()); i++ ) {
		// Compute root of the curvature (Nullstellen)
		double curvA;
		double curvB;
		mEdgeList.at( i-1 )->getFuncValue( &curvA );
		mEdgeList.at( i )->getFuncValue( &curvB );
		if( curvA * curvB < 0.0 ) {
			Vertex* newVert = new Vertex( mEdgeList.at( i-1 )->mVertPoly, mEdgeList.at( i )->mVertPoly, curvA, curvB, 0.0 );
			newVert->setFlag( Primitive::FLAG_BELONGS_TO_POLYLINE );
			if( hasLabel ) {
				newVert->setLabel( myLabel );
			}
			newVert->setRGB( 128, 128, 128 );
			someVerts->insert( newVert );
			cout << "o";
		}
		// Compute Maxima and Minima (Extremwerte)
		int nextIdx = getSafeIndex( i+1 );
		if( nextIdx < 0 ) {
			continue;
		}
		double curvC;
		mEdgeList.at( nextIdx )->getFuncValue( &curvC );
		double dCurvPrev = curvA - curvB;
		double dCurvNext = curvB - curvC;
		if( dCurvPrev * dCurvNext < 0.0 ) {
			Vertex* newVert = new Vertex( mEdgeList.at( i )->mVertPoly->getPositionVector() );
			newVert->setFlag( Primitive::FLAG_BELONGS_TO_POLYLINE );
			if( hasLabel ) {
				newVert->setLabel( myLabel );
			}
			someVerts->insert( newVert );
			if( dCurvPrev < 0.0 ) {
				cout << "-";
				if( curvB > 0.0 ) {
					newVert->setRGB( 0, 200, 0 );
				} else {
					newVert->setRGB( 85, 170, 255 );
				}
			} else {
				cout << "+";
				if( curvB > 0.0 ) {
					newVert->setRGB( 255, 85, 0 );
				} else {
					newVert->setRGB( 255, 77, 255 );
				}
			}
		}
	}
	cout << endl;

	return true;
}

//! Finds the neighbours left and right of vertNr within a given distance +/-.
//! The distance is measured along the polyline (AKA run-length).
//! The weights are half of the distances to the next vertex.
//! Returns false in case of an errror.
bool PolyLine::getNeighboursRunLen( int rVertNr, double rDist, vector<int>* rNeighVerts, vector<double>* rDists, vector<double>* rWeights ) {
	if( rVertNr < 0 ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: Vertex index negative!" << endl;
		return false;
	}
	if( rVertNr >= static_cast<int>(mEdgeList.size()) ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: Vertex index to large!" << endl;
		return false;
	}
	if( rDist < 0.0 ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] ERROR: Distance negative!" << endl;
		return false;
	}
	double   distLeft     = 0.0;
	double   distRight    = 0.0;
	Vertex*  vertLeft     = nullptr;
	Vertex*  vertRight    = nullptr;
	double   newDistLeft  = 0.0;
	double   newDistRight = 0.0;
	int      vertLeftIdx  = getSafeIndex( rVertNr-1 );
	int      vertRightIdx = getSafeIndex( rVertNr+1 );
	if( vertLeftIdx < 0 ) {
		newDistLeft = rDist * 2.0;
	} else {
		vertLeft  = mEdgeList.at( vertLeftIdx )->mVertPoly;
		newDistLeft = ( vertLeft->getCenterOfGravity()  - mEdgeList.at( rVertNr )->mVertPoly->getCenterOfGravity() ).getLength3();
	}
	if( vertRightIdx < 0 ) {
		newDistRight = rDist * 2.0;
	} else {
		vertRight = mEdgeList.at( vertRightIdx )->mVertPoly;
		newDistRight = ( vertRight->getCenterOfGravity() - mEdgeList.at( rVertNr )->mVertPoly->getCenterOfGravity() ).getLength3();
	}
	if( newDistLeft > rDist ) {
		newDistLeft = rDist * 2.0;
		distLeft    = rDist;
	} else {
		distLeft += newDistLeft;
	}
	if( newDistRight > rDist ) {
		newDistRight = rDist * 2.0;
		distRight    = rDist;
	} else {
		distRight += newDistRight;
	}
	rNeighVerts->push_back( rVertNr );
	rDists->push_back( 0.0 );
	rWeights->push_back( (newDistLeft+newDistRight)/2.0 );

	int iCurr = rVertNr;
	while( distRight < rDist ) {
		iCurr++;
		int iCurrSafe = getSafeIndex( iCurr );
		//cout << "[PolyLine::getNeighbours] walking right " << iCurr << " / " << iCurrSafe << endl;
		vertLeftIdx  = getSafeIndex( iCurr-1 );
		vertRightIdx = getSafeIndex( iCurr+1 );
		// Left is safe:
		vertLeft  = mEdgeList.at( vertLeftIdx )->mVertPoly;
		newDistLeft = ( vertLeft->getCenterOfGravity()  - mEdgeList.at( iCurrSafe )->mVertPoly->getCenterOfGravity() ).getLength3();
		// Right is unsafe:
		if( vertRightIdx < 0 ) {
			vertRight = nullptr;
			newDistRight = DBL_MAX;
		} else {
			vertRight = mEdgeList.at( vertRightIdx )->mVertPoly;
			newDistRight = ( vertRight->getCenterOfGravity() - mEdgeList.at( iCurrSafe )->mVertPoly->getCenterOfGravity() ).getLength3();
		}
		rDists->push_back( distRight );
		if( ( distRight + newDistRight ) > rDist ) {
			newDistRight = ( rDist - distRight ) * 2.0;
			distRight    = rDist;
		} else {
			distRight += newDistRight;
		}
		rNeighVerts->push_back( iCurrSafe );
		rWeights->push_back( (newDistLeft+newDistRight)/2.0 );
	}

	iCurr = rVertNr;
	while( distLeft < rDist ) {
		iCurr--;
		int iCurrSafe = getSafeIndex( iCurr );
		//cout << "[PolyLine::getNeighbours] walking left " << iCurr << " / " << iCurrSafe << endl;
		vertLeftIdx  = getSafeIndex( iCurr-1 );
		vertRightIdx = getSafeIndex( iCurr+1 );
		// Left is unsafe:
		if( vertLeftIdx < 0 ) {
			vertLeft = nullptr;
			newDistLeft = DBL_MAX;
		} else {
			vertLeft  = mEdgeList.at( vertLeftIdx )->mVertPoly;
			newDistLeft = ( vertLeft->getCenterOfGravity()  - mEdgeList.at( iCurrSafe )->mVertPoly->getCenterOfGravity() ).getLength3();
		}
		// Right is safe:
		vertRight = mEdgeList.at( vertRightIdx )->mVertPoly;
		newDistRight = ( vertRight->getCenterOfGravity() - mEdgeList.at( iCurrSafe )->mVertPoly->getCenterOfGravity() ).getLength3();

		rDists->insert( rDists->begin(), -distLeft );
		if( ( distLeft + newDistLeft ) > rDist ) {
			newDistLeft = ( rDist - distLeft ) * 2.0;
			distLeft    = rDist;
		} else {
			distLeft += newDistLeft;
		}
		rNeighVerts->insert( rNeighVerts->begin(), iCurrSafe );
		rWeights->insert( rWeights->begin(), (newDistLeft+newDistRight)/2.0 );
	}
#ifdef DEBUG_POLYLINE_GETNEIGHBOURS
	double sumWeight = 0.0;
	cout << "Weights:";
	for( unsigned int i=0; i<rWeights->size(); i++ ) {
		sumWeight += rWeights->at( i );
		cout << " " << rWeights->at( i );
	}
	cout << endl;
	cout << "sumWeight: " << sumWeight << endl;
	cout << "Dists:";
	for( unsigned int i=0; i<rDists->size(); i++ ) {
		cout << " " << rDists->at( i );
	}
	cout << endl;
	cout << "Indices:";
	for( unsigned int i=0; i<rNeighVerts->size(); i++ ) {
		cout << " " << rNeighVerts->at( i );
	}
	cout << endl;
#endif
	return true;
}

size_t PolyLine::getSafeIndex(size_t someIdx ) {
	//! Returns a valid index regarding open and closed polylines.
	//! Closed polylines: returns a corresponding (modulo) value within the index range.
	if( isClosed() ) {
		// Remark: last vertex equals first vertex, so we have to decrement by 1:
		auto maxIndex = mEdgeList.size()-1;
		if( ( someIdx >= 0 ) && ( someIdx < maxIndex ) ) {
			return someIdx;
		}
		// Modulo: a - (n * floor(a/n)) ... because "%" works differently for negative numbers in C/C++
		//cout << "[PolyLine::getSafeIndex] closed " << someIdx << " % " << maxIndex << " " << someIdx-(maxIndex*floor((double)someIdx/(double)maxIndex)) << endl;
		return MODULO_INT( someIdx, maxIndex );
		//return ( someIdx-(maxIndex*floor((double)someIdx/(double)maxIndex)) );
	}
	//! Open polylines: returns a negative value, when out of range.
	//cout << "[PolyLine::getSafeIndex] open " << someIdx << endl;
	if( someIdx < 0 ) {
		return -1;
	}
	if( someIdx >= mEdgeList.size() ) {
		return -1;
	}
	return someIdx;
}

// creation of the polyline -----------------------------------------------------------------------------------

void PolyLine::addFront(Vector3D rNewFrontPos, Vector3D rPosNormal, Face* rFromFace, Face::eEdgeNames rFromEdge ) {
	//! Adds a new Vertex to the beginning of the vertexList.
	//! Faster than PolyLine::addFrontNoDupe, because no check about dupes is peformed.
	//! Usually used, when Mesh::convertSelectedVerticesToPolyline is in reverse mode.
	Vertex* newFrontVert = new Vertex( rNewFrontPos, rPosNormal );
	newFrontVert->setFlag( Primitive::FLAG_BELONGS_TO_POLYLINE );
	addFront( newFrontVert, rFromFace, rFromEdge );
}

void PolyLine::addBack(Vector3D rNewBackPos, Vector3D rPosNormal, Face* rFromFace, Face::eEdgeNames rFromEdge ) {
	//! Adds a new Vertex to the end of the vertexList.
	//! Faster than PolyLine::addBackNoDupe, because no check about dupes is peformed.
	//! Usually used, when Mesh::convertSelectedVerticesToPolyline is NOT in reverse mode.
	Vertex* newBackVert = new Vertex( rNewBackPos, rPosNormal );
	newBackVert->setFlag( Primitive::FLAG_BELONGS_TO_POLYLINE );
	addBack( newBackVert, rFromFace, rFromEdge );
}

void PolyLine::addFront( Vertex* rNewFrontVert, Face* rFromFace, Face::eEdgeNames rFromEdge ) {
	//! Adds a Vertex reference to the beginning of the vertexList.
	//! Faster than PolyLine::addFrontNoDupe, because no check about dupes is peformed.
	//! Usually used, when Mesh::convertSelectedVerticesToPolyline is in reverse mode.
	PolyEdge* newEdge = new PolyEdge( rNewFrontVert, rFromFace, rFromEdge );
	mEdgeList.insert( mEdgeList.begin(), newEdge );
}

void PolyLine::addBack( Vertex* rNewBackVert, Face* rFromFace, Face::eEdgeNames rFromEdge ) {
	//! Adds a Vertex reference to the end of the vertexList.
	//! Faster than PolyLine::addBackNoDupe, because no check about dupes is peformed.
	//! Usually used, when Mesh::convertSelectedVerticesToPolyline is NOT in reverse mode.
	PolyEdge* newEdge = new PolyEdge( rNewBackVert, rFromFace, rFromEdge );
	mEdgeList.push_back( newEdge );
}

bool PolyLine::addFrontNoDupes( Vertex* rNewFrontVert, Face* rFromFace, Face::eEdgeNames rFromEdge ) {
	//! Adds a Vertex reference to the beginning of the vertexList.
	//! IMPORTANT: Check if the distance to the neigbouring vertex is not null!
	//! Usually used, when Mesh::advanceFuncValThres is in reverse mode.
	//! \todo TEST this!
	if( mEdgeList.size() > 0 ) {
		Vector3D vertDist = mEdgeList.front()->mVertPoly->getPositionVector() - rNewFrontVert->getPositionVector();
		if( vertDist.getLength3() <= DBL_EPSILON ) {
			return false;
		}
	}
	PolyEdge* newEdge = new PolyEdge( rNewFrontVert, rFromFace, rFromEdge );
	mEdgeList.insert( mEdgeList.begin(), newEdge );
	return true;
}

bool PolyLine::addBackNoDupes( Vertex* rNewBackVert, Face* rFromFace, Face::eEdgeNames rFromEdge ) {
	//! Adds a Vertex reference to the end of the vertexList.
	//! IMPORTANT: Check if the distance to the neigbouring vertex is not null!
	//! Usually used, when Mesh::advanceFuncValThres is NOT in reverse mode.
	if( mEdgeList.size() > 0 ) {
		Vector3D vertDist = mEdgeList.back()->mVertPoly->getPositionVector() - rNewBackVert->getPositionVector();
		if( vertDist.getLength3() <= DBL_EPSILON ) {
			return false;
		}
	}
	PolyEdge* newEdge = new PolyEdge( rNewBackVert, rFromFace, rFromEdge );
	mEdgeList.push_back( newEdge );
	return true;
}

size_t PolyLine::compileLine( set<labelLine*>* unsortedLines ) {
	//! Compiles a polyline using the list/set of labelLine. Returns the number of labelLines left in the unsortedLines list.

	PolyEdge* newEdge;
	set<labelLine*>::iterator itLine;
	itLine = unsortedLines->begin();
	Vertex* vertFront = (*itLine)->vertB;
	Vertex* vertBack  = (*itLine)->vertA;
	newEdge = new PolyEdge( vertFront, (*itLine)->mFromFace, (*itLine)->mFromEdge );
	mEdgeList.insert( mEdgeList.begin(), newEdge );
	newEdge = new PolyEdge( vertBack, (*itLine)->mFromFace, (*itLine)->mFromEdge );
	mEdgeList.push_back( newEdge );
	unsortedLines->erase( itLine );

	bool lineFound = true;
	while( lineFound ) {
		lineFound = false;
		// using break or continue to shortcut this procedure will not properly close the polyline
        //set<labelLine*>::iterator test = next(unsortedLines->begin(),3);
        //using next because the iterator is not working probably
        //for( itLine=unsortedLines->begin(); (itLine!=unsortedLines->end())&&(!lineFound); itLine++ ) {
        for(unsigned int i=0; i<unsortedLines->size();i++){
            itLine = next(unsortedLines->begin(),i);
            if( (*itLine)->vertA == vertFront ) {
				addFront( (*itLine)->vertB, (*itLine)->mFromFace, (*itLine)->mFromEdge );
				vertFront = (*itLine)->vertB;
				lineFound = true;
                delete (*itLine);
                unsortedLines->erase( itLine );
                //reset i because the next line could be in the front of the set
                i = 0;
                continue;
			} else
			if( (*itLine)->vertA == vertBack ) {
				addBack( (*itLine)->vertB, (*itLine)->mFromFace, (*itLine)->mFromEdge );
				vertBack = (*itLine)->vertB;
				lineFound = true;
                delete (*itLine);
                unsortedLines->erase( itLine );
                i = 0;
                continue;
			}

			if( (*itLine)->vertB == vertFront ) {
				addFront( (*itLine)->vertA, (*itLine)->mFromFace, (*itLine)->mFromEdge );
				vertFront = (*itLine)->vertA;
				lineFound = true;
                delete (*itLine);
                unsortedLines->erase( itLine );
                i = 0;
			} else
			if( (*itLine)->vertB == vertBack ) {
				addBack( (*itLine)->vertA, (*itLine)->mFromFace, (*itLine)->mFromEdge );
				vertBack = (*itLine)->vertA;
				lineFound = true;
                delete (*itLine);
                unsortedLines->erase( itLine );
                i = 0;
			}
		}
	}
	//cout << "[PolyLine::compileLine] " << unsortedLines->size() << endl;
	return unsortedLines->size();
}

// Post-creation of the polyline ---------------------------------------------------------------------------------

//! To be called after creating the polyline -- typically by methods of Mesh.
//! ATTENTION: as vector does not prevent duplicates nor does this method, calling the method twice can messup your Mesh.
//! The same apllies e.g. for Mesh borders, where the polyline contains references to existing vertices!
bool PolyLine::addVerticesTo( vector<Vertex*>* rSomeVertList ) {
	for( const PolyEdge* polyEdge : mEdgeList ) {
		Vertex* currVertex = polyEdge->mVertPoly;
		rSomeVertList->push_back( currVertex );
	}
	return true;
}

// Extrude --------------------------------------------------------------------------------------------------------

//! Extrude a new mesh using the given rotational axis.
//! @returns true in case of an error. False otherwise.
bool PolyLine::extrudeAxis(
	const Vector3D&        rAxisTop,
	const Vector3D&        rAxisBottom,
	vector<Vertex*>*       rVerticesToAppend, // actually: VertexOfFace
	vector<Face*>*         rFacesToAppend
) {
	//std::cout << "[PolyLine::" << __FUNCTION__ << "] Start" << std::endl;

	constexpr size_t angleCount = 100;
	constexpr double angleStep  = 2.0*M_PI/static_cast<double>(angleCount);
	Vector3D origin( 0.0, 0.0, 0.0, 1.0 );
	Vector3D zAxis( 0.0, 0.0, 1.0, 0.0 );
	Vector3D orient = rAxisTop - rAxisBottom;
	Matrix4D baseTrans;
	baseTrans.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Z, &rAxisBottom, &orient );
	Matrix4D rotateMat( origin, zAxis, angleStep );
	Matrix4D reBaseTrans( baseTrans );
	reBaseTrans.invert();

	auto sliceCount = mEdgeList.size();
	VertexOfFace** meshGrid = new VertexOfFace*[angleCount*sliceCount];

	// 1. Compute a regular point cloud
	for( size_t s=0; s<sliceCount; ++s ) {
	//for( auto const& currPolyEdge: mEdgeList ) {
		PolyEdge* currPolyEdge = mEdgeList.at( s );
		Vector3D currPos;
		currPolyEdge->getPosition( &currPos );
		bool normalPresent;
		Vector3D currNormal;
		normalPresent = currPolyEdge->getNormal( &currNormal );
		currPos    *= baseTrans;
		currNormal *= baseTrans;
		for( size_t i=0; i<angleCount; ++i ) {
			VertexOfFace* sectionVertex = new VertexOfFace( currPos );
			sectionVertex->setFlag( FLAG_SYNTHETIC );
			if( normalPresent ) {
				sectionVertex->setNormal( &currNormal );
				//cout << "[PolyLine::" << __FUNCTION__ << "] Normal present." << endl;
			}
			sectionVertex->applyTransfrom( &reBaseTrans );
			rVerticesToAppend->push_back( sectionVertex );
			meshGrid[i*sliceCount + s] = sectionVertex;
			currPos    *= rotateMat;
			currNormal *= rotateMat;
		}
	}

	// 2. Connect the regular grid of points to a mesh
	//    If present, the orientation of the vertices of the profile line
	//    will be used to set a proper orientation of the faces.
	for( size_t s=0; s<sliceCount-1; ++s ) {
		for( size_t i=0; i<angleCount; ++i ) {
			auto nextAngleIdx = i+1;
			if( nextAngleIdx >= angleCount ) {
				nextAngleIdx = 0;
			}
			VertexOfFace* vertA = meshGrid[i* sliceCount + s];
			VertexOfFace* vertB = meshGrid[i* sliceCount + s + 1];
			VertexOfFace* vertC = meshGrid[nextAngleIdx* sliceCount + s];
			VertexOfFace* vertD = meshGrid[nextAngleIdx* sliceCount + s + 1];
			Face* faceA = new Face( 0, vertA, vertC, vertB );
			Face* faceB = new Face( 0, vertB, vertC, vertD );
			faceA->setFlag( FLAG_SYNTHETIC );
			faceB->setFlag( FLAG_SYNTHETIC );
			Vector3D faceNormal = faceA->getNormal();
			Vector3D vertNormal = vertA->getNormal();
			//cout << "[PolyLine::" << __FUNCTION__ << "] Angle: " << acos( dot3( vertNormal, faceNormal ) ) * 180.0/M_PI << endl;
			if( dot3( vertNormal, faceNormal ) < 0.0 ) {
				//cout << "[PolyLine::" << __FUNCTION__ << "] New faces have to be inverted." << endl;
				faceA->invertFaceOrientation();
				faceB->invertFaceOrientation();
			}
			rFacesToAppend->push_back( faceA );
			rFacesToAppend->push_back( faceB );
		}
	}

	delete[] meshGrid;

	return( false );
}

// modification of the polyline -----------------------------------------------------------------------------------

//! Uses the function value of the Faces Vertices to advance the polyline.
//! Returns a new polyline with newly estimated vertices, but maintains the reference to Face an Edge.
//! Returns NULL in case of an error (e.g. no Face references).
PolyLine* PolyLine::advanceFuncValThres( double funcValThres ) {
	if( mEdgeList.size() < 2 ) {
		cerr << "[PolyLine::advanceFuncValThres] No advance possible: less than 2 vertices!" << endl;
		return nullptr;
	}

	bool wasClosed = isClosed();
	uint64_t currentLabel;
	getLabel( currentLabel );
	PolyLine* advancedLine = new PolyLine( getCenterOfGravity(), getNormal( false ), currentLabel );

	for( const PolyEdge* polyEdge : mEdgeList ) {
		Vertex* fromVertex = polyEdge->mVertPoly;
		Face* fromFace = polyEdge->mFromFace;
		Face::eEdgeNames fromEdge = polyEdge->mFromEdge;
		if( fromFace == nullptr ) {
			cerr << "[PolyLine::advanceFuncValThres] No reference to face set!" << endl;
			continue;
		}
		Vertex* newVert;
		bool newVertEstimated = fromFace->getInterpolVertFuncVal( &newVert, &fromFace, fromEdge, funcValThres );
		if( newVertEstimated ) {
			advancedLine->addBackNoDupes( newVert, fromFace );
		} else {
			//cout << "[PolyLine::advanceFuncValThres] No Advance" << endl;
			advancedLine->addBackNoDupes( fromVertex, fromFace, fromEdge );
		}
	}

	if( wasClosed && !advancedLine->isClosed() ) {
		cout << "[PolyLine::advanceFuncValThres] Warning: polyline might be badly closed." << endl;
		advancedLine->closeLine();
	}

	return advancedLine;
}

//! Set the normal of the vertices' of the polyline using the polyline's normal.
bool PolyLine::copyNormalToVertices() {
	if( std::isnan( getNormalLen() ) ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] Polyline has no normal ... will use local normals!" << endl;
		return false;
	}
	if( getNormalLen() == 0.0 ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] Polyline normal has zero length ... will use local normals!" << endl;
		return false;
	}
	double normX = getNormalX();
	double normY = getNormalY();
	double normZ = getNormalZ();
	for( const PolyEdge* polyEdge : mEdgeList ){
		polyEdge->mVertPoly->setNormal( normX, normY, normZ );
	}
	return true;
}

// e.g. for OpenGL ------------------------------------------------------------------------------------------------

int PolyLine::length() {
	//! Number of references - also the size of the pointer to the array returned in PolyLine::getVertexIndices().
	return mEdgeList.size();
}

int* PolyLine::getVertexIndices() {
	//! Return a pointer to an array holding the indices of the Vertices. 
	//! Typically used in MeshIO to write a file.
	//! See also PolyLine::length.
	if( length() <= 0 ) {
		return nullptr;
	}
	int* indexArr = static_cast<int*>(calloc( length(), sizeof(int) ));
	for( int vertIdx=0; vertIdx<static_cast<int>(mEdgeList.size()); vertIdx++ ) {
		indexArr[vertIdx] = mEdgeList.at( vertIdx )->mVertPoly->getIndex();
	}
	return indexArr;
}

uint* PolyLine::getVertexIndicesUnsigned() {
	//! Return a pointer to an array holding the original indices of the Vertices. 
	//! Typically used in MeshGL to draw using OpenGL.
	//! See also PolyLine::length.
	if( length() <= 0 ) {
		return nullptr;
	}
	uint* indexArr = static_cast<uint*>(calloc( mEdgeList.size(), sizeof(uint) ));
	for( int vertIdx=0; vertIdx<static_cast<int>(mEdgeList.size()); vertIdx++ ) {
		indexArr[vertIdx] = static_cast<uint>(mEdgeList.at( vertIdx )->mVertPoly->getIndex());
	}
	return indexArr;
}

//! Retrieve the vertex coordinates as a sequence.
//! It will clear the given vector rCoords.
//!
//! @returns false in case of an error. True otherwise.
bool PolyLine::getVertexCoords( vector<double>* rCoords ) {
	// Sanity check
	if( rCoords == nullptr ) {
		return( false );
	}
	rCoords->clear();
	rCoords->resize( mEdgeList.size()*3, _NOT_A_NUMBER_DBL_ );
	double* coordArray = rCoords->data();
	for( int vertIdx=0; vertIdx<static_cast<int>(mEdgeList.size()); vertIdx++ ) {
		mEdgeList.at( vertIdx )->mVertPoly->copyCoordsTo( &coordArray[vertIdx*3] );
	}
	return( true );
}


//! Retrieve the vertex coordinates as a sequence
//! projected onto the plane (mPlaneUsed).
//!
//! It will clear the given vector rCoords.
//!
//! @returns false in case of an error or mPlaneUsed not set. True otherwise.
bool PolyLine::getVertexCoordsInPlane(
                vector<Vector3D>* rCoords,
                bool rProjectToPlaneUsingAxis
) {
	// Sanity check
	if( rCoords == nullptr ) {
		return( false );
	}

	// Check existance and defintion of the plane
	if( mPlaneUsed == nullptr ) {
		return( false );
	}
	if( !mPlaneUsed->isValid() ) {
		return( false );
	}

	Matrix4D matBaseChange( Matrix4D::INIT_IDENTITY );
	mPlaneUsed->getChangeOfBasisTrans( &matBaseChange, rProjectToPlaneUsingAxis );

	rCoords->clear();
	rCoords->resize( mEdgeList.size() );
	for( int vertIdx=0; vertIdx<static_cast<int>(mEdgeList.size()); vertIdx++ ) {
		Vector3D currPos;
		mEdgeList.at( vertIdx )->getPositionTransformed( &currPos, matBaseChange );
		rCoords->at( vertIdx ) = currPos;
	}

	return( true );
}

//! Retrieve the bounding box.
//! Optional: project the vertices to the plane.
//! This option will only work if a valid plane is present.
//!
//! @returns false in case of an error. True otherwise.
bool PolyLine::getBoundingBox(
                double* rMinX, double* rMinY, double* rMinZ, \
                double* rMaxX, double* rMaxY, double* rMaxZ, \
                bool rProjectToPlane,         //!< Project onto plane, when present/defined.
                bool rProjectToPlaneUsingAxis //!< Only used together with rProjectToPlane using an optional axis.
) {
	// Check existance and defintion of the plane
	if( rProjectToPlane && mPlaneUsed == nullptr ) {
		return( false );
	}
	if( rProjectToPlane && !mPlaneUsed->isValid() ) {
		return( false );
	}

	*rMinX = +_INFINITE_DBL_;
	*rMinY = +_INFINITE_DBL_;
	*rMinZ = +_INFINITE_DBL_;
	*rMaxX = -_INFINITE_DBL_;
	*rMaxY = -_INFINITE_DBL_;
	*rMaxZ = -_INFINITE_DBL_;

	Matrix4D matBaseChange( Matrix4D::INIT_IDENTITY );
	if( rProjectToPlane ) { // Optional
		mPlaneUsed->getChangeOfBasisTrans( &matBaseChange, rProjectToPlaneUsingAxis );
		if( rProjectToPlaneUsingAxis ) {
			// Adding x=0 ensures that the axis can be drawn.
			// Z will be zero anyway due to the projection onto the plane
			// The Y extend of the axis will be determined automatically during the SVG export.
			*rMinX = 0.0;
			*rMaxX = 0.0;
		}
	}

	// Iteration
	vector<PolyEdge*>::iterator currPolyEdge;
	for( currPolyEdge=mEdgeList.begin(); currPolyEdge != mEdgeList.end(); currPolyEdge++ ) {
		Vector3D currPos;
		(*currPolyEdge)->getPositionTransformed( &currPos, matBaseChange );
		// Find smallest value
		if( currPos.getX() < *rMinX ) {
			*rMinX = currPos.getX();
		}
		if( currPos.getY() < *rMinY ) {
			*rMinY = currPos.getY();
		}
		if( currPos.getZ() < *rMinZ ) {
			*rMinZ = currPos.getZ();
		}
		// Find largest value
		if( currPos.getX() > *rMaxX ) {
			*rMaxX = currPos.getX();
		}
		if( currPos.getY() > *rMaxY ) {
			*rMaxY = currPos.getY();
		}
		if( currPos.getZ() > *rMaxZ ) {
			*rMaxZ = currPos.getZ();
		}
	}

	// Done:
	return( true );
}

double* PolyLine::getVertexNormals() {
	//! Returns a double x 3 array of the polylines normals per vertex.
	double* normals = static_cast<double*>(calloc( mEdgeList.size()*3, sizeof(double) ));
	for( int vertIdx=0; vertIdx<static_cast<int>(mEdgeList.size()); vertIdx++ ) {
		if( !mEdgeList.at( vertIdx )->mVertPoly->copyNormalXYZTo( &normals[vertIdx*3] ) ) {
			cerr << "[PolyLine::" << __FUNCTION__ << "] Vertex has no normal!" << endl;
		}
	}
	return normals;
}

//! Compute an average normal using the normals of the vertices of the polyedges.
bool PolyLine::compVertAvgNormal() {
	double sumNormal[3] = { 0.0, 0.0, 0.0 };
	for( const PolyEdge* polyEdge : mEdgeList ) {
		double vertNormal[3];
		if( !polyEdge->mVertPoly->copyNormalXYZTo( vertNormal ) ) {
			cerr << "[PolyLine::" << __FUNCTION__ << "] Vertex has no normal!" << endl;
		}
		sumNormal[0] += vertNormal[0];
		sumNormal[1] += vertNormal[1];
		sumNormal[2] += vertNormal[2];
	}
	cout << "[PolyLine::" << __FUNCTION__ << "] " << sumNormal[0] << ", " << sumNormal[1] << ", " << sumNormal[2] << endl;
	setNormal( sumNormal[0], sumNormal[1], sumNormal[2] );
	return true;
}

//! Compute a center of gravity using the vertices of the polyedges.
bool PolyLine::compVertAvgCog() {
	double sumCOG[3] = { 0.0, 0.0, 0.0 };
	for( const PolyEdge* polyEdge : mEdgeList ) {
		Vertex* currVert = polyEdge->mVertPoly;
		sumCOG[0] += currVert->getX();
		sumCOG[1] += currVert->getY();
		sumCOG[2] += currVert->getZ();
	}
	sumCOG[0] /= static_cast<double>(mEdgeList.size());
	sumCOG[1] /= static_cast<double>(mEdgeList.size());
	sumCOG[2] /= static_cast<double>(mEdgeList.size());
	cout << "[PolyLine::" << __FUNCTION__ << "] " << sumCOG[0] << ", " << sumCOG[1] << ", " << sumCOG[2] << endl;
	setNormal( sumCOG[0], sumCOG[1], sumCOG[2] );
	return true;
}

//! Appends vertices x 3 double values to the given vector (rNormals),
//! which represent normal vectors with the length corresponding the vertices function values.
//! When rShiftMin is true, the minimum of the polyline's vertices' function value is substracted.
//! rUsePolyNormal is only used, when true AND the polyline itself has a valid normal set.
bool PolyLine::getVertexNormalsFuncVal( vector<double>* rNormals, double rScale, bool rShiftMin, bool rUsePolyNormal ) {
	if( std::isnan( getNormalLen() ) ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] Polyline has no normal ... will use local normals!" << endl;
		rUsePolyNormal = false;
	}
	if( getNormalLen() == 0.0 ) {
		cerr << "[PolyLine::" << __FUNCTION__ << "] Polyline normal has zero length ... will use local normals!" << endl;
		rUsePolyNormal = false;
	}
	double minCurv = 0.0;
	if( rShiftMin ) {
		getCurvatureSmoothMin( &minCurv );
	}
	// pre-allocate memory
	rNormals->reserve( mEdgeList.size()*3 );
	// fetch normals
	vector<PolyEdge*>::iterator itPolyEdge;
	for( itPolyEdge=mEdgeList.begin(); itPolyEdge!=mEdgeList.end(); itPolyEdge++ ) {
		double normalXYZ[3];
		double currCurv;
		(*itPolyEdge)->getFuncValue( &currCurv );
		double normalLen = ( currCurv - minCurv ) * rScale;
		if( rUsePolyNormal ) {
			if( !copyNormalXYZTo( normalXYZ, normalLen ) ) {
				cerr << "[PolyLine::" << __FUNCTION__ << "] Polyline has no normal!" << endl;
			}
			//cout << "[PolyLine::" << __FUNCTION__ << "] Normal: " << normals[vertIdx*3] << " " << normals[vertIdx*3+1] << " " << normals[vertIdx*3+2] << endl;
		} else {
			if( !(*itPolyEdge)->mVertPoly->copyNormalXYZTo( normalXYZ, normalLen ) ) {
				cerr << "[PolyLine::" << __FUNCTION__ << "] Vertex of the Polyline has no normal!" << endl;
			}
		}
		rNormals->push_back( normalXYZ[0] );
		rNormals->push_back( normalXYZ[1] );
		rNormals->push_back( normalXYZ[2] );
	}
	return true;
}

Vertex* PolyLine::getVertexRef( int rIndex ) {
	//! Returns the reference to the i.th Vertex of the PolyLine
	//! Returns NULL, when the given index is out of range.
	rIndex = getSafeIndex( rIndex );
	if( rIndex < 0 ) {
		return nullptr;
	}
	return mEdgeList.at( rIndex )->mVertPoly;
}

Face* PolyLine::getFaceRef( int rIndex ) {
	//! Returns the reference to the i.th Vertex of the PolyLine
	//! Returns NULL, when the given index is out of range OR NO face was referenced.
	rIndex = getSafeIndex( rIndex );
	if( rIndex < 0 ) {
		return nullptr;
	}
	return mEdgeList.at( rIndex )->mFromFace;
}

// Common information ------------------------------------------------------------------------------------------------------------------------------------------

int PolyLine::getType() {
	//! Overloaded from Primitive::getType()
	return Primitive::IS_POLYLINE;
}

// // Comparison of function value - Heap/sorting related ------------------------------------------------------------------------------------------------------

bool PolyLine::sortByLenAsc( PolyLine* rPoly1, PolyLine* rPoly2 ) {
	//! Used for sorting e.g. a heap. by polyline absolut length, ascending.
	double lenPoly1;
	double lenPoly2;
	rPoly1->getLengthAbs( &lenPoly1 );
	rPoly2->getLengthAbs( &lenPoly2 );
	return ( lenPoly1 < lenPoly2 );
}

bool PolyLine::sortByLenDesc( PolyLine* rPoly1, PolyLine* rPoly2 ) {
	//! Used for sorting e.g. a heap. by polyline absolut length, descending.
	double lenPoly1;
	double lenPoly2;
	rPoly1->getLengthAbs( &lenPoly1 );
	rPoly2->getLengthAbs( &lenPoly2 );
	return ( lenPoly1 > lenPoly2 );
}

// Debuging ----------------------------------------------------------------------------------------------------------

void PolyLine::dumpCurvMat() {
	//! Dumps the curvature values as string for matlab.
	uint64_t label;
	getLabel( label );
	cout << "%--- Polyline from Label No " << label << " ---" << endl;
	cout << "curvature = [ ";
	for( const PolyEdge* polyEdge : mEdgeList ) {
		cout << polyEdge->mCurvature << " ";
	}
	cout << "];" << endl;
}

void PolyLine::dumpCurvSmoothMat() {
	//! Dumps the smoothed curvature values as string for matlab.
	uint64_t label;
	getLabel( label );
	cout << "%--- Polyline from Label No " << label << " ---" << endl;
	cout << "curvatureSmooth = [ ";
	for( const PolyEdge* polyEdge : mEdgeList ) {
		double currCurv;
		polyEdge->getFuncValue( &currCurv );
		cout << currCurv << " ";
	}
	cout << "];" << endl;
}

void PolyLine::dumpRunLenMat() {
	//! Dumps the run-length of the vertices as string for matlab (typically plotting the curvature).
	uint64_t label;
	getLabel( label );
	cout << "%--- Polyline from Label No " << label << " ---" << endl;
	double runLen = 0.0;
	cout << "runLengths = [ " << runLen;
	for( int i=1; i<static_cast<int>(mEdgeList.size()); i++ ) {
		runLen += ( mEdgeList.at( i-1 )->mVertPoly->getPositionVector() - mEdgeList.at( i )->mVertPoly->getPositionVector() ).getLength3();
		cout << " " << runLen;
	}
	cout << " ];" << endl;
}
