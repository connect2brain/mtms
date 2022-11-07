//
// Created by Kalle Jyrkinen on 23.2.2022.
//

#ifndef TMS_CPP_MATRIXBASE_HPP
#define TMS_CPP_MATRIXBASE_HPP

#include <cstdint>
#include <string>
#include <fstream>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <random>
#include <algorithm>

#include "SparseMatrix.hpp"

// StorageOrder specifies the layout of a matrix in memory. ColMajor means that a single column lies in a contiguous
// memory block, RowMajor means the same for rows.
enum StorageOrder { ColMajor, RowMajor };

// Some forward declarations related to derived classes:
template <typename T, StorageOrder S> class Matrix;
template <typename T, StorageOrder S> class Block;


// An abstract base class for matrices. Essentially a lightweight wrapper around a block of memory, specifying additional
// parameters such as number of rows, number of columns etc. Also provides some useful methods (arithmetic etc.)
//
// "abstract" = "not meant to be instantiated directly"; see the derived classes Matrix, Vector and Block
template <typename T, StorageOrder S = ColMajor>
class MatrixBase {
public:
    // Constructors are declared at the end of the class body, in 'protected' section. This is because this is an
    // abstract base class, and we want to hide the constructors!

    // GETTERS

    int32_t Rows() const { return rows_; } // get number of rows
    int32_t Cols() const { return cols_; } // get number of columns
    int32_t Stride() const { return stride_; } // get stride
    const T* Data() const { return data_; } // get pointer to first element; this is called for const objects
    T* Data() { return data_; } // same as above, but this is called for non-const objects

    // OTHER METHODS

    // Access element at location (i,j). For example A(3,2) returns a reference to element at location (3,2) in matrix A
    const T& operator()(int32_t i, int32_t j) const; // this is used for const objects; the element cannot be changed via const reference
    T& operator()(int32_t i, int32_t j); // this is used for non-const objects and allows modifying the element

    // Multiply from right by another matrix. Returns a new column-major matrix; if you need a row-major matrix as output,
    // look into function 'multiply'. The other matrix must be of the same data type but not necessarily of same storage order.
    template <StorageOrder S2>
    Matrix<T, ColMajor> operator*(const MatrixBase<T, S2>& other) const;

    // Add with another matrix. Returns a new column-major matrix; if you need a row-major matrix as output,
    // look into function 'add'. The other matrix must be of the same data type but not necessarily of same storage order.
    template <StorageOrder S2>
    Matrix<T, ColMajor> operator+(const MatrixBase<T, S2>& other) const;

    void operator+=(T a);
    void operator*=(T a);
    void operator+=(const SparseMatrix<T>& s);

    // Often we need to call *= and then += (or the other way round)
    //   => grouping them together requires less memory accesses
    void ScaleAndIncrement(T a, T b); // A = a*A + b
    void IncrementAndScale(T a, T b); // A = (a+A) * b

    // randomizes the matrix entries with uniformly distributed reals between a and b
    void Randomize(T a, T b);

    // Transpose. Returns a new matrix.
    Matrix<T,S> Transpose() const;
    // Matrix<T,S> TransposeInPlace(); // TODO: implement this if needed

    // Inverse. Returns a new matrix.
    Matrix<T,S> Inverse() const;
    // In-place inverse
    void InverseInPlace();

    // Make a copy of another data type.
    // Example: Suppose A is a float matrix. We can create a double version with A.Cast<double>();
    template <typename T_out>
    Matrix<T_out, S> Cast() const;

    // Calculate and return the mean of all elements
    T Mean() const;
    // Calculate and return the root mean square of all elements
    T Rms() const;

    void Show() const;
    void ShowBlock(int32_t i, int32_t j, int32_t rows, int32_t cols) const;

    // Get a new matrix that references a specific block of this matrix. Does not create a copy!
    Block<T,S> GetBlock(int32_t i, int32_t j, int32_t rows, int32_t cols) const;
    // A special case of GetBlock for extracting a single row.
    Block<T,S> GetRow(int32_t i) const;
    // A special case of GetBlock for extracting a single column.
    Block<T,S> GetCol(int32_t i) const;

    // Write matrix to a binary file
    void WriteToFile(const std::string& filename) const;

    // Get the type ID, i.e. the integer representing the data type in binary files
    int32_t TypeID() const;

