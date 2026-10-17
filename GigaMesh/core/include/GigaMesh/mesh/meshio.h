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

#ifndef MESHIO_H
#define MESHIO_H

#include "meshseedext.h"
#include <GigaMesh/mesh/MeshIO/ModelMetaData.h>

#include <list>

//!
//! \brief Class for handling file access. (Layer 0)
//!
//! File access (for meshes). It is also a parent of Mesh.
//!
//! Layer 0
//!

class MeshIO : public MeshSeedExt {

	public:
		MeshIO();
		~MeshIO() = default;

		//! Simple struct for parsing and passing points and vectors in R3
		struct grVector3ID {
			unsigned long mId;  //! Unique identifier
			double mX;   //! x-coordinate
			double mY;   //! y-coordinate
			double mZ;   //! z-coordinate
		};

		enum eExportFlags {
			EXPORT_BINARY,      //!< Binary or ASCII for those available in both formats.
			EXPORT_VERT_COLOR,  //!< Export color per vertex.
			EXPORT_VERT_NORMAL, //!< Export normal per vertex.
			EXPORT_VERT_FLAGS,  //!< Flags (bitarray) per vertex.
			EXPORT_VERT_LABEL,  //!< Label ID of the vertex.
			EXPORT_VERT_FTVEC,  //!< Feature vector per vertex, when present.
			EXPORT_POLYLINE,    //!< Export polylines.
			EXPORT_TEXTURE_COORDINATES, //!< Export per face texture coordinates
			EXPORT_TEXTURE_FILE,//!< Save texture file next to mesh
			EXPORT_FLAG_COUNT   //!< Number of flags available for export.
		};

		// Read:
		virtual bool readFile( const std::filesystem::path& rFileName, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps );
		virtual bool readIsRegularGrid( bool* rIsGrid );


		bool importTEXMap( const std::filesystem::path& rFileName, int* rNrLines, uint64_t** rRefToPrimitves, unsigned char** rTexMap );
		bool importNormals( const std::filesystem::path& rFileName, std::vector<grVector3ID> *rNormals );

		// Write:
		virtual bool setFlagExport( eExportFlags rFlag, bool rSetTo  );
        virtual bool writeFileUserInteract(const bool asLegacy = false);

	public: //! \todo this should be at least 'protected'.
		        bool writeFilePrimProps( const std::filesystem::path& rFileName,
		                                 std::vector<sVertexProperties>& rVertexProps,
		                                 std::vector<sFaceProperties>& rFaceProps );

	public:
		ModelMetaData& getModelMetaDataRef();

		// Provide Information:
		virtual std::filesystem::path getFileExtension() const;
		virtual std::filesystem::path getBaseName() const;
		virtual std::filesystem::path getFileLocation() const;
		virtual std::filesystem::path getFullName() const;

		virtual bool writeIcoNormalSphereData(const std::filesystem::path& rFilename, const std::list<sVertexProperties>& rVertexProps, int subdivisions, bool sphereCoordinates = false);

	private:
		std::array<bool, EXPORT_FLAG_COUNT>   mExportFlags; //!< Handles export options.
		bool   mSystemIsBigEndian; //!< Flag for proper Byte ordering during write/read.

		// File properties
		std::filesystem::path mFileNameFull;      //!< Full name and path of the current file.

		// Meta-Data NEW with vector of strings
		ModelMetaData mModelMetaData;

};
#endif
