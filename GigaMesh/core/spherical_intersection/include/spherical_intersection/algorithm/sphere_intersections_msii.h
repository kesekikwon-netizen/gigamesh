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

#ifndef SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_INTERSECTION_MSII_H
#define SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_INTERSECTION_MSII_H

#include <cstddef>
#include <vector>

namespace spherical_intersection {
class Graph;
namespace algorithm {
//! @brief Accesses the calculated intersections of the mesh's surface and the sphere
//! which are contained in the given precomputed graph.
//! @param graph reference to the graph representing the intersection.
//! @return matrix with vertex number, intersection number and intersection coordinates x/y/z.
std::vector<double> get_sphere_intersections(const Graph &graph);
} // namespace algorithm
} // namespace spherical_intersection

#endif
