#pragma once
#include <string>
#include "eigen_defines.hpp"

/// Enumeration for different Coil and Coilset types
enum CoilType {	CurrentDipole = 100, 
				MagneticDipole = 101, 
				Polyline = 102,
				CurrentDipoleSet = 200, 
				MagneticDipoleSet = 201, 
				PolylineSet = 202, 
				UnspecifiedCoilType = -1
				};

// Forward declaration of Coil before Coilset is included which tries to include Coil again
// This is needed because Coil and Coilset both include each other
template <typename T>
class Coil;

#include "Coilset.hpp"


/** 
*  @brief Class for a discretized %TMS coil
*  @note Can hold a Current dipole (100), Magnetic dipole (101) or polyline(102) coil
*
*/
template <typename T>
class Coil {
public:

	/**
	 * @brief file constructor for Coil class
	 * @details Coil binary file structure: \n 
	 * int32 type \ref CoilType \n 
	 * int32 NoD: Number of dipoles/points \n 
	 * float dpos[3, NoD] : dipole positions \n 
	 * float dmom[3, NoD] : dipole moments (polyline coil doesnt have moments) \n 
	 * 
	 * @param coilfile Coil as a binary file, see the structure above
	 */
	Coil(std::string coilfile);

	/**
	 * @details Matrix constructor for debug and internal use (magnetic dipole coil with weights)
	 * 
	 * @param p points
	 * @param n normals
	 * @param w weigths
	 */
	Coil(const MatrixX3T_RM<T> &p,  const MatrixX3T_RM<T> &n, const VectorXT<T> &w);
	/**
	 * @details Matrix constructor for debug and internal use (Magnetic or current dipole)
	 * 
	 * @param p  points 
	 * @param n  normals or moments
	 * @param ct coiltype 
	 */
	Coil(const MatrixX3T_RM<T> &p,  const MatrixX3T_RM<T> &n, enum CoilType ct = CurrentDipole);
	/**
	 * @details Matrix constructor for debug and internal use (polyline)
	 * 
	 * @param p points
	 */
	Coil(const MatrixX3T_RM<T> &p);
	/**
	 * @details Default constructor for creating empty Coil object
	 * 
	 */
	Coil() = default;

	/**
	 * @details Construct Coil from Coilset
	 * 
	 */
	explicit Coil(Coilset<T> coilset);


	/**
	 * @brief Applies rotation and translation to the coil
	 * 
	 * @note rotation and translation are applied to the TEMPLATE coil even if the coil is already configured to different orientation \n
	 * @note Multiplication with the rotation matrix rot is done from the right side  A_rotated = A*rot \n
	 * @note This pointer version handles the actual calculation. Implementation in Coil.cpp
	 * 
	 * @param rot Pointer to a 3x3 rotation matrix in row-major order
	 * @param transl Pointer to a translation vector of length 3
	 */
	void Transform(const T* rot,  const T* transl);


	/**
	 * @overload
	 * 
	 * @param rot Rotation matrix (Eigen Row-major 3x3)
	 * @param transl Translation vector (Eigen 3x1 i.e. Vector) 
	 */
	void Transform(const Eigen::Matrix<T, 3, 3, Eigen::RowMajor> &rot,  const Eigen::Matrix<T, 3, 1> &transl);
	/**
	 * @overload
	 * 
	 * @param rot Rotation matrix (Eigen Row-major 3x3)
	 * @param transl Translation vector (Eigen 1x3 i.e. RowVector) 
	 */
	void Transform(const Eigen::Matrix<T, 3, 3, Eigen::RowMajor> &rot,  const Eigen::Matrix<T, 1, 3> &transl);
	/**
	 * @overload
	 * 
	 * @param vect Rotation matrix and translation as a single column vector (Eigen 12x1 i.e. Vector) 
	 */
	void Transform(const Eigen::Matrix<T, 12, 1> &vect);
	/**
	 * @overload
	 * 
	 * @param vect Rotation matrix and translation as a single row vector (Eigen 1x12 i.e. RowVector) 
	 */
	void Transform(const Eigen::Matrix<T, 1, 12> &vect);
	/**
	 * @overload
	 * 
	 * @param p pointer to a rotation matrix and translation as a single array
	 */
	void Transform(const T* p);

