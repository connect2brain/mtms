//
// Created by Kalle Jyrkinen on 16.11.2021.
//

#ifndef TMS_CPP_SPARSEMATRIX_HPP
#define TMS_CPP_SPARSEMATRIX_HPP

#include <vector>

template <typename T>
struct COOTriplet {
    int32_t i,j;
    T value;
};

// A (really simple) representation of a sparse matrix, represented as a list of COO triplets

template <typename T>
class SparseMatrix {
public:
    SparseMatrix(int32_t capacity) : vec_(capacity, {0,0,0}) { }
    const COOTriplet<T>& operator()(int32_t i) const { return vec_[i]; }
    COOTriplet<T>& operator()(int32_t i) { return vec_[i]; }
    int32_t Capacity() const { return vec_.size(); }

protected:
    std::vector< COOTriplet<T> > vec_;
};

template <typename T>
SparseMatrix<T> Eye(int32_t n) {
    SparseMatrix<T> I(n);
    for (int32_t i = 0; i < n; i++) {
        I(i) = { i, i, 1 };
    }
    return I;
}

#endif //TMS_CPP_SPARSEMATRIX_HPP
