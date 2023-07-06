#ifndef EFIELD_ESTIMATION_H
#define EFIELD_ESTIMATION_H

#include "vector"

void efield_estimation(std::vector<float> &position, std::vector<float> &rot_matrix,
                       std::vector<double> &efield_vector);
void efield_estimation_vector(std::vector<float>& position, std::vector<float>& rot_matrix, std::vector<double> &efield_vector,
                              std::vector<double> &efield_vector_col1,std::vector<double> &efield_vector_col2,std::vector<double> &efield_vector_col3);
void init_efield(std::string cortexfile, std::vector<std::string> meshfile, std::vector<float> ci, std::vector<float> co, bool &success);
void set_coil(std::string coilfile, bool &success);
#endif
