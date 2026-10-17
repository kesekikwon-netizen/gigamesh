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

#ifndef MODELMETADATA_H
#define MODELMETADATA_H

#include <string>
#include <array>
#include <vector>
#include <filesystem>

using TextureHandle = unsigned char;

class ModelMetaData
{
	public:
		ModelMetaData();

		// Meta-Data as strings
		enum eMetaStrings {
			META_MODEL_ID,             //!< Given Id. Typically an inventory number.
			META_MODEL_MATERIAL,       //!< Material of the real world object. e.g. 'original, clay' for 3D-scans of cuneiform tablets.
			META_REFERENCE_WEB,        //!< Http URL with name format style '[URL|Name]'.
            META_MODEL_UNIT,          //!< As PLY and OBJ are unitless, the type of unit has to be kept in the meta-data.
			META_MODEL_UUID,           //!< UUID of this model.
			META_MODEL_UUID_PARENT,    //!< UUID of the parent model.
			META_MODEL_UUID_PROCESS,   //!< UUID of the process creating this model.
			META_MODEL_CREATORS,       //!< Linked Data: Creators.
			META_MODEL_CONTRIBUTORS,   //!< Linked Data: Contributors.
			META_FILENAME,             //!< Filename, when loaded.
			META_TEXTUREFILE,          //!< Meshlab texturefile stored in ply: e.g. "comment TextureFile texture.png"
            META_STRINGS_COUNT = 24    //!< Total number of strings for meta-data. It's the double because to assign alternative strings by modulo. see getModelMetaString
		};

		bool         setModelMeta( /*const*/ ModelMetaData& rOtherModelMeta );
		bool         setModelMetaString( eMetaStrings rMetaStrID, const std::string& rModelMeta );
		std::string  getModelMetaString( eMetaStrings rMetaStrID ) const;
		bool         getModelMetaStringName( eMetaStrings rMetaStrID, std::string& rModelMetaStringName ) const;
		bool         getModelMetaStringLabel( eMetaStrings rMetaStrID, std::string& rModelMetaStringLabel ) const;
		bool         getModelMetaStringId( const std::string& rModelMetaStringName, eMetaStrings& rMetaStrID ) const;
		bool         clearModelMetaStrings();

		// Texture mapping
		[[nodiscard]] bool hasTextureCoordinates() const;
		void setHasTextureCoordinates(bool hasTextureCoordinates);
		//.
		TextureHandle addTextureName(const std::filesystem::path& textureName);
		[[nodiscard]] std::filesystem::path getTextureName(TextureHandle id) const;
		[[nodiscard]] bool hasTextureFiles() const;
		std::vector<std::filesystem::path>& getTexturefilesRef();
//		const std::vector<std::filesystem::path>& getTexturefilesRefSafe() const;

private:
		std::array<std::string, META_STRINGS_COUNT> mMetaDataStrings;         //!< Meta-Data contents
		std::array<std::string, META_STRINGS_COUNT> mMetaDataStringNames;     //!< Meta-Data names used e.g. in the PLY header.
		std::array<std::string, META_STRINGS_COUNT> mMetaDataStringLabels;    //!< Meta-Data labels i.e. more human appealing than the names.

		std::vector<std::filesystem::path> mTextureFiles;                     //!< Stores referenced texture-files
		bool mHasTextureCoordinates = false;                                  //!< Stores if the mesh has texture-coordinates
};

#endif // MODELMETADATA_H
