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

#include <cassert>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "graph.h"
#include "graph_geometry.h"
#include "math3d/math3d.h"

using namespace spherical_intersection;

//! @cond DEV

namespace {
//! @brief Computes a unit vector describing the direction from the given arc's
//! start node's location to its end node's location. Estimates direction when
//! the distance from these locations is less than the given tolerance.
//! @param arc the arc.
//! @param tolerance the tolerance.
//! @return The unit vector describing the direction.
math3d::Vector get_direction(const Graph::Arc &arc, double tolerance) {
	const auto &start = arc.get_start();
	const auto &end = arc.get_end();
	auto start_location = get_location(start);
	auto end_location = get_location(end);

	auto try_normalize = [&tolerance](const math3d::Vector &v) {
		auto norm2 = math3d::norm2(v);
		if (norm2 >= std::pow(tolerance, 2)) {
			return (1 / std::sqrt(norm2)) * v;
		} else {
			return math3d::Vector{1, 0, 0};
		}
	};

	auto location_difference = end_location - start_location;
	auto location_difference_norm2 = math3d::norm2(location_difference);
	if (location_difference_norm2 < std::pow(tolerance, 2)) {
		const auto &start_vert_1 = start.get_edge().get_vertex(0);
		const auto &start_vert_2 = start.get_edge().get_vertex(1);
		const auto &end_vert_1 = end.get_edge().get_vertex(0);
		const auto &end_vert_2 = end.get_edge().get_vertex(1);
		if (&start_vert_1 == &end_vert_1 &&
		    &start_vert_2 == &end_vert_2) {
			if (start.get_position() < end.get_position()) {
				return try_normalize(
				    start_vert_2.get_location() -
				    start_vert_1.get_location());
			} else {
				return try_normalize(
				    start_vert_1.get_location() -
				    start_vert_2.get_location());
			}
		} else {
			auto estimate_direction_at_vertex =
			    [&tolerance, &try_normalize](
				const Mesh::Vertex &extension_of_start,
				const Mesh::Vertex &common_vertex,
				const Mesh::Vertex &extension_of_end) {
				    auto v_1 = try_normalize(
					extension_of_start.get_location() -
					common_vertex.get_location());
				    auto v_2 = try_normalize(
					extension_of_end.get_location() -
					common_vertex.get_location());

				    auto difference = v_2 - v_1;
				    auto difference_norm2 =
					math3d::norm2(difference);
				    if (difference_norm2 >=
					std::pow(tolerance, 2)) {
					    return (1 / std::sqrt(
							    difference_norm2)) *
						   difference;
				    } else {
					    return math3d::get_orthogonal(v_1);
				    }
			    };

			if (&start_vert_1 == &end_vert_1) {
				return estimate_direction_at_vertex(
				    start_vert_2, start_vert_1, end_vert_2);
			} else if (&start_vert_1 == &end_vert_2) {
				return estimate_direction_at_vertex(
				    start_vert_2, start_vert_1, end_vert_1);
			} else if (&start_vert_2 == &end_vert_1) {
				return estimate_direction_at_vertex(
				    start_vert_1, start_vert_2, end_vert_2);
			} else if (&start_vert_2 == &end_vert_2) {
				return estimate_direction_at_vertex(
				    start_vert_1, start_vert_2, end_vert_1);
			} else {
				throw std::logic_error(
				    "Incompatible start and end given.");
			}
		}
	} else {
		return (1 / std::sqrt(location_difference_norm2)) *
		       location_difference;
	}
}
} // namespace

//! @endcond

