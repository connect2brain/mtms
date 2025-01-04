
#pragma once

#include "eigen_defines.hpp"

/**
 * @brief Implements LG ISA tranfer matrix as given in M Stenroos and J Sarvas, Bioelectromagnetic forward problem: isolated source approach revis(it)ed
 * @note Use together with the LFM_Phi_LG function
 * @param D D operator from BEMOperatorsPhi_D_LG.hpp
 * @param A A operator from BEMOperatorsPhi_A_LG.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return inverted Transfer matrix 
 * @ingroup tm_group
 */
Eigen::MatrixXf TM_Phi_LG_ISA(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<Eigen::SparseMatrix<float> > &A, // A-matrices
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          int32_t zerolevel
);
/**
 * @brief LC ISA, see LG ISA above for interface
 * 
 */
Eigen::MatrixXf TM_Phi_LC_ISA(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          int32_t zerolevel
);


