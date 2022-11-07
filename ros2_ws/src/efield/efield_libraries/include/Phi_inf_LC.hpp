//
// Created by Kalle Jyrkinen on 17.11.2021.
//

#ifndef TMS_CPP_PHI_INF_LC_HPP
#define TMS_CPP_PHI_INF_LC_HPP

#include <tmsutil>

// Compute the electric potential due to current dipoles in infinite homogeneous volume
// conductor of unit conductivity, using LC.
//
// Outputs a [(number of field vertices) X (number of source positions)] matrix:
//   Phi_{i,j} = potential of dipole j at field mesh vertex i
//
// The third argument specifies the dipole moments and can be one of the following:
//   - [(number of source positions) X 3] matrix => a separate dipole moment is specified for each position
//   - nothing => 3 unit dipoles (x, y and z-oriented) are placed in each position

Matrix<float> Phi_inf_LC(const Matrix<float, RowMajor> &fp, // field mesh
                         const Matrix<float, RowMajor> &spos, // source positions, [M x 3]
                         const Matrix<float, RowMajor> &sdir // source dipole moments [M x 3]
);

Matrix<float> Phi_inf_LC(const Matrix<float, RowMajor> &fp,
                         const Matrix<float, RowMajor> &spos
);


#endif //TMS_CPP_PHI_INF_LC_HPP
