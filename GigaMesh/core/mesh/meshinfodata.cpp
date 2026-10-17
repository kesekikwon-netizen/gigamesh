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

#include <GigaMesh/mesh/meshinfodata.h>

#include <sstream>      // std::stringstream
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <iostream>     // std::cout
#include <string>       // std::string, std::to_string
#include <math.h>       // sqrt
#include <fstream>      // filestreams
#include <cctype>       // std::tolower
#include <thread>       // std::thread
#include <sstream>
#include <random>

#include <GigaMesh/mesh/gmcommon.h>
#include <GigaMesh/logging/Logging.h>
#include <GigaMesh/getuserandhostname.h>

//! Constructer calls MeshInfoData::reset() and sets the names for the enumerators.
MeshInfoData::MeshInfoData() {
	// String names
	mStringName[FILENAME]             = "Filename";
	mStringName[MODEL_ID]             = "Model Id";
	mStringName[MODEL_MATERIAL]       = "Model Material";
	mStringName[MODEL_WEBREFERENCE]   = "Web-Reference";

	// Unsigned long names
	mCountULongName[VERTICES_TOTAL] = "Total number of vertices";
	mCountULongName[VERTICES_NAN] = "Vertices not-a-number";
	mCountULongName[VERTICES_NORMAL_LEN_NORMAL] = "Vertex normal vector length normal";
	mCountULongName[VERTICES_SOLO] = "Vertices solo";
	mCountULongName[VERTICES_POLYLINE] = "Vertices of polylines";
	mCountULongName[VERTICES_BORDER] = "Border vertices";
	mCountULongName[VERTICES_NONMANIFOLD] = "Vertices non-manifold";
	mCountULongName[VERTICES_SINGULAR] = "Vertices singular";
	mCountULongName[VERTICES_ON_INVERTED_EDGE] = "Vertices along an inverted edge";
	mCountULongName[VERTICES_PART_OF_ZERO_FACE] = "Vertices part of a zero area face";
	mCountULongName[VERTICES_SYNTHETIC] = "Vertices synthetic";
	mCountULongName[VERTICES_MANUAL] = "Vertices manually added";
	mCountULongName[VERTICES_CIRCLE_CENTER] = "Vertices circle center";
	mCountULongName[VERTICES_SELECTED] = "Vertices selected";
	mCountULongName[VERTICES_FUNCVAL_FINITE] = "Vertices with finite function value";
	mCountULongName[VERTICES_FUNCVAL_LOCAL_MIN] = "Vertices with local function value minimum";
	mCountULongName[VERTICES_FUNCVAL_LOCAL_MAX] = "Vertices with local function value maximum";
	mCountULongName[FACES_TOTAL] = "Total number of faces";
	mCountULongName[FACES_SOLO] = "Solo faces";
	mCountULongName[FACES_BORDER] = "Border faces";
	mCountULongName[FACES_BORDER_THREE_VERTICES] = "Faces with three border vertices";
	mCountULongName[FACES_BORDER_BRDIGE_TRICONN] = "Border faces bridge tri-connection";
	mCountULongName[FACES_BORDER_BRDIGE] = "Bridge border faces";
	mCountULongName[FACES_BORDER_DANGLING] = "Dangling border faces";
	mCountULongName[FACES_MANIFOLD] = "Manifold faces";
	mCountULongName[FACES_NONMANIFOLD] = "Non-manifold faces";
	mCountULongName[FACES_STICKY] = "Sticky faces";
	mCountULongName[FACES_ZEROAREA] = "Faces with zero area";
	mCountULongName[FACES_INVERTED] = "Inverted Faces";
	mCountULongName[FACES_SELECTED] = "Selected Faces";
	mCountULongName[FACES_WITH_SYNTH_VERTICES] = "Faces with synthetic vertices";
    mCountULongName[FACES_SELFINTERSECTED] = "Faces which intersect other faces (self-intersection)";
	mCountULongName[CONNECTED_COMPONENTS] = "Connected components";

	// Double names
	mmCountDoubleName[BOUNDINGBOX_MIN_X]    = "Minimum x coordinate";
	mmCountDoubleName[BOUNDINGBOX_MIN_Y]    = "Minimum y coordinate";
	mmCountDoubleName[BOUNDINGBOX_MIN_Z]    = "Minimum z coordinate";
	mmCountDoubleName[BOUNDINGBOX_MAX_X]    = "Maximum x coordinate";
	mmCountDoubleName[BOUNDINGBOX_MAX_Y]    = "Maximum y coordinate";
	mmCountDoubleName[BOUNDINGBOX_MAX_Z]    = "Maximum z coordinate";
	mmCountDoubleName[BOUNDINGBOX_WIDTH]    = "Bounding box width";
	mmCountDoubleName[BOUNDINGBOX_HEIGHT]   = "Bounding box height";
	mmCountDoubleName[BOUNDINGBOX_THICK]    = "Bounding box thickness";
	mmCountDoubleName[FACES_AREA_SMALLEST]  = "Area largest face";
	mmCountDoubleName[FACES_AREA_LARGEST]   = "Area smallest face";
	mmCountDoubleName[TOTAL_AREA]           = "Total area";
	mmCountDoubleName[TOTAL_VOLUME_DX]      = "Total volume (dx)";
	mmCountDoubleName[TOTAL_VOLUME_DY]      = "Total volume (dy)";
	mmCountDoubleName[TOTAL_VOLUME_DZ]      = "Total volume (dz)";

	reset();
}

//! Reset all values.
void MeshInfoData::reset() {
	for( std::string& countValue : this->mStrings ) {
		countValue = "";
	}
	for( auto& countValue : this->mCountULong ) {
		countValue = _NOT_A_NUMBER_ULONG_;
	}
	for( double& countValue : this->mCountDouble ) {
		countValue = _NOT_A_NUMBER_DBL_;
	}
}

unsigned int random_char() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

std::string generate_hex(const unsigned int len) {
    std::stringstream ss;
    for (auto i = 0; i < len; i++) {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

std::string generate_UUID(){
    return generate_hex(8)+"-"+generate_hex(4)+"-"+generate_hex(4)+"-"+generate_hex(12);
}

std::string urlEncode(std::string str){
    std::string new_str = "";
    char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i=0;i<len;i++){
        c = chars[i];
        ic = c;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            new_str += c;
        }else {
            sprintf(bufHex,"%X",c);
            if(ic < 16) 
                new_str += "%0"; 
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
 }

//! Write mesh info to given file.
//! @returns false in case of an error. True otherwise.
bool MeshInfoData::writeMeshInfo( 
        std::filesystem::path rFilenameInfo, 
        bool rReplace
) {
	std::string fileExtension = rFilenameInfo.extension().string();

	if( fileExtension.empty() ) {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] No extension/type for file '" << rFilenameInfo << "' specified!\n";
		return( false );
	}

	for( char& character : fileExtension ) {
		character = std::tolower(character);
	}
	if( fileExtension.at(0) == '.') {
		fileExtension.erase( fileExtension.begin() );
	}
	LOG::debug() << "[MeshInfoData::" << __FUNCTION__ << "] extension: " << fileExtension << "\n";

	std::string infoStr;
	if( fileExtension == "html" ) {
		// Fetch HTML string
		if( !this->getMeshInfoHTML( infoStr ) ) {
			std::wcerr <<  "[MeshInfoData::" << __FUNCTION__ << "] ERROR: Could not fetch the HTML infos!" << std::endl;
			return( false );
		}
	} else if ( fileExtension == "ttl" ) {
		// Fetch TTL string
		if( !this->getMeshInfoTTL( infoStr ) ) {
			std::wcerr <<  "[MeshInfoData::" << __FUNCTION__ << "] ERROR: Could not fetch the TTL infos!" << std::endl;
			return( false );
		}
	} else if ( fileExtension == "json" ) {
		// Fetch JSON string
		if( !this->getMeshInfoJSON( infoStr ) ) {
			std::wcerr <<  "[MeshInfoData::" << __FUNCTION__ << "] ERROR: Could not fetch the JSON infos!" << std::endl;
			return( false );
		}
	} else if ( fileExtension == "xml" ) {
		// Fetch XML string
		if( !this->getMeshInfoXML( infoStr ) ) {
			std::wcerr <<  "[MeshInfoData::" << __FUNCTION__ << "] ERROR: Could not fetch the XML infos!" << std::endl;
			return( false );
		}
	} else {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] No valid extension/type for file '" << rFilenameInfo << "' specified!\n";
		return( false );
	}

	//!\todo implement overwrite flag
	if( rReplace ) {
		// check ....
	}

	// Write to file.
	std::fstream fileStrOut;
	fileStrOut.open( rFilenameInfo, std::fstream::out );
	if( fileStrOut.is_open() ) {
		fileStrOut << infoStr;
		fileStrOut.close();
	} else {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] Could not write file '" << rFilenameInfo << "'!\n";
		return( false );
	}

	return( true );
}

//! Writes Meta-Data about an executed process.
//! 
//! @returns false in case of an error. True otherwise.
bool MeshInfoData::writeMeshInfoProcess(
        const MeshInfoData&                            rMeshInfoPrevious,
        const std::filesystem::path&                   rFileNameOut,
        const std::string&                             rFunctionExecuted,
        const uint64_t&                                rIterationCount,
        const std::chrono::system_clock::time_point&   rTimeStart,
        const std::chrono::system_clock::time_point&   rTimeStop
) {
	// Sanity check
	//----------------------------------------------------------
	if( rFileNameOut.empty() ) {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] Empty filename given!\n";
		return( false );
	}

	// Set filename for meta-data
	//----------------------------------------------------------
	std::filesystem::path fileNameOutMeta = rFileNameOut;
	fileNameOutMeta.replace_extension( std::string( rFunctionExecuted + ".txt" ) );
	std::filesystem::path fileNameOutMetaTTL = rFileNameOut;
	fileNameOutMetaTTL.replace_extension( std::string( rFunctionExecuted + ".ttl" ) );
	std::filesystem::path fileNameOutMetaXML = rFileNameOut;
	fileNameOutMetaXML.replace_extension( std::string( rFunctionExecuted + ".xml" ) );
	std::filesystem::path fileNameOutMetaJSON = rFileNameOut;
	fileNameOutMetaJSON.replace_extension( std::string( rFunctionExecuted + ".json" ) );
    
	// Pre-compute information, which is (sort of) redundant
	//----------------------------------------------------------
	int64_t vertexCountDiff       = ( static_cast<long>(mCountULong[MeshInfoData::VERTICES_TOTAL])  - static_cast<long>(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_TOTAL])  );
	int64_t faceCountDiff         = ( static_cast<long>(mCountULong[MeshInfoData::FACES_TOTAL])     - static_cast<long>(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_TOTAL])     );
	int64_t vertexBorderCountDiff = ( static_cast<long>(mCountULong[MeshInfoData::VERTICES_BORDER]) - static_cast<long>(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_BORDER]) );
	time_t startTime = std::chrono::system_clock::to_time_t( rTimeStart );
	time_t endTime   = std::chrono::system_clock::to_time_t( rTimeStop );
	double timeElapsed = ( std::chrono::duration<double>( rTimeStop - rTimeStart ) ).count();

	// Fetch username and host for the technical meta-data
	//----------------------------------------------------------
	std::string userName( "unknown" );
	std::string hostName( "unknown" );
	getUserAndHostName( userName, hostName );

	// Fetch username and host for the technical meta-data
	//----------------------------------------------------------
	//! \todo Proper UUID generation.
	// getModelMetaString( eMetaStrings rMetaStrID )

	// Open file for meta-data
	//----------------------------------------------------------
	std::string xmlMeta="";
	std::string jsonMeta="";
	std::string ttlmeta="";
    std::fstream fileStrOutMeta;
	fileStrOutMeta.open( fileNameOutMeta, std::fstream::out );
	if( !fileStrOutMeta.is_open() ) {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] Could not open file '" << fileNameOutMeta << "' for writing!\n";
		return( false );
	}
    std::fstream fileStrOutMetaTTL;
	fileStrOutMetaTTL.open( fileNameOutMetaTTL, std::fstream::out );
	if( !fileStrOutMetaTTL.is_open() ) {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] Could not open file '" << fileNameOutMetaTTL << "' for writing!\n";
		return( false );
	}
    std::fstream fileStrOutMetaXML;
	fileStrOutMetaXML.open( fileNameOutMetaXML, std::fstream::out );
	if( !fileStrOutMetaXML.is_open() ) {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] Could not open file '" << fileNameOutMetaXML << "' for writing!\n";
		return( false );
	}
    std::fstream fileStrOutMetaJSON;
	fileStrOutMetaJSON.open( fileNameOutMetaJSON, std::fstream::out );
	if( !fileStrOutMetaJSON.is_open() ) {
		LOG::error() << "[MeshInfoData::" << __FUNCTION__ << "] Could not open file '" << fileNameOutMetaJSON << "' for writing!\n";
		return( false );
	}
	
	
	// WRITE Meta-Data
	//----------------------------------------------------------
	// Note: Redundant data is marked with a '#'
#ifdef VERSION_PACKAGE
	fileStrOutMeta << "GigaMesh Version:            " << VERSION_PACKAGE << std::endl;
#else
	fileStrOutMeta << "GigaMesh Version:            unknown" << std::endl;
