/** \file polygon.h
 * <File description>
 *
 * $Id: polygon.h,v 1.1 2000/12/13 10:25:22 berenguier Exp $
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

#ifndef NL_POLYGON_H
#define NL_POLYGON_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include <vector>


namespace NL3D 
{

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


// ***************************************************************************
/**
 * A polygon, with an unlimited size of vertices.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPolygon
{
public:
	std::vector<CVector>	Vertices;
	

public:

	/// Constructor
	CPolygon() {}
	/// Constructor. Init with a triangle.
	CPolygon(const CVector &a, const CVector &b, const CVector &c);

	sint			getNumVertices() const {return Vertices.size();}

	/// Clip a polygon with a set of planes. Cohen-sutherland... clipPolygonBack() is used on planes.
	void			clip(const CPlane *planes, uint nPlanes);
	/// Clip a polygon with a set of planes. Cohen-sutherland clipping... clipPolygonBack() is used on planes.
	void			clip(const std::vector<CPlane> &planes);

};


} // NL3D


#endif // NL_POLYGON_H

/* End of polygon.h */
