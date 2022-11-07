#pragma once

#include <cuda_runtime.h>
#include <linalg>
#include <iostream>
#include <cublas_v2.h>
#include "Coil.hpp"


#ifndef BLOCK_SIZE
    #define BLOCK_SIZE 128
#endif


// Macro for checking success of CUDA functions. If an error arises, prints
// error to stdout and terminates execution.

inline void check(cudaError_t err, const char* context) {
  if (err != cudaSuccess) {
    std::cerr << "CUDA error: " << context << ": "
              << cudaGetErrorString(err) << std::endl;
    std::exit(EXIT_FAILURE);
  }
}


#ifndef CHECK
  #define CHECK(x) check(x, #x)
#endif


/*
Transfers [N x 3] coordinate array into GPU memory and adds a padding of 0 to each triplet 

Increases CUDA performance as global reads can are coalesced to 16 bytes (4 floats)
*/
template<typename T>
void memcopyWithPadding(T *d_pointer, const Matrix<T, RowMajor> &coords);

template<typename T>
void memcopyWithPadding(T *d_pointer, const Matrix<T, RowMajor> &coords, const ColVector<T> &vec);

/*
Transfers [N x 3] coordinate array into GPU memory and adds a padding of 0 to each triplet 

Increases CUDA performance as global reads can are coalesced to 16 bytes (4 floats)
*/
template<typename T>
void memcopyWithPadding(T *d_pointer, const T *coords, const int32_t rows);
/*
Transfers [N x 3] coordinate array into GPU memory and adds a padding described by "vec" [N] to each triplet

Increases CUDA performance as global reads can are coalesced to 16 bytes (4 floats)
*/
template<typename T>
void memcopyWithPadding(T *d_pointer, const T *coords, const T *vec, const int32_t rows);


// y = mul*(A'x + y)
void SaxpyT_GPU_indlist(cublasHandle_t cublashandle, const float *d_A, const float *d_x, float *d_y, const float mul, const int32_t rows, const ColVector<int32_t> indlist);
//As above but instead of accumulating on d_y overwrites it with the product y = mul*A'x
void Multiply_GPU_indlist(cublasHandle_t cublashandle, const float *d_A, const float *d_x, float *d_y, const float mul, const int32_t rows, const ColVector<int32_t> indlist); 


/*
Requires correct size allocated gpu memory for the filtered array
*/
void filterByIndices_GPU(const float* d_input, float* d_filtered, const int32_t cols, const ColVector<int32_t> indlist);

void TMS_GPU_PrimaryField  (float *d_Ap,  float *d_field, int32_t ROI_n, float *d_sources, int32_t Nsp, float *d_moments, cudaStream_t stream, CoilType CT);
void TMS_GPU_SecondaryField(float *d_ABQ, float *d_bdpos, float *d_bdmom, int32_t Nfp, float *d_sources, int32_t Nsp, float *d_moments, cudaStream_t stream, CoilType CT);


