/** \file triangle.cpp
 * <File description>
 *
 * $Id: triangle.cpp,v 1.1 2001/02/28 14:39:04 berenguier Exp $
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

#include "nel/misc/triangle.h"
#include "nel/misc/plane.h"


namespace NLMISC 
{

#define EPSILON 0.0001f

// ***************************************************************************
bool CTriangle::intersect (const CVector& p0, const CVector& p1, CVector& hit, const CPlane& plane) const
{
	// Normale
	CVector normal=plane.getNormal();

	// Not clipped by the plane ?
	if (plane*p0<-EPSILON)
	{
		if (plane*p1<-EPSILON)
			return false;
	}
	else
	{
		if (plane*p1>EPSILON)
			return false;
	}

	// Point on the plane
	hit=plane.intersect (p0, p1);

	// Check the point...
	float f=((V0-hit)^(V1-hit))*normal;
	bool negative=f<EPSILON;
	bool positive=f>-EPSILON;

	float f2=((V1-hit)^(V2-hit))*normal;
	if ((!positive)&&(f2>EPSILON))
		return false;
	if ((!negative)&&(f2<-EPSILON))
		return false;

	f2=((V2-hit)^(V0-hit))*normal;
	if ((!positive)&&(f2>EPSILON))
		return false;
	if ((!negative)&&(f2<-EPSILON))
		return false;
	return true;
}


} // NLMISC

