/* vector.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: vector.h,v 1.1 2000/09/13 17:58:46 berenguier Exp $
 *
 * CVector class
 */

#ifndef NL_VECTOR_H
#define NL_VECTOR_H

#include	<math.h>
#include	"nel/misc/stream.h"


namespace	NLMISC
{


// ======================================================================================================
/**
 * A 3D vector of float.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CVector
{
public:		// Attributes.
	float	x,y,z;

public:		// const.
	/// Null vector (0,0,0).
	static const	CVector		Null;
	/// I vector (1,0,0).
	static const	CVector		I;
	/// J vector (0,1,0).
	static const	CVector		J;
	/// K vector (0,0,1).
	static const	CVector		K;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor wich do nothing.
	CVector() {}
	/// Constructor .
	CVector(float	_x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	/// Copy Constructor.
	CVector(const CVector &v) {*this=v;}
	//@}

	/// @name Base Maths.
	//@{
	CVector	&operator+=(const CVector &v);
	CVector	&operator-=(const CVector &v);
	CVector	&operator*=(float f);
	CVector	&operator/=(float f);
	CVector	operator+(const CVector &v) const;
	CVector	operator-(const CVector &v) const;
	CVector	operator*(float f) const;
	CVector	operator/(float f) const;
	CVector	operator-() const;
	friend	CVector	operator*(float f, const CVector &v0);
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	float	operator*(const CVector &v) const;
	/** Cross product.
	 * compute the cross product *this ^ v.
	 */
	CVector	operator^(const CVector &v) const;
	/// Return the norm of the vector.
	float	norm() const;
	/// Return the square of the norm of the vector.
	float	sqrnorm() const;
	/// Normalize the vector.
	void	normalize();
	/// Return the vector normalized.
	CVector	normed() const;
	//@}

	/// @name Misc.
	//@{
	void	set(float _x, float _y, float _z);
	bool	operator==(const CVector &v) const;
	bool	operator!=(const CVector &v) const;
	void	serial(IStream &f);
	//@}

};


#include "nel/misc/vector_inline.h"


}


#endif // NL_VECTOR_H

/* End of vector.h */
