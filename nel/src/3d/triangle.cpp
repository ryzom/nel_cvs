/** \file triangle.cpp
 * <File description>
 *
 * $Id: triangle.cpp,v 1.2 2001/01/23 14:31:41 corvazier Exp $
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

#include "nel/3d/triangle.h"
#include "nel/misc/plane.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************
bool CTriangle::intersect (const CVector& p0, const CVector& p1, CVector& hit, const CPlane& plane) const
{
	// Normale
	CVector normal=plane.getNormal();

	// Not clipped by the plane ?
	if (plane*p0<0.f)
	{
		if (plane*p1<0.f)
			return false;
	}
	else
	{
		if (plane*p1>=0.f)
			return false;
	}

	// Point on the plane
	hit=plane.intersect (p0, p1);

	// Check
	float fDist=plane*hit;

	// Check the point...
	bool positive=(((V0-hit)^(V1-hit))*normal>0.f);

	if ((((V1-hit)^(V2-hit))*normal>0.f)!=positive)
		return false;
	return ((((V2-hit)^(V0-hit))*normal>0.f)==positive);
}


} // NL3D
