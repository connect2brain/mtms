#pragma once

#include "eigen_defines.hpp"
#include <vector>
#include <string>

/**
 * @brief Function for loading multiple vector fields from a file into Row-major matrices without unnecessary transposing
 * @note The file has be previously saved using saveResults() function. Loading usual Eigen matrix binary file with this function might result in unexpected behaviour
 * @param res vector of matrices for the result. Declared beforehand.
 * @param filename Path to the binary file
 * @ingroup util
 */
template <typename T>
void loadResults(std::vector<MatrixX3T_RM<T>> &res, std::string filename);


/**
 * @brief Function for saving multiple vector fields from a file into Row-major matrices without unnecessary transposing
 * @param res vector of matrices to be saved
 * @param filename Path to the binary file
 * @ingroup util
 */
template <typename T>
void saveResults(std::vector<MatrixX3T_RM<T>> &res, std::string filename);