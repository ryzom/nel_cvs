/** \file triangle.h
 * <File description>
 *
 * $Id: triangle.h,v 1.1 2000/12/13 10:25:22 berenguier Exp $
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

#ifndef NL_TRIANGLE_H
#define NL_TRIANGLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NL3D 
{


using NLMISC::CVector;


// ***************************************************************************
/**
 * A simple triangles of 3 points.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTriangle
{
public:
	CVector		V0,V1,V2;

public:
	/// Constructor
	CTriangle() {}
	/// Constructor
	CTriangle(const CVector &a, const CVector &b, const CVector &c) : V0(a), V1(b), V2(c) {}

};


} // NL3D


#endif // NL_TRIANGLE_H

/* End of triangle.h */
