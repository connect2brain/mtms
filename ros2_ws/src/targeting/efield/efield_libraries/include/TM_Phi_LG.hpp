//
// Created by Kalle Jyrkinen on 14.3.2022.
//

#pragma once

#include "eigen_defines.hpp"

/**
 * @brief Builds the transfer matrix of potential using Linear Galerkin (LG), inverts the result
 * @note Use together with the LFM_Phi_LG function
 * @param D D operator from BEMOperatorsPhi_D_LG.hpp
 * @param A A operator from BEMOperatorsPhi_A_LG.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return inverted Transfer matrix 
 * @ingroup tm_group
 */
Eigen::MatrixXf TM_Phi_LG(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<Eigen::SparseMatrix<float> > &A, // A-matrices
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          int32_t zerolevel = -1 // index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
);

