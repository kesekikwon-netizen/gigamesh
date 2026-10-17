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

#ifndef SPHERICAL_INTERSECTION_MESH_H
#define SPHERICAL_INTERSECTION_MESH_H

#include <array>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <tuple>
#include <unordered_map> // TODO are all include everywhere needed?

#include "math3d/math3d.h" //TODO limit includes in header files everywhere

// TODO use reference wrappers to
// enforce non nullity? this woul
// also hide the fact that i want to
// compare ans hash memory adresses
// (pointers) in the sets and maps...

namespace spherical_intersection {
//! @brief A mesh that is prepared for the computation of graphs representing
//! its intersections with spheres.
class Mesh {
      public:
	class Vertex;
	class Edge;
	class Triangle;

	using Vertex_Container = std::deque<Vertex>;
	using Edge_Container = std::deque<Edge>;
	using Triangle_Container = std::deque<Triangle>;

	//! @brief A vertex.
	class Vertex {
	      public:
		//! @brief Information about adjacency.
		class Adjacency {
		      public:
			//! @brief Constructs information about adjacency from a
			//! given neighbor and the edge connecting this neighbor
			//! to the vertex.
			//!
			//! Using this constructor constructs adjacency
			//! information not associated with a vertex.
			//! @param edge the connecting edge.
			//! @param other_vertex the neighbor.
			Adjacency(const Edge &edge, const Vertex &other_vertex);

			//! @brief Copy constructor (deleted).
			//! @param other the other adjacency information.
			Adjacency(const Adjacency &other) = delete;

			//! @brief Move constructor.
			//! @param other the other adjacency information.
			Adjacency(Adjacency &&other) = default;

			//! @brief Copy assignment operator (deleted).
			//! @param other the other adjacency information.
			Adjacency &operator=(const Adjacency &other) = delete;

			//! @brief Move assignment operator.
			//! @param other the other adjacency information.
			Adjacency &operator=(Adjacency &&other) = default;

			//! @brief Gets the edge connecting the vertex to one of
			//! its neighbors described by this adjacency
			//! information.
			//! @return The connecting edge.
			const Edge &get_edge() const;

			//! @brief Gets the vertex's neighbor described by this
			//! adjacency information.
			//! @return The neighbor.
			const Vertex &get_other_vertex() const;

		      private:
			const Edge &edge;
			const Vertex &other_vertex;
		};
		using Adjacency_Container = std::vector<Adjacency>;

		//! @brief Constructs a Vertex at the given location with the
		//! given index.
		//!
		//! Using this constructor constructs a vertex not associated
		//! with a mesh.
		//! @param location the given location.
		//! @param index the given index.
		Vertex(const math3d::Vector location, const std::size_t index);

		//! @brief Copy constructor (deleted).
		//! @param other the other vertex.
		Vertex(const Vertex &other) = delete;

		//! @brief Move constructor.
		//! @param other the other vertex.
		Vertex(Vertex &&other) = default;

		//! @brief Copy assignment operator (deleted).
		//! @param other the other vertex.
		Vertex &operator=(const Vertex &other) = delete;

		//! @brief Move assignment operator.
		//! @param other the other vertex.
		Vertex &operator=(Vertex &&other) = default;

		//! @brief Gets this vertex's index.
		//! @return This vertex's index.
		std::size_t get_index() const;

		//! @brief Gets information about this vertex's adjacencies.
		//! @return A reference to a container containing information
		//! about each adjacent vertex / incident edge.
		const Adjacency_Container &get_adjacencies() const;

		//! @brief Gets this vertex's location.
		//! @return This vertex's location.
		const math3d::Vector &get_location() const;

	      private:
		math3d::Vector location;
		std::size_t index;
		mutable Adjacency_Container adjacencies;

		friend class Mesh;
	};

	//! @brief An edge whose vertices are vertices of the mesh.
	class Edge {
	      public:
		//! @brief Information about a triangle containing this edge.
		class Containing_Triangle {
		      public:
			//! @brief Constructs information about a triangle
			//! containing the edge.
			//!
			//! Using this constructor constructs information not
			//! associated with an edge.
			//! @param triangle the containing triangle.
			//! @param edge_index the edge's index in the zero-based
			//! list of the triangles edges.
			Containing_Triangle(const Triangle &triangle,
					    const unsigned int edge_index);

			//! @brief Copy constructor (deleted).
			//! @param other the other containing triangle.
			Containing_Triangle(const Containing_Triangle &other) =
			    delete;

			//! @brief Move constructor.
			//! @param other the other containing triangle.
			Containing_Triangle(Containing_Triangle &&other) =
			    default;

			//! @brief Copy assignment operator (deleted).
			//! @param other the other containing triangle.
			Containing_Triangle &
			operator=(const Containing_Triangle &other) = delete;

			//! @brief Move assignment operator.
			//! @param other the other containing triangle.
			Containing_Triangle &
			operator=(Containing_Triangle &&other) = default;

			//! @brief Gets the triangle containting the edge.
			//! @return A reference to the specified triangle.
			const Triangle &get_triangle() const;

			//! @brief Gets the edge's index in the zero-based list
			//! of the triangles edges.
			//! @return The specified index.
			unsigned int get_edge_index() const;

		      private:
			const Triangle &triangle;
			unsigned int edge_index;
		};
		using Containing_Triangle_Container =
		    std::vector<Containing_Triangle>;

