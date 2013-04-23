#pragma once

#include <Eigen\Sparse>
#include <opencv2\opencv.hpp>
#include <complex>
#include <limits>

#include "WeightedGraph.hpp"
#include "Utils.hpp"

using namespace std;
using namespace cv;

typedef Mat_<double> (*MatrixRepresentation)(const WeightedGraph&);

/**
 * Computes the unnormalized laplacian matrix L of a weighted graph, defined
 * by:
 *
 * L(u,v) = dv - w(v,v)	if u = v
 *          -w(u,v)		if u and v are adjacent
 *			0			otherwise
 *
 * Assumes the graph represents each edge only once, ie. that the
 * adjacency lists are not bidirectional.
 *
 * @param graph graph to compute the laplacian from.
 * @return the laplacian matrix of the weighted graph.
 */
Mat_<double> laplacian(const WeightedGraph &graph);

/**
 * Same as laplacian, but returns a sparse matrix data structure.
 * Useful when dealing with graphs with a lot of vertices, but
 * relatively few edges (for instances grid graphs, nearest neighbor
 * graphs, large planar graphs).
 */
Eigen::SparseMatrix<double> sparseLaplacian(const WeightedGraph &graph, bool bidirectional, Eigen::VectorXd &degrees);

/**
 * Computes the normalized laplacian NL matrix of a weighted graph, defined by:
 *
 * NL(u,v) = 1 - w(v,v)/dv			if u = v
 *			 -w(u,v)/sqrt(du*dv)	if u and v are adjacent
 *			0						otherwise
 * Assumes the graph represents each edge only once, ie. that the
 * adjacency lists are not bidirectional.
 *
 * @param graph graph to compute the laplacian from.
 * @return the normalized laplacian matrix of the weighted graph.
 */
Mat_<double> normalizedLaplacian(const WeightedGraph &graph);

/**
 * Same as normalizedLaplacian, but returns a sparse matrix data structure. Assumes
 * a bidirectional graph representation without loops.
 */
Eigen::SparseMatrix<double> normalizedSparseLaplacian(const WeightedGraph &graph);

/**
 * Computes the normalized laplacian of the graph in a upper triangular symmetric packed 
 * storage dense column major format intended for use with LAPACK. Expects a non bidirectional graph.
 *
 * @param graph graph to compute the normalized laplacian from.
 * @param L output array storage for the normalized laplacian. Memory management is up to the
 * caller, should be of size at least n(n + 1) / 2 where n is the number of vertices in the graph.
 */
void packedStorageNormalizedLaplacian(const WeightedGraph &graph, double *L);