    // Get storage order
    bool IsColMajor() const;

protected:
    // Declaring constructors as protected prevents instantiating a MatrixBase directly
    // (relevant since it is an abstract base class)

    // Default constructor
    MatrixBase() : data_(nullptr), rows_(0), cols_(0), stride_(0) { }
    // Constructor initializing all private members to "meaningful" values
    MatrixBase(T* data, int32_t rows, int32_t cols, int32_t stride) : data_(data), rows_(rows), cols_(cols), stride_(stride) { }

    // MEMBER VARIABLES:

    T* data_; // pointer to element at (0,0)
    int32_t rows_; // number of rows
    int32_t cols_; // number of columns
    int32_t stride_; // number of memory locations between the beginnings of two rows (if RowMajor) or cols (if ColMajor)
};


namespace matrix_helpers { // declare a separate namespace for helper functions to reduce risk of name collisions etc
    template <StorageOrder S>
    inline int32_t getIndex(int32_t i, int32_t j, int32_t stride);
    template <>
    inline int32_t getIndex<ColMajor>(int32_t i, int32_t j, int32_t stride) { return i + j*stride; }
    template <>
    inline int32_t getIndex<RowMajor>(int32_t i, int32_t j, int32_t stride) { return i*stride + j; }

    template <StorageOrder S>
    inline bool isColMajor();
    template <>
    inline bool isColMajor<ColMajor>() { return true; }
    template <>
    inline bool isColMajor<RowMajor>() { return false; }

    template <typename T>
    inline int32_t typeID();
    template <>
    inline int32_t typeID<double>() { return 0; }
    template <>
    inline int32_t typeID<float>() { return 1; }
    template <>
    inline int32_t typeID<int32_t>() { return 2; }
}


/*---------------------------------------------------------------------------------*/
// METHOD IMPLEMENTATIONS


template<typename T, StorageOrder S>
inline const T& MatrixBase<T, S>::operator()(int32_t i, int32_t j) const {
    assert(i < rows_ && j < cols_);
    assert(i >= 0 && j >= 0);
    return data_[matrix_helpers::getIndex<S>(i, j, stride_)];
}

template<typename T, StorageOrder S>
inline T& MatrixBase<T, S>::operator()(int32_t i, int32_t j) {
    assert(i < rows_ && j < cols_);
    assert(i >= 0 && j >= 0);
    return data_[matrix_helpers::getIndex<S>(i, j, stride_)];
}

template <typename T, StorageOrder S1, StorageOrder S2, StorageOrder S3>
Matrix<T,S1> multiply(const MatrixBase<T,S2>& A, const MatrixBase<T,S3>& B); // implemented in MatrixBase.cpp

template<typename T, StorageOrder S>
template<StorageOrder S2>
Matrix<T, ColMajor> MatrixBase<T, S>::operator*(const MatrixBase <T, S2> &other) const {
    return multiply<T,ColMajor>(*this, other);
}


template <typename T, StorageOrder S1, StorageOrder S2, StorageOrder S3>
Matrix<T,S1> add(const MatrixBase<T,S2>& A, const MatrixBase<T,S3>& B) {
    if (A.Cols() != B.Cols() || A.Rows() != B.Rows()) {
        throw std::invalid_argument("Can't sum matrices of different sizes");
    }

    int32_t rows = A.Rows();
    int32_t cols = A.Cols();
    Matrix<T,S1> ApB(rows, cols);
    int32_t n_rowmajor = 0;
    if (!A.IsColMajor()) n_rowmajor++;
    if (!B.IsColMajor()) n_rowmajor++;
    if (!ApB.IsColMajor()) n_rowmajor++;

    if (n_rowmajor >= 2) { // if at least 2 out of 3 matrices are row-major, loop row-wise
        for (int32_t i = 0; i < rows; i++) {
            for (int32_t j = 0; j < cols; j++) {
                ApB(i,j) = A(i,j) + B(i,j);
            }
        }
    }
    else {
        for (int32_t j = 0; j < cols; j++) {
            for (int32_t i = 0; i < rows; i++) {
                ApB(i,j) = A(i,j) + B(i,j);
            }
        }
    }
    return ApB;
}

template<typename T, StorageOrder S>
template<StorageOrder S2>
Matrix<T, ColMajor> MatrixBase<T, S>::operator+(const MatrixBase <T, S2> &other) const {
    return add<T, ColMajor>(*this, other);
}

