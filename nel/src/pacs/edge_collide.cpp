/** \file edge_collide.cpp
 * Collisions against edge in 2D.
 *
 * $Id: edge_collide.cpp,v 1.3 2001/05/17 17:00:36 berenguier Exp $
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

#include "nel/pacs/edge_collide.h"
#include "nel/misc/common.h"
#include <algorithm>

using namespace NLMISC;
using namespace std;



namespace NLPACS
{


// ***************************************************************************
void		CEdgeCollide::make(const CVector2f &p0, const CVector2f &p1)
{
	P0= p0;
	P1= p1;
	// translation axis of the edge.
	Dir= P1-P0;
	Dir.normalize();
	A0= P0*Dir;
	A1= P1*Dir;
	// line equation.
	Norm.x=  Dir.y;
	Norm.y= -Dir.x;
	C= - P0*Norm;
}


// ***************************************************************************
float		CEdgeCollide::testPoint(const CVector2f &start, const CVector2f &delta, float borderEpsilon)
{
	// distance from point to line.
	float	dist= start*Norm + C;
	// projection of speed on normal.
	float	speed= delta*Norm;

	// test if the movement is against the line or not.
	bool	sensPos= dist>0;
	bool	sensSpeed= speed>0;
	// if signs are equals, same side of the line.
	if(sensPos==sensSpeed)
		return 1;

	// Does the point intersect the line?
	dist= (float)fabs(dist);
	speed= (float)fabs(speed);
	if( dist > speed )
		return 1;

	// Yes, at what time.
	float	t= dist/speed;

	// Compute the collision position.
	CVector2f	proj= start + delta*t;
	// compute projection on edge.
	float		aProj= proj*Dir;

	// if not on the interval of the edge.
	if( aProj<A0-borderEpsilon || aProj>A1+borderEpsilon)
		return 1;
	else
	{
		// else return time of collision.
		return t;
	}
}


// ***************************************************************************
static	inline float		testCirclePoint(const CVector2f &start, const CVector2f &delta, float radius, const CVector2f &point)
{
	// factors of the qaudratic: at² + bt + c=0
	float		a,b,c;
	float		dta;
	float		r0, r1, res;
	CVector2f	relC, relV;

	// compute quadratic..
	relC= start-point;
	relV= delta;
	a= relV.x*relV.x + relV.y*relV.y;		// a>=0.
	b= 2* (relC.x*relV.x + relC.y*relV.y);
	c= relC.x*relC.x + relC.y*relC.y - radius*radius;
	// compute delta of the quadratic.
	dta= b*b - 4*a*c;	// b²-4ac
	if(dta>=0)
	{
		dta= (float)sqrt(dta);
		r0= (-b -dta)/(2*a);
		r1= (-b +dta)/(2*a);
		// since a>0, r0<=r1.
		if(r0>r1)
			swap(r0,r1);
		// if r1 is negative, then we are out and go away from this point. OK.
		if(r1<=0)
			res= 1;
		// if r0 is positive, then we may collide this point.
		else if(r0>=0)
			res= min(1.f, r0);
		else	// r0<0 && r1>0. the point is already in the sphere!!
		{
			// we allow the movement only if we go away from this point.
			// this is true if the derivative at t=0 is >=0 (because a>0).
			if(b>=0)
				res= min(1.f, r1);	// go out.
			else
				res=0;
		}
	}
	else
	{
		// never hit this point along this movement.
		res= 1;
	}

	return res;
}


// ***************************************************************************
float		CEdgeCollide::testCircle(const CVector2f &start, const CVector2f &delta, float radius, CVector2f &normal)
{
	// distance from point to line.
	float	dist= start*Norm + C;
	// projection of speed on normal.
	float	speed= delta*Norm;

	// test if the movement is against the line or not.
	bool	sensPos= dist>0;
	bool	sensSpeed= speed>0;
	// if signs are equals, same side of the line, so we allow the circle to leave the line.
	if(sensPos==sensSpeed)
		return 1;

	// Does the point intersect the line?
	dist= (float)fabs(dist) - radius;
	speed= (float)fabs(speed);
	if( dist > speed )
		return 1;

	// if not already in collision with the line, test when it collides.
	// ===============================
	if(dist>=0)
	{
		// collide the line, at what time.
		float	t= dist/speed;


		// compute the collision position of the Circle on the edge.
		// this gives the center of the sphere at the collision point.
		CVector2f	proj= start + delta*t;
		// must add radius vector.
		proj+= Norm * (sensSpeed?radius:-radius);
		// compute projection on edge.
		float		aProj= proj*Dir;

		// if on the interval of the edge.
		if( aProj>=A0 && aProj<=A1)
		{
			// collision occurs on interior of the edge. the normal to return is +- Norm.
			if(sensPos)	// if algebric distance of start position was >0.
				normal= Norm;
			else
				normal= -Norm;

			// return time of collision.
			return t;
		}
	}

	// In this case, the Circle do not hit the edge on the interior, but may hit on borders.
	// ===============================
	// Then, we must compute collision sphere-points.
	float		tmin, ttmp;
	// first point.
	tmin= testCirclePoint(start, delta, radius, P0);
	// second point.
	ttmp= testCirclePoint(start, delta, radius, P1);
	tmin= min(tmin, ttmp);

	// if collision occurs, compute normal of collision.
	if(tmin<1)
	{
		// to which point we collide?
		CVector2f	colPoint= tmin==ttmp? P1 : P0;
		// compute position of the entity at collision.
		CVector2f	colPos= start + delta*tmin;

		// and so we have this normal (the perpendicular of the tangent at this point).
		normal= colPos - colPoint;
		normal.normalize();
	}

	return tmin;
}



} // NLPACS
