/** \file aabbox.cpp
 * <File description>
 *
 * $Id: aabbox.cpp,v 1.2 2001/04/20 07:32:57 legros Exp $
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

#include "nel/misc/aabbox.h"
#include "nel/misc/polygon.h"


namespace NLMISC {


// ***************************************************************************
bool	CAABBox::clipFront(const CPlane &p) const
{
	CVector		hswap;

	// The bbox is front of the plane if only one of his vertex is in front.
	if(p*(Center + HalfSize) > 0)	return true;
	if(p*(Center - HalfSize) > 0)	return true;
	hswap.set(-HalfSize.x, HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) > 0)	return true;
	if(p*(Center - hswap) > 0)	return true;
	hswap.set(HalfSize.x, -HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) > 0)	return true;
	if(p*(Center - hswap) > 0)	return true;
	hswap.set(HalfSize.x, HalfSize.y, -HalfSize.z);
	if(p*(Center + hswap) > 0)	return true;
	if(p*(Center - hswap) > 0)	return true;

	return false;
}
// ***************************************************************************
bool	CAABBox::clipBack(const CPlane &p) const
{
	CVector		hswap;

	// The bbox is back of the plane if only one of his vertex is in back.
	if(p*(Center + HalfSize) < 0)	return true;
	if(p*(Center - HalfSize) < 0)	return true;
	hswap.set(-HalfSize.x, HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) < 0)	return true;
	if(p*(Center - hswap) < 0)	return true;
	hswap.set(HalfSize.x, -HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) < 0)	return true;
	if(p*(Center - hswap) < 0)	return true;
	hswap.set(HalfSize.x, HalfSize.y, -HalfSize.z);
	if(p*(Center + hswap) < 0)	return true;
	if(p*(Center - hswap) < 0)	return true;

	return false;
}


// ***************************************************************************
bool			CAABBox::include(const CVector &a) const
{
	if(Center.x+HalfSize.x<a.x)	return false;
	if(Center.x-HalfSize.x>a.x)	return false;
	if(Center.y+HalfSize.y<a.y)	return false;
	if(Center.y-HalfSize.y>a.y)	return false;
	if(Center.z+HalfSize.z<a.z)	return false;
	if(Center.z-HalfSize.z>a.z)	return false;
	return true;
}


// ***************************************************************************
bool			CAABBox::intersect(const CAABBox &box) const
{
	CVector	mina = getMin(), maxa = getMax(),
			minb = box.getMin(), maxb = box.getMax();

	return ! ( mina.x > maxb.x ||
			   mina.y > maxb.y ||
			   mina.z > maxb.z ||
			   minb.x > maxa.x ||
			   minb.y > maxa.y ||
			   minb.z > maxa.z);
}

// ***************************************************************************
bool			CAABBox::intersect(const CVector &a, const CVector &b, const CVector &c) const
{
	// Trivial test.
	if(include(a) || include(b) || include(c))
		return true;
	// Else, must test if the polygon intersect the pyamid.
	CPlane		planes[6];
	makePyramid(planes);
	CPolygon	poly(a,b,c);
	poly.clip(planes, 6);
	if(poly.getNumVertices()==0)
		return false;
	return true;
}


// ***************************************************************************
void			CAABBox::makePyramid(CPlane	planes[6]) const
{
	planes[0].make(CVector(-1,0,0), Center-HalfSize);
	planes[1].make(CVector(+1,0,0), Center+HalfSize);
	planes[2].make(CVector(0,-1,0), Center-HalfSize);
	planes[3].make(CVector(0,+1,0), Center+HalfSize);
	planes[4].make(CVector(0,0,-1), Center-HalfSize);
	planes[5].make(CVector(0,0,+1), Center+HalfSize);
}


// ***************************************************************************
void			CAABBox::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);
	f.serial(Center);
	f.serial(HalfSize);
}


// ***************************************************************************
void	CAABBox::extend(const CVector &v)
{
	CVector		bmin= getMin(), bmax= getMax();

	bmin.minof(bmin, v);
	bmax.maxof(bmax, v);
	setMinMax(bmin, bmax);
}


// ***************************************************************************
bool	CAABBoxExt::clipFront(const CPlane &p) const
{
	// Assume normalized planes.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d<-RadiusMax)
		return false;
	// if( SphereMin IN )	return true;
	if(d>-RadiusMin)
		return true;

	// else, standard clip box.
	return CAABBox::clipFront(p);
}


// ***************************************************************************
bool	CAABBoxExt::clipBack(const CPlane &p) const
{
	// Assume normalized planes.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d>RadiusMax)
		return false;
	// if( SphereMin IN )	return true;
	if(d<RadiusMin)
		return true;

	// else, standard clip box.
	return CAABBox::clipBack(p);
}


// ***************************************************************************
void			CAABBoxExt::serial(NLMISC::IStream &f)
{
	CAABBox::serial(f);
	if(f.isReading())
		updateRadius();
}


} // NLMISC
