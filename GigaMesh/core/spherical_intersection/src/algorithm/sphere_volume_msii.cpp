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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include <algorithm>
#include <limits>
#include <thread>
#include <vector>
#include <numeric>

#include "algorithm/sphere_volume_msii.h"
#include "graph.h"
#include "graph_geometry.h"
#include "math3d/math3d.h"
#include "mesh_spherical.h"

//! @cond DEV

using namespace spherical_intersection;
namespace {
//! @brief Computes the rotation angle when rotating one given vector onto
//! another around a given normal vector.
//! @param v_1 the first vector.
//! @param v_2 the second vector.
//! @param normal the normal vector
//! @return The rotation angle when rotating v_1 onto v_2 around normal.
double get_turning_angle(const math3d::Vector v_1, const math3d::Vector v_2,
			 const math3d::Vector normal) {
	if (math3d::dot_product(math3d::cross_product(v_1, v_2), normal) >= 0) {
		return math3d::get_angle(v_1, v_2);
	} else {
		return -math3d::get_angle(v_1, v_2);
	}
}

//! @brief Chooses an arbitrary arc of a given graph if that graph has at least
//! one arc. Otherwise returns std::numeric_limits::quiet_NaN. If an arc is
//! chosen, choses a cycle containing this arc if such a cycle exists. Otherwise
//! deletes the chosen arc and returns std::numeric_limits::quiet_NaN. If a
//! cycle is chosen, deletes that cycle's arcs from the graph and returns the
//! area of the surface whose boundary is given by the cycle. Additional arcs
//! that do not occur in any cycle may be deleted in any case.
//! @param graph the graph.
//! @return The area of the outward oriented sphere's submanifold whose oriented
//! boundary is given by a directed cycle if an arc was chosen that occurs in a
//! cycle. std::numeric_limits::quiet_NaN otherwise.

// The implementation uses the Gauss-Bonnet theorem. An arc's contribution is
// the sum of the integral over itss geodesic curvature and the turning angle
// from the end of the previous arc to its start.
// A cycle is chosen by starting at a random arc and randomly choosing a next
// arc starting at the previous arc's end.
double extract_cycle_contribution(Graph &graph) {
	using Graph = Graph;
	using Node = typename Graph::Node;
	using Arc = typename Graph::Arc;

	double accumulation = 0;
	const auto &arcs = graph.get_arcs();
	if (arcs.empty()) {
		return std::numeric_limits<double>::quiet_NaN();
	}

	std::vector<std::reference_wrapper<const Arc>> arcs_of_cycle;
	std::vector<const Node *> nodes_of_cycle_ptrs;
	// track all arc's contributions except the first, since it depends on
	// the last.
	std::vector<double> arc_contributions_to_cycle;
	const Arc &current_arc = *arcs.begin();
	arcs_of_cycle.push_back(current_arc);
	nodes_of_cycle_ptrs.push_back(&current_arc.get_start());
	const auto *current_node_ptr = &current_arc.get_end();
	Arc_Curve current_curve = Arc_Curve(current_arc);

	// given the next arc performs the following: adds the current arc's
	// contribution to arc_contributions_to_cycle, updates current_arc,
	// current_node_ptr, arcs_of_cycle and nodes_of_cycle_ptrs
	auto go_to_next_arc = [&arcs_of_cycle, &nodes_of_cycle_ptrs,
			       &arc_contributions_to_cycle, &current_curve,
			       &current_node_ptr](const Arc &arc) {
		auto end_tangent = current_curve.get_tangent_at(1);
		arcs_of_cycle.push_back(arc);
		current_curve = Arc_Curve(arc);
		auto start_tangent = current_curve.get_tangent_at(0);
		double angle = get_turning_angle(
		    end_tangent, start_tangent,
		    math3d::normalize(current_curve.eval(0) -
				      arc.get_sphere().get_center()));
		arc_contributions_to_cycle.push_back(
		    angle + current_curve.get_geodesic_curvature() *
				current_curve.get_length());

		nodes_of_cycle_ptrs.push_back(current_node_ptr);
		current_node_ptr = &arc.get_end();
	};

	const auto start_curve = current_curve;
	auto it = std::find(nodes_of_cycle_ptrs.begin(),
			    nodes_of_cycle_ptrs.end(), current_node_ptr);
	while (!nodes_of_cycle_ptrs.empty() &&
	       it == nodes_of_cycle_ptrs.end()) {
		const auto &current_outgoing_arcs =
		    current_node_ptr->get_outgoing_arcs();
		if (!current_outgoing_arcs.empty()) {
			go_to_next_arc(*current_outgoing_arcs.begin());
			it = std::find(nodes_of_cycle_ptrs.begin(),
				       nodes_of_cycle_ptrs.end(),
				       current_node_ptr);
		} else {
			const Arc &arc_to_remove = arcs_of_cycle.back();
			current_node_ptr = &arc_to_remove.get_start();
			graph.erase_arc(arc_to_remove);
			arcs_of_cycle.pop_back();
			nodes_of_cycle_ptrs.pop_back();
			it = nodes_of_cycle_ptrs.end();
		}
	}
	if (arcs_of_cycle.empty()) {
		return std::numeric_limits<double>::quiet_NaN();
	}
	// calculations to determine the first arc's contribution
	auto end_tangent = current_curve.get_tangent_at(1);
	auto start_tangent = start_curve.get_tangent_at(0);
	const Arc &arc = arcs_of_cycle.back();
	double angle =
	    get_turning_angle(end_tangent, start_tangent,
			      math3d::normalize(current_curve.eval(0) -
						arc.get_sphere().get_center()));

	// accumulate contributions
	accumulation += angle + start_curve.get_geodesic_curvature() *
				    start_curve.get_length();

	std::accumulate(arc_contributions_to_cycle.begin(), arc_contributions_to_cycle.end(), accumulation);

	// erase the cycle's arcs from graph
	for (const Arc &circleArc : arcs_of_cycle) {
		graph.erase_arc(circleArc);
	}

	return 2 * M_PI - accumulation;
}
} // namespace

//! @endcond

// The implementation makes use of the cycle cut relation in dual planar graphs.
double algorithm::get_sphere_volume_area(Graph &graph) {
	const auto &arcs = graph.get_arcs();

	auto accumulation = [&] {
		while (!arcs.empty()) {
			auto cycle_contribution =
			    extract_cycle_contribution(graph);
			if (!std::isnan(cycle_contribution)) {
				return cycle_contribution;
			}
		}
		return std::numeric_limits<double>::quiet_NaN();
	}();

	while (!arcs.empty()) {
		auto cycle_contribution = extract_cycle_contribution(graph);
		if (!std::isnan(cycle_contribution)) {
			accumulation += cycle_contribution;
		}
	}

	return accumulation - 4 * M_PI * std::floor(accumulation / (4 * M_PI));
}
