/* <quaternion.cpp>
 *
 * Copyright (C) 2000 Nevrax. All rights reserved. 
 * 
 * You may distribute under the terms of the GNU General Public.
 */
 
/*
 * $Id: quaternion.cpp,v 1.1 2000/09/20 15:55:32 viau Exp $
 * 
 * <Quaternion class bodies> 
 */

// <File content>

/* End of <quaternion.cpp> */

#include "quaternion.h"

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
float& Quat::operator[](uint8 i) 
{ 
	return (&x)[i]; 
}     

const float& Quat::operator[](uint8 i) const 
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