#endif
	fileStrOutMeta << "CPU Threads (available):     " << std::thread::hardware_concurrency() - 1 << std::endl;
	fileStrOutMeta << "Username:                    " << userName << std::endl;
	fileStrOutMeta << "Hostname:                    " << hostName << std::endl;
	fileStrOutMeta << "File Input:                  " << rMeshInfoPrevious.mStrings[MeshInfoData::FILENAME] << std::endl;
	fileStrOutMeta << "File Output:                 " << rFileNameOut.string() << std::endl;
	fileStrOutMeta << "Model Id:                    " << mStrings[MeshInfoData::MODEL_ID] << std::endl;
	fileStrOutMeta << "Model Material:              " << mStrings[MeshInfoData::MODEL_MATERIAL] << std::endl;
	fileStrOutMeta << "Web-Reference:               " << mStrings[MeshInfoData::MODEL_WEBREFERENCE] << std::endl;
	fileStrOutMeta << "UUID parent model:           " << std::endl; //! \todo Proper UUID generation.
	fileStrOutMeta << "UUID created model:          " << std::endl; //! \todo Proper UUID generation.
	fileStrOutMeta << "UUID of this process:        " << std::endl; //! \todo Proper UUID generation.
	fileStrOutMeta << "Processed function:          " << rFunctionExecuted << std::endl;
	fileStrOutMeta << "Vertex count (in):           " << rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_TOTAL] << std::endl;
	fileStrOutMeta << "Vertex count (out)           " << mCountULong[MeshInfoData::VERTICES_TOTAL] << std::endl;
	fileStrOutMeta << "Face count (in):             " << rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_TOTAL]  << std::endl;
	fileStrOutMeta << "Face count (out):            " << mCountULong[MeshInfoData::FACES_TOTAL] << std::endl;
	fileStrOutMeta << "# Vertex count (diff):       " << vertexCountDiff << std::endl;
	fileStrOutMeta << "# Face count (diff):         " << faceCountDiff << std::endl;
	fileStrOutMeta << "Connected components (in):   " << rMeshInfoPrevious.mCountULong[CONNECTED_COMPONENTS] << std::endl;
	fileStrOutMeta << "Connected components (out):  " << mCountULong[CONNECTED_COMPONENTS] << std::endl;
	fileStrOutMeta << "Vertices at border (in):     " << rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_BORDER] << std::endl;
	fileStrOutMeta << "Vertices at border (out):    " << mCountULong[MeshInfoData::VERTICES_BORDER] << std::endl;
	fileStrOutMeta << "# Vertex at border (diff):   " << vertexBorderCountDiff << std::endl;
	fileStrOutMeta << "Vertices synthetic (in):     " << rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_SYNTHETIC] << std::endl;
	fileStrOutMeta << "Vertices synthetic (out):    " << mCountULong[MeshInfoData::VERTICES_SYNTHETIC] << std::endl;
	fileStrOutMeta << "Faces synthetic (in):        " << rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES] << std::endl;
	fileStrOutMeta << "Faces synthetic (out):       " << mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES] << std::endl;
	fileStrOutMeta << "Iterations:                  " << rIterationCount << std::endl;
	fileStrOutMeta << "Start time:                  " << std::ctime( &startTime );
	fileStrOutMeta << "Finish time:                 " << std::ctime( &endTime );
	fileStrOutMeta << "# Timespan (sec):            " << timeElapsed << std::endl;
	fileStrOutMeta.close();
	std::wcout << "[MeshInfoData::" << __FUNCTION__ << "] Wrote meta-data to: " << fileNameOutMeta << std::endl;
    std::string indid=urlEncode(this->mStrings[MeshInfoData::FILENAME]);
    std::string indidnotencoded=this->mStrings[MeshInfoData::FILENAME];
    std::size_t pos = indidnotencoded.find_last_of("/");
    std::string indname = indidnotencoded.substr(pos+1,indidnotencoded.size());
    std::string funcid=urlEncode(rFunctionExecuted);    
    std::string starttime=urlEncode(std::ctime( &startTime ));   
    std::string actid=funcid+"_"+generate_UUID();           
    std::string newindid=generate_UUID(); 
    std::string personid="person"; 
    ttlmeta+="@prefix rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n@prefix foaf:<http://xmlns.com/foaf/0.1/> . \n@prefix skos: <http://www.w3.org/2004/02/skos/core#> .\n@prefix xsd:<http://www.w3.org/2001/XMLSchema#> .\n@prefix rdfs:<http://www.w3.org/2000/01/rdf-schema#> .\n@prefix owl:<http://www.w3.org/2002/07/owl#> .\n@prefix dcat:<http://www.w3.org/ns/dcat#> .\n@prefix prov:<http://www.w3.org/ns/prov#> .\n@prefix giga:<http://www.gigamesh.eu/ont#> .\n@prefix dc:<http://purl.org/dc/terms/> .\n@prefix ex:<http://purl.org/net/ns/ex#> .\n@prefix geo:<http://www.opengis.net/ont/geosparql#> .\n@prefix wdt:<http://www.wikidata.org/prop/direct/> .\n@prefix om:<http://www.ontology-of-units-of-measure.org/resource/om-2/>.\n";
    ttlmeta+="giga:"+urlEncode(rFunctionExecuted)+" rdf:type giga:ProcessingFunction .\n";
    ttlmeta+="giga:ProcessingFunction rdfs:subClassOf prov:Agent .\n";
    ttlmeta+="giga:"+indid+" rdf:type giga:Mesh .\n";    
    ttlmeta+="giga:Mesh rdfs:subClassOf prov:Entity .\n";    
    ttlmeta+="om:Unit rdf:type owl:Class .\n";
    ttlmeta+="om:Unit rdfs:label \"Unit\"@en .\n";
    ttlmeta+="om:TimeUnit rdfs:subClassOf om:Unit .\n";
    ttlmeta+="om:TimeUnit rdfs:label \"time unit\"@en .\n";
    ttlmeta+="om:second-Time rdf:type om:TimeUnit, owl:NamedIndividual  .\n";
    ttlmeta+="om:second-Time rdfs:label \"second\"@en .\n";
    ttlmeta+="prov:Entity rdf:type owl:Class .\n";
    ttlmeta+="prov:Entity rdfs:label \"Entity\"@en .\n";     
    ttlmeta+="prov:Agent rdf:type owl:Class .\n";
    ttlmeta+="prov:Agent rdfs:label \"Agent\"@en .\n"; 
    ttlmeta+="prov:Activity rdf:type owl:Class .\n";   
    ttlmeta+="prov:Activity rdfs:label \"Activity\"@en .\n"; 
    ttlmeta+="foaf:Person rdf:type owl:Class .\n";    
    ttlmeta+="foaf:Person rdfs:label \"Person\"@en .\n"; 
    ttlmeta+="giga:"+actid+" rdf:type prov:Activity .\n";
    ttlmeta+="giga:iterations rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:iterations rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:iterations rdfs:range xsd:integer .\n";
    ttlmeta+="giga:iterations rdfs:label \"Number of iterations of the processing function\"@en .\n";
    ttlmeta+="giga:cputhreads rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:cputhreads rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:cputhreads rdfs:range xsd:integer .\n";
    ttlmeta+="giga:cputhreads rdfs:label \"Number of available CPU threads which could have been used by the processing function\"@en .\n";
    ttlmeta+="giga:gigaMeshVersion rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:gigaMeshVersion rdfs:range xsd:string .\n";
    ttlmeta+="giga:gigaMeshVersion rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:gigaMeshVersion rdfs:label \"The version of the Gigamesh software of this processing algorithm\"@en .\n";
    ttlmeta+="giga:vertexCountDifference rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:vertexCountDifference rdfs:range xsd:integer .\n";
    ttlmeta+="giga:vertexCountDifference rdfs:seeAlso giga:totalNumberOfVertices .\n";
    ttlmeta+="giga:vertexCountDifference rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:vertexCountDifference rdfs:label \"The difference of vertex counts after a processing function has been applied\"@en .\n";
    ttlmeta+="giga:borderVertexCountDifference rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:borderVertexCountDifference rdfs:range xsd:integer .\n";
    ttlmeta+="giga:borderVertexCountDifference rdfs:seeAlso giga:borderVertices .\n";
    ttlmeta+="giga:borderVertexCountDifference rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:borderVertexCountDifference rdfs:label \"The difference of border vertex counts after a processing function has been applied\"@en .\n";
    ttlmeta+="giga:facesCountDifference rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:facesCountDifference rdfs:range xsd:integer .\n";
    ttlmeta+="giga:facesCountDifference rdfs:seeAlso giga:totalNumberOfFaces .\n";
    ttlmeta+="giga:facesCountDifference rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:facesCountDifference rdfs:label \"The difference of faces counts after a processing function has been applied\"@en .\n";
    ttlmeta+="giga:connectedComponentCountDifference rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:connectedComponentCountDifference rdfs:range xsd:integer .\n";
    ttlmeta+="giga:connectedComponentCountDifference rdfs:seeAlso giga:amountOfConnectedComponents .\n";
    ttlmeta+="giga:connectedComponentCountDifference rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:connectedComponentCountDifference rdfs:label \"The difference of connected component counts after a processing function has been applied\"@en .\n";
    ttlmeta+="giga:syntheticVertexCountDifference rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:syntheticVertexCountDifference rdfs:range xsd:integer .\n";
    ttlmeta+="giga:syntheticVertexCountDifference rdfs:seeAlso giga:syntheticVertices .\n";
    ttlmeta+="giga:syntheticVertexCountDifference rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:syntheticVertexCountDifference rdfs:label \"The difference of synthetic vertex counts after a processing function has been applied\"@en .\n";
    ttlmeta+="giga:syntheticFacesCountDifference rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:syntheticFacesCountDifference rdfs:range xsd:integer .\n";
    ttlmeta+="giga:syntheticFacesCountDifference rdfs:seeAlso giga:syntheticVertexFaces .\n";
    ttlmeta+="giga:syntheticFacesCountDifference rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:syntheticFacesCountDifference rdfs:label \"The difference of synthetic faces counts after a processing function has been applied\"@en .\n";
    ttlmeta+="giga:duration rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="giga:duration rdfs:range xsd:integer .\n";
    ttlmeta+="giga:duration rdfs:domain giga:ProcessingFunction .\n";
    ttlmeta+="giga:duration rdfs:label \"The execution time of the processing function\"@en .\n";
    ttlmeta+="prov:startedAtTime rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="prov:startedAtTime rdfs:range xsd:dateTime .\n";
    ttlmeta+="prov:startedAtTime rdfs:label \"Time of the start of the activity\"@en .\n";
    ttlmeta+="prov:endedAtTime rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="prov:endedAtTime rdfs:range xsd:dateTime .\n";
    ttlmeta+="prov:endedAtTime rdfs:label \"Time of the end of the activity\"@en .\n";
    ttlmeta+="prov:generatedAtTime rdf:type owl:DatatypeProperty .\n";
    ttlmeta+="prov:generatedAtTime rdfs:range xsd:dateTime .\n";
    ttlmeta+="prov:generatedAtTime rdfs:label \"Time when an entity was generated\"@en .\n";
    ttlmeta+="prov:actedOnBehalfOf rdf:type owl:ObjectProperty .\n";
    ttlmeta+="prov:actedOnBehalfOf rdfs:label \"acted on behalf of\"@en .\n";
    ttlmeta+="prov:used rdf:type owl:ObjectProperty .\n";
    ttlmeta+="prov:used rdfs:label \"used\"@en .\n";
    ttlmeta+="prov:wasAssociatedWith rdf:type owl:ObjectProperty .\n";
    ttlmeta+="prov:wasAssociatedWith rdfs:label \"was associated with\"@en .\n";
    ttlmeta+="prov:wasAttributedTo rdf:type owl:ObjectProperty .\n";
    ttlmeta+="prov:wasAttributedTo rdfs:label \"was attributed to\"@en .\n";
    ttlmeta+="prov:wasDerivedFrom rdf:type owl:ObjectProperty .\n";
    ttlmeta+="prov:wasDerivedFrom rdfs:label \"was derived from\"@en .\n";
    ttlmeta+="prov:wasGeneratedBy rdf:type owl:ObjectProperty .\n";
    ttlmeta+="prov:wasGeneratedBy rdfs:label \"was generated by\"@en .\n";
    ttlmeta+="dc:contributor rdf:type owl:ObjectProperty .\n";
    ttlmeta+="dc:contributor rdfs:label \"contributor\"@en .\n";
    ttlmeta+="dc:license rdf:type owl:ObjectProperty .\n";
    ttlmeta+="dc:license rdfs:label \"license\"@en .\n";
    ttlmeta+="giga:"+actid+" rdfs:label \""+indname+" processed by "+rFunctionExecuted+"\"@en .\n";
    ttlmeta+="giga:"+actid+" giga:iterations \""+std::to_string(rIterationCount)+"\"^^xsd:integer .\n";
    ttlmeta+="giga:"+actid+" prov:wasAssociatedWith giga:"+funcid+" .\n";
    ttlmeta+="giga:"+actid+" giga:duration giga:"+actid+"_duration . \n";
    ttlmeta+="giga:"+actid+"_duration om:hasUnit om:second-Time . \n";
    ttlmeta+="giga:"+actid+"_duration om:hasPhenomenon giga:"+actid+" . \n";
    ttlmeta+="giga:"+actid+"_duration om:hasNumericalValue \""+std::to_string(timeElapsed)+"\"^^xsd:integer . \n";
    ttlmeta+="giga:"+funcid+" rdfs:label \""+funcid+"\" .\n";
    ttlmeta+="giga:"+actid+" giga:gigaMeshVersion \""+VERSION_PACKAGE+"\"^^xsd:string .\n";
    ttlmeta+="giga:"+actid+" giga:vertexCountDifference \""+std::to_string(vertexCountDiff)+"\"^^xsd:integer .\n";
    ttlmeta+="giga:"+actid+" giga:syntheticVertexCountDifference \""+std::to_string(mCountULong[MeshInfoData::VERTICES_SYNTHETIC] -rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_SYNTHETIC] )+"\"^^xsd:integer .\n";
    ttlmeta+="giga:"+actid+" giga:syntheticFacesCountDifference \""+std::to_string(mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES]-rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES]) +"\"^^xsd:integer .\n";
    ttlmeta+="giga:"+actid+" giga:borderVertexCountDifference \""+std::to_string(vertexBorderCountDiff)+"\"^^xsd:integer .\n";
    ttlmeta+="giga:"+actid+" giga:facesCountDifference \""+std::to_string(faceCountDiff)+"\"^^xsd:integer .\n";
    ttlmeta+="giga:"+actid+" giga:connectedComponentCountDifference \""+std::to_string(mCountULong[CONNECTED_COMPONENTS]-rMeshInfoPrevious.mCountULong[CONNECTED_COMPONENTS])+"\"^^xsd:integer .\n";
    ttlmeta+="giga:"+actid+" giga:cputhreads \""+std::to_string(std::thread::hardware_concurrency() - 1)+"\"^^xsd:integer .\n";
    char startTimeBuf[256];
    const std::tm * startptm = std::localtime(&startTime);
    strftime(startTimeBuf, sizeof(startTimeBuf), "%Y-%m-%dT%H:%M:%S", startptm);
    ttlmeta+="giga:"+actid+" prov:startedAtTime \""+std::string(startTimeBuf)+"\"^^xsd:dateTime .\n";
    char endTimeBuf[256];
    const std::tm * endptm = std::localtime(&endTime);
    strftime(endTimeBuf, sizeof(endTimeBuf), "%Y-%m-%dT%H:%M:%S", endptm);
    ttlmeta+="giga:"+actid+" prov:endedAtTime \""+std::string(endTimeBuf)+"\"^^xsd:dateTime .\n";
    ttlmeta+="giga:"+actid+" prov:used giga:"+indid+" .\n";
    ttlmeta+="giga:"+newindid+" rdf:type giga:Mesh .\n";
    ttlmeta+="giga:"+newindid+" dc:license <https://creativecommons.org/licenses/by-sa/4.0/> .\n";
    ttlmeta+="giga:"+newindid+" prov:wasGeneratedBy giga:"+actid+" .\n";
    ttlmeta+="giga:"+newindid+" prov:wasDerivedFrom giga:"+indid+" .\n";
    ttlmeta+="giga:"+newindid+" prov:wasAttributedTo giga:"+funcid+" .\n";
    ttlmeta+="giga:"+newindid+" prov:generatedAtTime \""+std::string(endTimeBuf)+"\"^^xsd:dateTime .\n";  
    ttlmeta+="giga:"+personid+" rdf:type foaf:Person .\n";
    ttlmeta+="giga:"+personid+" foaf:userName \""+userName+"\" .\n";
    ttlmeta+="giga:"+newindid+" dc:contributor giga:"+personid+" .\n";  
    ttlmeta+="giga:"+actid+" prov:actedOnBehalfOf giga:"+personid+" .\n";    
    fileStrOutMetaTTL << ttlmeta << std::endl;
    fileStrOutMetaTTL.close();
    jsonMeta+="{\n";
    
    jsonMeta+="}\n";
    fileStrOutMetaJSON << jsonMeta << std::endl;
    fileStrOutMetaJSON.close();
    xmlMeta+="<?xml version=\"1.0\"?>\n";
    xmlMeta+="<GigaMeshMetaData xmlns=\"http://www.gigamesh.eu/ont#\" xmlns:dc=\"http://purl.org/dc/terms/\" xmlns:prov=\"http://www.w3.org/ns/prov#\" xmlns:om=\"http://www.ontology-of-units-of-measure.org/resource/om-2/ \">";
    xmlMeta+="<Mesh id=\""+indid+"\">\n";      
    xmlMeta+="<TotalNumberOfVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_TOTAL])+"</TotalNumberOfVertices>\n";
    xmlMeta+="<NaNVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_NAN])+"</NaNVertices>\n";
    xmlMeta+="<NormalLengthVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL])+"</NormalLengthVertices>\n";
    xmlMeta+="<PolylineVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_POLYLINE])+"</PolylineVertices>\n";
    xmlMeta+="<IsolatedVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_SOLO])+"</IsolatedVertices>\n";
    xmlMeta+="<BorderVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_BORDER])+"</BorderVertices>\n";
    xmlMeta+="<NonManifoldVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_NONMANIFOLD])+"</NonManifoldVertices>\n";
    xmlMeta+="<SingularVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_SINGULAR])+"</SingularVertices>\n";
    xmlMeta+="<VerticesOnInvertedEdge>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE])+"</VerticesOnInvertedEdge>\n";
    xmlMeta+="<VerticesPartOfZeroAreaFace>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE])+"</VerticesPartOfZeroAreaFace>\n";
    xmlMeta+="<SyntheticVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_SYNTHETIC])+"</SyntheticVertices>\n";
    xmlMeta+="<ManuallyAddedVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_MANUAL])+"</ManuallyAddedVertices>\n";
    xmlMeta+="<CircleCenterVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER])+"</CircleCenterVertices>\n";
    xmlMeta+="<SelectedVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_SELECTED])+"</SelectedVertices>\n";
    xmlMeta+="<FiniteFunctionValueVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE])+"</FiniteFunctionValueVertices>\n";
    xmlMeta+="<LocalFunctionMinValueVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN])+"</LocalFunctionMinValueVertices>\n";
    xmlMeta+="<LocalFunctionMaxValueVertices>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX])+"</LocalFunctionMaxValueVertices>\n";
    xmlMeta+="</VertexInformation>\n";
    xmlMeta+="<FacesInformation>\n";
    xmlMeta+="<TotalNumberOfFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_TOTAL])+"</TotalNumberOfFaces>\n";
    xmlMeta+="<SoloFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_SOLO])+"</SoloFaces>\n";
    xmlMeta+="<BorderFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_BORDER])+"</BorderFaces>\n";
    xmlMeta+="<ThreeBorderVertexFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES])+"</ThreeBorderVertexFaces>\n";
    xmlMeta+="<BorderFacesBridgeTriConnection>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN])+"</BorderFacesBridgeTriConnection>\n";
    xmlMeta+="<BridgeBorderFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_BORDER_BRDIGE])+"</BridgeBorderFaces>\n";
    xmlMeta+="<DanglingBorderFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_BORDER_DANGLING])+"</DanglingBorderFaces>\n";
    xmlMeta+="<ManifoldFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_MANIFOLD])+"</ManifoldFaces>\n";
    xmlMeta+="<NonManifoldFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_NONMANIFOLD])+"</NonManifoldFaces>\n";
    xmlMeta+="<StickyFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_STICKY])+"</StickyFaces>\n";
    xmlMeta+="<ZeroAreaFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_ZEROAREA])+"</ZeroAreaFaces>\n";
    xmlMeta+="<InvertedFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_INVERTED])+"</InvertedFaces>\n";
    xmlMeta+="<SelfIntersectedFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_SELFINTERSECTED])+"</SelfIntersectedFaces>\n";
    xmlMeta+="<SelectedFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_SELECTED])+"</SelectedFaces>\n";
    xmlMeta+="<SyntheticVertexFaces>"+std::to_string(rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES])+"</SyntheticVertexFaces>\n";
    xmlMeta+="</FacesInformation>\n";
    xmlMeta+="<BoundingBox>\n";
    xmlMeta+="<MinimumXCoordinate>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X])+"</MinimumXCoordinate>\n";
    xmlMeta+="<MinimumYCoordinate>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y])+"</MinimumYCoordinate>\n";
    xmlMeta+="<MinimumZCoordinate>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z])+"</MinimumZCoordinate>\n";
    xmlMeta+="<MaximumXCoordinate>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X])+"</MaximumXCoordinate>\n";
    xmlMeta+="<MaximumYCoordinate>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y])+"</MaximumYCoordinate>\n";
    xmlMeta+="<MaximumZCoordinate>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z])+"</MaximumZCoordinate>\n";
    xmlMeta+="<BoundingBoxWidth>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_WIDTH])+"</BoundingBoxWidth>\n";
    xmlMeta+="<BoundingBoxHeight>"+std::to_string(rMeshInfoPrevious.mCountDouble[MeshInfoData::BOUNDINGBOX_HEIGHT])+"</BoundingBoxHeight>\n";
    xmlMeta+="</BoundingBox>\n";    
    xmlMeta+="</Mesh>\n";  
    xmlMeta+="<Mesh id=\""+newindid+"\">\n";      
    xmlMeta+="<TotalNumberOfVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_TOTAL])+"</TotalNumberOfVertices>\n";
    xmlMeta+="<NaNVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NAN])+"</NaNVertices>\n";
    xmlMeta+="<NormalLengthVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL])+"</NormalLengthVertices>\n";
    xmlMeta+="<IsolatedVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SOLO])+"</IsolatedVertices>\n";
    xmlMeta+="<BorderVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_BORDER])+"</BorderVertices>\n";
    xmlMeta+="<NonManifoldVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NONMANIFOLD])+"</NonManifoldVertices>\n";
    xmlMeta+="<SingularVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SINGULAR])+"</SingularVertices>\n";
    xmlMeta+="<VerticesOnInvertedEdge>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE])+"</VerticesOnInvertedEdge>\n";
    xmlMeta+="<VerticesPartOfZeroAreaFace>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE])+"</VerticesPartOfZeroAreaFace>\n";
    xmlMeta+="<SyntheticVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SYNTHETIC])+"</SyntheticVertices>\n";
    xmlMeta+="<ManuallyAddedVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_MANUAL])+"</ManuallyAddedVertices>\n";
    xmlMeta+="<CircleCenterVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER])+"</CircleCenterVertices>\n";
    xmlMeta+="<SelectedVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SELECTED])+"</SelectedVertices>\n";
    xmlMeta+="<FiniteFunctionValueVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE])+"</FiniteFunctionValueVertices>\n";
    xmlMeta+="<LocalFunctionMinValueVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN])+"</LocalFunctionMinValueVertices>\n";
    xmlMeta+="<LocalFunctionMaxValueVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX])+"</LocalFunctionMaxValueVertices>\n";
    xmlMeta+="</VertexInformation>\n";
    xmlMeta+="<FacesInformation>\n";
    xmlMeta+="<TotalNumberOfFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_TOTAL])+"</TotalNumberOfFaces>\n";
    xmlMeta+="<SoloFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_SOLO])+"</SoloFaces>\n";
    xmlMeta+="<BorderFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER])+"</BorderFaces>\n";
    xmlMeta+="<ThreeBorderVertexFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES])+"</ThreeBorderVertexFaces>\n";
    xmlMeta+="<BorderFacesBridgeTriConnection>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN])+"</BorderFacesBridgeTriConnection>\n";
    xmlMeta+="<BridgeBorderFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE])+"</BridgeBorderFaces>\n";
    xmlMeta+="<DanglingBorderFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_DANGLING])+"</DanglingBorderFaces>\n";
    xmlMeta+="<ManifoldFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_MANIFOLD])+"</ManifoldFaces>\n";
    xmlMeta+="<NonManifoldFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_NONMANIFOLD])+"</NonManifoldFaces>\n";
    xmlMeta+="<StickyFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_STICKY])+"</StickyFaces>\n";
    xmlMeta+="<ZeroAreaFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_ZEROAREA])+"</ZeroAreaFaces>\n";
    xmlMeta+="<InvertedFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_INVERTED])+"</InvertedFaces>\n";
    xmlMeta+="<SelectedFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_SELECTED])+"</SelectedFaces>\n";
    xmlMeta+="<SyntheticVertexFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES])+"</SyntheticVertexFaces>\n";
    xmlMeta+="</FacesInformation>\n";
    xmlMeta+="<BoundingBox>\n";
    xmlMeta+="<MinimumXCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X])+"</MinimumXCoordinate>\n";
    xmlMeta+="<MinimumYCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y])+"</MinimumYCoordinate>\n";
    xmlMeta+="<MinimumZCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z])+"</MinimumZCoordinate>\n";
    xmlMeta+="<MaximumXCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X])+"</MaximumXCoordinate>\n";
    xmlMeta+="<MaximumYCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y])+"</MaximumYCoordinate>\n";
    xmlMeta+="<MaximumZCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z])+"</MaximumZCoordinate>\n";
    xmlMeta+="<BoundingBoxWidth>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_WIDTH])+"</BoundingBoxWidth>\n";
    xmlMeta+="<BoundingBoxHeight>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_HEIGHT])+"</BoundingBoxHeight>\n";
    xmlMeta+="<BoundingBoxThickness>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_THICK])+"</BoundingBoxThickness>\n";
    xmlMeta+="</BoundingBox>\n";    
    xmlMeta+="<prov:wasAttributedTo>"+funcid+"</prov:wasAttributedTo>\n";
    xmlMeta+="<prov:wasGeneratedBy>"+actid+"</prov:wasGeneratedBy>\n";    
    xmlMeta+="<prov:wasDerivedFrom>"+indid+"</prov:wasDerivedFrom>\n";  
    xmlMeta+="<prov:generatedAtTime>"+std::string(endTimeBuf)+"</prov:generatedAtTime>\n";  
    xmlMeta+="<dc:license>https://creativecommons.org/licenses/by-sa/4.0/</dc:license>\n";  
    xmlMeta+="</Mesh>\n";    
    xmlMeta+="<ProcessingInformation id=\""+actid+"\">\n";
    xmlMeta+="<cputhreads>"+std::to_string(std::thread::hardware_concurrency() - 1)+"</cputhreads>\n";
    xmlMeta+="<iterations>"+std::to_string(rIterationCount)+"</iterations>\n";
    xmlMeta+="<duration unit=\"om:second\">"+std::to_string(timeElapsed)+"</duration>\n";
    xmlMeta+="<connectedComponentCountDifference>"+std::to_string(mCountULong[CONNECTED_COMPONENTS]-rMeshInfoPrevious.mCountULong[CONNECTED_COMPONENTS])+"</connectedComponentCountDifference>\n";
    xmlMeta+="<syntheticVertexCountDifference>"+std::to_string(mCountULong[MeshInfoData::VERTICES_SYNTHETIC] -rMeshInfoPrevious.mCountULong[MeshInfoData::VERTICES_SYNTHETIC] )+"</syntheticVertexCountDifference>\n";
    xmlMeta+="<syntheticFacesCountDifference>"+std::to_string(mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES]-rMeshInfoPrevious.mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES])+"</syntheticFacesCountDifference>\n";
    xmlMeta+="<vertexCountDifference>"+std::to_string(vertexCountDiff)+"</vertexCountDifference>\n";
    xmlMeta+="<facesCountDifference>"+std::to_string(faceCountDiff)+"</facesCountDifference>\n";
    xmlMeta+="<prov:startedAtTime>"+std::string(startTimeBuf)+"</prov:startedAtTime>\n";
    xmlMeta+="<prov:endedAtTime>"+std::string(endTimeBuf)+"</prov:endedAtTime>\n";
    xmlMeta+="<prov:used>"+indid+"</prov:used>";
    xmlMeta+="<prov:wasAssociatedWith>\n<"+funcid+">\n<prov:actedOnBehalfOf>\n<foaf:Person id=\""+personid+"\">\n<foaf:userName>"+userName+"</foaf:userName>\n</foaf:Person>\n</prov:actedOnBehalfOf>\n</"+funcid+">\n</prov:wasAssociatedWith>\n";
    xmlMeta+="</ProcessingInformation>\n";
    xmlMeta+="</GigaMeshMetaData>\n";
    fileStrOutMetaXML << xmlMeta << std::endl;
    fileStrOutMetaXML.close();
    // Done
	return( true );
}