		//! @brief Constructs an edge connecting two given vertices.
		//!
		//! Using this constructor constructs an edge not associated
		//! with a mesh.
		//! @param vertex_1 reference to the edge's first vertex.
		//! @param vertex_2 reference to the edge's second vertex.
		Edge(const Vertex &vertex_1, const Vertex &vertex_2);

		//! @brief Copy constructor (deleted).
		//! @param other the other edge.
		Edge(const Edge &other) = delete;

		//! @brief Move constructor.
		//! @param other the other edge.
		Edge(Edge &&other) = default;

		//! @brief Copy assignment operator (deleted).
		//! @param other the other edge.
		Edge &operator=(const Edge &other) = delete;

		//! @brief Move assignment operator.
		//! @param other the other edge.
		Edge &operator=(Edge &&other) = default;

		//! @brief Gets information about the triangles containing this
		//! edge.
		//! @return A reference to a container containing information
		//! about each triangle containing this edge.
		const Containing_Triangle_Container &
		get_containing_triangles() const;

		//! @brief Gets the edge's first or second vertex.
		//! @param index specifies which vertex to return.
		//! @return The first vertex if and only if the given index is 0
		//! and the second vertex if and only if the given index is 1.
		const Vertex &get_vertex(const unsigned int index) const;

	      private:
		const Vertex &vertex_1;
		const Vertex &vertex_2;
		mutable Containing_Triangle_Container containing_triangles;

		friend class Mesh;
	};

	//! @brief A triangle whose vertices are vertices of the mesh.
	class Triangle {
	      public:
		//! @brief Constructs a triangle with the given vertices and
		//! edges.
		//!
		//! Using this constructor constructs a triangle not associated
		//! with a mesh. It is expected that the edge e_1 connects the
		//! vertices v_1 and v_2, that the edge e_2 connects the
		//! vertices v_2 and v_3 and that the edge e_3 connects the
		//! vertices v_3 and v_1 where [e_1, e_2, e_3] is the given
		//! array of edges and [v_1, v_2, v_3] is the given array of
		//! vertices. In this situation v_i will be the triangles i-th
		//! vertex for i=1,2,3 and e_i will be the triangles i-th edge
		//! for i=1,2,3.
		//! @param vertices an array of the given vertices.
		//! @param edges an array of the given edges.
		Triangle(
		    const std::array<std::reference_wrapper<const Vertex>, 3>
			vertices,
		    const std::array<std::reference_wrapper<const Edge>, 3>
			edges);

		//! @brief Copy constructor (deleted).
		//! @param other the other triangle.
		Triangle(const Triangle &other) = delete;

		//! @brief Move constructor.
		//! @param other the other triangle.
		Triangle(Triangle &&other) = default;

		//! @brief Copy assignment operator (deleted).
		//! @param other the other triangle.
		Triangle &operator=(const Triangle &other) = delete;

		//! @brief Move assignment operator.
		//! @param other the other triangle.
		Triangle &operator=(Triangle &&other) = default;

		//! @brief Gets the triangles (i+1)-th vertex where i is a given
		//! index.
		//! @param index the given index.
		//! @return A reference to the given vertex.
		const Vertex &get_vertex(const unsigned int index) const;

		//! @brief Gets the triangles (i+1)-th edge where i is a given
		//! index.
		//! @param index the given index.
		//! @return A reference to the given edge.
		const Edge &get_edge(const unsigned int index) const;

	      private:
		std::array<std::reference_wrapper<const Vertex>, 3> vertices;
		std::array<std::reference_wrapper<const Edge>, 3> edges;
	};

	// construction

	//! @brief Constructs an empty mesh.
	Mesh();

	//! @brief Copy constructor (deleted).
	//! @param other the other mesh.
	Mesh(const Mesh &other) = delete;

	//! @brief Move constructor.
	//! @param other the other mesh.
	Mesh(Mesh &&other) = default;

	//! @brief Copy assignment operator (deleted).
	//! @param other the other mesh.
	Mesh &operator=(const Mesh &other) = delete;

	//! @brief Move assignment operator.
	//! @param other the other mesh.
	Mesh &operator=(Mesh &&other) = default;

	//! @brief Adds a vertex at the given location.
	//! @param location the given location.
	void add_vertex(const math3d::Vector location);

	//! @brief Adds a triangle with the given vertices as vertices.
	//! @param vertex_1 reference to the triangles first vertex.
	//! @param vertex_2 reference to the triangles second vertex.
	//! @param vertex_3 reference to the triangles third vertex.
	void add_triangle(const Vertex &vertex_1, const Vertex &vertex_2,
			  const Vertex &vertex_3);

	// retrieval

	//! @brief Gets the mesh's vertices.
	//! @return A reference to a container containing the mesh's vertices.
	const Vertex_Container &get_vertices() const;

	//! @brief Gets the mesh's triangles.
	//! @return A reference to a container containing the mesh's triangles.
	const Triangle_Container &get_triangles() const;

      private:
	Edge &to_edge(const Vertex &vertex_1, const Vertex &vertex_2);

	Vertex_Container vertices;
	Edge_Container edges;
	Triangle_Container triangles;

	using Edge_Key = std::pair<const Vertex *, const Vertex *>;
	std::unordered_map<
	    Edge_Key, std::reference_wrapper<Edge>,
	    std::function<std::size_t(const Edge_Key &)>,
	    std::function<bool(const Edge_Key &, const Edge_Key &)>>
	    vertex_ptrs_to_edge;
};

} // namespace spherical_intersection

#endif
