/* vector_inline.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: vector_inline.h,v 1.4 2000/10/06 10:27:36 lecroart Exp $
 *
 * CVector class inline definitions.
 */


#ifndef NL_VECTOR_INLINE_H
#define NL_VECTOR_INLINE_H


namespace	NLMISC
{


// ============================================================================================
// Base Maths.
inline	CVector	&CVector::operator+=(const CVector &v)
{
	x+=v.x;
	y+=v.y;
	z+=v.z;
	return *this;
}
inline	CVector	&CVector::operator-=(const CVector &v)
{
	x-=v.x;
	y-=v.y;
	z-=v.z;
	return *this;
}
inline	CVector	&CVector::operator*=(float f)
{
	x*=f;
	y*=f;
	z*=f;
	return *this;
}
inline	CVector	&CVector::operator/=(float f)
{
	return *this*= (1.0f/f);
}
inline	CVector	CVector::operator+(const CVector &v) const
{
	CVector	ret(x+v.x, y+v.y, z+v.z);
	return ret;
}
inline	CVector	CVector::operator-(const CVector &v) const
{
	CVector	ret(x-v.x, y-v.y, z-v.z);
	return ret;
}
inline	CVector	CVector::operator*(float f) const
{
	CVector	ret(x*f, y*f, z*f);
	return ret;
}
inline	CVector	CVector::operator/(float f) const
{
	return *this*(1.0f/f);
}
inline	CVector	CVector::operator-() const
{
	return CVector(-x,-y,-z);
}
inline CVector	operator*(float f, const CVector &v)
{
	CVector	ret(v.x*f, v.y*f, v.z*f);
	return ret;
}


// ============================================================================================
// Advanced Maths.
inline	float	CVector::operator*(const CVector &v) const
{
	return x*v.x + y*v.y + z*v.z;
}
inline	CVector	CVector::operator^(const CVector &v) const
{
	CVector	ret;

	ret.x= y*v.z - z*v.y;
	ret.y= z*v.x - x*v.z;
	ret.z= x*v.y - y*v.x;

	return ret;
}
inline	float	CVector::sqrnorm() const
{
	return (float)(x*x + y*y + z*z);
}
inline	float	CVector::norm() const
{
	return (float)sqrt(x*x + y*y + z*z);
}
inline	void	CVector::normalize()
{
	float	n=norm();
	if(n)
		*this/=n;
}
inline	CVector	CVector::normed() const
{
	CVector	ret;
	ret= *this;
	ret.normalize();
	return ret;
}


// ============================================================================================
// Misc.
inline	void	CVector::set(float _x, float _y, float _z)
{
	x=_x; y=_y; z=_z;
}
inline	bool	CVector::operator==(const CVector &v) const 
{
	return x==v.x && y==v.y && z==v.z;
}
inline	bool	CVector::operator!=(const CVector &v) const 
{
	return !(*this==v);
}
inline	bool	CVector::isNull() const
{
	return *this==CVector::Null;
}
inline	void	CVector::cartesianToSpheric(float &r, float &theta,float &phi) const
{
	CVector v;

	r= norm();
	v= normed();

	// phi E [-PI/2 et PI/2]
	phi=(float)asin(v.z);

	// theta [-PI,PI]
	theta=(float)atan2(v.x,v.y);
}
inline	void	CVector::sphericToCartesian(float r, float theta,float phi)
{
	double	ct= cos(theta);
	double	st= sin(theta);
	double	cp= cos(phi);
	double	sp= sin(phi);

	x=(float) (r*cos(theta)*cos(phi));
	y=(float) (r*sin(theta)*cos(phi));
	z=(float) (r*sin(phi));
}
inline	void	CVector::serial(IStream &f)
{
	f.serial(x,y,z);
}


}


#endif