#pragma once

#include "cudaCheck.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

/**
 * @brief storage orderings for MatrixGPU
 * @note These need to be distinguished from Eigen::RowMajor and Eigen::ColMajor
 */
enum StorageOrder {
    Colmajor, Rowmajor
}; 


/**
 * @brief A simple struct for representing a matrix in GPU memory
 * 
 * @tparam T type parameter:  float, double or int32_t
 * @tparam S storage order: Colmajor or Rowmajor
 */
template<typename T, StorageOrder S>
struct MatrixGPU {
    int32_t rows, cols; /**<  Number of rows and columns */
    T *data;  /**<  Pointer to GPU data*/
};

// Some helper functions for working with the structs


/**
 * @brief Initialize a MatrixGPU of given size and allocate memory
 * 
 * @param rows 
 * @param cols 
 * @return MatrixGPU<T, S> , has to be freed explicitly
 */
template<typename T, StorageOrder S>
MatrixGPU<T, S> MatrixGPU_malloc(int32_t rows, int32_t cols) {
    MatrixGPU<T, S> A{rows, cols, nullptr};
    CHECK(cudaMalloc((void **) &(A.data), rows * cols * sizeof(T)));
    return A;
}


/**
 * @brief Initialize a MatrixGPU of given size, allocate memory, and set memory to zeros
 * 
 * @param rows 
 * @param cols 
 * @return MatrixGPU<T, S> , has to be freed explicitly
 */
template<typename T, StorageOrder S>
MatrixGPU<T, S> MatrixGPU_calloc(int32_t rows, int32_t cols) {
    MatrixGPU<T, S> A{rows, cols, nullptr};
    CHECK(cudaMalloc((void **) &(A.data), rows * cols * sizeof(T)));
    CHECK(cudaMemset(A.data, 0, rows * cols * sizeof(T)));
    return A;
}


/**
 * @brief Free memory
 * 
 * @param A MatrixGPU to be freed
 */
template<typename T, StorageOrder S>
void free(MatrixGPU<T, S> &A) {
    CHECK(cudaFree(A.data));
}

/**
 * @brief Copy data to GPU
 * 
 * @param A Preallocated MatrixGPU struct for the GPU data (destination)
 * @param CPUptr Pointer to the CPU data
 */
template<typename T, StorageOrder S>
void copyToGPU(MatrixGPU<T, S> &A, const T *CPUptr) {
    CHECK(cudaMemcpy(A.data, CPUptr, A.rows * A.cols * sizeof(T), cudaMemcpyHostToDevice));
}

/**
 * @brief Copy data from GPU to CPU
 * 
 * @param A MatrixGPU with data to be copied
 * @param CPUptr Pointer to a preallocated CPU array
 */
template<typename T, StorageOrder S>
void copyFromGPU(const MatrixGPU<T, S> &A, T *CPUptr) {
    CHECK(cudaMemcpy(CPUptr, A.data, A.rows * A.cols * sizeof(T), cudaMemcpyDeviceToHost));
}




/**
 * @brief Create (=allocate and copy data) MatrixGPU based on existing data on CPU.
 * @note Be careful to give the right storage order as a template parameter (no conversions are made)!
 * @note Also make sure that the dimensions are right and that the matrix lies in a contiguous block of memory.
 * 
 * @param CPUptr Pointer to CPU data
 * @param rows 
 * @param cols 
 * @return MatrixGPU<T, S> , has to be freed explicitly
 */
template<typename T, StorageOrder S>
MatrixGPU<T, S> createMatrixGPU(const T *CPUptr, int32_t rows, int32_t cols) {
    MatrixGPU<T, S> A = MatrixGPU_malloc<T, S>(rows, cols);
    copyToGPU(A, CPUptr);
    return A;
}


// Coefficient accessors

/**
 * @brief Extract a pointer to an element at (i,j)
 * 
 * @param A MatrixGPU struct
 * @param i Row index
 * @param j Column index
 * @return Pointer to the i,j element
 */
template<typename T>
inline __device__ T *getPtr(MatrixGPU<T, Colmajor> A, int32_t i, int32_t j) { return A.data + i + A.rows * j; }

template<typename T>
inline __device__ T *getPtr(MatrixGPU<T, Rowmajor> A, int32_t i, int32_t j) { return A.data + A.cols * i + j; }


/**
 * @brief Extract the value of the element at (i,j)
 * 
 * @param A MatrixGPU struct
 * @param i Row index
 * @param j Column index
 * @return Pointer to the i,j element
 */
template<typename T>
inline __device__ T getValue(MatrixGPU<T, Colmajor> A, int32_t i, int32_t j) { return A.data[i + A.rows * j]; }

