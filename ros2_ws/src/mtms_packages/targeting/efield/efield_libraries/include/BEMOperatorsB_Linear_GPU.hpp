#pragma once
#include <vector>
#include "Mesh.hpp"
#include "Coil.hpp"
#include "Coilset.hpp"
#include "eigen_defines.hpp"

/**
 * @file BEMOperatorsB_Linear_GPU.hpp
 * @brief GPU versions of functions in BEMOperatorsB_Linear.hpp with similar CPU interface
 * 
 */

std::vector<Eigen::MatrixXf> BEMOperatorsB_Linear_GPU(const std::vector<Mesh<float>> &meshes,
                                                  const MatrixX3f_RM &fpos, 
                                                  const MatrixX3f_RM &fdir);

std::vector<Eigen::MatrixXf> BEMOperatorsB_Linear_GPU(const std::vector<Mesh<float>> &meshes, const Coilset<float> &cset);



Eigen::VectorXf Beta_GPU(const std::vector<Mesh<float>> &meshes, const Coil<float> &coil);