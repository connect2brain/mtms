#pragma once

#include <cassert>
//#include "A_LG.hpp" // removed by Matti 030213 --- caused trouble in using the library; replaced essentials below 
#include <vector>
#include "Mesh.hpp"
#include "Eigen/Sparse"
/**
 * @brief Build the blocks of the A matrix, given a vector of meshes.
 * 
 * @param meshes a vector of meshes, starting from the innermost one
 * @return a vector containing the diagonal blocks of A (the off-diagonal blocks are zeros). Since A is mostly filled with zeros, use sparse matrices
 * @ingroup bem_group
 */
std::vector<Eigen::SparseMatrix<float> > BEMOperatorsPhi_A_LG(const std::vector<Mesh<float>> &meshes);
