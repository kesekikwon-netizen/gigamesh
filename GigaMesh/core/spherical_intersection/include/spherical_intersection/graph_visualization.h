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

#ifndef SPHERICAL_INTERSECTION_GRAPH_VISUALIZATION_H
#define SPHERICAL_INTERSECTION_GRAPH_VISUALIZATION_H

#include <string>
#include <tuple>
#include <vector>

// TODO nested classes cant be forward delcared... fix this with a workaround of
// some sort?

namespace spherical_intersection {
class Graph;

namespace graph_visualization {
//! @brief Gets a string representation of a given graph.
//!
//! To construct the representation, a number from 0 to n-1 is uniquely assigned
//! to each of the graph's nodes where n is the number of nodes. The
//! representation's first line announces the representation and contains a
//! string representation of the pointer to the given graph. Each following line
//! represents one of the adjacency lists of the given graph and each adjacency
//! list is represented. Each adjacency list's representation starts with the
//! number that was assigned to the node whose adjacency list is represented.
//! This number is followed by the pair of vertex indices specifying the edge of
//! the mesh M whose intersection with a sphere is represented by the graph and
//! the position of the node as a point on that edge when ordering the nodes on
//! the edge in ascending order by their distance to the vertex whose index is
//! listed first and assigning the positions 0 to m in that order where m is the
//! number of nodes on the edge. After that, a comma separated list follows,
//! where each arc emanating from the node whose adjacency list is represented
//! is represented. Each arc representing list element consists of the vertex
//! indices of the triangle of M that contains the arc and the number
//! assigned to the arc's end node.
//! @param graph a reference to the graph.
//! @return The string representation
std::string to_string(const Graph &graph);

using Vertex_Location = std::array<double, 3>;
using Triangle_Indices = std::array<std::size_t, 3>;
using Raw_Mesh =
    std::pair<std::vector<Vertex_Location>, std::vector<Triangle_Indices>>;

//! @brief Gets a mesh representation of a given graph.
//!
//! The mesh representation represents the graphs arcs where each arc is
//! represented as a pyramid bent along the arc with its base at the arc's start
//! node's location and its top at the arc's end node's location.
//! @param graph a reference to the graph.
//! @param arc_resolution the resolution of the arc representations.
//! @return The mesh representation
Raw_Mesh to_raw_mesh(const Graph &graph, const std::size_t arc_resolution);

} // namespace graph_visualization
} // namespace spherical_intersection

#endif