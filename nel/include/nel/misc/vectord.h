/* vectord.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: vectord.h,v 1.1 2000/09/14 16:29:44 berenguier Exp $
 *
 * CVectorD class
 */

#ifndef NL_VECTORD_H
#define NL_VECTORD_H

#include	<math.h>
#include	"nel/misc/vector.h"


namespace	NLMISC
{


// ======================================================================================================
/**
 * A 3D vector of double.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CVectorD
{
public:		// Attributes.
	double	x,y,z;

public:		// const.
	/// Null vector (0,0,0).
	static const	CVectorD		Null;
	/// I vector (1,0,0).
	static const	CVectorD		I;
	/// J vector (0,1,0).
	static const	CVectorD		J;
	/// K vector (0,0,1).
	static const	CVectorD		K;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor wich do nothing.
	CVectorD() {}
	/// Constructor .
	CVectorD(double	_x, double _y, double _z) : x(_x), y(_y), z(_z) {}
	/// Constructor with a CVector.
	CVectorD(const CVector &v) : x(v.x), y(v.y), z(v.z) {}
	/// Copy Constructor.
	CVectorD(const CVectorD &v) : x(v.x), y(v.y), z(v.z) {}
	//@}

	/// @name Base Maths.
	//@{
	CVectorD	&operator+=(const CVectorD &v);
	CVectorD	&operator-=(const CVectorD &v);
	CVectorD	&operator*=(double f);
	CVectorD	&operator/=(double f);
	CVectorD	operator+(const CVectorD &v) const;
	CVectorD	operator-(const CVectorD &v) const;
	CVectorD	operator*(double f) const;
	CVectorD	operator/(double f) const;
	CVectorD	operator-() const;
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	double	operator*(const CVectorD &v) const;
	/** Cross product.
	 * compute the cross product *this ^ v.
	 */
	CVectorD	operator^(const CVectorD &v) const;
	/// Return the norm of the vector.
	double	norm() const;
	/// Return the square of the norm of the vector.
	double	sqrnorm() const;
	/// Normalize the vector.
	void	normalize();
	/// Return the vector normalized.
	CVectorD	normed() const;
	//@}

	/// @name Misc.
	//@{
	void	set(double _x, double _y, double _z);
	bool	operator==(const CVectorD &v) const;
	bool	operator!=(const CVectorD &v) const;
	bool	isNull() const;
	/** 
	 * Setup the vector with spheric coordinates.
	 * the formula is:  \n
	 * x= r*cos(theta)*cos(phi) \n
	 * y= r*sin(theta)*cos(phi) \n
	 * z= r*sin(phi) \n
	 * \sa cartesianToSpheric()
	 */
	void	sphericToCartesian(double r, double theta,double phi);
	/**
	 * Get the sphreic coordinates of the vector.
	 * See sphericToCartesian() to know coordinates conventions.
	 * \sa sphericToCartesian()
	 */
	void	cartesianToSpheric(double &theta,double &phi, double &r) const;
	void	serial(IStream &f);
	CVectorD &operator=(const CVector &v);
	operator CVector() const;
	//@}

	// friends.
	friend	CVectorD	operator*(double f, const CVectorD &v0);
};


}


#include "nel/misc/vectord_inline.h"


#endif // NL_VECTOR_H

/* End of vector.h */
