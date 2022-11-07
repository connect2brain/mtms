#pragma once

#include <new>
#include <cmath>
#include <malloc.h>
#include <immintrin.h>


// Define a 256-bit vector type that stores 8 floats. This is
// equivalent to Intel intrinsic type __m256.
#ifdef __clang__
typedef float float8_t __attribute__ ((ext_vector_type (8)));
#else
typedef float float8_t __attribute__ ((vector_size (8 * sizeof(float))));
#endif

/*
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        //redefine posix_memalign for windows
        #define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ?0 :errno)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __linux__
    // linux
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif
*/
// A function to allocate float8_t array of length n from heap.
// Quarantees proper memory alignment:

float8_t* f8_alloc(std::size_t n);

// All of the stuff above for doubles:

#ifdef __clang__
typedef double double4_t __attribute__ ((ext_vector_type (4)));
#else
typedef double double4_t __attribute__ ((vector_size (4 * sizeof(double))));
#endif


double4_t* d4_alloc(std::size_t n);





