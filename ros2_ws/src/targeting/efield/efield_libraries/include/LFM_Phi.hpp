//
// Created by Kalle Jyrkinen on 14.3.2022.
//
// Edited by Tuomas Mylläri 27.6.2022

#pragma once

#include "Mesh.hpp"

/**
 * @brief Build the electric lead field matrix from INVERTED Transfer matrix. Uses \b LC and source orientations
 * @note Uses Eigen if macros USE_LAPACKE or USE_MKL are not defined
 * 
 * @param meshes BEM geometry
 * @param Tphi invertedBEM transfer matrix
 * @param spos source positions, [M x 3]
 * @param sdir source orientations, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFM_Phi_LC(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &Tphi, // BEM transfer matrix
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           const MatrixX3f_RM &sdir, // source orientations, [M x 3]
                           bool averef = true // use average reference?
);
/**
 * @brief Build the electric lead field matrix from INVERTED Transfer matrix. Uses \b LC and xyz unit dipole triplets
 * @note Uses Eigen if macros USE_LAPACKE or USE_MKL are not defined
 * 
 * @param meshes BEM geometry
 * @param Tphi invertedBEM transfer matrix
 * @param spos source positions, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFM_Phi_LC(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &Tphi, // BEM transfer matrix
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           bool averef = true // use average reference? 1 for yes, 2 for no
);

/**
 * @brief Build the electric lead field matrix from INVERTED Transfer matrix. Uses \b LG and source orientations
 * @note Uses Eigen if macros USE_LAPACKE or USE_MKL are not defined
 * 
 * @param meshes BEM geometry
 * @param Tphi invertedBEM transfer matrix
 * @param spos source positions, [M x 3]
 * @param sdir source orientations, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFM_Phi_LG(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &Tphi, // BEM transfer matrix
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           const MatrixX3f_RM &sdir, // source orientations, [M x 3]
                           bool averef = true, // use average reference?
                           bool use_turbo = false
);

/**
 * @brief Build the electric lead field matrix from INVERTED Transfer matrix. Uses \b LG and xyz unit dipole triplets
 * @note Uses Eigen if macros USE_LAPACKE or USE_MKL are not defined
 * 
 * @param meshes BEM geometry
 * @param Tphi invertedBEM transfer matrix
 * @param spos source positions, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFM_Phi_LG(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &Tphi, // BEM transfer matrix
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           bool averef = true, // use average reference?
                           bool use_turbo = false
);



