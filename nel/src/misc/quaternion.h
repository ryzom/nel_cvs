/* <quaternion.h>
 *
 * Copyright (C) 2000 Nevrax. All rights reserved. 
 * 
 * You may distribute under the terms of the GNU General Public.
 */
 
/*
 * $Id: quaternion.h,v 1.1 2000/09/20 15:55:32 viau Exp $
 * 
 * <Quaternion class header> 
 */

// <File content>

/* End of <quaternion.h> */
#ifndef _QUAT_H 
#define _QUAT_H 

// ---------------------------------------------

class Quat 
{
	public:
		float x,y,z,w;
	// --- Constructors
	Quat() : x(0.0f),y(0.0f),z(0.0f),w(1.0f) {}
	Quat(float X, float Y, float Z, float W);
	Quat(double X, double Y, double Z, double W);
	Quat(const Quat& a);
	Quat(float af[4]);
	// --- Accessors
	float& operator[](uint i);
	const float& operator[](uint i) const;
	float Scalar();
	// --- Quat address
	operator float*();
	// --- Unary operators
	Quat operator-() const;
	Quat operator+() const; 
    
	// --- Assignment operators
	Quat& operator-=(const Quat&);
	Quat& operator+=(const Quat&);
	Quat& operator*=(const Quat&);
	Quat& operator*=(float);
	Quat& operator/=(float);
	Quat& Set(float X, float Y, float Z, float W);
	Quat& Set(double X, double Y, double Z, double W);

	// --- Comparison
	bool operator==(const Quat& a) const;
	bool Equals(const Quat& a, float epsilon) const;

	// --- Misc stuff
	void Identity();
	bool IsIdentity() const;

	// --- Binary operators
	Quat operator-(const Quat&) const; 
	Quat operator+(const Quat&) const; 
	Quat operator*(const Quat&) const; 
};

// ---------------------------------------------

#endif _QUAT_H 

