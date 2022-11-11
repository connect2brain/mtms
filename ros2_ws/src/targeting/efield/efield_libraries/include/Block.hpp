//
// Created by Kalle Jyrkinen on 23.2.2022.
//

#ifndef TMS_CPP_BLOCK_HPP
#define TMS_CPP_BLOCK_HPP


// A class representing a block of an existing matrix. Different from Matrix in the sense that Block does not manage its
// own memory - it merely stores a pointer. However, Block objects support derive many useful methods from MatrixBase.
//
// On top of memory management, another key difference between Matrix and Block is how they handle assignment.
// When a Matrix is assigned to, it can change size and allocate more memory if needed. However, when a Block is
// assigned to, the only thing that happens is that the elements are copied; size and the pointer are unchanged. If the
// dimensions do not match, an error is thrown.
template <typename T, StorageOrder S = ColMajor>
class Block : public MatrixBase<T,S> {
public:
    // Default constructor
    Block() : MatrixBase<T, S>() { }
    // Constructor taking the necessary parameters to initialize a "meaningful" Block object
    Block(T* data, int32_t rows, int32_t cols, int32_t stride) : MatrixBase<T, S>(data, rows, cols, stride) { }

    // Default copy assignment operator
    Block& operator=(const Block<T,S>& old);
    // A more general assignment operator, works with any MatrixBase object as parameter
    template <StorageOrder S1>
    Block& operator=(const MatrixBase<T, S1>& old);
};


/*---------------------------------------------------------------------------------*/
// METHOD IMPLEMENTATIONS

template<typename T, StorageOrder S>
Block<T,S>& Block<T, S>::operator=(const Block<T, S> &old) {
    if (this->cols_ != old.Cols() || this->rows_!= old.Rows()) {
        throw std::invalid_argument("Error trying to assign to a block: dimensions do not match");
    }
    if (this->IsColMajor()) {
        for (int32_t j = 0; j < this->cols_; j++) {
            for (int32_t i = 0; i < this->rows_; i++) {
                (*this)(i,j) = old(i,j);
            }
        }
    }
    else {
        for (int32_t i = 0; i < this->rows_; i++) {
            for (int32_t j = 0; j < this->cols_; j++) {
                (*this)(i,j) = old(i,j);
            }
        }
    }
    return *this;
}


template<typename T, StorageOrder S>
template<StorageOrder S1>
Block<T,S>& Block<T, S>::operator=(const MatrixBase<T, S1> &old) {
    if (this->cols_ != old.Cols() || this->rows_!= old.Rows()) {
        throw std::invalid_argument("Error trying to assign to a block: dimensions do not match");
    }
    if (old.IsColMajor()) {
        for (int32_t j = 0; j < this->cols_; j++) {
            for (int32_t i = 0; i < this->rows_; i++) {
                (*this)(i,j) = old(i,j);
            }
        }
    }
    else {
        for (int32_t i = 0; i < this->rows_; i++) {
            for (int32_t j = 0; j < this->cols_; j++) {
                (*this)(i,j) = old(i,j);
            }
        }
    }
    return *this;
}

#endif //TMS_CPP_BLOCK_HPP
