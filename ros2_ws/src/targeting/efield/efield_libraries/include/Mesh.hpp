#pragma once

#include <string>
#include <linalg>
#include <stdexcept>

// A triangular mesh. Stores 1-2 matrices:
//     - points_ :   The triangle vertices. [nop x 3], row-major.
//     - elements_ : The triangle element connectivity list, as a list of indices
//                  pointing to rows of 'points_'. [noe_ x 3], row-major.

#include <iostream>
#include <fstream>

template<typename T>
class Mesh {
public:

    Mesh(std::string pointfile, std::string elementfile);

    Mesh(std::string meshfile);

    Mesh(Matrix<T, RowMajor> p, Matrix<int32_t, RowMajor> e);

    Mesh(Matrix<T, RowMajor> p);

    Mesh() = default;

    int32_t Nop() const { return nop_; }

    int32_t Noe() const { return noe_; }

    int32_t MeshType() const { return meshtype_; }

    const Matrix<T, RowMajor> &Points() const { return points_; }

    const Matrix<int32_t, RowMajor> &Elements() const { return elements_; }

private:
    int32_t nop_, noe_, meshtype_;
    Matrix<T, RowMajor> points_;
    Matrix<int32_t, RowMajor> elements_;
};

template<typename T>
Mesh<T>::Mesh(std::string pointfile, std::string elementfile) : points_(pointfile), elements_(elementfile) {
    if (points_.Cols() != 3 || elements_.Cols() != 3) {
        throw std::invalid_argument("Points and elements_ matrices must have 3 columns");
    }
    nop_ = points_.Rows();
    noe_ = elements_.Rows();
}

template<typename T>
Mesh<T>::Mesh(std::string meshfile) {
    if (meshfile.empty()) return;
    std::ifstream fin(meshfile.c_str());
    if (!fin.is_open()) {
        std::cout << "File " << meshfile << std::endl;
        throw std::runtime_error("Opening file failed.");
    }

    fin.read((char *) &meshtype_, sizeof(int32_t));
    fin.read((char *) &nop_, sizeof(int32_t));
    fin.read((char *) &noe_, sizeof(int32_t));

    points_ = Matrix<T, RowMajor>(nop_, 3);
    elements_ = Matrix<int32_t, RowMajor>(noe_, 3);

    points_.ReadDataFromStream(fin, 1, true); // 1 since datatype=='float'
    elements_.ReadDataFromStream(fin, 2, true); // 2 since datatype='int32_t'
    // ReadRowMajor = true as the we want to read the data from the file linearily into [Nx3] row-major without automatic transpose

    fin.close();
}

template<typename T>
Mesh<T>::Mesh(Matrix<T, RowMajor> p, Matrix<int32_t, RowMajor> e) : points_(p), elements_(e) {
    if (points_.Cols() != 3 || elements_.Cols() != 3) {
        throw std::invalid_argument("Points and elements_ matrices must have 3 columns");
    }
    nop_ = points_.Rows();
    noe_ = elements_.Rows();
}

template<typename T>
Mesh<T>::Mesh(Matrix<T, RowMajor> p) : points_(p) {
    if (points_.Cols() != 3) {
        throw std::invalid_argument("Points matrix must have 3 columns");
    }
    nop_ = points_.Rows();
    noe_ = 0;
}

