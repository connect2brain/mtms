#include "TMS_CPU.hpp"
#include "TMS_CPU_dir.hpp"
#include "A_CurrentDipoleSet.hpp"
#include "AlphaQ.hpp"
#include "A_WirePath.hpp"
#include "BpFlux_xyz.hpp"
#include "BetaQ.hpp"
#include "BetaDipoles.hpp"
#include "BetaQDipoles.hpp"
#include "Coil.hpp"
#include "Coilset.hpp"
#include "Mesh.hpp"
#include "Timer.hpp"
#include "JsonIO.hpp"
#include "ResultsIO.hpp"
#include "WeightedPhi.hpp"
//DONT INCLUDE json.hpp

#ifdef USE_CUDA
    #include "GPU_helpers.hpp"
    #include "cudaCheck.hpp"
    #include "TMS_GPU.hpp"
    #include "TMS_GPU_dir.hpp"
#endif

/**
 * @defgroup cpu_s_field CPU Secondary field calculation
 * @{
 */
/** @} */

/**
 * @defgroup cpu_p_field CPU Primary field calculation
 * @{
 */
/**@}*/

/**
 * \defgroup  Cuda_p_field CUDA primary field calculation
 * @{
 */
/**@}*/

/**
 * \defgroup  Cuda_s_field CUDA secondary field calculation
 * @{
 */
/**@}*/

/**
 * \defgroup  util Utility and IO functions
 * @{
 */
/**@}*/

#include "eigen_defines.hpp" // configures macros etc. to be used with Eigen, thus this should be the first one
#include "Mesh.hpp"

// LC CPU headers:
#include "BEMOperatorsPhi_LC.hpp"
#include "TM_Phi_LC.hpp"
#include "Phi_inf_LC.hpp"

// LG CPU headers:
#include "BEMOperatorsPhi_D_LG.hpp"
#include "BEMOperatorsPhi_A_LG.hpp"
#include "TM_Phi_LG.hpp"
#include "Phi_inf_LG.hpp"

//Common headers, contain both LC and LG versions
#include "LFM_Phi.hpp"

// Direct solve and LU headers, LG and LC 
#include  "LFMLU_Phi.hpp"
#include  "LFMinv_Phi.hpp"
#include  "TMinv_Phi_LG.hpp"
#include  "TMinv_Phi_LC.hpp"
#include  "TMLU_Phi_LC.hpp"
#include  "TMLU_Phi_LG.hpp"

//DB 
#include   "BEMOperatorsB_Linear.hpp"
#include   "TM_Bvol_Linear.hpp"
#include   "LFM_B.hpp"
#include "TM_Phi_ISA.hpp"

// Misc
#include "Timer.hpp"

#ifdef USE_CUDA
    // Direct solve and LU headers, GPU versions, LG and LC 
    #include  "LFMLU_Phi_GPU.hpp"
    #include  "LFMinv_Phi_GPU.hpp"
    #include "TM_Phi_GPU.hpp" // Contains both LU and inversion versions
    #include "Phi_inf_GPU.hpp" // Eigen interface for Phi_inf_XX_GPU

    // LC GPU headers:
    #include "BEMOperatorsPhi_LC_GPU.hpp"
    #include "LFM_Phi_LC_GPU.hpp"

    // LG GPU headers:
    #include "BEMOperatorsPhi_D_LG_GPU.hpp"
    #include "LFM_Phi_LG_GPU.hpp"

    #include   "BEMOperatorsB_Linear_GPU.hpp"
    #include   "TM_Bvol_Linear_GPU.hpp"
    #include   "LFM_B_GPU.hpp"

    #include "TM_Phi_ISA_GPU.hpp"
#endif

// Group definitions

/**
 * @defgroup tm_group Transfer Matrices
 * @{
 */

/** @}*/

/**
 * @defgroup bem_group BEM Operators
 * @{
 */

/** @}*/

/**
 * @defgroup lfm_group Lead Field Matrices
 * @{
 */

/** @}*/