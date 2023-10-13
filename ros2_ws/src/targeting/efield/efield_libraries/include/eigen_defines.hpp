#pragma once

// Specify macros etc.
// This should be the first header file that is included during compilation => Any changes will cause the whole library to compile again..

// Standard library headers needed later
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdint>

#define EIGEN_MATRIXBASE_PLUGIN "MatrixBaseAddons.hpp"

#include "Eigen/Eigen"

/*
#define MatrixX3T_RM Eigen::Matrix<T, Eigen::Dynamic, 3, Eigen::RowMajor>
#define MatrixX3f_RM Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>
#define MatrixX3i_RM Eigen::Matrix<int32_t, Eigen::Dynamic, 3, Eigen::RowMajor>
#define MatrixX3d_RM Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>

#define MatrixXT<T> Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>
#define VectorXT<T> Eigen::Matrix<T, Eigen::Dynamic, 1>

#define MatrixX4T_RM<T> Eigen::Matrix<T, Eigen::Dynamic, 4, Eigen::RowMajor>
#define MatrixX4f_RM Eigen::Matrix<float, Eigen::Dynamic, 4, Eigen::RowMajor>
#define MatrixX4i_RM Eigen::Matrix<int32_t, Eigen::Dynamic, 4, Eigen::RowMajor>
*/
template <typename T> using MatrixX3T_RM = Eigen::Matrix<T, Eigen::Dynamic, 3, Eigen::RowMajor>;
using MatrixX3f_RM = Eigen::Matrix<float, Eigen::Dynamic, 3, Eigen::RowMajor>;
using MatrixX3i_RM = Eigen::Matrix<int32_t, Eigen::Dynamic, 3, Eigen::RowMajor>;
using MatrixX3d_RM = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;

template <typename T> using MatrixXT    = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
template <typename T> using VectorXT    = Eigen::Matrix<T, Eigen::Dynamic, 1>;

template <typename T> using MatrixX4T_RM = Eigen::Matrix<T, Eigen::Dynamic, 4, Eigen::RowMajor>;
using MatrixX4f_RM = Eigen::Matrix<float, Eigen::Dynamic, 4, Eigen::RowMajor>;
using MatrixX4i_RM = Eigen::Matrix<int32_t, Eigen::Dynamic, 4, Eigen::RowMajor>;
// addition by matti v230210
using MatrixXf_RM = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

/**
 * @brief Function for checking if A approx B (elementwise); used for testing
 * @details rms(A)*thresholdScale is used as the equality threshold (rms stands for root mean square). Should work with any Eigen matrices of similar size
 * 
 * @param A Matrix A
 * @param B Matrix B
 * @param thresholdScale Threshold to compare the absolute coefficientwise error
 * @return true if matrices are approximately same coefficientwise
 * @return false if not
 * @ingroup util
 */
template<typename Derived1, typename Derived2, typename T>
bool almostEqual(const Eigen::MatrixBase<Derived1> &A, const Eigen::MatrixBase<Derived2> &B, T thresholdScale) {
    if (A.rows() != B.rows() || A.cols() != B.cols()) return false;
    T thres = thresholdScale * std::sqrt(A.cwiseAbs2().mean());
    T maxAbsError = (A - B).cwiseAbs().maxCoeff();
    return maxAbsError < thres;
}

/**
 * @brief Calculates the maximum relative error by columns or rows
 * 
 * @param A Reference matrix
 * @param B Test matrix
 * @param dim 1 for comparing columns , 2 for rows
 * @return Maximum of relative errors
 * @ingroup util
 */