bool MeshInfoData::getMeshInfoXML( std::string& rInfoXML ){
    std::string infoStr = "<?xml version=\"1.0\"?>\n<GigaMeshInfo xmlns=\"http://www.gigamesh.eu/ont#\" xmlns:dc=\"http://purl.org/dc/terms/\">\n";
    infoStr+="<VertexInformation>\n";
    infoStr+="<TotalNumberOfVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_TOTAL])+"</TotalNumberOfVertices>\n";
    infoStr+="<NaNVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NAN])+"</NaNVertices>\n";
    infoStr+="<NormalLengthVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL])+"</NormalLengthVertices>\n";
    infoStr+="<PolylineVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_POLYLINE])+"</PolylineVertices>\n";
    infoStr+="<IsolatedVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SOLO])+"</IsolatedVertices>\n";
    infoStr+="<BorderVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_BORDER])+"</BorderVertices>\n";
    infoStr+="<NonManifoldVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NONMANIFOLD])+"</NonManifoldVertices>\n";
    infoStr+="<SingularVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SINGULAR])+"</SingularVertices>\n";
    infoStr+="<VerticesOnInvertedEdge>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE])+"</VerticesOnInvertedEdge>\n";
    infoStr+="<VerticesPartOfZeroAreaFace>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE])+"</VerticesPartOfZeroAreaFace>\n";
    infoStr+="<SyntheticVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SYNTHETIC])+"</SyntheticVertices>\n";
    infoStr+="<ManuallyAddedVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_MANUAL])+"</ManuallyAddedVertices>\n";
    infoStr+="<CircleCenterVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER])+"</CircleCenterVertices>\n";
    infoStr+="<SelectedVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SELECTED])+"</SelectedVertices>\n";
    infoStr+="<FiniteFunctionValueVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE])+"</FiniteFunctionValueVertices>\n";
    infoStr+="<LocalFunctionMinValueVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN])+"</LocalFunctionMinValueVertices>\n";
    infoStr+="<LocalFunctionMaxValueVertices>"+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX])+"</LocalFunctionMaxValueVertices>\n";
    infoStr+="</VertexInformation>\n";
    infoStr+="<FacesInformation>\n";
    infoStr+="<TotalNumberOfFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_TOTAL])+"</TotalNumberOfFaces>\n";
    infoStr+="<SoloFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_SOLO])+"</SoloFaces>\n";
    infoStr+="<BorderFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER])+"</BorderFaces>\n";
    infoStr+="<ThreeBorderVertexFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES])+"</ThreeBorderVertexFaces>\n";
    infoStr+="<BorderFacesBridgeTriConnection>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN])+"</BorderFacesBridgeTriConnection>\n";
    infoStr+="<BridgeBorderFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE])+"</BridgeBorderFaces>\n";
    infoStr+="<DanglingBorderFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_DANGLING])+"</DanglingBorderFaces>\n";
    infoStr+="<ManifoldFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_MANIFOLD])+"</ManifoldFaces>\n";
    infoStr+="<NonManifoldFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_NONMANIFOLD])+"</NonManifoldFaces>\n";
    infoStr+="<StickyFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_STICKY])+"</StickyFaces>\n";
    infoStr+="<ZeroAreaFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_ZEROAREA])+"</ZeroAreaFaces>\n";
    infoStr+="<InvertedFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_INVERTED])+"</InvertedFaces>\n";
    infoStr+="<SelectedFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_SELECTED])+"</SelectedFaces>\n";
    infoStr+="<SelfIntersectedFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_SELFINTERSECTED])+"</SelfIntersectedFaces>\n";
    infoStr+="<SyntheticVertexFaces>"+std::to_string(this->mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES])+"</SyntheticVertexFaces>\n";
    infoStr+="</FacesInformation>\n";
    infoStr+="<BoundingBox>\n";
    infoStr+="<MinimumXCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X])+"</MinimumXCoordinate>\n";
    infoStr+="<MinimumYCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y])+"</MinimumYCoordinate>\n";
    infoStr+="<MinimumZCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z])+"</MinimumZCoordinate>\n";
    infoStr+="<MaximumXCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X])+"</MaximumXCoordinate>\n";
    infoStr+="<MaximumYCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y])+"</MaximumYCoordinate>\n";
    infoStr+="<MaximumZCoordinate>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z])+"</MaximumZCoordinate>\n";
    infoStr+="<BoundingBoxWidth>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_WIDTH])+"</BoundingBoxWidth>\n";
    infoStr+="<BoundingBoxHeight>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_HEIGHT])+"</BoundingBoxHeight>\n";
    infoStr+="<BoundingBoxThickness>"+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_THICK])+"</BoundingBoxThickness>\n";
    infoStr+="</BoundingBox>\n";    
    infoStr+="<MetadataObject>\n";
    infoStr+="<Filename>"+this->mStrings[MeshInfoData::FILENAME]+"</Filename>\n";
    infoStr+="<GigaMeshVersion> </GigaMeshVersion>\n";
    //infoStr+="<pcname>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"</pcname>\n"; 
    //infoStr+="<dc:license>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"</dc:license>\n"; 
    //infoStr+="<dc:format>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"</dc:format>\n";  
    //infoStr+="<dc:date>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"</dc:date>\n";  
    //infoStr+="<dc:contributor>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"</dc:contributor>\n";   
    //infoStr+="<dc:creator>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"</dc:creator>\n";   
    infoStr+="<SmallestArea>"+std::to_string(this->mCountDouble[MeshInfoData::FACES_AREA_SMALLEST])+"</SmallestArea>\n";
    infoStr+="<LargestArea>"+std::to_string(this->mCountDouble[MeshInfoData::FACES_AREA_LARGEST])+"</LargestArea>\n";
    infoStr+="<TotalArea>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"</TotalArea>\n";    
    infoStr+="<TotalVolumeDX>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DX])+"</TotalVolumeDX>\n"; 
    infoStr+="<TotalVolumeDY>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DY])+"</TotalVolumeDY>\n";
    infoStr+="<TotalVolumeDZ>"+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DZ])+"</TotalVolumeDZ>\n"; 
    infoStr+="</MetadataObject>\n";
    infoStr+="</GigaMeshInfo>";
    rInfoXML = infoStr;
	return( true );    
}


