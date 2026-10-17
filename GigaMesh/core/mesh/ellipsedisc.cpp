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

#include <GigaMesh/mesh/ellipsedisc.h>

#define cross(a, b, ab) (ab)[0] = (a)[1]*(b)[2] - (a)[2]*(b)[1]; \
	(ab)[1] = (a)[2]*(b)[0] - (a)[0]*(b)[2]; \
	(ab)[2] = (a)[0]*(b)[1] - (a)[1]*(b)[0];

using namespace std;

//! Direct Least Square Fitting of Ellipses by A. Fitzgibbon, M. Pilu , R.Fisher
//! from Java-Code --> http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/PILU1/demo.html
//! \param at least six points to fit an ellipse to
//! \return error, centerX, centerY, radius1, radius2, theta
bool EllipseDisc::findEllipseParams( eComputeMode rMode,                    //! Algorithm used to compute an ellipse.
                                     vector<pair<double,double> > rPoints   //! Candidate points in 2D e.g. projected to a plane.
                                   ) {
	if( rMode == CONIC ) {
		return toconic( rPoints );
	}

	size_t np = rPoints.size();           // number of points
	double tx,ty;
	int nrot=0;

	double** D = nullptr;
	double** S = nullptr;
	double** L = nullptr;
	double** invL = nullptr;
	double** Const = nullptr;
	double** temp = nullptr;
	double** C = nullptr;
	double** V = nullptr;
	double** sol = nullptr;
	double* d = nullptr;

	d= static_cast<double *>(malloc(7* sizeof(double)));
	Const= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		Const[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	S= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		S[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	temp= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		temp[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	L= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		L[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	C= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		C[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	invL= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		invL[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	V= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		V[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	sol= static_cast<double **>(malloc(7* sizeof(double *)));
	for (int i= 0; i< 7; i++){
		sol[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	D= static_cast<double **>(malloc((np+1)* sizeof(double *)));
	for (size_t i= 0; i< np+1; i++){
		D[i]= static_cast<double *>(malloc(7* sizeof(double)));
	}
	if( rMode == FPF ) {
		Const[1][3] = -2;
		Const[2][2] = +1;
		Const[3][1] = -2;
	}

	if( rMode == BOOKSTEIN ) {
		Const[1][1] = +2;
		Const[2][2] = +1;
		Const[3][3] = +2;
	}

	// Now first fill design matrix
	for (size_t i=0; i < np; i++) {
		tx = rPoints[i].first;
		ty = rPoints[i].second;
		D[i+1][1] = tx*tx;
		D[i+1][2] = tx*ty;
		D[i+1][3] = ty*ty;
		D[i+1][4] = tx;
		D[i+1][5] = ty;
		D[i+1][6] = 1.0;
	}

	// Now compute scatter matrix  S
	A_TperB(D,D,S,static_cast<int>(np),6,static_cast<int>(np),6);

	choldc(S,6,L);

	inverse(L,invL,6);

	AperB_T(Const,invL,temp,6,6,6,6);
	AperB(invL,temp,C,6,6,6,6);

	jacobi(C,6,d,V,nrot);

	A_TperB(invL,V,sol,6,6,6,6);

	// Now normalize them
	for (int j=1;j<=6;j++) { // Scan columns
		double mod = 0.0;
		for (int i=1;i<=6;i++)
			mod += sol[i][j]*sol[i][j];
		for (int i=1;i<=6;i++)
			sol[i][j] /=  sqrt(mod);
	}

	double zero=10e-20;
	double minev=10e+20;
	int  solind=0;

	if (rMode == FPF) {
		for (int i=1; i<=6; i++)
			if (d[i]<0 && abs(d[i])>zero)
				solind = i;
	}

	if (rMode == BOOKSTEIN) {
		for (int i=1; i<=6; i++)
			if (d[i]<minev && abs(d[i])>zero)
				solind = i;
	}

	// Now fetch the right solution
	vector<double> pvec;
	pvec.resize(7,0.0);

	for (int j=1;j<=6;j++)
		pvec[j] = sol[j][solind];

	// ...and solve polynom to standard form
	bool retVal = solveellipse( pvec );

	for (int i= 0; i< 7; i++){
		free(S[i]);
		free(Const[i]);
		free(temp[i]);
		free(L[i]);
		free(C[i]);
		free(invL[i]);
		free(V[i]);
		free(sol[i]);
	}
	free(S);
	free(Const);
	free(temp);
	free(L);
	free(C);
	free(invL);
	free(V);
	free(sol);
	//	free(pvec);
	free(d);

	for (size_t i= 0; i< np+1; i++){
		free(D[i]);
	}
	free(D);
	return retVal;
}

/* toconic takes five points in homogeneous coordinates, and returns the
 * coefficients of a general conic equation in a, b, c, ..., f:
 *
 * a*x*x + b*x*y + c*y*y + d*x + e*y + f = 0.
 *
 * The routine returns 1 on success; 0 otherwise.  (It can fail, for
 * example, if there are duplicate points.
 *
 * Typically, the points will be finite, in which case the third (w)
 * coordinate for all the input vectors will be 1, although the code
 * deals cleanly with points at infinity.
 *
 * For example, to find the equation of the conic passing through (5, 0),
 * (-5, 0), (3, 2), (3, -2), and (-3, 2), set:
 *
 * p0[0] =  5, p0[1] =  0, p0[2] = 1,
 * p1[0] = -5, p1[1] =  0, p1[2] = 1,
 * p2[0] =  3, p2[1] =  2, p2[2] = 1,
 * p3[0] =  3, p3[1] = -2, p3[2] = 1,
 * p4[0] = -3, p4[1] =  2, p4[2] = 1.
 *
 * But if you want the equation of the hyperbola that is tangent to the
 * line 2x=y at infinity,  simply make one of the points be the point at
 * infinity along that line, for example:
 *
 * p0[0] = 1, p0[1] = 2, p0[2] = 0.
 */


//! checks if the 3D centerPoints define an axis orthogonal to the cuttingPlanes in proper distance.
//! \param exactly five points to fit an ellipse to
//! \return error, centerX, centerY, radius1, radius2, theta
bool EllipseDisc::toconic( vector<pair<double,double> > rFivePoints ) {
	// Sanity checks.
	if( rFivePoints.size() < 5 ) {
		return false;
	}
	if( rFivePoints.size() > 5 ) {
		cout << "[EllipseDisc::" << __FUNCTION__ << "] Requires only 5 points. Will ignore the last " << rFivePoints.size()-5 << " points." << endl;
	}

	double p0[3];
	p0[0] = rFivePoints[0].first;
	p0[1] = rFivePoints[0].second;
	p0[2] = 1;

	double p1[3];
	p1[0] = rFivePoints[1].first;
	p1[1] = rFivePoints[1].second;
	p1[2] = 1;

	double p2[3];
	p2[0] = rFivePoints[2].first;
	p2[1] = rFivePoints[2].second;
	p2[2] = 1;

	double p3[3];
	p3[0] = rFivePoints[3].first;
	p3[1] = rFivePoints[3].second;
	p3[2] = 1;

	double p4[3];
	p4[0] = rFivePoints[4].first;
	p4[1] = rFivePoints[4].second;
	p4[2] = 1;

	double L0[3], L1[3], L2[3], L3[3];
	double A, B, C, Q[3];
	double a1, a2, b1, b2, c1, c2;
	double x0, x4, y0, y4, w0, w4;
	double aa, bb, cc, dd, ee, ff;
	double y4w0, w4y0, w4w0, y4y0, x4w0, w4x0, x4x0, y4x0, x4y0;
	double a1a2, a1b2, a1c2, b1a2, b1b2, b1c2, c1a2, c1b2, c1c2;

	cross(p0, p1, L0)
	cross(p1, p2, L1)
	cross(p2, p3, L2)
	cross(p3, p4, L3)
	cross(L0, L3, Q)
	A = Q[0]; B = Q[1]; C = Q[2];
	a1 = L1[0]; b1 = L1[1]; c1 = L1[2];
	a2 = L2[0]; b2 = L2[1]; c2 = L2[2];
	x0 = p0[0]; y0 = p0[1]; w0 = p0[2];
	x4 = p4[0]; y4 = p4[1]; w4 = p4[2];

	y4w0 = y4*w0;
	w4y0 = w4*y0;
	w4w0 = w4*w0;
	y4y0 = y4*y0;
	x4w0 = x4*w0;
	w4x0 = w4*x0;
	x4x0 = x4*x0;
	y4x0 = y4*x0;
	x4y0 = x4*y0;
	a1a2 = a1*a2;
	a1b2 = a1*b2;
	a1c2 = a1*c2;
	b1a2 = b1*a2;
	b1b2 = b1*b2;
	b1c2 = b1*c2;
	c1a2 = c1*a2;
	c1b2 = c1*b2;
	c1c2 = c1*c2;

	aa = -A*a1a2*y4w0
	+A*a1a2*w4y0
	-B*b1a2*y4w0
	-B*c1a2*w4w0
	+B*a1b2*w4y0
	+B*a1c2*w4w0
	+C*b1a2*y4y0
	+C*c1a2*w4y0
	-C*a1b2*y4y0
	-C*a1c2*y4w0;

	cc =  A*c1b2*w4w0
	+A*a1b2*x4w0
	-A*b1c2*w4w0
	-A*b1a2*w4x0
	+B*b1b2*x4w0
	-B*b1b2*w4x0
	+C*b1c2*x4w0
	+C*b1a2*x4x0
	-C*c1b2*w4x0
	-C*a1b2*x4x0;

	ff =  A*c1a2*y4x0
	+A*c1b2*y4y0
	-A*a1c2*x4y0
	-A*b1c2*y4y0
	-B*c1a2*x4x0
	-B*c1b2*x4y0
	+B*a1c2*x4x0
	+B*b1c2*y4x0
	-C*c1c2*x4y0
	+C*c1c2*y4x0;

	bb =  A*c1a2*w4w0
	+A*a1a2*x4w0
	-A*a1b2*y4w0
	-A*a1c2*w4w0
	-A*a1a2*w4x0
	+A*b1a2*w4y0
	+B*b1a2*x4w0
	-B*b1b2*y4w0
	-B*c1b2*w4w0
	-B*a1b2*w4x0
	+B*b1b2*w4y0
	+B*b1c2*w4w0
	-C*b1c2*y4w0
	-C*b1a2*x4y0
	-C*b1a2*y4x0
	-C*c1a2*w4x0
	+C*c1b2*w4y0
	+C*a1b2*x4y0
	+C*a1b2*y4x0
	+C*a1c2*x4w0;

	dd = -A*c1a2*y4w0
	+A*a1a2*y4x0
	+A*a1b2*y4y0
	+A*a1c2*w4y0
	-A*a1a2*x4y0
	-A*b1a2*y4y0
	+B*b1a2*y4x0
	+B*c1a2*w4x0
	+B*c1a2*x4w0
	+B*c1b2*w4y0
	-B*a1b2*x4y0
	-B*a1c2*w4x0
	-B*a1c2*x4w0
	-B*b1c2*y4w0
	+C*b1c2*y4y0
	+C*c1c2*w4y0
	-C*c1a2*x4y0
	-C*c1b2*y4y0
	-C*c1c2*y4w0
	+C*a1c2*y4x0;

	ee = -A*c1a2*w4x0
	-A*c1b2*y4w0
	-A*c1b2*w4y0
	-A*a1b2*x4y0
	+A*a1c2*x4w0
	+A*b1c2*y4w0
	+A*b1c2*w4y0
	+A*b1a2*y4x0
	-B*b1a2*x4x0
	-B*b1b2*x4y0
	+B*c1b2*x4w0
	+B*a1b2*x4x0
	+B*b1b2*y4x0
	-B*b1c2*w4x0
	-C*b1c2*x4y0
	+C*c1c2*x4w0
	+C*c1a2*x4x0
	+C*c1b2*y4x0
	-C*c1c2*w4x0
	-C*a1c2*x4x0;

	if (aa != 0.0) {
		bb /= aa; cc /= aa; dd /= aa; ee /= aa; ff /= aa; aa = 1.0;
	} else if (bb != 0.0) {
		cc /= bb; dd /= bb; ee /= bb; ff /= bb; bb = 1.0;
	} else if (cc != 0.0) {
		dd /= cc; ee /= cc; ff /= cc; cc = 1.0;
	} else if (dd != 0.0) {
		ee /= dd; ff /= dd; dd = 1.0;
	} else if (ee != 0.0) {
		ff /= ee; ee = 1.0;
	}

	vector<double> params;
	params.push_back(0.0);
	params.push_back(aa);
	params.push_back(bb);
	params.push_back(cc);
	params.push_back(dd);
	params.push_back(ee);
	params.push_back(ff);

	// polynom to standard parameter - not used
	//    double x, y, w;
	//    x = p0[0]; y = p0[1]; w = p0[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p1[0]; y = p1[1]; w = p1[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p2[0]; y = p2[1]; w = p2[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p3[0]; y = p3[1]; w = p3[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p4[0]; y = p4[1]; w = p4[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);

	// polynom to standard parameter
	bool retVal = solveellipse(params);
    return retVal;
}

//! transposes second matrix and then multipies first matrix with second matrix
//! \param _A first matrix
//! \param _B second matrix
//! \param _res result matrix
//! \param _righA row count of first matrix
//! \param _colA column count of first matrix
//! \param _righB row count of second matrix
//! \param _colB column count of secound matrix
void EllipseDisc::AperB_T( double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB ) {
	int p,q,l;
	p= _righB;
	for (p=1;p<=_colA;p++)
		for (q=1;q<=_colB;q++)
		{ _res[p][q]=0.0;
		for (l=1;l<=_righA;l++)
			_res[p][q]=_res[p][q]+_A[p][l]*_B[q][l];
		}
}

//! transposes first matrix and then multipies first matrix with second matrix 
//! \param _A first matrix
//! \param _B second matrix
//! \param _res result matrix
//! \param _righA row count of first matrix
//! \param _colA column count of first matrix
//! \param _righB row count of second matrix
//! \param _colB column count of secound matrix
void EllipseDisc::A_TperB( double** _A, double**  _B, double** _res, int _righA, int _colA, int _righB, int _colB ) {
	int p,q,l;
	p= _righB;
	for (p=1;p<=_colA;p++)
		for (q=1;q<=_colB;q++)
		{ _res[p][q]=0.0;
		for (l=1;l<=_righA;l++)
			_res[p][q]=_res[p][q]+_A[l][p]*_B[l][q];
		}
}

//! multipies matrix a with matrix b
//! \param _A first matrix
//! \param _B second matrix
//! \param _res result matrix
//! \param _righA row count of first matrix
//! \param _colA column count of first matrix
//! \param _righB row count of second matrix
//! \param _colB column count of secound matrix
void EllipseDisc::AperB( double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB ) {
	int p,q,l;
	p= _righB;
	for (p=1;p<=_righA;p++)
		for (q=1;q<=_colB;q++)
		{ _res[p][q]=0.0;
		for (l=1;l<=_colA;l++)
			_res[p][q]=_res[p][q]+_A[p][l]*_B[l][q];
		}
}

//! computes inverse of given matric
//! \param TB the matrix we are operating on
//! \param InvB result
//! \param N size of (NxN) matrices
//! \return 0 if successful
int EllipseDisc::inverse( double** TB, double** InvB, int N ) {

	int k,i,j,q;
	double mult;
	double D,temp;
	double maxpivot;
	int npivot;
	//	double B[N+1][N+2];
	double** B;
	B=static_cast<double**>(malloc((N+1)*sizeof(double*)));
	for (int i = 0; i<(N+1);i++){
		B[i]=static_cast<double*>(malloc((N+2)*sizeof(double)));
	}

	//	double A[N+1][2*N+2];
	double** A;
	A=static_cast<double**>(malloc((N+1)*sizeof(double*)));
	for (int i = 0; i<(N+1);i++){
		A[i]=static_cast<double*>(malloc((2*N+2)*sizeof(double)));
	}
	//	double C[N+1][N+1];
	double eps = 10e-10;


	for(k=1;k<=N;k++)
		for(j=1;j<=N;j++)
			B[k][j]=TB[k][j];

	for (k=1;k<=N;k++)
	{
		for (j=1;j<=N+1;j++)
			A[k][j]=B[k][j];
		for (j=N+2;j<=2*N+1;j++)
			A[k][j]= 0.0F;
		A[k][k-1+N+2]= 1.0F;
	}
	for (k=1;k<=N;k++)
	{
		maxpivot=abs(A[k][k]);
		npivot=k;
		for (i=k;i<=N;i++)
			if (maxpivot<abs(A[i][k]))
			{
				maxpivot=abs(A[i][k]);
				npivot=i;
			}
		if (maxpivot>=eps)
		{      if (npivot!=k)
			for (j=k;j<=2*N+1;j++)
			{
				temp=A[npivot][j];
				A[npivot][j]=A[k][j];
				A[k][j]=temp;
			} ;
			D=A[k][k];
			for (j=2*N+1;j>=k;j--)
				A[k][j]=A[k][j]/D;
			for (i=1;i<=N;i++)
			{
				if (i!=k)
				{
					mult=A[i][k];
					for (j=2*N+1;j>=k;j--)
						A[i][j]=A[i][j]-mult*A[k][j] ;
				}
			}
		}
		else
		{  // printf("\n The matrix may be singular !!") ;
			for (int i=0;i<(N+1);i++) {
				free(A[i]);
				free(B[i]);
			}
			free(A);
			free(B);
			return(-1);
		};
	}
	/**   Copia il risultato nella matrice InvB  ***/
	for (k=1;k<=N;k++)
		for (q=1;q+N+1<=2*N+1;q++)
			InvB[k][q]=A[k][q+N+1];
	for (int i=0;i<(N+1);i++) {
		free(A[i]);
		free(B[i]);
	}
	free(A);
	free(B);
	return(0);
} // End of INVERSE

//! Perform the Cholesky decomposition
//! \param a the matrix we are operating on
//! \param n size of (NxN) matrix
//! \param l the lower triangular L  such that L*L'=A
void EllipseDisc::choldc( double** a, int n, double** l ) {
	int i,j,k;
	double sum;
	double* p = new double [n+1]();

	for (i=1; i<=n; i++)  {
		for (j=i; j<=n; j++)  {
			for (sum=a[i][j],k=i-1;k>=1;k--) sum -= a[i][k]*a[j][k];
			if (i == j) {
				if (sum<=0.0)
					// printf("\nA is not poitive definite!");
				{}
				else
					p[i]=sqrt(sum); }
			else
			{
				a[j][i]=sum/p[i];
			}
		}
	}
	for (i=1; i<=n; i++)
		for (j=i; j<=n; j++)
			if (i==j)
				l[i][i] = p[i];
			else
			{
				l[j][i]=a[j][i];
				l[i][j]=0.0;
			}

	delete[] p;
}

//! computes Jacobi-matrix
//! \param a the matrix we are operating on
//! \param n size of (NxN) marix
//! \param d array with eigenvalues
//! \param v martix with eigenvectors
//! \param nrot not longer used
void EllipseDisc::jacobi( double** a, int n, double d[] , double** v, int nrot ) {
	int j,iq,ip,i;
	double tresh,theta,tau,t,sm,s,h,g,c;

	double* b;
	b=static_cast<double*>(malloc((n+1)*sizeof(double)));
	double* z;
	z=static_cast<double*>(malloc((n+1)*sizeof(double)));

	for (ip=1;ip<=n;ip++) {
		for (iq=1;iq<=n;iq++) v[ip][iq]=0.0;
		v[ip][ip]=1.0;
	}
	for (ip=1;ip<=n;ip++) {
		b[ip]=d[ip]=a[ip][ip];
		z[ip]=0.0;
	}
	nrot=0;
	for (i=1;i<=50;i++) {
		sm=0.0;
		for (ip=1;ip<=n-1;ip++) {
			for (iq=ip+1;iq<=n;iq++)
				sm += abs(a[ip][iq]);
		}
		if (sm == 0.0) {
			/*    free_vector(z,1,n);
		  free_vector(b,1,n);  */
			free(b);
			free(z);
			return;
		}
		if (i < 4)
			tresh=0.2*sm/(n*n);
		else
			tresh=0.0;
		for (ip=1;ip<=n-1;ip++) {
			for (iq=ip+1;iq<=n;iq++) {
				g=100.0*abs(a[ip][iq]);
				if (i > 4 && abs(d[ip])+g == abs(d[ip])
						&& abs(d[iq])+g == abs(d[iq]))
					a[ip][iq]=0.0;
				else if (abs(a[ip][iq]) > tresh) {
					h=d[iq]-d[ip];
					if (abs(h)+g == abs(h))
						t=(a[ip][iq])/h;
					else {
						theta=0.5*h/(a[ip][iq]);
						t=1.0/(abs(theta)+sqrt(1.0+theta*theta));
						if (theta < 0.0) t = -t;
					}
					c=1.0/sqrt(1+t*t);
					s=t*c;
					tau=s/(1.0+c);
					h=t*a[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					a[ip][iq]=0.0;
					for (j=1;j<=ip-1;j++) {
						ROTATE(a,j,ip,j,iq,tau,s);
					}
					for (j=ip+1;j<=iq-1;j++) {
						ROTATE(a,ip,j,j,iq,tau,s);
					}
					for (j=iq+1;j<=n;j++) {
						ROTATE(a,ip,j,iq,j,tau,s);
					}
					for (j=1;j<=n;j++) {
						ROTATE(v,j,ip,j,iq,tau,s);
					}
					++nrot;
				}
			}
		}
		for (ip=1;ip<=n;ip++) {
			b[ip] += z[ip];
			d[ip]=b[ip];
			z[ip]=0.0;
		}
	}
	//printf("Too many iterations in routine JACOBI");
	free(b);
	free(z);
}

//! Ellipse polygon to standard form from MatLab-Code
//! -->  http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/FISHER/ELLIPFIT/solveellipse.m
//! Given an ellipse in polynomial form, finds the standard form: ((x-cx)/r1)^2 + ((y-cy)/r2)^2 = 1
//! error < 0 indicates a failed fit containing "NotANumber"-values
//! \param ellipsePolynom: a(1)x^2 + a(2)xy + a(3)y^2 + a(4)x + a(5)y + a(6) = 0
//! \return error, centerX, centerY, radius1, radius2, theta
bool EllipseDisc::solveellipse( vector<double> a ) {
	vector<double> res;
	res.resize(6,0.0);
	// get ellipse orientation
	//theta = atan2(a(2),a(1)-a(3))/2;
	res[5] = atan2(a[2],a[1]-a[3])/2;

	// get scaled major/minor axes
	//        ct = cos(theta);
	double ct = cos(res[5]);
	//        st = sin(theta);
	double st = sin(res[5]);
	//        ap = a(1)*ct*ct + a(2)*ct*st + a(3)*st*st;
	double ap = a[1]*ct*ct + a[2]*ct*st + a[3]*st*st;
	//        cp = a(1)*st*st - a(2)*ct*st + a(3)*ct*ct;
	double cp = a[1]*st*st - a[2]*ct*st + a[3]*ct*ct;

	double** T;
	T = static_cast<double**>(malloc(3*sizeof(double*)));
	T[1] = static_cast<double*>(malloc(3*sizeof(double)));
	T[2] = static_cast<double*>(malloc(3*sizeof(double)));

	double** invT;
	invT = static_cast<double**>(malloc(3*sizeof(double*)));
	invT[1] = static_cast<double*>(malloc(3*sizeof(double)));
	invT[2] = static_cast<double*>(malloc(3*sizeof(double)));

	//        T = [[a(1) a(2)/2]' [a(2)/2 a(3)]'];
	T[1][1] = 2*a[1];
	T[2][1] = 2*a[2]/2;
	T[1][2] = 2*a[2]/2;
	T[2][2] = 2*a[3];

	double* t;
	t = static_cast<double*>(malloc(3*sizeof(double)));

	//        t = -inv(2*T)*[a(4) a(5)]';
	inverse(T,invT,2);
	invT[1][1] *= -1;
	invT[1][2] *= -1;
	invT[2][1] *= -1;
	invT[2][2] *= -1;

	t[1] = invT[1][1]*a[4]+invT[1][2]*a[5];
	t[2] = invT[2][1]*a[4]+invT[2][2]*a[5];

	//        cx = t(1);
	//        cy = t(2);
	res[1] = t[1];
	res[2] = t[2];

	// get scale factor
	//        val = t'*T*t;
	//        scale = 1 / (val- a(6));

	T[1][1] *= 0.5;
	T[1][2] *= 0.5;
	T[2][1] *= 0.5;
	T[2][2] *= 0.5;

	double val = (T[1][1]*t[1]+T[2][1]*t[2])*t[1]+(T[1][2]*t[1]+T[2][2]*t[2])*t[2];
	double scale = 1 / (val- a[6]);

	// get major/minor axis radii
	//        r1 = 1/sqrt(scale*ap);
	//        r2 = 1/sqrt(scale*cp);
	//        v = [r1 r2 cx cy theta]';

	res[3] = 1/sqrt(scale*ap);
	res[4] = 1/sqrt(scale*cp);
	res[5]*=180/M_PI;

	// catching errors that happen when a fit fails
	if (std::isnan(res[3])) res[0]-=1;
	if (std::isnan(res[4])) res[0]-=1;

	free(T[1]);
	free(T[2]);
	free(T);
	free(t);
	free(invT[1]);
	free(invT[2]);
	free(invT);
	
	mError   = res[0];
	mCenterX = res[1];
	mCenterY = res[2]; 
	mRadius1 = res[3];
	mRadius2 = res[4];
	mTheta   = res[5];

	return ( mError == 0.0 );
}

//! special operation needed by jacobi-function
//! \param a the matrix we are operating on
//! \param i row of first element
//! \param j column of first element
//! \param k row of second element
//! \param l column of second element
//! \param tau set by jacobi-function
//! \param s set by jacobi-function
void EllipseDisc::ROTATE( double** a, int i, int j, int k, int l, double tau, double s ) {
	double g,h;
	g=a[i][j];
	h=a[k][l];
	a[i][j]=g-s*(h+g*tau);
	a[k][l]=h+s*(g-h*tau);
}

//! Dump information about the ellipse to std::out.
void EllipseDisc::dumpInfo() {
	cout << "[EllipseDisc] Error:    " << mError   << endl;
	cout << "[EllipseDisc] Center X: " << mCenterX << endl;
	cout << "[EllipseDisc] Center Y: " << mCenterY << endl;
	cout << "[EllipseDisc] Radius 1: " << mRadius1 << endl;
	cout << "[EllipseDisc] Radius 2: " << mRadius2 << endl;
	cout << "[EllipseDisc] Theta:    " << mTheta   << endl;
	cout << "[EllipseDisc] ----------------------" << endl;
}
