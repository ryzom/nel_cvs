/** \file quat.cpp
 * CQuat class
 *
 * $Id: quat.cpp,v 1.1 2000/11/10 11:19:21 lecroart Exp $
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

#include "nel/misc/quat.h"

namespace	NLMISC
{

// ---------------------------------------------
// --- Constructors
CQuat::CQuat(float X, float Y, float Z, float W)
{ 
	x = X; 
	y = Y; 
	z = Z; 
	w = W; 
}

CQuat::CQuat(double X, double Y, double Z, double W)  
{ 
	x = (float)X; 
	y = (float)Y; 
	z = (float)Z; 
	w = (float)W; 
}

CQuat::CQuat(const CQuat& a) 
{ 
	x = a.x; 
	y = a.y; 
	z = a.z; 
	w = a.w; 
} 

CQuat::CQuat(float af[4]) 
{ 
	x = af[0]; 
	y = af[1]; 
	z = af[2]; 
	w = af[3]; 
}

// ---------------------------------------------
// --- Accessors
float& CQuat::operator[](uint i) 
{ 
	return (&x)[i]; 
}     

const float& CQuat::operator[](uint i) const 
{ 
	return (&x)[i]; 
}

float CQuat::Scalar() 
{ 
	return w; 
}

// ---------------------------------------------
// --- CQuat address
CQuat::operator float*() 
{ 
	return(&x); 
}

// ---------------------------------------------
// --- Unary operators
CQuat CQuat::operator-() const 
{ 
	return(CQuat(-x,-y,-z,-w)); 
} 

CQuat CQuat::operator+() const 
{ 
	return *this; 
}

// ---------------------------------------------
// --- Assignment operators
CQuat& CQuat::operator-=(const CQuat& other)
{
	x-=other.x;
	y-=other.y;
	z-=other.z;
	w-=other.w;
	return(*this);
}

CQuat& CQuat::operator+=(const CQuat& other)
{
	x+=other.x;
	y+=other.y;
	z+=other.z;
	w+=other.w;
	return(*this);
}

CQuat& CQuat::operator*=(const CQuat& other)
{
	x*=other.x;
	y*=other.y;
	z*=other.z;
	w*=other.w;
	return(*this);
}

CQuat& CQuat::operator*=(float scalar)
{
	x*=scalar;
	y*=scalar;
	z*=scalar;
	w*=scalar;
	return(*this);

}

CQuat& CQuat::operator/=(float scalar)
{
	x/=scalar;
	y/=scalar;
	z/=scalar;
	w/=scalar;
	return(*this);
}

CQuat& CQuat::Set(float X, float Y, float Z, float W)
{ 
	x = X; 
	y = Y; 
	z = Z; 
	w = W; 
	return(*this); 
}

CQuat& CQuat::Set(double X, double Y, double Z, double W)
{ 
	x = (float)X; 
	y = (float)Y; 
	z = (float)Z; 
	w = (float)W;
	return(*this); 
}

// ---------------------------------------------
// --- Comparison
bool CQuat::operator==(const CQuat& a) const
{
	if (x==a.x && y==a.y && z==a.z && w==a.w)
	{
		return(true);
	}
	return(false);
}

bool CQuat::Equals(const CQuat& a, float epsilon = 1E-6f) const
{
	if (	(x-epsilon)<a.x && a.x<(x+epsilon) &&
			(y-epsilon)<a.y && a.y<(y+epsilon) &&
			(z-epsilon)<a.z && a.z<(z+epsilon) &&
			(w-epsilon)<a.w && a.w<(w+epsilon) )
	{
		return(true);
	}
	return(false);
}

// ---------------------------------------------
// --- Misc stuff
void CQuat::Identity() 
{ 
	x = y = z = (float)0.0; 
	w = (float) 1.0; 
}

bool CQuat::IsIdentity() const
{
	if (x==0 && y==0 && z==0 && w==1.0f)
	{
		return(1);
	}
	return(0);
}

// ---------------------------------------------
// --- Binary operators
CQuat CQuat::operator-(const CQuat& other) const
{
	return( CQuat(x-other.x,y-other.y,z-other.z,w-other.w) );
}

CQuat CQuat::operator+(const CQuat& other) const
{
	return( CQuat(x+other.x,y+other.y,z+other.z,w+other.w) );
}

CQuat CQuat::operator*(const CQuat& other) const
{
	return( CQuat(	(x*other.w)+(w*other.x)-(z*other.y)+(y * other.z),
					(y*other.w)+(z*other.x)+(w*other.y)-(x * other.z),
					(z*other.w)-(y*other.x)+(x*other.y)+(w * other.z),
					(w*other.w)-(x*other.x)-(y*other.y)-(z*other.z) )
			);
}

} // NLMISC
