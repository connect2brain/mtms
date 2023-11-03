//The functions in this file are partially based on hbftms_cpp library example codes written by M. Stenroos
//This library is not free to use without written permission from the owner Matti Stenroos
#define USE_CUDA
#include "tms_cpp.hpp"
#ifdef USE_CUDA
#include "TMS_GPU.hpp"
#endif
#include "TMS_CPU.hpp"
#include "tms_json.h"
//#include "efield_estimation.h"
#include <iostream>
#include <algorithm>
using namespace Eigen;
using namespace std;
std::vector<double> efield_vector;
Timer t;
Mesh<float> *cortex;
Coil<float> coilmodel;
Coilset<float> coils;
std::vector< Mesh<float>>meshes;
MatrixXf Phi;
MatrixX3T_RM<float> spos;
Timer timer;
bool mtms_coil;
#ifdef USE_CUDA
TMS_GPU *TMS_obj;
#else
TMS <float,float>*TMS_obj;
#endif

std::vector<float> dIperdt_vect;

//This function Initializes the Efield service on my application, here is modified for this test
void init_efield(std::string cortexfile, std::vector<std::string> meshfile, std::vector<float> ci, std::vector<float> co, bool &success)
{
    meshes.clear();
    for (auto element: meshfile){
        meshes.push_back(element);
    }
    cortex =new Mesh<float>(cortexfile);
    spos= cortex->Points();

#ifdef USE_CUDA
    timer.Start();
    vector< MatrixXf > D = BEMOperatorsPhi_LC_GPU(meshes);
    MatrixXf TM = TM_Phi_LC_GPU(D, ci, co);
    Phi = LFM_Phi_LC_GPU(meshes, TM, spos);
    cout << "Time for the full phi calculations and TMS setup:  " << timer.GetElapsed() << std::endl;
#else
    vector< MatrixXf > D = BEMOperatorsPhi_LC(meshes);
    MatrixXf TM = TM_Phi_LC(D, ci, co);
    Phi = LFM_Phi_LC(meshes, TM, spos);
    cout<<"Phi dimensions: "<< Phi.rows() << " " <<Phi.cols()<<std::endl;
#endif
    WeightedPhi(meshes, Phi, ci, co);

#ifdef USE_CUDA
    TMS_obj= new TMS_GPU(Phi, meshes, spos);
#else
    TMS_obj=new TMS<float,float>(Phi, meshes, spos);
#endif
    success = true;
    std::cout<<"Done init! "<<std::endl;
}

void set_coil(std::string coilfile, bool coilset, bool &success)
{
    if (coilset){
        coils = Coilset<float> (coilfile);
    } else {
        coilmodel = Coil<float>(coilfile);
    }
    std::cout << "coilfile changed: " << coilfile << std::endl;
    mtms_coil = coilset;
    std::cout << "coil_set: " << mtms_coil << std::endl;
    success = true;
}

void set_dIperdt(std::vector<float> &dIperdt_vectInv, bool &success)
{   dIperdt_vect.clear();
    dIperdt_vect = dIperdt_vectInv;
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
        //efield_vector_column1.push_back(Etms(i,0));
        //efield_vector_column2.push_back(Etms(i,1));
        //efield_vector_column3.push_back(Etms(i,2));

    }
}

//void efield_estimation(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<double> &efield_vector,std::vector<double> &efield_vector_column1, std::vector<double> &efield_vector_column2, std::vector<double> &efield_vector_column3)
//void efield_estimation(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<std::vector<double>>& trial_vector)
void efield_estimation(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<double> &efield_vector)
{
    //** Convert vectors to Eigen matrices
    Matrix<float, 3, 3, RowMajor> T_rot;
    T_rot = Map<Matrix<float, 3, 3, RowMajor>>(rot_matrix.data(), 3,3);
    Matrix<float, 3, 1> cp;
    cp = Map<Matrix<float, 3, 1>>(position.data(),3,1);

    // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.
    float minusdIPerdt = -6600.0*10000.0;

    MatrixX3f_RM Etms;
    t.Start();
    // Apply the rotation T_rot and translation cp to the coil§
    coilmodel.Transform(T_rot,cp);

    //** Calculate E-field
    Etms = TMS_obj->Efield(coilmodel, minusdIPerdt);
    t.Elapsed();
    E_norm(Etms, efield_vector);
    //trial_vector.reserve(Etms.rows());

    //for (int i =0; i<Etms.rows();i++){
    //    trial_vector.push_back({Etms(i,0),Etms(i,1),Etms(i,2)});
    //}
}

