/** \file matrix.h
 * <description>
 *
 * $Id: matrix.h,v 1.8 2000/11/22 13:13:29 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_MATRIX_H
#define NL_MATRIX_H

#include "nel/misc/vector.h"
#include "nel/misc/vector_h.h"


namespace	NLMISC
{

class	CPlane;


// ======================================================================================================
/**
 * A 4*4 Homogenous Matrix.
 * This is a column matrix, so operations like: \c v1=A*B*C*v0; applies C first , then B, then A to vector v0. \n
 * Since it is a column matrix, the first column is the I vector of the base, 2nd is J, 3th is K. \n
 * 4th column vector is T, the translation vector.
 *
 * Angle orientation are: Xaxis: YtoZ. Yaxis: ZtoX. Zaxis: XtoY.
 *
 * This matrix keep a matrix state to improve Matrix, vector and plane computing (matrix inversion, vector multiplication...).
 * The internal matrix know if:
 * - matrix is identity
 * - matrix has a translation component
 * - matrix has a rotation component
 * - matrix has a uniform scale component (scale which is the same along the 3 axis)
 * - matrix has a non-uniform scale component
 * - matrix has a projection component (4th line of the matrix is not 0 0 0 1).
 *
 * An example of improvement is that CMatrix::operator*(const CVector &v) return v if the matrix is identity.
 *
 * By default, a matrix is undefined. You must use copy ctor, operator=(), identity(), set(m44), or serial() with an 
 * input stream to fully define it. Any other operation on an undefined matrix will assert().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMatrix
{
public:
	/// Rotation Order.
	enum	TRotOrder
	{
		XYZ,
		XZY,
		YXZ,
		YZX,
		ZXY,
		ZYX
	};


public:

	/// \name Object
	//@{
	/// Constructor which do nothing!!
	CMatrix() {StateBit= 0; /* Not valid By default*/}
	/// Copy Constructor.
	CMatrix(const CMatrix &);
	/// operator=.
	CMatrix	&operator=(const CMatrix &);
	//@}



	/// \name Sets
	//@{
	/// Reset the matrix to identity.
	void		identity();
	/** Explicit setup the Rotation/Scale matrix (base).
	 * Avoid it. It implies low compute since no check is done on base to see what type of matrix it is 
	 * (identity, rotation, scale, uniform scale...)
	 * \param i The I vector of the cartesian base.
	 * \param j The J vector of the cartesian base.
	 * \param k The K vector of the cartesian base.
	 * \param hintNoScale set it to true if you are sure that your rot matrix is a pure rot matrix with no scale. 
	 * If set to true and your rotation is not an orthonormal basis, unpredictable result are excepted.
	 */
	void		setRot(const CVector &i, const CVector &j, const CVector &k, bool hintNoScale=false);
	/** Explicit setup the Rotation/Scale matrix (base).
	 * Avoid it. It implies low compute since no check is done on m33 to see what type of matrix it is 
	 * (identity, raotation, scale, uniform scale)
	 * \param m33 the 3*3 column rotation matrix. (3x3 matrix stored in column-major order as 9 consecutive values)
	 * \param hintNoScale set it to true if you are sure that your rot matrix is a pure rot matrix with no scale. 
	 * If set to true and your rotation is not an orthonormal basis, unpredictable result are excepted.
	 */
	void		setRot(const float m33[9], bool hintNoScale=false);
	/** Explicit setup the Translation component.
	 * v==Null is tested to see if the matrix now have a translation component.
	 * \param v the translation vector.
	 */
	void		setPos(const CVector &v);
	/** Explicit move the Translation component.
	 * \param v a vector to move the translation vector.
	 */
	void		movePos(const CVector &v);
	/** Explicit setup the Projection component.
	 * Proj is tested to see if the matrix now have a projection component.
	 * \param proj the 4th line of the matrix. Set it to 0 0 0 1 to reset it to default.
	 */
	void		setProj(const float proj[4]);
	/** Reset the Projection component to 0 0 0 1.
	 */
	void		resetProj();
	/** Explicit setup the 4*4 matrix.
	 * Avoid it. It implies low compute since no check is done on rotation matrix to see what type of matrix it is 
	 * (identity, rotation, scale, uniform scale).
	 * BUT check are made to see if it has translation or projection components.
	 * \param m44 the 4*4 column matrix (4x4 matrix stored in column-major order as 16 consecutive values)
	 */
	void		set(const float m44[16]);
	//@}



	/// \name Gets.
	//@{
	/** Get the Rotation/Scale matrix (base).
	 * \param i The matrix's I vector of the cartesian base.
	 * \param j The matrix's J vector of the cartesian base.
	 * \param k The matrix's K vector of the cartesian base.
	 */
	void		getRot(CVector &i, CVector &j, CVector &k) const;
	/** Get the Rotation/Scale matrix (base).
	 * \param m33 the matrix's 3*3 column rotation matrix. (3x3 matrix stored in column-major order as 9 consecutive values)
	 */
	void		getRot(float m33[9]) const;
	/** Get the Translation component.
	 * \param v the matrix's translation vector.
	 */
	void		getPos(CVector &v) const;
	/** Get the Translation component.
	 * \return the matrix's translation vector.
	 */
	CVector		getPos() const;
	/** Get the Projection component.
	 * \param proj the matrix's projection vector.
	 */
	void		getProj(float proj[4]) const;
	/// Get the I vector of the Rotation/Scale matrix (base).
	CVector		getI() const;
	/// Get the J vector of the Rotation/Scale matrix (base).
	CVector		getJ() const;
	/// Get the K vector of the Rotation/Scale matrix (base).
	CVector		getK() const;
	/** Get 4*4 matrix.
	 * \param m44 the matrix's 4*4 column matrix (4x4 matrix stored in column-major order as 16 consecutive values)
	 */
	void		get(float m44[16]) const;
	/** Get 4*4 matrix.
	 * \return the matrix's 4*4 column matrix (4x4 matrix stored in column-major order as 16 consecutive values)
	 */
	const float *get() const;
	/** Get Euler angles (in radians).
	 * \param ro the RotOrder which show how to get the euler.
	 * \return one solution of Euler angle. A rotate() with this value and the RotOrder ro will give the same matrix.
	 * \todo TODO.
	 */
	//CVector		toEuler(TRotOrder ro) const;
	//@}



	/// \name 3D Operations.
	//@{
	/// Apply a translation to the matrix. same as M=M*T
	void		translate(const CVector &v);
	/** Apply a rotation on axis X to the matrix. same as M=M*Rx
	 * \param a angle (in radian).
	 */
	void		rotateX(float a);
	/** Apply a rotation on axis Y to the matrix. same as M=M*Ry
	 * \param a angle (in radian).
	 */
	void		rotateY(float a);
	/** Apply a rotation on axis Z to the matrix. same as M=M*Rz
	 * \param a angle (in radian).
	 */
	void		rotateZ(float a);
	/** Apply a euler rotation.
	 * \param v a vector of 3 angle (in radian), giving rotation around each axis (x,y,z)
	 * \param ro the order of transformation applied. if ro==XYZ, then the transform is M=M*Rx*Ry*Rz
	 */
	void		rotate(const CVector &v, TRotOrder ro);
	/// Apply a uniform scale to the matrix.
	void		scale(float f);
	/// Apply a non-uniform scale to the matrix.
	void		scale(const CVector &scale);
	//@}



	/// \name Matrix Operations.
	//@{
	CMatrix	operator*(const CMatrix &) const;
	CMatrix	&operator*=(const CMatrix &);
	/** Invert the matrix.
	 * if the matrix can't be inverted, it is set to identity.
	 */
	void		invert();
	/** Return the matrix inverted.
	 * if the matrix can't be inverted, identity is returned.
	 */
	CMatrix		inverted() const;
	/** Normalize the matrix so that the rotation part is now an orthonormal basis, ie a rotation with no scale.
	 * NB: projection part and translation part are not modified.
	 * \param pref the preference axis order to normalize. ZYX means that K direction will be kept, and the plane JK 
	 * will be used to lead the I vector.
	 * \return false if One of the vector basis is null. true otherwise.
	 */
	bool		normalize(TRotOrder pref);
	//@}



	/// \ Vector Operations.
	//@{
	/// Multiply a normal. ie v.w=0 so the Translation component doesn't affect result. Projection doesn't affect result.
	CVector		mulVector(const CVector &v) const;
	/// Multiply a point. ie v.w=1 so the Translation component do affect result. Projection doesn't affect result.
	CVector		mulPoint(const CVector &v) const;
	/// Multiply a point. \sa mulPoint
	CVector		operator*(const CVector &v) const
	{
		return mulPoint(v);
	}

	/// Multiply
	CVectorH	operator*(const CVectorH& v) const;
	//@}

	/// \name Misc
	//@{
	void		serial(IStream &f);
	//@}

	// Friend.
	/// Plane (line vector) multiplication.
	friend CPlane		operator*(const CPlane &p, const CMatrix &m);


private:
	float	M[16];
	float	Scale33;
	uint32	StateBit;	// BitVector. 0<=>identity.

	// Methods For inversion.
	void	fastInvert33(CMatrix &ret) const;
	bool	slowInvert33(CMatrix &ret) const;
	bool	slowInvert44(CMatrix &ret) const;
	// access to M, in math conventions (mat(1,1) ... mat(4,4)).
	float	&mat(sint i, sint j)
	{
		return M[ (j<<2) + i];
	}
	const float	&mat(sint i, sint j) const
	{
		return M[ (j<<2) + i];
	}
	// return the good 3x3 Id to compute the minor of (i,j);
	void	getCofactIndex(sint i, sint &l1, sint &l2, sint &l3) const
	{
		switch(i)
		{
			case 0: l1=1; l2=2; l3=3; break;
			case 1: l1=0; l2=2; l3=3; break;
			case 2: l1=0; l2=1; l3=3; break;
			case 3: l1=0; l2=1; l3=2; break;
		}
	}
};


}


#endif // NL_MATRIX_H

/* End of matrix.h */
