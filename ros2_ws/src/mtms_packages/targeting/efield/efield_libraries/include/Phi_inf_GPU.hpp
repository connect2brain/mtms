#pragma once

#include "Mesh.hpp"
#include "MatrixGPU.hpp"



/**
 * @brief  Eigen interface for Phi_inf_LG_GPU. Checks for available GPU memory and uses appropriate method accordingly
 */
Eigen::MatrixXf Phi_inf_LG_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos,
                                const MatrixX3f_RM &sdir, 
                                bool use_turbo = false 
);
/**
 * @brief  Eigen interface for Phi_inf_LG_GPU. Checks for available GPU memory and uses appropriate method accordingly
 * 
 */
Eigen::MatrixXf Phi_inf_LG_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos,
                                bool use_turbo = false
);


/**
 * @brief  Eigen interface for Phi_inf_LC_GPU. Checks for available GPU memory and uses appropriate method accordingly
 */
Eigen::MatrixXf Phi_inf_LC_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos,
                                const MatrixX3f_RM &sdir
);
/**
 * @brief  Eigen interface for Phi_inf_LC_GPU. Checks for available GPU memory and uses appropriate method accordingly
 */
Eigen::MatrixXf Phi_inf_LC_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos
);


///////////////////////////////////////////////////////////////////////////


/**
 * @brief  Mixed interface for internal use.
 * @warning Does not check if there is enough GPU memory available. Phiinf on GPU must be preallocated
 */
void Phi_inf_LG_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos,
                                const MatrixX3f_RM &sdir,
                                MatrixGPU<float, Colmajor> d_Phiinf, 
                                bool use_turbo = false
);
/**
 * @brief  Mixed interface for internal use.
 * @warning Does not check if there is enough GPU memory available. Phiinf on GPU must be preallocated
 */
void Phi_inf_LG_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos,
                                MatrixGPU<float, Colmajor> d_Phiinf,
                                bool use_turbo = false
);


/**
 * @brief  Mixed interface for internal use.
 * @warning Does not check if there is enough GPU memory available. Phiinf on GPU must be preallocated
 */
void Phi_inf_LC_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos,
                                const MatrixX3f_RM &sdir,
                                MatrixGPU<float, Colmajor> d_Phiinf
);
/**
 * @brief  Mixed interface for internal use.
 * @warning Does not check if there is enough GPU memory available. Phiinf on GPU must be preallocated
 */
void Phi_inf_LC_GPU( const std::vector<Mesh<float> > &meshes, 
                                const MatrixX3f_RM &spos,
                                MatrixGPU<float, Colmajor> d_Phiinf
);