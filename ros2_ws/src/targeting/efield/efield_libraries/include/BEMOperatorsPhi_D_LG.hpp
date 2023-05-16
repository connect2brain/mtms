#pragma once

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "Mesh.hpp"


/**
 * @brief Build the blocks of the D matrix, given a vector of meshes.
 * 
 * @param meshes  a vector of meshes, starting from the innermost one
 * @param quadDegree the quadrature degree used for numerical integration, i.e. the highest order polynomial that will be evaluated exactly. \n  
 *                   A recommended one achieving a decent accuracy is 7 (meaning 13 points per triangle)
 *
 * @return vector containing the blocks of D, column-major ordering
 * @ingroup bem_group
 */
std::vector<Eigen::MatrixXf> BEMOperatorsPhi_D_LG(const std::vector<Mesh<float>> &meshes, int quadDegree);
