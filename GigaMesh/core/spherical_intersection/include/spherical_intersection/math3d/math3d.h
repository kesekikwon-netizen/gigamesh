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

#ifndef SPHERICAL_INTERSECTION_MATH3D_MATH3D_H
#define SPHERICAL_INTERSECTION_MATH3D_MATH3D_H

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace spherical_intersection {
namespace math3d {

//! @brief Element of the three dimensional real coordinate space.
class Vector {
      public:
	//! @brief Constructs the zero vector.
	Vector() { elements.fill(0); }

	//! @brief Constructs a vector with given components.
	//! @param elements the vectors components.
	Vector(std::initializer_list<double> elements) {
		assert(elements.size() == 3);
		std::copy(elements.begin(), elements.end(),
			  this->elements.begin());
	}

	//! @brief Returns a reference to the requested component.
	//! @param i index of the requested component.
	//! @return A reference to the requested component.
	double &at(std::size_t i) { return elements[i]; }

	//! @brief Returns a copy of the requested component.
	//! @param i index of the requested component.
	//! @return A copy of the requested component.
	double get(std::size_t i) const { return elements[i]; }

      private:
	std::array<double, 3> elements;
};

//! @brief A real 3 x 3 matrix.
class Matrix {
      public:
	//! @brief Constructs the zero matrix.
	Matrix() { elements.fill(0); }

	//! @brief Constructs a matrix with the given entries.
	//! @param elements the matrix entries.
	Matrix(std::initializer_list<double> elements) {
		assert(elements.size() == 9);
		std::copy(elements.begin(), elements.end(),
			  this->elements.begin());
	}

	//! @brief Returns a reference to the requested entry.
	//! @param i row index of the requested entry.
	//! @param j column index of the requested entry.
	//! @return A reference to the requested entry.
	double &at(std::size_t i, std::size_t j) { return elements[i * 3 + j]; }

	//! @brief Returns a copy of the requested entry.
	//! @param i row index of the requested entry.
	//! @param j column index of the requested entry.
	//! @return A copy of the requested entry.
	double get(std::size_t i, std::size_t j) const {
		return elements[i * 3 + j];
	}

	//! @brief Returns a copy of the requested row vector.
	//! @param i row index of the requested row vector.
	//! @return A copy of the requested row vector.
	Vector get_row(std::size_t i) const {
		return Vector{this->elements[i * 3], this->elements[i * 3 + 1],
			      this->elements[i * 3 + 2]};
	}

	//! @brief Returns a copy of the requested column vector.
	//! @param j column index of the requested column vector.
	//! @return A copy of the requested row vector.
	Vector get_column(std::size_t j) const {
		return Vector{this->elements[j], this->elements[j + 3],
			      this->elements[j + 6]};
	}

      private:
	std::array<double, 9> elements;
};

//! @brief A sphere in the three dimensional real coordinate space.
class Sphere {
      public:
	//! @brief Constructs a sphere with given center and radius.
	//! @param center the sphere's center.
	//! @param radius the sphere's radius.
	Sphere(Vector center, double radius) : center(center), radius(radius) {}

	//! @brief Returns a reference to the sphere's center.
	//! @return The sphere's center.
	const Vector &get_center() const { return this->center; }

	//! @brief Returns a reference to the sphere's radius.
	//! @return The sphere's radius.
	double get_radius() const { return this->radius; }

      private:
	Vector center;
	double radius;
};

//! @brief Computes the dot product of two given vectors.
//! @param v_1 the first vector.
//! @param v_2 the second vector.
//! @return The dot product v_1 * v_2.
inline double dot_product(const Vector &v_1, const Vector &v_2) {
	return v_1.get(0) * v_2.get(0) + v_1.get(1) * v_2.get(1) +
	       v_1.get(2) * v_2.get(2);
}

//! @brief Computes the cross product of two given vectors.
//! @param v_1 the first vector.
//! @param v_2 the second vector.
//! @return The cross product v_1 x v_2.
inline Vector cross_product(const Vector &v_1, const Vector &v_2) {
	Vector result;
	for (std::size_t i = 0; i < 3; i++) {
		result.at(i) = v_1.get((i + 1) % 3) * v_2.get((i + 2) % 3) -
			       v_1.get((i + 2) % 3) * v_2.get((i + 1) % 3);
	}
	return result;
}

//! @brief Computes the square of the euclidean norm of a given vector.
//! @param v the vector.
//! @return The square of the euclidean norm of v.
inline double norm2(const Vector &v) { return dot_product(v, v); }

//! @brief Concatenates three column vectors to form a 3 x 3 matrix.
//! @param v_1 the vector for the first column.
//! @param v_2 the vector for the second column.
//! @param v_3 the vector for the third column.
//! @return The matrix.
inline Matrix concat(const Vector &v_1, const Vector &v_2, const Vector &v_3) {
	return Matrix{v_1.get(0), v_2.get(0), v_3.get(0),
		      v_1.get(1), v_2.get(1), v_3.get(1),
		      v_1.get(2), v_2.get(2), v_3.get(2)};
}

//! @brief Computes the matrix vector product of a given matrix and a given
//! vector.
//! @param m the matrix.
//! @param v the vector
//! @return The product m * v.
inline Vector operator*(const Matrix &m,
			const Vector &v) { // TODO faster algorithms exist...
	return Vector{dot_product(m.get_row(0), v),
		      dot_product(m.get_row(1), v),
		      dot_product(m.get_row(2), v)};
}

//! @brief Computes the sum of two given vectors.
//! @param v_1 the first vector.
//! @param v_2 the second vector.
//! @return The sum v_1 + v_2.
inline Vector operator+(const Vector &v_1, const Vector &v_2) {
	return Vector{v_1.get(0) + v_2.get(0), v_1.get(1) + v_2.get(1),
		      v_1.get(2) + v_2.get(2)};
}

//! @brief Computes the difference of two given vectors.
//! @param v_1 the first vector.
//! @param v_2 the second vector.
//! @return The difference v_1 - v_2.
inline Vector operator-(const Vector &v_1, const Vector &v_2) {
	return Vector{v_1.get(0) - v_2.get(0), v_1.get(1) - v_2.get(1),
		      v_1.get(2) - v_2.get(2)};
}

//! @brief Computes the product of a given scalar and a given vector.
//! @param factor the scalar.
//! @param v the vector
//! @return The product factor * v.
inline Vector operator*(double factor, const Vector &v) {
	return Vector{factor * v.get(0), factor * v.get(1), factor * v.get(2)};
}

//! @brief Computes the normal vector parallel to a given vector.
//!
//! The vector is expected to be nonzero.
//! @param v the vector.
//! @return The normal vector parallel to v.
inline Vector normalize(const Vector &v) {
	double inverse_factor = std::sqrt(norm2(v));
	assert(inverse_factor != 0);
	return (1 / inverse_factor) * v;
}

//! @brief Computes a nonzero vector whose scalar product with a given vector
//! is zero and whose uniform norm is the same as the given vector's.
//! @param v a reference to the given vector.
//! @return A nonzero vector whose scalar product with v is zero and whose
//! uniform norm is the same as that of v.
inline Vector get_orthogonal(const Vector &v) {
	auto index_of_max = [&] {
		std::size_t index_of_max = 0;
		for (std::size_t index = 0; index < 3; index++) {
			if (v.get(index) > v.get(index_of_max)) {
				index_of_max = index;
			}
		}
		return index_of_max;
	}();

	Vector orthogonal_vector{0, 0, 0};
	orthogonal_vector.at(index_of_max) = v.get((index_of_max + 1) % 3);
	orthogonal_vector.at((index_of_max + 1) % 3) = -v.get(index_of_max);

	return orthogonal_vector;
}

//! @brief Computes the minimum of the two angles enclosed by two given vectors.
//!
//! Both vectors are expected to be nonzero.
//! @param v_1 the first vector.
//! @param v_2 the second vector.
//! @return The minimum of the two angles enclosed by v_1 and v_2.
inline double get_angle(const Vector &v_1, const Vector &v_2) {
	return std::acos(
	    std::max(std::min(math3d::dot_product(math3d::normalize(v_1),
						  math3d::normalize(v_2)),
			      1.0),
		     -1.0));
}

} // namespace math3d
} // namespace spherical_intersection

#endif