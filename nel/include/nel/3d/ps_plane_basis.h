/** \file ps_plane_basis.h
 * <File description>
 *
 * $Id: ps_plane_basis.h,v 1.1 2001/05/23 15:19:36 vizerie Exp $
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

#ifndef NL_PS_PLANE_BASIS_H
#define NL_PS_PLANE_BASIS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"
#include "nel/3d/ps_util.h"


namespace NL3D {

using NLMISC::CVector ;
using NLMISC::CMatrix ;




/** A basis for plane object, used by particle by face and shockwaves
 *  It's a like a 2x3 matrix, (with only the X and Y vector defined)
 */

struct CPlaneBasis
{	
	CVector X ;
	CVector Y ;


	// default ctor
	CPlaneBasis() {}

	
	// construct this basis by giving a normal to the plane that contains it
	CPlaneBasis(const CVector &normal)
	{
		CMatrix mat = CPSUtil::buildSchmidtBasis(normal) ;
		X = mat.getI() ;
		Y = mat.getJ() ;
	}

	/// compute the normal of the plane basis
	CVector getNormal(void) const
	{
		return X ^ Y ;
	}


	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(X, Y) ;
	}
} ;


// for map insertion

inline bool operator<(const CPlaneBasis &p1, const CPlaneBasis &p2)
{
	if (p1.X < p2.X) return true ;
	else if (p1.X == p2.X) return false ;
		else if (p1.Y < p2.Y) return true ;
			else if (p1.X == p2.Y) return false ;
				else return true ;
}






} // NL3D


#endif // NL_PS_PLANE_BASIS_H

/* End of ps_plane_basis.h */
