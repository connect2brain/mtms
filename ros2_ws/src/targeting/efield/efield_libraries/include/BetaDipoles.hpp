#pragma once

#include <stdexcept>
#include <linalg>

// A class representing a group of N beta dipoles.
template <typename T>
class BetaDipoles {

public:
    BetaDipoles() = default;

    BetaDipoles(Matrix<T, RowMajor> dpos, Matrix<T, RowMajor> dmom): points(dpos), moments(dmom) {
        if (dpos.Rows() != dmom.Rows()){
            throw std::invalid_argument("The number of dipole positions and dipole moments must match");
        }else if(dpos.Cols() != 3 || dmom.Cols() != 3){
            throw std::invalid_argument("BetaDipole positions and moments must have 3 columns");
        }
        nop = dpos.Rows();
    }

    int32_t Nop() const {return nop;}

    const Matrix<T, RowMajor>& Points() const {
        return points;
    }
    const Matrix<T, RowMajor>& Moments() const {
        return moments;
    }

private:
    int32_t nop;
    Matrix<T, RowMajor> points;  // dipole positions, [n x 3]
    Matrix<T, RowMajor> moments; // dipole moments, [n x 3]
};



