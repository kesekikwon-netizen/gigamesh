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

#include "RegularGridTxtReader.h"
#include <fstream>
#include <string>
#include <algorithm>
#include <locale>

using namespace std;

enum eTxtRegularTags {
	TXT_REGULAR_IGNORE,  //!< Tags for tags to be ignored
	TXT_REGULAR_X,       //!< Tag for x-coordinate
	TXT_REGULAR_Y,       //!< Tag for y-coordinate
	TXT_REGULAR_Z,       //!< Tag for z-coordinate
	TXT_REGULAR_UNKNOWN, //!< Tag for unknown tags.
	TXT_REGULAR_COUNT
};

bool RegularGridTxtReader::readFile(const std::filesystem::path& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed)
{
	// Functionality:
	ifstream fp( rFilename );
	fp.imbue(std::locale("C"));
	if( !fp.is_open() ) {
		cerr << "[RegularGridTxtReader::" << __FUNCTION__ << "] Could not open file: '" << rFilename << "'." << endl;
		return false;
	} else {
		cout << "[RegularGridTxtReader::" << __FUNCTION__ << "] File opened: '" << rFilename << "'." << endl;
	}

	// Read header line with tags
	vector<int> headerTags;
	char nextByte = fp.peek();
	while( ( nextByte != '\n' ) && ( nextByte != '\r' ) && ( nextByte != -1 ) ) {
		string headerTag;
		fp >> headerTag;
		std::transform( headerTag.begin(), headerTag.end(), headerTag.begin(), ::toupper );
		//cout << "[RegularGridTxtReader::" << __FUNCTION__ << "] Header tag: " << headerTag << endl;
		if( headerTag == "POINTID" ) {
			cout << "[RegularGridTxtReader::" << __FUNCTION__ << "] Header tag known, but ignored: " << headerTag << endl;
			headerTags.push_back( TXT_REGULAR_IGNORE );
		} else if( headerTag == "X_COORD" ) {
			headerTags.push_back( TXT_REGULAR_X );
		} else if( headerTag == "Y_COORD" ) {
			headerTags.push_back( TXT_REGULAR_Y );
		} else if( headerTag == "GRID_CODE" ) {
			headerTags.push_back( TXT_REGULAR_Z );
		} else {
			cerr << "[RegularGridTxtReader::" << __FUNCTION__ << "] ERROR: Unknown header tag: " << headerTag << "!" << endl;
			headerTags.push_back( TXT_REGULAR_UNKNOWN );
		}
		nextByte = fp.peek();
	}

	// Read the file:
	vector<double> coordX;
	vector<double> coordY;
	vector<double> coordZ;
	vector<int>::iterator itTag;
	while( fp.good() ) {

		bool nextWhiteSpace = true;
		do {
			nextByte = fp.peek();
			if( ( nextByte == ' ' ) || ( nextByte == '\t' ) ) {
				fp.get();
			} else {
				nextWhiteSpace = false;
			}
		} while( nextWhiteSpace );

		string strIgnore;
		for( itTag=headerTags.begin(); itTag!=headerTags.end(); itTag++ ) {
			double someCoord;
			//int someInt;
			switch( *itTag ) {
				case TXT_REGULAR_IGNORE:
					fp >> strIgnore;
					cout << "[" << strIgnore << "] ";
					break;
				case TXT_REGULAR_X:
					fp >> someCoord;
					coordX.push_back( someCoord );
					cout << someCoord << " ";
					break;
				case TXT_REGULAR_Y:
					fp >> someCoord;
					coordY.push_back( someCoord );
					cout << someCoord << " ";
					break;
				case TXT_REGULAR_Z:
					fp >> someCoord;
					coordZ.push_back( someCoord );
					cout << someCoord << " ";
					break;
				default: // ignore
					fp >> strIgnore;
					cout << "{" << strIgnore << "} ";
			}
		}
		cout << endl;
		// Read the rest of the line - typically \n
		getline( fp, strIgnore );
		cout << "-" << strIgnore << "-" << endl;
	}

	cout << "[RegularGridTxtReader::" << __FUNCTION__ << "] X-coordinates: " << coordX.size() << endl;
	cout << "[RegularGridTxtReader::" << __FUNCTION__ << "] Y-coordinates: " << coordY.size() << endl;
	cout << "[RegularGridTxtReader::" << __FUNCTION__ << "] Z-coordinates: " << coordZ.size() << endl;

	fp.close();
	return true;
}