void efield_estimation_ROI(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<int32_t>& id_list, std::vector<double> &efield_vector, std::vector<double> &efield_vector_col1,std::vector<double> &efield_vector_col2,std::vector<double> &efield_vector_col3,std::vector<double> &efield_vector_array,int &maxIndex)
{
    //** Convert vectors to Eigen matrices
    Matrix<float, 3, 3, RowMajor> T_rot;
    T_rot = Map<Matrix<float, 3, 3, RowMajor>>(rot_matrix.data(), 3,3);
    Matrix<float, 3, 1> cp;
    cp = Map<Matrix<float, 3, 1>>(position.data(),3,1);
    VectorXi Id_list(id_list.size());
    Id_list = Map<VectorXi>(id_list.data(), id_list.size());

    // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.
    //float minusdIPerdt = -6600.0*10000.0;
    if (mtms_coil) {
        t.Start();

        vector <MatrixX3f_RM> Ecoilset(coils.Noc());
        coils.Transform(T_rot, cp);
        MatrixX3f_RM sumEfield = MatrixX3f_RM::Zero(id_list.size(), 3);

        for (int I = 0; I < coils.Noc(); I++) {
            Ecoilset[I] = TMS_obj->Efield(coils.GetCoil(I), -dIperdt_vect.at(I), Id_list);
            sumEfield += Ecoilset[I];
            std::cout << "dIperdt: " << dIperdt_vect.at(I) << std::endl;
        }

        E_norm(sumEfield, efield_vector);
        t.Elapsed();
        std::cout << "efield vector:" << efield_vector.size() << std::endl;
        auto max = std::max_element(efield_vector.begin(), efield_vector.end());
        std::cout << "Max value" << *max << std::endl;
        maxIndex = std::distance(efield_vector.begin(), max);
        std::cout << "Index" << maxIndex << std::endl;

        std::cout << "Enorm: " << sumEfield.rows() << std::endl;
        efield_vector_array.push_back(sumEfield(maxIndex, 0));
        efield_vector_array.push_back(sumEfield(maxIndex, 1));
        efield_vector_array.push_back(sumEfield(maxIndex, 2));

        for (int i = 0; i < sumEfield.rows(); i++) {
            efield_vector_col1.push_back(sumEfield(i, 0));
            efield_vector_col2.push_back(sumEfield(i, 1));
            efield_vector_col3.push_back(sumEfield(i, 2));
        }

    } else {

        MatrixX3f_RM Etms;

        t.Start();
        // Apply the rotation T_rot and translation cp to the coil§
        coilmodel.Transform(T_rot, cp);

        //** Calculate E-field
        Etms = TMS_obj->Efield(coilmodel, -dIperdt_vect.at(0), Id_list);
        t.Elapsed();
        E_norm(Etms, efield_vector);
        std::cout << "Enorm: " << Etms.rows() << std::endl;
        std::cout << "id_list" << Id_list.size() << std::endl;
        std::cout << "efield vector:" << efield_vector.size() << std::endl;

        auto max = std::max_element(efield_vector.begin(), efield_vector.end());
        std::cout << "Max value" << *max << std::endl;
        maxIndex = std::distance(efield_vector.begin(), max);
        std::cout << "Index " << maxIndex << std::endl;
        efield_vector_array.push_back(Etms(maxIndex, 0));
        efield_vector_array.push_back(Etms(maxIndex, 1));
        efield_vector_array.push_back(Etms(maxIndex, 2));

        for (int i = 0; i < Etms.rows(); i++) {
            efield_vector_col1.push_back(Etms(i, 0));
            efield_vector_col2.push_back(Etms(i, 1));
            efield_vector_col3.push_back(Etms(i, 2));
        }
    }
}

