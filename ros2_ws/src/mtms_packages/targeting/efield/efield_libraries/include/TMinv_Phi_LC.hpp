#pragma once

#include "eigen_defines.hpp"

/**
 * @brief Builds the transfer matrix of potential using Linear Collocation (LC), does not invert
 * @note Use together with the LFMinv_Phi_LC function
 * @param D D operator from BEMOperatorsPhi_LC.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return Transfer matrix 
 * @ingroup tm_group
 */
Eigen::MatrixXf TMinv_Phi_LC(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          int32_t zerolevel = -1 // index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
);
