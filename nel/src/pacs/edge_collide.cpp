/** \file edge_collide.cpp
 * Collisions against edge in 2D.
 *
 * $Id: edge_collide.cpp,v 1.6 2001/05/22 07:36:02 berenguier Exp $
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


static	const	float	EdgeCollideEpsilon= 1e-5f;


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
float		CEdgeCollide::testPointMove(const CVector2f &start, const CVector2f &delta, float borderEpsilon)
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
		{
			res= 1;
		}
		// if r0 is positive, then we may collide this point.
		else if(r0>=0)
		{
			res= min(1.f, r0);
		}
		else	// r0<0 && r1>0. the point is already in the sphere!!
		{
			//nldebug("COL: Point problem: %.2f, %.2f", r0, r1);
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
float		CEdgeCollide::testCircleMove(const CVector2f &start, const CVector2f &delta, float radius, CVector2f &normal)
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
	// else, must test if circle collide segment at t=0. if yes, return 0.
	// ===============================
	else
	{
		// There is just need to test if projection of circle's center onto the line hit the segment.
		// This is not a good test to know if a circle intersect a segment, but other cases are
		// managed with test of endPoints of the segment after.
		float		aProj= start*Dir;

		// if on the interval of the edge.
		if( aProj>=A0 && aProj<=A1)
		{
			// hit the interior of the edge, and sensPos!=sensSpeed. So must stop now!!
			return 0;
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



// ***************************************************************************
float		CEdgeCollide::testEdgeMove(const CVector2f &q0, const CVector2f &q1, const CVector2f &delta)
{
	float	a,b,cv,cc,  d,e,f;
	CVector2f	tmp;

	// compute D1 line equation of q0q1. bx - ay + c(t)=0, where c is function of time [0,1].
	// ===========================
	tmp= q1 - q0;		// NB: along time, the direction doesn't change.
	// Divide by norm()², so that  a projection on this edge is true if the proj is in interval [0,1].
	tmp/= tmp.sqrnorm();
	a= tmp.x;
	b= tmp.y;
	// c= - q0(t)*CVector2f(b,-a).  but since q0(t) is a function of time t (q0+delta*t), compute cv, and cc.
	// so c= cv*t + cc.
	cv= - delta*CVector2f(b,-a);
	cc= - q0*CVector2f(b,-a);

	// compute D2 line equation of P0P1. ex - dy + f=0.
	// ===========================
	tmp= P1 - P0;
	// Divide by norm()², so that  a projection on this edge is true if the proj is in interval [0,1].
	tmp/= tmp.sqrnorm();
	d= tmp.x;
	e= tmp.y;
	f= - P0*CVector2f(e,-d);


	// Solve system.
	// ===========================
	/*
		Compute the intersection I of 2 lines across time.
		We have the system:
			bx - ay + c(t)=0
			ex - dy + f=0

		which solve for:
			det= ae-bd	(0 <=> // lines)
			x(t)= (d*c(t) - fa) / det
			y(t)= (e*c(t) - fb) / det
	*/

	// determinant of matrix2x2.
	float	det= a*e - b*d;
	// if to near of 0. (take delta for reference of test).
	if(det==0 || fabs(det)<delta.norm()*EdgeCollideEpsilon)
		return 1;

	// intersection I(t)= pInt + vInt*t.
	CVector2f		pInt, vInt;
	pInt.x= ( d*cc - f*a ) / det;
	pInt.y= ( e*cc - f*b ) / det;
	vInt.x= ( d*cv ) / det;
	vInt.y= ( e*cv ) / det;


	// Project Intersection.
	// ===========================
	/*
		Now, we project x,y onto each line D1 and D2, which gives  u(t) and v(t), each one giving the parameter of 
		the parametric line function. When it is in [0,1], we are on the edge.

		u(t)= (I(t)-q0(t)) * CVector2f(a,b)	= uc + uv*t
		v(t)= (I(t)-P0) * CVector2f(d,e)	= vc + vv*t
	*/
	float	uc, uv;
	float	vc, vv;
	// NB: q0(t)= q0+delta*t
	uc= (pInt-q0) * CVector2f(a,b);
	uv= (vInt-delta) * CVector2f(a,b);
	vc= (pInt-P0) * CVector2f(d,e);
	vv= (vInt) * CVector2f(d,e);


	// Compute intervals.
	// ===========================
	/*
		Now, for each edge, compute time interval where parameter is in [0,1]. If intervals overlap, there is a collision.
		Then clamp this collision time with [0,1].
	*/
	float	tu0, tu1, tv0, tv1;

	// compute time interval for u(t).
	if(uv==0 || fabs(uv)<EdgeCollideEpsilon)
	{
		// The intersection does not move along D1. Always projected on u(t)=uc. so if in [0,1], OK, else never collide.
		if(uc<0 || uc>1)
			return 1;
		// else suppose "always valid".
		tu0= 0;
		tu1= 1;
	}
	else
	{
		tu0= (0-uc)/uv;	// t for u(t)=0
		tu1= (1-uc)/uv;	// t for u(t)=1
	}

	// compute time interval for v(t).
	if(vv==0 || fabs(vv)<EdgeCollideEpsilon)
	{
		// The intersection does not move along D2. Always projected on v(t)=vc. so if in [0,1], OK, else never collide.
		if(vc<0 || vc>1)
			return 1;
		// else suppose "always valid".
		tv0= 0;
		tv1= 1;
	}
	else
	{
		tv0= (0-vc)/vv;	// t for v(t)=0
		tv1= (1-vc)/vv;	// t for v(t)=1
	}


	// clip intervals.
	// ===========================
	// order time interval.
	if(tu0>tu1)
		swap(tu0, tu1);		// now, [tu0, tu1] represent the time interval where line D2 hit the edge D1.
	if(tv0>tv1)
		swap(tv0, tv1);		// now, [tv0, tv1] represent the time interval where line D1 hit the edge D2.

	// if intervals do not overlap, no collision.
	if(tu0>tv1 || tv0>tu1)
		return 1;
	else
	{
		// compute intersection of intervals.
		float	tInt0= max(tu0, tv0);
		float	tInt1= min(tu1, tv1);

		// if this interval do not overlap with [0,1], no collision.
		if(tInt0>1 || 0>tInt1)
			return 1;
		else
			// return time of collision of the 2 edges.
			return max(0.0f, tInt0);
	}
	
}


// ***************************************************************************
float		CEdgeCollide::testBBoxMove(const CVector2f &start, const CVector2f &delta, const CVector2f bbox[4], CVector2f &normal)
{
	// distance from center to line.
	float	dist= start*Norm + C;
	// projection of speed on normal.
	float	speed= delta*Norm;

	// test if the movement is against the line or not.
	bool	sensPos= dist>0;
	bool	sensSpeed= speed>0;
	// if signs are equals, same side of the line, so we allow the circle to leave the line.
	if(sensPos==sensSpeed)
		return 1;

	// Else, do 4 test edge/edge, and return Tmin.
	float	tMin=1;
	for(sint i=0;i<4;i++)
	{
		float	t;
		t= testEdgeMove(bbox[i], bbox[(i+1)&3], delta);
		tMin= min(t, tMin);
	}

	if(tMin<1)
	{
		// always assume collision occurs on interior of the edge. the normal to return is +- Norm.
		if(sensPos)	// if algebric distance of start position was >0.
			normal= Norm;
		else
			normal= -Norm;

		// return time of collision.
		return tMin;
	}
	else
		return 1;

}



} // NLPACS
