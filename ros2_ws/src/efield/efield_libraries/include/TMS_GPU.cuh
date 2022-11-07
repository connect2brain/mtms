#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include "Mesh.hpp"

#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <linalg>

//provides template class used with doubles
//#include "TMS_CPU.hpp"


/* 
Use by including the file TMS and specifying compile time macro "-DUSE_CUDA"

For now GPU is only used with floats (template specialization <float,float> which defaults to the CPU implementation when doubles are used)
*/

template <typename T1, typename T2>
class TMS_GPU {

};

template <>
class TMS_GPU<float,float> {

    public:
        /*
        Constructor for the TMS class
        IN:
         - Phi: Potential matrix                                     [sum(bmeshes.Nop()) x cortex.Nop()]
         - bmeshes: head geometry in std::vector of mesh classes     [N]
         - cortex: Mesh or matrix containing ROI for field calculation         1 x [cortex.Nop()]                         
        */
        TMS_GPU(const Matrix<float> &Phi, const std::vector<Mesh<float>> &bmeshes,  const Mesh<float> &cortex);

        TMS_GPU(const Matrix<float> &Phi, const std::vector<Mesh<float>> &bmeshes,  const Matrix<float, RowMajor> &cortex);

        TMS_GPU(const Matrix<float> &Phi, const Matrix<float> &TM, const std::vector<Mesh<float>> &bmeshes,  const Mesh<float> &cortex);

        TMS_GPU(const Matrix<float> &Phi, const Matrix<float> &TM, const std::vector<Mesh<float>> &bmeshes,  const Matrix<float, RowMajor> &cortex);
        
        /*
        Efield calculation on ROI specified by parameters ROI_start and  ROI_n
        */
        Matrix<float, RowMajor> Efield         (const Coil<float> &coil, const float  minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        Matrix<float, RowMajor> EfieldPrimary  (const Coil<float> &coil, const float  minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        Matrix<float, RowMajor> EfieldSecondary(const Coil<float> &coil, const float  minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);


        /*
        Efield calculation on ROI specified by an indexlist
        */
        Matrix<float, RowMajor> Efield         (const Coil<float> &coil, const float  minusdIPerdt, const ColVector<int32_t> &indlist);
        Matrix<float, RowMajor> EfieldPrimary  (const Coil<float> &coil, const float  minusdIPerdt, const ColVector<int32_t> &indlist);
        Matrix<float, RowMajor> EfieldSecondary(const Coil<float> &coil, const float  minusdIPerdt, const ColVector<int32_t> &indlist);

        /*
        Efield default overload on the whole cortex
        */
        Matrix<float, RowMajor> Efield         (const Coil<float> &coil, const float  minusdIPerdt){return Efield         (coil, minusdIPerdt, 0, cortex_.Rows());}
        Matrix<float, RowMajor> EfieldPrimary  (const Coil<float> &coil, const float  minusdIPerdt){return EfieldPrimary  (coil, minusdIPerdt, 0, cortex_.Rows());}
        Matrix<float, RowMajor> EfieldSecondary(const Coil<float> &coil, const float  minusdIPerdt){return EfieldSecondary(coil, minusdIPerdt, 0, cortex_.Rows());}


        // default overloads for minusIperdt
        Matrix<float, RowMajor> Efield         (const Coil<float> &coil){ return Efield         (coil, 1.0, 0, cortex_.Rows()); }
        Matrix<float, RowMajor> EfieldPrimary  (const Coil<float> &coil){ return EfieldPrimary  (coil, 1.0, 0, cortex_.Rows()); }
        Matrix<float, RowMajor> EfieldSecondary(const Coil<float> &coil){ return EfieldSecondary(coil, 1.0, 0, cortex_.Rows()); }
        Matrix<float, RowMajor> Efield         (const Coil<float> &coil, const int32_t ROI_start, const int32_t ROI_n){ return Efield         (coil, 1.0, ROI_start, ROI_n); }
        Matrix<float, RowMajor> EfieldPrimary  (const Coil<float> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldPrimary  (coil, 1.0, ROI_start, ROI_n); }
        Matrix<float, RowMajor> EfieldSecondary(const Coil<float> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldSecondary(coil, 1.0, ROI_start, ROI_n); }

        // default overloads for minusIperdt
        Matrix<float, RowMajor> Efield         (const Coil<float> &coil, const ColVector<int32_t> &indlist){ return Efield         (coil, 1.0, indlist); }
        Matrix<float, RowMajor> EfieldPrimary  (const Coil<float> &coil, const ColVector<int32_t> &indlist){ return EfieldPrimary  (coil, 1.0, indlist); }
        Matrix<float, RowMajor> EfieldSecondary(const Coil<float> &coil, const ColVector<int32_t> &indlist){ return EfieldSecondary(coil, 1.0, indlist); }

        /*
        Returns the number of points on the cortex
        
        Helps with choosing ROI when using the json constructor
        */
        int32_t CortexSize(){
            return cortex_.Rows();
        }

        /*
        Returns the coordinates of the scalp point with index "ind"

        Can be used to define coil locations 
        */
        RowVector<float> ScalpPoint(int32_t ind){
            if(ind >= scalp_.Rows()){
                std::cout<<"TRYING TO ACCESS SCALP POINT OUT OF RANGE (index " <<  ind <<")\n Possible indices are 0,...,"<< scalp_.Rows() - 1 <<std::endl;
                return RowVector<float>(3);
            }else{
               return scalp_.GetRow(ind);
            }
        }

        //destructor
        ~TMS_GPU();

    private:
        // Does all required memory allocations needed in constructor. 
        // Separate function is used to reduce repetition when using multiple different constructor interfaces
        void Setup();

        //Max dimensions of Phi matrix / cortex and meshes
        const int32_t rows_;  // number of meshpoints
        const int32_t cols_;  // number of cortex points * 3
        int32_t GPU_nop_; // number of columns computed on GPU
        //GPU pointers
        float* d_bdmom_;
        float* d_bdpos_;
        float* d_Phi_;
        float* d_cortex_;
        float* d_Etms_;
        float* d_BQ_;
        //CPU arrays
        const Matrix<float, RowMajor> &scalp_;
        const Matrix<float, RowMajor> &cortex_;
        const BetaDipoles<float> bd_;
        const float * p_wPhi_;    
        const float * p_TM_;  

        Matrix<float, RowMajor> Etms_;
        float *BQ_pinned;

        const bool UseTM_; // This is set in constructor to know which method of calculation to use. 
        // CUBLAS functions status
        cublasStatus_t stat_; 
        cublasHandle_t handle_; 
        cudaStream_t stream1, stream2;

};
