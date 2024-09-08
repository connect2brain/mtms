#pragma once

#include <cuda_runtime.h>
#include <stdint.h>



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
 * @param d_cmom GPU pointer to an array of discretized coil moments
 * @param stream optional cudastream object, defaults to NULL for basic usage
 */
void d_AlphaQ_GPU(float *d_ABQ, float *d_bdpos, float *d_bdmom, int32_t Nfp, float *d_cpos, int32_t Ncp, float *d_cmom, cudaStream_t stream = NULL);
