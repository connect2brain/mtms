#ifndef EFIELD_ESTIMATION_H
#define EFIELD_ESTIMATION_H

#include "vector"

void efield_estimation(std::vector<float> &position, std::vector<float> &rot_matrix,
                       std::vector<double> &efield_vector);
void efield_estimation_vector(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<double> &efield_vector,
                              std::vector<double> &efield_vector_col1,std::vector<double> &efield_vector_col2,std::vector<double> &efield_vector_col3);
void efield_estimation_ROI(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<int32_t>& id_list, std::vector<double> &efield_vector, std::vector<double> &efield_vector_col1,std::vector<double> &efield_vector_col2,std::vector<double> &efield_vector_col3, std::vector<double> &efield_vector_array,int &maxIndex);
void efield_estimation_ROI_max_loc(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<int32_t>& id_list, std::vector<double> &efield_vector, std::vector<double> &efield_vector_array, int &maxIndex);

void init_efield(std::string cortexfile, std::vector<std::string> meshfile, std::vector<float> ci, std::vector<float> co, bool &success);
void set_coil(std::string coilfile, bool coilset, bool &success);
#endif
