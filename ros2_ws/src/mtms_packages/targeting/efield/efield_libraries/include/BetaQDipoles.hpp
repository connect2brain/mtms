#pragma once

#include <vector>
#include "BetaDipoles.hpp"
#include "Mesh.hpp"



/**
 * @brief Calculates the quadrature points and moments used in calculating the secondary B-field using numerical Beta-integrals (1 quadrature point for vertex neighbourhood)
 * @details Implements Eqs. 6 and 7 from Stenroos & Koponen, Real-time computation of the TMS-induced electric field in a realistic head model, NeuroImage, 2019 Dec; 203:116159, doi: 10.1016/j.neuroimage.2019.116159.
 * @param meshes head model as a vector of meshes
 * @ingroup cpu_s_field
 * @return BetaDipoles object holding the points and moments
 */
template <typename T>
BetaDipoles<T> BetaQDipoles(const std::vector<Mesh<T>> &meshes);
