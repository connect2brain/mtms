#pragma once

#include "Mesh.hpp"
#include "Coilset.hpp"
#include "eigen_defines.hpp"
/**
 * @ingroup meg
 */


/**
 * @brief Magnetic lead field matrix with source dipole triplets and LC Phi_inf
 * 
 * @param meshes BEM geometry, vector of meshes, total number of points M
 * @param coilset Coilset struct [Nc coils]
 * @param TBvol Output from TM_Bvol function [M x Nc]
 * @param spos Source/cortex points [N x 3]
 * @return B lead field matrix [3*N x Nc]
 */
Eigen::MatrixXf LFM_B_LC(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos);

/**
 * @brief Magnetic lead field matrix with source directions and LC Phi_inf
 * 
 * @param meshes BEM geometry, vector of meshes, total number of points M
 * @param coilset Coilset struct [Nc coils]
 * @param TBvol Output from TM_Bvol function [M x Nc]
 * @param spos Source/cortex points [N x 3]
 * @param sdir Source/cortex orientations [N x 3]
 * @return B lead field matrix [N x Nc]
 */
Eigen::MatrixXf LFM_B_LC(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos, const MatrixX3f_RM &sdir);

/**
 * @brief Magnetic lead field matrix with source dipole triplets and LG Phi_inf
 * 
 * @param meshes BEM geometry, vector of meshes, total number of points M
 * @param coilset Coilset struct [Nc coils]
 * @param TBvol Output from TM_Bvol function [M x Nc]
 * @param spos Source/cortex points [N x 3]
 * @param use_turbo Whether to use dipole approxximation in Phi_inf with large distances
 * @return B lead field matrix [3*N x Nc]
 */
Eigen::MatrixXf LFM_B_LG(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos, bool use_turbo = false);

/**
 * @brief Magnetic lead field matrix with source directions and LG Phi_inf
 * 
 * @param meshes BEM geometry, vector of meshes, total number of points M
 * @param coilset Coilset struct [Nc coils]
 * @param TBvol Output from TM_Bvol function [M x Nc]
 * @param spos Source/cortex points [N x 3]
 * @param sdir Source/cortex orientations [N x 3]
 * @param use_turbo Whether to use dipole approxximation in Phi_inf with large distances
 * @return B lead field matrix [N x Nc]
 */
Eigen::MatrixXf LFM_B_LG(const std::vector<Mesh<float>>  &meshes, const Coilset<float> &coilset, const Eigen::MatrixXf &TBvol, const MatrixX3f_RM &spos, const MatrixX3f_RM &sdir, bool use_turbo = false);