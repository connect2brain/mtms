#pragma once
#include <cuda_runtime.h>
#include <stdint.h>

/**
 * @ingroup Cuda_p_field
 * @brief Calculates the primary vector potential A on a set of field points due to current dipole moments on GPU
 * @note excludes the factor of mu0/4pi=1e-7
 * @param d_Ap GPU pointer to a preallocated array for the result
 * @param d_field GPU pointer to an array of field positions
 * @param Nfp  Number of field points
 * @param d_cpos GPU pointer to an array of coil dipole positions
 * @param Ncp Number of coil points (source points)
 * @param d_cmom GPU pointer to an array of coil dipole moments
 * @param stream optional cudastream object, defaults to NULL for basic usage
 */
void d_A_CurrentDipoleSet_GPU(float *d_Ap, float *d_field, int32_t Nfp, float *d_cpos, int32_t Ncp, float *d_cmom, cudaStream_t stream = NULL);



/**
 * @ingroup Cuda_p_field
 * @brief CPU pointer version for d_A_CurrentDipoleSet_GPU. Handles all communication with the GPU.  Reserves and frees GPU memory on each call to the function.
 * @note When calling the function multiple times on the same (large) set of sourcepoints, consider using the GPU pointer version for better performance
 */
void A_CurrentDipoleSet_GPU(float *A_field, const float *fpos, const int32_t Nfp, const float * cpos, const float * cmom, const int32_t Ncp);