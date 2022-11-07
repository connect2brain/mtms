#pragma once

#include <stdexcept>
#include <iostream>
#include <cuda_runtime.h>

#include <linalg>
#include "Coil.hpp"
#include "Mesh.hpp"

/* 
Calculates vector potential A on fieldmesh points due to the current dipole moments in the Coil struct.

Arguments are GPU pointers. "stream" can be set to NULL for basic usage
*/
void d_CUDA_A_CurrentDipoleSet(float *d_Ap, float *d_field, int32_t Nfp, float *d_sources, int32_t Nsp, float *d_normals, cudaStream_t stream);


//CPU pointer version, handles memory allocations and transfers
void CUDA_A_CurrentDipoleSet(float *A_field, const float *fpos, const int32_t Nfp, const float * cpos, const float * cmom, const int32_t Nsp);

/*
Calculates the vector potential on fieldmesh caused by a current dipole coil 

(excluding factor of mu0/4pi=1e-7).

Handles all communication with the GPU.  Reserves and frees GPU memory on each call to the function.
When calling the function multiple times on the same (large) set of sourcepoints, consider using the version CUDA_A_CurrentDipoleSet() for better performance

ROI_start and ROI_n parameters are optional, if none are given the function will default to calculating the vector potential on all points of fieldmesh
*/
template <typename T>
Matrix<T, RowMajor> CUDA_A_CurrentDipoleSet(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex, const int32_t ROI_start, const int32_t ROI_n){

     if(coil.CoilType() != CurrentDipole){
        std::cout<<"Trying to call a current Dipole function CUDA_A_CurrentDipoleSet on a coil with type: "<< coil.CoilType() <<std::endl;
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
    const T *cpos = coil.Points().Data();
    const T *cmom = coil.Moments().Data();

    
    Matrix<T, RowMajor> Ap(ROI_n,3);

    CUDA_A_CurrentDipoleSet(Ap.Data(), spos, ROI_n, cpos, cmom, coil.Nop());

    return Ap;
 }


template <typename T>
Matrix<T, RowMajor> CUDA_A_CurrentDipoleSet(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex){
    return CUDA_A_CurrentDipoleSet(coil, cortex, 0, cortex.Rows());
}

template <typename T>
Matrix<T, RowMajor> CUDA_A_CurrentDipoleSet(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t ROI_start, const int32_t ROI_n){
    return CUDA_A_CurrentDipoleSet(coil, sourcemesh.Points(), ROI_start, ROI_n);
 }

template <typename T>
Matrix<T, RowMajor> CUDA_A_CurrentDipoleSet(const Coil<T> &coil, const Mesh<T> &sourcemesh){
    return CUDA_A_CurrentDipoleSet(coil, sourcemesh.Points());
}