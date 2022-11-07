//
// Created by Kalle Jyrkinen on 24.9.2021.
//

#ifndef TMS_CPP_MATRIX_HPP
#define TMS_CPP_MATRIX_HPP

#include "MatrixBase.hpp"

// A class representing a matrix located within one contiguous block of memory.
// Takes care of its own memory management.
//
// Most of the useful methods are inherited from MatrixBase
template <typename T, StorageOrder S = ColMajor>
class Matrix : public MatrixBase<T,S> {
public:
    // CONSTRUCTORS AND DESTRUCTOR

    // Default constructor
    Matrix();
    // Constructor allocating and zeroing a contiguous memory block of size (rows*cols)
    Matrix(int32_t rows, int32_t cols);
    // Constructor reading a matrix from a file
    Matrix(const std::string& filename);
    // Constructor taking an std::vector as a parameter. Allows easy construction of matrices. Accepts also
    // brace-enclosed initializer lists => we can write "Matrix<float> A(2,2,{1,2,3,4})" for instance.
    // The elements are written according to storage order (row-major matrices are filled row-wise and vice versa for col-major).
    Matrix(int32_t rows, int32_t cols, std::vector<T> vec);
    // Constructor taking stride and a pointer as a parameter. This allows creating matrices referencing memory blocks
    // allocated somewhere else. Used by GetBlock for instance.
    Matrix(T* data, int32_t rows, int32_t cols, int32_t stride);
    // "Copy constructor" to allow copying from any MatrixBase instance
    template <StorageOrder S1>
    Matrix(const MatrixBase<T, S1>& old);

    // Destructor
    ~Matrix();

    // COPY AND MOVE CONSTRUCTOR & ASSIGNMENT

    // Copy constructor
    Matrix(const Matrix<T,S>& old);
    // Copy assignment operator
    Matrix<T,S>& operator=(const Matrix<T,S>& old);
    // Move constructor
    Matrix(Matrix<T,S>&& old) noexcept;
    // Move assignment operator
    Matrix<T,S>& operator=(Matrix<T,S>&& old) noexcept;

    // "Copy assignment" to allow copying from any MatrixBase instance
    template <StorageOrder S1>
    Matrix<T,S>& operator=(const MatrixBase<T,S1>& old);


    // OTHER METHODS

    // Free memory
    void Free();

    // Read in data from a binary file. Sets the number of rows and columns and reads the elements.
    void ReadDataFromFile(const std::string& filename);

    // Read in data from a file stream, pointing at the start of the array in the binary file.
    // Assumes that the number of rows and columns is correct and that there is enough memory allocated.
    // "datatype" specifies the type of elements_ in the array: 0 for double, 1 for float, 2 for int32_t
    void ReadDataFromStream(std::ifstream& stream, int32_t datatype, bool RowMajorRead = false);
};



/*---------------------------------------------------------------------------------*/
// METHOD IMPLEMENTATIONS

template<typename T, StorageOrder S>
Matrix<T, S>::Matrix() : MatrixBase<T, S>() { }

template<typename T, StorageOrder S>
Matrix<T, S>::Matrix(int32_t rows, int32_t cols) : MatrixBase<T, S>(nullptr, rows, cols, this->IsColMajor() ? rows : cols) {
    this->data_ = (T*) std::calloc(rows*cols, sizeof(T));
    if (!(this->data_)) {
        throw std::bad_alloc();
    }
}

template<typename T, StorageOrder S>
Matrix<T, S>::Matrix(const std::string &filename) : MatrixBase<T, S>() {
    ReadDataFromFile(filename);
}

template<typename T, StorageOrder S>
Matrix<T, S>::Matrix(int32_t rows, int32_t cols, std::vector<T> vec) : Matrix(rows, cols) {
    assert(vec.size() == static_cast<std::size_t>(rows*cols));
    std::copy(vec.cbegin(), vec.cend(), this->data_);
}

template<typename T, StorageOrder S>
Matrix<T, S>::Matrix(T* data, int32_t rows, int32_t cols, int32_t stride) : MatrixBase<T, S>(data, rows, cols, stride) { }

template<typename T, StorageOrder S>
Matrix<T, S>::~Matrix() {
    Free();
}

template<typename T, StorageOrder S>
Matrix<T, S>::Matrix(const Matrix<T, S> &old): MatrixBase<T,S>() {
    this->rows_ = old.Rows();
    this->cols_ = old.Cols();
    this->stride_ = (this->IsColMajor()) ? this->rows_ : this->cols_;
    this->data_ = (T*) std::malloc(this->rows_*this->cols_*sizeof(T));
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
}

template<typename T, StorageOrder S>
Matrix<T, S> &Matrix<T, S>::operator=(const Matrix<T, S> &old) {
    if (this != &old) {
        this->rows_ = old.Rows();
        this->cols_ = old.Cols();
        this->stride_ = (this->IsColMajor()) ? this->rows_ : this->cols_;
        void* tmp_p = std::realloc(this->data_, this->rows_ * this->cols_ * sizeof(T));
        if (!tmp_p) {
            throw std::bad_alloc();
        }
        this->data_ = (T*) tmp_p;
        if (this->IsColMajor()) {
            for (int32_t j = 0; j < this->cols_; j++) {
                for (int32_t i = 0; i < this->rows_; i++) {
                    (*this)(i, j) = old(i, j);
                }
            }
        }
        else {
            for (int32_t i = 0; i < this->rows_; i++) {
                for (int32_t j = 0; j < this->cols_; j++) {
                    (*this)(i, j) = old(i, j);
                }
            }
        }
    }
    return *this;
}


