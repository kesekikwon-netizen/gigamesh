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

#include "TxtReader.h"
#include <fstream>
#include <string>
#include <clocale>
#include <list>

using namespace std;


bool TxtReader::readFile(const std::filesystem::path& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed)
{
	//! Reads a simple ASCII file having X,Y,Z and R,G,B per line.

	auto oldLocale = std::setlocale(LC_NUMERIC,"");
	std::setlocale(LC_NUMERIC,"C");


	fstream filestr;
	string  lineToParse;
	int     linesRead     = 1;
	uint64_t     linesVertices = 0;
	bool    rgbFloat = true;

	int timeStart, timeStop; // for performance mesurement

	timeStart = clock();
	filestr.open( rFilename, fstream::in );

	int   texR, texG, texB;
	float texRf, texGf, texBf;
	float someCoordX, someCoordY, someCoordZ; // for testing sscanf and nothing else.
	int  varsParsed;

	getline( filestr, lineToParse );
	varsParsed = sscanf( lineToParse.c_str(), "%f %f %f %i %i %i", &someCoordX, &someCoordY, &someCoordZ, &texR, &texG, &texB );
	if( varsParsed == 6 ) {
		rgbFloat = false;
		cout << "[TxtReader::readTXT] RGB is int." << endl;
	} else {
		cout << "[TxtReader::readTXT] RGB is float or not existing." << endl;
	}
	filestr.seekg( 0, ios::beg );

	struct Pos{
		Pos(float X, float Y, float Z) : x(X), y(Y), z(Z){ }
		float x;
		float y;
		float z;
	};

	struct Col {
		Col(unsigned char R, unsigned char G, unsigned char B) : r(R), g(G), b(B) {}
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	std::list<Pos> positions;
	std::list<Col> colors;

	while( !filestr.eof() ) {
		if( rgbFloat ) {
			varsParsed = sscanf( lineToParse.c_str(), "%f %f %f %f %f %f", &someCoordX, &someCoordY, &someCoordZ, &texRf, &texGf, &texBf );
			positions.emplace_back(Pos(someCoordX,someCoordY,someCoordZ));
			colors.emplace_back(Col(texRf * 255.0f, texGf * 255.0f, texBf * 255.0f));
		} else {
			varsParsed = sscanf( lineToParse.c_str(), "%f %f %f %i %i %i", &someCoordX, &someCoordY, &someCoordZ, &texR, &texG, &texB );
			positions.emplace_back(Pos(someCoordX,someCoordY,someCoordZ));
			colors.emplace_back(Col(texR, texG, texB));
		}
		if( ( varsParsed == 3 ) || ( varsParsed == 6 ) ) {
			linesVertices++;
		} else {
			cerr << "[TxtReader] Problem in line " << linesRead << " parsed: " << varsParsed << " - " << lineToParse << endl;
		}
		getline( filestr, lineToParse );
		linesRead++;
	}

	timeStop  = clock();
	cout << "[TxtReader] read XYZ/TXT 1st parse:      " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	cout << "[TxtReader] " << linesRead << " lines read from " << rFilename << "." << endl;

	// allocate memory:
	rVertexProps.resize( linesVertices );
	// init vertex coordinates and function value
	for( uint64_t vertexIdx=0; vertexIdx<linesVertices; vertexIdx++ ) {
		// don't init function value with NaN, as this results in performance issues by repeatingly checking min/max values in MeshGL::getFuncValMinMaxUser.
		rVertexProps.at( vertexIdx ).mCoordX  = _NOT_A_NUMBER_DBL_;
		rVertexProps.at( vertexIdx ).mCoordY  = _NOT_A_NUMBER_DBL_;
		rVertexProps.at( vertexIdx ).mCoordZ  = _NOT_A_NUMBER_DBL_;
		// init function value
		rVertexProps.at( vertexIdx ).mFuncVal = 0.0;
		// init colors
		rVertexProps.at( vertexIdx ).mColorRed = 187;
		rVertexProps.at( vertexIdx ).mColorGrn = 187;
		rVertexProps.at( vertexIdx ).mColorBle = 187;
		rVertexProps.at( vertexIdx ).mColorAlp = 255;
	}

	// parse to arrays:
	timeStart = clock();

	uint64_t vertexIdx = 0;
	for( const auto& pos : positions ) {
		rVertexProps.at( vertexIdx ).mCoordX  = pos.x;
		rVertexProps.at( vertexIdx ).mCoordY  = pos.y;
		rVertexProps.at( vertexIdx ).mCoordZ  = pos.z;
		++vertexIdx;
	}
	vertexIdx = 0;
	for( const auto& col : colors ) {
		rVertexProps.at( vertexIdx ).mColorRed = col.r;
		rVertexProps.at( vertexIdx ).mColorGrn = col.g;
		rVertexProps.at( vertexIdx ).mColorBle = col.b;
		++vertexIdx;
	}

	timeStop  = clock();
	std::cout << "[TxtReader] read XYZ/TXT 2nd parse:      " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds. " << std::endl;

	std::setlocale( LC_NUMERIC, oldLocale );
	return( true );
}
