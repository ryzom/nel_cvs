/* vectord_inline.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: vectord_inline.h,v 1.1 2000/09/14 16:29:44 berenguier Exp $
 *
 * CVectorD class inline definitions.
 */


#ifndef NL_VECTORD_INLINE_H
#define NL_VECTORD_INLINE_H


namespace	NLMISC
{


// ============================================================================================
// Base Maths.
inline	CVectorD	&CVectorD::operator+=(const CVectorD &v)
{
	x+=v.x;
	y+=v.y;
	z+=v.z;
	return *this;
}
inline	CVectorD	&CVectorD::operator-=(const CVectorD &v)
{
	x-=v.x;
	y-=v.y;
	z-=v.z;
	return *this;
}
inline	CVectorD	&CVectorD::operator*=(double f)
{
	x*=f;
	y*=f;
	z*=f;
	return *this;
}
inline	CVectorD	&CVectorD::operator/=(double f)
{
	return *this*= (1.0f/f);
}
inline	CVectorD	CVectorD::operator+(const CVectorD &v) const
{
	CVectorD	ret(x+v.x, y+v.y, z+v.z);
	return ret;
}
inline	CVectorD	CVectorD::operator-(const CVectorD &v) const
{
	CVectorD	ret(x-v.x, y-v.y, z-v.z);
	return ret;
}
inline	CVectorD	CVectorD::operator*(double f) const
{
	CVectorD	ret(x*f, y*f, z*f);
	return ret;
}
inline	CVectorD	CVectorD::operator/(double f) const
{
	return *this*(1.0f/f);
}
inline	CVectorD	CVectorD::operator-() const
{
	return CVectorD(-x,-y,-z);
}
inline CVectorD	operator*(double f, const CVectorD &v)
{
	CVectorD	ret(v.x*f, v.y*f, v.z*f);
	return ret;
}


// ============================================================================================
// Advanced Maths.
inline	double	CVectorD::operator*(const CVectorD &v) const
{
	return x*v.x + y*v.y + z*v.z;
}
inline	CVectorD	CVectorD::operator^(const CVectorD &v) const
{
	CVectorD	ret;

	ret.x= y*v.z - z*v.y;
	ret.y= z*v.x - x*v.z;
	ret.z= x*v.y - y*v.x;

	return ret;
}
inline	double	CVectorD::sqrnorm() const
{
	return (double)(x*x + y*y + z*z);
}
inline	double	CVectorD::norm() const
{
	return (double)sqrt(x*x + y*y + z*z);
}
inline	void	CVectorD::normalize()
{
	double	n=norm();
	if(n)
		*this/=n;
}
inline	CVectorD	CVectorD::normed() const
{
	CVectorD	ret;
	ret= *this;
	ret.normalize();
	return ret;
}


// ============================================================================================
// Misc.
inline	void	CVectorD::set(double _x, double _y, double _z)
{
	x=_x; y=_y; z=_z;
}
inline	bool	CVectorD::operator==(const CVectorD &v) const 
{
	return x==v.x && y==v.y && z==v.z;
}
inline	bool	CVectorD::operator!=(const CVectorD &v) const 
{
	return !(*this==v);
}
inline	bool	CVectorD::isNull() const
{
	return *this==CVectorD::Null;
}
inline	void	CVectorD::cartesianToSpheric(double &r, double &theta,double &phi) const
{
	CVectorD v;

	r= norm();
	v= normed();

	// phi E [-PI/2 et PI/2]
	phi=asin(v.z);

	// theta [-PI,PI]
	theta=atan2(v.x,v.y);
}
inline	void	CVectorD::sphericToCartesian(double r, double theta,double phi)
{
	double	ct= cos(theta);
	double	st= sin(theta);
	double	cp= cos(phi);
	double	sp= sin(phi);

	x= r*cos(theta)*cos(phi);
	y= r*sin(theta)*cos(phi);
	z= r*sin(phi);
}
inline CVectorD &CVectorD::operator=(const CVector &v)
{
	x=v.x;
	y=v.y;
	z=v.z;
	return *this;
}
inline CVectorD::operator CVector() const
{
	return CVector((float)x, (float)y, (float)z);
}
inline	void	CVectorD::serial(IStream &f)
{
	f.serial(x,y,z);
}


}


#endif