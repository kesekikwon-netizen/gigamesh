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

#include <fstream>
#include <iostream>
#include <sstream>

#include "obj.h"

using namespace object_io;

Object_Information object_io::obj::load_obj(const std::string &path) {
	std::ifstream file{path};
	Object_Information object_information;
	if (file.fail()) {
		throw std::invalid_argument("File not found.");
	}

	std::string line;
	auto &vertex_locations = object_information.first;
	auto &triangle_indices = object_information.second;
	while (std::getline(file, line)) {
		std::istringstream iss{line};
		std::string line_type_str;
		if (!(iss >> line_type_str)) {
			continue;
		}
		if (line_type_str == "v") {
			double x, y, z;
			iss >> x >> y >> z;
			vertex_locations.push_back({x, y, z});
		} else if (line_type_str == "f") {
			std::string s_1, s_2, s_3;
			iss >> s_1 >> s_2 >> s_3;
			if (!s_1.empty() && !s_2.empty() && !s_3.empty()) {
				std::size_t i_1, i_2, i_3;
				auto string_to_index =
				    [](const std::string &string) {
					    std::size_t index = std::string::npos;
					    std::stringstream{string.substr(
						0, string.find("/"))} >>
						index;
					    return index;
				    };
				i_1 = string_to_index(s_1);
				i_2 = string_to_index(s_2);
				i_3 = string_to_index(s_3);
				triangle_indices.push_back(
				    {i_1 - 1, i_2 - 1, i_3 - 1});
			}
		}
	}

	file.close();

	return object_information;
}

void object_io::obj::save_obj(const std::string &path,
			      const Object_Information &object_information) {
	const auto &vertex_locations = object_information.first;
	const auto &triangle_indices = object_information.second;

	std::ofstream file{path};
	if (file.fail()) {
		std::cout << "File \"" + path + "\" not found." << std::endl;
		return;
	}

	for (const auto &location : vertex_locations) {
		file << "v " << static_cast<float>(location[0]) << " "
		     << static_cast<float>(location[1]) << " "
		     << static_cast<float>(location[2]) << "\n";
	}
	file << "s off"
	     << "\n";
	for (const auto &indices : triangle_indices) {
		file << "f " << (indices[0] + 1) << " " << (indices[1] + 1)
		     << " " << (indices[2] + 1) << "\n";
	}

	file.close();
}
