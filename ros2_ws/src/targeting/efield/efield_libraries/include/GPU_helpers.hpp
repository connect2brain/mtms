#pragma once

#include <cuda_runtime.h>
#include <cublas_v2.h>
#include "eigen_defines.hpp"
#include <iostream>
#include "Coil.hpp"
#include "Mesh.hpp"
#include "MatrixGPU.hpp"
#include "VectorGPU.hpp"
#include "CoilGPU.hpp"
#include "MeshGPU.hpp"
#include <vector>

/**
 * @file GPU_helpers.hpp
 * @brief This file collects a bunch of utility functions used in TMS_GPU class and Overloads for basic field calculation functions with Eigen interface (Same name, but have to be in separate file not compiled with nvcc)
 * @note DO NOT INCLUDE IN CU OR CUH FILES
 */



/**
 * @brief Construct a MeshGPU object from Mesh object
 */
template <typename T>
MeshGPU<T> createMeshGPU(const Mesh<T> &mesh);
/**
 * @brief Construct a vector of MeshGPU objects from vector of Mesh objects
 */
template <typename T>
std::vector<MeshGPU<T>> createMeshGPU(const std::vector<Mesh<T>> &meshes);
/**
 * @brief Free vector of Meshes with one command
 */
template <typename T>
void free(std::vector<MeshGPU<T>> &meshesGPU);


/**
 * @brief Create a CoilGPU object from a Coil on CPU
 * @note %CoilGPU object created with this function must be explicitly freed by calling free(coilGPUobject)
 * 
 * @param coil coil object on CPU
 * @return coil object on GPU
 */
template <typename T>
CoilGPU<T> createCoilGPU(const Coil<T> &coil);


/**
 * @brief Calculates the product of matrix A transposed with vector x and adds it to y on GPU. Multiplies the result by mul \n 
 *        y = mul * (A' * x + y)
 * @details Calculates the product on columns of A specified by an indexlist
 * @note Writes and reads y pointer linearly meaning that it (cortex points as y) has to be filtered beforehand accoridng to the indlist in TMS calculation
 * @note Does not check any array boundaries and assumes that the indices in indlist are not out of bounds
 * 
 * 
 * @param cublashandle Instance of cublas library. Has to be initialized beforehand with cublasCreate() which takes fair amount of time
 * @param d_A GPU pointer to the col-major matrix A [rows x (> N_inds x 3) ]
 * @param d_x GPU pointer to the vector x [rows]
 * @param d_y GPU pointer to the vector y [N_inds x 3]
 * @param mul Constant multiplier
 * @param rows Number of rows
 * @param indlist Pointer to the indexlist (note, CPU pointer)
 * @param N_inds Number of indices in the list
 */
void d_SaxpyT_GPU_indlist_xyz(cublasHandle_t cublashandle, const float *d_A, const float *d_x, float *d_y, const float mul, const int32_t rows, const int32_t* indlist, int32_t N_inds);

/**
 * @brief As d_SaxpyT_GPU_indlist_xyz but assumes that indices point to single components/columns of A instead of xyz triplets
 *
 */
void d_SaxpyT_GPU_indlist_dir(cublasHandle_t cublashandle, const float *d_A, const float *d_x, float *d_y, const float mul, const int32_t rows, const int32_t* indlist, int32_t N_inds);

/**
 * @brief As d_SaxpyT_GPU_indlist_xyz above but instead of accumulating on d_y overwrites it with the product y = mul*A'x
 */
void d_Multiply_GPU_indlist_xyz(cublasHandle_t cublashandle, const float *d_A, const float *d_x, float *d_y, const float mul, const int32_t rows, const int32_t* indlist, int32_t N_inds); 

/**
 * @brief As d_Multiply_GPU_indlist_xyz but assumes that indices point to single components/columns of A instead of xyz triplets
 *
 */
void d_Multiply_GPU_indlist_dir(cublasHandle_t cublashandle, const float *d_A, const float *d_x, float *d_y, const float mul, const int32_t rows, const int32_t* indlist, int32_t N_inds); 

/* These interfaces seem abit dangerous as they allocate GPU memory inside the functions
MatrixGPU<float, Rowmajor> d_TMS_GPU_PrimaryField(const CoilGPU<float> &coil, const MatrixGPU<float, Rowmajor> &cortex, int32_t ROI_start, int32_t ROI_n, cudaStream_t stream = NULL);
MatrixGPU<float, Rowmajor> d_TMS_GPU_PrimaryField(const CoilGPU<float> &coil, const MatrixGPU<float, Rowmajor> &cortex, cudaStream_t stream = NULL);
VectorGPU<float>  d_TMS_GPU_SecondaryField(const CoilGPU<float> &coil, const MatrixGPU<float, Rowmajor> &bdpos , const MatrixGPU<float, Rowmajor> &bdmom, cudaStream_t stream = NULL);
*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
                    EIGEN OVERLOADS FOR FIELD CALCULATIONS
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * \addtogroup  Cuda_p_field
 * @{
 */
