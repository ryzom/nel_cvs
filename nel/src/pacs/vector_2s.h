/** \file vector_2s.h
 * <File description>
 *
 * $Id: vector_2s.h,v 1.2 2001/07/09 09:15:52 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_VECTOR_2S_H
#define NL_VECTOR_2S_H

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/vector.h"


namespace NLPACS {


const float		Vector2sAccuracy = 128.0f;

/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CVector2s
{
public:

public:		// Attributes.
	sint16	x, y;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor which do nothing.
	CVector2s() {}
	/// Constructor .
	CVector2s(sint16 _x, sint16 _y) : x(_x), y(_y) {}
	/// Constructor .
//	CVector2s(float _x, float _y) : x((sint16)(_x*256.0)), y((sint16)(_y*256.0)) {}
	/// Copy Constructor.
	CVector2s(const CVector2s &v) : x(v.x), y(v.y) {}
	CVector2s(const NLMISC::CVector &v) : x((sint16)(v.x*Vector2sAccuracy)), y((sint16)(v.y*Vector2sAccuracy)) {}
	//@}

	/// @name Base Maths.
	//@{
	CVector2s	&operator+=(const CVector2s &v)		{x+=v.x; y+=v.y; return *this;}
	CVector2s	&operator-=(const CVector2s &v)		{x-=v.x; y-=v.y; return *this;}
	CVector2s	operator+(const CVector2s &v) const	{return CVector2s(x+v.x, y+v.y);}
	CVector2s	operator-(const CVector2s &v) const	{return CVector2s(x-v.x, y-v.y);}
	CVector2s	operator-() const					{return CVector2s(-x, -y);}

	CVector2s	&operator*=(float f)				{ x = (sint16)(f*x); y = (sint16)(f*y); return *this; }
	CVector2s	&operator/=(float f)				{ x = (sint16)(f/x); y = (sint16)(f/y); return *this; }
	CVector2s	operator*(float f) const			{return CVector2s((sint16)(x*f), (sint16)(y*f));}
	CVector2s	operator/(float f) const			{return CVector2s((sint16)(x/f), (sint16)(y/f));}
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	float	operator*(const CVector2s &v) const		{return (float)((sint32)x*(sint32)v.x + (sint32)y*(sint32)v.y)/(Vector2sAccuracy*Vector2sAccuracy);}
	/// Return the norm of the vector.
	float	norm() const							{return (float)sqrt(sqrnorm());}
	/// Return the square of the norm of the vector.
	float	sqrnorm() const							{return (float)((sint32)x*(sint32)x + (sint32)y*(sint32)y)/(Vector2sAccuracy*Vector2sAccuracy);}
	/// Normalize the vector.
	void	normalize()
	{
		float	f= norm();
		if(f>0)
			*this/=f;
	}
	/// Return the vector normalized.
	CVector2s	normed() const
	{
		CVector2s	v= *this;
		v.normalize();
		return v;
	}
	//@}

	/// @name Misc.
	//@{
	void	set(sint16 _x, sint16 _y)				{x= _x; y=_y;}
	bool	operator==(const CVector2s &v) const	{return x==v.x && y==v.y;}
	bool	operator!=(const CVector2s &v) const	{return !(*this==v);}
	bool	isNull() const							{return x==0.0f && y==0.0f;}
	/// Set all vector x/y/z as minimum of a/b x/y/z  (respectively).
	void	minof(const CVector2s &a, const CVector2s &b)
	{
		x= std::min(a.x, b.x);
		y= std::min(a.y, b.y);
	}
	/// Set all vector x/y/z as maximum of a/b x/y/z  (respectively).
	void	maxof(const CVector2s &a, const CVector2s &b)
	{
		x= std::max(a.x, b.x);
		y= std::max(a.y, b.y);
	}
	/// serial.
	void	serial(NLMISC::IStream &f)				{f.serial(x,y);}
	//@}

	void				pack(const NLMISC::CVector &v)		{ x = (sint16)(v.x*Vector2sAccuracy); y = (sint16)(v.y*Vector2sAccuracy); }
	void				pack(const NLMISC::CVector2f &v)	{ x = (sint16)(v.x*Vector2sAccuracy); y = (sint16)(v.y*Vector2sAccuracy); }
	NLMISC::CVector2f	unpack() const						{ return NLMISC::CVector2f((float)x/Vector2sAccuracy, (float)y/Vector2sAccuracy); }
	NLMISC::CVector		unpack3f() const					{ return NLMISC::CVector((float)x/Vector2sAccuracy, (float)y/Vector2sAccuracy, 0.0f); }
};

} // NLPACS


#endif // NL_VECTOR_2S_H

/* End of vector_2s.h */
