/** \file quaternion.cpp
 * 
 *
 * $Id: quaternion.cpp,v 1.3 2000/11/09 17:00:06 viau Exp $
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

#include "nel/misc/quaternion.h"

// ---------------------------------------------
// --- Constructors
Quat::Quat(float X, float Y, float Z, float W)
{ 
	x = X; 
	y = Y; 
	z = Z; 
	w = W; 
}

Quat::Quat(double X, double Y, double Z, double W)  
{ 
	x = (float)X; 
	y = (float)Y; 
	z = (float)Z; 
	w = (float)W; 
}

Quat::Quat(const Quat& a) 
{ 
	x = a.x; 
	y = a.y; 
	z = a.z; 
	w = a.w; 
} 

Quat::Quat(float af[4]) 
{ 
	x = af[0]; 
	y = af[1]; 
	z = af[2]; 
	w = af[3]; 
}

// ---------------------------------------------
// --- Accessors
float& Quat::operator[](uint i) 
{ 
	return (&x)[i]; 
}     

const float& Quat::operator[](uint i) const 
{ 
	return (&x)[i]; 
}

float Quat::Scalar() 
{ 
	return w; 
}

// ---------------------------------------------
// --- Quat address
Quat::operator float*() 
{ 
	return(&x); 
}

// ---------------------------------------------
// --- Unary operators
Quat Quat::operator-() const 
{ 
	return(Quat(-x,-y,-z,-w)); 
} 

Quat Quat::operator+() const 
{ 
	return *this; 
}

// ---------------------------------------------
// --- Assignment operators
Quat& Quat::operator-=(const Quat& other)
{
	x-=other.x;
	y-=other.y;
	z-=other.z;
	w-=other.w;
	return(*this);
}

Quat& Quat::operator+=(const Quat& other)
{
	x+=other.x;
	y+=other.y;
	z+=other.z;
	w+=other.w;
	return(*this);
}

Quat& Quat::operator*=(const Quat& other)
{
	x*=other.x;
	y*=other.y;
	z*=other.z;
	w*=other.w;
	return(*this);
}

Quat& Quat::operator*=(float scalar)
{
	x*=scalar;
	y*=scalar;
	z*=scalar;
	w*=scalar;
	return(*this);

}

Quat& Quat::operator/=(float scalar)
{
	x/=scalar;
	y/=scalar;
	z/=scalar;
	w/=scalar;
	return(*this);
}

Quat& Quat::Set(float X, float Y, float Z, float W)
{ 
	x = X; 
	y = Y; 
	z = Z; 
	w = W; 
	return(*this); 
}

Quat& Quat::Set(double X, double Y, double Z, double W)
{ 
	x = (float)X; 
	y = (float)Y; 
	z = (float)Z; 
	w = (float)W;
	return(*this); 
}

// ---------------------------------------------
// --- Comparison
bool Quat::operator==(const Quat& a) const
{
	if (x==a.x && y==a.y && z==a.z && w==a.w)
	{
		return(true);
	}
	return(false);
}

bool Quat::Equals(const Quat& a, float epsilon = 1E-6f) const
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
void Quat::Identity() 
{ 
	x = y = z = (float)0.0; 
	w = (float) 1.0; 
}

bool Quat::IsIdentity() const
{
	if (x==0 && y==0 && z==0 && w==1.0f)
	{
		return(1);
	}
	return(0);
}

// ---------------------------------------------
// --- Binary operators
Quat Quat::operator-(const Quat& other) const
{
	return( Quat(x-other.x,y-other.y,z-other.z,w-other.w) );
}

Quat Quat::operator+(const Quat& other) const
{
	return( Quat(x+other.x,y+other.y,z+other.z,w+other.w) );
}

Quat Quat::operator*(const Quat& other) const
{
	return( Quat(	(x*other.w)+(w*other.x)-(z*other.y)+(y * other.z),
					(y*other.w)+(z*other.x)+(w*other.y)-(x * other.z),
					(z*other.w)-(y*other.x)+(x*other.y)+(w * other.z),
					(w*other.w)-(x*other.x)-(y*other.y)-(z*other.z) )
			);
}

// ---------------------------------------------


