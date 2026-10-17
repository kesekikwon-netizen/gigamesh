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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>
#include <unordered_set>

#include "graph.h"
#include "mesh_spherical.h"
#include "utility/hash_combine.h"

using namespace spherical_intersection;

// Graph::Node
Graph::Node::Node(const math3d::Sphere &sphere, const Mesh::Edge &edge,
		  const bool enters_on_first, const std::size_t position)
    : sphere(sphere), edge(edge), enters_on_first(enters_on_first),
      position(position) {}

const math3d::Sphere &Graph::Node::get_sphere() const { return this->sphere; }

const Mesh::Edge &Graph::Node::get_edge() const { return this->edge; }

bool Graph::Node::get_enters_on_first() const { return this->enters_on_first; }

std::size_t Graph::Node::get_position() const { return this->position; }

const Graph::Node::Arc_Container &Graph::Node::get_incoming_arcs() const {
	return this->incoming_arcs;
}

const Graph::Node::Arc_Container &Graph::Node::get_outgoing_arcs() const {
	return this->outgoing_arcs;
}

// Graph::Arc
Graph::Arc::Arc(const math3d::Sphere &sphere, const Mesh::Triangle &triangle,
		const Node &start, const Node &end)
    : sphere(sphere), triangle(triangle), start(start), end(end) {}

const math3d::Sphere &Graph::Arc::get_sphere() const { return this->sphere; }

const Mesh::Triangle &Graph::Arc::get_triangle() const {
	return this->triangle;
}

const Graph::Node &Graph::Arc::get_start() const { return this->start; }

const Graph::Node &Graph::Arc::get_end() const { return this->end; }

//! @cond DEV

// Graph
// helper structures for Graph's constructor
namespace {
// declarations

//! @brief A utility class to map pairs of vertices to nodes of a graph
//! representing the elements of the intersection of the edge connecting them
//! with a sphere.
class Intersection_Mapper {
      public:
	using Key = std::pair<const Mesh::Vertex *, const Mesh::Vertex *>;

	//! @brief Constructs a utility object to map pairs of vertices of a
	//! mesh to nodes of a given graph that represent the elements of the
	//! intersection of the mesh's edge connecting the vertices with a
	//! sphere where the vertices expected to be part of a component mesh of
	//! the mesh whose intersection with the sphere is represented by the
	//! graph. All required graph nodes are constructed in the given graph.
	//! @param graph the representing graph.
	//! @param vertex_seed a vertex of the component mesh.
	//! @param sphere the sphere.
	Intersection_Mapper(Graph &graph, const Mesh::Vertex &vertex_seed,
			    const math3d::Sphere &sphere);

	//! @brief Applies the map to a given element.
	//! @param key the given element.
	//! @return a pointer to the given elements image under the map if the
	//! map is defined at the given element and nullptr otherwise.
	const std::vector<std::reference_wrapper<const Graph::Node>> *
	get(const Key &key) const;

      private:
	Graph &graph;
	const math3d::Sphere &sphere;
	std::unordered_map<
	    Key, std::vector<std::reference_wrapper<const Graph::Node>>,
	    std::function<std::size_t(const Key &)>,
	    std::function<bool(const Key &, const Key &)>>
	    intersection_map;