template<typename T>
inline __device__ T getValue(MatrixGPU<T, Rowmajor> A, int32_t i, int32_t j) { return A.data[A.cols * i + j]; }



// helper functions to directly print matrices from GPU memory


/**
 * @brief Prints first 6x6 block of a MatrixGPU (Col major)
 * @ingroup util
 */
template<typename T>
void ShowGPU(const MatrixGPU<T, Colmajor> &A){
    int32_t rowlim = (A.rows > 6) ? 6: A.rows ;
    int32_t collim = (A.cols > 6) ? 6: A.cols ;
    if((A.cols > 6) || (A.rows > 6)){
        std::cout<< "Showing the first ("<<rowlim  << ", "<< collim <<") block of the matrix with total dimensions ("<<A.rows << ", "<<A.cols<< ")."<<std::endl;
    }
    std::vector<T> Acpu(rowlim*collim);
    CHECK(cudaMemcpy2D(Acpu.data(),rowlim*sizeof(T), A.data, A.rows*sizeof(T),rowlim*sizeof(T),collim ,cudaMemcpyDeviceToHost));
    
    for(int32_t i = 0; i < rowlim; i++){
        for(int32_t j = 0; j < collim; j++){
           std::cout<< Acpu[j*rowlim + i] << " ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

/**
 * @brief Prints first 6x6 block of a MatrixGPU (Row major)
 * @ingroup util
 */
template<typename T>
void ShowGPU(const MatrixGPU<T, Rowmajor> &A){
    int32_t rowlim = (A.rows > 6) ? 6: A.rows ;
    int32_t collim = (A.cols > 6) ? 6: A.cols ;
    if((A.cols > 6) || (A.rows > 6)){
        std::cout<< "Showing the first ("<<rowlim  << ", "<< collim <<") block of the matrix with total dimensions ("<<A.rows << ", "<<A.cols<< ")."<<std::endl;
    }
    std::vector<T> Acpu(rowlim*collim);
    CHECK(cudaMemcpy2D(Acpu.data(), collim*sizeof(T), A.data, A.cols*sizeof(T), collim*sizeof(T), rowlim ,cudaMemcpyDeviceToHost));
    
    for(int32_t i = 0; i < rowlim; i++){
        for(int32_t j = 0; j < collim; j++){
           std::cout<< Acpu[i*collim + j] << " ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

/**
 * @brief Prints a block of a MatrixGPU specified by indices (Col major)
 * 
 * @param row_start starting row
 * @param col_start starting column
 * @param rows  Number of rows
 * @param cols  Number of columns
 * @ingroup util
 */
template<typename T>
void ShowBlockGPU(const MatrixGPU<T, Colmajor> &A, int32_t row_start, int32_t col_start, int32_t rows, int32_t cols){
    if (row_start < 0 || col_start < 0 || rows < 0 || cols < 0) {
        throw std::invalid_argument("Block indices and sizes should be positive");
    }
    if (row_start + rows > A.rows || col_start + cols > A.cols) {
        throw std::invalid_argument("Some block indices are out of range of the original matrix.");
    }
    std::vector<T> Acpu(rows*cols);
    CHECK(cudaMemcpy2D(Acpu.data(),rows*sizeof(T), A.data + A.rows*col_start + row_start, A.rows*sizeof(T), rows*sizeof(T),cols,cudaMemcpyDeviceToHost));
    
    for(int32_t i = 0; i < rows; i++){
        for(int32_t j = 0; j < cols; j++){
           std::cout<< Acpu[j*rows + i]<< " ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

/**
 * @brief Prints a block of a MatrixGPU specified by indices (Row major)
 * 
 * @param row_start starting row
 * @param col_start starting column
 * @param rows  Number of rows
 * @param cols  Number of columns
 * @ingroup util
 */
template<typename T>
void ShowBlockGPU(const MatrixGPU<T, Rowmajor> &A, int32_t row_start, int32_t col_start, int32_t rows, int32_t cols){
    if (row_start < 0 || col_start < 0 || rows < 0 || cols < 0) {
        throw std::invalid_argument("Block indices and sizes should be positive");
    }
    if (row_start + rows > A.rows || col_start + cols > A.cols) {
        throw std::invalid_argument("Some block indices are out of range of the original matrix.");
    }
    std::vector<T> Acpu(rows*cols);
    CHECK(cudaMemcpy2D(Acpu.data(), cols*sizeof(T), A.data + A.cols*row_start + col_start, A.cols*sizeof(T), cols*sizeof(T), rows ,cudaMemcpyDeviceToHost));
    
    for(int32_t i = 0; i < rows; i++){
        for(int32_t j = 0; j < cols; j++){
           std::cout<< Acpu[i*cols + j]<< " ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}