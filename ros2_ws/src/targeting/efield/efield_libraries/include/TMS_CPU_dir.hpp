#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#include "Coil.hpp"
#include "Mesh.hpp"
#include "eigen_defines.hpp"
#include "BetaDipoles.hpp"

/** 
*  @brief Class for calculating the Efield in TMS using only CPU
*  @note Templated such that potential matrix can have different type than other inputs (defaults to float)
*  @note Result will be of type T (other inputs). If Potential matrix is given as floats it will be cast to doubles in the multiplication
*  @note WARNING: TMS and TMS_GPU classes do not have copy or move constructors as they are not meant to be given as argument to functions or copied between scopes
*  @note The source directions are already given in the constructor as they are tied to the Potential matrix used which cannot be changed after creating TMS object
*/
template <typename T = float, typename T2 = float>
class TMS_dir {
    public:
        /**
       * @brief Constructor for the TMS_dir class
       * @details Handles necessary preprocessing (BetaQDipoles) and memory allocations
       * @param Phi Weighted Potential Matrix [Nmesh x (3*Ncortex)]
       * @param bmeshes vector of meshes
       * @param cortex cortex as a Mesh, only the points will be used
       */
        TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const std::vector<Mesh<T>> &bmeshes,  const Mesh<T> &cortex,  const MatrixX3T_RM<T> &sdir);
        /**
       * @overload
       * @param Phi Weighted Potential Matrix [Nmesh x (3*Ncortex)]
       * @param bmeshes vector of meshes
       * @param cortex cortex as a [Ncortex x 3] matrix
       */
        TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const std::vector<Mesh<T>> &bmeshes,  const MatrixX3T_RM<T> &cortex,  const MatrixX3T_RM<T> &sdir);
        /**
       * @overload
       * @details Using the constructors with TM as input saves time in previous LFM part, results in an extra matrix vector multiplication in each Efield call 
       * @param Phi Unweighted Potential Matrix [Nmesh x (3*Ncortex)]
       * @param TM Weighted Transfer matrix     [Nmesh x Nmesh]
       * @param bmeshes vector of meshes
       * @param cortex cortex as a Mesh, only the points will be used
       */
        TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &TM, const std::vector<Mesh<T>> &bmeshes,  const Mesh<T> &cortex,  const MatrixX3T_RM<T> &sdir);
        /**
       * @overload
       * @details Using the constructors with TM as input saves time in previous LFM part, results in an extra matrix vector multiplication in each Efield call 
       * @param Phi Unweighted Potential Matrix [Nmesh x (3*Ncortex)]
       * @param TM Weighted Transfer matrix     [Nmesh x Nmesh]
       * @param bmeshes vector of meshes
       * @param cortex cortex as a [Ncortex x 3] matrix
       */
        TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &TM, const std::vector<Mesh<T>> &bmeshes,  const MatrixX3T_RM<T> &cortex,  const MatrixX3T_RM<T> &sdir);
 

        ///////////////////////////////////////////////////////////////
        //                         EFIELD
        ///////////////////////////////////////////////////////////////

        /**
         * @brief Calculates the electric field on cortex due to a coil outside the head in TMS_dir
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
         * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
         * @return \b Total electric field on the cortex points defined by ROI_start and ROI_n
         */
        VectorXT<T> Efield         (const Coil<T> &coil, const T minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
          /**
         * @overload
         * @details Uses list of indices ROI choice instead of contiguous block
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @param indlist List of cortex indices where the Electric field will be calculated
         * @return \b Total Electric field on the cortex points defined by indlist
         */
        VectorXT<T> Efield         (const Coil<T> &coil, const T minusdIPerdt, const Eigen::VectorXi &indlist);
        /**
         * @overload
         * @details Defaults to calculating the E-field on whole cortex
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @return \b Total electric field on the cortex
         */
        VectorXT<T> Efield         (const Coil<T> &coil, const T minusdIPerdt){return Efield         (coil, minusdIPerdt, 0, cortex_.rows());}
         /**
         * @details Defaults to calculating the E-field on whole cortex and using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @return \b Total electric field on the cortex
         */
        VectorXT<T> Efield         (const Coil<T> &coil){ return Efield         (coil, 1.0, 0, cortex_.rows()); }
        /**
         * @overload
         * @details Defaults to using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
         * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
         * @return \b Total electric field on the cortex points defined by ROI_start and ROI_n
         */
        VectorXT<T> Efield         (const Coil<T> &coil, const int32_t ROI_start, const int32_t ROI_n){ return Efield         (coil, 1.0, ROI_start, ROI_n); }
        /**
         * @overload 
         * @details Defaults to using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param indlist List of cortex indices where the Electric field will be calculated
         * @return \b Total electric field on the cortex points defined by indlist
         */
        VectorXT<T> Efield         (const Coil<T> &coil, const Eigen::VectorXi &indlist){ return Efield         (coil, 1.0, indlist); }
        ///////////////////////////////////////////////////////////////
        //                         EFIELD PRIMARY
        ///////////////////////////////////////////////////////////////

        /**
         * @brief Calculates the @b primary electric field on cortex due to a coil outside the head in TMS_dir \n 
         * For primary field calculation without providing Potential matrix see the standalone function @ref TMS_CPU_PrimaryField
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
         * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
         * @return \b Primary electric field on the cortex points defined by ROI_start and ROI_n
         */
        VectorXT<T> EfieldPrimary  (const Coil<T> &coil, const T minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        /**
         * @overload
         * @details Uses list of indices ROI choice instead of contiguous block
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @param indlist List of cortex indices where the Electric field will be calculated
         * @return \b Primary electric field on the cortex points defined by indlist
         */
        VectorXT<T> EfieldPrimary  (const Coil<T> &coil, const T minusdIPerdt, const Eigen::VectorXi &indlist);
         /**
         * @overload
         * @details Defaults to calculating the E-field on whole cortex
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @return \b Primary electric field on the cortex
         */
        VectorXT<T> EfieldPrimary  (const Coil<T> &coil, const T minusdIPerdt){return EfieldPrimary  (coil, minusdIPerdt, 0, cortex_.rows());}
        /**
         * @details Defaults to calculating the E-field on whole cortex and using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @return \b Primary electric field on the cortex
         */
        VectorXT<T> EfieldPrimary  (const Coil<T> &coil){ return EfieldPrimary  (coil, 1.0, 0, cortex_.rows()); }
        /**
         * @overload
         * @details Defaults to using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
         * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
         * @return \b Primary electric field on the cortex points defined by ROI_start and ROI_n
         */
        VectorXT<T> EfieldPrimary  (const Coil<T> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldPrimary  (coil, 1.0, ROI_start, ROI_n); }
        /**
         * @overload 
         * @details Defaults to using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param indlist List of cortex indices where the Electric field will be calculated
         * @return \b Primary electric field on the cortex points defined by indlist
         */
        VectorXT<T> EfieldPrimary  (const Coil<T> &coil, const Eigen::VectorXi &indlist){ return EfieldPrimary  (coil, 1.0, indlist); }
        
        
        ///////////////////////////////////////////////////////////////
        //                         EFIELD SECONDARY
        ///////////////////////////////////////////////////////////////

        /**
         * @brief Calculates the @b secondary electric field on cortex due to a coil outside the head in TMS_dir
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
         * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
         * @return \b Secondary electric field on the cortex points defined by ROI_start and ROI_n
         */
        VectorXT<T> EfieldSecondary(const Coil<T> &coil, const T minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        /**
         * @overload
         * @details Uses list of indices ROI choice instead of contiguous block
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @param indlist List of cortex indices where the Electric field will be calculated
         * @return \b Secondary electric field on the cortex points defined by indlist
         */
        VectorXT<T> EfieldSecondary(const Coil<T> &coil, const T minusdIPerdt, const Eigen::VectorXi &indlist);
         /**
         * @overload
         * @details Defaults to calculating the E-field on whole cortex
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param minusdIPerdt Negative rate of change of current run in the coil -dI/dt
         * @return \b Secondary electric electric field on the cortex
         */
        VectorXT<T> EfieldSecondary(const Coil<T> &coil, const T minusdIPerdt){return EfieldSecondary(coil, minusdIPerdt, 0, cortex_.rows());}
        /**
         * @details Defaults to calculating the E-field on whole cortex and using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @return \b Secondary electric field on the cortex
         */
        VectorXT<T> EfieldSecondary(const Coil<T> &coil){ return EfieldSecondary(coil, 1.0, 0, cortex_.rows()); }
        /**
         * @overload
         * @details Defaults to using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param ROI_start  Starting index of Region of Interest (ROI) on the cortex
         * @param ROI_n Size of ROI, i.e., E-field calculated in ROI_n points after index ROI_start
         * @return \b Secondary electric field on the cortex points defined by ROI_start and ROI_n
         */
        VectorXT<T> EfieldSecondary(const Coil<T> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldSecondary(coil, 1.0, ROI_start, ROI_n); }
        /**
         * @overload 
         * @details Defaults to using dI/dt = 1
         * @param coil Coil object, supports all coiltypes defined in Coil.hpp
         * @param indlist List of cortex indices where the Electric field will be calculated
         * @return \b Secondary electric field on the cortex points defined by indlist
         */
        VectorXT<T> EfieldSecondary(const Coil<T> &coil, const Eigen::VectorXi &indlist){ return EfieldSecondary(coil, 1.0, indlist); }



    private:
        const MatrixX3T_RM<T> &cortex_;
        const MatrixX3T_RM<T> &sdir_;
        const BetaDipoles<T> bd_;
        const T2 *p_wPhi_;  
        const T2 *p_TM_;     
        VectorXT<T> Etms_;
        const bool UseTM_; // This is set in constructor to know which method of calculation to use. 
        const int32_t rows_;
        const int32_t cols_;

        // Separate function is used to reduce repetition when using multiple different constructor interfaces
        void Setup();
        void CheckBlockROI(const int32_t &ROI_start, const int32_t &ROI_n);
};

//user defined template argument deduction
// No idea why this is needed, but doesnt compile without it..
//template <typename T, typename T2> TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const std::vector<Mesh<T>> &bmeshes,  const Mesh<T> &cortex, const MatrixX3T_RM<T> &sdir) -> TMS_dir<T,T2>;
//template <typename T, typename T2> TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const std::vector<Mesh<T>> &bmeshes,  const MatrixX3T_RM<T> &cortex, const MatrixX3T_RM<T> &sdir)-> TMS_dir<T,T2>;

//template <typename T, typename T2> TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &TM, const std::vector<Mesh<T>> &bmeshes,  const Mesh<T> &cortex, const MatrixX3T_RM<T> &sdir)-> TMS_dir<T,T2>;
//template <typename T, typename T2> TMS_dir(const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &Phi, const Eigen::Matrix<T2, Eigen::Dynamic, Eigen::Dynamic> &TM, const std::vector<Mesh<T>> &bmeshes,  const MatrixX3T_RM<T> &cortex, const MatrixX3T_RM<T> &sdir)-> TMS_dir<T,T2>;