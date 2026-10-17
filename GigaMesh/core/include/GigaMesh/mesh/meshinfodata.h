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

#ifndef MESHINFODATA_H
#define MESHINFODATA_H

#include <string>
#include <filesystem>

class MeshInfoData {
	public:
		MeshInfoData();
		~MeshInfoData() = default;

	public:
		enum eMeshPropertyString {
			FILENAME,                      //!< Filename with path and extension.
			MODEL_ID,                      //!< Meta-data: Id of the object e.g. inventory number.
			MODEL_MATERIAL,                //!< Meta-data: Material(s) of the acquired object e.g. clay, original or gypsum, cast copy.
			MODEL_WEBREFERENCE,            //!< Meta-data: Web-Reference e.g. CDLI url for a cuneiform tablet.
			STRING_COUNT                   //!< Number of elements.
		};
	public:
		std::string mStrings[STRING_COUNT];
	private:
		std::string mStringName[STRING_COUNT];

	public:
		enum eMeshPropertyULongCount {
			VERTICES_TOTAL,                //!< Number of vertices.
			VERTICES_NAN,
			VERTICES_NORMAL_LEN_NORMAL,
			VERTICES_SOLO,
			VERTICES_POLYLINE,
			VERTICES_BORDER,
			VERTICES_NONMANIFOLD,
			VERTICES_SINGULAR,             //!< AKA double cone
			VERTICES_ON_INVERTED_EDGE,
			VERTICES_PART_OF_ZERO_FACE,
			VERTICES_SYNTHETIC,
			VERTICES_MANUAL,
			VERTICES_CIRCLE_CENTER,
			VERTICES_SELECTED,
			VERTICES_FUNCVAL_FINITE,
			VERTICES_FUNCVAL_LOCAL_MIN,
			VERTICES_FUNCVAL_LOCAL_MAX,
			FACES_TOTAL,                   //!< Number of faces.
			FACES_SOLO,
			FACES_BORDER,
			FACES_BORDER_THREE_VERTICES,
			FACES_BORDER_BRDIGE_TRICONN,
			FACES_BORDER_BRDIGE,
			FACES_BORDER_DANGLING,
			FACES_MANIFOLD,
			FACES_NONMANIFOLD,
			FACES_STICKY,
			FACES_ZEROAREA,
			FACES_INVERTED,
			FACES_SELECTED,
			FACES_WITH_SYNTH_VERTICES,     //!< Number of faces having only synthetic vertices i.e. all three vertices are synthetic.
            FACES_SELFINTERSECTED,         //! Number of faces which intersect a other face (self-intersection)
			CONNECTED_COMPONENTS,          //!< Number of connected components
			ULONG_COUNT,                   //!< Number of elements.
		};
	public:
		uint64_t mCountULong[ULONG_COUNT];
	private:
		std::string mCountULongName[ULONG_COUNT];

	public:
		enum eMeshPropertyDouble {
			BOUNDINGBOX_MIN_X,                  //!< Minimum coordinate in x.
			BOUNDINGBOX_MIN_Y,                  //!< Minimum coordinate in y.
			BOUNDINGBOX_MIN_Z,                  //!< Minimum coordinate in z.
			BOUNDINGBOX_MAX_X,                  //!< Maximum coordinate in x.
			BOUNDINGBOX_MAX_Y,                  //!< Maximum coordinate in y.
			BOUNDINGBOX_MAX_Z,                  //!< Maximum coordinate in z.
			BOUNDINGBOX_WIDTH,                  //!< Bounding box width.
			BOUNDINGBOX_HEIGHT,                 //!< Bounding box height.
			BOUNDINGBOX_THICK,                  //!< Bounding box thickness.
			FACES_AREA_SMALLEST,                //!< Area of the smallest face.
			FACES_AREA_LARGEST,                 //!< Area of the largest face.
			TOTAL_AREA,                         //!< Total area.
			TOTAL_VOLUME_DX,                    //!< Total volume in dx.
			TOTAL_VOLUME_DY,                    //!< Total volume in dy.
			TOTAL_VOLUME_DZ,                    //!< Total volume in dz.
			DOUBLE_COUNT                        //!< Number of elements.
		};
	public:
		double mCountDouble[DOUBLE_COUNT];
	private:
		std::string mmCountDoubleName[DOUBLE_COUNT];

	public:
		void reset();

		// Fetch formatted text
		bool getMeshInfoTTL(  std::string& rInfoTTL  );
		bool getMeshInfoHTML( std::string& rInfoHTML );
		bool getMeshInfoJSON( std::string& rInfoJSON );
		bool getMeshInfoXML(  std::string& rInfoXML  );

		// Write formatted text
		bool writeMeshInfo( std::filesystem::path rFilenameInfo, bool rReplace=true );
		bool writeMeshInfoProcess( const MeshInfoData& rMeshInfoPrevious, const std::filesystem::path& rFileNameOut,
		                           const std::string& rFunctionExecuted, const uint64_t& rIterationCount,
		                           const std::chrono::system_clock::time_point& rTimeStart, 
		                           const std::chrono::system_clock::time_point& rTimeStop );

		// Property names
		bool getMeshInfoPropertyName( const MeshInfoData::eMeshPropertyString     rPropId, std::string& rPropName );
		bool getMeshInfoPropertyName( const MeshInfoData::eMeshPropertyULongCount rPropId, std::string& rPropName );
		bool getMeshInfoPropertyName( const MeshInfoData::eMeshPropertyDouble     rPropId, std::string& rPropName );
};

#endif // MESHINFODATA_H