/*
MatrixX3f_RM BpFlux_xyz_GPU(const Coil<float> &coil, const MatrixX3f_RM &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM BpFlux_xyz_GPU(const Coil<float> &coil, const MatrixX3f_RM &cortex);
MatrixX3f_RM BpFlux_xyz_GPU(const Coil<float> &coil, const Mesh<float> &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM BpFlux_xyz_GPU(const Coil<float> &coil, const Mesh<float> &cortex);

MatrixX3f_RM A_WirePath_GPU(const Coil<float> &coil, const MatrixX3f_RM &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM A_WirePath_GPU(const Coil<float> &coil, const MatrixX3f_RM &cortex);
MatrixX3f_RM A_WirePath_GPU(const Coil<float> &coil, const Mesh<float> &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM A_WirePath_GPU(const Coil<float> &coil, const Mesh<float> &cortex);

MatrixX3f_RM A_CurrentDipoleSet_GPU(const Coil<float> &coil, const MatrixX3f_RM &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM A_CurrentDipoleSet_GPU(const Coil<float> &coil, const MatrixX3f_RM &cortex);
MatrixX3f_RM A_CurrentDipoleSet_GPU(const Coil<float> &coil, const Mesh<float> &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM A_CurrentDipoleSet_GPU(const Coil<float> &coil, const Mesh<float> &cortex);*/
MatrixX3f_RM TMS_GPU_PrimaryField(const Coil<float> &coil, const MatrixX3f_RM &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM TMS_GPU_PrimaryField(const Coil<float> &coil, const MatrixX3f_RM &cortex);
MatrixX3f_RM TMS_GPU_PrimaryField(const Coil<float> &coil, const Mesh<float> &cortex, const int32_t ROI_start, const int32_t ROI_n);
MatrixX3f_RM TMS_GPU_PrimaryField(const Coil<float> &coil, const Mesh<float> &cortex);
/**@}*/



/**
 * @brief Helper function for automatically choosing the right \b primary field calculation based on the Coiltype CT. Uses GPU pointers. 
 * @details For details on the interface see BpFlux_xyz_GPU.cuh or A_CurrentDipoleSet_GPU.cuh
 * @ingroup cuda_p_field
 */
void d_TMS_GPU_PrimaryField  (float *d_Ap,  float *d_field, int32_t ROI_n, float *d_sources, int32_t Nsp, float *d_moments, cudaStream_t stream, CoilType CT);
/**
 * @brief MatrixGPU interface for d_TMS_GPU_PrimaryField 
 * @ingroup cuda_p_field
 */
void d_TMS_GPU_PrimaryField(MatrixGPU<float, Rowmajor> &d_Ap, const MatrixGPU<float, Rowmajor> &d_cortex, const CoilGPU<float> &d_coil, cudaStream_t stream);

/**
 * @brief Helper function for automatically choosing the right \b secondary field calculation based on the Coiltype CT. Uses GPU pointers. 
 * @details For details on the interface see BetaQ_GPU.cuh or AlphaQ_GPU.cuh
 * @note Equivalent to dot product of d_TMS_GPU_PrimaryField with bdmom
 * @ingroup cuda_s_field
 */
void d_TMS_GPU_SecondaryField(float *d_ABQ, float *d_bdpos, float *d_bdmom, int32_t Nfp, float *d_sources, int32_t Nsp, float *d_moments, cudaStream_t stream, CoilType CT);
/**
 * @brief MatrixGPU interface for d_TMS_GPU_SecondaryField 
 * @ingroup cuda_p_field
 */
void d_TMS_GPU_SecondaryField(VectorGPU<float> &d_ABQ, const MatrixGPU<float, Rowmajor> &d_bdpos, const MatrixGPU<float, Rowmajor> &d_bdmom, const CoilGPU<float> &d_coil, cudaStream_t stream);

/**
 * @brief Saves a MatrixGPU to a file without need to seaparately call memory tranfer
 * @ingroup util
 */
template<typename T>
void WriteMatrixGPU(const MatrixGPU<T, Colmajor> &A, std::string filename){
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> tmp(A.rows, A.cols);
    copyFromGPU(A, tmp.data());
    tmp.WriteToFile(filename);
}
/**
 * @brief Saves a MatrixGPU to a file without need to seaparately call memory tranfer
 * @ingroup util
 */
template<typename T>
void WriteMatrixGPU(const MatrixGPU<T, Rowmajor> &A, std::string filename){
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> tmp(A.rows, A.cols);
    copyFromGPU(A, tmp.data());
    tmp.WriteToFile(filename);
}