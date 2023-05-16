// Header for including all jeaders not shared with the library
#pragma once

#include "D_LC.hpp"
#include "D_LG.hpp"
#include "DB_Linear.hpp"
#include "LAPACKE_Wrappers.hpp"
#include "mesh_SourceVals_D_LC.hpp"
#include "gaussTables.hpp"
#include "quadpoints.hpp"
#include "shapes_D_LC.hpp"
#include "shapes_Dvec_LC.hpp"
#include "SourceVals_D_LC.hpp"
#include "SoureVals_Dvec_LC.hpp"
#include "A_LG.hpp"

#ifdef USE_CUDA
    #include "CUBLAS_Wrappers.hpp"
    #include "cudaCheck.hpp"
    #include "D_LC_GPU.cuh"
    #include "D_LG_GPU.cuh"
    #include "DB_Linear_GPU.cuh"
    #include "Gemm_GPU.hpp"
    #include "Phi_inf_LC_GPU.cuh"
    #include "Phi_inf_LG_GPU.cuh"
    #include "cudaCheck.hpp"
    #include "setIdentity.cuh"
#endif