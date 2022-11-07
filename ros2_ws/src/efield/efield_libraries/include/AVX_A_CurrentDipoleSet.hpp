#pragma once 

#include <stdexcept>
#include "Coil.hpp"
#include "Mesh.hpp"
#include <linalg>


/* 
Calculates vector potential A on fieldmesh points due to the current dipole moments in the Coil struct.

IN
- Class coil
- Class fieldmesh
Optionally specify the region of interest
- int32_t ROI_start: starting fieldpoint
- int32_t ROI_n    : How many field points to calculate
OUT
- Matrix<T, RowMajor> A_field: Vector potential (excluding factor of mu0/4pi=1e-7) in ROI points, flattened into 1 rowwise [3*ROI_n]

Defaults to using the whole field mesh if no arguments are given for the ROI
*/

//float version
void A_CurrentDipoleSet(float *A_field, const float *fpos, const int32_t Nfp, const float * cpos, const float * cmom, const int32_t Nsp);

//double version
void A_CurrentDipoleSet(double * A_field, const double *fpos, const int32_t Nfp, const double * cpos, const double * cmom, const int32_t Nsp);


template <typename T>
Matrix<T, RowMajor> A_CurrentDipoleSet(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex, const int32_t ROI_start, const int32_t ROI_n){

     if(coil.CoilType() != CurrentDipole){
        std::cout<<"Trying to call a current Dipole function A_CurrentDipoleSet on a coil with type: "<< coil.CoilType() <<std::endl;
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

    A_CurrentDipoleSet(Ap.Data(), spos, ROI_n, cpos, cmom, coil.Nop());

    return Ap;
 }


template <typename T>
Matrix<T, RowMajor> A_CurrentDipoleSet(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex){
    return A_CurrentDipoleSet(coil, cortex, 0, cortex.Rows());
}

template <typename T>
Matrix<T, RowMajor> A_CurrentDipoleSet(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t ROI_start, const int32_t ROI_n){
    return A_CurrentDipoleSet(coil, sourcemesh.Points(), ROI_start, ROI_n);
 }

template <typename T>
Matrix<T, RowMajor> A_CurrentDipoleSet(const Coil<T> &coil, const Mesh<T> &sourcemesh){
    return A_CurrentDipoleSet(coil, sourcemesh.Points());
}