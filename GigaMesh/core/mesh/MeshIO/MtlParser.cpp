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

#include "MtlParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <GigaMesh/logging/Logging.h>

enum class MtlToken {
	NEWMTL,
	KA,
	KD,
	KS,
	D,
	ILLUM,
	TF,
	NS,
	SHARPNESS,
	NI,
	MAP_KA,
	MAP_KD,
	MAP_KS,
	MAP_NS,
	MAP_D,
	DISP,
	DECAL,
	BUMP,
	COMMENT,
	TR,
	KE,			//Ke is an extension for PBR rendering. Exported by Blender
	UNKNOWN
};

MtlToken toToken(const std::string& s)
{
	if(s[0] == '#')
		return MtlToken::COMMENT;
	if(s == "newmtl")
		return MtlToken::NEWMTL;
	if (s == "Ka")
		return MtlToken::KA;
	if (s == "Kd")
		return MtlToken::KD;
	if (s == "Ks")
		return MtlToken::KS;
	if (s == "d")
		return MtlToken::D;
	if(s == "illum")
		return MtlToken::ILLUM;
	if (s == "Tf")
		return MtlToken::TF;
	if (s == "Ns")
		return MtlToken::NS;
	if (s == "sharpness")
		return MtlToken::SHARPNESS;
	if (s == "Ni")
		return MtlToken::NI;
	if (s == "map_Ka")
		return MtlToken::MAP_KA;
	if (s == "map_Kd")
		return MtlToken::MAP_KD;
	if (s == "map_Ks")
		return MtlToken::MAP_KS;
	if (s == "map_Ns")
		return MtlToken::MAP_NS;
	if (s == "map_d")
		return MtlToken::MAP_D;
	if (s == "disp")
		return MtlToken::DISP;
	if (s == "decal")
		return MtlToken::DECAL;
	if (s == "bump")
		return MtlToken::BUMP;
	if (s == "Tr")
		return MtlToken::TR;
	if( s == "Ke")
		return MtlToken::KE;

	return MtlToken::UNKNOWN;
}

//see https://www.image-engineering.de/library/technotes/958-how-to-convert-between-srgb-and-ciexyz
void xyzToRgb(const std::array<float,3>& xyzCol, std::array<float,3>& rgbCol)
{
	rgbCol[0] =  3.2404542F * xyzCol[0] + -1.5371385F * xyzCol[1] + -0.4985413F * xyzCol[2];
	rgbCol[1] = -0.9692660F * xyzCol[0] +  1.8760108F * xyzCol[1] +  0.0415560F * xyzCol[2];
	rgbCol[2] =  0.0556434F * xyzCol[0] + -0.2040259F * xyzCol[1] +  1.0572252F * xyzCol[2];
}

/*
 * Color Value might be one of the following:
 * r g b
 * spectral file.rfl factor
 * xyz x y z
 */
void parseColorValue(std::stringstream& sStream, std::array<float,3>& color)
{
	std::string option;
	sStream >> option;

	if(option == "spectral")
	{
		LOG::warn() << "Warning, spectral opion is not supported\n";
	}

	else if(option == "xyz")
	{
		std::array<float,3> xyzCol {0.0F,0.0F,0.0F};

		sStream >> xyzCol[0];

		//y and z are optional
		if(!(sStream >> xyzCol[1]))
		{
			xyzCol[1] = xyzCol[0];
		}

		if(!(sStream >> xyzCol[2]))
		{
			xyzCol[2] = xyzCol[0];
		}

		xyzToRgb(xyzCol, color);
	}
	else
	{
		//if no option is given, the first string is the r component
		color[0] = std::stof(option);
		sStream >> color[1];
		sStream >> color[2];
	}

	//spec allows that rgb is outside 0.0-1.0 range. In this case the values need to be adjusted
	if(color[0] > 1.0F || color[1] > 1.0F || color[2] > 1.0F)
	{
		float maxVal = std::max(color[0], std::max(color[1], color[2]));

		color[0] /= maxVal;
		color[1] /= maxVal;
		color[2] /= maxVal;
	}
}

void parseFloatValue(std::stringstream& sStream, float& val)
{
	sStream >> val;
}

/*
 * Syntax is: <-options args> filename
 */

