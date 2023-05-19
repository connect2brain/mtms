//The functions in this file are partially based on hbftms_cpp library example codes written by M. Stenroos
//This library is not free to use without written permission from the owner Matti Stenroos
//#define USE_CUDA
#include "tms_cpp.hpp"
#ifdef USE_CUDA
#include "TMS_GPU.hpp"
#endif
#include "TMS_CPU.hpp"
#include "tms_json.h"
//#include "efield_estimation.h"
#include <iostream>

using namespace Eigen;
using namespace std;
std::vector<double> efield_vector;
Timer t;
Mesh<float> *cortex;
Coil<float> coilmodel;
std::vector< Mesh<float>>meshes;
MatrixXf Phi;
MatrixX3T_RM<float> spos;
Timer timer;

#ifdef USE_CUDA
TMS_GPU<float,float> *TMS_obj;
#else
TMS <float,float>*TMS_obj;
#endif

//This function Initializes the Efield service on my application, here is modified for this test
void init_efield(std::string cortexfile, std::vector<std::string> meshfile, std::vector<float> ci, std::vector<float> co, bool &success)
{
    for (auto element: meshfile){
        meshes.push_back(element);
    }
    cortex =new Mesh<float>(cortexfile);
    spos= cortex->Points();

#ifdef USE_CUDA
    timer.Start();
    vector< MatrixXf > D = BEMOperatorsPhi_LC_GPU(meshes);
    MatrixXf TM = TM_Phi_LC_GPU(D, ci, co);
    Phi = LFM_Phi_LC_GPU(meshes, TM, cortex->Points());
    cout << "Time for the full phi calculations and TMS setup:  " << timer.GetElapsed() << std::endl;
#else
    vector< MatrixXf > D = BEMOperatorsPhi_LC(meshes);
    MatrixXf TM = TM_Phi_LC(D, ci, co);
    Phi = LFM_Phi_LC(meshes, TM, cortex->Points());
    cout<<"Phi dimensions: "<< Phi.rows() << " " <<Phi.cols()<<std::endl;
#endif
    WeightedPhi(meshes, Phi, ci, co);

#ifdef USE_CUDA
    TMS_obj= new TMS_GPU<float,float>(Phi, meshes, spos);
#else
    TMS_obj=new TMS<float,float>(Phi, meshes, spos);
#endif
    success = true;
    std::cout<<"Done init! "<<std::endl;
}

void set_coil(std::string coilfile, bool &success)
{
    std::cout<<"coilfile changed: "<<coilfile<<std::endl;
    coilmodel = Coil<float>(coilfile);
    success = true;
}

void E_norm(MatrixX3f_RM &Etms, std::vector<double> &efield_vector)
{
    for (int i = 0; i < Etms.rows(); i++)
    {
        double acc=0.0;
        for (int j = 0; j < Etms.cols(); j++)
        {
            acc += Etms(i,j)*Etms(i,j);
        }
        efield_vector.push_back(std::sqrt(acc));
    }
}


void efield_estimation(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<double> &efield_vector)
{
    //** Convert vectors to Eigen matrices
    Matrix<float, 3, 3, RowMajor> T_rot;
    T_rot = Map<Matrix<float, 3, 3, RowMajor>>(rot_matrix.data(), 3,3);
    Matrix<float, 3, 1> cp;
    cp = Map<Matrix<float, 3, 1>>(position.data(),3,1);
    cout<<"T_rot: "<<T_rot<< endl;
    cout<<"cp: "<<cp<<endl;


    // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.
    float minusdIPerdt = -6600.0*10000.0;

    MatrixX3f_RM Etms;
    t.Start();
    // Apply the rotation T_rot and translation cp to the coil§
    coilmodel.Transform(T_rot,cp);
    cout<< "Transform done"<<endl;
    cout<<"coil type:"<<coilmodel.CoilType()<<endl;
    //** Calculate E-field
    //*** HERE is where it seems to fail
    //TMS_obj->print(coilmodel, minusdIPerdt);
    Etms = TMS_obj->Efield(coilmodel, minusdIPerdt);
    cout<< "Efield done"<<endl;
    //cout<<Etms<<endl;
    t.Elapsed();
    E_norm(Etms, efield_vector);

}
int main(void)
{
    return 0;
}
