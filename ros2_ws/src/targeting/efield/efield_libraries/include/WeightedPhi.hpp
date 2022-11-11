
#pragma once

#include <vector>
#include "Mesh.hpp"
#include <linalg>

/*
    Modifies the matrix Phi, Does NOT transpose

    Assumes column major ordering for phiM
*/
/*template <typename T, typename T2, typename T3>
void WeightedPhi(const std::vector<Mesh<T2>> &meshes, 
                 MATRIX_XT &phiM, 
                 const std::vector<T3> &ci , 
                 const std::vector<T3> &co);
*/
template <typename T, typename T2, typename T3>
void WeightedPhi(const std::vector<Mesh<T2>> &meshes, 
                 Matrix<T> &phiM,
                 const std::vector<T3> &ci , 
                 const std::vector<T3> &co,
                 const bool average = true);
