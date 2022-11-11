//
// Created by Kalle Jyrkinen on 24.11.2021.
//

#ifndef TMS_CPP_VECTOR_HPP
#define TMS_CPP_VECTOR_HPP

#include "Matrix.hpp"

template <typename T> class RowVector; // forward declaration

// ColVector: a class derived from Matrix. The main differences are
//   - ColVector is allowed to have only 1 column
//   - ColVector supports indexing with 1 index
//
// Manages its own memory just like a Matrix.
// A copy of the data is created during instantiation - if you need to avoid that, use Block instead.
template <typename T>
class ColVector : public Matrix<T, ColMajor> { // storage order doesn't matter here, thus fix ColMajor
public:
    // CONSTRUCTORS AND DESTRUCTOR

    // Default constructor
    ColVector();
    // Constructor allocating and zeroing a [elems x 1] column vector
    ColVector(int32_t elems);
    // Constructor allocating a [elems x 1] column vector and filling it with data given in an std::vector (copies the data)
    ColVector(int32_t elems, const std::vector<T>& data);
    // Constructor reading data from a binary file
    ColVector(const std::string& s);
    // Constructor copying data from an existing instance of MatrixBase (matrix must be of size [n x 1])
    template <StorageOrder S>
    ColVector(const MatrixBase<T,S>& old);

    // Destructor; use default (calls base class destructor by default and that's all we need)
    ~ColVector() = default;

    // COPY CONSTRUCTOR & ASSIGNMENT, MOVE CONSTRUCTOR AND ASSIGNMENT

    // Copy constructor
    ColVector(const ColVector<T>& old);
    // Copy assignment operator
    ColVector<T>& operator=(const ColVector<T>& old);
    // Move constructor
    ColVector(ColVector<T>&& old) noexcept;
    // Move assignment operator
    ColVector<T>& operator=(ColVector<T>&& old) noexcept;


    // OTHER METHODS

    // "Copy assignment operator" taking an existing matrix as a parameter. Overrides the method inherited from Matrix
    // since we need to check that the number of columns is 1.
    template <StorageOrder S>
    ColVector<T>& operator=(const MatrixBase<T,S>& other);

    // Allow indexing with a single index; for example "vec(0)" to get the first element
    const T& operator()(int32_t i) const { return this->data_[i]; } // used for const objects
    T& operator()(int32_t i) { return this->data_[i]; } // used for non-const objects

    // Transpose
    RowVector<T> Transpose();
};


// RowVector: a class derived from Matrix. The main differences are
//   - RowVector is allowed to have only 1 column
//   - RowVector supports indexing with 1 index
//
// Manages its own memory just like a Matrix.
// A copy of the data is created during instantiation - if you need to avoid that, use Block instead.
template <typename T>
class RowVector : public Matrix<T, ColMajor> {
public:
    // CONSTRUCTORS AND DESTRUCTOR

    // Default constructor
    RowVector();
    // Constructor allocating and zeroing a [1 x elems] row vector
    RowVector(int32_t elems);
    // Constructor allocating a [1 x elems] row vector and filling it with data given in an std::vector (copies the data)
    RowVector(int32_t elems, const std::vector<T>& data);
    // Constructor reading data from a binary file
    RowVector(const std::string& s);
    // Constructor copying data from an existing MatrixBase instance (matrix must be of size [1 x n])
    template <StorageOrder S>
    RowVector(const MatrixBase<T,S>& old);

    // Destructor; use default (calls base class destructor by default and that's all we need)
    ~RowVector() = default;

    // COPY CONSTRUCTOR & ASSIGNMENT, MOVE CONSTRUCTOR AND ASSIGNMENT

    // Copy constructor
    RowVector(const RowVector<T>& old);
    // Copy assignment operator
    RowVector<T>& operator=(const RowVector<T>& old);
    // Move constructor
    RowVector(RowVector<T>&& old) noexcept;
    // Move assignment operator
    RowVector<T>& operator=(RowVector<T>&& old) noexcept;


    // OTHER METHODS

    // "Copy assignment operator" taking an existing matrix as a parameter. Overrides the method inherited from Matrix
    // since we need to check that the number of rows is 1.
    template <StorageOrder S>
    RowVector<T>& operator=(const MatrixBase<T,S>& other);

    // Allow indexing with a single index; for example "vec(0)" to get the first element
    const T& operator()(int32_t i) const { return this->data_[i]; } // used for const objects
    T& operator()(int32_t i) { return this->data_[i]; } // used for non-const objects

