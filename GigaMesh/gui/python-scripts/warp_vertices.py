# coding=utf-8

# GigaMesh - The GigaMesh Software Framework is a modular software for display,
# editing and visualization of 3D-data typically acquired with structured light or
# structure from motion.
# Copyright (C) 2009-2020 Hubert Mara
#
# This file is part of GigaMesh.
#
# GigaMesh is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GigaMesh is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.


import argparse
import os.path

import numpy
import numpy.linalg
import scipy.spatial.distance


def transform(points, control, affine, warping):
    assert numpy.allclose(points[:, 0], 1)

    assert affine.shape[0] == affine.shape[1]
    assert warping.shape == control.shape

    kernel = scipy.spatial.distance.cdist(points, control)
    kernel = numpy.power(kernel, 2) * numpy.log(kernel, where=kernel > 0)
    kernel[numpy.where(numpy.isnan(kernel))] = 0

    assert numpy.all(numpy.isfinite(kernel))

    points_1 = numpy.dot(points, affine) + numpy.dot(kernel, warping)

    assert numpy.all(numpy.isfinite(points_1))

    return points_1


def minimize(V, Y, l):
    # TODO: Document lacking regularization on affine transformation.

    assert l >= 0
    assert V.shape == Y.shape
    assert numpy.allclose(V[:, 0], 1)
    assert numpy.allclose(Y[:, 0], 1)

    k, m = V.shape

    I = numpy.identity(k - m)

    phi = scipy.spatial.distance.pdist(V)
    phi = scipy.spatial.distance.squareform(phi)
    phi = numpy.power(phi, 2) * numpy.log(phi, where=phi > 0)
    phi[numpy.where(numpy.isnan(phi))] = 0

    assert phi.shape == (k, k)
    assert numpy.all(numpy.isfinite(phi))

    # If R is left untouched, it cannot be inverted later.
    # In Chui's .m code, however, R is sliced down to m,
    # i.e. the point's dimensions. This is not explained in the paper,
    # the formula shows R to be in the upper left of a matrix expression.

    Q, R = numpy.linalg.qr(V, mode="complete")
    R = R[:m, :m]
    Q1, Q2 = Q[:, :m], Q[:, m:]

    assert Q1.shape == (k, m)
    assert Q2.shape == (k, k - m)

    w_hat = Q2 @ numpy.linalg.inv(Q2.T @ phi @ Q2 + l * I) @ Q2.T @ Y

    assert w_hat.shape == (k, m)

    # In the paper it is stated R^-1 (Q1' V - K w), however the dimensions
    # of the matrices, i.e. (k, m) (m, k) - (k, k) (k, m),
    # are not compatible this way. Further, the weighted assignment Y has
    # no influence on the rigid transformation.
    # In his code, Chui uses a similar formulation
    # where the parenthesis is moved once left and Y is used instead of V
    # Thus, I assume the parenthesis placement and the usage of V
    # in the paper are typos and incorrect.

    R_inv = numpy.linalg.inv(R)
    d_hat = R_inv @ Q1.T @ (Y - phi @ w_hat)

    assert d_hat.shape == (m, m)

    assert numpy.all(numpy.isfinite(d_hat))
    assert numpy.all(numpy.isfinite(w_hat))

    return (V, d_hat, w_hat)


def main():
    parser = argparse.ArgumentParser(
        description='Warp left mesh given left control points to fit right control points.')
    parser.add_argument('--in-vertices',
                        help='Vertices of the original mesh to be warped.'
                             'Formatted as CSV file with one vertex per row.')
    parser.add_argument('--in-control-left',
                        help='Control points on the left mesh.'
                             'Formatted as CSV file with one control '
                             'point per row.')
    parser.add_argument('--in-control-right',
                        help='Control points on the right mesh.'
                             'Formatted as CSV file with one control '
                             'point per row.')
    parser.add_argument('--out-vertices',
                        help='Filename of warped output mesh.'
                             'File must not exist yet and must end in .csv, vertices will be given as points per row.')
    args = parser.parse_args()

    assert os.path.exists(args.in_vertices)
    assert os.path.exists(args.in_control_left)
    assert os.path.exists(args.in_control_right)

    in_vertices = numpy.loadtxt(args.in_vertices)
    control_left = numpy.loadtxt(args.in_control_left)
    control_right = numpy.loadtxt(args.in_control_right)

    # Save and strip indices, them convert to
    # homogeneous (1, x, y, z) coordinates
    in_vertices_indices = numpy.copy(in_vertices[:, 0])
    in_vertices[:, 0] = 1

    # Convert control points to homogeneous coordinates.
    # No indices to strip here.
    control_left = numpy.stack((numpy.ones((control_left.shape[0],)),
                                control_left[:, 0], control_left[:, 1], control_left[:, 2]), axis=1)

    control_right = numpy.stack((numpy.ones((control_right.shape[0],)),
                                 control_right[:, 0], control_right[:, 1], control_right[:, 2]), axis=1)

    assert control_left.shape[0] == control_right.shape[0]

    # Fit transform params
    control_left, affine, warping = minimize(control_left, control_right, 1.0)

    # Perform actual transform
    out_vertices = transform(in_vertices, control_left, affine, warping)

    # Compute distance between warped vertices
    distances = numpy.linalg.norm(out_vertices - in_vertices, axis=1)

    # Compose result matrix
    result = numpy.zeros((out_vertices.shape[0], 5))
    result[:, 0] = in_vertices_indices
    result[:, [1, 2, 3]] = out_vertices[:, [1, 2, 3]]
    result[:, 4] = distances

    numpy.savetxt(args.out_vertices, result, fmt="%d %f %f %f %f")


if __name__ == "__main__":
    main()