	//! @brief Defines the map at vertex pairs given by the given edge while
	//! the images of these pairs are given by information about the given
	//! edges intersection with the sphere that was given during the map
	//! object's construction. This is a conversion of information
	//! equivalent to a restriction of the map to the corresponding partial
	//! definition of the map.
	//! @param edge a reference to the given edge
	//! @param enters_on_first Expected to be the specification whether the
	//! given edge enters the intersecting sphere on its first intersection
	//! with the sphere when directing the edge from its first vertex to its
	//! second.
	//! @param intersection_count Expected to be the number of elements in
	//! the given edge's intersection with the sphere.
	void create_intersection(const Mesh::Edge &edge, bool enters_on_first,
				 std::size_t intersection_count);
};

//! @brief Decides whether a given vector is inside the ball enclosed by a given
//! sphere.
//! @param v a reference to the given vector.
//! @param sphere a reference to the given sphere.
//! @return True if and only if the given vector is inside the ball enclosed by
//! the given sphere.
bool is_in_ball(const math3d::Vector &v, const math3d::Sphere &sphere);

// implementations
Intersection_Mapper::Intersection_Mapper(Graph &graph,
					 const Mesh::Vertex &vertex_seed,
					 const math3d::Sphere &sphere)
    : graph(graph),
      sphere(sphere), intersection_map{
			  1,
			  [](const Key &node_ptrs) {
				  std::size_t seed = 0;
				  utility::hash_combine(seed, node_ptrs.first);
				  utility::hash_combine(seed, node_ptrs.second);
				  return seed;
			  },
			  [](const Key &node_ptrs_1, const Key &node_ptrs_2) {
				  return (std::make_pair(node_ptrs_1.first,
							 node_ptrs_1.second) ==
					  std::make_pair(node_ptrs_2.first,
							 node_ptrs_2.second));
			  }} {
	std::unordered_set<const Mesh::Vertex *> known_vertex_ptrs = {
	    &vertex_seed};
	std::unordered_set<const Mesh::Edge *> known_edge_ptrs;
	std::vector<std::reference_wrapper<const Mesh::Vertex>>
	    active_vertices = {vertex_seed};
	std::vector<std::reference_wrapper<const Mesh::Edge>> active_edges;

	auto notice_vertex = [&known_vertex_ptrs,
			      &active_vertices](const Mesh::Vertex &vertex) {
		bool vertex_is_new;
		std::tie(std::ignore, vertex_is_new) =
		    known_vertex_ptrs.insert(&vertex);
		if (vertex_is_new) {
			active_vertices.push_back(vertex);
		}
	};

	auto notice_edge = [&known_edge_ptrs,
			    &active_edges](const Mesh::Edge &edge) {
		bool edge_is_new;
		std::tie(std::ignore, edge_is_new) =
		    known_edge_ptrs.insert(&edge);
		if (edge_is_new) {
			active_edges.push_back(edge);
		}
	};

	auto notice_containing_triangle_edges =
	    [&notice_edge](const Mesh::Edge &edge) {
		    for (auto &containing_triangle :
			 edge.get_containing_triangles()) {
			    auto index = containing_triangle.get_edge_index();
			    auto &triangle = containing_triangle.get_triangle();
			    notice_edge(triangle.get_edge((index + 1) % 3));
			    notice_edge(triangle.get_edge((index + 2) % 3));
		    }
	    };

	auto process_adjacency = [&sphere, &known_edge_ptrs,
	              &notice_vertex,
				  &notice_containing_triangle_edges, this](
				     const Mesh::Vertex::Adjacency &adjacency) {
		auto &edge = adjacency.get_edge();
		bool edge_is_new;
		std::tie(std::ignore, edge_is_new) =
		    known_edge_ptrs.insert(&edge);
		if (edge_is_new) {
			auto &other_vertex = adjacency.get_other_vertex();
			if (is_in_ball(other_vertex.get_location(), sphere)) {
				notice_vertex(other_vertex);
			} else {
				this->create_intersection(
				    edge, &edge.get_vertex(0) == &other_vertex,
				    1);
				notice_containing_triangle_edges(edge);
			}
		}
	};

	auto process_edge = [&sphere, &notice_vertex,
			     &notice_containing_triangle_edges,
			     this](const Mesh::Edge &edge) {
		auto &vertex_1 = edge.get_vertex(0);
		auto &vertex_2 = edge.get_vertex(1);

		auto is_in_ball_1 = is_in_ball(vertex_1.get_location(), sphere);
		auto is_in_ball_2 = is_in_ball(vertex_2.get_location(), sphere);

		if (is_in_ball_1) {
			notice_vertex(vertex_1);
		}
		if (is_in_ball_2) {
			notice_vertex(vertex_2);
		}
		if (is_in_ball_1 != is_in_ball_2) {
			this->create_intersection(edge, !is_in_ball_1, 1);
			notice_containing_triangle_edges(edge);
		}
		if (!is_in_ball_1 && !is_in_ball_2) {
			auto &vector_1 = vertex_1.get_location();
			auto &vector_2 = vertex_2.get_location();

			double dot_p =
			    math3d::dot_product(vector_1 - sphere.get_center(),
						vector_2 - sphere.get_center());

			double vector_1_norm2 =
			    math3d::norm2(vector_1 - sphere.get_center());
			double vector_2_norm2 =
			    math3d::norm2(vector_2 - sphere.get_center());

			if (dot_p < vector_1_norm2 && dot_p < vector_2_norm2 &&
			    (vector_1_norm2 -
			     std::pow(sphere.get_radius(), 2)) *
				    (vector_2_norm2 -
				     std::pow(sphere.get_radius(), 2)) <
				std::pow(dot_p -
					     std::pow(sphere.get_radius(), 2),
					 2)) {
				this->create_intersection(edge, true, 2);
				notice_containing_triangle_edges(edge);
			}
		}
	};

	while (!active_vertices.empty() || !active_edges.empty()) {
		while (!active_vertices.empty()) {
			const Mesh::Vertex &current_vertex =
			    active_vertices.back();
			active_vertices.pop_back();
			for (const auto &adjacency :
			     current_vertex.get_adjacencies()) {
				process_adjacency(adjacency);
			}
		}
		while (!active_edges.empty()) {
			const Mesh::Edge &current_edge = active_edges.back();
			active_edges.pop_back();
			process_edge(current_edge);
		}
	}
}

const std::vector<std::reference_wrapper<const Graph::Node>> *
Intersection_Mapper::get(const Key &key) const {
	auto it = this->intersection_map.find(key);
	if (it == this->intersection_map.end()) {
		return nullptr;
	} else {
		return &(it->second);
	}
}

void Intersection_Mapper::create_intersection(const Mesh::Edge &edge,
					      bool enters_on_first,
					      std::size_t intersection_count) {

	std::vector<std::reference_wrapper<const Graph::Node>> nodes;
	for (std::size_t position = 0; position < intersection_count;
	     position++) {
		const Graph::Node &node = this->graph.emplace_node(
		    sphere, edge, enters_on_first, position);
		nodes.push_back(node);
	}
	std::vector<std::reference_wrapper<const Graph::Node>> nodes_in_reverse;
	std::reverse_copy(nodes.begin(), nodes.end(),
			  std::back_inserter(nodes_in_reverse));

	intersection_map[{&edge.get_vertex(0), &edge.get_vertex(1)}] = nodes;
	intersection_map[{&edge.get_vertex(1), &edge.get_vertex(0)}] =
	    nodes_in_reverse;
}

bool is_in_ball(const math3d::Vector &v, const math3d::Sphere &sphere) {
	return math3d::norm2(v - sphere.get_center()) <
	       std::pow(sphere.get_radius(), 2);
}

} // namespace

