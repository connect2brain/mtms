#pragma once

#include "Mesh.hpp"
#include "Coilset.hpp"
#include "eigen_defines.hpp"

// TODO should Tphi be matrixGPU?

/**
 * @ingroup meg
 */
Eigen::MatrixXf LFM_B_LC_GPU(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos);
/**
 * @ingroup meg
 */
Eigen::MatrixXf LFM_B_LC_GPU(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos, const MatrixX3f_RM &sdir);

/**
 * @ingroup meg
 * @note Checks for available GPU memory and either does calculation one mesh at a time or everything at once
 */
Eigen::MatrixXf LFM_B_LG_GPU(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos, bool use_turbo = false);
/**
 * @ingroup meg
 */
Eigen::MatrixXf LFM_B_LG_GPU(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos, const MatrixX3f_RM &sdir, bool use_turbo = false);