bool MeshInfoData::getMeshInfoJSON(std::string& rInfoJSON){
    std::string infoStr = "{\n";
    infoStr+="\"@context\":{\n";
    infoStr+="\"giga\":\"http://www.gigamesh.eu/ont#\",\n";
    infoStr+="\"dc\":\"http://purl.org/dc/terms/\",\n";
    infoStr+="\"foaf\":\"http://xmlns.com/foaf/0.1/\"\n";
    infoStr+="},\n";
    infoStr+="\"@type\":\"GigaMeshInfo\",\n"; 
    infoStr+="\"giga:FileName\":\""+this->mStrings[MeshInfoData::FILENAME]+"\",\n";
    infoStr+="\"giga:GigaMeshVersion\":\"\",\n";
    infoStr+="\"giga:VertexInformation\":{\n";
    infoStr+="\"dc:license\":\"https://creativecommons.org/licenses/by-sa/4.0/\",\n";
    infoStr+="\"giga:TotalNumberOfVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_TOTAL])+"\",\n";   
    infoStr+="\"giga:NaNVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NAN])+"\",\n";    
    infoStr+="\"giga:NormalLengthVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL])+"\",\n";    
    infoStr+="\"giga:IsolatedVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SOLO])+"\",\n";
    infoStr+="\"giga:PolylineVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_POLYLINE])+"\",\n";
    infoStr+="\"giga:BorderVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_BORDER])+"\",\n"; 
    infoStr+="\"giga:NonManifoldVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NONMANIFOLD])+"\",\n"; 
    infoStr+="\"giga:SingularVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SINGULAR])+"\",\n"; 
    infoStr+="\"giga:VerticesOnInvertedEdge\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE])+"\",\n"; 
    infoStr+="\"giga:VerticesPartOfZeroAreaFace\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE])+"\",\n";
    infoStr+="\"giga:SyntheticVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SYNTHETIC])+"\",\n"; 
    infoStr+="\"giga:ManuallyAddedVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_MANUAL])+"\",\n"; 
    infoStr+="\"giga:CircleCenterVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER])+"\",\n";
    infoStr+="\"giga:SelectedVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SELECTED])+"\",\n";
    infoStr+="\"giga:FiniteFunctionValueVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE])+"\",\n";
    infoStr+="\"giga:LocalFunctionMinValueVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN])+"\",\n";
    infoStr+="\"giga:LocalFunctionMaxValueVertices\":\""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX])+"\"\n";
    infoStr+="},\n";
    infoStr+="\"giga:FacesInformation\":{\n";
    infoStr+="\"giga:TotalNumberOfFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_TOTAL])+"\",\n";     
    infoStr+="\"giga:SoloFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_SOLO])+"\",\n";   
    infoStr+="\"giga:BorderFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER])+"\",\n";   
    infoStr+="\"giga:ThreeBorderVertexFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES])+"\",\n";   
    infoStr+="\"giga:BorderFacesBridgeTriConnection\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN])+"\",\n"; 
    infoStr+="\"giga:BridgeBorderFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE])+"\",\n"; 
    infoStr+="\"giga:DanglingBorderFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_DANGLING])+"\",\n"; 
    infoStr+="\"giga:ManifoldFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_MANIFOLD])+"\",\n"; 
    infoStr+="\"giga:NonManifoldFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_NONMANIFOLD])+"\",\n"; 
    infoStr+="\"giga:StickyFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_STICKY])+"\",\n"; 
    infoStr+="\"giga:ZeroAreaFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_ZEROAREA])+"\",\n"; 
    infoStr+="\"giga:InvertedFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_INVERTED])+"\",\n"; 
    infoStr+="\"giga:SelectedFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_SELECTED])+"\",\n"; 
    infoStr+="\"giga:SelfIntersectedFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_SELFINTERSECTED])+"\",\n";
    infoStr+="\"giga:SyntheticVertexFaces\":\""+std::to_string(this->mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES])+"\"\n"; 
    infoStr+="},\n";    
    infoStr+="\"giga:BoundingBox\":{\n";
    infoStr+="\"giga:MinimumXCoordinate\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X])+"\",\n";    
    infoStr+="\"giga:MinimumYCoordinate\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y])+"\",\n"; 
    infoStr+="\"giga:MinimumZCoordinate\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z])+"\",\n";
    infoStr+="\"giga:MaximumXCoordinate\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X])+"\",\n"; 
    infoStr+="\"giga:MaximumYCoordinate\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y])+"\",\n"; 
    infoStr+="\"giga:MaximumZCoordinate\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z])+"\",\n"; 
    infoStr+="\"giga:BoundingBoxWidth\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_WIDTH])+"\",\n"; 
    infoStr+="\"giga:BoundingBoxHeight\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_HEIGHT])+"\"\n"; 
    infoStr+="\"giga:BoundingBoxThickness\":\""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_THICK])+"\"\n"; 
    infoStr+="},\n";
    infoStr+="\"giga:Metadata\":{\n";
    infoStr+="\"giga:SmallestArea\":\""+std::to_string(this->mCountDouble[MeshInfoData::FACES_AREA_SMALLEST])+"\",\n";
    infoStr+="\"giga:LargestArea\":\""+std::to_string(this->mCountDouble[MeshInfoData::FACES_AREA_LARGEST])+"\",\n";  
    infoStr+="\"giga:TotalArea\":\""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"\",\n";   
    infoStr+="\"giga:TotalVolumeDX\":\""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DX])+"\",\n";   
    infoStr+="\"giga:TotalVolumeDY\":\""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DY])+"\",\n";  
    infoStr+="\"giga:TotalVolumeDZ\":\""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DZ])+"\"\n";  
    infoStr+="},\n";
    infoStr+="\"dc:creator\":{\n";
    infoStr+="\"foaf:name\":\"___\",\n";
    infoStr+="\"foaf:accountName\":\"___\",\n";
    infoStr+="\"giga:pcname\":\"___\"\n";  
    infoStr+="}\n";
    infoStr+="}";
    rInfoJSON = infoStr;
	return( true );
}

