#pragma once 


#include <vector>
#include <cmath>
#include <stdexcept>

#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include "Mesh.hpp"
#include <linalg>


 //float version
 void A_WirePath(float * Ap, const float *fp, int32_t Nfp, const float * sp, const int32_t Nsp);
 

 //double version
 void A_WirePath(double * Ap, const double *fp, int32_t Nfp, const double * sp, const int32_t Nsp);

// For secondary field in TMS calculation
template <typename T>
ColVector<T> A_WirePath_Secondary(const Coil<T> &coil, const BetaDipoles<T> &bd);

template <typename T>
Matrix<T, RowMajor> A_WirePath(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex, const int32_t ROI_start, const int32_t ROI_n){

     if(coil.CoilType() != Polyline){
        std::cout<<"Trying to call Polyline function A_WirePath on a coil with type: "<< coil.CoilType() <<std::endl;
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

    Matrix<T, RowMajor> Ap(ROI_n,3);

    A_WirePath(Ap.Data(), spos, ROI_n, cpos, coil.Nop());

    return Ap;
 }


template <typename T>
Matrix<T, RowMajor> A_WirePath(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex){
    return A_WirePath(coil, cortex, 0, cortex.Rows());
}

template <typename T>
Matrix<T, RowMajor> A_WirePath(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t ROI_start, const int32_t ROI_n){
    return A_WirePath(coil, sourcemesh.Points(), ROI_start, ROI_n);
 }

template <typename T>
Matrix<T, RowMajor> A_WirePath(const Coil<T> &coil, const Mesh<T> &sourcemesh){
    return A_WirePath(coil, sourcemesh.Points());
}


