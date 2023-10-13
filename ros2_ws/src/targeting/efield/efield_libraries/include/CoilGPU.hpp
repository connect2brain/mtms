#pragma once

#include "MatrixGPU.hpp"
#include "memcopyWithPadding.hpp"


/**
 * @brief A container struct for holding data related to a coil on GPU
 * 
 */
template <typename T>
struct CoilGPU {
    MatrixGPU<T,Rowmajor> points;  /**<  MatrixGPU for coil points on GPU  [nop x 3] */
    MatrixGPU<T,Rowmajor> moments; /**<  MatrixGPU for coil moments on GPU  [nop x 3] */
    int32_t nop;                   /**<  number of points / dipoles */
    int32_t coiltype;              /**<  enumeration CoilType as defined in file Coil.hpp */
};



/**
 * @brief Create a CoilGPU from CPU data
 * 
 * @param pointsCPU Pointer to the coil dipole positions on CPU
 * @param momentsCPU Pointer to the coil dipole moments on CPU
 * @param nop  Number of points /dipoles
 * @param coiltype enumeration CoilType as defined in file Coil.hpp
 * @return CoilGPU struct, has to be explicitly freed 
 */
template <typename T>
CoilGPU<T> createCoilGPU(const T* pointsCPU, const T* momentsCPU, int32_t nop, int32_t coiltype) {
    CoilGPU<T> coil;
    coil.nop = nop;
    coil.coiltype = coiltype;
    coil.points = createMatrixGPU_padded(pointsCPU, nop, 3, 1);
    if(coiltype != 102){//If not polyline
        coil.moments = createMatrixGPU_padded(momentsCPU, nop, 3, 1);
    }else{
        coil.moments = { 0,0, NULL };
    }
    return coil;
}



/**
 * @brief Free GPU memory associated with the CoilGPU struct
 * @param coil CoilGPU struct to be freed 
 */
template <typename T>
void free(CoilGPU<T> coil) {
    free(coil.points);
    if(coil.coiltype != 102){ //If not polyline
        free(coil.moments);
    }
}