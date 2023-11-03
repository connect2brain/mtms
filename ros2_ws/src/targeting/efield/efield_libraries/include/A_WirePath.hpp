#pragma once 


#include <vector>
#include <cmath>
#include <stdexcept>

#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include "Mesh.hpp"
#include "eigen_defines.hpp"

// Choice of log implementation used: 
// -1 =  standard log implementation. 
// Other integer N > 1  = Taylor series with that many terms. 
// Note that the mathematical highest order term is then 2(N-1)+1 as the series only has odd powers 
// For example using logDegree=5 is much more efficient than to use C++ standard log function.
// Relative difference compared to log function is of the same order as numerical relative error with floats ~1e-6-1e-7
// As long as (|r'_i - r|+|r'_f - r|)/| r'_i - r'_f| > 1, i.e the field point is not on the coil segment
#define POLYLINE_LOG_DEGREE_CPU 5 
// Todo, maybe separate macros for floats and doubles 

/**
 * @brief Calculates the primary A field on a set of field points due to a polyline coil
 * @note Function explicitly uses AVX instructions
 * @note excludes the factor of mu0/4pi=1e-7
 * @note The  
 * @ingroup cpu_p_field
 * 
 * @param Ap Pointer to a preallocated array for the result
 * @param fp Pointer to an array of field positions
 * @param Nfp Number of field points
 * @param sp Pointer to an array of discretized coil positions
 * @param Nsp Number of source points (coil points)
 * @param logDegree Number of terms used in the Taylor series of ln(). Cpp standard log implementation is used with input of -1. Defaults to the value of macro POLYLINE_LOG_DEGREE_CPU.
 */
 template <typename T>
 void A_WirePath(T* Ap, const T* fp, int32_t Nfp, const T* sp, const int32_t Nsp, const int32_t logDegree = POLYLINE_LOG_DEGREE_CPU);
 
/**
* @brief Take dot product of the result of A-field with the vectors given in fdir
* @note Same calculation as A_WirePath_Secondary but more general interface
* @ingroup cpu_p_field
*/
template <typename T>
void A_WirePath_dir(T * Ap, const T *fp, const T *fdir, int32_t Nfp, const T * sp, const int32_t Nsp, const int32_t logDegree = POLYLINE_LOG_DEGREE_CPU);
    

/**
 * @brief Calculates the dotproducts of BetaDipoles moments and the Coil induced A-field on the same dipole positions
 * @details Result is used to calculate secondary E-field when multiplied with Potential Matrix Phi. (similarly to the result of BetaQ and AlphaQ)
 * @note Same calculation as A_WirePath_dir but specialised interface for TMS calculation
 * @ingroup cpu_s_field
 * 
 * @param coil Coil object
 * @param bd BetaDipoles object 
 * @param logDegree Number of terms used in the Taylor series of ln(). Cpp standard log implementation is used with input of -1. Defaults to the value of macro POLYLINE_LOG_DEGREE_CPU.
 * @return Alpha factors, similar to result of Eq. 8 (Stenroos & Koponen, Real-time computation of the TMS-induced electric field in a realistic head model, NeuroImage, 2019 Dec; 203:116159, doi: 10.1016/j.neuroimage.2019.116159.), but obtained in different way
 */
template <typename T>
VectorXT<T> A_WirePath_Secondary(const Coil<T> &coil, const BetaDipoles<T> &bd, const int32_t logDegree = POLYLINE_LOG_DEGREE_CPU);


/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_WirePath(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex, const int32_t ROI_start, const int32_t ROI_n, const int32_t logDegree = POLYLINE_LOG_DEGREE_CPU);

/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_WirePath(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex, const int32_t logDegree = POLYLINE_LOG_DEGREE_CPU){
    return A_WirePath(coil, cortex, 0, cortex.rows(), logDegree);
}
/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_WirePath(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t ROI_start, const int32_t ROI_n, const int32_t logDegree = POLYLINE_LOG_DEGREE_CPU){
    return A_WirePath(coil, sourcemesh.Points(), ROI_start, ROI_n, logDegree);
 }
/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_WirePath(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t logDegree = POLYLINE_LOG_DEGREE_CPU){
    return A_WirePath(coil, sourcemesh.Points(), logDegree);
}


