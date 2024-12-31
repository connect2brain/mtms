#pragma once
#include "eigen_defines.hpp"
#include <vector>

/**
 * @brief weighs DB by conductivities and multiplies the result by TM.
 * 
 * @param DB vector of DB matrices, 1 for each Mesh [[Nsp_1 x Nfp] , [Nsp_2 x Nfp], ..., [Nsp_N x Nfp]]
 * @param TM BEM transfer matrix for potential [Nsp x Nsp], Nsp = sum(Nsp_i)
 * @param ci conductivities inside the meshes [N]
 * @param co conductivities outside the meshes [N]
 * @return Transfer matrix for computing B_vol [Nsp, Nfp]
 */
Eigen::MatrixXf TM_Bvol_Linear(const std::vector<Eigen::MatrixXf> &DB, const Eigen::MatrixXf &TM,  const std::vector<float> &ci, const std::vector<float> &co);