//! @endcond

Graph::Graph(const Mesh::Vertex &vertex_seed, const math3d::Sphere &sphere) {
	Intersection_Mapper intersection_mapper{*this, vertex_seed, sphere};

	auto get_successor =
	    [&intersection_mapper](const Mesh::Triangle &triangle,
				   std::size_t edge_index) {
		    std::size_t index_offset = 3;
		    while (index_offset > 0) {
			    index_offset--;
			    const auto &vertex_1 = triangle.get_vertex(
				(edge_index + index_offset) % 3);
			    const auto &vertex_2 = triangle.get_vertex(
				(edge_index + index_offset + 1) % 3);
			    const auto *nodes_ptr =
				intersection_mapper.get({&vertex_1, &vertex_2});

			    if (nodes_ptr && !nodes_ptr->empty()) {
				    return nodes_ptr->back();
			    }
		    }
		    throw std::logic_error("Error: Impossible triangle "
					   "intersection.");
	    };

	for (auto &node : this->nodes) {
		const Mesh::Edge &edge = node.get_edge();
		for (const auto &containing_triangle :
		     edge.get_containing_triangles()) {
			const Mesh::Triangle &triangle =
			    containing_triangle.get_triangle();
			unsigned int triangle_edge_idx =
			    containing_triangle.get_edge_index();

			bool enters_on_node =
			    ((node.enters_on_first !=
			      (node.position % 2 == 1)) !=
			     (&triangle.get_vertex(triangle_edge_idx) !=
			      &edge.get_vertex(0)));

			if (enters_on_node) {
				const Graph::Node &successor =
				    get_successor(triangle, triangle_edge_idx);

				this->emplace_arc(sphere, triangle, node,
						  successor);
			}
		}
	}
}

Graph::Node &Graph::emplace_node(const math3d::Sphere &sphere,
				 const Mesh::Edge &edge,
				 const bool enters_on_first,
				 const std::size_t position) {
	this->nodes.emplace_back(sphere, edge, enters_on_first, position);
	auto &node = this->nodes.back();
	node.in_graph = --this->nodes.end();
	return node;
}

Graph::Arc &Graph::emplace_arc(const math3d::Sphere &sphere,
			       const Mesh::Triangle &triangle,
			       const Node &start, const Node &end) {
	this->arcs.emplace_back(sphere, triangle, start, end);
	auto &arc = this->arcs.back();
	start.outgoing_arcs.push_back(arc);
	end.incoming_arcs.push_back(arc);
	arc.in_graph = --this->arcs.end();
	arc.in_start = --start.outgoing_arcs.end();
	arc.in_end = --end.incoming_arcs.end();

	return arc;
}

void Graph::erase_node(const Node &node) {
	while (!node.incoming_arcs.empty()) {
		this->erase_arc(node.incoming_arcs.front());
	}
	while (!node.outgoing_arcs.empty()) {
		this->erase_arc(node.outgoing_arcs.front());
	}
	this->nodes.erase(node.in_graph);
}

void Graph::erase_arc(const Arc &arc) {
	arc.start.outgoing_arcs.erase(arc.in_start);
	arc.end.incoming_arcs.erase(arc.in_end);
	this->arcs.erase(arc.in_graph);
}

const Graph::Node_Container &Graph::get_nodes() const { return this->nodes; }

const Graph::Arc_Container &Graph::get_arcs() const { return this->arcs; }
