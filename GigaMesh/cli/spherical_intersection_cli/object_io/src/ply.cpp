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
#include <memory>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <vector>
#include <limits>

#include "endian.h"
#include "ply.h"

using namespace object_io;
using namespace ply;
using namespace endian;

namespace {
enum Property_Type {
	const_one,
	int8,
	uint8,
	int16,
	uint16,
	int32,
	uint32,
	float32,
	float64
};

template <Property_Type Property_Type> struct Property_Type_Traits {};
template <> struct Property_Type_Traits<int8> {
	using Implementation = std::int8_t;
};
template <> struct Property_Type_Traits<uint8> {
	using Implementation = std::uint8_t;
};
template <> struct Property_Type_Traits<int16> {
	using Implementation = std::int16_t;
};
template <> struct Property_Type_Traits<uint16> {
	using Implementation = std::uint16_t;
};
template <> struct Property_Type_Traits<int32> {
	using Implementation = std::int32_t;
};
template <> struct Property_Type_Traits<uint32> {
	using Implementation = std::uint32_t;
};
template <> struct Property_Type_Traits<float32> {
	static_assert(std::numeric_limits<float>::is_iec559);
	using Implementation = float;
};
template <> struct Property_Type_Traits<float64> {
	static_assert(std::numeric_limits<double>::is_iec559);
	using Implementation = double;
};

template <typename T>
T get_as(Property_Type type, char *data_ptr, Endian endian) {
	static_assert(sizeof(char) == sizeof(unsigned char));
	switch (type) {
	case int8:
		return parse_binary<
		    typename Property_Type_Traits<int8>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	case uint8:
		return parse_binary<
		    typename Property_Type_Traits<uint8>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	case int16:
		return parse_binary<
		    typename Property_Type_Traits<int16>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	case uint16:
		return parse_binary<
		    typename Property_Type_Traits<uint16>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	case int32:
		return parse_binary<
		    typename Property_Type_Traits<int32>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	case uint32:
		return parse_binary<
		    typename Property_Type_Traits<uint32>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	case float32:
		return parse_binary<
		    typename Property_Type_Traits<float32>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	case float64:
		return parse_binary<
		    typename Property_Type_Traits<float64>::Implementation>(
		    reinterpret_cast<unsigned char *>(data_ptr), endian);
	default:
		throw std::invalid_argument("Unknown element type.");
	}
}

std::size_t get_byte_size(Property_Type type) {
	switch (type) {
	case int8:
		return sizeof(Property_Type_Traits<int8>::Implementation);
	case uint8:
		return sizeof(Property_Type_Traits<uint8>::Implementation);
	case int16:
		return sizeof(Property_Type_Traits<int16>::Implementation);
	case uint16:
		return sizeof(Property_Type_Traits<uint16>::Implementation);
	case int32:
		return sizeof(Property_Type_Traits<int32>::Implementation);
	case uint32:
		return sizeof(Property_Type_Traits<uint32>::Implementation);
	case float32:
		return sizeof(Property_Type_Traits<float32>::Implementation);
	case float64:
		return sizeof(Property_Type_Traits<float64>::Implementation);
	default:
		throw std::invalid_argument("Unknown element type.");
	}
}

struct Elements {
	std::string name;
	std::size_t element_count;
	std::vector<std::string> property_names;
	std::vector<Property_Type> property_size_types;
	std::vector<Property_Type> property_value_types;
};

std::pair<Format, std::vector<Elements>> parse_header(std::ifstream &file) {
	Format format;
	std::vector<Elements> elements_container;

	const std::unordered_map<std::string, Format> name_to_format{
	    {"ascii", ascii},
	    {"binary_little_endian", binary_little_endian},
	    {"binary_big_endian", binary_big_endian}};

	const std::unordered_map<std::string, Property_Type> name_to_type = {
	    {"char", int8},      {"uchar", uint8},    {"short", int16},
	    {"ushort", uint16},  {"int", int32},      {"uint", uint32},
	    {"float", float32},  {"double", float64}, {"int8", int8},
	    {"uint8", uint8},    {"int16", int16},    {"uint16", uint16},
	    {"int32", int32},    {"uint32", uint32},  {"float32", float32},
	    {"float64", float64}};

	std::string line;
	while (std::getline(file, line)) {
		std::stringstream ss{line};
		std::string line_identifier;
		ss >> line_identifier;
		if (line_identifier == "format") {
			std::string format_name;
			ss >> format_name;
			format = name_to_format.at(format_name);
		}
		if (line_identifier == "element") {
			elements_container.emplace_back();
			ss >> elements_container.back().name;
			ss >> elements_container.back().element_count;
		}
		if (line_identifier == "property") {
			std::string type_name;
			ss >> type_name;
			if (type_name == "list") {
				std::string size_type_name;
				std::string element_type_name;
				std::string name;
				ss >> size_type_name;
				ss >> element_type_name;
				ss >> name;
				elements_container.back()
				    .property_size_types.push_back(
					name_to_type.at(size_type_name));
				elements_container.back()
				    .property_value_types.push_back(
					name_to_type.at(element_type_name));
				elements_container.back()
				    .property_names.push_back(name);
			} else {
				std::string name;
				ss >> name;
				elements_container.back()
				    .property_size_types.push_back(const_one);
				elements_container.back()
				    .property_value_types.push_back(
					name_to_type.at(type_name));
				elements_container.back()
				    .property_names.push_back(name);
			}
		}
		if (line_identifier == "end_header") {
			break;
		}
	}
	return {format, elements_container};
}

std::vector<std::string> read_property_ascii(Property_Type size_type,
					     std::ifstream &file) {
	std::string first_non_empty;
	do {
		file >> first_non_empty;
	} while (first_non_empty.empty());
	if (size_type == const_one) {
		return {first_non_empty};
	}
	std::size_t size = std::stoul(first_non_empty);
	std::vector<std::string> output;
	output.reserve(size);
	for (std::size_t index = 0; index < size; index++) {
		output.emplace_back();
		file >> output.back();
	}
	return output;
}

Object_Information
parse_body_ascii(const std::vector<Elements> &elements_container,
		 std::ifstream &file) {

	Object_Information object_information;
	auto &vertex_locations = object_information.first;
	auto &triangle_indices = object_information.second;

	auto parse_vertices = [&file,
			       &vertex_locations](const Elements &elements) {
		for (std::size_t element_index = 0;
		     element_index < elements.element_count; element_index++) {
			Vertex_Location location;
			for (std::size_t property_index = 0;
			     property_index < elements.property_names.size();
			     property_index++) {
				auto size_type =
				    elements
					.property_size_types[property_index];
				auto name =
				    elements.property_names[property_index];
				auto values =
				    read_property_ascii(size_type, file);
				if (name == "x") {
					location[0] = std::stod(values[0]);
				}
				if (name == "y") {
					location[1] = std::stod(values[0]);
				}
				if (name == "z") {
					location[2] = std::stod(values[0]);
				}
			}
			vertex_locations.push_back(location);
		}
	};

	auto parse_triangles = [&file,
				&triangle_indices](const Elements &elements) {
		for (std::size_t element_index = 0;
		     element_index < elements.element_count; element_index++) {
			Triangle_Indices indices;
			for (std::size_t property_index = 0;
			     property_index < elements.property_names.size();
			     property_index++) {
				auto size_type =
				    elements
					.property_size_types[property_index];
				auto name =
				    elements.property_names[property_index];
				auto values =
				    read_property_ascii(size_type, file);
				if (name == "vertex_indices"  && values.size() == 3) {
					indices[0] = std::stoul(values[0]);
					indices[1] = std::stoul(values[1]);
					indices[2] = std::stoul(values[2]);
				}
			}
			triangle_indices.push_back(indices);
		}
	};

	auto skip_elements = [&file](const Elements &elements) {
		for (std::size_t element_index = 0;
		     element_index < elements.element_count; element_index++) {
			for (std::size_t property_index = 0;
			     property_index < elements.property_names.size();
			     property_index++) {
				auto size_type =
				    elements
					.property_size_types[property_index];
				read_property_ascii(size_type, file);
			}
		}
	};

	for (const auto &elements : elements_container) {
		if (elements.name == "vertex") {
			parse_vertices(elements);
			continue;
		}
		if (elements.name == "face") {
			parse_triangles(elements);
			continue;
		}
		skip_elements(elements);
	}

	return object_information;
}

std::vector<char> read_property_binary(Property_Type size_type,
				       Property_Type element_type,
				       std::ifstream &file, Endian endian) {
	std::vector<char> output;
	auto append_element = [&file, &output, &element_type]() {
		auto index = output.size();
		auto byte_size = get_byte_size(element_type);
		output.resize(index + byte_size);
		file.read(&output[index], byte_size);
	};
	if (size_type == const_one) {
		append_element();
		return output;
	}
	auto size_data_size = get_byte_size(size_type);
	std::vector<char> size_data(size_data_size);
	file.read(size_data.data(), size_data_size);
	std::size_t size =
	    get_as<std::size_t>(size_type, size_data.data(), endian);
	output.reserve(size);
	for (std::size_t index = 0; index < size; index++) {
		append_element();
	}
	return output;
}

Object_Information
parse_body_binary(const std::vector<Elements> &elements_container,
		  std::ifstream &file, Endian endian) {

	Object_Information object_information;
	auto &vertex_locations = object_information.first;
	auto &triangle_indices = object_information.second;

	auto parse_vertices = [&file, &vertex_locations,
			       &endian](const Elements &elements) {
		for (std::size_t element_index = 0;
		     element_index < elements.element_count; element_index++) {
			Vertex_Location location;
			for (std::size_t property_index = 0;
			     property_index < elements.property_names.size();
			     property_index++) {
				auto size_type =
				    elements
					.property_size_types[property_index];
				auto value_type =
				    elements
					.property_value_types[property_index];
				auto name =
				    elements.property_names[property_index];
				auto values = read_property_binary(
				    size_type, value_type, file, endian);
				if (name == "x") {
					location[0] = get_as<double>(
					    value_type, values.data(), endian);
				}
				if (name == "y") {
					location[1] = get_as<double>(
					    value_type, values.data(), endian);
				}
				if (name == "z") {
					location[2] = get_as<double>(
					    value_type, values.data(), endian);
				}
			}
			vertex_locations.push_back(location);
		}
	};

	auto parse_triangles = [&file, &triangle_indices,
				&endian](const Elements &elements) {
		for (std::size_t element_index = 0;
		     element_index < elements.element_count; element_index++) {
			Triangle_Indices indices;
			for (std::size_t property_index = 0;
			     property_index < elements.property_names.size();
			     property_index++) {
				auto size_type =
				    elements
					.property_size_types[property_index];
				auto value_type =
				    elements
					.property_value_types[property_index];
				auto name =
				    elements.property_names[property_index];
				auto values = read_property_binary(
				    size_type, value_type, file, endian);
				if (name == "vertex_indices" && values.size() / get_byte_size(value_type) == 3) {
					indices[0] = get_as<std::size_t>(
					    value_type, &values[0], endian);
					indices[1] = get_as<std::size_t>(
					    value_type,
					    &values[get_byte_size(value_type)],
					    endian);
					indices[2] = get_as<std::size_t>(
					    value_type,
					    &values[2 *
						    get_byte_size(value_type)],
					    endian);
				}
			}
			triangle_indices.push_back(indices);
		}
	};

	auto skip_elements = [&file, &endian](const Elements &elements) {
		for (std::size_t element_index = 0;
		     element_index < elements.element_count; element_index++) {
			for (std::size_t property_index = 0;
			     property_index < elements.property_names.size();
			     property_index++) {
				auto size_type =
				    elements
					.property_size_types[property_index];
				auto value_type =
				    elements
					.property_value_types[property_index];
				read_property_binary(size_type, value_type,
						     file, endian);
			}
		}
	};

	for (const auto &elements : elements_container) {
		if (elements.name == "vertex") {
			parse_vertices(elements);
			continue;
		}
		if (elements.name == "face") {
			parse_triangles(elements);
			continue;
		}
		skip_elements(elements);
	}

	return object_information;
}
} // namespace

