/** \file quaternion.h
 * CQuat class
 *
 * $Id: quat.h,v 1.1 2000/11/10 11:19:21 lecroart Exp $
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

#ifndef NL_QUAT_H 
#define NL_QUAT_H 

#include "nel/misc/types_nl.h"

namespace	NLMISC
{

class CQuat 
{
	public:
		float x,y,z,w;
	// --- Constructors
	CQuat() : x(0.0f),y(0.0f),z(0.0f),w(1.0f) {}
	CQuat(float X, float Y, float Z, float W);
	CQuat(double X, double Y, double Z, double W);
	CQuat(const CQuat& a);
	CQuat(float af[4]);
	// --- Accessors
	float& operator[](uint i);
	const float& operator[](uint i) const;
	float Scalar();
	// --- CQuat address
	operator float*();
	// --- Unary operators
	CQuat operator-() const;
	CQuat operator+() const; 
    
	// --- Assignment operators
	CQuat& operator-=(const CQuat&);
	CQuat& operator+=(const CQuat&);
	CQuat& operator*=(const CQuat&);
	CQuat& operator*=(float);
	CQuat& operator/=(float);
	CQuat& Set(float X, float Y, float Z, float W);
	CQuat& Set(double X, double Y, double Z, double W);

	// --- Comparison
	bool operator==(const CQuat& a) const;
	bool Equals(const CQuat& a, float epsilon) const;

	// --- Misc stuff
	void Identity();
	bool IsIdentity() const;

	// --- Binary operators
	CQuat operator-(const CQuat&) const; 
	CQuat operator+(const CQuat&) const; 
	CQuat operator*(const CQuat&) const; 
};

} // NLMISC

#endif // NL_QUAT_H 

