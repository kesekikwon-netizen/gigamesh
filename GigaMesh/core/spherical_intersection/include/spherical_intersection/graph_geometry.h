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

#ifndef SPHERICAL_INTERSECTION_GRAPH_GEOMETRY_H
#define SPHERICAL_INTERSECTION_GRAPH_GEOMETRY_H

#include "graph.h"
#include "math3d/math3d.h"

namespace spherical_intersection {
//! @brief Representation of an arc of a graph as a parametrized curve. The
//! parametrization is chosen such that its value at zero is the arc's start
//! node's location.
class Arc_Curve {
      public:
	//! @brief Constructs the curve representation of the given arc.
	//! Estimations are used when the distance from the arc's start node's
	//! location to its end node's location is less than the given
	//! tolerance.
	//! @param arc the arc.
	//! @param tolerance the tolerance.
	Arc_Curve(const Graph::Arc &arc, const double tolerance = 1E-10);

	//! @brief Computes the value c(x/l) where c is the representing curve's
	//! parametrization by arc length, l its arc length and x a given value.
	//! @param value the given value.
	//! @return The computed location.
	math3d::Vector eval(double value) const;

	//! @brief Computes the representing curve's unit tangent vector at
	//! c(x/l) where c is the representing curve's parametrization by arc
	//! length, l its arc length and x a given value.
	//! @param value the given value.
	//! @return The computed unit tangent vector.
	math3d::Vector get_tangent_at(double value) const;

	//! @brief Computes the cross product of the representing curve's unit
	//! tangent vector at c(x/l) and the normal of the mesh triangle
	//! containing the arc, where c is the representing curve's
	//! parametrization by arc length, l its arc length and x a given value.
	//! @param value the given value.
	//! @return The computed cross product.
	math3d::Vector get_normal_at(double value) const;

	//! @brief Gets the representing curve's (constant) geodesic curvature
	//! as a curve in the sphere whose intersection with a mesh is
	//! represented by the graph the arc belongs to.
	//! @return The geodesic curvature.
	double get_geodesic_curvature() const;

	//! @brief Gets the representing curve's arc length
	//! @return The arc length.
	double get_length() const;

      private:
	double angle;
	double radius;
	math3d::Vector center;
	math3d::Matrix rotation;
	double geodesic_curvature;
	double length;
};

//! @brief Computes a given node's location.
//! @param node the node.
//! @return The location.
math3d::Vector get_location(const Graph::Node &node);

} // namespace spherical_intersection

#endif