template<typename Derived1, typename Derived2>
float maxCompREs(const Eigen::MatrixBase<Derived1> &ref, const Eigen::MatrixBase<Derived2> &test, int32_t dim) {
    if((ref.rows() != test.rows()) || (ref.cols() != test.cols()) ){
        std::cout<< "Array dimensions ref: ["<< ref.rows() <<" x "<< ref.cols()<< "] , test: ["<< test.rows() <<" x "<< test.cols()<< "]"<<std::endl;
        throw std::invalid_argument("TRYING TO COMPARE ARRAYS OF DIFFERENT SIZES");
    }
    if (dim == 1){
        return (float) (((ref - test).colwise().norm()).cwiseProduct(ref.colwise().norm().cwiseInverse())  ).maxCoeff();
    }else if(dim == 2){
        return (float) (((ref - test).rowwise().norm()).cwiseProduct(ref.rowwise().norm().cwiseInverse())  ).maxCoeff();
    }else{
        throw std::invalid_argument("Unsupported dimension arugment in maxCompREs ( " + std::to_string(dim) + " )");
        return 0.0;
    }
}

/**
 * @brief Calculates the "correlation" between rows or columns of 2 matrices
 * 
 * @param A Reference matrix
 * @param B Test matrix
 * @param dim 1 for comparing columns , 2 for rows
 * @return Maximum correlation (cosine of the angle between n-dimensional vectors)
 * @ingroup util
 */
template<typename Derived1, typename Derived2>
float maxCompCCs(const Eigen::MatrixBase<Derived1> &ref, const Eigen::MatrixBase<Derived2> &test, int32_t dim) {
    if((ref.rows() != test.rows()) || (ref.cols() != test.cols()) ){
        std::cout<< "Array dimensions ref: ["<< ref.rows() <<" x "<< ref.cols()<< "] , test: ["<< test.rows() <<" x "<< test.cols()<< "]"<<std::endl;
        throw std::invalid_argument("TRYING TO COMPARE ARRAYS OF DIFFERENT SIZES");
    }
    if (dim == 1){
        auto mref = ref.colwise().mean();
        auto mtest = test.colwise().mean();

        Eigen::Matrix<typename Derived1::Scalar, Eigen::Dynamic, Eigen::Dynamic> d1 = ref;
        Eigen::Matrix<typename Derived2::Scalar, Eigen::Dynamic, Eigen::Dynamic> d2 = test;

        d1.rowwise() -= mref;
        d2.rowwise() -= mtest;
        auto d1n = d1.colwise().norm();
        auto d2n = d2.colwise().norm();

        return (float) (((d1.cwiseProduct(d2)).colwise().sum() ).cwiseProduct((d1n.cwiseProduct(d2n)).cwiseInverse()) ).maxCoeff();
    }else if(dim == 2){
        auto mref = ref.rowwise().mean();
        auto mtest = test.rowwise().mean();
        Eigen::Matrix<typename Derived1::Scalar, Eigen::Dynamic, Eigen::Dynamic> d1 = ref;
        Eigen::Matrix<typename Derived2::Scalar, Eigen::Dynamic, Eigen::Dynamic> d2 = test;

        d1.colwise() -= mref;
        d2.colwise() -= mtest;

        auto d1n = d1.rowwise().norm();
        auto d2n = d2.rowwise().norm();
        return (float) ( ((d1.cwiseProduct(d2)).rowwise().sum() ).cwiseProduct((d1n.cwiseProduct(d2n)).cwiseInverse())).maxCoeff();
    }else{
        throw std::invalid_argument("Unsupported dimension arugment in maxCompCCs ( " + std::to_string(dim) + " )");
        return 0.0;
    }
}

/**
 * @brief Calculates the max relative magnitudes between columns or rows of 2 arrays
 * 
 * @param A Reference matrix
 * @param B Test matrix
 * @param dim 1 for comparing columns , 2 for rows
 * @return Maximum correlation (cosine of the angle between n-dimensional vectors)
 * @ingroup util
 */
