#pragma once


/**
 * @file MatrixBaseAddons.hpp
 * @brief Add functionalities to Eigen::MatrixBase, as described in https://eigen.tuxfamily.org/dox/TopicCustomizing_Plugins.html
 * 
 * @details Matrix binary data structure:
 * \b Header int32_t[3]  : [typeID, number of rows, number of cols] \n 
 * \b Data   float[rows x cols] \n 
 * Data is always stored in column major ordering 
 */


/**
 * @brief Query the type of the matrix entries
 * 
 * @return returns 0 for double, 1 for float, 2 for int32_t
 */
inline int32_t TypeID() const {
    MatrixBase<Derived>::Scalar t;
    std::string t_id = typeid(t).name();
    if (t_id == "d") return 0;
    else if (t_id == "f") return 1;
    else if (t_id == "i") return 2;
    else {
        std::cout << "Error: Method TypeID called on a matrix of unknown type: " << t_id << std::endl;
        throw std::runtime_error("Error: Method TypeID called on a matrix of unknown type.");

    }
}


/**
 * @brief Query the size of the matrix entries
 * 
 * @return size in bytes
 */
inline int32_t SizeOfElem() const {
    MatrixBase<Derived>::Scalar t;
    return sizeof(t);
}

/**
 * @brief Initialize matrix from binary file (Must be constructed before! Default constructor suffices)
 * @note Assumes that the file data is column-major and transposes when initializing a row-major matrix
 * @ingroup util
 */
inline void ReadFromFile(const std::string &filename) {
    if (filename.empty()) {
        throw std::invalid_argument("Empty filename given as constructor parameter.");
    }

    std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);

    if (!fin.is_open()) {
        std::cout << "File " << filename << std::endl;
        throw std::runtime_error("Opening file failed.");
    }

    int32_t header[3]; // metadata: { typeID, number of rows, number of cols }
    fin.read((char *) header, 3 * sizeof(int32_t));

    this->derived().resize(header[1], header[2]);

    if (this->TypeID() == header[0]) {
        for (int32_t j = 0; j < this->cols(); j++) {
            for (int32_t i = 0; i < this->rows(); i++) {
                fin.read((char *) &((*this)(i, j)), this->SizeOfElem());
            }
        }
    } else if (this->TypeID() == 0 && header[0] == 1) {
        float read_buf;
        for (int32_t j = 0; j < this->cols(); j++) {
            for (int32_t i = 0; i < this->rows(); i++) {
                fin.read((char *) &read_buf, sizeof(float));
                (*this)(i, j) = (double) read_buf;
            }
        }
    } else if (this->TypeID() == 1 && header[0] == 0) {
        double read_buf;
        for (int32_t j = 0; j < this->cols(); j++) {
            for (int32_t i = 0; i < this->rows(); i++) {
                fin.read((char *) &read_buf, sizeof(double));
                (*this)(i, j) = (float) read_buf;
            }
        }
    } else {
        std::cout << "File data type ID is " << header[0] << " and matrix data type ID is " << this->TypeID()
                  << std::endl;
        throw std::invalid_argument("File and matrix data types do not match.");
    }

    fin.close();
}




/**
 * @brief Write a matrix to a binary file.
 * @note Transposes Rowmajor matrices (change order of data, not dimensions)
 * @ingroup util
 */
inline void WriteToFile(const std::string &filename) {
    std::ofstream fout(filename.c_str(), std::ios::out | std::ios::binary);

    if (!fout.is_open()) throw std::runtime_error("Opening file failed.");

    int32_t header[3] = {this->TypeID(), (int32_t) this->rows(), (int32_t) this->cols()};
    fout.write((char *) header, 3 * sizeof(int32_t));
    for (int32_t j = 0; j < this->cols(); j++) {
        for (int32_t i = 0; i < this->rows(); i++) {
            fout.write((char *) &((*this)(i, j)), this->SizeOfElem());
        }
    }
    fout.close();
}

/**
 * @brief Prints the first 6x6 block of a matrix
 * @ingroup util
 */
inline void Show() const {
    int32_t rowlim = (this->rows() > 6) ? 6: this->rows() ;
    int32_t collim = (this->cols() > 6) ? 6: this->cols() ;
    // Unecessary repetition, but maybe it is better to print this clarification on dimensions in case some are restricted
    if((this->cols() > 6) || (this->rows() > 6)){
        std::cout<< "Showing the first ("<<rowlim  << ", "<< collim <<") block of the matrix with total dimensions ("<<this->rows() << ", "<<this->cols()<< ")."<<std::endl;
    }
    std::cout<<"\n"<< this->block(0,0,rowlim,collim)<< "\n" <<std::endl;
}


/**
 * @brief Prints a block specified by indices
 * 
 * @param row_start starting row
 * @param col_start starting column
 * @param rows  Number of rows
 * @param cols  NUmber of columns
 * @ingroup util
 */
inline void ShowBlock(int32_t row_start, int32_t col_start, int32_t rows, int32_t cols) const {
    if (row_start < 0 || col_start < 0 || rows < 0 || cols < 0) {
        throw std::invalid_argument("Block indices and sizes should be positive\n");
    }
    if (row_start + rows > this->rows() || col_start + cols > this->cols()) {
        throw std::invalid_argument("Some block indices are out of range of the original matrix.\n");
    }
    std::cout<<"\n"<< this->block(row_start,col_start,rows,cols)<< "\n" <<std::endl;
}
