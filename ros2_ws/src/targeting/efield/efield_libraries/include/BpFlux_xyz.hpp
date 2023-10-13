#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>
#include "Coil.hpp"
#include "Mesh.hpp"
#include "eigen_defines.hpp"
#include "Coilset.hpp"

/**
 * @brief Calculates the primary B field flux through the coil caused by unit dipole triplets in sourcemesh points.
 * @note Function explicitly uses AVX instructions
 * @note excludes the factor of mu0/4pi=1e-7
 * 
 * @param Bfinf Pointer to a preallocated array for the result
 * @param spos Pointer to an array of source positions, i.e. the head model meshpoints
 * @param Nsp Number of source points
 * @param cpos Pointer to an array of coil dipole positions
 * @param cmom Pointer to an array of coil dipole moments
 * @ingroup cpu_p_field
 * @param Nfp Number of field points (coil points)
 */
void BpFlux_xyz(float *Bfinf, const float *spos, const int32_t Nsp, const float * cpos, const float * cmom,const int32_t Nfp);

/**
 * @brief Overload with doubles
 * @ingroup cpu_p_field
 * 
 */
void BpFlux_xyz(double *Bfinf, const double *spos, const int32_t Nsp, const double * cpos, const double * cmom ,const int32_t Nfp);


/**
 * @overload
 * @brief choose contiguous ROI
 * @param coil a Coil object
 * @param cortex cortex as a matrix
 * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
 * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
 * @ingroup cpu_p_field
 * @return Resulting B field flux
 */
template <typename T>
MatrixX3T_RM<T> BpFlux_xyz(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex, const int32_t ROI_start, const int32_t ROI_n){

    if(coil.CoilType() != MagneticDipole){
        std::cout<<"Trying to call a magentic Dipole function BpFlux_xyz on a coil with type: "<< coil.CoilType() <<std::endl;
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

    
    MatrixX3T_RM<T> Bfinf(ROI_n,3);

    BpFlux_xyz(Bfinf.data(), spos, ROI_n, cpos, cmom, coil.Nop());

    return Bfinf;
 }

/**
 * @overload
 * @brief Defaults to calculating on whole cortex
 * 
 * @param coil a Coil object
 * @param cortex cortex as a matrix
 * @ingroup cpu_p_field
 * @return Resulting B field flux
 */
template <typename T>
MatrixX3T_RM<T> BpFlux_xyz(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex){
    return BpFlux_xyz(coil, cortex, 0, cortex.rows());
}


/**
 * @overload
 * @brief choose contiguous ROI
 * @param coil a Coil object
 * @param cortex cortex as a Mesh, only points are used
 * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
 * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
 * @ingroup cpu_p_field
 * @return Resulting B field flux
 */
template <typename T>
MatrixX3T_RM<T> BpFlux_xyz(const Coil<T> &coil, const Mesh<T> &cortex, const int32_t ROI_start, const int32_t ROI_n){
    return BpFlux_xyz(coil, cortex.Points(), ROI_start, ROI_n);
 }

/**
 * @overload
 * @brief Defaults to calculating on whole cortex
 * @param coil a Coil object
 * @param cortex cortex as a Mesh, only points are used
 * @ingroup cpu_p_field
 * @return Resulting B field flux
 */
template <typename T>
MatrixX3T_RM<T> BpFlux_xyz(const Coil<T> &coil, const Mesh<T> &cortex){
    return BpFlux_xyz(coil, cortex.Points());
}


template <typename T>
MatrixXT<T> BpFlux_xyz(const Coilset<T> &cset, const MatrixX3T_RM<T> &cortex){
    if(cset.CoilType() != MagneticDipoleSet){
        std::cout<<"Trying to call a magentic Dipole function BpFlux_dir on a coil with type: "<< cset.CoilType() <<std::endl;
        throw std::invalid_argument("----MIXED COIL TYPES----");
    }
    MatrixXT<T> Bfinf(3*cortex.rows(), cset.Noc());
    const Eigen::VectorXi &startinds =cset.Startinds();
    const Eigen::VectorXi &sizes     =cset.Sizes();
    const T * cpos = cset.Points().data();
    const T * cmom = cset.Moments().data();

    for(int32_t i = 0; i < cset.Noc(); i++){
        // this is slow as  Bfinf and TBvol are Colmajor..
        // This automatically flattens the Nx3 matrix and puts it on one row
        BpFlux_xyz(&Bfinf(0,i), cortex.data(), cortex.rows(), cpos + 3*startinds(i) ,cmom + 3*startinds(i) , sizes(i));
    }
    return Bfinf;
}


template <typename T>
MatrixXT<T> BpFlux_xyz(const Coilset<T> &cset, const Mesh<T> &cortex){
    return BpFlux_xyz(cset, cortex.Points());
}