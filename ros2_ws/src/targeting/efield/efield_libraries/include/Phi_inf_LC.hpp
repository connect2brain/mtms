#pragma once

#include "eigen_defines.hpp"
#include "Mesh.hpp"





/**
 * @brief Compute the electric potential due to current dipoles in infinite homogeneous volume conductor of unit conductivity, using LC.
 * @details The third argument specifies the dipole moments and can be one of the following: \n 
 *  - [(number of source positions) X 3] matrix => a separate dipole moment is specified for each position \n 
 *  - nothing => 3 unit dipoles (x, y and z-oriented) are placed in each position
 * 
 * @param fp field points, [N x 3]
 * @param spos source positions, [M x 3]
 * @param sdir source dipole moments [M x 3]
 * @return [N X M] matrix: Phi_{i,j} = potential of dipole j at field mesh vertex i
 */
Eigen::MatrixXf Phi_inf_LC(const MatrixX3f_RM &fp, // field points, [N x 3]
                           const MatrixX3f_RM &spos, // source positions, [M x 3]
                           const MatrixX3f_RM &sdir // source dipole moments [M x 3]
);

/**
 * @brief Compute the electric potential due to current dipoles in infinite homogeneous volume conductor of unit conductivity, using LC.
 * @details 3 unit dipoles (x, y and z-oriented) are placed in each position
 * 
 * @param fp field points, [N x 3]
 * @param spos source positions, [M x 3]
 * @return [N X 3M] matrix: Phi_{i,j} = potential of dipole j at field mesh vertex i
 */
Eigen::MatrixXf Phi_inf_LC(const MatrixX3f_RM &fp,
                           const MatrixX3f_RM &spos
);

/**
 * @brief Overloads of the above for multiple meshes
 */
Eigen::MatrixXf Phi_inf_LC(const std::vector<Mesh<float>> &meshes,
                           const MatrixX3f_RM &spos,
                           const MatrixX3f_RM &sdir);
/**
 * @brief Overloads of the above for multiple meshes
 */
Eigen::MatrixXf Phi_inf_LC(const std::vector<Mesh<float>> &meshes,
                           const MatrixX3f_RM &spos);