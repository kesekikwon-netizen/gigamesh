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

#include <GigaMesh/mesh/MeshIO/ModelMetaData.h>
#include <iostream>

//! Set Meta-Data as strings.
//! @returns false in case of an error. True otherwise.
ModelMetaData::ModelMetaData()
{
	clearModelMetaStrings();
}

//! Copies the metadata from a given reference.
//!
//! Copies strings and texture information.
//!
//! @returns false in case of an error. True otherwise.
bool ModelMetaData::setModelMeta( /*const*/ ModelMetaData& rOtherModelMeta ) {
	// Strings
	for( unsigned i=0; i<META_STRINGS_COUNT ; i++ ) {
		eMetaStrings currId = static_cast<eMetaStrings>( i );
		std::string currString = rOtherModelMeta.getModelMetaString( currId );
		setModelMetaString( currId, currString );
	}

	// Texture information:
	mHasTextureCoordinates = rOtherModelMeta.hasTextureCoordinates();
//	const std::vector<std::filesystem::path> textureFiles = rOtherModelMeta.getTexturefilesRefSafe();
	const std::vector<std::filesystem::path> textureFiles = rOtherModelMeta.getTexturefilesRef();
	std::copy( textureFiles.begin(), textureFiles.end(), std::back_inserter( mTextureFiles ) );

	// Done.
	return( true );
}

//! Set the given metadata (string).
//!
//! @returns false in case of an error. True otherwise.
bool ModelMetaData::setModelMetaString(
        eMetaStrings       rMetaStrID,        //!< Id of the meta-data string.
        const std::string& rModelMeta         //!< Meta-data content as string.
) {
	mMetaDataStrings[rMetaStrID] = rModelMeta;
	return( true );
}

//! Get Meta-Data as strings.
//! @returns nullptr case of an error.
std::string ModelMetaData::getModelMetaString(
        eMetaStrings rMetaStrID               //!< Id of the meta-data string.
) const {
	if( rMetaStrID == META_STRINGS_COUNT ) {
		std::cerr << "[ModelMetaData::" << __FUNCTION__ << "] ERROR: Id META_STRINGS_COUNT has no content!\n" << std::endl;
		return std::string();
	}
	return( mMetaDataStrings[rMetaStrID] );
}

//! Fetch the name of the Meta-Data strings using an Id.
//! @returns false in case of an error. True otherwise.
bool ModelMetaData::getModelMetaStringName(
        eMetaStrings rMetaStrID,                   //!< Id of the meta-data string.
        std::string& rModelMetaStringName          //!< Name as string of the meta-data string.
) const {
	if( rMetaStrID == META_STRINGS_COUNT ) {
		std::cerr << "[ModelMetaData::" << __FUNCTION__ << "] ERROR: Id META_STRINGS_COUNT has no name!" << std::endl;
		return( false );
	}
	rModelMetaStringName = mMetaDataStringNames[rMetaStrID];
	return( true );
}

//! Fetch the label for the name of a Meta-Data strings using an Id.
//!
//! @returns false in case of an error. True otherwise.
bool ModelMetaData::getModelMetaStringLabel(
        eMetaStrings rMetaStrID,                   //!< Id of the meta-data string.
        std::string& rModelMetaStringLabel         //!< Name as string of the meta-data string.
) const {
	if( rMetaStrID == META_STRINGS_COUNT ) {
		std::cerr << "[ModelMetaData::" << __FUNCTION__ << "] ERROR: Id META_STRINGS_COUNT has no name!" << std::endl;
		return( false );
	}
	rModelMetaStringLabel = mMetaDataStringLabels[rMetaStrID];
	return( true );
}

//! Fetch the Id of the Meta-Data strings using its name as string.
//! @returns false in case of an error or no Id found. True otherwise.
bool ModelMetaData::getModelMetaStringId(
        const std::string& rModelMetaStringName,   //!< Id of the meta-data string.
        eMetaStrings& rMetaStrID                   //!< Name as string of the meta-data string.
) const {
	for( unsigned i=0; i<META_STRINGS_COUNT ; i++ ) {
        if( rModelMetaStringName == mMetaDataStringNames[i]) {
            //rMetaStrID = static_cast<eMetaStrings>( i );
            //! the constant value is accessed by a modulo operation,
            //! since this allows us to extend the mMetaDataStringNames with alternative names.
            //! Therefore we can add external name conventions e.g. defined by 3D printer companies
            rMetaStrID = static_cast<eMetaStrings>( i%12 );
			return( true );
		}
	}
	// Nothing found
	return( false );
}