template<typename Derived1, typename Derived2>
float maxCompMAGs(const Eigen::MatrixBase<Derived1> &ref, const Eigen::MatrixBase<Derived2> &test, int32_t dim) {
    if((ref.rows() != test.rows()) || (ref.cols() != test.cols()) ){
        std::cout<< "Array dimensions ref: ["<< ref.rows() <<" x "<< ref.cols()<< "] , test: ["<< test.rows() <<" x "<< test.cols()<< "]"<<std::endl;
        throw std::invalid_argument("TRYING TO COMPARE ARRAYS OF DIFFERENT SIZES");
    }
    if (dim == 1){
        auto mref = ref.colwise().norm();
        auto mtest = test.colwise().norm();

        return (float) (mtest.cwiseProduct(mref.cwiseInverse())).maxCoeff();
    }else if(dim == 2){
        auto mref = ref.rowwise().norm();
        auto mtest = test.rowwise().norm();

        return (float) (mtest.cwiseProduct(mref.cwiseInverse())).maxCoeff();
    }else{
        throw std::invalid_argument("Unsupported dimension arugment in maxCompMAGs ( " + std::to_string(dim) + " )");
        return 0.0;
    }
}

/**
 * @brief returns a string with mean and max coefficientwise differences between matrices as well as their mean values as single matrices
 * 
 * @param M1 Matrix 1
 * @param M2 Matrix 2
 * @return string that can be later printed
 * @ingroup util
 */
template <class A >
std::string strdiff(A &M1 ,A &M2 ){
   if(M1.rows() != M2.rows() || M1.cols() != M2.cols()){
       std::string err = "Dimensions in strdiff do not match!\nM1: [ " + std::to_string(M1.rows()) + " X " + std::to_string(M1.cols()) 
                                                        + " ]\nM2: [ " + std::to_string(M2.rows()) + " X " + std::to_string(M2.cols()) + " ]\n";
       throw std::invalid_argument(err);
   }
   std::string s =  "Max diff:  " +  std::to_string((M1 - M2).cwiseAbs().maxCoeff()) 
   + " Mean diff: " +  std::to_string((M1 - M2).cwiseAbs().mean()) 
   + "\nMean values : " +   std::to_string(M1.cwiseAbs().mean()) 
   + " and " + std::to_string(M2.cwiseAbs().mean()) + "--------------\n" ;
   return s;
}

/**
 * @brief Calculate the condition number of a matrix in 2-norm
 * @note Uses SVD, very slow
 */
 template <typename T>
T Cond2(const MatrixXT<T> &A) {
    Eigen::JacobiSVD<MatrixXT<T>> svd(A);
    return svd.singularValues()(0) / svd.singularValues()(svd.singularValues().size()-1);
}

/**
 * @brief Generates a square matrix with given condition number (in 2-norm)
 * @details Uses eigen functionalities. Generates 2 random orthogonal matrices with QR decomposition 
 * and a diagonal matrix with uniformaly distributed singular values 1,.., cond and multiplies these together
 * @param n Size of the matrix
 * @param cond Desired condition number
 */
template <typename T>
MatrixXT<T> GenerateMatrix(const int32_t n, const T cond){
    // random seed
    srand((unsigned int) time(0));
    //diagonal as vector
    VectorXT<T> diag = VectorXT<T>::LinSpaced(n, 1.0, cond);
    //Orthogonal matrices
    MatrixXT<T> Q1, Q2; 

    {   // Generate Q1
        MatrixXT<T> A(n,n);
        A.setRandom();
        Eigen::HouseholderQR<MatrixXT<T>> qr(A);
        Q1 = qr.householderQ();
    }
    {   // Generate Q2
        MatrixXT<T> A(n,n);
        A.setRandom();
        Eigen::HouseholderQR<MatrixXT<T>> qr(A);
        Q2 = qr.householderQ();
    }
    //multiply
    return Q1*diag.asDiagonal()*Q2;
}


/**
 * @brief Prints the given string in green boldface with extra line changes
 */
inline void printHeader(std::string text ){
    std::cout << "\033[1;32m" << "************************************************** \n"
                         << "         " << text 
                         << "\n************************************************** "
                         <<"\033[0m \n" << std::endl;
}