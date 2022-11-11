//
// Created by Kalle Jyrkinen on 13.12.2021.
//

#ifndef TMS_CPP_LFM_PHI_LC_HPP
#define TMS_CPP_LFM_PHI_LC_HPP

#include <tmsutil>

// Build the electric lead field matrix. If 'sdir' is omitted, LFM will be computed using xyz unit dipole triplets
Matrix<float> LFM_Phi_LC(const std::vector<Mesh<float> > &meshes, // BEM geometry
                         const Matrix<float> &Tphi, // BEM transfer matrix
                         const Matrix<float, RowMajor> &spos, // source positions, [M x 3]
                         const Matrix<float, RowMajor> &sdir, // source orientations, [M x 3]
                         bool averef = true // use average reference?
);

Matrix<float> LFM_Phi_LC(const std::vector<Mesh<float> > &meshes, // BEM geometry
                         const Matrix<float> &Tphi, // BEM transfer matrix
                         const Matrix<float, RowMajor> &spos, // source positions, [M x 3]
                         bool averef = true // use average reference? 1 for yes, 2 for no
);


#endif //TMS_CPP_LFM_PHI_LC_HPP
