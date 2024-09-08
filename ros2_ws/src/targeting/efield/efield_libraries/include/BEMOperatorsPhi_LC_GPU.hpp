#pragma once

#include "Mesh.hpp"

/**
 * @brief  Makes the double-layer (D) matrices for LC BEM, using GPU acceleration
 * @details Auto-solid angle is set for diagonal blocks
 * 
 * @param meshes BEM geometry
 * @return D blocks in an std::vector, in column-major ordering
 * @ingroup bem_group
 */
std::vector<Eigen::MatrixXf> BEMOperatorsPhi_LC_GPU(const std::vector<Mesh<float>> &meshes // BEM geometry
);