//! Clear and reset meta-data.
//! @returns false in case of an error or no Id found. True otherwise.
bool ModelMetaData::clearModelMetaStrings() {
	// Initialize strings holding meta-data
	for( unsigned i=0; i<META_STRINGS_COUNT ; i++ ) {
		mMetaDataStrings[i].clear();
		mMetaDataStringNames[i] = "ERROR: Not Set!";
	}

	// Initialze names of the strings holding meta-data
	mMetaDataStringNames[META_MODEL_ID]         = "ModelID";
	mMetaDataStringNames[META_MODEL_MATERIAL]   = "ModelMaterial";
	mMetaDataStringNames[META_FILENAME]         = "ModelFileName";
	mMetaDataStringNames[META_REFERENCE_WEB]    = "ModelReferenceWeb";
	mMetaDataStringNames[META_MODEL_UNIT]           = "ModelUnit";
	mMetaDataStringNames[META_MODEL_UUID]           = "ModelUUID";
	mMetaDataStringNames[META_MODEL_UUID_PARENT]    = "ModelUUIDParent";
	mMetaDataStringNames[META_MODEL_UUID_PROCESS]   = "ModelUUIDProcess";
	mMetaDataStringNames[META_MODEL_CREATORS]       = "ModelCreators";
	mMetaDataStringNames[META_MODEL_CONTRIBUTORS]   = "ModelContributors";
	mMetaDataStringNames[META_TEXTUREFILE]      = "TextureFile";
    //start with the alternatives
    mMetaDataStringNames[META_MODEL_UNIT+12]           = "Unit:";

	// Initialze labels for the names of the strings holding meta-data
	mMetaDataStringLabels[META_MODEL_ID]             = "ID of the model";
	mMetaDataStringLabels[META_MODEL_MATERIAL]       = "Material of the object";
	mMetaDataStringLabels[META_FILENAME]             = "Filename";
	mMetaDataStringLabels[META_REFERENCE_WEB]        = "Reference / URL";
	mMetaDataStringLabels[META_MODEL_UNIT]           = "Unit";
	mMetaDataStringLabels[META_MODEL_UUID]           = "UUID of the model";
	mMetaDataStringLabels[META_MODEL_UUID_PARENT]    = "UUID of the model's parent";
	mMetaDataStringLabels[META_MODEL_UUID_PROCESS]   = "UUID of the last applied function";
	mMetaDataStringLabels[META_MODEL_CREATORS]       = "Creator(s) of the model";
	mMetaDataStringLabels[META_MODEL_CONTRIBUTORS]   = "Contributor(s)";
	mMetaDataStringLabels[META_TEXTUREFILE]          = "TextureFile";

	// Done.
	return( true );
}

bool ModelMetaData::hasTextureCoordinates() const
{
	return mHasTextureCoordinates;
}

void ModelMetaData::setHasTextureCoordinates(bool hasTextureCoordinates)
{
	mHasTextureCoordinates = hasTextureCoordinates;
}

TextureHandle ModelMetaData::addTextureName(const std::filesystem::path& textureName)
{
	for(size_t i = 0; i<mTextureFiles.size(); ++i)
	{
		if(mTextureFiles[i] == textureName)
			return i;
	}

	mTextureFiles.push_back(textureName);
	return mTextureFiles.size() - 1;
}

std::filesystem::path ModelMetaData::getTextureName(TextureHandle id) const
{
	if(id < mTextureFiles.size())
		return mTextureFiles[id];

	return std::filesystem::path();
}

bool ModelMetaData::hasTextureFiles() const
{
	return !mTextureFiles.empty();
}

std::vector<std::filesystem::path>& ModelMetaData::getTexturefilesRef()
{
	return mTextureFiles;
}

//const std::vector<std::filesystem::path> &ModelMetaData::getTexturefilesRefSafe() const
//{
//	return mTextureFiles;
//}
