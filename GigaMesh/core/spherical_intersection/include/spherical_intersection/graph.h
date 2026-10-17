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

#ifndef SPHERICAL_INTERSECTION_GRAPH_H
#define SPHERICAL_INTERSECTION_GRAPH_H

#include <functional>
#include <memory>
#include <tuple>
#include <vector>

#include "mesh_spherical.h"

namespace spherical_intersection {
//! @brief Directed Multigraph embedded into a sphere that represents that
//! sphere's intersection with a mesh.
//!
//! It is expected, that all references to spheres given during the construction
//! of nodes and arcs refer to the same sphere and that all parts of meshes
//! specified during the construction of nodes and arcs belong to the same mesh.
class Graph {
      public:
	class Node;
	class Arc;

	using Node_Container = std::list<Node>;
	using Arc_Container = std::list<Arc>;

	//! @brief A node.
	class Node {
	      public:
		using Arc_Container =
		    std::list<std::reference_wrapper<const Arc>>;

		//! @brief Constructs a node from information about the
		//! intersection of a mesh with a sphere.
		//!
		//! Using this constructor constructs nodes not associated with
		//! a graph.
		//! @param sphere a reference to the intersecting sphere.
		//! @param edge a reference to the edge of the mesh containing
		//! the node.
		//! @param enters_on_first specification whether the given edge
		//! enters the intersecting sphere on its first intersection
		//! with the sphere when directing the edge from its first
		//! vertex to its second
		//! @param position position of the node as a point on the given
		//! edge when ordering the nodes on the edge in ascending order
		//! by their distance to the edge's first vertex
		//! and assigning the positions 0 to m in that order where m is
		//! the number of nodes on the edge.
		Node(const math3d::Sphere &sphere, const Mesh::Edge &edge,
		     const bool enters_on_first, const std::size_t position);

		//! @brief Copy constructor (deleted).
		//! @param other the other node.
		Node(const Node &other) = delete;

		//! @brief Move constructor.
		//! @param other the other node.
		Node(Node &&other) = default;

		//! @brief Copy assignment operator (deleted).
		//! @param other the other node.
		Node &operator=(const Node &other) = delete;

		//! @brief Move assignment operator.
		//! @param other the other node.
		Node &operator=(Node &&other) = delete;

		//! @brief Gets the sphere whose intersection with a mesh is
		//! represented by the graph.
		//! @return A reference to the sphere.
		const math3d::Sphere &get_sphere() const;

		//! @brief Gets the edge containing the node.
		//! @return A reference to the edge.
		const Mesh::Edge &get_edge() const;

		//! @brief Decides whether the given edge
		//! enters the intersecting sphere on its first intersection
		//! with the sphere when directing the edge from its first
		//! vertex to its second
		//! @return True if and only if the edge enters the sphere on
		//! its first intersection as described.
		bool get_enters_on_first() const;

		//! @brief Gets the node's assigned position as a point on the
		//! edge that contains it when ordering the nodes on this edge
		//! in ascending order by their distance to the edge's first
		//! vertex and assigning the positions 0 to m in that order
		//! where m is the number of nodes on the edge.
		//! @return The position.
		std::size_t get_position() const;

		//! @brief Gets the graph's arcs whose end node is this node.
		//! @return A reference to a container containing these arcs.
		const Arc_Container &get_incoming_arcs() const;

		//! @brief Gets the graph's arcs whose start node is this node.
		//! @return A reference to a container containing these arcs.
		const Arc_Container &get_outgoing_arcs() const;

	      private:
		const math3d::Sphere &sphere;
		const Mesh::Edge &edge;
		const bool enters_on_first;
		const std::size_t position;

		Graph::Node_Container::const_iterator in_graph;
		mutable Arc_Container incoming_arcs;
		mutable Arc_Container outgoing_arcs;

		friend class Graph;
	};

	//! @brief An arc.
	class Arc {
	      public:
		//! @brief Constructs an arc from information about the
		//! intersection of a mesh with a sphere.
		//!
		//! Using this constructor constructs arcs not associated with
		//! a graph.
		//! @param sphere reference to the intersecting sphere.
		//! @param triangle reference to the triangle of the mesh
		//! containing the arc.
		//! @param start reference to the arc's start node.
		//! @param end reference to the arc's end node.
		Arc(const math3d::Sphere &sphere,
		    const Mesh::Triangle &triangle, const Node &start,
		    const Node &end);

		//! @brief Copy constructor (deleted).
		//! @param other the other arc.
		Arc(const Arc &other) = delete;

