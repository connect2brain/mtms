#pragma once

#include "cudaCheck.hpp"
#include <cstdint>

/**
 * @brief Like MatrixGPU, but 1-D array
 * 
 */
template<typename T>
struct VectorGPU {
    int32_t length;
    T *data;
};


/**
 * @brief Initialize a VectorGPU of given size and allocate memory
 * 
 */
template<typename T>
VectorGPU<T> VectorGPU_malloc(int32_t length) {
    VectorGPU<T> A{length, nullptr};
    CHECK(cudaMalloc((void **) &(A.data), length * sizeof(T)));
    return A;
}

/**
 * @brief Initialize a VectorGPU of given size, allocate memory, and set memory to zeros
 * 
 */
template<typename T>
VectorGPU<T> VectorGPU_calloc(int32_t length) {
    VectorGPU<T> A{length, nullptr};
    CHECK(cudaMalloc((void **) &(A.data), length * sizeof(T)));
    CHECK(cudaMemset(A.data, 0, length * sizeof(T)));
    return A;
}

/**
 * @brief Free memory
 * 
 */
template<typename T>
void free(VectorGPU<T> &A) {
    CHECK(cudaFree(A.data));
}

/**
 * @brief Copy data to A.data from CPUptr
 * 
 */
template<typename T>
void copyToGPU(VectorGPU<T> &A, const T *CPUptr) {
    CHECK(cudaMemcpy(A.data, CPUptr, A.length * sizeof(T), cudaMemcpyHostToDevice));
}

/**
 * @brief Copy data from A.data to CPUptr
 * 
 */
template<typename T>
void copyFromGPU(const VectorGPU<T> &A, T *CPUptr) {
    CHECK(cudaMemcpy(CPUptr, A.data, A.length * sizeof(T), cudaMemcpyDeviceToHost));
}

/**
 * @brief Create (=allocate and copy data) VectorGPU based on existing data on CPU.
 * 
 */
template<typename T>
VectorGPU<T> createVectorGPU(const T *CPUptr, int32_t length) {
    VectorGPU<T> vec = VectorGPU_malloc<T>(length);
    copyToGPU(vec, CPUptr);
    return vec;
}