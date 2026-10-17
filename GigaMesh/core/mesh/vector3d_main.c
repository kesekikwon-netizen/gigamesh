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

#include <stdio.h>
#include <stdlib.h> // calloc

#include "vector3d.h"
#include "matrix4d.h"

#include "gmcommon.h" // for windows

using namespace std;

int main(void) {
	//! Main routine for testing various Vector3D examples
	//==========================================================================

	cout << "=== Z Rotation =================================================================" << endl;

	Vector3D vectorX( 1.0f, 0.0f, 2.0f );
	vectorX.dumpInfo();

	cout << "--------------------------------------------------------------------------------" << endl;

	Matrix4D transformMatrixZ( _MATRIX4D_INIT_ROTATION_Z_, M_PI/8.0 );
	transformMatrixZ.dumpInfo( true, "transformMatrixZ" );

	double arrayX[4] = { 1.0, 0.0, 2.0, 1.0 };
	transformMatrixZ.applyTo( arrayX, 1 );
	cout << arrayX[0] << " " << arrayX[1] << " " << arrayX[2] << " " << arrayX[3] << endl;

	cout << "--------------------------------------------------------------------------------" << endl;

	vectorX *= transformMatrixZ;
	vectorX.dumpInfo();

	vectorX *= transformMatrixZ;
	vectorX.dumpInfo();
	vectorX *= transformMatrixZ;
	vectorX.dumpInfo();
	vectorX *= transformMatrixZ;
	vectorX.dumpInfo();

	Matrix4D transformMatrixZ90( _MATRIX4D_INIT_ROTATION_Z_, M_PI/2.0 );
	vectorX *= transformMatrixZ90;
	vectorX.dumpInfo();
	vectorX *= transformMatrixZ90;
	vectorX.dumpInfo();
	vectorX *= transformMatrixZ90;
	vectorX.dumpInfo();

	Matrix4D transformMatrixZ180( _MATRIX4D_INIT_ROTATION_Z_, M_PI/1.0 );
	vectorX *= transformMatrixZ180;
	vectorX.dumpInfo();

	cout << "=== X Rotation =================================================================" << endl;

	Vector3D vectorY( 2.0f, 1.0f, 0.0f );
	vectorY.dumpInfo();

	cout << "--------------------------------------------------------------------------------" << endl;

	vector<double> rotX22 { M_PI/8.0 };
	Matrix4D transformMatrixX( Matrix4D::INIT_ROTATE_ABOUT_X, &rotX22 );
	// old - deprecated: Matrix4D transformMatrixX( _MATRIX4D_INIT_ROTATION_X_, M_PI/8.0 );
	transformMatrixX.dumpInfo( true, "transformMatrixY" );

	cout << "--------------------------------------------------------------------------------" << endl;

	vectorY *= transformMatrixX;
	vectorY.dumpInfo();
	vectorY *= transformMatrixX;
	vectorY.dumpInfo();
	vectorY *= transformMatrixX;
	vectorY.dumpInfo();
	vectorY *= transformMatrixX;
	vectorY.dumpInfo();

	vector<double> rotX90 { M_PI/2.0 };
	Matrix4D transformMatrixX90( Matrix4D::INIT_ROTATE_ABOUT_X, &rotX90 );
	// old - deprecated: Matrix4D transformMatrixX90( _MATRIX4D_INIT_ROTATION_X_, M_PI/2.0 );
	vectorY *= transformMatrixX90;
	vectorY.dumpInfo();
	vectorY *= transformMatrixX90;
	vectorY.dumpInfo();
	vectorY *= transformMatrixX90;
	vectorY.dumpInfo();

	vector<double> rotX180 { M_PI/1.0 };
	Matrix4D transformMatrixX180( Matrix4D::INIT_ROTATE_ABOUT_X, &rotX180 );
	// old - deprecated: Matrix4D transformMatrixX180( _MATRIX4D_INIT_ROTATION_X_, M_PI/1.0 );
	vectorY *= transformMatrixX180;
	vectorY.dumpInfo();

	cout << "=== Y Rotation =================================================================" << endl;

	Vector3D vectorZ( 0.0f, 2.0f, 1.0f );
	vectorZ.dumpInfo();

	cout << "--------------------------------------------------------------------------------" << endl;

	Matrix4D transformMatrixY( _MATRIX4D_INIT_ROTATION_Y_, M_PI/8.0 );
	transformMatrixX.dumpInfo( true, "transformMatrixY" );

	cout << "--------------------------------------------------------------------------------" << endl;

	vectorZ *= transformMatrixY;
	vectorZ.dumpInfo();
	vectorZ *= transformMatrixY;
	vectorZ.dumpInfo();
	vectorZ *= transformMatrixY;
	vectorZ.dumpInfo();
	vectorZ *= transformMatrixY;
	vectorZ.dumpInfo();

	Matrix4D transformMatrixY90( _MATRIX4D_INIT_ROTATION_Y_, M_PI/2.0 );
	vectorZ *= transformMatrixY90;
	vectorZ.dumpInfo();
	vectorZ *= transformMatrixY90;
	vectorZ.dumpInfo();
	vectorZ *= transformMatrixY90;
	vectorZ.dumpInfo();

	Matrix4D transformMatrixY180( _MATRIX4D_INIT_ROTATION_Y_, M_PI/1.0 );
	vectorZ *= transformMatrixY180;
	vectorZ.dumpInfo();

	cout << "================================================================================" << endl;

	Vector3D zAxis( 0.0f, 0.0f, 1.0f );
	Vector3D inYZ( 0.0f, sin( 34.0*M_PI/180.0 ), cos( 34.0*M_PI/180.0 ) );

	cout << "Test angle: " << angle( inYZ, zAxis ) * 180/M_PI << endl;
	cout << "Test angle: " << inYZ.getAngleToZinYZ() * 180/M_PI << endl;

	cout << "================================================================================" << endl;

	Vector3D vector2( 0, sin( 34*M_PI/180 ), cos( 34*M_PI/180 ) );
	vector2.dumpInfo();

	vector<double> rotX2 { -34*M_PI/180 };
	Matrix4D transformMatrix2( Matrix4D::INIT_ROTATE_ABOUT_X, &rotX2 );
	// old - deprecated: Matrix4D transformMatrix2( _MATRIX4D_INIT_ROTATION_X_, -34*M_PI/180 );
	transformMatrix2.dumpInfo();
	transformMatrix2.dumpInfo( true, (char*) "rx" );

	vector2 *= transformMatrix2;
	vector2.dumpInfo();

	cout << "================================================================================" << endl;

	Vector3D vector3( 1.0, 2.0, 3.0 );
	vector3.dumpInfo();

	Matrix4D transformMatrix3( 4.0, 5.0, 6.0 );
	transformMatrix3.dumpInfo();
	transformMatrix3.dumpInfo( true, (char*) "translate" );

	vector3 *= transformMatrix3;
	vector3.dumpInfo();

	cout << "================================================================================" << endl;

	Vector3D vectorQ1(  cos(30.0*M_PI/180.0),  sin(30.0*M_PI/180.0), 1.0 );
	Vector3D vectorQ12(  0.0, 1.0, 1.0 );
	Vector3D vectorQ2( -cos(30.0*M_PI/180.0),  sin(30.0*M_PI/180.0), 1.0 );
	Vector3D vectorQ3( -cos(30.0*M_PI/180.0), -sin(30.0*M_PI/180.0), 1.0 );
	Vector3D vectorQ34( 0.0, -1.0, 1.0 );
	Vector3D vectorQ4(  cos(30.0*M_PI/180.0), -sin(30.0*M_PI/180.0), 1.0 );
	cout << "Q1  getAngleToXinXY: " << vectorQ1.getAngleToXinXY()  * 180.0 / M_PI << "° to Z: " << vectorQ1.getAngleToZ()  * 180.0 / M_PI << "°" << endl;
	cout << "Q12 getAngleToXinXY: " << vectorQ12.getAngleToXinXY() * 180.0 / M_PI << "° to Z: " << vectorQ12.getAngleToZ() * 180.0 / M_PI << "°" << endl;
	cout << "Q2  getAngleToXinXY: " << vectorQ2.getAngleToXinXY()  * 180.0 / M_PI << "° to Z: " << vectorQ2.getAngleToZ()  * 180.0 / M_PI << "°" << endl;
	cout << "Q3  getAngleToXinXY: " << vectorQ3.getAngleToXinXY()  * 180.0 / M_PI << "° to Z: " << vectorQ3.getAngleToZ()  * 180.0 / M_PI << "°" << endl;
	cout << "Q34 getAngleToXinXY: " << vectorQ34.getAngleToXinXY() * 180.0 / M_PI << "° to Z: " << vectorQ34.getAngleToZ() * 180.0 / M_PI << "°" << endl;
	cout << "Q4  getAngleToXinXY: " << vectorQ4.getAngleToXinXY()  * 180.0 / M_PI << "° to Z: " << vectorQ4.getAngleToZ()  * 180.0 / M_PI << "°" << endl;

	cout << "================================================================================" << endl;

	Vector3D vectorTest = vectorQ4;
	cout << "getAngleToXinXY: " << vectorTest.getAngleToXinXY() * 180.0 / M_PI << "° to Z: " << vectorTest.getAngleToZ() * 180.0 / M_PI << "°" << endl;
	float rotateAngleInXY = vectorTest.getAngleToXinXY();
	float rotateAngleZ = vectorTest.getAngleToZ();
	Matrix4D matAllTransformations( _MATRIX4D_INIT_IDENTITY_, 1.0 );
	matAllTransformations *= Matrix4D ( _MATRIX4D_INIT_ROTATION_Z_, -rotateAngleInXY );
	matAllTransformations *= Matrix4D ( _MATRIX4D_INIT_ROTATION_Y_, -rotateAngleZ );
	matAllTransformations *= Matrix4D ( _MATRIX4D_INIT_ROTATION_Z_, +rotateAngleInXY );
	vectorTest *= matAllTransformations;
	vectorTest.dumpInfo();
	cout << "getAngleToXinXY: " << vectorTest.getAngleToXinXY() * 180.0 / M_PI << "° to Z: " << vectorTest.getAngleToZ() * 180.0 / M_PI << "°" << endl;

	cout << "================================================================================" << endl;
	{
		Vector3D positionVec( 1, 2, 3, 1 );
		vector<double> transVec { 3.0, 4.0, 7.0 };
		Matrix4D translateMat( Matrix4D::INIT_TRANSLATE, &transVec );
		// old - deprecated: Matrix4D translateMat( 3.0, 4.0, 7.0 );
		positionVec *= translateMat;
		positionVec.dumpInfo();
	}
	cout << "================================================================================" << endl;
	{
		Vector3D posVec( 0, 0, 0, 1 );
		Vector3D dirVec( 0, 0, 1, 0 );
		Vector3D pointV( 0, 1, 0, 1 );

		Matrix4D translateMat( posVec, dirVec, M_PI/3.0 );
		pointV *= translateMat;
		pointV.dumpInfo();
		translateMat.dumpInfo( true, "abRot" );

		Matrix4D knowMat( _MATRIX4D_INIT_ROTATION_Z_, M_PI/3.0 );
		knowMat.dumpInfo( true, "kbRot" );
	}
	cout << "================================================================================" << endl;
	{
		Vector3D posVec( 0, 0, 0, 1 );
		Vector3D dirVec( 0, 1, 0, 0 );
		Vector3D pointV( 0, 0, 1, 1 );

		Matrix4D translateMat( posVec, dirVec, M_PI/3.0 );
		pointV *= translateMat;
		pointV.dumpInfo();
		translateMat.dumpInfo( true, "abRot" );

		Matrix4D knowMat( _MATRIX4D_INIT_ROTATION_Y_, M_PI/3.0 );
		knowMat.dumpInfo( true, "kbRot" );
	}
	cout << "================================================================================" << endl;
	{
		Vector3D posVec( 0, 0, 0, 1 );
		Vector3D dirVec( 1, 0, 0, 0 );
		Vector3D pointV( 0, 1, 0, 1 );

		Matrix4D translateMat( posVec, dirVec, M_PI/3.0 );
		pointV *= translateMat;
		pointV.dumpInfo();
		translateMat.dumpInfo( true, "abRot" );

		vector<double> rotXkM { M_PI/3.0 };
		Matrix4D knowMat( Matrix4D::INIT_ROTATE_ABOUT_X, &rotXkM );
		// old - deprecated: Matrix4D knowMat( _MATRIX4D_INIT_ROTATION_X_, M_PI/3.0 );
		knowMat.dumpInfo( true, "kbRot" );
	}
	cout << "================================================================================" << endl;
	{
		double mat1[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
		double mat2[16] = { 3, 7, 5, 1, 2, 9, 10, 16, 11, 8, 4, -1, -4, -5, -9, -1 };
		Matrix4D transMat1( mat1 );
		transMat1.dumpInfo( true, "mat1" );
		Matrix4D transMat2( mat2 );
		transMat2.dumpInfo( true, "mat2" );
		transMat1 *= transMat2;
		transMat1.dumpInfo( true, "mat1new" );
	}
}
