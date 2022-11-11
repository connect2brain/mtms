//
// Created by Kalle Jyrkinen on 13.12.2021.
//

#ifndef TMS_CPP_BEMOPERATORSPHI_LC_HPP
#define TMS_CPP_BEMOPERATORSPHI_LC_HPP

#include <tmsutil>

// Makes the double-layer (D) matrices for LC BEM
// Returns the blocks in an std::vector, in column-major ordering
std::vector<Matrix<float>> BEMOperatorsPhi_LC(const std::vector<Mesh<float>> &meshes // BEM geometry
);

#endif //TMS_CPP_BEMOPERATORSPHI_LC_HPP
