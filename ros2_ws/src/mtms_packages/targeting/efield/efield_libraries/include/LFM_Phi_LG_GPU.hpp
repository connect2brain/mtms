//
// Created by kalle on 17.3.2022.
//

#pragma once

#include "Mesh.hpp"

/**
 * @file LFM_Phi_LG_GPU.hpp
 * @brief GPU versions of LFM_Phi_LG. similar interfaces
 */

/**
 * @ingroup lfm_group
 */
Eigen::MatrixXf LFM_Phi_LG_GPU( const std::vector<Mesh<float> > &meshes, 
                                const Eigen::MatrixXf &Tphi, 
                                const MatrixX3f_RM &spos,
                                const MatrixX3f_RM &sdir, 
                                bool averef = true, 
                                bool use_turbo = false 
);
/**
 * @ingroup lfm_group
 * @note Checks for available GPU memory and either does calculation one mesh at a time or everything at once
 */
Eigen::MatrixXf LFM_Phi_LG_GPU( const std::vector<Mesh<float> > &meshes, 
                                const Eigen::MatrixXf &Tphi, 
                                const MatrixX3f_RM &spos,
                                bool averef = true,
                                bool use_turbo = false
);


