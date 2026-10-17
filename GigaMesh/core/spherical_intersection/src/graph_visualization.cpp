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
#include <math.h>
#include <sstream>
#include <vector>

#include "graph.h"
#include "graph_geometry.h"
#include "graph_visualization.h"

using namespace spherical_intersection;

std::string graph_visualization::to_string(const Graph &graph) {
	std::unordered_map<const void *, std::size_t> ids;
	auto ptr_to_id = [&ids](const void *ptr) {
		std::unordered_map<const void *, std::size_t>::iterator it;
		std::tie(it, std::ignore) = ids.insert({ptr, ids.size()});
		return it->second;
	};
	auto node_to_str = [&ptr_to_id](const Graph::Node &node) {
		std::stringstream ss;
		ss << ptr_to_id(static_cast<const void *>(&node));
		const auto &v_1 = node.get_edge().get_vertex(0);
		const auto &v_2 = node.get_edge().get_vertex(1);
		ss << " (" << v_1.get_index() << "-" << v_2.get_index() << ", "
		   << node.get_position() << ")";
		return ss.str();
	};
	auto arc_to_str = [](const Graph::Arc &arc) {
		std::stringstream ss;
		ss << "[" << arc.get_triangle().get_vertex(0).get_index()
		   << ", " << arc.get_triangle().get_vertex(1).get_index()
		   << ", " << arc.get_triangle().get_vertex(2).get_index()
		   << "]";
		return ss.str();
	};
	auto successor_to_str = [&ptr_to_id](const Graph::Node &node) {
		std::stringstream ss;
		ss << ptr_to_id(static_cast<const void *>(&node));
		return ss.str();
	};
	auto to_succession = [&arc_to_str,
			      &successor_to_str](const Graph::Arc &arc) {
		std::stringstream ss;
		std::string arc_as_string = arc_to_str(arc);
		if (!arc_as_string.empty()) {
			ss << arc_as_string << " ";
		}
		ss << successor_to_str(arc.get_end());
		return ss.str();
	};
	auto to_adjacency_list = [&node_to_str,
				  &to_succession](const Graph::Node &node) {
		std::stringstream ss;
		ss << node_to_str(node) << ":";
		const typename Graph::Node::Arc_Container &outgoing_arcs =
		    node.get_outgoing_arcs();
		auto arc_it = outgoing_arcs.begin();
		if (arc_it != outgoing_arcs.end()) {
			ss << " " << to_succession(*arc_it);
			++arc_it;
		}
		for (; arc_it != outgoing_arcs.end(); ++arc_it) {
			ss << ", " << to_succession(*arc_it);
		}
		return ss.str();
	};
	std::stringstream ss;
	ss << "directed multigraph " << &graph << " as successor lists:\n";
	const typename Graph::Node_Container &nodes = graph.get_nodes();
	auto node_it = nodes.begin();
	if (node_it != nodes.end()) {
		ss << to_adjacency_list(*node_it);
		++node_it;
	}
	for (; node_it != nodes.end(); ++node_it) {
		ss << "\n" << to_adjacency_list(*node_it);
	}
	return ss.str();
}

graph_visualization::Raw_Mesh
graph_visualization::to_raw_mesh(const Graph &graph,
				 const std::size_t arc_resolution) {
	const Graph::Arc_Container &arcs = graph.get_arcs();

	Raw_Mesh raw_mesh;
	auto &vertex_locations = raw_mesh.first;
	auto &triangle_indices = raw_mesh.second;

	auto append_vertex_location =
	    [&vertex_locations](const math3d::Vector &location) {
		    vertex_locations.push_back(
			{location.get(0), location.get(1), location.get(2)});
	    };
	auto append_triangle_indices =
	    [&triangle_indices](std::size_t index_1, std::size_t index_2,
				std::size_t index_3) {
		    triangle_indices.push_back({index_1, index_2, index_3});
	    };

	for (const auto &arc : arcs) {
		Arc_Curve curve(arc);
		auto generate_verts = [&append_vertex_location, &curve](
					  double pos, double offset_scale) {
			auto center = curve.eval(pos);
			auto h_offset = offset_scale * curve.get_normal_at(pos);
			auto v_offset = math3d::cross_product(
			    curve.get_tangent_at(pos), h_offset);

			append_vertex_location(center + h_offset);
			append_vertex_location(center + v_offset);
			append_vertex_location(center - h_offset);
			append_vertex_location(center - v_offset);
		};

		auto arc_scale = 0.1 * arc.get_sphere().get_radius();
		generate_verts(0, arc_scale);

		for (unsigned int i = 1; i <= arc_resolution; i++) {
			generate_verts(i / static_cast<double>(arc_resolution),
				       arc_scale - arc_scale / arc_resolution * i);
			for (std::size_t edge_idx = 0; edge_idx < 4;
			     edge_idx++) {
				append_triangle_indices(
				    vertex_locations.size() - 8 + edge_idx,
				    vertex_locations.size() - 8 +
					((edge_idx + 1) % 4),
				    vertex_locations.size() - 4 + edge_idx);
				append_triangle_indices(
				    vertex_locations.size() - 8 +
					((edge_idx + 1) % 4),
				    vertex_locations.size() - 4 +
					((edge_idx + 1) % 4),
				    vertex_locations.size() - 4 + edge_idx);
			}
		}
	}

	return raw_mesh;
}