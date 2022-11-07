#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

#include "Coil.hpp"
#include "Mesh.hpp"
#include <linalg>
#include "BetaDipoles.hpp"

/* 
Class for calculating the Efield in TMS 
T2 : Type of Potential matrix
T  : Type of all other inputs

Result will be of type T. If Potential matrix is given as floats it will be cast to doubles in the multiplication
*/
template <typename T, typename T2>
class TMS {
    public:
        /*
        Constructor for the TMS class
        IN:
         - Phi: Potential matrix                                     [sum(bmeshes.Nop()) x cortex.Nop()]
         - bmeshes: head geometry in std::vector of mesh classes     [N]
         - cortex: Mesh or Matrix containing ROI for field calculation         1 x [cortex.Nop()]                          
        */

        TMS(const Matrix<T2> &Phi, const std::vector<Mesh<T>> &bmeshes,  const Mesh<T> &cortex);
        TMS(const Matrix<T2> &Phi, const std::vector<Mesh<T>> &bmeshes,  const Matrix<T, RowMajor> &cortex);

        TMS(const Matrix<T2> &Phi, const Matrix<T2> &TM, const std::vector<Mesh<T>> &bmeshes,  const Mesh<T> &cortex);
        TMS(const Matrix<T2> &Phi, const Matrix<T2> &TM, const std::vector<Mesh<T>> &bmeshes,  const Matrix<T, RowMajor> &cortex);
 
        /*
        Efield calculation on ROI specified by parameters ROI_start and  ROI_n
        */
        Matrix<T, RowMajor> Efield         (const Coil<T> &coil, const T minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        Matrix<T, RowMajor> EfieldPrimary  (const Coil<T> &coil, const T minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);
        Matrix<T, RowMajor> EfieldSecondary(const Coil<T> &coil, const T minusdIPerdt, const int32_t ROI_start, const int32_t ROI_n);


        /*
        Efield calculation on ROI specified by an indexlist
        */
        Matrix<T, RowMajor> Efield         (const Coil<T> &coil, const T minusdIPerdt, const ColVector<int32_t> &indlist);
        Matrix<T, RowMajor> EfieldPrimary  (const Coil<T> &coil, const T minusdIPerdt, const ColVector<int32_t> &indlist);
        Matrix<T, RowMajor> EfieldSecondary(const Coil<T> &coil, const T minusdIPerdt, const ColVector<int32_t> &indlist);

        /*
        Efield default overload on the whole cortex
        */
        Matrix<T, RowMajor> Efield         (const Coil<T> &coil, const T minusdIPerdt){return Efield         (coil, minusdIPerdt, 0, cortex_.Rows());}
        Matrix<T, RowMajor> EfieldPrimary  (const Coil<T> &coil, const T minusdIPerdt){return EfieldPrimary  (coil, minusdIPerdt, 0, cortex_.Rows());}
        Matrix<T, RowMajor> EfieldSecondary(const Coil<T> &coil, const T minusdIPerdt){return EfieldSecondary(coil, minusdIPerdt, 0, cortex_.Rows());}


        // default overloads for minusIperdt
        Matrix<T, RowMajor> Efield         (const Coil<T> &coil){ return Efield         (coil, 1.0, 0, cortex_.Rows()); }
        Matrix<T, RowMajor> EfieldPrimary  (const Coil<T> &coil){ return EfieldPrimary  (coil, 1.0, 0, cortex_.Rows()); }
        Matrix<T, RowMajor> EfieldSecondary(const Coil<T> &coil){ return EfieldSecondary(coil, 1.0, 0, cortex_.Rows()); }
        Matrix<T, RowMajor> Efield         (const Coil<T> &coil, const int32_t ROI_start, const int32_t ROI_n){ return Efield         (coil, 1.0, ROI_start, ROI_n); }
        Matrix<T, RowMajor> EfieldPrimary  (const Coil<T> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldPrimary  (coil, 1.0, ROI_start, ROI_n); }
        Matrix<T, RowMajor> EfieldSecondary(const Coil<T> &coil, const int32_t ROI_start, const int32_t ROI_n){ return EfieldSecondary(coil, 1.0, ROI_start, ROI_n); }

        // default overloads for minusIperdt
        Matrix<T, RowMajor> Efield         (const Coil<T> &coil, const ColVector<int32_t> &indlist){ return Efield         (coil, 1.0, indlist); }
        Matrix<T, RowMajor> EfieldPrimary  (const Coil<T> &coil, const ColVector<int32_t> &indlist){ return EfieldPrimary  (coil, 1.0, indlist); }
        Matrix<T, RowMajor> EfieldSecondary(const Coil<T> &coil, const ColVector<int32_t> &indlist){ return EfieldSecondary(coil, 1.0, indlist); }

        /*
        Returns the number of points on the cortex
        
        Helps with choosing ROI when using the json constructor
        */
        int32_t CortexSize() {return cortex_.Rows();}

        /*
        Returns the coordinates of the scalp point with index "ind"

        Can be used to define coil locations 
        */
        RowVector<T> ScalpPoint(const int32_t ind);

    private:
        const Matrix<T, RowMajor> &scalp_;
        const Matrix<T, RowMajor> &cortex_;
        const BetaDipoles<T> bd_;
        const T2 *p_wPhi_;  
        const T2 *p_TM_;     
        Matrix<T, RowMajor> Etms_;
        const bool UseTM_; // This is set in constructor to know which method of calculation to use. 
        const int32_t rows_;
        const int32_t cols_;

        // Separate function is used to reduce repetition when using multiple different constructor interfaces
        void Setup();
};
