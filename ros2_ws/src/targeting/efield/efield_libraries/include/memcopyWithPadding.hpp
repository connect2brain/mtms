#pragma once
#include <cuda_runtime.h>
#include "MatrixGPU.hpp"


/**
 * @brief Create a MatrixGPU from CPU data with extra padding on leading dimension (assumed to be row-major so padding adds an extra column)
 * @details padding the row size to a multiple of 4 increases CUDA performance as global reads are coalesced to 16 bytes (4 floats)
 * 
 * @param CPUptr Pointer to the CPU data
 * @param rows Number of rows 
 * @param cols Original number of columns, defaults to 3
 * @param padding Number of columns to be added, defaults to 1
 * @return padded GPU array as matrixGPU object, has to be freed later by the user
 */
template <typename T>
MatrixGPU<T,Rowmajor> createMatrixGPU_padded(const T* CPUptr, int32_t rows, int32_t cols = 3, int32_t padding = 1);

/**
 * @brief Allocates and transfers [N x 3] Rowmajor coordinate array into GPU memory and adds a padding of 0 to each triplet 
 * @note similar to above function, but less general and doesnt use MatrixGPU struct
 * @param d_pointer GPU pointer for the output padded array (has to be freed after)
 * @param coords Pointer to the CPU data
 * @param rows Number of rows
 */
template<typename T>
void memcopyWithPadding(T *d_pointer, const T *coords, const int32_t rows);

/**
 * @brief Has additional pointer parameter to an vector vec which is added as the padding instead of zeros
 * 
 * @param d_pointer GPU pointer for the output padded array (has to be freed after)
 * @param coords Pointer to the CPU data
 * @param vec Pointer to a vector, element of which will be added as the 4th element of each row instead of zeros
 * @param rows Number of rows
 */
template<typename T>
void memcopyWithPadding(T *d_pointer, const T *coords, const T *vec, const int32_t rows);