bool MeshInfoData::getMeshInfoTTL(std::string& rInfoTTL){
    std::string indid=urlEncode(this->mStrings[MeshInfoData::FILENAME]);
    std::string indidnotencoded=this->mStrings[MeshInfoData::FILENAME];
    std::size_t pos = indidnotencoded.find_last_of("/");
    std::string indname = indidnotencoded.substr(pos+1,indidnotencoded.size());
    std::string infoStr = "@prefix rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n@prefix foaf:<http://xmlns.com/foaf/0.1/> . \n@prefix skos: <http://www.w3.org/2004/02/skos/core#> .\n@prefix xsd:<http://www.w3.org/2001/XMLSchema#> .\n@prefix rdfs:<http://www.w3.org/2000/01/rdf-schema#> .\n@prefix owl:<http://www.w3.org/2002/07/owl#> .\n@prefix dcat:<http://www.w3.org/ns/dcat#> .\n@prefix prov:<http://www.w3.org/ns/prov#> .\n@prefix giga:<http://www.gigamesh.eu/ont#> .\n@prefix dc:<http://purl.org/dc/terms/> .\n@prefix ex:<http://purl.org/net/ns/ex#> .\n@prefix geo:<http://www.opengis.net/ont/geosparql#> .\n@prefix wdt:<http://www.wikidata.org/prop/direct/> .\n@prefix om:<http://www.ontology-of-units-of-measure.org/resource/om-2/>.\n";
    infoStr+="giga:GigameshInfo rdf:type owl:Class .\n";
    infoStr+="giga:GigameshInfo rdfs:label \"Gigamesh Info\"@en .\n";
    infoStr+="prov:Entity rdf:type owl:Class .\n";
    infoStr+="prov:Entity rdfs:label \"Entity\"@en .\n";    
    infoStr+="prov:Agent rdf:type owl:Class .\n";
    infoStr+="prov:Agent rdfs:label \"Agent\"@en .\n";   
    infoStr+="giga:Mesh rdf:type owl:Class .\n";
    infoStr+="giga:Mesh rdfs:subClassOf prov:Entity .\n";
    infoStr+="giga:Mesh rdfs:label \"Mesh\"@en .\n";
    infoStr+="geo:Geometry rdf:type owl:Class .\n";
    infoStr+="geo:Geometry rdfs:label \"Geometry\"@en .\n";
    infoStr+="foaf:Person rdf:type owl:Class .\n";
    infoStr+="foaf:Person rdfs:subClassOf prov:Agent .\n";
    infoStr+="foaf:Person rdfs:label \"Person\"@en .\n";
    infoStr+="foaf:accountName rdf:type owl:DatatypeProperty . \n";
    infoStr+="foaf:accountName rdfs:domain foaf:Person . \n";
    infoStr+="foaf:accountName rdfs:range xsd:string . \n";
    infoStr+="foaf:accountName rdfs:label \"account name\"@en .\n";
    infoStr+="dc:contributor rdf:type owl:ObjectProperty . \n";
    infoStr+="dc:contributor rdfs:label \"contributor\"@en .\n";
    infoStr+="dc:license rdf:type owl:ObjectProperty .\n";
    infoStr+="dc:license rdfs:label \"license\"@en .\n";
    infoStr+="wdt:P361 rdf:type owl:ObjectProperty . \n";
    infoStr+="wdt:P361 rdfs:label \"part of\"@en .\n";
    infoStr+="giga:"+indid+" rdf:type giga:Mesh .\n";
    infoStr+="giga:"+indid+" dc:contributor giga:"+indid+"_contributor .\n";
    infoStr+="giga:"+indid+" dc:license <https://creativecommons.org/licenses/by-sa/4.0/> .\n";
    infoStr+="giga:"+indid+"_contributor rdf:type foaf:Person .\n";
    infoStr+="giga:"+indid+"_contributor rdfs:label  \""+indname+" Contributor\"@en .\n";
    infoStr+="giga:"+indid+"_contributor foaf:accountName \"\" .\n";
    infoStr+="giga:"+indid+" rdfs:label \""+indname+"\"@en .\n";
    infoStr+="giga:TotalNumberOfVertices rdf:type owl:Class .\n";  
    infoStr+="giga:TotalNumberOfVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_totalNumberOfVertices rdf:type giga:TotalNumberOfVertices .\n";   
    infoStr+="giga:"+indid+"_totalNumberOfVertices rdfs:label \""+indname+" Total Number of Vertices\"@en .\n";   
    infoStr+="giga:"+indid+"_totalNumberOfVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_TOTAL])+"\"^^xsd:integer .\n";   
    infoStr+="giga:totalNumberOfVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:totalNumberOfVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:totalNumberOfVertices rdfs:label \"Total number of vertices\"@en .\n";
    infoStr+="giga:totalNumberOfVertices skos:definition \"The total number of vertices as represented in the mesh\" .\n";
    infoStr+="giga:totalNumberOfVertices rdfs:comment \"Gigamesh Info: Total number of vertices\" .\n";
    infoStr+="giga:"+indid+" giga:totalNumberOfVertices giga:"+indid+"_totalNumberOfVertices .\n"; 
    infoStr+="giga:NaNVertices rdf:type owl:Class .\n";    
    infoStr+="giga:NaNVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_NaNVertices rdf:type giga:NaNVertices .\n";   
    infoStr+="giga:"+indid+"_NaNVertices rdfs:label \""+indname+" Amount of NaN Vertices\"@en .\n";   
    infoStr+="giga:"+indid+"_NaNVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NAN])+"\"^^xsd:integer .\n";   
    infoStr+="giga:NaNVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:NaNVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:NaNVertices rdfs:label \"NaN Vertices\"@en .\n";
    infoStr+="giga:NaNVertices skos:definition \"The total number of vertices which are not represented by a number\" .\n";
    infoStr+="giga:NaNVertices rdfs:comment \"Gigamesh Info: Vertices not-a-number\" .\n";
    infoStr+="giga:"+indid+" giga:NaNVertices giga:"+indid+"_NaNVertices .\n";       
    infoStr+="giga:"+indid+"_NaNVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:VertexNormalLength rdf:type owl:Class .\n";    
    infoStr+="giga:VertexNormalLength rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_vertexNormalLength rdf:type giga:VertexNormalLength .\n";   
    infoStr+="giga:"+indid+"_vertexNormalLength rdfs:label \""+indname+" Amount of Vertices with normal length\"@en .\n"; 
    infoStr+="giga:"+indid+"_vertexNormalLength rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL])+"\"^^xsd:integer .\n";   
    infoStr+="giga:vertexNormalLength rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:vertexNormalLength rdfs:domain giga:Mesh .\n";
    infoStr+="giga:vertexNormalLength rdfs:label \"Vertex normal vector length normal\"@en .\n";
    infoStr+="giga:vertexNormalLength skos:definition \"The toal number of vertices with a normal vector length\" .\n";
    infoStr+="giga:vertexNormalLength rdfs:comment \"Gigamesh Info: Vertex normal vector length normal\" .\n";
    infoStr+="giga:"+indid+" giga:vertexNormalLength giga:"+indid+"_vertexNormalLength .\n"; 
    infoStr+="giga:"+indid+"_vertexNormalLength wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:VertexIsolated rdf:type owl:Class .\n";    
    infoStr+="giga:VertexIsolated rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_vertexIsolated rdf:type giga:VertexIsolated .\n";   
    infoStr+="giga:"+indid+"_vertexIsolated rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SOLO])+"\"^^xsd:integer .\n";   
    infoStr+="giga:"+indid+"_vertexIsolated rdfs:label \""+indname+" Isolated Vertices\"@en .\n";
    infoStr+="giga:vertexIsolated rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:vertexIsolated rdfs:domain giga:Mesh .\n";
    infoStr+="giga:vertexIsolated rdfs:label \"Vertices solo\"@en .\n";
    infoStr+="giga:vertexIsolated skos:definition \"The total number of isolated vertices\" .\n";
    infoStr+="giga:vertexIsolated rdfs:comment \"Gigamesh Info: Vertices solo\" .\n";
    infoStr+="giga:"+indid+" giga:vertexIsolated giga:"+indid+"_vertexIsolated .\n"; 
    infoStr+="giga:"+indid+"_vertexIsolated wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:BorderVertices rdf:type owl:Class .\n";    
    infoStr+="giga:BorderVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_borderVertices rdf:type giga:BorderVertices .\n";  
    infoStr+="giga:"+indid+"_borderVertices rdfs:label \""+indname+" Amount of Border Vertices\"@en .\n"; 
    infoStr+="giga:"+indid+"_borderVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_BORDER])+"\"^^xsd:integer .\n";   
    infoStr+="giga:borderVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:borderVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:borderVertices rdfs:label \"Border vertices\"@en .\n";
    infoStr+="giga:borderVertices skos:definition \"The total number of vertices at borders of the mesh\" .\n";
    infoStr+="giga:borderVertices rdfs:comment \"Gigamesh Info: Border vertices\" .\n";
    infoStr+="giga:"+indid+" giga:borderVertices giga:"+indid+"_borderVertices .\n";
    infoStr+="giga:"+indid+"_borderVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:NonManifoldVertices rdf:type owl:Class .\n";    
    infoStr+="giga:NonManifoldVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_nonManifoldVertices rdf:type giga:NonManifoldVertices .\n";   
    infoStr+="giga:"+indid+"_nonManifoldVertices rdfs:label \""+indname+" Amount of Non-Manifold Vertices\"@en .\n"; 
    infoStr+="giga:"+indid+"_nonManifoldVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_NONMANIFOLD])+"\"^^xsd:integer .\n";  
    infoStr+="giga:nonManifoldVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:nonManifoldVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:nonManifoldVertices rdfs:label \"Vertices non-manifold\"@en .\n";
    infoStr+="giga:nonManifoldVertices skos:definition \"The total number of vertices which are not part of a manifold\" .\n";
    infoStr+="giga:nonManifoldVertices rdfs:comment \"Gigamesh Info: Vertices non-manifold\" .\n";
    infoStr+="giga:"+indid+" giga:nonManifoldVertices giga:"+indid+"_nonManifoldVertices .\n"; 
    infoStr+="giga:"+indid+"_nonManifoldVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:SingularVertices rdf:type owl:Class .\n";    
    infoStr+="giga:SingularVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_singularVertices rdf:type giga:SingularVertices .\n";   
    infoStr+="giga:"+indid+"_singularVertices rdfs:label \""+indname+" Amount of Singular Vertices\"@en .\n"; 
    infoStr+="giga:"+indid+"_singularVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SINGULAR])+"\"^^xsd:integer .\n";  
    infoStr+="giga:singularVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:singularVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:singularVertices rdfs:label \"Vertices singular\"@en .\n";
    infoStr+="giga:singularVertices skos:definition \"The total number of vertices which are singular\" .\n";
    infoStr+="giga:singularVertices rdfs:comment \"Gigamesh Info: Vertices singular\" .\n";
    infoStr+="giga:"+indid+" giga:singularVertices giga:"+indid+"_singularVertices .\n"; 
    infoStr+="giga:"+indid+"_singularVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:VerticesOnInvertedEdge rdf:type owl:Class .\n";    
    infoStr+="giga:VerticesOnInvertedEdge rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_verticesOnInvertedEdge rdf:type giga:VerticesOnInvertedEdge .\n";   
    infoStr+="giga:"+indid+"_verticesOnInvertedEdge rdfs:label \""+indname+" Amount of Vertices on inverted edges\"@en .\n";
    infoStr+="giga:"+indid+"_verticesOnInvertedEdge rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE])+"\"^^xsd:integer .\n";  
    infoStr+="giga:verticesOnInvertedEdge rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:verticesOnInvertedEdge rdfs:domain giga:Mesh .\n";
    infoStr+="giga:verticesOnInvertedEdge rdfs:label \"Vertices along an inverted edge\"@en .\n";
    infoStr+="giga:verticesOnInvertedEdge skos:definition \"The total number of vertices on an inverted edge\" .\n";
    infoStr+="giga:verticesOnInvertedEdge rdfs:comment \"Gigamesh Info: Vertices along an inverted edge\" .\n";
    infoStr+="giga:"+indid+" giga:verticesOnInvertedEdge giga:"+indid+"_verticesOnInvertedEdge .\n";  
    infoStr+="giga:"+indid+"_verticesOnInvertedEdge wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";    
    infoStr+="giga:VerticesPartOfZeroAreaFace rdf:type owl:Class .\n";    
    infoStr+="giga:VerticesPartOfZeroAreaFace rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_verticesPartOfZeroAreaFace rdf:type giga:VerticesPartOfZeroAreaFace .\n";   
    infoStr+="giga:"+indid+"_verticesPartOfZeroAreaFace rdfs:label \""+indname+" Amount of Vertices which are part of zero area faces\"@en .\n";
    infoStr+="giga:"+indid+"_verticesPartOfZeroAreaFace rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE])+"\"^^xsd:integer .\n";  
    infoStr+="giga:verticesPartOfZeroAreaFace rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:verticesPartOfZeroAreaFace rdfs:domain giga:Mesh .\n";
    infoStr+="giga:verticesPartOfZeroAreaFace rdfs:label \"Vertices part of a zero area face\"@en .\n";
    infoStr+="giga:verticesPartOfZeroAreaFace skos:definition \"The total number of vertices which are part of a zero area face\" .\n";
    infoStr+="giga:verticesPartOfZeroAreaFace rdfs:comment \"Gigamesh Info: Vertices part of a zero area face\" .\n";
    infoStr+="giga:"+indid+" giga:verticesPartOfZeroAreaFace giga:"+indid+"_verticesPartOfZeroAreaFace .\n"; 
    infoStr+="giga:"+indid+"_verticesPartOfZeroAreaFace wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";    
    infoStr+="giga:SyntheticVertices rdf:type owl:Class .\n";    
    infoStr+="giga:SyntheticVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_syntheticVertices rdf:type giga:SyntheticVertices .\n";   
    infoStr+="giga:"+indid+"_syntheticVertices rdfs:label \""+indname+" Amount of synthetic vertices\"@en .\n";
    infoStr+="giga:"+indid+"_syntheticVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SYNTHETIC])+"\"^^xsd:integer .\n";  
    infoStr+="giga:syntheticVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:syntheticVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:syntheticVertices rdfs:label \"Vertices synthetic\"@en .\n";
    infoStr+="giga:syntheticVertices skos:definition \"The total number of vertices which are synthetic\" .\n";
    infoStr+="giga:syntheticVertices rdfs:comment \"Gigamesh Info: Vertices synthetic\" .\n";
    infoStr+="giga:"+indid+" giga:syntheticVertices giga:"+indid+"_syntheticVertices .\n";  
    infoStr+="giga:"+indid+"_syntheticVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:ManuallyAddedVertices rdf:type owl:Class .\n";    
    infoStr+="giga:ManuallyAddedVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_manuallyAddedVertices rdf:type giga:ManuallyAddedVertices .\n";   
    infoStr+="giga:"+indid+"_manuallyAddedVertices rdfs:label \""+indname+" Amount of manually added vertices\"@en .\n";
    infoStr+="giga:"+indid+"_manuallyAddedVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_MANUAL])+"\"^^xsd:integer .\n";  
    infoStr+="giga:manuallyAddedVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:manuallyAddedVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:manuallyAddedVertices rdfs:label \"Vertices manually added\"@en .\n";
    infoStr+="giga:manuallyAddedVertices skos:definition \"The total number of vertices which have been manually added\" .\n";
    infoStr+="giga:manuallyAddedVertices rdfs:comment \"Gigamesh Info: Vertices manually added\" .\n";
    infoStr+="giga:"+indid+" giga:manuallyAddedVertices giga:"+indid+"_manuallyAddedVertices .\n"; 
    infoStr+="giga:"+indid+"_manuallyAddedVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:CircleCenterVertices rdf:type owl:Class .\n";    
    infoStr+="giga:CircleCenterVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_circleCenterVertices rdf:type giga:CircleCenterVertices .\n";   
    infoStr+="giga:"+indid+"_circleCenterVertices rdfs:label \""+indname+" Amount of circle center vertices\"@en .\n";
    infoStr+="giga:"+indid+"_circleCenterVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER])+"\"^^xsd:integer .\n";  
    infoStr+="giga:circleCenterVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:circleCenterVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:circleCenterVertices rdfs:label \"Vertices circle center\"@en .\n";
    infoStr+="giga:circleCenterVertices skos:definition \"The total number of vertices which comprise the center of a circle\" .\n";
    infoStr+="giga:circleCenterVertices rdfs:comment \"Gigamesh Info: Vertices circle center\" .\n";
    infoStr+="giga:"+indid+" giga:circleCenterVertices giga:"+indid+"_circleCenterVertices .\n"; 
    infoStr+="giga:"+indid+"_circleCenterVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:PolylineVertices rdf:type owl:Class .\n";    
    infoStr+="giga:PolylineVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_polylineVertices rdf:type giga:SelectedVertices .\n";   
    infoStr+="giga:"+indid+"_polylineVertices rdfs:label \""+indname+" Amount of selected vertices\"@en .\n";
    infoStr+="giga:"+indid+"_polylineVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_POLYLINE])+"\"^^xsd:integer .\n";  
    infoStr+="giga:polylineVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:polylineVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:polylineVertices rdfs:label \"Vertices selected\"@en .\n";
    infoStr+="giga:polylineVertices skos:definition \"The total number of vertices of polylines\" .\n";
    infoStr+="giga:polylineVertices rdfs:comment \"Gigamesh Info: Vertices of polylines\" .\n";
    infoStr+="giga:"+indid+" giga:polylineVertices giga:"+indid+"_polylineVertices .\n"; 
    infoStr+="giga:SelectedVertices rdf:type owl:Class .\n";    
    infoStr+="giga:SelectedVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_selectedVertices rdf:type giga:SelectedVertices .\n";   
    infoStr+="giga:"+indid+"_selectedVertices rdfs:label \""+indname+" Amount of selected vertices\"@en .\n";
    infoStr+="giga:"+indid+"_selectedVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_SELECTED])+"\"^^xsd:integer .\n";  
    infoStr+="giga:selectedVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:selectedVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:selectedVertices rdfs:label \"Vertices selected\"@en .\n";
    infoStr+="giga:selectedVertices skos:definition \"The total number of vertices which are selected\" .\n";
    infoStr+="giga:selectedVertices rdfs:comment \"Gigamesh Info: Vertices selected\" .\n";
    infoStr+="giga:"+indid+" giga:selectedVertices giga:"+indid+"_selectedVertices .\n"; 
    infoStr+="giga:"+indid+"_selectedVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:FiniteFunctionValueVertices rdf:type owl:Class .\n";    
    infoStr+="giga:FiniteFunctionValueVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_finiteFunctionValueVertices rdf:type giga:FiniteFunctionValueVertices .\n";   
    infoStr+="giga:"+indid+"_finiteFunctionValueVertices rdfs:label \""+indname+" Amount of finite function value vertices\"@en .\n";
    infoStr+="giga:"+indid+"_finiteFunctionValueVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE])+"\"^^xsd:integer .\n";  
    infoStr+="giga:finiteFunctionValueVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:finiteFunctionValueVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:finiteFunctionValueVertices rdfs:label \"Vertices with finite function value\"@en .\n";
    infoStr+="giga:finiteFunctionValueVertices skos:definition \"The total number of vertices which have a finite function value\" .\n";
    infoStr+="giga:finiteFunctionValueVertices rdfs:comment \"Gigamesh Info: Vertices with finite function value\" .\n";
    infoStr+="giga:"+indid+" giga:finiteFunctionValueVertices giga:"+indid+"_finiteFunctionValueVertices .\n"; 
    infoStr+="giga:"+indid+"_finiteFunctionValueVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:LocalFunctionMinValueVertices rdf:type owl:Class .\n";    
    infoStr+="giga:LocalFunctionMinValueVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_localFunctionMinValueVertices rdf:type giga:LocalFunctionMinValueVertices .\n";   
    infoStr+="giga:"+indid+"_localFunctionMinValueVertices rdfs:label \""+indname+" Amount of local function minimum value vertices\"@en .\n";
    infoStr+="giga:"+indid+"_localFunctionMinValueVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN])+"\"^^xsd:integer .\n";  
    infoStr+="giga:localFunctionValueMinVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:localFunctionValueMinVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:localFunctionValueMinVertices rdfs:label \"Vertices with local function value minimum\"@en .\n";
    infoStr+="giga:localFunctionValueMinVertices skos:definition \"The total number of vertices which have a local function value minimum\" .\n";
    infoStr+="giga:localFunctionValueMinVertices rdfs:comment \"Gigamesh Info: Vertices with local function value minimum\" .\n";
    infoStr+="giga:"+indid+" giga:localFunctionValueMinVertices giga:"+indid+"_localFunctionMinValueVertices .\n";   
    infoStr+="giga:"+indid+"_localFunctionMinValueVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:LocalFunctionMaxValueVertices rdf:type owl:Class .\n";    
    infoStr+="giga:LocalFunctionMaxValueVertices rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_localFunctionMaxValueVertices rdf:type giga:LocalFunctionMaxValueVertices .\n";   
    infoStr+="giga:"+indid+"_localFunctionMaxValueVertices rdfs:label \""+indname+" Amount of local function maximum value vertices\"@en .\n";
    infoStr+="giga:"+indid+"_localFunctionMaxValueVertices rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX])+"\"^^xsd:integer .\n";  
    infoStr+="giga:localFunctionValueMaxVertices rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:localFunctionValueMaxVertices rdfs:domain giga:Mesh .\n";
    infoStr+="giga:localFunctionValueMaxVertices rdfs:label \"Vertices with local function value maximum\"@en .\n";
    infoStr+="giga:localFunctionValueMaxVertices skos:definition \"The total number of vertices which have a local function value maximum\" .\n";
    infoStr+="giga:localFunctionValueMaxVertices rdfs:comment \"Gigamesh Info: Vertices with local function value maximum\" .\n";
    infoStr+="giga:"+indid+" giga:localFunctionValueMaxVertices giga:"+indid+"_localFunctionMaxValueVertices .\n";  
    infoStr+="giga:"+indid+"_localFunctionMaxValueVertices wdt:P361 giga:"+indid+"_totalNumberOfVertices .\n";
    infoStr+="giga:TotalNumberOfFaces rdf:type owl:Class .\n";    
    infoStr+="giga:TotalNumberOfFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_totalNumberOfFaces rdf:type giga:TotalNumberOfFaces .\n";   
    infoStr+="giga:"+indid+"_totalNumberOfFaces rdfs:label \""+indname+" Total number of faces\"@en .\n";
    infoStr+="giga:"+indid+"_totalNumberOfFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_TOTAL])+"\"^^xsd:integer .\n";  
    infoStr+="giga:totalNumberOfFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:totalNumberOfFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:totalNumberOfFaces rdfs:label \"Total number of faces\"@en .\n";
    infoStr+="giga:totalNumberOfFaces skos:definition \"The total number of faces in the given mesh\" .\n";
    infoStr+="giga:totalNumberOfFaces rdfs:comment \"Gigamesh Info: Total number of faces\" .\n";
    infoStr+="giga:"+indid+" giga:totalNumberOfFaces giga:"+indid+"_totalNumberOfFaces .\n";  
    infoStr+="giga:SoloFaces rdf:type owl:Class .\n";    
    infoStr+="giga:SoloFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_soloFaces rdf:type giga:SoloFaces .\n";   
    infoStr+="giga:"+indid+"_soloFaces rdfs:label \""+indname+" Amount of solo faces\"@en .\n";
    infoStr+="giga:"+indid+"_soloFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_SOLO])+"\"^^xsd:integer .\n";  
    infoStr+="giga:soloFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:soloFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:soloFaces rdfs:label \"Solo faces\"@en .\n";
    infoStr+="giga:soloFaces skos:definition \"The total number of solo faces\" .\n";
    infoStr+="giga:soloFaces rdfs:comment \"Gigamesh Info: Solo faces\" .\n";
    infoStr+="giga:"+indid+" giga:soloFaces giga:"+indid+"_soloFaces .\n";  
    infoStr+="giga:"+indid+"_soloFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:BorderFaces rdf:type owl:Class .\n";    
    infoStr+="giga:BorderFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_borderFaces rdf:type giga:BorderFaces .\n";   
    infoStr+="giga:"+indid+"_borderFaces rdfs:label \""+indname+" Amount of border faces\"@en .\n";
    infoStr+="giga:"+indid+"_borderFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER])+"\"^^xsd:integer .\n";  
    infoStr+="giga:borderFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:borderFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:borderFaces rdfs:label \"Border faces\"@en .\n";
    infoStr+="giga:borderFaces skos:definition \"The total number of border faces\" .\n";
    infoStr+="giga:borderFaces rdfs:comment \"Gigamesh Info: Border faces\" .\n";
    infoStr+="giga:"+indid+" giga:borderFaces giga:"+indid+"_borderFaces . \n";  
    infoStr+="giga:"+indid+"_borderFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:ThreeBorderVertexFaces rdf:type owl:Class .\n";    
    infoStr+="giga:ThreeBorderVertexFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_threeBorderVertexFaces rdf:type giga:ThreeBorderVertexFaces .\n";   
    infoStr+="giga:"+indid+"_threeBorderVertexFaces rdfs:label \""+indname+" Amount of three border vertex faces\"@en .\n";
    infoStr+="giga:"+indid+"_threeBorderVertexFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES])+"\"^^xsd:integer .\n";  
    infoStr+="giga:threeBorderVertexFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:threeBorderVertexFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:threeBorderVertexFaces rdfs:label \"Faces with three border vertices\"@en .\n";
    infoStr+="giga:threeBorderVertexFaces skos:definition \"The total number of faces with three border vertices\" .\n";
    infoStr+="giga:threeBorderVertexFaces rdfs:comment \"Gigamesh Info: Faces with three border vertices\" .\n";
    infoStr+="giga:"+indid+" giga:threeBorderVertexFaces giga:"+indid+"_threeBorderVertexFaces .\n";      
    infoStr+="giga:"+indid+"_threeBorderVertexFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:BorderFacesBridgeTriConnection rdf:type owl:Class .\n";    
    infoStr+="giga:BorderFacesBridgeTriConnection rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_borderFacesBridgeTriConnection rdf:type giga:BorderFacesBridgeTriConnection .\n";   
    infoStr+="giga:"+indid+"_borderFacesBridgeTriConnection rdfs:label \""+indname+" Amount of border faces with a bridge tri connection\"@en .\n";
    infoStr+="giga:"+indid+"_borderFacesBridgeTriConnection rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN])+"\"^^xsd:integer .\n";  
    infoStr+="giga:borderFacesBridgeTriConnection rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:borderFacesBridgeTriConnection rdfs:domain giga:Mesh .\n";
    infoStr+="giga:borderFacesBridgeTriConnection rdfs:label \"Border faces bridge tri-connection\"@en .\n";
    infoStr+="giga:borderFacesBridgeTriConnection skos:definition \"The total number of faces with three border vertices\" .\n";
    infoStr+="giga:borderFacesBridgeTriConnection rdfs:comment \"Gigamesh Info: Border faces bridge tri-connection\" .\n";
    infoStr+="giga:"+indid+" giga:borderFacesBridgeTriConnection giga:"+indid+"_borderFacesBridgeTriConnection .\n";   
    infoStr+="giga:"+indid+"_borderFacesBridgeTriConnection wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:BridgeBorderFaces rdf:type owl:Class .\n";    
    infoStr+="giga:BridgeBorderFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_bridgeBorderFaces rdf:type giga:BridgeBorderFaces .\n";   
    infoStr+="giga:"+indid+"_bridgeBorderFaces rdfs:label \""+indname+" Amount of bridge border faces\"@en .\n";
    infoStr+="giga:"+indid+"_bridgeBorderFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE])+"\"^^xsd:integer .\n";  
    infoStr+="giga:bridgeBorderFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:bridgeBorderFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:bridgeBorderFaces rdfs:label \"Bridge border faces\"@en .\n";
    infoStr+="giga:bridgeBorderFaces skos:definition \"The total number of bridge border faces\" .\n";
    infoStr+="giga:bridgeBorderFaces rdfs:comment \"Gigamesh Info: Bridge border faces\" .\n";
    infoStr+="giga:"+indid+" giga:bridgeBorderFaces giga:"+indid+"_bridgeBorderFaces .\n";   
    infoStr+="giga:"+indid+"_bridgeBorderFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:DanglingBorderFaces rdf:type owl:Class .\n";    
    infoStr+="giga:DanglingBorderFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_danglingBorderFaces rdf:type giga:DanglingBorderFaces .\n";   
    infoStr+="giga:"+indid+"_danglingBorderFaces rdfs:label \""+indname+" Amount of dangling border faces\"@en .\n";
    infoStr+="giga:"+indid+"_danglingBorderFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_BORDER_DANGLING])+"\"^^xsd:integer .\n";  
    infoStr+="giga:danglingBorderFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:danglingBorderFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:danglingBorderFaces rdfs:label \"Dangling border faces\"@en .\n";
    infoStr+="giga:danglingBorderFaces skos:definition \"The total number of dangling border faces\" .\n";
    infoStr+="giga:danglingBorderFaces rdfs:comment \"Gigamesh Info: Dangling border faces\" .\n";
    infoStr+="giga:"+indid+" giga:danglingBorderFaces giga:"+indid+"_danglingBorderFaces .\n";
    infoStr+="giga:"+indid+"_danglingBorderFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:ManifoldFaces rdf:type owl:Class .\n";    
    infoStr+="giga:ManifoldFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_manifoldFaces rdf:type giga:ManifoldFaces .\n";   
    infoStr+="giga:"+indid+"_manifoldFaces rdfs:label \""+indname+" Amount of manifold faces\"@en .\n";
    infoStr+="giga:"+indid+"_manifoldFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_MANIFOLD])+"\"^^xsd:integer .\n";  
    infoStr+="giga:manifoldFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:manifoldFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:manifoldFaces rdfs:label \"Manifold faces\"@en .\n";
    infoStr+="giga:manifoldFaces skos:definition \"The total number of manifold faces\" .\n";
    infoStr+="giga:manifoldFaces rdfs:comment \"Gigamesh Info: Manifold faces\" .\n";
    infoStr+="giga:"+indid+" giga:manifoldFaces giga:"+indid+"_manifoldFaces .\n";
    infoStr+="giga:"+indid+"_manifoldFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:NonManifoldFaces rdf:type owl:Class .\n";    
    infoStr+="giga:NonManifoldFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_nonManifoldFaces rdf:type giga:NonManifoldFaces .\n";   
    infoStr+="giga:"+indid+"_nonManifoldFaces rdfs:label \""+indname+" Amount of non-manifold faces\"@en .\n";
    infoStr+="giga:"+indid+"_nonManifoldFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_NONMANIFOLD])+"\"^^xsd:integer .\n";  
    infoStr+="giga:nonManifoldFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:nonManifoldFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:nonManifoldFaces rdfs:label \"Non-Manifold faces\"@en .\n";
    infoStr+="giga:nonManifoldFaces skos:definition \"The total number of non-manifold faces\" .\n";
    infoStr+="giga:nonManifoldFaces rdfs:comment \"Gigamesh Info: Non-Manifold faces\" .\n";
    infoStr+="giga:"+indid+" giga:nonManifoldFaces giga:"+indid+"_nonManifoldFaces .\n";
    infoStr+="giga:"+indid+"_nonManifoldFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:StickyFaces rdf:type owl:Class .\n";    
    infoStr+="giga:StickyFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_stickyFaces rdf:type giga:StickyFaces .\n";   
    infoStr+="giga:"+indid+"_stickyFaces rdfs:label \""+indname+" Amount of sticky faces\"@en .\n";
    infoStr+="giga:"+indid+"_stickyFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_STICKY])+"\"^^xsd:integer .\n";  
    infoStr+="giga:stickyFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:stickyFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:stickyFaces rdfs:label \"Sticky faces\"@en .\n";
    infoStr+="giga:stickyFaces skos:definition \"The total number of sticky faces\" .\n";
    infoStr+="giga:stickyFaces rdfs:comment \"Gigamesh Info: Sticky faces\" .\n";
    infoStr+="giga:"+indid+" giga:stickyFaces giga:"+indid+"_stickyFaces .\n";
    infoStr+="giga:"+indid+"_stickyFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:ZeroAreaFaces rdf:type owl:Class .\n";    
    infoStr+="giga:ZeroAreaFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_zeroAreaFaces rdf:type giga:ZeroAreaFaces .\n";   
    infoStr+="giga:"+indid+"_zeroAreaFaces rdfs:label \""+indname+" Amount of zero area faces\"@en .\n";
    infoStr+="giga:"+indid+"_zeroAreaFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_ZEROAREA])+"\"^^xsd:integer .\n";  
    infoStr+="giga:zeroAreaFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:zeroAreaFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:zeroAreaFaces rdfs:label \"Faces with zero area\"@en .\n";
    infoStr+="giga:zeroAreaFaces skos:definition \"The total number of faces with zero area\" .\n";
    infoStr+="giga:zeroAreaFaces rdfs:comment \"Gigamesh Info: Faces with zero area\" .\n";
    infoStr+="giga:"+indid+" giga:zeroAreaFaces giga:"+indid+"_zeroAreaFaces . \n";
    infoStr+="giga:"+indid+"_zeroAreaFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:InvertedFaces rdf:type owl:Class .\n";    
    infoStr+="giga:InvertedFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_invertedFaces rdf:type giga:InvertedFaces .\n";   
    infoStr+="giga:"+indid+"_invertedFaces rdfs:label \""+indname+" Amount of inverted faces\"@en .\n";
    infoStr+="giga:"+indid+"_invertedFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_INVERTED])+"\"^^xsd:integer .\n";  
    infoStr+="giga:invertedFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:invertedFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:invertedFaces rdfs:label \"Inverted Faces\"@en .\n";
    infoStr+="giga:invertedFaces skos:definition \"The total number of faces which are inverted\" .\n";
    infoStr+="giga:invertedFaces rdfs:comment \"Gigamesh Info: Inverted Faces\" .\n";
    infoStr+="giga:"+indid+" giga:invertedFaces giga:"+indid+"_invertedFaces .\n";
    infoStr+="giga:"+indid+"_invertedFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:SelectedFaces rdf:type owl:Class .\n";    
    infoStr+="giga:SelectedFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_selectedFaces rdf:type giga:SelectedFaces .\n";   
    infoStr+="giga:"+indid+"_selectedFaces rdfs:label \""+indname+" Amount of selected faces\"@en .\n";
    infoStr+="giga:"+indid+"_selectedFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_SELECTED])+"\"^^xsd:integer .\n";  
    infoStr+="giga:selectedFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:selectedFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:selectedFaces rdfs:label \"Selected Faces\"@en .\n";
    infoStr+="giga:selectedFaces skos:definition \"The total number of selected faces\" .\n";
    infoStr+="giga:selectedFaces rdfs:comment \"Gigamesh Info: Selected Faces\" .\n";
    infoStr+="giga:"+indid+" giga:selectedFaces giga:"+indid+"_selectedFaces .\n";
    infoStr+="giga:"+indid+"_selectedFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:SelfIntersectedFaces rdf:type owl:Class .\n";    
    infoStr+="giga:SelfIntersectedFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_selfIntersectedFaces rdf:type giga:SelfIntersectedFaces .\n";   
    infoStr+="giga:"+indid+"_selfIntersectedFaces rdfs:label \""+indname+" Amount of self-intersected faces\"@en .\n";
    infoStr+="giga:"+indid+"_selfIntersectedFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_SELFINTERSECTED])+"\"^^xsd:integer .\n";  
    infoStr+="giga:selfIntersectedFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:selfIntersectedFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:selfIntersectedFaces rdfs:label \"Selected Faces\"@en .\n";
    infoStr+="giga:selfIntersectedFaces skos:definition \"The total number of selected faces\" .\n";
    infoStr+="giga:selfIntersectedFaces rdfs:comment \"Gigamesh Info: Selected Faces\" .\n";
    infoStr+="giga:"+indid+" giga:selfIntersectedFaces giga:"+indid+"_selfIntersectedFaces .\n";
    infoStr+="giga:"+indid+"_selfIntersectedFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:SyntheticVertexFaces rdf:type owl:Class .\n";    
    infoStr+="giga:SyntheticVertexFaces rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_syntheticVertexFaces rdf:type giga:SyntheticVertexFaces .\n";   
    infoStr+="giga:"+indid+"_syntheticVertexFaces rdfs:label \""+indname+" Amount of synthetic vertex faces\"@en .\n";
    infoStr+="giga:"+indid+"_syntheticVertexFaces rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES])+"\"^^xsd:integer .\n";  
    infoStr+="giga:syntheticVertexFaces rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:syntheticVertexFaces rdfs:domain giga:Mesh .\n";
    infoStr+="giga:syntheticVertexFaces rdfs:label \"Faces with synthetic vertices\"@en .\n";
    infoStr+="giga:syntheticVertexFaces skos:definition \"The total number of faces with synthetic vertices\" .\n";
    infoStr+="giga:syntheticVertexFaces rdfs:comment \"Gigamesh Info: Faces with synthetic vertices\" .\n";
    infoStr+="giga:"+indid+" giga:syntheticVertexFaces giga:"+indid+"_syntheticVertexFaces .\n";
    infoStr+="giga:"+indid+"_syntheticVertexFaces wdt:P361 giga:"+indid+"_totalNumberOfFaces .\n";
    infoStr+="giga:ConnectedComponents rdf:type owl:Class .\n";    
    infoStr+="giga:ConnectedComponents rdfs:subClassOf giga:GigameshInfo .\n";    
    infoStr+="giga:"+indid+"_connectedComponents rdf:type giga:ConnectedComponents .\n";   
    infoStr+="giga:"+indid+"_connectedComponents rdfs:label \""+indname+" Amount of connected components\"@en .\n";
    infoStr+="giga:"+indid+"_connectedComponents rdf:value \""+std::to_string(this->mCountULong[MeshInfoData::CONNECTED_COMPONENTS])+"\"^^xsd:integer .\n";  
    infoStr+="giga:amountOfConnectedComponents rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:amountOfConnectedComponents rdfs:domain giga:Mesh .\n";
    infoStr+="giga:amountOfConnectedComponents rdfs:label \"Connected components\"@en .\n";
    infoStr+="giga:amountOfConnectedComponents skos:definition \"The total number of connected components\" .\n";
    infoStr+="giga:amountOfConnectedComponents rdfs:comment \"Gigamesh Info: Connected components\" .\n";
    infoStr+="giga:"+indid+" giga:amountOfConnectedComponents giga:"+indid+"_connectedComponents .\n";
    infoStr+="giga:boundingBox rdf:type owl:ObjectProperty .\n";
    infoStr+="giga:boundingBox rdfs:domain giga:Mesh .\n";
    infoStr+="giga:boundingBox rdfs:label \"Bounding Box\"@en . \n";
    infoStr+="giga:boundingBox skos:definition \"The bounding box of the mesh\" .\n";
    infoStr+="giga:boundingBox rdfs:comment \"Gigamesh Info: The bounding box of the mesh\" .\n";
    infoStr+="giga:"+indid+"_geom rdf:type geo:Geometry .\n";
    infoStr+="giga:"+indid+"_geom rdfs:label \""+indname+" Bounding Box\"@en .\n";
    infoStr+="giga:"+indid+" giga:boundingBox giga:"+indid+"_geom . giga:"+indid+"_geom geo:asWKT \"ENVELOPE("+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X])+" "+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y])+" "+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z])+","+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X])+" "+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y])+" "+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z])+")\"^^geo:wktLiteral .\n";
    infoStr+="giga:minimumXCoordinate rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:minimumXCoordinate rdfs:domain geo:Geometry .\n";
    infoStr+="giga:minimumXCoordinate rdfs:label \"Minimum X Coordinate\"@en . \n";
    infoStr+="giga:minimumXCoordinate skos:definition \"The minimum x coordinate of the mesh\" .\n";
    infoStr+="giga:minimumXCoordinate rdfs:comment \"Gigamesh Info: Minimum X Coordinate\" .\n";
    infoStr+="giga:"+indid+"_geom giga:minimumXCoordinate \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X])+"\"^^xsd:double .\n";
    infoStr+="giga:minimumYCoordinate rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:minimumYCoordinate rdfs:domain geo:Geometry .\n";
    infoStr+="giga:minimumYCoordinate rdfs:label \"Minimum Y Coordinate\"@en . \n";
    infoStr+="giga:minimumYCoordinate skos:definition \"The minimum y coordinate of the mesh\" .\n";
    infoStr+="giga:minimumYCoordinate rdfs:comment \"Gigamesh Info: Minimum Y Coordinate\" .\n";
    infoStr+="giga:"+indid+"_geom giga:minimumYCoordinate \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y])+"\"^^xsd:double .\n";
    infoStr+="giga:minimumZCoordinate rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:minimumZCoordinate rdfs:domain geo:Geometry .\n";
    infoStr+="giga:minimumZCoordinate rdfs:label \"Minimum Z Coordinate\"@en . \n";
    infoStr+="giga:minimumZCoordinate skos:definition \"The minimum z coordinate of the mesh\" .\n";
    infoStr+="giga:minimumZCoordinate rdfs:comment \"Gigamesh Info: Minimum Z Coordinate\" .\n";
    infoStr+="giga:"+indid+"_geom giga:minimumZCoordinate \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z])+"\"^^xsd:double .\n";
    infoStr+="giga:maximumXCoordinate rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:maximumXCoordinate rdfs:domain geo:Geometry .\n";
    infoStr+="giga:maximumXCoordinate rdfs:label \"Maximum X Coordinate\"@en . \n";
    infoStr+="giga:maximumXCoordinate skos:definition \"The maximum x coordinate of the mesh\" .\n";
    infoStr+="giga:maximumXCoordinate rdfs:comment \"Gigamesh Info: Maximum X Coordinate\" .\n";
    infoStr+="giga:"+indid+"_geom giga:maximumXCoordinate \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X])+"\"^^xsd:double .\n";
    infoStr+="giga:maximumYCoordinate rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:maximumYCoordinate rdfs:domain geo:Geometry .\n";
    infoStr+="giga:maximumYCoordinate rdfs:label \"Maximum Y Coordinate\"@en . \n";
    infoStr+="giga:maximumYCoordinate skos:definition \"The maximum y coordinate of the mesh\" .\n";
    infoStr+="giga:maximumYCoordinate rdfs:comment \"Gigamesh Info: Maximum Y Coordinate\" .\n";
    infoStr+="giga:"+indid+"_geom giga:maximumYCoordinate \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y])+"\"^^xsd:double .\n";
    infoStr+="giga:maximumZCoordinate rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:maximumZCoordinate rdfs:domain geo:Geometry .\n";
    infoStr+="giga:maximumZCoordinate rdfs:label \"Maximum Z Coordinate\"@en . \n";
    infoStr+="giga:maximumZCoordinate skos:definition \"The maximum y coordinate of the mesh\" .\n";
    infoStr+="giga:maximumZCoordinate rdfs:comment \"Gigamesh Info: Maximum Z Coordinate\" .\n";
    infoStr+="giga:"+indid+"_geom giga:maximumZCoordinate \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z])+"\"^^xsd:double .\n";
    infoStr+="giga:boundingBoxWidth rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:boundingBoxWidth rdfs:domain geo:Geometry .\n";
    infoStr+="giga:boundingBoxWidth rdfs:label \"Bounding Box Width\"@en . \n";
    infoStr+="giga:boundingBoxWidth skos:definition \"The width of the bounding box encompassing the mesh\" .\n";
    infoStr+="giga:boundingBoxWidth rdfs:comment \"Gigamesh Info: Bounding Box Width\" .\n";
    infoStr+="giga:"+indid+"_geom giga:boundingBoxWidth \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_WIDTH])+"\"^^xsd:double .\n";
    infoStr+="giga:boundingBoxHeight rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:boundingBoxHeight rdfs:domain geo:Geometry .\n";
    infoStr+="giga:boundingBoxHeight rdfs:label \"Bounding Box Height\"@en. \n";
    infoStr+="giga:boundingBoxHeight skos:definition \"The height of the bounding box encompassing the mesh\" .\n";
    infoStr+="giga:boundingBoxHeight rdfs:comment \"Gigamesh Info: Bounding Box Height\" .\n";
    infoStr+="giga:"+indid+"_geom giga:boundingBoxHeight \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_HEIGHT])+"\"^^xsd:double .\n";
    infoStr+="giga:boundingBoxThickness rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:boundingBoxThickness rdfs:domain geo:Geometry .\n";
    infoStr+="giga:boundingBoxThickness rdfs:label \"Bounding Box Thickness\"@en . \n";
    infoStr+="giga:boundingBoxThickness skos:definition \"The thickness of the bounding box encompassing the mesh\" .\n";
    infoStr+="giga:boundingBoxThickness rdfs:comment \"Gigamesh Info: Bounding Box Thickness\" .\n";
    infoStr+="giga:"+indid+"_geom giga:boundingBoxThickness \""+std::to_string(this->mCountDouble[MeshInfoData::BOUNDINGBOX_THICK])+"\"^^xsd:double .\n";
    infoStr+="giga:largestArea rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:largestArea rdfs:domain giga:Mesh .\n";
    infoStr+="giga:largestArea rdfs:label \"Largest Area\"@en . \n";
    infoStr+="giga:largestArea skos:definition \"The largest area of the mesh\" .\n";
    infoStr+="giga:largestArea rdfs:comment \"Gigamesh Info: Largest Area\" .\n";
    infoStr+="giga:"+indid+" giga:largestArea \""+std::to_string(this->mCountDouble[MeshInfoData::FACES_AREA_LARGEST])+"\"^^xsd:double .\n";
    infoStr+="giga:smallestArea rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:smallestArea rdfs:domain giga:Mesh .\n";
    infoStr+="giga:smallestArea rdfs:label \"Smallest Area\"@en . \n";
    infoStr+="giga:smallestArea skos:definition \"The smallest area of the mesh\" .\n";
    infoStr+="giga:smallestArea rdfs:comment \"Gigamesh Info: Smallest Area\" .\n";
    infoStr+="giga:"+indid+" giga:smallestArea \""+std::to_string(this->mCountDouble[MeshInfoData::FACES_AREA_SMALLEST])+"\"^^xsd:double .\n";
    infoStr+="giga:totalArea rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:totalArea rdfs:domain giga:Mesh .\n";
    infoStr+="giga:totalArea rdfs:label \"Total Area\"@en . \n";
    infoStr+="giga:totalArea skos:definition \"The total area of the mesh\" .\n";
    infoStr+="giga:totalArea rdfs:comment \"Gigamesh Info: Total Area\" .\n";
    infoStr+="giga:"+indid+" giga:totalArea \""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_AREA])+"\"^^xsd:double .\n";
    infoStr+="giga:totalVolumeDX rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:totalVolumeDX rdfs:domain giga:Mesh .\n";
    infoStr+="giga:totalVolumeDX rdfs:label \"Total Volume DX\"@en . \n";
    infoStr+="giga:totalVolumeDX skos:definition \"The total volume DX of the mesh\" .\n";
    infoStr+="giga:totalVolumeDX rdfs:comment \"Gigamesh Info: Total Volume DX\" .\n";
    infoStr+="giga:"+indid+" giga:totalVolumeDX \""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DX])+"\"^^xsd:double .\n";
    infoStr+="giga:totalVolumeDY rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:totalVolumeDY rdfs:domain giga:Mesh .\n";
    infoStr+="giga:totalVolumeDY rdfs:label \"Total Volume DY\"@en. \n";
    infoStr+="giga:totalVolumeDY skos:definition \"The total volume DY of the mesh\" .\n";
    infoStr+="giga:totalVolumeDY rdfs:comment \"Gigamesh Info: Total Volume DY\" .\n";
    infoStr+="giga:"+indid+" giga:totalVolumeDY \""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DY])+"\"^^xsd:double .\n";
    infoStr+="giga:totalVolumeDZ rdf:type owl:DatatypeProperty .\n";
    infoStr+="giga:totalVolumeDZ rdfs:domain giga:Mesh .\n";
    infoStr+="giga:totalVolumeDZ rdfs:label \"Total Volume DZ\"@en . \n";
    infoStr+="giga:totalVolumeDZ skos:definition \"The total volume DZ of the mesh\" .\n";
    infoStr+="giga:totalVolumeDZ rdfs:comment \"Gigamesh Info: Total Volume DZ\" .\n";
    infoStr+="giga:"+indid+" giga:totalVolumeDZ \""+std::to_string(this->mCountDouble[MeshInfoData::TOTAL_VOLUME_DZ])+"\"^^xsd:double .\n";
    rInfoTTL = infoStr;
	return( true );
}

