//
// Created by Kalle Jyrkinen on 16.11.2021.
//

#ifndef TMS_CPP_MATH_HPP
#define TMS_CPP_MATH_HPP

// Helper math functions

#include <cmath>

namespace mathutil {

    inline int divup(int a, int b) {
        return (a + b - 1) / b; // = ceil(a/b)
    }

    template<typename T>
    inline T dotp(const T *a, const T *b) {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    template<typename T>
    inline T norm(const T *a) {
        return std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    }

    template<typename T>
    inline void crossp(const T *a, const T *b, T *dest) {
        dest[0] = a[1] * b[2] - a[2] * b[1];
        dest[1] = a[2] * b[0] - a[0] * b[2];
        dest[2] = a[0] * b[1] - a[1] * b[0];
    }

    template<typename T>
    inline T triple(const T *a, const T *b, const T *c) {
        return a[0] * (b[1] * c[2] - b[2] * c[1]) +
               a[1] * (b[2] * c[0] - b[0] * c[2]) +
               a[2] * (b[0] * c[1] - b[1] * c[0]);
    }
}

#endif //TMS_CPP_MATH_HPP