		//! @brief Move constructor.
		//! @param other the other arc.
		Arc(Arc &&other) = default;

		//! @brief Copy assignment operator (deleted).
		//! @param other the other arc.
		Arc &operator=(const Arc &other) = delete;

		//! @brief Move assignment operator.
		//! @param other the other arc.
		Arc &operator=(Arc &&other) = delete;

		//! @brief Gets the sphere whose intersection with a mesh is
		//! represented by the graph.
		//! @return A reference to the sphere.
		const math3d::Sphere &get_sphere() const;

		//! @brief Gets the triangle containing the arc.
		//! @return A reference to the triangle.
		const Mesh::Triangle &get_triangle() const;

		//! @brief Gets the arc's start node.
		//! @return A reference to the arc's start node.
		const Node &get_start() const;

		//! @brief Gets the arc's end node.
		//! @return A reference to the arc's end node.
		const Node &get_end() const;

	      private:
		const math3d::Sphere &sphere;
		const Mesh::Triangle &triangle;

		const Node &start;
		const Node &end;
		Graph::Arc_Container::const_iterator in_graph;
		Graph::Node::Arc_Container::const_iterator in_start;
		Graph::Node::Arc_Container::const_iterator in_end;

		friend class Graph;
	};

	//! @brief Constructs the graph representing the intersection of the
	//! mesh containing a given vertex and a given sphere.
	//!
	//! It is expected that the given vertex is inside the ball enclosed by
	//! the given sphere. The graph's nodes are the intersections of the
	//! mesh's edges and its arcs are the intersections of the mesh's
	//! triangle's with the sphere directed such that a triangles normal
	//! points from left to right when following an arc in its direction
	//! with the outward oriented sphere's current surface normal pointing up.
	//! @param vertex_seed a reference to the given vertex.
	//! @param sphere a reference to the given sphere.
	Graph(const Mesh::Vertex &vertex_seed, const math3d::Sphere &sphere);

	//! @brief Copy constructor (deleted).
	//! @param other the other graph.
	Graph(const Graph &other) = delete;

	//! @brief Move constructor.
	//! @param other the other graph.
	Graph(Graph &&other) = default;

	//! @brief Copy assignment operator (deleted).
	//! @param other the other graph.
	Graph &operator=(const Graph &other) = delete;

	//! @brief Move assignment operator.
	//! @param other the other graph.
	Graph &operator=(Graph &&other) = default;

	//! @brief Constructs a node from information about the
	//! intersection of a mesh with a sphere and associates it with this
	//! graph.
	//! @param sphere reference to the intersecting sphere.
	//! @param edge reference to the edge of the mesh containing
	//! the node.
	//! @param enters_on_first specification whether the given edge
	//! enters the intersecting sphere on its first intersection
	//! with the sphere when directing the edge from its first
	//! vertex to its second
	//! @param position position of the node as a point on the given
	//! edge when ordering the nodes on the edge in ascending order
	//! by their distance to the edge's first vertex
	//! and assigning the positions 0 to m in that order where m is
	//! the number of nodes on the edge.
	Node &emplace_node(const math3d::Sphere &sphere, const Mesh::Edge &edge,
			   const bool enters_on_first,
			   const std::size_t position);

	//! @brief Constructs an arc from information about the
	//! intersection of a mesh with a sphere and associates it with this
	//! graph.
	//! @param sphere reference to the intersecting sphere.
	//! @param triangle reference to the triangle of the mesh
	//! containing the arc.
	//! @param start reference to the arc's start node.
	//! @param end reference to the arc's end node.
	Arc &emplace_arc(const math3d::Sphere &sphere,
			 const Mesh::Triangle &triangle, const Node &start,
			 const Node &end);

	//! @brief Removes a given node from the graph.
	//!
	//! This invalidates the given node.
	//! @param a reference to the given node.
	void erase_node(const Node &node);

	//! @brief Removes a given arc from the graph.
	//!
	//! This invalidates the given arc.
	//! @param a reference to the given arc.
	void erase_arc(const Arc &arc);

	//! @brief Gets the graph's nodes.
	//! @return a reference to a container containing the graph's nodes.
	const Node_Container &get_nodes() const;

	//! @brief Gets the graph's arcs.
	//! @return a reference to a container containing the graph's arcs.
	const Arc_Container &get_arcs() const;

      private:
	Node_Container nodes;
	Arc_Container arcs;
};
} // namespace spherical_intersection

#endif
