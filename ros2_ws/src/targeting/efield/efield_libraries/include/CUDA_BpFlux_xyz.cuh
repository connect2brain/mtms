#pragma once

#include <stdexcept>
#include <stdio.h>
#include <iostream>
#include <cuda_runtime.h>

#include <linalg>
#include "Coil.hpp"
#include "Mesh.hpp"


void d_CUDA_BpFlux_xyz(float *d_Bp, float *d_field, int32_t ROI_n, float *d_sources, int32_t Nsp, float *d_normals, cudaStream_t stream);


void CUDA_BpFlux_xyz(float *B_field, const float *fpos, const int32_t Nfp, const float * cpos, const float * cmom, const int32_t Nsp);

 /*
Calculates the Magnetic flux though a coil caused by unit magnetic dipole triplets at positions specified by the sourcemesh, separately for each triplet  

(excluding factor of mu0/4pi=1e-7)

Handles all communication with the GPU.  Reserves and frees GPU memory on each call to the function. 
When calling the function multiple times on the same (large) set of sourcepoints, consider using the version CUDA_BpFlux_xyz() for better performance

ROI_start and ROI_n parameters are optional, if none are given the function will default to calculating the flux due to all points of sourcemesh
*/
template <typename T>
Matrix<T, RowMajor> CUDA_BpFlux_xyz(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex, const int32_t ROI_start, const int32_t ROI_n){

    if(coil.CoilType() != MagneticDipole){
        std::cout<<"Trying to call a magentic Dipole function CUDA_BpFlux_xyz on a coil with type: "<< coil.CoilType() <<std::endl;
        throw std::invalid_argument("----MIXED COIL TYPES----");
    }

    if ((ROI_n + ROI_start) > cortex.Rows() ){
        std::cout<<"Trying to access element: " << (ROI_n + ROI_start)*3  <<"in array with length " << cortex.Rows()*3<< std::endl;
	    throw std::invalid_argument("Invalid ROI dimensions! \n");
    }
    if(ROI_start < 0 || ROI_n < 0){
        throw std::invalid_argument("Invalid ROI dimensions! Negative ROI dimensions \n");
    }

    //pointer to the start of ROI
    const T * spos = cortex.Data()  + ROI_start*3;
    const T *cp = coil.Points().Data();
    const T *cn = coil.Moments().Data();

    
    Matrix<T, RowMajor> Bfinf(ROI_n,3);

    CUDA_BpFlux_xyz(Bfinf.Data(), spos, ROI_n, cp, cn, coil.Nop());

    return Bfinf;
 }


template <typename T>
Matrix<T, RowMajor> CUDA_BpFlux_xyz(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex){
    return CUDA_BpFlux_xyz(coil, cortex, 0, cortex.Rows());
}

template <typename T>
Matrix<T, RowMajor> CUDA_BpFlux_xyz(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t ROI_start, const int32_t ROI_n){
    return CUDA_BpFlux_xyz(coil, sourcemesh.Points(), ROI_start, ROI_n);
 }

template <typename T>
Matrix<T, RowMajor> CUDA_BpFlux_xyz(const Coil<T> &coil, const Mesh<T> &sourcemesh){
    return CUDA_BpFlux_xyz(coil, sourcemesh.Points());
}
