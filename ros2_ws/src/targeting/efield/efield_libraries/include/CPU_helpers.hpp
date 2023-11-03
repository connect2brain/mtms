#pragma once
#include "eigen_defines.hpp"
#include "Coil.hpp"
#include "Mesh.hpp"
#include "BetaDipoles.hpp"

/**
 * @defgroup cpu_p_field CPU Primary field calculation
 * @{
 */
/**@}*/

/**
 * @brief A helper function used by the TMS class to automatically choose the correct primary field calculation function depending on the coil type
 * @details Calls either BpFlux_xyz, A_CurrentDipoleSet, or A_WirePath
 * @note excludes the factor of mu0/4pi=1e-7
 *
 * @param coil Coil object
 * @param cortex cortex as an Eigen matrix
 * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
 * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
 * @return The primary A field on cortex or B-field flux through the Coil depending on CoilType
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> TMS_CPU_PrimaryField(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex, const int32_t ROI_start, const int32_t ROI_n);

/**
 * @brief Defaults to using whole cortex
 * @ingroup cpu_p_field
 */
template <typename T>
MatrixX3T_RM<T> TMS_CPU_PrimaryField(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex);


template <typename T>
VectorXT<T> TMS_CPU_PrimaryField_dir(const Coil<T> &coil, const MatrixX3T_RM<T> &cortex, const MatrixX3T_RM<T> &dirs, const int32_t ROI_start, const int32_t ROI_n);


/**
 * @ingroup cpu_s_field
 * @brief A helper function used by the TMS class to automatically choose the correct secondary field calculation function depending on the coil type
 * @details Calls either BetaQ, AlphaQ, or A_WirePath_Secondary
 * @note excludes the factor of mu0/4pi=1e-7
 * 
 * @param coil Coil object
 * @param bd BetaDipoles object
 * @return Beta-factors to be multiplied by potential matrix
 */
template <typename T>
VectorXT<T> TMS_CPU_SecondaryField(const Coil<T> &coil, const BetaDipoles<T> &bd);

/**
 * @brief A helper function for filtering a coordinate array when choosing the ROI with an index list. Chooses rows defined by indlist
 * 
 * @param coords Coordinate array to be filtered (Eigen matrix)
 * @param indlist indices (rows) to keep
 * @return Filtered array (Eigen matrix)
 */
template <typename T>
MatrixX3T_RM<T> filterByIndices(const MatrixX3T_RM<T> &coords, const Eigen::VectorXi &indlist);
template <typename T>
VectorXT<T> filterByIndices(const VectorXT<T> &coords, const Eigen::VectorXi &indlist);

/**
 * @brief Calculates the condition number of a square matrix
 * 
 * @param p_A  Pointer to the matrix
 * @param rows number of rows
 * @param norm type of norm used, '1' for 1-norm, 'I' for inf-norm 
 * @return float 
 */
float Mcond(const float *A_in, int32_t rows, char norm);



template <typename T>
void array_dotp(T* p_res ,const T* p_A, const T* p_B, const int32_t N);
template <typename T>
void array_dotp_indlist(T* p_res ,const T* p_A, const T* p_B, const int32_t N, const int32_t* indlist);
template <typename T>
VectorXT<T> array_dotp(const MatrixX3T_RM<T> &A, const MatrixX3T_RM<T> &B);


// Helper function that returns the permutation of sorting indexlist (but does not actually do the sorting on it)
Eigen::VectorXi get_permutation_indlist(const Eigen::VectorXi &indlist);

// Helper function that reverses the permutation perm earlier applied to sorted and returns the un-permuted array

template <typename T>
VectorXT<T> reverse_permutation(VectorXT<T> &sorted, const Eigen::VectorXi &perm);

template <typename T>
MatrixX3T_RM<T> reverse_permutation(MatrixX3T_RM<T> &sorted, const Eigen::VectorXi &perm);

/**
 * @brief Debug function for printing string out in hexadecimal
 * 
 */
void str_print_hex(const std::string &str);
