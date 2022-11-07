#pragma once

#include <linalg>

/*
Calculates the product of matrix A transposed with vector x and adds it to y. Multiplies the result by mul

y = mul * (A' * x + y)

Doesn't use any intrinsics, but is "manually vectorized" and allows the compiler to use AVX when using -O3 and -mavx2 flags
*/


/* 
col_start and col_n are absolute numbers of columns and not the number of triplets as used in other functions (ROI_start and ROI_n)

Input/output y is read linearly in all cases and the ROI/col start and size parameters only affect the indexing on matrix A. 
That is, y is assumed to be resized to result size before matrix vector multiplication. For TMS calculation this means that y contains the primary B field only in ROI points.
*/
template <typename T, typename T2>
void SaxpyT(const T2 *A, const T *x, T *y, const T mul, const int32_t rows, const int32_t col_start, const int32_t col_n);


/*
Specialised for this particular TMS application. Notice the different indexing on ROI choice, triplets vs single columns
*/
template <typename T, typename T2>
void SaxpyT(const Matrix<T2> &A, const ColVector<T> &x,  Matrix<T, RowMajor> &y, const T mul, const int32_t ROI_start, const int32_t ROI_n){
    if(A.Rows() != x.Rows()){
        throw std::invalid_argument("Incorrect dimensions in function SaxpyT");
    }
    SaxpyT(A.Data(), x.Data(), y.Data(), mul, A.Rows(), 3*ROI_start, 3*ROI_n);
}

template <typename T, typename T2>
void SaxpyT(const Matrix<T2> &A, const ColVector<T> &x,  Matrix<T, RowMajor> &y, const T mul){
    if(A.Rows() != x.Rows() || A.Cols() != 3* y.Rows()){
        throw std::invalid_argument("Incorrect dimensions in function SaxpyT");
    }
    SaxpyT(A.Data(), x.Data(), y.Data(), mul, A.Rows(), 0, 3*y.Rows());
}

/* 
Uses only the columns of phi defined by the indlist. 

Writes and reads y pointer linearly meaning that it has to be filtered beforehand accoridng to the indlist.

Does not check any array boundaries and assumes that the indices in indlist are not out of bounds
*/
template <typename T, typename T2>
void SaxpyT_indlist(const T2 *A, const T *x, T *y, const T mul, const int32_t rows, const int32_t *p_indlist, const int32_t N_inds);

/*
Specialised for this particular TMS application. 
*/
template <typename T, typename T2>
void SaxpyT_indlist(const Matrix<T2> &A, const ColVector<T> &x,  Matrix<T, RowMajor> &y, const T mul, const ColVector<int32_t> &indlist){
    if( A.Rows() != x.Rows()){
        throw std::invalid_argument("Incorrect dimensions in function SaxpyT");
    }
    SaxpyT(A.Data(), x.Data(), y.Data(), mul, A.Rows(), indlist, indlist.Rows());
}