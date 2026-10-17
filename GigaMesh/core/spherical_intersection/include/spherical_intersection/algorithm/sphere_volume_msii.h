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

#ifndef SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_VOLUME_MSII_H
#define SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_VOLUME_MSII_H

#include <cstddef>


namespace spherical_intersection {
namespace math3d {
class Sphere;
}

class Graph;

namespace algorithm {
//! @brief Computes a normalized area of the
//! intersection of the mesh's volume and the sphere that define the
//! intersection that is represented by the given graph if the graph contains at
//! least one cycle. The normalization factor is 1/(r^2) where r is the sphere's
//! radius. Deletes the graph's arcs in any case.
//! @param graph reference to the Graph representing the intersection.
//! @return The area if the graph contains at least one cycle.
//! std::numeric_limits<double>::is_NaN() otherwise.
double get_sphere_volume_area(Graph &graph);
} // namespace algorithm
} // namespace spherical_intersection

#endif
