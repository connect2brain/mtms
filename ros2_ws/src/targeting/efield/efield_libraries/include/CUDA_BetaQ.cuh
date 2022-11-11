#pragma once

#include <cuda_runtime.h>

/*
Implements the approximate Beta integrals used to calculate the secondary magnetic field due to magnetic dipole coil. 

Parameters are pointers to GPU memory
*/


void d_CUDA_BetaQ(float *d_BQ, float *d_field, float *d_moments, int32_t Nfp, float *d_sources, int32_t Nsp, float *d_normals, cudaStream_t stream);