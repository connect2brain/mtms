//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// main.cpp
//
// Code generation for function 'main'
//

/*************************************************************************/
/* This automatically generated example C++ main file shows how to call  */
/* entry-point functions that MATLAB Coder generated. You must customize */
/* this file for your application. Do not modify this file directly.     */
/* Instead, make a copy of this file, modify it, and integrate it into   */
/* your development environment.                                         */
/*                                                                       */
/* This file initializes entry-point function arguments to a default     */
/* size and value before calling the entry-point functions. It does      */
/* not store or use any values returned from the entry-point functions.  */
/* If necessary, it does pre-allocate memory for returned values.        */
/* You can use this file as a starting point for a main function that    */
/* you can deploy in your application.                                   */
/*                                                                       */
/* After you copy the file, and before you deploy it, you must make the  */
/* following changes:                                                    */
/* * For variable-size function arguments, change the example sizes to   */
/* the sizes that your application requires.                             */
/* * Change the example values of function arguments to the values that  */
/* your application requires.                                            */
/* * If the entry-point functions return values, store these values or   */
/* otherwise use them as required by your application.                   */
/*                                                                       */
/*************************************************************************/

// Include files
#include "main.h"
#include "rt_nonfinite.h"
#include "run_processor.h"
#include "run_processor_terminate.h"
#include "coder_array.h"

// Function Declarations
static void argInit_62x1_real_T(double result[62]);

static boolean_T argInit_boolean_T();

static double argInit_real_T();

static unsigned short argInit_uint16_T();

static unsigned int argInit_uint32_T();

static unsigned long argInit_uint64_T();

static void main_run_processor();

// Function Definitions
static void argInit_62x1_real_T(double result[62])
{
  // Loop over the array to initialize each element.
  for (int idx0{0}; idx0 < 62; idx0++) {
    // Set the value of the array element.
    // Change this value to the value that the application requires.
    result[idx0] = argInit_real_T();
  }
}

static boolean_T argInit_boolean_T()
{
  return false;
}

static double argInit_real_T()
{
  return 0.0;
}

static unsigned short argInit_uint16_T()
{
  return 0U;
}

static unsigned int argInit_uint32_T()
{
  return 0U;
}

static unsigned long argInit_uint64_T()
{
  return 0UL;
}

static void main_run_processor()
{
  coder::array<double, 2U> out;
  double dv[62];
  // Initialize function 'run_processor' input arguments.
  // Initialize function input argument 'data_sample'.
  // Call the entry-point 'run_processor'.
  argInit_62x1_real_T(dv);
  run_processor(argInit_uint32_T(), argInit_uint16_T(), dv, argInit_uint64_T(),
                argInit_boolean_T(), out);
}

int main(int, char **)
{
  // The initialize function is being called automatically from your entry-point
  // function. So, a call to initialize is not included here. Invoke the
  // entry-point functions.
  // You can call entry-point functions multiple times.
  main_run_processor();
  // Terminate the application.
  // You do not need to do this more than one time.
  run_processor_terminate();
  return 0;
}

// End of code generation (main.cpp)
