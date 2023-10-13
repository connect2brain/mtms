#pragma once

#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include "Coilset.hpp"
#include "Mesh.hpp"
#include <vector>
#include "eigen_defines.hpp"

/**
 * @brief Implements the approximate Beta integrals used to calculate the secondary magnetic field due to magnetic dipole coil
 * @ingroup cpu_s_field
 * @param coil Coil object
 * @param bd BetaDipoles object 
 * @return Beta-factors from Eq. 8 in Stenroos & Koponen, Real-time computation of the TMS-induced electric field in a realistic head model, NeuroImage, 2019 Dec; 203:116159, doi: 10.1016/j.neuroimage.2019.116159.
 */
template <typename T>
VectorXT<T> BetaQ(const Coil<T> &coil, const BetaDipoles<T> &bd);

/**
 * @brief 
 * 
 * @param Bfinf POinter to a result array (has to be preallocated)
 * @param spos  Source positions (betadipoles or cortex depending on usage)
 * @param sdir Source directions
 * @param Nsp Number of source poitns
 * @param cp Coil points
 * @param cn coil moments
 * @param Nfp number of coil points (field points)
 * @ingroup cpu_s_field
 */
void BpFlux_dir(float *Bfinf, const float *spos, const float *sdir, const int32_t Nsp, const float * cp, const float * cn, const int32_t Nfp);
void BpFlux_dir(double *Bfinf, const double *spos, const double *sdir, const int32_t Nsp, const double * cp, const double * cn, const int32_t Nfp);

/**
 * @brief Coilset version of above, returns separate result for each coil
 * 
 */
template <typename T>
MatrixXT<T> BpFlux_dir(const Coilset<T> &cset, const MatrixX3T_RM<T> &spos, const MatrixX3T_RM<T> &sdir);





// Accurate version according to Ferguson et al. "A Complete Linear Discretization for Calculating the Magnetic Field Using the Boundary Element Method"
//Eigen::VectorXf BetaQ_Acc(const Coil<float> &coil,  const std::vector<Mesh<float>> &meshes);