template<typename T, StorageOrder S>
Matrix<T, S>::Matrix(Matrix<T, S> &&old) noexcept {
    this->data_ = old.data_;
    this->rows_ = old.rows_;
    this->cols_ = old.cols_;
    this->stride_ = old.stride_;
    old.data_ = nullptr;
    old.rows_ = 0;
    old.cols_ = 0;
    old.stride_ = 0;
}

template<typename T, StorageOrder S>
Matrix<T, S> &Matrix<T, S>::operator=(Matrix<T, S> &&old) noexcept {
    if (this != &old) {
        this->data_ = old.data_;
        this->rows_ = old.rows_;
        this->cols_ = old.cols_;
        this->stride_ = old.stride_;
        old.data_ = nullptr;
        old.rows_ = 0;
        old.cols_ = 0;
        old.stride_ = 0;
    }
    return *this;
}

template<typename T, StorageOrder S>
template<StorageOrder S1>
Matrix<T, S>::Matrix(const MatrixBase<T, S1> &old) : MatrixBase<T, S>() {
    this->rows_ = old.Rows();
    this->cols_ = old.Cols();
    this->stride_ = (this->IsColMajor()) ? this->rows_ : this->cols_;
    this->data_ = (T*) std::malloc(this->rows_*this->cols_*sizeof(T));
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
}

template<typename T, StorageOrder S>
template<StorageOrder S1>
Matrix<T, S> &Matrix<T, S>::operator=(const MatrixBase<T, S1>& old) {
    if ((void*) this != (void*) &old) {
        this->rows_ = old.Rows();
        this->cols_ = old.Cols();
        this->stride_ = (this->IsColMajor()) ? this->rows_ : this->cols_;
        this->data_ = (T*) std::malloc(this->rows_ * this->cols_ * sizeof(T));
        if (this->IsColMajor()) {
            for (int32_t j = 0; j < this->cols_; j++) {
                for (int32_t i = 0; i < this->rows_; i++) {
                    (*this)(i, j) = old(i, j);
                }
            }
        }
        else {
            for (int32_t i = 0; i < this->rows_; i++) {
                for (int32_t j = 0; j < this->cols_; j++) {
                    (*this)(i, j) = old(i, j);
                }
            }
        }
    }
    return *this;
}

template<typename T, StorageOrder S>
void Matrix<T, S>::Free() {
    if (this->data_) {
        std::free(this->data_);
        this->data_ = nullptr;
    }
}

template<typename T, StorageOrder S>
void Matrix<T, S>::ReadDataFromFile(const std::string &filename) {
    if (filename.empty()) return;
    std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        std::cout << "File " << filename << std::endl;
        throw std::runtime_error("Opening file failed.");
    }

    int32_t header[3];
    fin.read((char*) header, 3*sizeof(int32_t));

    this->rows_ = header[1];
    this->cols_ = header[2];
    this->stride_ = (this->IsColMajor()) ? this->rows_ : this->cols_;

    void* tmp = std::realloc(this->data_, this->rows_*this->cols_*sizeof(T));
    if (!tmp) {
        throw std::bad_alloc();
    }
    this->data_ = (T*) tmp;

    ReadDataFromStream(fin, header[0]);

    fin.close();
}

template<typename T, StorageOrder S>
void Matrix<T, S>::ReadDataFromStream(std::ifstream &stream, int32_t datatype, bool RowMajorRead) {
    if (RowMajorRead) {
        if (datatype == this->TypeID()) {
            for (int32_t i = 0; i < this->rows_; i++) {
                for (int32_t j = 0; j < this->cols_; j++) {
                    stream.read((char*) &((*this)(i,j)), sizeof(T));
                }
            }
        }
        else {
            if (datatype == 0 && this->TypeID() == 1) {  // cast double to float
                double read_buffer;
                for (int32_t i = 0; i < this->rows_; i++) {
                    for (int32_t j = 0; j < this->cols_; j++) {
                        stream.read((char*) &read_buffer, sizeof(double));
                        (*this)(i,j) = (float) read_buffer;
                    }
                }
            }
            else if (datatype == 1 && this->TypeID() == 0){ // cast float to double
                float read_buffer;
                for (int32_t i = 0; i < this->rows_; i++) {
                    for (int32_t j = 0; j < this->cols_; j++) {
                        stream.read((char*) &read_buffer, sizeof(float));
                        (*this)(i,j) = (double) read_buffer;
                    }
                }
            }
            else {
                std::cout << " Type_ID matrix: " << this->TypeID() << " Type id file: " << datatype << std::endl;
                throw std::runtime_error("Matrix and file data types do not match.");
            }
        }
    }
    else {
        if (datatype == this->TypeID()) {
            for (int32_t j = 0; j < this->cols_; j++) {
                for (int32_t i = 0; i < this->rows_; i++) {
                    stream.read((char*) &((*this)(i,j)), sizeof(T));
                }
            }
        }
        else {
            if (datatype == 0 && this->TypeID() == 1){  // cast double to float
                double read_buffer;
                for (int32_t j = 0; j < this->cols_; j++) {
                    for (int32_t i = 0; i < this->rows_; i++) {
                        stream.read((char*) &read_buffer, sizeof(double));
                        (*this)(i,j) = (float) read_buffer;
                    }
                }
            }
            else if(datatype == 1 && this->TypeID() == 0){ // cast float to double
                float read_buffer;
                for (int32_t j = 0; j < this->cols_; j++) {
                    for (int32_t i = 0; i < this->rows_; i++) {
                        stream.read((char*) &read_buffer, sizeof(float));
                        (*this)(i,j) = (double) read_buffer;
                    }
                }
            }
            else {
                std::cout << " Type_ID matrix: " << this->TypeID() << " Type id file: " << datatype << std::endl;
                throw std::runtime_error("Matrix and file data types do not match.");
            }
        }
    }
}


#endif //TMS_CPP_MATRIX_HPP
