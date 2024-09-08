#pragma once
#include <vector>
#include "Mesh.hpp"
#include "Coil.hpp"
#include "Coilset.hpp"
#include "eigen_defines.hpp"
/**
 * @brief Make DB matrices for linear-basis BEM
 * 
 * @param meshes head geometry as vector of meshes
 * @param fpos field points (coil points)
 * @param fdir field directions (coil moments)
 * @return vector of DB matrices, for each mesh [fpos.rows() x meshes[i].Nop()]
 */
std::vector<Eigen::MatrixXf> BEMOperatorsB_Linear(const std::vector<Mesh<float>> &meshes,
                                                  const MatrixX3f_RM &fpos, 
                                                  const MatrixX3f_RM &fdir);
//For a single coil, TODO
//Eigen::VectorXf BetaQ_Acc(const Coil<float> &coil,  const std::vector<Mesh<float>> &meshes);


/**
 * @brief Make DB matrices for linear-basis BEM
 * 
 * @param meshes head geometry as vector of meshes
 * @param cset Coilset object
 * @return vector of DB matrices, for each mesh [cset.Noc() x meshes[i].Nop()]
 */
std::vector<Eigen::MatrixXf> BEMOperatorsB_Linear(const std::vector<Mesh<float>> &meshes, const Coilset<float> &cset);


/**
 * @brief Calculate Accurate Beta integrals for single Coil using BEMOperatorsB_Linear
 * 
 * @param meshes head geometry as vector of meshes
 * @param cset Coilset object
 * @return vector of Beta factors [Total number of meshpoints]
 */
Eigen::VectorXf Beta(const std::vector<Mesh<float>> &meshes, const Coil<float> &coil);