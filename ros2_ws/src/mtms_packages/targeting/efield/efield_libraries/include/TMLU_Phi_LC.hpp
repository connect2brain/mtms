#pragma once

#include "eigen_defines.hpp"


/**
 * @brief Builds the transfer matrix of potential using Linear Collocation(LC), returns LU decomposition
 * @note Use together with the LFMLU_Phi_LC function
 * @param D D operator from BEMOperatorsPhi_LC.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param pivots Empty pivot vector for saving the permutations. (can be uninitialized, will be resized) The modified pivot vector is later needed with LFMLU_Phi_LC for solving the system
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return LU decomposition of the transfer matrix
 * @ingroup tm_group
 */
Eigen::MatrixXf TMLU_Phi_LC(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          Eigen::VectorXi &pivots, // pivots for the LU decomposition
                          int32_t zerolevel = -1 // index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
);
