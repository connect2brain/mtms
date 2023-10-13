#pragma once

#include <string>
#include "Mesh.hpp"
#include "Coil.hpp"
#include "Coilset.hpp"
#include <vector>
//using namespace Eigen; // temp addition by Matti v230210 
// FOR USER
/**
 * @brief Function for loading head model and conductivities etc. using a json configuration file
 * @details Can be used to modify used data and run different models without need to recompile
 * @note parameters have to be declared before calling the function, but not allocated or initialised.
 * 
 * @param conf_path configuration file path (input)
 * @param bmeshes head model as vector of meshes (output)
 * @param cortex cortex as a Mesh (output)
 * @param ci conductivities inside the meshes (output)
 * @param co conductivities outside the meshes (output)
 * @return path to the data root directory given in configuration file
 * 
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       Mesh<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co);
/**
 * @brief As above, but cortex as a Matrix instead of Mesh
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       MatrixX3T_RM<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co);

/**
 * @brief Coil as additional argument
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       Mesh<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co,
                       Coil<T> &coil);
/**
 * @brief Coil as additional argument
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       MatrixX3T_RM<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co,
                       Coil<T> &coil);


/**
 * @brief Coilset as additional argument
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       Mesh<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co,
                       Coilset<T> &coilset);
/**
 * @brief Coilset as additional argument
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       MatrixX3T_RM<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co,
                       Coilset<T> &coilset);


//additions by Matti v230210 
/**
 * @brief Coil and coil transformations file as additional arguments
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       Mesh<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co,
                       Coil<T> &coil,
                       MatrixXf_RM &ctransset);
/**
 * @brief Coil and coil transformation file as additional arguments
 * @ingroup util
 */
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       MatrixX3T_RM<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co,
                       Coil<T> &coil,
                       MatrixXf_RM &ctransset);


