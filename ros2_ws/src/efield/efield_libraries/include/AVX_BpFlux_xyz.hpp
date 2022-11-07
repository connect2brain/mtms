#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>


#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include "Mesh.hpp"
#include <linalg>

/* 
Calculates B field flux through the coil caused by unit dipole triplets in sourcemesh points.

IN
- Class coil
- Class fieldmesh
Optionally specify the region of interest
- int32_t ROI_start: starting fieldpoint
- int32_t ROI_n    : How many field points to calculate
OUT
- Matrix<T, RowMajor> Bfinf: Bflux (excluding factor of mu0/4pi=1e-7) caused by ROI points, flattened into 1 dimension rowwise [3*ROI_n]

Defaults to using the whole field mesh if no arguments are given for the ROI

*/

void BpFlux_xyz(float *Bfinf, const float *spos, const int32_t Nsp, const float * cp, const float * cn,const int32_t Nfp);

void BpFlux_xyz(double *Bfinf, const double *spos, const int32_t Nsp, const double * cp, const double * cn ,const int32_t Nfp);


template <typename T>
Matrix<T, RowMajor> BpFlux_xyz(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex, const int32_t ROI_start, const int32_t ROI_n){

    if(coil.CoilType() != MagneticDipole){
        std::cout<<"Trying to call a magentic Dipole function BpFlux_xyz on a coil with type: "<< coil.CoilType() <<std::endl;
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

    BpFlux_xyz(Bfinf.Data(), spos, ROI_n, cp, cn, coil.Nop());

    return Bfinf;
 }


template <typename T>
Matrix<T, RowMajor> BpFlux_xyz(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex){
    return BpFlux_xyz(coil, cortex, 0, cortex.Rows());
}

template <typename T>
Matrix<T, RowMajor> BpFlux_xyz(const Coil<T> &coil, const Mesh<T> &sourcemesh, const int32_t ROI_start, const int32_t ROI_n){
    return BpFlux_xyz(coil, sourcemesh.Points(), ROI_start, ROI_n);
 }

template <typename T>
Matrix<T, RowMajor> BpFlux_xyz(const Coil<T> &coil, const Mesh<T> &sourcemesh){
    return BpFlux_xyz(coil, sourcemesh.Points());
}
