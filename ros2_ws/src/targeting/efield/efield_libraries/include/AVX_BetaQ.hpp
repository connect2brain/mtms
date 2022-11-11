#pragma once

#include "BetaDipoles.hpp"
#include "Coil.hpp"
#include <linalg>


/*
Implements the approximate Beta integrals used to calculate the secondary magnetic field due to magnetic dipole coil
*/


ColVector<float> BetaQ(const Coil<float> &coil, const BetaDipoles<float> &bd);


ColVector<double> BetaQ(const Coil<double> &coil, const BetaDipoles<double> &bd);