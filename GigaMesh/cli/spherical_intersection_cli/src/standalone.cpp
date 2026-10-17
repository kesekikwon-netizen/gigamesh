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

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "input_parser.h"
#include "obj.h"
#include "ply.h"
#include <spherical_intersection/algorithm/component_count.h>
#include <spherical_intersection/algorithm/sphere_surface_msii.h>
#include <spherical_intersection/algorithm/sphere_volume_msii.h>
#include <spherical_intersection/graph.h>
#include <spherical_intersection/mesh_spherical.h>
#include "timer.h"

namespace {
bool help_is_requested = false;

std::string input_path;
bool input_path_is_set = false;

enum File_Format { obj, ply };
File_Format input_format;
bool input_format_is_set = false;

double radius = 0.0;
bool radius_is_set = false;

std::function<double(spherical_intersection::Graph &)> algorithm;
bool algorithm_is_set = false;

std::string output_path;
bool output_path_is_set = false;

std::size_t thread_count = 1;
bool thread_count_is_set = false;

std::size_t max_thread_load;
bool max_thread_load_is_set = false;

void set_parser_up(Input_Parser &input_parser) {
	input_parser.add_flag(help_is_requested, "-help", "Displays help.");

	input_parser.add_value(input_path, "-input", "Sets the input path.",
			       &input_path_is_set);

	std::function<File_Format(const std::string &format_name)>
	    format_conversion =
		[](const std::string &format_name) -> File_Format {
		if (format_name == "obj") {
			return File_Format::obj;
		} else if (format_name == "ply") {
			return File_Format::ply;
		} else {
			throw std::invalid_argument(
			    "Unknown file format given.");
		}
	};

	input_parser.add_value(
	    input_format, "-format",
	    "Sets the input format. Possible values: obj, ply",
	    &input_format_is_set, format_conversion);

	input_parser.add_value(radius, "-radius", "Sets the sphere radius.",
			       &radius_is_set);

	std::function<std::function<double(spherical_intersection::Graph &)>(
	    const std::string &string)>
	    algorithm_conversion = [](const std::string &algorithm_name)
	    -> std::function<double(spherical_intersection::Graph &)> {
		if (algorithm_name == "sphere_surface") {
			return
			    [](spherical_intersection::Graph &graph) -> double {
				    return spherical_intersection::algorithm::
					get_sphere_surface_length(graph);
			    };
		} else if (algorithm_name == "sphere_volume") {
			return
			    [](spherical_intersection::Graph &graph) -> double {
				    return spherical_intersection::algorithm::
					get_sphere_volume_area(graph);
			    };
		} else if (algorithm_name == "components") {
			return
			    [](spherical_intersection::Graph &graph) -> double {
				    return spherical_intersection::algorithm::
					get_component_count(graph);
			    };
		} else {
			throw std::invalid_argument("Unknown algorithm given.");
		}
	};
	input_parser.add_value(algorithm, "-algorithm",
			       "Sets the algorithm. Possible values: "
			       "components, sphere_volume, sphere_surface",
			       &algorithm_is_set, algorithm_conversion);

	input_parser.add_value(
	    output_path, "-output",
	    "Sets the output path for the generated .ply file.",
	    &output_path_is_set);

	input_parser.add_value(thread_count, "-threads",
			       "Sets the number of threads to be used.",
			       &thread_count_is_set);

	input_parser.add_value(
	    max_thread_load, "-max_load",
	    "Sets the maximum number of values to be calculated in each thread "
	    "without notification.",
	    &max_thread_load_is_set);
}

bool arguments_are_valid() {
	bool result = true;
	if (!input_path_is_set) {
		std::cout << "Error: No input path given. Use -help for help."
			  << std::endl;
		result = false;
	}
	if (!input_format_is_set) {
		std::cout << "Error: No file format given. Use -help for help."
			  << std::endl;
		result = false;
	}
	if (!radius_is_set) {
		std::cout << "Error: No radius given. Use -help for help."
			  << std::endl;
		result = false;
	}
	if (!algorithm_is_set) {
		std::cout << "Error: No algorithm given. Use -help for help."
			  << std::endl;
		result = false;
	}
	if (!output_path_is_set) {
		std::cout << "Error: No output path given. Use -help for help."
			  << std::endl;
		result = false;
	}
	if (!thread_count_is_set) {
		std::cout << "Error: No thread count given. Use -help for help."
			  << std::endl;
		result = false;
	}
	if (!max_thread_load_is_set) {
		std::cout
		    << "Error: No maximum number of values to be calculated in "
		       "each thread given. Use -help for help."
		    << std::endl;
		result = false;
	}
	return result;
}

std::vector<double> compute_all_values(
    const spherical_intersection::Mesh &mesh, double radius,
    std::function<double(spherical_intersection::Graph &graph)> algorithm,
    std::size_t thread_count) {
	const auto &vertices = mesh.get_vertices();
	std::size_t vertex_count = vertices.size();
	std::vector<double> values(vertex_count);
	auto set_range = [&vertices, &values, &algorithm, &radius](
			     std::size_t start_index, std::size_t amount) {
		for (std::size_t index = start_index;
		     index < start_index + amount; index++) {
			const auto &vertex = vertices[index];
			spherical_intersection::math3d::Sphere sphere{
			    vertex.get_location(), radius};
			spherical_intersection::Graph graph{vertices[index],
							    sphere};
			values[index] = algorithm(graph);
		}
	};

	std::size_t current_start_index = 0;
	std::size_t load_per_thread = max_thread_load;
	std::cout << std::endl;
	while (thread_count * load_per_thread <
	       vertex_count - current_start_index) {
		std::vector<std::thread> threads;
		for (std::size_t thread_idx = 0; thread_idx < thread_count;
		     thread_idx++) {
			threads.emplace_back(set_range, current_start_index,
					     load_per_thread);
			current_start_index += load_per_thread;
		}
		for (auto &thread : threads) {
			thread.join();
		}
		std::cout << (100.0 * current_start_index) / vertex_count << "%"
			  << std::endl;
	}

	std::vector<std::thread> threads;
	load_per_thread = (vertex_count - current_start_index) / thread_count;
	std::size_t load_for_last_thread =
	    ((vertex_count - current_start_index) / thread_count) +
	    ((vertex_count - current_start_index) % thread_count);
	for (std::size_t thread_idx = 0; thread_idx < thread_count - 1;
	     thread_idx++) {
		threads.emplace_back(set_range, current_start_index,
				     load_per_thread);
		current_start_index += load_per_thread;
	}
	threads.emplace_back(set_range, current_start_index,
			     load_for_last_thread);
	for (auto &thread : threads) {
		thread.join();
	}
	std::cout << "100%" << std::endl;
	return values;
}
} // namespace

