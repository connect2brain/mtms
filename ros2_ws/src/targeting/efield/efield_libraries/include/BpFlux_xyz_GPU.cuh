#pragma once

#include <stdint.h>
#include <cuda_runtime.h>


/**
 * @ingroup Cuda_p_field
 * @brief Calculates the Magnetic flux though a coil caused by unit magnetic dipole triplets at positions specified by the sourcemesh, separately for each triplet  
 * @note excludes the factor of mu0/4pi=1e-7
 * @param d_Bp GPU pointer to a preallocated array for the result
 * @param d_fpos GPU pointer to an array of field positions
 * @param Nfp  Number of field points
 * @param d_cpos GPU pointer to an array of coil dipole positions
 * @param Ncp Number of coil points (source points)
 * @param d_cmom GPU pointer to an array of coil dipole moments
 * @param stream optional cudastream object, defaults to NULL for basic usage
 */
void d_BpFlux_xyz_GPU(float *d_Bp, float *d_fpos, int32_t Nfp, float *d_cpos, int32_t Ncp, float *d_cmom, cudaStream_t stream = NULL);


/**
 * @ingroup Cuda_p_field
 * @brief CPU pointer version for d_BpFlux_xyz_GPU. Handles all communication with the GPU. Reserves and frees GPU memory on each call to the function.
 * @note When calling the function multiple times on the same (large) set of sourcepoints, consider using the GPU pointer version for better performance
 */
void BpFlux_xyz_GPU(float *Bp, const float *fpos, const int32_t Nfp, const float * cpos, const float * cmom, const int32_t Ncp);