    // Transpose
    ColVector<T> Transpose();
};


/*---------------------------------------------------------------------------------*/
// METHOD IMPLEMENTATIONS


template<typename T>
ColVector<T>::ColVector() : Matrix<T, ColMajor>() { }

template<typename T>
ColVector<T>::ColVector(int32_t elems) : Matrix<T, ColMajor>(elems, 1) { }

template<typename T>
ColVector<T>::ColVector(int32_t elems, const std::vector<T> &data) : Matrix<T, ColMajor>(elems, 1, data) { }

template<typename T>
ColVector<T>::ColVector(const std::string &s) : Matrix<T, ColMajor>(s) {
    if (this->cols_ != 1) {
        throw std::invalid_argument("Trying to read a matrix with column number not equal to 1 to a ColVector");
    }
}

template<typename T>
template<StorageOrder S>
ColVector<T>::ColVector(const MatrixBase<T, S> &old) : Matrix<T, ColMajor>(old) {
    if (old.Cols() != 1) {
        throw std::invalid_argument("Matrix being copied from should have exactly 1 column.");
    }
}

template<typename T>
ColVector<T>::ColVector(const ColVector<T> &old) : Matrix<T, ColMajor>(old) { }

template<typename T>
ColVector<T> &ColVector<T>::operator=(const ColVector<T> &old) {
    Matrix<T,ColMajor>::operator=(old);
    return *this;
}

template<typename T>
ColVector<T>::ColVector(ColVector<T> &&old) noexcept : Matrix<T, ColMajor>(std::move(old)) { }

template<typename T>
ColVector<T> &ColVector<T>::operator=(ColVector<T> &&old) noexcept {
    Matrix<T, ColMajor>::operator=(std::move(old));
    return *this;
}

template<typename T>
template<StorageOrder S>
ColVector<T> &ColVector<T>::operator=(const MatrixBase<T, S> &other) {
    if (other.Cols() != 1) {
        throw std::invalid_argument("Matrix being assigned from should have exactly 1 column.");
    }
    Matrix<T, ColMajor>::operator=(other);
    return *this;
}

template<typename T>
RowVector<T> ColVector<T>::Transpose() {
    RowVector<T> rv(this->rows_);
    rv = (*this);
    return rv;
}

/*---------------------------------------------------------------------------------*/

template<typename T>
RowVector<T>::RowVector() : Matrix<T, ColMajor>() { }

template<typename T>
RowVector<T>::RowVector(int32_t elems) : Matrix<T, ColMajor>(elems, 1) { }

template<typename T>
RowVector<T>::RowVector(int32_t elems, const std::vector<T> &data) : Matrix<T, ColMajor>(1, elems, data) { }

template<typename T>
RowVector<T>::RowVector(const std::string &s) : Matrix<T, ColMajor>(s) {
    if (this->rows_ != 1) {
        throw std::invalid_argument("Trying to read a matrix with row number not equal to 1 to a RowVector");
    }
}

template<typename T>
template<StorageOrder S>
RowVector<T>::RowVector(const MatrixBase<T, S> &old) : Matrix<T, ColMajor>(old) {
    if (old.Rows() != 1) {
        throw std::invalid_argument("Matrix being copied from should have exactly 1 row.");
    }
}

template<typename T>
RowVector<T>::RowVector(const RowVector<T> &old) : Matrix<T, ColMajor>(old) { }

template<typename T>
RowVector<T> &RowVector<T>::operator=(const RowVector<T> &old) {
    Matrix<T,ColMajor>::operator=(old);
    return *this;
}

template<typename T>
RowVector<T>::RowVector(RowVector<T> &&old) noexcept : Matrix<T, ColMajor>(std::move(old)) { }

template<typename T>
RowVector<T> &RowVector<T>::operator=(RowVector<T> &&old) noexcept {
    Matrix<T, ColMajor>::operator=(std::move(old));
    return *this;
}

template<typename T>
template<StorageOrder S>
RowVector<T> &RowVector<T>::operator=(const MatrixBase<T, S> &other) {
    if (other.Rows() != 1) {
        throw std::invalid_argument("Matrix being assigned from should have exactly 1 row.");
    }
    Matrix<T, ColMajor>::operator=(other);
    return *this;
}

template<typename T>
ColVector<T> RowVector<T>::Transpose() {
    ColVector<T> cv(this->rows_);
    cv = (*this);
    return cv;
}

#endif //TMS_CPP_VECTOR_HPP