	/**
	 * @return Dipole positions of the coil
	 */
	const MatrixX3T_RM<T>& Points() const {
        return points_;
    }
	/**
	 * @return Dipole moments of the coil
	 */
	const MatrixX3T_RM<T>& Moments() const {
        return moments_;
    }
	/**
	 * @return Number of dipoles in the coil
	 */
	int32_t Nop() const {
		return nop_;
	}
	/**
	 * @return Type of the coil as defined in enumeration CoilType
	 */
	enum CoilType CoilType() const {
		return coiltype_;
	}
	/**
	 * @return Current rotation applied to the coil
	 */
	MatrixX3T_RM<T> GetRotation() const {
		return rot_;
	}
	/**
	 * @return Current translation applied to the coil
	 */
	Eigen::Matrix<T, 1, 3> GetTranslation() const {
		return transl_;
	}

	// Adding copy and move constrcutors should make loading coils faster but it doesnt seem so
/*
	Coil(const Coil& other) // copy constructor
	:	rot_(other.rot_),
		transl_(other.transl_),
		points_(other.points_),
		moments_(other.moments_),
		template_points_(other.template_points_),
		template_moments_(other.template_moments_),
		nop_(other.nop_),
		coiltype_(other.coiltype_){std::cout<< "coil move constructor"<<std::endl;}
		
	Coil& operator=(const Coil& other) // copy assignment
    {
        return *this = Coil(other);
    }
 
	
    Coil(Coil&& other) noexcept // move constructor
    : 	rot_(std::move(other.rot_)),
		transl_(std::move(other.transl_)),
		points_(std::move(other.points_)),
		moments_(std::move(other.moments_)),
		template_points_(std::move(other.template_points_)),
		template_moments_(std::move(other.template_moments_)),
		nop_(std::exchange(other.nop_,0)),
		coiltype_(std::exchange(other.coiltype_, UnspecifiedCoilType)) {std::cout<< "coil move constructor"<<std::endl;}
	

 
    Coil& operator=(Coil&& other) noexcept // move assignment
    {
        return *this = Coil(other);
    }
*/
private:
	Eigen::Matrix<T, 3, 3, Eigen::RowMajor> rot_ = Eigen::Matrix<T, 3, 3, Eigen::RowMajor>::Identity(3,3);
	Eigen::Matrix<T, 1, 3> transl_ = Eigen::Matrix<T, 1, 3>::Zero(1,3);

	void CheckDimensions();

	MatrixX3T_RM<T> points_;   // dipole positions, [nop x 3]
	MatrixX3T_RM<T> moments_;  // dipole moments [nop x 3]
	MatrixX3T_RM<T> template_points_;
	MatrixX3T_RM<T> template_moments_;

	int32_t nop_;
	enum CoilType coiltype_ = UnspecifiedCoilType; 

	friend class Coilset<T>; // Allows Coilset to access private variables
};

/************************************************/
/*              Standalone functions            */
/************************************************/


/**
* @brief helper to return the single coil type correspondign to the coilset type
*/
enum CoilType GetSingleCoilType(enum CoilType ct);
/**
* @brief helper to return the coilset type correspondign to a single coil type
*/
enum CoilType GetMultiCoilType(enum CoilType ct);

 /**
  * @brief Loads a vector of Coils from a single binary file. See Coilset.hpp for info on the binary file structure
  * 
  * @param coilfile  Binary file representing a coilset (same file can be used to construct the specific class Coilset<T>)
  * @return vector of Coils
  * @ingroup util
  */
template <typename T>
void LoadCoils(std::vector<Coil<T>> &coils, std::string coilfile);




