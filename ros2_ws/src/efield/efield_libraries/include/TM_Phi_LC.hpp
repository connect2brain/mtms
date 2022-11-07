//
// Created by Kalle Jyrkinen on 16.11.2021.
//

#ifndef TMS_CPP_TM_PHI_LC_HPP
#define TMS_CPP_TM_PHI_LC_HPP

#include <linalg>

// Build the transfer matrix for potential.

Matrix<float> TM_Phi_LC(const std::vector<Matrix<float> > &D, // D-matrices, col-major ordering
                        const std::vector<float> &ci, // conductivities inside
                        const std::vector<float> &co, // conductivities outside
                        int32_t zerolevel = -1 // index of the mesh on which the mean potential over the vertices is set to zero - by default the last mesh
);

#endif //TMS_CPP_TM_PHI_LC_HPP
