/* plane.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: plane.cpp,v 1.1 2000/09/15 11:39:18 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/plane.h"


namespace	NLMISC
{


//============================================================
void	CPlane::make(const CVector &normal, const CVector &p)
{
	CVector	v= normal.normed();
	a= v.x;
	b= v.y;
	c= v.z;
	d=-(v*p);		// d=- (ax+by+cz).
}
void	CPlane::make(const CVector &p0, const CVector &p1, const CVector &p2)
{
	CVector v;

	v=(p1-p0)^(p2-p1);
	make(v,p1);
}


//============================================================
bool	CPlane::clipSegmentBack(CVector &p0, CVector &p1) const
{
	float	d0,d1,decal;
	CVector	proj;

	d0= (*this)*p0;
	d1= (*this)*p1;
	if(d0<0 && d1<0)
		return true;
	if(d0>=0 && d1>=0)
		return false;
	// Clip line.
	decal= (0-d0) / (d1-d0);
	proj= p0+ (p1-p0)*decal;
	if(d0>=0)
		p0=proj;
	else
		p1=proj;
	return true;
}
bool	CPlane::clipSegmentFront(CVector &p0, CVector &p1) const
{
	float	d0,d1,decal;
	CVector	proj;

	d0= (*this)*p0;
	d1= (*this)*p1;
	if(d0>=0 && d1>=0)
		return true;
	if(d0<0 && d1<0)
		return false;
	// Clip line.
	decal= (0-d0) / (d1-d0);
	proj= p0+ (p1-p0)*decal;
	if(d0<0)
		p0=proj;
	else
		p1=proj;
	return true;
}


//============================================================
sint	CPlane::clipPolygonBack(CVector in[], CVector out[], sint nIn) const
{
	sint nOut=0,s,p,i;
	if(nIn<=2) return 0;

	s=nIn-1;

	for (i=0;i<nIn;i++)
	{
		p=i;
		if ( (*this)*in[p] < 0 )
		{
			if ( (*this)*in[s] > 0 ) 
				out[nOut++]= project(in[s],in[p]);
			out[nOut++]=in[p];
		}
		else
		{
			if ( (*this)*in[s] < 0 ) 
				out[nOut++]= project(in[s],in[p]);
		}
		s=p;
	}

	return nOut;
}
sint	CPlane::clipPolygonFront(CVector in[], CVector out[], sint nIn) const
{
	sint nOut=0,s,p,i;
	if(nIn<=2) return 0;

	s=nIn-1;

	for (i=0;i<nIn;i++)
	{
		p=i;
		if ( (*this)*in[p] > 0 )
		{
			if ( (*this)*in[s] < 0 ) 
				out[nOut++]= project(in[s],in[p]);
			out[nOut++]=in[p];
		}
		else
		{
			if ( (*this)*in[s] > 0 ) 
				out[nOut++]= project(in[s],in[p]);
		}
		s=p;
	}

	return nOut;
}



}