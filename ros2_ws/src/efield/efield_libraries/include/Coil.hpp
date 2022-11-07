#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
#include <linalg>

enum CoilType {CurrentDipole = 100, MagneticDipole = 101, Polyline = 102, UnspecifiedCoilType = -1};

// A TMS coil discretized into nop quadrature points.
template <typename T>
class Coil {
public:

	/*	
		Coil can be constructed by either giving a path to the binary file containing the data 
		Type of coil is determined by identifier in the binary file
		100 - for  current dipole coil
		101 - for magnetic dipole coil
	 	102 - for polyline coil (not supported in this version)
		Coiltype can be later checked with the .CoilType() method
	*/
	// single coilfile constructor
	Coil(std::string coilfile);


	// Legacy multiple file constructors(excluding polyline), can be removed later once all the binary data is also transformed
	//magnetic dipole constructor
    Coil(std::string pointfile,  std::string normalfile, std::string weightfile);
	//current dipole constructor
	Coil(std::string pointfile,  std::string normalfile);


	//Matrix constructors (mainly for Debug) 
	Coil(Matrix<T, RowMajor> &p,  Matrix<T, RowMajor> &n, ColVector<T> &w);
	Coil(Matrix<T, RowMajor> &p,  Matrix<T, RowMajor> &n);
	Coil(Matrix<T, RowMajor> &p);
	Coil() = default;

	//Applies the rotation and translation to the TEMPLATE coil even if the coil is already configured to different orientation
	//Versions working both with colmajor and Rowmajor rotation matrices as well as overload to acept both row-and colvectors as translations
	void Transform(const Matrix<T, ColMajor> &rot,  const ColVector<T> &transl);
	void Transform(const Matrix<T, RowMajor> &rot,  const ColVector<T> &transl);
	void Transform(const Matrix<T, ColMajor> &rot,  const RowVector<T> &transl);
	void Transform(const Matrix<T, RowMajor> &rot,  const RowVector<T> &transl);
	// One vector with length 12 containing rotation matrix in row-major order and translation vector
	void Transform(const RowVector<T> &vect);
	void Transform(const ColVector<T> &vect);

	/*
	Pointer version. Rotation matrix in row-major order and multiplication done from right side  A_rotated = A*rot
	This handles the actual calculation. Implementation in Coil.cpp
	*/
	void Transform(const T* rot,  const T* transl);
	/*
	Pointer version with rotation and translation concatenated into 12 floats
	*/
	void Transform(const T* p){
		Transform(p , p + 9);
	}


	const Matrix<T, RowMajor>& Points() const {
        return points_;
    }
	const Matrix<T, RowMajor>& Moments() const {
        return moments_;
    }
	int32_t Nop() const {
		return nop_;
	}
	enum CoilType CoilType() const {
		return coiltype_;
	}

	Matrix<T, RowMajor> GetRotation() const {
		return rot_;
	}
	RowVector<T> GetTranslation() const {
		return transl_;
	}

private:
	Matrix<T, RowMajor> rot_ = Matrix<T, RowMajor>(3,3,{1, 0, 0, 0, 1, 0, 0, 0, 1});
	RowVector<T> transl_ = RowVector<T>(3, {0,0,0});

	void CheckDimensions();

	Matrix<T, RowMajor> points_;   // dipole locations, [nop x 3]
	Matrix<T, RowMajor> moments_;  // dipole moments [nop x 3]
	Matrix<T, RowMajor> template_points_;
	Matrix<T, RowMajor> template_moments_;

	int32_t nop_;
	enum CoilType coiltype_ = UnspecifiedCoilType; 
};
