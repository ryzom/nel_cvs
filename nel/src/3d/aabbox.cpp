/** \file aabbox.cpp
 * <File description>
 *
 * $Id: aabbox.cpp,v 1.3 2000/12/11 15:52:33 berenguier Exp $
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

#include "nel/3d/aabbox.h"


namespace NL3D {


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
void			CAABBox::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);
	f.serial(Center);
	f.serial(HalfSize);
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
void	CAABBox::extend(const CVector &v)
{
	CVector		bmin= getMin(), bmax= getMax();

	bmin.minof(bmin, v);
	bmax.maxof(bmax, v);
	setMinMax(bmin, bmax);
}


// ***************************************************************************
void			CAABBoxExt::serial(NLMISC::IStream &f)
{
	CAABBox::serial(f);
	if(f.isReading())
		updateRadius();
}


} // NL3D
