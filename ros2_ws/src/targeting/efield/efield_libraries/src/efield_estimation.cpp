//The functions in this file are partially based on hbftms_cpp library example codes written by M. Stenroos
//This library is not free to use without written permission from the owner Matti Stenroos

#define DATAROOT "/app/ros2_ws/src/targeting/efield/efield_libraries/data/"
#define DATAROOT1 "/app/data/neuronavigation/efield/"
#define FLAG_MAGSTIM70 1 // set to 1 to use 42-dipole Magstim 70 model

#include <phi_lc>
#include <tms>
//#include "efield_estimation.h"
#include <iostream>
#include "EfieldInterface.h"


std::string meshroot = std::string(DATAROOT1) ;//+ "headmodels/invesalius/";
std::string coilfile = std::string(DATAROOT) + "coilmodels/magstim70/magstim70_42.bin";
//std::string meshfile = meshroot + "is_scaled_up_inv_exp.bin";
std::vector<double> efield_vector;
// Initialize first the mesh, then build the meshes vector:


//Mesh<float> cortex();
//Matrix<float, RowMajor> spos(cortex.Points());

Timer t;

// set BEM model conductivities --- in single-shell model, the value of 'ci' doesn't matter
//std::vector<float> ci = {0.3300};
//std::vector<float> co = {0};

//std::vector< Matrix<float> > D = BEMOperatorsPhi_LC(meshes);
//Matrix<float> TM = TM_Phi_LC(D, ci, co);
//Matrix<float> Phi;
Mesh<float> *cortex;
//Coil<float> coilmodel(coilfile);
Coil<float> coilmodel;
TMS<float,float> *TMS_obj;
std::vector<float> ci= {0.3300};
std::vector<float> co= {0};
Mesh<float> m;
std::vector< Mesh<float>>meshes;
Matrix<float>Phi;
Matrix<float, RowMajor> spos;

//function to input data from efield_service
/*void init_efield(std::string cortexfile, std::string meshfile, bool &success)
{
    std::cout<<"cortexfile: "<<cortexfile<<std::endl;
    std::cout<<"meshfile: "<<meshfile<<std::endl;

    m =Mesh<float>(meshfile);
    meshes = std::vector<Mesh<float>>({m});
    cortex =new Mesh<float>(cortexfile);
    spos= Matrix<float, RowMajor>(cortex->Points());
    std::vector< Matrix<float> > D = BEMOperatorsPhi_LC(meshes);
    Matrix<float> TM = TM_Phi_LC(D, ci, co);
    Phi=LFM_Phi_LC(meshes, TM, spos);
    TMS_obj=new TMS<float,float>(Phi, meshes, spos);
    WeightedPhi(meshes, Phi, ci, co);
    success = true;
    std::cout<<"Done init! "<<std::endl;
}*/
void init_efield(std::string cortexfile, std::vector<std::string> meshfile, bool &success)
{
    std::cout<<"cortexfile: "<<cortexfile<<std::endl;
    std::cout<<"meshfile: "<<meshfile.at(0)<<std::endl;

    m =Mesh<float>(meshfile.at(0));
    meshes = std::vector<Mesh<float>>({m});
    cortex =new Mesh<float>(cortexfile);
    spos= Matrix<float, RowMajor>(cortex->Points());
    std::vector< Matrix<float> > D = BEMOperatorsPhi_LC(meshes);
    Matrix<float> TM = TM_Phi_LC(D, ci, co);
    Phi=LFM_Phi_LC(meshes, TM, spos);
    TMS_obj=new TMS<float,float>(Phi, meshes, spos);
    WeightedPhi(meshes, Phi, ci, co);
    success = true;
    std::cout<<"Done init! "<<std::endl;
}

void add_coil(std::string coilfile, bool &success)
{   std::cout<<"coilfile: "<<coilfile<<std::endl;
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
    std::string meshroot = std::string(DATAROOT) + "/data/headmodels/invesalius1/";
    std::string coilroot = std::string(DATAROOT) + "/data/coilmodels/";
    std::vector<double> efield_vector;

    std::string coilfile;
    if (FLAG_MAGSTIM70)  // Magstim 70, 42 magnetic dipoles
        coilfile = coilroot + "magstim70/magstim70_42.bin";
    else // "Navigation coil" of the 5-coil mTMS transducer
        coilfile = coilroot + "coilset-5/coilset-5-4.bin";
    std::cout<<"coilfile"<<coilfile<<std::endl;

    std::string cortexfile1 = meshroot + "S2-brainc.bin";
    std::string bmeshfile1 = meshroot + "S2-scalp.bin";
    bool success= false;
    std::string conf_file = "sub-c2b006_1shell.json";


    EfieldInterface EfieldInterface(cortexfile1, bmeshfile1, success);
    std::cout << "Success: " << success<<std::endl;
    std::cout << "Elements: "<<success <<std::endl;
    std::cout<<"TMS_obj size Main: "<<EfieldInterface.TMS_obj->CortexSize()<<std::endl;


    // set example coil location to vertex 599 (in 0-basis) of the scalp mesh
    int32_t pind = 599;
    RowVector<float> cp = EfieldInterface.m.Points().GetRow(pind);
    std::cout <<"cp:"<< cp(0) <<std::endl;

//    // Define rotation matrix for the coil (Applied from right  x' = x*T_rot)
    Matrix<float, RowMajor> T_rot(3,3,{0.698830F, 0.0F, 0.715288F,
                                       0.118700F, 0.986135F, -0.115969F,
                                       -0.705370F, 0.165947F, 0.689140F});


    EfieldInterface.efield_estimation(cp, T_rot, efield_vector);

    return 0;
}
