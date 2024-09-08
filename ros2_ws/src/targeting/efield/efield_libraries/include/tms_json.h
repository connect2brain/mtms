//
// Created by mtms on 4.5.2023.
//

#ifndef HBFCPP_EXAMPLES_NBE_TMS_JSON_H
#define HBFCPP_EXAMPLES_NBE_TMS_JSON_H

void efield_estimation(std::vector<float> &position, std::vector<double> &orientation, std::vector<float> &rot_matrix,
                       std::vector<double> &efield_vector);
void init_efield(std::string cortexfile, std::vector<std::string> meshfile, std::vector<float> ci, std::vector<float> co, bool &success);
void add_coil(std::string coilfile, bool &success);
#endif //HBFCPP_EXAMPLES_NBE_TMS_JSON_H
