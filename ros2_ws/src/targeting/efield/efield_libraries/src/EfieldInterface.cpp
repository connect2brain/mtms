//
// Created by mtms on 18.4.2023.
//

#include "EfieldInterface.h"
#include <phi_lc>
#include <tms>
#include <memory>


EfieldInterface::EfieldInterface(std::string cortexfile,std::string meshfile,bool &success):cortex(cortexfile)
    {
        std::string meshroot{"/home/mtms/Documents/Repositories/hbftms_cpp/data/headmodels/invesalius1/"};
        std::string coilroot{ "/home/mtms/Documents/Repositories/hbftms_cpp/data/coilmodels/"};

        std::string coilfile{coilroot + "magstim70/magstim70_42.bin"};

        coilmodel = Coil<float> (coilfile);
        // cortex(cortexfile);
        m = Mesh<float>(meshfile);
        std::cout<<"mesh:"<< m.Nop()<<std::endl;
        meshes = std::vector<Mesh<float>>({m});

//
        spos= Matrix<float, RowMajor>(cortex.Points());
        std::vector< Matrix<float> > D = BEMOperatorsPhi_LC(meshes);
        Matrix<float> TM = TM_Phi_LC(D, ci, co);
        Phi= LFM_Phi_LC(meshes, TM, spos);
        std::cout<<"spos: "<<spos(0,0)<<std::endl;
//
        TMS_obj= new TMS<float,float>(Phi, meshes, spos);
        std::cout<<"TMS_obj size: "<<TMS_obj->CortexSize()<<std::endl;
        WeightedPhi(meshes, Phi, ci, co);
        std::cout<<"TMS_obj size2: "<<TMS_obj->CortexSize()<<std::endl;
//
        success = true;
        std::cout<<"Done init! "<<std::endl;
    };

void EfieldInterface::E_norm(Matrix<float, RowMajor> &Etms, std::vector<double> &efield_vector)
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
    };


void EfieldInterface::efield_estimation(RowVector<float>& position, Matrix<float, RowMajor>& rot_matrix, std::vector<double> &efield_vector)
    {
        // set example coil location to vertex 599 (in 0-basis) of the scalp mesh
        int32_t pind = 1;
        std::cout<<"position "<<position(0)<<std::endl;
        std::cout<<"rot_matrix "<<rot_matrix(0,0)<<std::endl;
        // Define rotation matrix for the coil (Applied from right  x' = x*T_rot)
        //Matrix<float, RowMajor> T_rot(3,3,{rot_matrix});

        // Reciprocity: to convert the computed magnetic flux to E, multiply with -dI/dt.*/

        Matrix<float, RowMajor> Etms;
        //t.Start();
        // Apply the rotation T_rot and translation cp to the coil§
        coilmodel.Transform(rot_matrix,position);
        std::cout<<"coilmodel: "<<coilmodel.Nop()<<std::endl;
        auto coil_points = coilmodel.Points();
        std::cout<<"coilmodel: "<<coil_points(0,0)<<std::endl;
//        std::cout<<"coilmodel done"<<std::endl;
        std::cout<<"minusIPerdt "<<minusdIPerdt<<std::endl;
        std::cout<<"TMS_obj size efield estimation: "<<TMS_obj->CortexSize()<<std::endl;
//
//        // Calculate E-field
        Etms=TMS_obj->Efield(coilmodel, minusdIPerdt);
        std::cout<<"Etms done"<<std::endl;
        std::cout<<"YES YOU DID IT AGAIN!"<<std::endl;

        //t.Elapsed();
        E_norm(Etms, efield_vector);
//        std::cout<<"Enorm done"<<std::endl;
//
#ifdef USE_CUDA
        std::string savefile = "/home/mtms/Documents/Repositories/hbftms_cpp/data/headmodels/invesalius1/efields/E_tms1_example_cuda.bin";
#else
        std::string savefile = "/home/mtms/Documents/Repositories/hbftms_cpp/data/headmodels/invesalius1/efields/E_tms1_example_cpu.bin";
#endif
        std::cout << "Saving E in file " << savefile << std::endl;
        Etms.WriteToFile(savefile);
    };
