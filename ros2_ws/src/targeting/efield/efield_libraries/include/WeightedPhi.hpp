
#pragma once

#include <vector>
#include "Mesh.hpp"
#include "eigen_defines.hpp"


/**
 * @brief Multiplies the potential matrix phiM by the differences of conductivity between meshes and substracts the average along columns 
 * @details Calculation is done independenlty for each block of phiM corresponding to different mesh 
 * @note assumes column major ordering for phiM
 * 
 * @tparam T type of potential matrix phiM
 * @tparam T2 type of meshes
 * @tparam T3 type of conductivities
 * @param meshes vector of meshes, only used to find out the sizes of meshes
 * @param phiM Potential matrix, will be modified by the function
 * @param ci conductivities inside the meshes 
 * @param co conductivities outside the meshes 
 * @param average Whether to substract the average of each column from phiM (separately for all meshes)
 * @ingroup cpu_s_field
 */
template <typename T, typename T2, typename T3>
void WeightedPhi(const std::vector<Mesh<T2>> &meshes, 
                 MatrixXT<T> &phiM,
                 const std::vector<T3> &ci , 
                 const std::vector<T3> &co,
                 const bool average = true);
