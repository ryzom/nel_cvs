/** \file triangle.cpp
 * <File description>
 *
 * $Id: triangle.cpp,v 1.2 2001/04/04 10:10:59 legros Exp $
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
	CVector	normal = plane.getNormal();

	float	np1 = normal*p1;
	float	lambda = (plane.d+np1)/(np1-normal*p0);

	// Checks the intersection belongs to the segment
	if (lambda < -EPSILON || lambda > 1.0f+EPSILON)
		return false;

	// The intersection on the plane
	hit = p0*lambda+p1*(1.0f-lambda);

	float	d0 = ((V1-V0)^normal)*(hit-V0);
	float	d1 = ((V2-V1)^normal)*(hit-V1);
	float	d2 = ((V0-V2)^normal)*(hit-V2);

	return (d0 < +EPSILON && d1 < +EPSILON && d2 < +EPSILON) ||
		   (d0 > -EPSILON && d1 > -EPSILON && d2 > -EPSILON);
}


} // NLMISC

