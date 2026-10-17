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

#ifndef SPHERICAL_INTERSECTION_IMPLEMENTATION_UTILITY_HASH_COMBINE_H
#define SPHERICAL_INTERSECTION_IMPLEMENTATION_UTILITY_HASH_COMBINE_H

#include <functional>

//! @cond DEV

namespace spherical_intersection {
namespace utility {
//! @brief Combines a given hash with the hash of a given value,
//! @param seed the given hash,
//! @param v a reference to the given value,
//! @return The combined hashes.
template <class T>
inline void hash_combine(
    std::size_t &seed,
    const T &v) { // TODO taken from https://stackoverflow.com/a/2595226
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
} // namespace utility
} // namespace spherical_intersection

//! @endcond

#endif
