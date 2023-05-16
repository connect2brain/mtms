#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <map>

#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include "Mesh.hpp"
#include "JsonIO.hpp"

#include <cublas_v2.h>
#include <cuda_runtime.h>
#include "eigen_defines.hpp"

/** 
*  @brief Class for calculating the Efield in %TMS using GPU/CPU \n 
*  For more detailed member function documentation see TMS (identical interfaces)
*  @see TMS
*  @note Only supports floats (at least for now)
*  @note Calculations will be divided between CPU and GPU if there is not enough GPU memory
*  @note WARNING: TMS and TMS_GPU classes do not have copy or move constructors as they are not meant to be given as argument to functions or copied between scopes
*/
class TMS_GPU {
    public:
    
        TMS_GPU(const Eigen::MatrixXf &Phi, const std::vector<Mesh<float>> &bmeshes,  const Mesh<float> &cortex);
        TMS_GPU(const Eigen::MatrixXf &Phi, const std::vector<Mesh<float>> &bmeshes,  const MatrixX3f_RM &cortex);
        TMS_GPU(const Eigen::MatrixXf &Phi, const Eigen::MatrixXf &TM, const std::vector<Mesh<float>> &bmeshes,  const Mesh<float> &cortex);
        TMS_GPU(const Eigen::MatrixXf &Phi, const Eigen::MatrixXf &TM, const std::vector<Mesh<float>> &bmeshes,  const MatrixX3f_RM &cortex);
        

        MatrixX3f_RM Efield         (const Coil<float> &coil, const float  minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        MatrixX3f_RM Efield         (const Coil<float> &coil, const float  minusdIPerdt, const Eigen::VectorXi &indlist);
        MatrixX3f_RM Efield         (const Coil<float> &coil, const float  minusdIPerdt){return Efield         (coil, minusdIPerdt, 0, cortex_.rows());}
        MatrixX3f_RM Efield         (const Coil<float> &coil){ return Efield         (coil, 1.0, 0, cortex_.rows()); }
        MatrixX3f_RM Efield         (const Coil<float> &coil, const int32_t ROI_start, const int32_t ROI_n){ return Efield         (coil, 1.0, ROI_start, ROI_n); }
        MatrixX3f_RM Efield         (const Coil<float> &coil, const Eigen::VectorXi &indlist){ return Efield         (coil, 1.0, indlist); }

        MatrixX3f_RM EfieldPrimary  (const Coil<float> &coil, const float  minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        MatrixX3f_RM EfieldPrimary  (const Coil<float> &coil, const float  minusdIPerdt, const Eigen::VectorXi &indlist);
        MatrixX3f_RM EfieldPrimary  (const Coil<float> &coil, const float  minusdIPerdt){return EfieldPrimary  (coil, minusdIPerdt, 0, cortex_.rows());}
        MatrixX3f_RM EfieldPrimary  (const Coil<float> &coil){ return EfieldPrimary  (coil, 1.0, 0, cortex_.rows()); }
        MatrixX3f_RM EfieldPrimary  (const Coil<float> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldPrimary  (coil, 1.0, ROI_start, ROI_n); }
        MatrixX3f_RM EfieldPrimary  (const Coil<float> &coil, const Eigen::VectorXi &indlist){ return EfieldPrimary  (coil, 1.0, indlist); }

        MatrixX3f_RM EfieldSecondary(const Coil<float> &coil, const float  minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        MatrixX3f_RM EfieldSecondary(const Coil<float> &coil, const float  minusdIPerdt, const Eigen::VectorXi &indlist);
        MatrixX3f_RM EfieldSecondary(const Coil<float> &coil, const float  minusdIPerdt){return EfieldSecondary(coil, minusdIPerdt, 0, cortex_.rows());}
        MatrixX3f_RM EfieldSecondary(const Coil<float> &coil){ return EfieldSecondary(coil, 1.0, 0, cortex_.rows()); }
        MatrixX3f_RM EfieldSecondary(const Coil<float> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldSecondary(coil, 1.0, ROI_start, ROI_n); }
        MatrixX3f_RM EfieldSecondary(const Coil<float> &coil, const Eigen::VectorXi &indlist){ return EfieldSecondary(coil, 1.0, indlist); }

        void print(const Coil<float> &coil, const float  minusdIPerdt){
            std::cout << "HELLO \n";
            std::cout<< "coil type:"<<coil.CoilType()<<std::endl;
            std::cout<< "minusIPerdt"<< minusdIPerdt<<std::endl;
            std::cout << "Cortex"<< cols_<<std::endl;
            std::cout << "Mesh"<< rows_<<std::endl;
            std::cout << "GPU_Phi_cols "<< GPU_Phi_cols_<<std::endl;
        }
        //destructor
        ~TMS_GPU(){std::cout << "GOOD BYE\n";};

    private:
        // Does all required memory allocations needed in constructor. 
        // Separate function is used to reduce repetition when using multiple different constructor interfaces
        void Setup();

        //Max dimensions of Phi matrix / cortex and meshes
        const int32_t rows_;  // number of meshpoints
        const int32_t cols_;  // number of cortex points * 3
        int32_t GPU_Phi_cols_; // number of columns of Phi on GPU

        //GPU pointers
        float* d_bdmom_;
        float* d_bdpos_;
        float* d_Phi_;
        float* d_cortex_;
        float* d_Etms_;
        float* d_BQ_;

        //CPU arrays
        const MatrixX3f_RM &cortex_;
        const BetaDipoles<float> bd_;
        const float * p_wPhi_;    

        MatrixX3f_RM Etms_;
        float *BQ_pinned_;

        // Following only initialized and used in case T matrix is given in constructor
        const bool UseTM_; // This is set in constructor to know which method of calculation to use. 
        int32_t GPU_TM_cols_ = -1; // Number of columns of TM on GPU
        float *TBvec_pinned_;   // CPU cuda host malloc
        float *d_TBvec_;        // GPU
        const float * p_TM_;    // CPU, not allocated by this class
        float* d_TM_;           // GPU

        // CUBLAS functions status
        cublasHandle_t handle_; 
        cudaStream_t stream1, stream2;

        float const1 = 1.0;

        void CheckIndexList(const Eigen::VectorXi &indlist);
        void CheckBlockROI(const int32_t &ROI_start, const int32_t &ROI_n);
        //float OptimizePartition();

        // helper to reduce repetition
        void GPU_multiply_TBetaQ(const bool moveResultToCPU);
};