Object_Information ply::load_ply(const std::string &path) {
	std::ifstream file{path};
	Object_Information object_information;
	if (file.fail()) {
		throw std::invalid_argument("File not found.");
	}

	auto [format, elements_container] = parse_header(file);

	switch (format) {
	case ascii:
		return parse_body_ascii(elements_container, file);
	case binary_little_endian:
		return parse_body_binary(elements_container, file, little);
	case binary_big_endian:
		return parse_body_binary(elements_container, file, big);
	default:
		throw std::invalid_argument("Unknown format.");
	}
}

namespace {
void write_header(std::ofstream &file, Format format, std::size_t vertex_count,
		  std::size_t face_count) {
	file << "ply\n";
	file << "format ";
	switch (format) {
	case ascii:
		file << "ascii 1.0"
		     << "\n";
		break;
	case binary_little_endian:
		file << "binary_little_endian 1.0"
		     << "\n";
		break;
	case binary_big_endian:
		file << "binary_big_endian 1.0"
		     << "\n";
		break;
	default:
		throw std::invalid_argument("Unknown format.");
		break;
	}
	file << "element vertex " << vertex_count << "\n";
	file << "property float x"
	     << "\n";
	file << "property float y"
	     << "\n";
	file << "property float z"
	     << "\n";
	file << "property float quality"
	     << "\n";
	file << "element face " << face_count << "\n";
	file << "property list uint32 uint32 vertex_indices"
	     << "\n";
	file << "end_header" << std::endl;
}

void write_body_ascii(const std::vector<Vertex_Location> &vertex_locations,
		      const std::vector<double> &qualities,
		      const std::vector<Triangle_Indices> &triangle_indices,
		      std::ofstream &file) {
	for (std::size_t vertex_index = 0;
	     vertex_index < vertex_locations.size(); vertex_index++) {
		const auto &location = vertex_locations[vertex_index];
		auto quality = qualities[vertex_index];
		file << location[0] << " " << location[1] << " " << location[2]
		     << " " << quality << "\n";
	}
	for (std::size_t triangle_index = 0;
	     triangle_index < triangle_indices.size(); triangle_index++) {
		const auto &indices = triangle_indices[triangle_index];
		file << "3 " << indices[0] << " " << indices[1] << " "
		     << indices[2] << "\n";
	}
}

template <typename T>
void append_as_binary(const T &value, std::ofstream &file, Endian endian) {
	auto data = get_binary(value, endian);
	file << std::string{reinterpret_cast<char *>(data.data()), data.size()};
}

void write_body_binary(const std::vector<Vertex_Location> &vertex_locations,
		       const std::vector<double> &qualities,
		       const std::vector<Triangle_Indices> &triangle_indices,
		       std::ofstream &file, Endian endian) {
	for (std::size_t vertex_index = 0;
	     vertex_index < vertex_locations.size(); vertex_index++) {
		const auto &location = vertex_locations[vertex_index];
		auto quality = qualities[vertex_index];
		append_as_binary<float>(location[0], file, endian);
		append_as_binary<float>(location[1], file, endian);
		append_as_binary<float>(location[2], file, endian);
		append_as_binary<float>(quality, file, endian);
	}
	for (std::size_t triangle_index = 0;
	     triangle_index < triangle_indices.size(); triangle_index++) {
		const auto &indices = triangle_indices[triangle_index];
		append_as_binary<std::uint32_t>(3, file, endian);
		append_as_binary<std::uint32_t>(indices[0], file, endian);
		append_as_binary<std::uint32_t>(indices[1], file, endian);
		append_as_binary<std::uint32_t>(indices[2], file, endian);
	}
}
} // namespace

void ply::save_ply(const std::string &path,
		   const Object_Information &object_information,
		   const std::vector<double> &qualities, Format format) {
	std::ofstream file{path};
	if (file.fail()) {
		std::cout << "File \"" + path + "\" not found." << std::endl;
		return;
	}

	auto &vertex_locations = object_information.first;
	auto &triangle_indices = object_information.second;

	write_header(file, format, vertex_locations.size(),
		     triangle_indices.size());

	switch (format) {
	case ascii:
		return write_body_ascii(vertex_locations, qualities,
					triangle_indices, file);
	case binary_little_endian:
		return write_body_binary(vertex_locations, qualities,
					 triangle_indices, file, little);
	case binary_big_endian:
		return write_body_binary(vertex_locations, qualities,
					 triangle_indices, file, big);
	default:
		throw std::invalid_argument("Unknown format.");
	}
}