int main(int argc, char *argv[]) {
	Input_Parser input_parser;
	set_parser_up(input_parser);
	input_parser.parse(argc, argv);
	bool args_are_valid = arguments_are_valid();

	if (help_is_requested) {
		std::cout << input_parser.get_help() << std::endl;
	}

	if (!args_are_valid) {
		return 0;
	}

	spherical_intersection::Mesh mesh;

	std::cout << "Loading mesh... " << std::flush;
	auto object_information = [&] {
		switch (input_format) {
		case obj:
			return object_io::obj::load_obj(input_path);
		case ply:
			return object_io::ply::load_ply(input_path);
		default:
			throw std::invalid_argument("Unknown input format.");
		}
	}();
	std::cout << "Done!" << std::endl;

	std::cout << "Building mesh... " << std::flush;
	for (const auto &vertex_location : object_information.first) {
		mesh.add_vertex(spherical_intersection::math3d::Vector{
		    vertex_location[0], vertex_location[1],
		    vertex_location[2]});
	}
	for (const auto &triangle_indices : object_information.second) {
		const auto &vertices = mesh.get_vertices();
		mesh.add_triangle(vertices[triangle_indices[0]],
				  vertices[triangle_indices[1]],
				  vertices[triangle_indices[2]]);
	}
	std::cout << "Done!" << std::endl;

	std::cout << "Computing values... " << std::flush;
	timer::Timer::start("computation");
	auto values = compute_all_values(mesh, radius, algorithm, thread_count);
	timer::Timer::stop("computation");
	std::cout << "Done!" << std::endl;

	std::cout << "Computation speed: "
		  << values.size() / timer::Timer::get("computation")
		  << " vertices per second" << std::endl;

	std::cout << "Exporting values... " << std::flush;
	object_io::ply::save_ply(output_path, object_information, values,
				 object_io::ply::Format::binary_little_endian);
	std::cout << "Done!" << std::endl;

	return 0;
}
