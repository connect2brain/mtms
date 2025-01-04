#pragma once 

#include <stdexcept>
#include "Coil.hpp"
#include "Mesh.hpp"
#include "eigen_defines.hpp"



/**
 * @brief Calculates the primary vector potential A on a set of field points due to current dipole moments
 * @note Function explicitly uses AVX instructions
 * @note excludes the factor of mu0/4pi=1e-7
 * @ingroup cpu_p_field
 * 
 * @param A_field Pointer to a preallocated array for the result
 * @param fpos Pointer to an array of field positions
 * @param Nfp Number of field points
 * @param cpos Pointer to an array of coil dipole positions
 * @param cmom Pointer to an array of coil dipole moments
 * @param Nsp Number of source points (coil points)
 */
void A_CurrentDipoleSet(float *A_field, const float *fpos, const int32_t Nfp, const float * cpos, const float * cmom, const int32_t Nsp);

/**
 * @brief overload for doubles
 * @ingroup cpu_p_field
 */
void A_CurrentDipoleSet(double * A_field, const double *fpos, const int32_t Nfp, const double * cpos, const double * cmom, const int32_t Nsp);

/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_CurrentDipoleSet(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex, const int32_t ROI_start, const int32_t ROI_n){

     if(coil.CoilType() != CurrentDipole){
        std::cout<<"Trying to call a current Dipole function A_CurrentDipoleSet on a coil with type: "<< coil.CoilType() <<std::endl;
        throw std::invalid_argument("----MIXED COIL TYPES----");
    }

    if ((ROI_n + ROI_start) > cortex.rows() ){
        std::cout<<"Trying to access element: " << (ROI_n + ROI_start)*3  <<"in array with length " << cortex.rows()*3<< std::endl;
	    throw std::invalid_argument("Invalid ROI dimensions! \n");
    }
    if(ROI_start < 0 || ROI_n < 0){
        throw std::invalid_argument("Invalid ROI dimensions! Negative ROI dimensions \n");
    }

    //pointer to the start of ROI
    const T * spos = cortex.data()  + ROI_start*3;
    const T *cpos = coil.Points().data();
    const T *cmom = coil.Moments().data();

    
    MatrixX3T_RM<T> Ap(ROI_n,3);

    A_CurrentDipoleSet(Ap.data(), spos, ROI_n, cpos, cmom, coil.Nop());

    return Ap;
 }

/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_CurrentDipoleSet(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex){
    return A_CurrentDipoleSet(coil, cortex, 0, cortex.rows());
}
/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_CurrentDipoleSet(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t ROI_start, const int32_t ROI_n){
    return A_CurrentDipoleSet(coil, sourcemesh.Points(), ROI_start, ROI_n);
 }
/**
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> A_CurrentDipoleSet(const Coil<T> &coil, const Mesh<T> &sourcemesh){
    return A_CurrentDipoleSet(coil, sourcemesh.Points());
}