void efield_estimation_ROI_max_loc(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<int32_t>& id_list, std::vector<double> &efield_vector, std::vector<double> &efield_vector_array, int &maxIndex)
{
    //** Convert vectors to Eigen matrices
    Matrix<float, 3, 3, RowMajor> T_rot;
    T_rot = Map<Matrix<float, 3, 3, RowMajor>>(rot_matrix.data(), 3,3);
    Matrix<float, 3, 1> cp;
    cp = Map<Matrix<float, 3, 1>>(position.data(),3,1);

    // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.
//float minusdIPerdt = -6600.0*10000.0;


    VectorXi Id_list(id_list.size());
    Id_list = Map<VectorXi>(id_list.data(), id_list.size());
    std::cout << "coil_set: " << mtms_coil << std::endl;

    if (mtms_coil){
        t.Start();

        vector< MatrixX3f_RM > Ecoilset(coils.Noc());
        coils.Transform(T_rot,cp);
        MatrixX3f_RM sumEfield = MatrixX3f_RM::Zero(id_list.size(),3);

        for (int I = 0; I < coils.Noc(); I++) {
            Ecoilset[I] = TMS_obj->Efield(coils.GetCoil(I), dIperdt_vect.at(I), Id_list);
            sumEfield += Ecoilset[I];
            std::cout<<"dIperdt: "<<-dIperdt_vect.at(I)<<std::endl;
        }

        E_norm(sumEfield, efield_vector);
        t.Elapsed();
        std::cout<<"efield vector:"<<efield_vector.size()<<std::endl;
        auto max = std::max_element(efield_vector.begin(), efield_vector.end());
        std::cout<<"Max value"<<*max<<std::endl;
        maxIndex = std::distance(efield_vector.begin(), max);
        std::cout<<"Index"<<maxIndex<<std::endl;

        std::cout << "Enorm: " << sumEfield.rows() << std::endl;
        efield_vector_array.push_back(sumEfield(maxIndex,0));
        efield_vector_array.push_back(sumEfield(maxIndex,1));
        efield_vector_array.push_back(sumEfield(maxIndex,2));

    } else {
        // Apply the rotation T_rot and translation cp to the coil§
        t.Start();
        MatrixX3f_RM Etms;
        coilmodel.Transform(T_rot, cp);
        std::cout<<"T_rot: "<<T_rot<<std::endl;
        std::cout<<"cp: "<<cp<<std::endl;
        //** Calculate E-field
        Etms = TMS_obj->Efield(coilmodel, -dIperdt_vect.at(0), Id_list);
        t.Elapsed();
        E_norm(Etms, efield_vector);

        std::cout<<"efield vector:"<<efield_vector.size()<<std::endl;
        auto max = std::max_element(efield_vector.begin(), efield_vector.end());
        std::cout<<"Max value"<<*max<<std::endl;
        maxIndex = std::distance(efield_vector.begin(), max);
        std::cout<<"Index"<<maxIndex<<std::endl;

        std::cout << "Enorm: " << Etms.rows() << std::endl;
        efield_vector_array.push_back(Etms(maxIndex,0));
        efield_vector_array.push_back(Etms(maxIndex,1));
        efield_vector_array.push_back(Etms(maxIndex,2));
    }
    std::cout<<"id_list"<<Id_list.size()<<std::endl;

}


void efield_estimation_vector(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<double> &efield_vector, std::vector<double> &efield_vector_col1,std::vector<double> &efield_vector_col2,std::vector<double> &efield_vector_col3)
{
    //** Convert vectors to Eigen matrices
    Matrix<float, 3, 3, RowMajor> T_rot;
    T_rot = Map<Matrix<float, 3, 3, RowMajor>>(rot_matrix.data(), 3,3);
    Matrix<float, 3, 1> cp;
    cp = Map<Matrix<float, 3, 1>>(position.data(),3,1);

    // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.
    float minusdIPerdt = -6600.0*10000.0;

    MatrixX3f_RM Etms;
    t.Start();
    // Apply the rotation T_rot and translation cp to the coil§
    coilmodel.Transform(T_rot,cp);

    //** Calculate E-field

    Etms = TMS_obj->Efield(coilmodel, minusdIPerdt);

    t.Elapsed();
    E_norm(Etms, efield_vector);
    for (int i = 0; i < Etms.rows(); i++)
    {
        efield_vector_col1.push_back(Etms(i,0));
        efield_vector_col2.push_back(Etms(i,1));
        efield_vector_col3.push_back(Etms(i,2));
    }
}

int main(void)
{
    return 0;
}
