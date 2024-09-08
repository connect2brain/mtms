#pragma once

#include <cuda_runtime.h>
#include <stdint.h>


/**
 * @ingroup Cuda_p_field
 * @brief Calculates the primary A field on a set of field points due to a polyline coil on GPU
 * @note excludes the factor of mu0/4pi=1e-7
 * 
 * @param d_Ap GPU pointer to a preallocated array for the result
 * @param d_field GPU pointer to an array of field positions
 * @param Nfp Number of field points
 * @param d_cpos GPU pointer to an array of discretized coil positions
 * @param Ncp Number of source points (coil points)
 * @param stream optional cudastream object, defaults to NULL for basic usage
 */
void d_A_WirePath_GPU(float *d_Ap, float *d_field, int32_t Nfp, float *d_cpos, int32_t Ncp, cudaStream_t stream = NULL);



/**
 * @ingroup Cuda_p_field
 * @brief CPU pointer version for d_A_WirePath_GPU. Handles all communication with the GPU.  Reserves and frees GPU memory on each call to the function.
 * @note When calling the function multiple times on the same (large) set of sourcepoints, consider using the GPU pointer version for better performance
 */
void A_WirePath_GPU(float *A_field, const float *fpos, const int32_t Nfp, const float * cpos, const int32_t Ncp);





/**
 * @ingroup Cuda_s_field
 * @brief Calculates the dotproducts of BetaDipoles moments and the Coil induced A-field on the same dipole positions
 * @details Result is used to calculate secondary E-field when multiplied with Potential Matrix Phi. (similarly to the result of BetaQ and AlphaQ)
 * 
 * @param d_ABQ GPU pointer to a preallocated array for the result
 * @param d_bdpos GPU pointer to an array of BetaDipoles positions
 * @param d_bdmom GPU pointer to an array of BetaDipoles moments
 * @param Nfp Number of field points (%BetaDipoles)
 * @param d_cpos GPU pointer to an array of discretized coil positions
 * @param Ncp Number of source points (coil points)
 * @param stream optional cudastream object, defaults to NULL for basic usage
 */
void d_A_WirePath_Secondary_GPU(float *d_ABQ, float *d_bdpos, float *d_bdmom, int32_t Nfp, float *d_cpos, int32_t Ncp, cudaStream_t stream = NULL);
