//
// Created by mtms on 18.4.2023.
//

#ifndef TMS_CPP_EFIELDINTERFACE_H
#define TMS_CPP_EFIELDINTERFACE_H
#include <phi_lc>
#include <tms>

class EfieldInterface{
public:
    EfieldInterface(std::string cortexfile,std::string meshfile,bool &success);
    void E_norm(Matrix<float, RowMajor> &Etms, std::vector<double> &efield_vector);
    void efield_estimation(RowVector<float>& position, Matrix<float, RowMajor>& rot_matrix, std::vector<double> &efield_vector);

private:
    Mesh<float> cortex;
    std::vector<float> ci = {0.3300};
    std::vector<float> co = {0};
    float minusdIPerdt = -6600.0F*10000.0F;

    Matrix<float, RowMajor> spos;
    Coil<float> coilmodel;
    Matrix<float>Phi;

public:
    Mesh<float> m;
    std::vector< Mesh<float>>meshes;
    TMS<float,float> *TMS_obj;
    Matrix<float, RowMajor> Etms;

};


#endif //TMS_CPP_EFIELDINTERFACE_H
