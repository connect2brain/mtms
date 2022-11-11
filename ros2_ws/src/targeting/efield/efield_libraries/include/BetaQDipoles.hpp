#pragma once

#include <vector>
#include "BetaDipoles.hpp"
#include "Mesh.hpp"


/*
Calculates the quadrature points and moments used in calculating the secondary B-field using numerical Beta-integrals (1 quadrature point for vertex neighbourhood)
IN
 - Class Mesh 
OUT
 - Class BetaDipoles: contains 2 arrays with the modified positions and related moments for Beta integral calculation
*/
template <typename T>
BetaDipoles<T> BetaQDipoles(const std::vector<Mesh<T>> &meshes);
