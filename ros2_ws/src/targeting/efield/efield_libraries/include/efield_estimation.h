#ifndef EFIELD_ESTIMATION_H
#define EFIELD_ESTIMATION_H

void init_efield();
void efield_estimation(std::vector<float>& position, std::vector<double>& orientation, std::vector<float>& rot_matrix, std::vector<double> &efield_vector);

#endif
