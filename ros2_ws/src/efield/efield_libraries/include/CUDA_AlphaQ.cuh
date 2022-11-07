#pragma once

#include <cuda_runtime.h>

/* 
Calculates the dotproducts of BetaQDipole moments and the coil induced A-field on the same dipole locations

Result is used to calculate secondary A-field when multiplied with Potential Matrix Phi


Arguments are GPU pointers. "stream" can be set to NULL for basic usage
*/
void d_CUDA_AlphaQ(float *d_ABQ, float *d_field, float *d_moments, int32_t Nfp, float *d_sources, int32_t Nsp, float *d_normals, cudaStream_t stream);
