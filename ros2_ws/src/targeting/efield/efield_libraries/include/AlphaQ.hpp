#pragma once 


#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include "eigen_defines.hpp"


/**
 * @brief Calculates the dotproducts of BetaDipoles moments and the Coil induced A-field on the same dipole positions
 * @details Result is used to calculate secondary E-field when multiplied with Potential Matrix Phi. (similarly to the result of BetaQ)
 * @ingroup cpu_s_field
 * 
 * @param coil Coil object
 * @param bd BetaDipoles object 
 * @return Alpha factors, similar to result of Eq. 8 (Stenroos & Koponen, Real-time computation of the TMS-induced electric field in a realistic head model, NeuroImage, 2019 Dec; 203:116159, doi: 10.1016/j.neuroimage.2019.116159.), but obtained in different way
 */
template <typename T>
VectorXT<T> AlphaQ(const Coil<T> &coil, const BetaDipoles<T> &bd);


void A_CurrentDipoleSet_dir(      float *p_A, 
                            const float *fpos, 
                            const float *bdmom, 
                            const int32_t Nfp, 
                            const float * cpos, 
                            const float * cmom, 
                            const int32_t Nsp);

void A_CurrentDipoleSet_dir(      double *p_A, 
                            const double *fpos, 
                            const double *bdmom, 
                            const int32_t Nfp, 
                            const double * cpos, 
                            const double * cmom, 
                            const int32_t Nsp);