template<typename T, StorageOrder S>
Matrix<T, S> MatrixBase<T, S>::Transpose() const { // TODO: this is still a very naive implementation
    Matrix<T,S> M(cols_, rows_);
    for (int32_t i = 0; i < rows_; i++) {
        for (int32_t j = 0; j < cols_; j++) {
            M(j,i) = (*this)(i,j);
        }
    }
    return M;
}

template <typename T, StorageOrder S>
void MatrixBase<T,S>::operator+=(T a) {
    if (IsColMajor()) {
        for(int32_t j = 0; j < cols_; j++) {
            for(int32_t i = 0; i < rows_; i++) {
                (*this)(i,j) += a;
            }
        }
    }
    else {
        for(int32_t i = 0; i < rows_; i++) {
            for(int32_t j = 0; j < cols_; j++) {
                (*this)(i,j) += a;
            }
        }
    }
}


template <typename T, StorageOrder S>
void MatrixBase<T,S>::operator*=(T a) {
    if (IsColMajor()) {
#pragma omp parallel for
        for(int32_t j = 0; j < cols_; j++) {
            for(int32_t i = 0; i < rows_; i++) {
                (*this)(i,j) *= a;
            }
        }
    }
    else {
#pragma omp parallel for
        for(int32_t i = 0; i < rows_; i++) {
            for(int32_t j = 0; j < cols_; j++) {
                (*this)(i,j) *= a;
            }
        }
    }
}


template <typename T, StorageOrder S>
void MatrixBase<T, S>::operator+=(const SparseMatrix<T>& s) {
    for(int32_t i = 0; i < s.Capacity(); i++) {
        COOTriplet<T> tr = s(i);
        (*this)(tr.i, tr.j) += tr.value;
    }
}


template <typename T, StorageOrder S>
void MatrixBase<T,S>::ScaleAndIncrement(T a, T b) {
    if (IsColMajor()) {
#pragma omp parallel for
        for(int32_t j = 0; j < cols_; j++) {
            for(int32_t i = 0; i < rows_; i++) {
                T& e = (*this)(i,j);
                e = a*e + b;
            }
        }
    }
    else {
#pragma omp parallel for
        for(int32_t i = 0; i < rows_; i++) {
            for(int32_t j = 0; j < cols_; j++) {
                T& e = (*this)(i,j);
                e = a*e + b;
            }
        }
    }
}


template <typename T, StorageOrder S>
void MatrixBase<T,S>::IncrementAndScale(T a, T b) {
    if (IsColMajor()) {
#pragma omp parallel for
        for(int32_t j = 0; j < cols_; j++) {
            for(int32_t i = 0; i < rows_; i++) {
                T& e = (*this)(i,j);
                e = (a+e) * b;
            }
        }
    }
    else {
#pragma omp parallel for
        for(int32_t i = 0; i < rows_; i++) {
            for(int32_t j = 0; j < cols_; j++) {
                T& e = (*this)(i,j);
                e = (a+e) * b;
            }
        }
    }
}

template <typename T, StorageOrder S>
void MatrixBase<T,S>::Randomize(T a, T b){
    //set random seed based on time
    //srand (static_cast <unsigned> (time(0)));
    std::random_device rnd_device;
    // Specify the engine and distribution.
    std::mt19937 mersenne_engine {rnd_device()};
    std::uniform_real_distribution<T> dist {a, b};

    auto gen = [&dist, &mersenne_engine](){
        return dist(mersenne_engine);
    };

    std::generate(data_ , data_ + (rows_* cols_), gen);
}

template<typename T, StorageOrder S>
template<typename T_out>
Matrix<T_out, S> MatrixBase<T, S>::Cast() const {
    Matrix<T_out, S> M(rows_, cols_);
    if (IsColMajor()) {
        for (int32_t j = 0; j < cols_; j++) {
            for (int32_t i = 0; i < rows_; i++) {
                M(i,j) = (T_out) (*this)(i,j);
            }
        }
    }
    else {
        for (int32_t i = 0; i < rows_; i++) {
            for (int32_t j = 0; j < cols_; j++) {
                M(i,j) = (T_out) (*this)(i,j);
            }
        }
    }
    return M;
}

