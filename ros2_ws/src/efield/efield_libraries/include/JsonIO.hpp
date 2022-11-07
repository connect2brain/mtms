#pragma once

#include <string>
#include <fstream>
#include <map>
#include "Mesh.hpp"
#include <vector>

// FOR USER
// First argument is input for configuration file path others are output which have to be declared beforehand(becomes difficult to output many objects from single function in C++)
template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       Mesh<T> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co);

template <typename T>
std::string loadConfiguration(std::string conf_path,
                       std::vector<Mesh<T>> &bmeshes,
                       Matrix<T, RowMajor> &cortex,
                       std::vector<T> &ci,
                       std::vector<T> &co);


