#pragma once

#include "eigen_defines.hpp"

/**
 * @brief Builds the transfer matrix of potential using Linear Galerkin (LG), does not invert
 * @note Use together with the LFMinv_Phi_LG function
 * @param D D operator from BEMOperatorsPhi_D_LG.hpp
 * @param A A operator from BEMOperatorsPhi_A_LG.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return Transfer matrix 
 * @ingroup tm_group
 */
Eigen::MatrixXf TMinv_Phi_LG(const std::vector<Eigen::MatrixXf> &D, 
                            const std::vector<Eigen::SparseMatrix<float> > &A,
                            const std::vector<float> &ci, 
                            const std::vector<float> &co, 
                            int32_t zerolevel = -1);