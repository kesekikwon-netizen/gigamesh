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
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "mesh_spherical.h"
#include "utility/hash_combine.h" 

using namespace spherical_intersection;

// Mesh::Vertex::Adjacency
Mesh::Vertex::Adjacency::Adjacency(const Edge &edge, const Vertex &other_vertex)
    : edge(edge), other_vertex(other_vertex) {}

const Mesh::Edge &Mesh::Vertex::Adjacency::get_edge() const { return edge; }

const Mesh::Vertex &Mesh::Vertex::Adjacency::get_other_vertex() const {
	return other_vertex;
}

// Mesh::Vertex
Mesh::Vertex::Vertex(const math3d::Vector location, const std::size_t index)
    : location(location), index(index) {}

std::size_t Mesh::Vertex::get_index() const { return this->index; }

const Mesh::Vertex::Adjacency_Container &Mesh::Vertex::get_adjacencies() const {
	return this->adjacencies;
}

const math3d::Vector &Mesh::Vertex::get_location() const {
	return this->location;
}

// Mesh::Edge::Containing_Triangle
Mesh::Edge::Containing_Triangle::Containing_Triangle(
    const Triangle &triangle, const unsigned int edge_index)
    : triangle(triangle), edge_index(edge_index) {}

const Mesh::Triangle &Mesh::Edge::Containing_Triangle::get_triangle() const {
	return this->triangle;
}

unsigned int Mesh::Edge::Containing_Triangle::get_edge_index() const {
	return this->edge_index;
}

// Mesh::Edge
Mesh::Edge::Edge(const Vertex &vertex_1, const Vertex &vertex_2)
    : vertex_1(vertex_1), vertex_2(vertex_2) {}

const Mesh::Edge::Containing_Triangle_Container &
Mesh::Edge::get_containing_triangles() const {
	return this->containing_triangles;
}

const Mesh::Vertex &Mesh::Edge::get_vertex(const unsigned int index) const {
	switch (index) {
	case 0:
		return this->vertex_1;
	case 1:
		return this->vertex_2;
	default:
		throw std::logic_error("Index out of bounds.");
	}
}

// Mesh::Triangle
Mesh::Triangle::Triangle(
    const std::array<std::reference_wrapper<const Vertex>, 3> vertices,
    const std::array<std::reference_wrapper<const Edge>, 3> edges)
    : vertices(vertices), edges(edges) {}

const Mesh::Vertex &Mesh::Triangle::get_vertex(const unsigned int index) const {
	return this->vertices[index];
}

const Mesh::Edge &Mesh::Triangle::get_edge(const unsigned int index) const {
	return this->edges[index];
}

// Mesh
Mesh::Mesh()
    : vertex_ptrs_to_edge{
	  1,
	  [](const Edge_Key &node_ptrs) {
		  auto ordered_node_ptrs =
		      std::minmax(node_ptrs.first, node_ptrs.second);
		  std::size_t seed = 0;
		  utility::hash_combine(seed, ordered_node_ptrs.first);
		  utility::hash_combine(seed, ordered_node_ptrs.second);
		  return seed;
	  },
	  [](const Edge_Key &node_ptrs_1, const Edge_Key &node_ptrs_2) {
		  return (std::minmax(node_ptrs_1.first, node_ptrs_1.second) ==
			  std::minmax(node_ptrs_2.first, node_ptrs_2.second));
	  }} {}

void Mesh::add_vertex(const math3d::Vector location) {
	this->vertices.emplace_back(location, this->vertices.size());
}

void Mesh::add_triangle(const Vertex &vertex_1, const Vertex &vertex_2,
			const Vertex &vertex_3) {
	auto &edge_1 = this->to_edge(vertex_1, vertex_2);
	auto &edge_2 = this->to_edge(vertex_2, vertex_3);
	auto &edge_3 = this->to_edge(vertex_3, vertex_1);

	this->triangles.emplace_back(
	    std::array<std::reference_wrapper<const Vertex>, 3>{
		vertex_1, vertex_2, vertex_3},
	    std::array<std::reference_wrapper<const Edge>, 3>{edge_1, edge_2,
							      edge_3});

	const auto &triangle = triangles.back();

	edge_1.containing_triangles.emplace_back(triangle, 0);
	edge_2.containing_triangles.emplace_back(triangle, 1);
	edge_3.containing_triangles.emplace_back(triangle, 2);
}

const Mesh::Vertex_Container &Mesh::get_vertices() const {
	return this->vertices;
}

const Mesh::Triangle_Container &Mesh::get_triangles() const {
	return this->triangles;
}

Mesh::Edge &Mesh::to_edge(const Vertex &vertex_1, const Vertex &vertex_2) {
	auto it = this->vertex_ptrs_to_edge.find({&vertex_1, &vertex_2});
	if (it == this->vertex_ptrs_to_edge.end()) {
		this->edges.emplace_back(vertex_1, vertex_2);
		auto &edge = this->edges.back();
		vertex_1.adjacencies.emplace_back(edge, vertex_2);
		vertex_2.adjacencies.emplace_back(edge, vertex_1);
		this->vertex_ptrs_to_edge.insert(
		    {{&vertex_1, &vertex_2}, edge});
		return edge;
	} else {
		return it->second;
	}
}
