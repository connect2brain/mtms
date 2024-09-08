#pragma once


// Forward declaration of Coil before Coilset is included which tries to include Coil again
// This is needed because Coil and Coilset both include each other
template <typename T>
class Coilset;
#include "Coil.hpp"

#include <vector>


/**
 * @brief Container class for multiple TMS coils. Used to easily construct many coils from a single file
 * @see Coil
 */
template <typename T>
class Coilset {
public:

	/**
	 * @brief Construct a new Coilset object from a file
	 * @details Coilset binary file structure: \n 
	 *  Int32 Coiltype  \n 
	 * 	Int32 N_coils  \n 
	 *	int32 startinds [N_coils] \n 
	 *	For j in N_coils: \n 
	 *		float dpos[3, startinds [j - 1]: startinds [j] ] \n 
 	 *		float dmom[3, startinds [j - 1]: startinds [j] ] \n 
	 * @param coilfile Binary file representing a coilset, see structure above
	 */
	Coilset(std::string coilfile);
	/**
	 * @brief Default constructor for creating empty Coilset object
	 */
	Coilset() = default;

	 /**
 	 * @brief Transform all coils in the coilset similarily. See Coil for detailed documentation on different overloads
 	 * 
 	 */
	 //@{
	void Transform(const Eigen::Matrix<T, 3, 3, Eigen::RowMajor> &rot,  const Eigen::Matrix<T, 3, 1> &transl);
	void Transform(const Eigen::Matrix<T, 3, 3, Eigen::RowMajor> &rot,  const Eigen::Matrix<T, 1, 3> &transl);
	void Transform(const Eigen::Matrix<T, 12, 1> &vect);
	void Transform(const Eigen::Matrix<T, 1, 12> &vect);
	void Transform(const T* rot,  const T* transl);
	void Transform(const T* p);
	//@}

	/**
	 * @brief returns the number of coils in the coilset
	 * 
	 * @return number of coils in the coilset
	 */
    int32_t Noc() const {
		return noc_;
	}

	/**
	 * @brief Returns coil type indicator
	 * @details 200:CurrentDipoleSet, 201:MagneticDipoleSet, 202:PolylineSet
	 */
	enum CoilType CoilType() const {
		return coiltype_;
	}

	/**
	 * @brief Returns the currently applied rotation matrix
	 */
	MatrixX3T_RM<T> GetRotation() const {
		return rot_;
	}
	/**
	 * @brief Returns the currently applied translation vector
	 */
	Eigen::Matrix<T, 1, 3> GetTranslation() const {
		return transl_;
	}
	/**
	 * @brief  Dipole positions of the coil
	 */
	const MatrixX3T_RM<T>& Points() const {
        return points_;
    }
	/**
	 * @brief  Dipole moments of the coil
	 */
	const MatrixX3T_RM<T>& Moments() const {
        return moments_;
    }

	/**
	 * @brief Returns sizes of coils as vector
	 */
    const Eigen::VectorXi& Sizes() const {
		return sizes_;
	}

	/**
	 * @brief Coil start indices for indexing Points() and Moments() matrices
	 */
    const Eigen::VectorXi& Startinds() const {
		return startinds_;
	}
		/**
	 * @brief returns the coil object with index ind
	 * @param ind index for choosing a coil in the coilset
	 * @return Coil object with index ind
	 */
	const Coil<T> GetCoil(int32_t ind) const;

	/**
	* @brief Conversion from Coil to Coilset with a single Coil
	* @param coil Instance of Coil class
	*/
	//explicit void operator=(Coil<T> coil);
	explicit Coilset(Coil<T> coil, int32_t Ncoils);




private:
	Eigen::Matrix<T, 3, 3, Eigen::RowMajor> rot_ = Eigen::Matrix<T, 3, 3, Eigen::RowMajor>::Identity(3,3);
	Eigen::Matrix<T, 1, 3> transl_ = Eigen::Matrix<T, 1, 3>::Zero(1,3);


	MatrixX3T_RM<T> points_;
	MatrixX3T_RM<T> moments_;
	MatrixX3T_RM<T> template_points_;
	MatrixX3T_RM<T> template_moments_;

	Eigen::VectorXi startinds_;
	Eigen::VectorXi sizes_;

    int32_t noc_;
	enum CoilType coiltype_ = UnspecifiedCoilType;

	friend class Coil<T>; // Allows Coil to access private variables

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                         Alternative C++ implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////