Arc_Curve::Arc_Curve(const Graph::Arc &arc, const double tolerance) {
	math3d::Vector v_1 = arc.get_triangle().get_vertex(0).get_location();
	math3d::Vector v_2 = arc.get_triangle().get_vertex(1).get_location();
	math3d::Vector v_3 = arc.get_triangle().get_vertex(2).get_location();
	math3d::Vector triangle_normal_vec =
	    math3d::cross_product(v_2 - v_1, v_3 - v_1);

	math3d::Vector sphere_center = arc.get_sphere().get_center();
	double sphere_radius = arc.get_sphere().get_radius();

	math3d::Vector start_location = get_location(arc.get_start());
	math3d::Vector end_location = get_location(arc.get_end());

	math3d::Vector triangle_normal = [&] {
		if (math3d::norm2(triangle_normal_vec) <
		    std::pow(tolerance, 2)) {
			return math3d::normalize(start_location -
						 sphere_center);
		} else {
			return math3d::normalize(triangle_normal_vec);
		}
	}();

	double othogonal_offset_from_sphere_center =
	    math3d::dot_product(triangle_normal, v_1 - sphere_center);
	this->center = othogonal_offset_from_sphere_center * triangle_normal +
		       sphere_center;
	this->radius =
	    std::sqrt(std::pow(sphere_radius, 2) -
		      std::pow(othogonal_offset_from_sphere_center, 2));

	math3d::Vector normal_at_start = [&] {
		if (this->radius < tolerance) {
			if (triangle_normal.get(0) == 0) {
				return math3d::normalize(
				    math3d::Vector{0, -triangle_normal.get(2),
						   triangle_normal.get(1)});
			} else {
				return math3d::normalize(
				    math3d::Vector{-triangle_normal.get(1),
						   triangle_normal.get(0), 0});
			}
		} else {
			return math3d::normalize(start_location - center);
		}
	}();

	math3d::Vector tangent_at_start =
	    math3d::cross_product(normal_at_start, triangle_normal);
	this->rotation =
	    math3d::concat(normal_at_start, triangle_normal, tangent_at_start);

	this->angle = [&] {
		// checks on which side of the line connecting start
		// and end locations the center is to see whether
		// the angle is larger than pi. in case of very
		// close start and end locations a good enough
		// estimate for the line is implicetly used
		math3d::Vector line_normal_vec = math3d::cross_product(
		    get_direction(arc, tolerance), triangle_normal);
		if (math3d::dot_product(start_location - this->center,
					line_normal_vec) <= 0) {
			return math3d::get_angle(start_location - this->center,
						 end_location - this->center);
		} else {
			return 2 * M_PI -
			       math3d::get_angle(start_location - this->center,
						 end_location - this->center);
		}
	}();

	this->geodesic_curvature = [&] {
		if (othogonal_offset_from_sphere_center > 0) {
			return -std::sqrt(
				   1 -
				   std::pow(this->radius / sphere_radius, 2)) /
			       this->radius;
		} else {
			return std::sqrt(
				   1 -
				   std::pow(this->radius / sphere_radius, 2)) /
			       this->radius;
		}
	}();

	this->length = angle * radius;
}

math3d::Vector Arc_Curve::eval(double value) const {
	return this->radius * (this->rotation *
			       math3d::Vector{std::cos(this->angle * value), 0,
					      std::sin(this->angle * value)}) +
	       this->center;
}

math3d::Vector Arc_Curve::get_tangent_at(double value) const {
	return this->rotation * math3d::Vector{-std::sin(this->angle * value),
					       0,
					       std::cos(this->angle * value)};
}

math3d::Vector Arc_Curve::get_normal_at(double value) const {
	return this->rotation * math3d::Vector{std::cos(this->angle * value), 0,
					       std::sin(this->angle * value)};
}

double Arc_Curve::get_geodesic_curvature() const {
	return this->geodesic_curvature;
}

double Arc_Curve::get_length() const { return this->length; }

math3d::Vector spherical_intersection::get_location(const Graph::Node &node) {
	const auto &vertex_1 = node.get_edge().get_vertex(0);
	const auto &vertex_2 = node.get_edge().get_vertex(1);

	const auto &sphere = node.get_sphere();
	math3d::Vector u = vertex_1.get_location() - sphere.get_center();
	math3d::Vector v = vertex_2.get_location() - sphere.get_center();

	std::vector<double> factors;

	double a = math3d::norm2(v - u);
	assert(a != 0);
	double b = 2 * math3d::dot_product(u, v - u);
	double c = math3d::norm2(u) - std::pow(sphere.get_radius(), 2);

	double d = std::pow(b, 2) - 4 * a * c;

	if (d < 0) {
		d = 0;
	}
	if (d == 0) {
		factors.push_back(-b / (2 * a));
		factors.push_back(-b / (2 * a));
	} else {
		double sqrt_d = std::sqrt(d);
		if (node.get_enters_on_first()) {
			if (a > 0) {
				factors.push_back((-b - sqrt_d) / (2 * a));
				factors.push_back((-b + sqrt_d) / (2 * a));
			} else {
				factors.push_back((-b + sqrt_d) / (2 * a));
				factors.push_back((-b - sqrt_d) / (2 * a));
			}
		} else {
			if (a > 0) {
				factors.push_back((-b + sqrt_d) / (2 * a));
			} else {
				factors.push_back((-b - sqrt_d) / (2 * a));
			}
		}
	}

	return u + factors[node.get_position()] * (v - u) + sphere.get_center();
}
