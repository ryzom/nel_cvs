/** \file bsphere.cpp
 * <File description>
 *
 * $Id: bsphere.cpp,v 1.3 2000/11/10 09:58:04 berenguier Exp $
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

#include "nel/3d/bsphere.h"
#include "nel/misc/common.h"
using namespace	NLMISC;


namespace NL3D {


bool	CBSphere::clipFront(const CPlane &p) const
{
	// don't assume normalized planes.
	float	norm= p.getNormal().norm();
	// This is faster than normalize p.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d<-Radius*norm)
		return false;

	return true;
}

bool	CBSphere::clipFrontUnitPlane(const CPlane &p) const
{
	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d<-Radius)
		return false;

	return true;
}


bool	CBSphere::clipBack(const CPlane &p) const
{
	// don't assume normalized planes.
	float	norm= p.getNormal().norm();
	// This is faster than normalize p.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d>Radius*norm)
		return false;

	return true;
}

bool	CBSphere::clipBackUnitPlane(const CPlane &p) const
{
	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d>Radius)
		return false;

	return true;
}


bool	CBSphere::include(const CVector &p) const
{
	float	r2= (p-Center).sqrnorm();
	return (r2<=sqr(Radius));
}

bool	CBSphere::intersect(const CBSphere &s) const
{
	float	r2= (s.Center-Center).sqrnorm();

	if(r2<=sqr(Radius))
		return true;
	if(r2<=sqr(s.Radius))
		return true;
	// else std method.
	return sqrt(r2)<=(Radius+s.Radius);

}


} // NL3D
