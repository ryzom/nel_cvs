/** \file quaternion.h
 * 
 *
 * $Id: quaternion.h,v 1.2 2000/11/09 13:25:12 viau Exp $
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

#ifndef _QUAT_H 
#define _QUAT_H 

#include "nel/misc/types_nl.h"

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

