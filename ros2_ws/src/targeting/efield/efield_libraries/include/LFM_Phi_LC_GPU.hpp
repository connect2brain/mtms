#pragma once

#include "Mesh.hpp"

/**
 * @file LFM_Phi_LC_GPU.hpp
 * @brief GPU versions of LFM_Phi_LC. similar interfaces
 */

/**
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFM_Phi_LC_GPU(const std::vector<Mesh<float>> &meshes,
                               const Eigen::MatrixXf &Tphi,
                               const MatrixX3f_RM &spos,
                               const MatrixX3f_RM &sdir,
                               bool averef = true);
/**
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFM_Phi_LC_GPU(const std::vector<Mesh<float>> &meshes,
                               const Eigen::MatrixXf &Tphi,
                               const MatrixX3f_RM &spos,
                               bool averef = true);