//! Format mesh information as HTML.
//!
//! See Mesh::dumpMeshInfo for a related plain text method.
//!
//! @returns false in case of an error. True otherwise.
bool MeshInfoData::getMeshInfoHTML(
        std::string&          rInfoHTML    //!< Output: String to be passed e.g. for Qt Infobox
) {
	// Compute and format relative amount
	//-----------------------------------------------------------
	std::stringstream fractionsFormatted[MeshInfoData::ULONG_COUNT];
	for( int i=0; i<=MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX; i++ ) {
		fractionsFormatted[i] << std::fixed << std::setprecision(2) << this->mCountULong[i]*100.0 / this->mCountULong[MeshInfoData::VERTICES_TOTAL];
	}
	for( int i=MeshInfoData::FACES_SOLO; i<MeshInfoData::ULONG_COUNT; i++ ) {
		fractionsFormatted[i] << std::fixed << std::setprecision(2) << this->mCountULong[i]*100.0 / this->mCountULong[MeshInfoData::FACES_TOTAL];
	}

	// Compute the area and the resolution of the mesh
	//-----------------------------------------------------------
	double areaAcq = this->mCountDouble[MeshInfoData::TOTAL_AREA];
	std::string areaAcqStr   = std::to_string( areaAcq );
	std::string avgResMetric = std::to_string( this->mCountULong[MeshInfoData::VERTICES_TOTAL]/areaAcq );
	std::string avgResDPI    = std::to_string( 2.54*sqrt(this->mCountULong[MeshInfoData::VERTICES_TOTAL]/areaAcq) );
	// Format the numbers
	std::size_t foundDot = areaAcqStr.find('.');
	if( foundDot > 4 ) { // Cut-off fractions for large values
		areaAcqStr = areaAcqStr.substr( 0 , foundDot );
	} else { // Show only 5 digits including the fraction
		areaAcqStr = areaAcqStr.substr( 0 , 6 );
	}
	foundDot = avgResMetric.find('.');
	if( foundDot > 4 ) { // Cut-off fractions for large values
		avgResMetric = avgResMetric.substr( 0 , foundDot );
	} else { // Show only 5 digits including the fraction
		avgResMetric = avgResMetric.substr( 0 , 6 );
	}
	foundDot = avgResDPI.find('.');
	if( foundDot > 4 ) { // Cut-off fractions for large values
		avgResDPI = avgResDPI.substr( 0 , foundDot );
	} else { // Show only 5 digits including the fraction
		avgResDPI = avgResDPI.substr( 0 , 6 );
	}
	//-----------------------------------------------------------

	std::string tableBorder = "0"; // For visual debugging set to 1 - 0 (zero) for release!
    std::string indid;
    indid=urlEncode(this->mStrings[MeshInfoData::FILENAME]);
	std::string infoStr = "<!DOCTYPE html>\n";
	infoStr += "<html about=\""+indid+"\" typeof=\"http://www.gigamesh.eu/ont#Mesh\">\n";
	infoStr += "<head>\n";
	infoStr += "<title property=\"http://purl.org/dc/elements/1.1/title\">GigaMesh Information about [" + this->mStrings[MeshInfoData::FILENAME] + "]</title>\n";
	infoStr += "</head>\n";
	infoStr += "<body>\n";

	infoStr += "<b>Filename:</b> <span property=\"http://purl.org/dc/elements/1.1/identifier\">" + this->mStrings[MeshInfoData::FILENAME] + "</span><br />\n";
	infoStr += "<br />\n";

	infoStr += "Connected components: ";
	if( isnormal( static_cast<double>(this->mCountULong[MeshInfoData::CONNECTED_COMPONENTS]) ) ) {
		infoStr += "<span property=\"http://www.gigamesh.eu/ont#connectedComponentCountDifference\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">"+std::to_string( this->mCountULong[MeshInfoData::CONNECTED_COMPONENTS] )+"</span>";
	} else {
		infoStr += "not determined";
	}
	infoStr += "<br />\n";

	// Outer table - Row I, Col I
	infoStr += "<table align='center' border='" + tableBorder + "'>\n";
	infoStr += "<tr><td align='center'>\n";

	infoStr += "<b property=\"http://www.gigamesh.eu/ont#boundingBox\" resource=\""+indid+"_geom\">Bounding Box</b> in mm (assumed)\n";
	infoStr += "<table border='" + tableBorder + "' about=\""+indid+"_geom\" typeof=\"http://www.opengis.net/ont/geosparql#Geometry\">\n";
	infoStr += "<tr><td>X:</td><td align='right' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#minimumXCoordinate\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;&#8722;&nbsp;</td>";
	infoStr += "<td align='left' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#maximumXCoordinate\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;=&nbsp;</td>";
	infoStr += "<td align='center' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#boundingboxWidth\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">&nbsp;" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X] - this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X] ) + "</td></tr>\n";
	infoStr += "<tr><td>Y:</td><td align='right' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#minimumYCoordinate\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;&#8722;&nbsp;</td>";
	infoStr += "<td align='left' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#maximumYCoordinate\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;=&nbsp;</td>";
	infoStr += "<td align='center' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#boundingboxHeight\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">&nbsp;" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y] - this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y] ) + "</td></tr>\n";
	infoStr += "<tr><td>Z:</td><td align='right' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#minimumZCoordinate\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;&#8722;&nbsp;</td>";
	infoStr += "<td align='left' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#maximumZCoordinate\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z] ) + "</td>";
	infoStr += "<td align='center'>&nbsp;=&nbsp;</td>";
	infoStr += "<td align='center' about=\""+indid+"_geom\" property=\"http://www.gigamesh.eu/ont#boundingBoxThickness\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">&nbsp;" + std::to_string( this->mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z] - this->mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z] ) + "</td></tr>\n";
	infoStr += "</table>\n";

	// Outer table - Row I, Col II
	infoStr += "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td align='center'>\n";

	infoStr += "(units assumed)\n";
	infoStr += "<table align=\"center\" width=\"99%\" border='" + tableBorder + "'>\n";
	infoStr += "<tr>\n";
	infoStr += "<td align=\"left\" property=\"https://www.w3.org/2003/12/exif/resolution\" resource=\""+indid+"_avgmres\"><b>Resolution,&nbsp;average:</b></td>";
    infoStr += "<td align=\"right\" about=\""+indid+"_avgmres\" property=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#value\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + avgResMetric + "</td>";
	infoStr += "<td align=\"left\"  about=\""+indid+"_avgmres\" property=\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\" resource=\"http://www.ontology-of-units-of-measure.org/resource/om-2/squareCentimetre\">cm<sup>-2</sup></td>";
	infoStr += "</tr>\n";
	infoStr += "<tr>\n";
	infoStr += "<td align=\"left\" property=\"https://www.w3.org/2003/12/exif/resolution\" resource=\""+indid+"_avgresdpi\"></td>";
	infoStr += "<td align=\"right\" about=\""+indid+"_avgresdpi\" property=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#value\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + avgResDPI + "</td>";
	infoStr += "<td align=\"left\" about=\""+indid+"_avgresdpi\" property=\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\" resource=\"http://www.ontology-of-units-of-measure.org/resource/om-2/dotsPerInch\">DPI</td>";
	infoStr += "</tr>\n";
	infoStr += "<tr>\n";
	infoStr += "<td align=\"left\" property=\"http://www.gigamesh.eu/ont#totalArea\" resource=\""+indid+"_totalArea\"><b>Total&nbsp;surface area:</b></td>";
	infoStr += "<td align=\"right\" about=\""+indid+"_totalArea\" property=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#value\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">" + areaAcqStr + "</td>";
	infoStr += "<td align=\"left\" about=\""+indid+"_totalArea\" property=\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\" resource=\"http://www.ontology-of-units-of-measure.org/resource/om-2/squareCentimetre\">cm<sup>2</sup></td>";
	infoStr += "</tr>\n";
	infoStr += "</table>\n";

	// Outer table - Row II, Col I
	infoStr += "</td></tr>\n"
	           "<tr><td colspan='3'><hr /></tr>\n"
	           "<tr><td>\n";

	infoStr += "<table align=\"center\" width=\"99%\" border='" + tableBorder + "'>\n";
	infoStr += "<tr><td colspan=\"3\" align=\"center\"><b>Vertices</b></td></tr>\n";
	infoStr += "<tr><td>Total:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#totalNumberOfVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"+ std::to_string( this->mCountULong[MeshInfoData::VERTICES_TOTAL] )                  + "</td><td align=\"center\">-</td></tr>\n";
	infoStr += "<tr><td>NaN<sup>a)</sup>&ensp;coordinate(s):</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#naNVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"  + std::to_string( this->mCountULong[MeshInfoData::VERTICES_NAN] )                    + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_NAN].str()                   + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Normal&nbsp;length&nbsp;not&nbsp;normal:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#normalLengthVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"+ std::to_string( this->mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL].str()      + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Solo:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#isolatedVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                                 + std::to_string( this->mCountULong[MeshInfoData::VERTICES_SOLO] )                   + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SOLO].str()                   + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Polyline:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#polylineVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                             + std::to_string( this->mCountULong[MeshInfoData::VERTICES_POLYLINE] )               + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_POLYLINE].str()               + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Border:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#borderVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                               + std::to_string( this->mCountULong[MeshInfoData::VERTICES_BORDER] )                 + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_BORDER].str()                 + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Non-manifold:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#nonManifoldVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                         + std::to_string( this->mCountULong[MeshInfoData::VERTICES_NONMANIFOLD] )            + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_NONMANIFOLD].str()            + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Singular:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#singularVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                             + std::to_string( this->mCountULong[MeshInfoData::VERTICES_SINGULAR] )               + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SINGULAR].str()               + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Inverted edge<sup>b)</sup>:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#verticesOnInvertedEdge\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"           + std::to_string( this->mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE] )       + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_ON_INVERTED_EDGE].str()       + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Part&nbsp;of&nbsp;zero&nbsp;area&nbsp;face:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#verticesPartOfZeroAreaFace\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"   + std::to_string( this->mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_PART_OF_ZERO_FACE].str()      + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Synthetic:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#syntheticVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                            + std::to_string( this->mCountULong[MeshInfoData::VERTICES_SYNTHETIC] )              + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SYNTHETIC].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Manual:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#manuallyAddedVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                               + std::to_string( this->mCountULong[MeshInfoData::VERTICES_MANUAL] )                 + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_MANUAL].str()                 + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Circle Centers:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#circleCenterVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                       + std::to_string( this->mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER] )          + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_CIRCLE_CENTER].str()          + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Selected:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#selectedVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                             + std::to_string( this->mCountULong[MeshInfoData::VERTICES_SELECTED] )               + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_SELECTED].str()               + "&#37;</td></tr>\n";
	infoStr += "<tr><td colspan=\"3\"><i><u>Function values:</u></i></td></tr>\n";
	infoStr += "<tr><td>...&ensp;non finite:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#finiteFunctionValueVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                           + std::to_string( this->mCountULong[MeshInfoData::VERTICES_TOTAL] - this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE] )         + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_FUNCVAL_FINITE].str()         + "&#37;</td></tr>\n";
	infoStr += "<tr><td>...&ensp;local&ensp;minimum:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#localFunctionMinValueVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                   + std::to_string( this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN].str()      + "&#37;</td></tr>\n";
	infoStr += "<tr><td>...&ensp;local&ensp;maximum:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#localFunctionMaxValueVertices\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                   + std::to_string( this->mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX] )      + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX].str()      + "&#37;</td></tr>\n";
	infoStr += "</table>\n";

	// Outer table - Row II, Col II
	infoStr += "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td>\n";

	infoStr += "<table align=\"center\" width=\"99%\" border='" + tableBorder + "'>\n";
	infoStr += "<tr><td colspan=\"3\" align=\"center\"><b>Faces</b></td></tr>\n";
	infoStr += "<tr><td>Total:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#totalNumberOfFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                                       + std::to_string( this->mCountULong[MeshInfoData::FACES_TOTAL] )                   + "</td><td align=\"center\">-</td></tr>\n";
	infoStr += "<tr><td>Manifold:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#manifoldFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                                    + std::to_string( this->mCountULong[MeshInfoData::FACES_MANIFOLD] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_MANIFOLD].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Non-manifold:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#nonManifoldFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                                + std::to_string( this->mCountULong[MeshInfoData::FACES_NONMANIFOLD] )             + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_NONMANIFOLD].str()           + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Inverted<sup>c)</sup>:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#invertedFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                       + std::to_string( this->mCountULong[MeshInfoData::FACES_INVERTED] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_INVERTED].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Sticky:</td><td align=\"right\">"                                      + std::to_string( this->mCountULong[MeshInfoData::FACES_STICKY] )                  + "</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#stickyFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">" + fractionsFormatted[MeshInfoData::FACES_STICKY].str()                + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Zero&ensp;area:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#zeroAreaFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                              + std::to_string( this->mCountULong[MeshInfoData::FACES_ZEROAREA] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_ZEROAREA].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td>Border:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#borderFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                                      + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER] )                  + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER].str()                + "&#37;</td></tr>\n";
	infoStr += "<tr><td>...&nbsp;3&nbsp;Vertices&nbsp;(3V):</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#threeBorderVertexFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"          + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES] )   + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_THREE_VERTICES].str() + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;Bridge&nbsp;triconn.&nbsp;(3V0E):</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#borderFacesBridgeTriConnection\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"   + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN] )   + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN].str() + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;Bridge&nbsp;(3V1E):</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#bridgeBorderFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                 + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_BRDIGE] )           + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_BRDIGE].str()         + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;Dangling&nbsp;(3V2E):</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#danglingBorderFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"               + std::to_string( this->mCountULong[MeshInfoData::FACES_BORDER_DANGLING] )         + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_BORDER_DANGLING].str()       + "&#37;</td></tr>\n";
	infoStr += "<tr><td>&nbsp;&nbsp;&nbsp;...&nbsp;3&nbsp;Edges&nbsp;(Solo,3E):</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#soloFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"        + std::to_string( this->mCountULong[MeshInfoData::FACES_SOLO] )                    + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_SOLO].str()                  + "&#37;</td></tr>\n";
	//infoStr += "<tr><td></td><td></td><td></td></tr>\n"; // Empty line
	infoStr += "<tr><td>Selected:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#selectedFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                                    + std::to_string( this->mCountULong[MeshInfoData::FACES_SELECTED] )                + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_SELECTED].str()              + "&#37;</td></tr>\n";
	infoStr += "<tr><td></td><td></td><td></td></tr>\n"; // Empty line
	infoStr += "<tr><td property=\"http://www.gigamesh.eu/ont#smallestArea\" resource=\""+indid+"_smallestArea\">Smallest&nbsp;area:</td><td align=\"right\" about=\""+indid+"_smallestArea\" property=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#value\" datatype=\"http://www.w3.org/2001/XMLSchema#double\">"+ std::to_string( this->mCountDouble[MeshInfoData::FACES_AREA_SMALLEST] )+ "</td><td align=\"right\" about=\""+indid+"_smallestArea\" property=\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\" resource=\"http://www.ontology-of-units-of-measure.org/resource/om-2/squareMillimetre\">mm<sup>2</sup></td></tr>\n";
	infoStr += "<tr><td property=\"http://www.gigamesh.eu/ont#largestArea\" resource=\""+indid+"_largestArea\">Largest&nbsp;area:</td><td align=\"right\" about=\""+indid+"_largestArea\" property=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#value\" ><span datatype=\"http://www.w3.org/2001/XMLSchema#double\">"                           + std::to_string( this->mCountDouble[MeshInfoData::FACES_AREA_LARGEST] )           + "</td><td align=\"right\" about=\""+indid+"_largestArea\" property=\"http://www.ontology-of-units-of-measure.org/resource/om-2/hasUnit\" resource=\"http://www.ontology-of-units-of-measure.org/resource/om-2/squareMillimetre\">mm<sup>2</sup></td></tr>\n";
    if(this->mCountULong[MeshInfoData::FACES_SELFINTERSECTED] != -1){
        infoStr += "<tr><td>Self-intersected:</td><td align=\"right\" property=\"http://www.gigamesh.eu/ont#selfIntersectedFaces\" datatype=\"http://www.w3.org/2001/XMLSchema#integer\">"                     + std::to_string( this->mCountULong[MeshInfoData::FACES_SELFINTERSECTED] )               + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_SELFINTERSECTED].str()               + "&#37;</td></tr>\n";
    }
    //infoStr += "<tr><td>Self-intersected:</td><td align=\"right\">"                     + std::to_string( this->mCountULong[MeshInfoData::FACES_SELFINTERSECTED] )               + "</td><td align=\"right\">" + fractionsFormatted[MeshInfoData::FACES_SELFINTERSECTED].str()               + "&#37;</td></tr>\n";
    infoStr += "</table>\n";

	// Outer table - End
	infoStr += "</td></tr>\n";
	infoStr += "</table>\n";

	//! \todo provide more information about polylines - maybe using another dialog.
	//infoStr += "<p align=\"center\"><b>Polylines:</b>&nbsp;" + std::to_string( getPolyLineNr() ) + "</p>\n";

	// Footnotes
	infoStr += "<p align='left'>";
	infoStr += "<i>a)</i> Not a Number.<br />\n";
	infoStr += "<i>b)</i> Some non-manifold edges are counted as inverted.<br />\n";
	infoStr += "<i>c)</i> Contains non-manifold faces.<br />\n";
	infoStr += "</p>";

	infoStr += "</body>\n";
	infoStr += "</html>\n";
	// std::cout << infoStr << std::endl; // For debugging the HTML code.

	rInfoHTML = infoStr;
	return( true );
}

bool MeshInfoData::getMeshInfoPropertyName(
                const MeshInfoData::eMeshPropertyString rPropId,
                std::string& rPropName
) {
	if( rPropId == STRING_COUNT ) {
		return( false );
	}
	rPropName = mStringName[rPropId];
	return( true );
}


bool MeshInfoData::getMeshInfoPropertyName(
                const MeshInfoData::eMeshPropertyULongCount rPropId,
                std::string& rPropName
) {
	if( rPropId == ULONG_COUNT ) {
		return( false );
	}
	rPropName = mCountULongName[rPropId];
	return( true );
}


bool MeshInfoData::getMeshInfoPropertyName(
                const MeshInfoData::eMeshPropertyDouble rPropId,
                std::string& rPropName
) {
	if( rPropId == DOUBLE_COUNT ) {
		return( false );
	}
	rPropName = mmCountDoubleName[rPropId];
	return( true );
}
