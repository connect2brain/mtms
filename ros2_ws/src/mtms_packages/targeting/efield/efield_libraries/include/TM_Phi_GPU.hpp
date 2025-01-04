#pragma once

#include "eigen_defines.hpp"

#include "TMinv_Phi_LC.hpp"
#include "TMinv_Phi_LG.hpp"
// TM calculation without inverting or factoring the matrix is very fast and does not benefit from using GPU. 
// For completeness we can define an alias such that calling the function with _GPU suffix calls the CPU version without copy pasting the whole code

// This seems very complicated due to the need for default parameter

auto TMinv_Phi_LC_GPU = []( const std::vector<Eigen::MatrixXf> &D,
                            const std::vector<float> &ci,
                            const std::vector<float> &co, 
                            int32_t zerolevel = -1)
{
    printf("Note: all computation is done on CPU in TMinv_Phi_LC_GPU\nIt is the same function as TMinv_Phi_LC and the GPU version will be removed in future version\n");
    return TMinv_Phi_LC(D,ci,co,zerolevel);
};

auto TMinv_Phi_LG_GPU = []( const std::vector<Eigen::MatrixXf> &D,
                            const std::vector<Eigen::SparseMatrix<float> > &A,
                            const std::vector<float> &ci,
                            const std::vector<float> &co, 
                            int32_t zerolevel = -1)
{
    printf("Note: all computation is done on CPU in TMinv_Phi_LG_GPU\nIt is the same function as TMinv_Phi_LG and the GPU version will be removed in future version\n");
    return TMinv_Phi_LG(D,A,ci,co,zerolevel);
};

/**
 * @brief Builds the transfer matrix of potential on \b GPU using Linear Collocation(LC), returns LU decomposition
 * @note Use together with the LFMLU_Phi_LC function
 * @param D D operator from BEMOperatorsPhi_LC.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param pivots Empty pivot vector for saving the permutations. (can be uninitialized, will be resized) The modified pivot vector is later needed with LFMLU_Phi_LC for solving the system
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return LU decomposition of the transfer matrix
 * @ingroup tm_group
 */
Eigen::MatrixXf TMLU_Phi_LC_GPU(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          Eigen::VectorXi &pivots, // pivots for the LU decomposition
                          int32_t zerolevel = -1 // index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
);

/**
 * @brief Builds the transfer matrix of potential on \b GPU using Linear Galerkin (LG), returns LU decomposition
 * @note Use together with the LFMLU_Phi_LG function
 * @param D D operator from BEMOperatorsPhi_D_LG.hpp
 * @param A A operator from BEMOperatorsPhi_A_LG.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param pivots Empty pivot vector for saving the permutations. (can be uninitialized, will be resized) The modified pivot vector is later needed with LFMLU_Phi_LC for solving the system
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return LU decomposition of the transfer matrix
 * @ingroup tm_group
 */
Eigen::MatrixXf TMLU_Phi_LG_GPU(const std::vector<Eigen::MatrixXf> &D, 
                                const std::vector<Eigen::SparseMatrix<float> > &A,
                                const std::vector<float> &ci, 
                                const std::vector<float> &co, 
                                Eigen::VectorXi &pivots, 
                                int32_t zerolevel = -1);

/**
 * @brief Builds the transfer matrix of potential on \b GPU using Linear Collocation (LC), inverts the result
 * @note Use together with the LFM_Phi_LC function
 * @param D D operator from BEMOperatorsPhi_LC.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return inverted Transfer matrix 
 * @ingroup tm_group
 */
Eigen::MatrixXf TM_Phi_LC_GPU(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          int32_t zerolevel = -1 // index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
);


/**
 * @brief Builds the transfer matrix of potential on \b GPU using Linear Galerkin (LG), inverts the result
 * @note Use together with the LFM_Phi_LG function
 * @param D D operator from BEMOperatorsPhi_D_LG.hpp
 * @param A A operator from BEMOperatorsPhi_A_LG.hpp
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param zerolevel Index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
 * @return inverted Transfer matrix 
 * @ingroup tm_group
 */
Eigen::MatrixXf TM_Phi_LG_GPU(const std::vector<Eigen::MatrixXf> &D, // D-matrices, col-major ordering
                          const std::vector<Eigen::SparseMatrix<float> > &A, // A-matrices
                          const std::vector<float> &ci, // conductivities inside
                          const std::vector<float> &co, // conductivities outside
                          int32_t zerolevel = -1 // index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
);





