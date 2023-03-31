//The functions in this file are partially based on hbftms_cpp library example codes written by M. Stenroos
//This library is not free to use without written permission from the owner Matti Stenroos

#define DATAROOT "/app/ros2_ws/src/targeting/efield/efield_libraries/data/"
#define FLAG_MAGSTIM70 1 // set to 1 to use 42-dipole Magstim 70 model

#include <phi_lc>
#include <tms>

#include "efield_estimation.h"
#define USE_CUDA

std::string meshroot = std::string(DATAROOT) + "headmodels/invesalius/";
std::string meshfile = meshroot + "example-scalp.bin";
std::string cortexfile = meshroot + "example-cortex.bin";
std::string coilfile = std::string(DATAROOT) + "coilmodels/magstim70/magstim70_42.bin";

std::vector<double> efield_vector;
// Initialize first the mesh, then build the meshes vector:
Mesh<float> m(meshfile);
std::vector< Mesh<float> > meshes{m};

Mesh<float> cortex(cortexfile);
Matrix<float, RowMajor> spos(cortex.Points());

Timer t;

// set BEM model conductivities --- in single-shell model, the value of 'ci' doesn't matter
std::vector<float> ci = {0.3300};
std::vector<float> co = {0};

std::vector< Matrix<float> > D = BEMOperatorsPhi_LC(meshes);
Matrix<float> TM = TM_Phi_LC(D, ci, co);
Matrix<float> Phi = LFM_Phi_LC(meshes, TM, spos);

Coil<float> coilmodel(coilfile);
//TMS<float,float> TMS_obj(Phi, meshes, spos);
TMS_GPU<float,float> TMS_obj(Phi, meshes, spos);

//function to input data from efield_service
void init_efield()
{
    WeightedPhi(meshes, Phi, ci, co);
    //TMS<float,float> TMS_obj(Phi, meshes, spos);
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
    // set example coil location to vertex 599 (in 0-basis) of the scalp mesh
    int32_t pind = 1;
    //RowVector<float> cp = m.Points().GetRow(pind);
    RowVector<float> cp(3,position);
    std::cout << "Elements: " <<std::endl;
    cortex.Elements().ShowBlock(0, 0, 5, 3);
    std::cout << "Points:" <<std::endl;
    cortex.Points().ShowBlock(0, 0, 5, 3);
    // This is also possible
    // RowVector<float> cpold = TMS_obj.ScalpPoint(pind);

    // Define rotation matrix for the coil (Applied from right  x' = x*T_rot)
    Matrix<float, RowMajor> T_rot(3,3,{rot_matrix});

    // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.
    float minusdIPerdt = -6600.0*10000.0;

    Matrix<float, RowMajor> Etms;
    t.Start();
    // Apply the rotation T_rot and translation cp to the coil§
    coilmodel.Transform(T_rot,cp);

    // Calculate E-field
    Etms = TMS_obj.Efield(coilmodel, minusdIPerdt);
    t.Elapsed();


    E_norm(Etms, efield_vector);

}

int main(void)
{
    return 0;
}