template <typename T, StorageOrder S>
void MatrixBase<T,S>::Show() const {
    int32_t rowlim = (rows_ > 6) ? 6: rows_ ;
    int32_t collim = (cols_ > 6) ? 6: cols_ ;
    // Unecessary repetition, but maybe it is better to print this clarification on dimensions in case some are restricted
    if((cols_ > 6) || (rows_ > 6)){
        std::cout<< "Showing the first ("<<rowlim  << ", "<< collim <<") block of the matrix with total dimensions ("<<rows_ << ", "<<cols_<< ")."<<std::endl;
    }
    std::cout << std::endl;
    for (int32_t i = 0; i < rowlim ; i++) {
        for (int32_t j = 0; j < collim; j++) {
            std::cout << (*this)(i,j)<< " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template <typename T, StorageOrder S>
void MatrixBase<T,S>::ShowBlock(int32_t row_start, int32_t col_start, int32_t rows, int32_t cols) const {
    if (row_start < 0 || col_start < 0 || rows < 0 || cols < 0) {
        throw std::invalid_argument("Block indices and sizes should be positive");
    }
    if (row_start + rows > rows_ || col_start + cols > cols_) {
        throw std::invalid_argument("Some block indices are out of range of the original matrix.");
    }
    std::cout << std::endl;
    for (int32_t i = 0; i < rows ; i++) {
        for (int32_t j = 0; j < cols; j++) {
            std::cout << (*this)(row_start + i, col_start + j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template<typename T, StorageOrder S>
Block<T, S> MatrixBase<T, S>::GetBlock(int32_t i, int32_t j, int32_t rows, int32_t cols) const {
    if (i < 0 || j < 0 || rows < 0 || cols < 0) {
        throw std::invalid_argument("Block indices and sizes should be positive");
    }
    if (i + rows > rows_ || j + cols > cols_) {
        throw std::invalid_argument("Some block indices are out of range of the original matrix.");
    }
    return Block<T, S>(const_cast<T*>(&((*this)(i,j))), rows, cols, stride_);
    // const_cast here on the last line is a bit dubious workaround for the fact that the constructor - and ultimately
    // the new Block object - require a non-const pointer. This allows changing a "const MatrixBase" via taking blocks of it
    // (since the blocks do not need to be const). This is of course a violation of const-correctness, but likely does
    // not pose much practical problems. It would be nice to find a neat and simple alternative that would retain
    // const-correctness, though.
}

template<typename T, StorageOrder S>
Block<T, S> MatrixBase<T, S>::GetRow(int32_t i) const {
    return GetBlock(i, 0, 1, cols_);
}

template<typename T, StorageOrder S>
Block<T, S> MatrixBase<T, S>::GetCol(int32_t i) const {
    return GetBlock(0, i, rows_, 1);
}


template<typename T, StorageOrder S>
void MatrixBase<T, S>::WriteToFile(const std::string &filename) const {
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    if (!fout.is_open()) throw std::runtime_error("Opening file failed.");

    int32_t header[3] = { TypeID(), rows_, cols_ };
    fout.write((char*) header, 3*sizeof(int32_t));
    for (int32_t j = 0; j < cols_; j++) {
        for (int32_t i = 0; i < rows_; i++) {
            fout.write((char*) &((*this)(i,j)), sizeof(T));
        }
    }
    fout.close();
}

template <typename T, StorageOrder S>
inline int32_t MatrixBase<T, S>::TypeID() const {
    return matrix_helpers::typeID<T>();
}

template <typename T, StorageOrder S>
inline bool MatrixBase<T, S>::IsColMajor() const {
    return matrix_helpers::isColMajor<S>();
}




/*---------------------------------------------------------------------------------*/
// OUT-OF-CLASS FUNCTIONS

// Comparison function for unit-testing - use scaled root-mean-square
// of the first matrix as the equality threshold
template <typename T1, typename T2, StorageOrder S1, StorageOrder S2>
bool almostEqual(const MatrixBase<T1, S1>& A, const MatrixBase<T2, S2>& B, T1 thresholdScale) {
    if (A.Rows() != B.Rows() || A.Cols() != B.Cols()) return false;
    T1 thres = A.Rms() * thresholdScale;
    for (int32_t i = 0; i < A.Rows(); i++) {
        for (int32_t j = 0; j < A.Cols(); j++) {
            if (std::abs(A(i,j) - B(i,j)) > thres) return false;
        }
    }
    return true;
}

#endif //TMS_CPP_MATRIXBASE_HPP
