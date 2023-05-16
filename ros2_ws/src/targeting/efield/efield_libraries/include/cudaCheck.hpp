#pragma once

#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <cusolverDn.h>

// Checks success of a CUDA function. If an error arises, prints error to
// stdout and terminates execution.
void check(cudaError_t err, const char* context);
void check(cublasStatus_t err, const char *context);
void check(cusolverStatus_t err, const char *context);

#define CHECK(x) check(x, #x)

// How much GPU memory is manually left for the OS and other unexpected needs when allocating 
// We need this as the available memory "oscillates", see teams BIOEM Notebook Problem: GPU memory "oscillations" 
#define GPU_MEMORY_MARGIN ((size_t) 200*1024*1024) 

// How much memory is needed in GPU LU factorization compared to the size of the original matrix 
// See plot and test script in tms_cpp/tests/LUMemory_test. 
// With large matrices (n > 8000) about twice the memory of original matrix is needed
// If using small matrices, consider using LU_MemNeed(n) as the multiplier to check exact memory need on each call to LU/SGETRF
#define LU_MEMORY_MULTIPLIER 2.0

//#define CUSOLVER_HANDLE_MEM ((size_t) 500*1024*1024)


/**
 * @brief Get available GPU memory 
 * 
 * @return size_t 
 */
size_t CudaMemAvail();
/**
 * @brief Get available GPU memory after loading cusolver
 * 
 * @return size_t 
 */
size_t CudaMemAvailCUSOLVER();
/**
 * @brief Get available GPU memory after loading cublas
 * 
 * @return size_t 
 */
size_t CudaMemAvailCUBLAS();

/**
 * @brief Calculate the needed extra GPU memory space (in bytes) for factoring an n-by-n matrix on GPU. Does not include the memory needed for the matrix itself
 * 
 * @return size_t 
 */
size_t LU_MemNeed(int32_t n);
