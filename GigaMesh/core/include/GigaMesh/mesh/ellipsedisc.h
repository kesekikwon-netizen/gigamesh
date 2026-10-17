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

#ifndef ELLIPSEDISC_H
#define ELLIPSEDISC_H

#include "primitive.h"

class EllipseDisc : public Primitive { // Ellipse is a win32 function :(

public:
	EllipseDisc() = default;

	enum eComputeMode {
		BOOKSTEIN,
		FPF,
		CONIC
	};

	bool findEllipseParams( eComputeMode rMode, std::vector<std::pair<double,double> > rPoints );
	bool toconic( std::vector<std::pair<double,double> > rFivePoints );

	void dumpInfo();

    // Ellipse definition (in 2D)
    double mError;
    double mCenterX;
    double mCenterY;
    double mRadius1;
    double mRadius2;
    double mTheta;
private:
	void AperB_T(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	void A_TperB(double** _A, double**  _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	void AperB(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	int inverse(double** TB, double** InvB, int N);
	void choldc(double** a, int n, double** l);
	void jacobi(double** a, int n, double d[] , double** v, int nrot);
	bool solveellipse( std::vector<double> a );
	void ROTATE(double** a, int i, int j, int k, int l, double tau, double s);


};

#endif // ELLIPSEDISC_H