void parseTextureValue(std::stringstream& sStream, std::string& str)
{
	//skip the options, just get the texture-name, which is the last argument
	for(std::string temp; sStream >> temp;)
	{
		if(temp == "-blendu" ||
		   temp == "-blendv" ||
		   temp == "-cc"     ||
		   temp == "-clamp"  ||
		   temp == "-texres")
		{
			sStream >> temp;
		}
		else if(temp == "-mm")
		{
			sStream >> temp;
			sStream >> temp;
		}
		else if(temp == "-o" ||
		        temp == "-s" ||
		        temp == "-t")
		{
			sStream >> temp;
			sStream >> temp;
			sStream >> temp;
		}

		//handle fileName
		else
		{
			str = temp;
			//fileName may contain whitespaces
			while(sStream >> temp)
			{
				str += " " + temp;
			}
		}
	}
}


bool MtlParser::parseFile(const std::filesystem::path& fileName)
{
	std::ifstream fStream;

	fStream.imbue(std::locale("C"));
	fStream.open(fileName);

	if(!fStream.is_open())
		return false;

	std::string parsedLine;
	MtlMaterial* currentMaterial = nullptr;
	unsigned int lineNum = 0;
	while(std::getline(fStream,parsedLine))
	{
		++lineNum;
		if(parsedLine.length() <= 1)	//windows files may contain a CR character in a empty line
			continue;

		std::stringstream sStream(parsedLine);
		sStream.imbue(std::locale("C"));

		std::string stringToken;
		sStream >> stringToken;

		if(stringToken[0] == '#')
		{
			continue;
		}

		MtlToken token = toToken(stringToken);

		if(token == MtlToken::NEWMTL)
		{
			sStream >> stringToken;
			mMtlMaterials[stringToken].name = stringToken;
			currentMaterial = &mMtlMaterials[stringToken];
		}

		else if(currentMaterial != nullptr)
		{
			switch (token) {
				case MtlToken::NEWMTL:
					break;
				case MtlToken::KA:
					parseColorValue(sStream, currentMaterial->Ka);
					break;
				case MtlToken::KD:
					parseColorValue(sStream, currentMaterial->Kd);
					break;
				case MtlToken::KS:
					parseColorValue(sStream, currentMaterial->Ks);
					break;
				case MtlToken::D:
				{
					std::string option;
					sStream >> option;

					if(option == "-halo")
					{
						currentMaterial->dIsHalo = true;
						sStream >> currentMaterial->d;
					}
					else
					{
						currentMaterial->d = std::stof(option);
					}
				}
					break;
				case MtlToken::ILLUM:
				{
					short int val;
					sStream >> val;
					currentMaterial->illumModel = static_cast<unsigned char>(val);
				}
					break;
				case MtlToken::TF:
					parseColorValue(sStream, currentMaterial->Tf);
					break;
				case MtlToken::NS:
					parseFloatValue(sStream, currentMaterial->Ns);
					break;
				case MtlToken::SHARPNESS:
					parseFloatValue(sStream, currentMaterial->sharpness);
					break;
				case MtlToken::NI:
					parseFloatValue(sStream, currentMaterial->Ni);
					break;
				case MtlToken::MAP_KA:
					parseTextureValue(sStream, currentMaterial->map_Ka);
					break;
				case MtlToken::MAP_KD:
					parseTextureValue(sStream, currentMaterial->map_Kd);
					break;
				case MtlToken::MAP_KS:
					parseTextureValue(sStream, currentMaterial->map_Ks);
					break;
				case MtlToken::MAP_NS:
					parseTextureValue(sStream, currentMaterial->map_Ns);
					break;
				case MtlToken::MAP_D:
					parseTextureValue(sStream, currentMaterial->map_d);
					break;
				case MtlToken::DISP:
					parseTextureValue(sStream, currentMaterial->disp);
					break;
				case MtlToken::DECAL:
					parseTextureValue(sStream, currentMaterial->decal);
					break;
				case MtlToken::BUMP:
					parseTextureValue(sStream, currentMaterial->bump);
					break;
				case MtlToken::COMMENT:
					break;
				case MtlToken::TR:
				{
					float tr;
					sStream >> tr;
					currentMaterial->d = 1.0F - tr;
				}
					break;
				case MtlToken::KE:
					break;	//ignore KE
				case MtlToken::UNKNOWN:
					LOG::warn() << "Unknown MTL token in line " << lineNum << " : " << stringToken << "\n";
					break;
			}

		}
	}

	return true;
}

bool MtlParser::hasMaterial(const std::string& name)
{
	return mMtlMaterials.find(name) != mMtlMaterials.end();
}

MtlMaterial& MtlParser::getMaterial(const std::string& name)
{
	return mMtlMaterials[name];
}

std::unordered_map<std::string, MtlMaterial>& MtlParser::getMaterialsRef()
{
	return mMtlMaterials;
}
