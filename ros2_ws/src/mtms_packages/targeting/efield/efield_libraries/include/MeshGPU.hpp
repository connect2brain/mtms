#pragma once

#include "MatrixGPU.hpp"

/**
 * @brief A container struct for holding data related to a mesh on GPU
 * 
 */
template<typename T>
struct MeshGPU {
    MatrixGPU<T, Rowmajor> points; /**<  MatrixGPU for mesh points on GPU  [nop x 3] */
    MatrixGPU<int32_t, Rowmajor> elements; /**<  MatrixGPU for mesh elements on GPU  [noe x 3] */
    int32_t nop, noe;  /**<  Number of points and elements (triangles)*/
};


/**
 * @brief Create a MeshGPU from CPU data
 * 
 * @param pointsCPU Pointer to meshpoint array on CPU
 * @param elementsCPU Pointer to mesh elements array on CPU
 * @param nop Number of points
 * @param noe Number of elements
 * @return MeshGPU<T> struct, has to be explicitly freed 
 */
template<typename T>
MeshGPU<T> createMeshGPU(const T *pointsCPU, const int32_t *elementsCPU, int32_t nop, int32_t noe) {
    MeshGPU<T> mesh;
    mesh.noe = noe;
    mesh.nop = nop;
    mesh.points = MatrixGPU_malloc<T, Rowmajor>(nop, 3);
    copyToGPU(mesh.points, pointsCPU);
    mesh.elements = MatrixGPU_malloc<int32_t, Rowmajor>(noe, 3);
    copyToGPU(mesh.elements, elementsCPU);
    return mesh;
}

/**
 * @brief Free mesh data
 * 
 * @param mesh Mesh to be freed
 */
template<typename T>
void free(MeshGPU<T> mesh) {
    free(mesh.points);
    free(mesh.elements);
}