#pragma once

#include <string>
#include <tuple>
#include "eigen_defines.hpp"

/**
 * @brief A class representing a triangular mesh
 * @details Stores 2 matrices: \n 
 *    - points_ :   The triangle vertices. [nop x 3], row-major. \n 
 *    - elements_ : The triangle element connectivity list, as a list of indices pointing to rows of 'points_'. [noe_ x 3], row-major.
 */
template<typename T>
class Mesh {
public:
    /**
     * @brief Construct a new Mesh object from file
     * 
     * @param meshfile Path to the binary file
     */
    Mesh(const std::string &meshfile);
    /**
     * @brief Construct an empty Mesh object
     */
    Mesh() = default;
    /**
     * @brief Returns the number of points in the mesh
     */
    int32_t Nop() const { return nop_; }
    /**
     * @brief Returns the number of elements (triangles) in the mesh
     */
    int32_t Noe() const { return noe_; }
    /**
     * @brief Returns a meshtype identifier, currently only one type is used
     */
    int32_t MeshType() const { return meshtype_; }
    /**
     * @brief Returns a reference to the mesh points matrix
     */
    const MatrixX3T_RM<T> &Points() const { return points_; }
    /**
     * @brief Returns a reference to the mesh elements matrix
     */
    const MatrixX3i_RM &Elements() const { return elements_; }

private:
    int32_t nop_, noe_, meshtype_;
    MatrixX3T_RM<T> points_;
    MatrixX3i_RM elements_; 
};

/**
 * @brief Helper to calculate triangle unit normals
 * 
 * @param mesh Mesh object
 * @return Normals [Noe x 3]
 */
template <typename T>
MatrixX3T_RM<T> UnitNormals(const Mesh<T> &mesh);

/**
 * @brief Helper function to avoid repetition. Returns the total number of points in all meshes, maximum number of points and maximum number of elements
 * @param meshes Vector of meshes
 * @return std::tuple<int32_t, int32_t, int32_t> 
 */
template <typename T>
std::tuple<int32_t, int32_t, int32_t, int32_t> MeshTotals(const std::vector<Mesh<T>> &meshes);


/**
 * @brief Find total number of meshpints and list of mesh starting indices when all meshes are concatenated
 * 
 * @param meshes vector of meshes
 * @param start_ind vector of indices, will be modified by the function
 * @return Total number of mesh points
 */
template <typename T>
int32_t NodeIndices(const std::vector<Mesh<T>> &meshes, std::vector<int32_t> &start_ind);

/**
 * @brief Concatenate all mesh points into one [Ntot x 3] array
 * 
 * @param meshes vector of meshes
 * @return MatrixX3f_RM All msh points
 */
template <typename T>
MatrixX3T_RM<T> ConcatPoints(const std::vector<Mesh<T>> &meshes);

