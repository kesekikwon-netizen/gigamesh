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

#include <thread>
#include <unordered_set>
#include <vector>

#include "graph.h"
#include "algorithm/component_count.h"

using namespace spherical_intersection;

std::size_t algorithm::get_component_count(
    Graph &graph) {
	using namespace spherical_intersection;

	const auto &nodes = graph.get_nodes();
	auto remove_component = [&graph](const Graph::Node &node) {
		std::unordered_set<const Graph::Node *> active_node_ptrs{&node};
		while (!active_node_ptrs.empty()) {
			const auto &current_node = **active_node_ptrs.begin();
			active_node_ptrs.erase(active_node_ptrs.begin());

			auto outgoing_arcs = current_node.get_outgoing_arcs();
			std::vector<const Graph::Node *> successor_ptrs;
			for (const Graph::Arc &arc : outgoing_arcs) {
				const auto &end = arc.get_end();
				if (&end != &current_node) {
					active_node_ptrs.insert(&end);
				}
			}

			auto incoming_arcs = current_node.get_incoming_arcs();
			for (const Graph::Arc &arc : incoming_arcs) {
				const auto &start = arc.get_start();
				if (&start != &current_node) {
					active_node_ptrs.insert(&start);
				}
			}

			graph.erase_node(current_node);
		}
	};
	std::size_t component_count = 0;
	while (!nodes.empty()) {
		remove_component(*nodes.begin());
		component_count++;
	}
	return component_count;
}
