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

#ifndef MTLPARSER_H
#define MTLPARSER_H

#include <string>
#include <unordered_map>
#include <array>
#include <filesystem>

struct MtlMaterial {
	//color and illumination
	std::array<float,3> Ka = {0.0F,0.0F,0.0F};
	std::array<float,3> Kd = {0.0F,0.0F,0.0F};
	std::array<float,3> Ks = {0.0F,0.0F,0.0F};
	std::array<float,3> Tf = {0.0F,0.0F,0.0F};

	float d                = 0.0F;
	float Ns               = 0.0F;
	float sharpness        = 0.0F;
	float Ni               = 0.0F;

	//name
	std::string name;

	//Texture map Statements
	std::string map_Ka;
	std::string map_Kd;
	std::string map_Ks;
	std::string map_Ns;
	std::string map_d;
	std::string disp;
	std::string decal;
	std::string bump;

	//isHalo and illumination, moved here for struct-packing reasons
	bool dIsHalo            = false;
	unsigned char illumModel= 0;
};



class MtlParser
{
	public:
		MtlParser() = default;

		bool parseFile(const std::filesystem::path& fileName);
		bool hasMaterial(const std::string& name);

		MtlMaterial& getMaterial(const std::string& name);
		std::unordered_map<std::string, MtlMaterial>& getMaterialsRef();

	private:
		std::unordered_map<std::string, MtlMaterial> mMtlMaterials;
};

#endif // MTLPARSER_H
