#pragma once
//
// Created by Kalle Jyrkinen on 14.3.2022.
//

#include "Mesh.hpp"

/**
 * @brief Compute potential due to xyz-oriented unit dipole triplets in infinite homogeneous conductor of unit conductivity (= LFM in infinite homogeneous medium)
 * @details Uses analytical Dvec integrals using double precision or dipole approximation when source points are far from mesh depending on use_turbo
 * @param mesh field mesh
 * @param sp source positions, [M x 3]
 * @param use_turbo Whether to use the accelerated turbo version of the calculation where dipole approximation is used(Dvec turbo)
 * @return resulting potential for each field point and dipole [N x 3M], [phiinf_1x phiinf_1y phiinf_1z ... phiinf_Mx phiinf_My phiinf_Mz], a column-major matrix
 */
Eigen::MatrixXf Phi_inf_LG(const Mesh<float> &mesh, // field mesh
                           const MatrixX3f_RM &sp,
                           bool use_turbo = false);

/**
 * @brief Compute potential due to electric dipoles in infinite homogeneous conductor of unit conductivity (= LFM in infinite homogeneous medium)
 * @details Uses analytical Dvec integrals using double precision or dipole approximation when source points are far from mesh depending on use_turbo
 * 
 * @param mesh field mesh
 * @param sp source positions, [M x 3]
 * @param sdir source dipole moments, [M x 3]
 * @param use_turbo Whether to use the accelerated turbo version of the calculation where dipole approximation is used(Dvec turbo)
 * @return resulting potential for each field point and dipole [N x M] [phiinf_1 ... phiinf_M], a column-major matrix
 */
Eigen::MatrixXf Phi_inf_LG(const Mesh<float> &mesh, // field mesh
                           const MatrixX3f_RM &sp, // source positions, [M x 3]
                           const MatrixX3f_RM &sdir,
                           bool use_turbo = false);

/**
 * @brief Overloads of the above for multiple meshes
 * @details Uses analytical Dvec integrals using double precision or dipole approximation when source points are far from mesh depending on use_turbo
 * @param use_turbo Whether to use the accelerated turbo version of the calculation where dipole approximation is used(Dvec turbo)
 */
Eigen::MatrixXf Phi_inf_LG(const std::vector<Mesh<float>> &meshes,
                           const MatrixX3f_RM &sp,
                           bool use_turbo = false);
/**
 * @brief Overloads of the above for multiple meshes
 * @details Uses analytical Dvec integrals using double precision or dipole approximation when source points are far from mesh depending on use_turbo
 * @param use_turbo Whether to use the accelerated turbo version of the calculation where dipole approximation is used(Dvec turbo)
 */
Eigen::MatrixXf Phi_inf_LG(const std::vector<Mesh<float>> &meshes,
                           const MatrixX3f_RM &sp,
                           const MatrixX3f_RM &sdir,
                           bool use_turbo = false);



// DOUBLE VERSIONS FOR TESTING
/*
Eigen::MatrixXd Phi_inf_LG(const Mesh<double> &mesh, const MatrixX3d_RM &sp);
Eigen::MatrixXd Phi_inf_LG(const std::vector<Mesh<double>> &meshes, const MatrixX3d_RM &sp);
*/
