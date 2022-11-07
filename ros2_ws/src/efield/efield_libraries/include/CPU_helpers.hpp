#pragma once
#include <linalg>
#include "Coil.hpp"
#include "Mesh.hpp"
#include "BetaDipoles.hpp"

// A helper function used by the TMS class to automatically choose the correct field calculation function depending on the coil type

template <typename T>
Matrix<T, RowMajor> TMS_CPU_PrimaryField(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex, const int32_t ROI_start, const int32_t ROI_n);

template <typename T>
Matrix<T, RowMajor> TMS_CPU_PrimaryField(const Coil<T> &coil, const Matrix<T, RowMajor> &cortex);

// A helper function used by the TMS class to automatically choose the correct field calculation function depending on the coil type
template <typename T>
ColVector<T> TMS_CPU_SecondaryField(const Coil<T> &coil, const BetaDipoles<T> &bd);

template <typename T>
Matrix<T, RowMajor> filterByIndices(const Matrix<T, RowMajor> &coords, const ColVector<int32_t> &indlist);
