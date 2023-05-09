//The functions in this file are partially based on hbftms_cpp library example codes written by M. Stenroos
//This library is not free to use without written permission from the owner Matti Stenroos


#include <phi_lc>
#include <tms>
//#include "efield_estimation.h"
#include <iostream>
#include "EfieldInterface.h"

std::vector<double> efield_vector;
Timer t;
Mesh<float> *cortex;
Coil<float> coilmodel;
#ifdef USE_CUDA
TMS_GPU<float,float> *TMS_obj;
#else
TMS<float,float> *TMS_obj;
#endif
Mesh<float> m;
std::vector< Mesh<float>>meshes;
Matrix<float>Phi;
Matrix<float, RowMajor> spos;

// Function to input data from efield_service
// ci conductivities inside, co conductivities outside
void init_efield(std::string cortexfile, std::vector<std::string> meshfile, std::vector<float> ci, std::vector<float> co, bool &success)
{
    std::cout<<"cortexfile: "<<cortexfile<<std::endl;
    std::cout<<"meshfile: "<<meshfile.size()<<std::endl;
    std::cout <<"conductivities outside: "<< co.at(0)<<std::endl;
    std::cout <<"conductivities inside: "<< ci.at(0)<<std::endl;

    for (auto element: meshfile){
        meshes.push_back(element);
    }
    cortex =new Mesh<float>(cortexfile);
    spos= Matrix<float, RowMajor>(cortex->Points());
    std::vector< Matrix<float> > D = BEMOperatorsPhi_LC(meshes);
    Matrix<float> TM = TM_Phi_LC(D, ci, co);
    Phi=LFM_Phi_LC(meshes, TM, spos);
    #ifdef USE_CUDA
    TMS_obj=new TMS_GPU<float,float>(Phi, meshes, spos);
    #else
    TMS_obj=new TMS<float,float>(Phi, meshes, spos);
    #endif
    WeightedPhi(meshes, Phi, ci, co);

    co.clear();
    ci.clear();
    D.clear();
    meshes.clear();
    success = true;
    std::cout<<"Done init! "<<std::endl;
}

void set_coil(std::string coilfile, bool &success)
{
    std::cout<<"coilfile set: "<<coilfile<<std::endl;
    coilmodel = Coil<float>(coilfile);
    success = true;
}

void E_norm(Matrix<float, RowMajor> &Etms, std::vector<double> &efield_vector)
{
    for (int i = 0; i < Etms.Rows(); i++)
    {
        double acc=0.0;
        for (int j = 0; j < Etms.Cols(); j++)
        {
            acc += Etms(i,j)*Etms(i,j);
        }
        efield_vector.push_back(std::sqrt(acc));
    }
}


void efield_estimation(std::vector<float>& position, std::vector<double>& orientation, std::vector<float>& rot_matrix, std::vector<double> &efield_vector)
{

    RowVector<float> cp(3,position);

    // Define rotation matrix for the coil (Applied from right  x' = x*T_rot)
    Matrix<float, RowMajor> T_rot(3,3,{rot_matrix});

    // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.
    float minusdIPerdt = -6600.0*10000.0;

    Matrix<float, RowMajor> Etms;
    t.Start();
    // Apply the rotation T_rot and translation cp to the coil§
    coilmodel.Transform(T_rot,cp);

    // Calculate E-field
    Etms = TMS_obj->Efield(coilmodel, minusdIPerdt);
    t.Elapsed();
    E_norm(Etms, efield_vector);

}

int main(void)
{
    return 0;
}
