#pragma once

#include <stdexcept>
#include "eigen_defines.hpp"


/**
 * @brief A container class representing quadrature points and moments used in secondary E-field calculation
 */
template <typename T>
class BetaDipoles {

public:
    /**
     * @brief Default constructor for creating empty BetaDipoles object
     */
    BetaDipoles() = default;

    /**
     * @brief Array constructor
     * 
     * @param dpos Quadrature points as Eigen Row-major matrix [NoP x 3] 
     * @param dmom Quadrature moments as Eigen Row-major matrix [NoP x 3] 
     */
    BetaDipoles(MatrixX3T_RM<T> dpos, MatrixX3T_RM<T> dmom): points(dpos), moments(dmom) {
        if (dpos.rows() != dmom.rows()){
            throw std::invalid_argument("The number of dipole positions and dipole moments must match");
        }else if(dpos.cols() != 3 || dmom.cols() != 3){
            throw std::invalid_argument("BetaDipole positions and moments must have 3 columns");
        }
        nop = dpos.rows();
    }
    /**
     * @return Number of points
     */
    int32_t Nop() const {return nop;}
    /**
     * @return Quadrature points asEigen Row-major matrix [NoP x 3]
     */
    const MatrixX3T_RM<T>& Points() const {
        return points;
    }
     /**
     * @return Quadrature moments as Eigen Row-major matrix [NoP x 3]
     */
    const MatrixX3T_RM<T>& Moments() const {
        return moments;
    }

private:
    int32_t nop;
    MatrixX3T_RM<T> points;  // dipole positions, [n x 3]
    MatrixX3T_RM<T> moments; // dipole moments, [n x 3]
};



