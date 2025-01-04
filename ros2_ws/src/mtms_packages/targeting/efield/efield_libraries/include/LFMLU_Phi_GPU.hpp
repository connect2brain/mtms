#pragma once

#include "Mesh.hpp"


/**
 * @brief Build the electric lead field matrix from LU decomposition of T matrix using \b GPU if possible. Uses \b LC and source orientations
 * @note No eigen option available at least for now
 * 
 * @param meshes BEM geometry
 * @param LU LU decomposition of BEM transfer matrix
 * @param pivots pivots from the decomposition
 * @param spos source positions, [M x 3]
 * @param sdir source orientations, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFMLU_Phi_LC_GPU(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &LU, // LU decomposition of BEM transfer matrix
                           Eigen::VectorXi &pivots, // pivots from the decomposition
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           const MatrixX3f_RM &sdir, // source orientations, [M x 3]
                           bool averef = true // use average reference?
);
/**
 * @brief Build the electric lead field matrix from LU decomposition of T matrix using \b GPU if possible. Uses \b LC and xyz unit triplets per source point
 * @note No eigen option available at least for now
 * 
 * @param meshes BEM geometry
 * @param LU LU decomposition of BEM transfer matrix
 * @param pivots pivots from the decomposition
 * @param spos source positions, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFMLU_Phi_LC_GPU(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &LU, // LU decomposition of BEM transfer matrix
                           Eigen::VectorXi &pivots, // pivots from the decomposition
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           bool averef = true // use average reference? 1 for yes, 2 for no
);

/**
 * @brief Build the electric lead field matrix from LU decomposition of T matrix using \b GPU if possible. Uses \b LG and source orientations
 * @note No eigen option available at least for now
 * 
 * @param meshes BEM geometry
 * @param LU LU decomposition of BEM transfer matrix
 * @param pivots pivots from the decomposition
 * @param spos source positions, [M x 3]
 * @param sdir source orientations, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFMLU_Phi_LG_GPU(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &LU, // LU decomposition of BEM transfer matrix
                           Eigen::VectorXi &pivots, // pivots from the decomposition
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           const MatrixX3f_RM &sdir, // source orientations, [M x 3]
                           bool averef = true, // use average reference?
                           bool use_turbo = false 
);

/**
 * @brief Build the electric lead field matrix from LU decomposition of T matrix using \b GPU if possible. Uses \b LG and xyz unit triplets per source point
 * @note No eigen option available at least for now
 * 
 * @param meshes BEM geometry
 * @param LU LU decomposition of BEM transfer matrix
 * @param pivots pivots from the decomposition
 * @param spos source positions, [M x 3]
 * @param averef use average reference?  defaults to true
 * @return Lead field matrix
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFMLU_Phi_LG_GPU(const std::vector<Mesh<float> > &meshes, // BEM geometry
                           const Eigen::MatrixXf &LU, // LU decomposition of BEM transfer matrix
                           Eigen::VectorXi &pivots, // pivots from the decomposition
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           bool averef = true, // use average reference? 1 for yes, 2 for no
                           bool use_turbo = false 
);