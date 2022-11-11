//
// Created by Kalle Jyrkinen on 16.12.2021.
//

#ifndef TMS_CPP_D_LC_HPP
#define TMS_CPP_D_LC_HPP

#include <tmsutil>

// Build a single linear collocation double-layer matrix, for 2 different meshes.
// Used as a subroutine of D_LC for building the off-diagonal entries of the full D matrix.
Matrix<float> D_LC_DiffMesh(const Mesh<float> &field,  // field mesh
                            const Mesh<float> &source      // source mesh
);

// Build a single linear collocation double-layer matrix, for a single mesh.
// Used as a subroutine of D_LC for building the diagonal entries of the full D matrix.
Matrix<float> D_LC_SameMesh(const Mesh<float> &field  // field mesh
);

#endif //TMS_CPP_D_LC_HPP
