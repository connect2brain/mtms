#pragma once 


#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include <linalg>


/* 
Calculates the dot product of BetaDipole moments and the vector potential at every Beta dipole location. Used to calculate the secondary E field. As = Phi*AlphaQ
*/

//float version
ColVector<float> AlphaQ(const Coil<float> &coil, const BetaDipoles<float> &bd);

//double version
ColVector<double>  AlphaQ(const Coil<double> &coil, const BetaDipoles<double> &bd);
