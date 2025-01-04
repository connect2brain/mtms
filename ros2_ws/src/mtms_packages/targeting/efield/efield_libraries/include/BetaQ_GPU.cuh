#pragma once

#include <cuda_runtime.h>
#include <stdint.h>


/**
 * @ingroup Cuda_s_field
 * @brief Implements the approximate Beta integrals used to calculate the secondary magnetic field due to magnetic dipole coil. 
 * @details Result is used to calculate secondary E-field when multiplied with Potential Matrix Phi. See Eq. 8 in Stenroos & Koponen, Real-time computation of the TMS-induced electric field in a realistic head model, NeuroImage, 2019 Dec; 203:116159, doi: 10.1016/j.neuroimage.2019.116159.
 * 
 * @param d_BQ GPU pointer to a preallocated array for the result
 * @param d_bdpos GPU pointer to an array of BetaDipoles positions
 * @param d_bdmom GPU pointer to an array of BetaDipoles moments
 * @param Nfp Number of field points (%BetaDipoles)
 * @param d_cpos GPU pointer to an array of discretized coil positions
 * @param Ncp Number of source points (coil points)
 * @param d_cmom GPU pointer to an array of discretized coil moments
 * @param stream optional cudastream object, defaults to NULL for basic usage
 */
void d_BetaQ_GPU(float *d_BQ, float *d_bdpos, float *d_bdmom, int32_t Nfp, float *d_cpos, int32_t Ncp, float *d_cmom, cudaStream_t stream = NULL);
