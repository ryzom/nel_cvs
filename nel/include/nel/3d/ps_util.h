/** \file ps_util.h
 * <File description>
 *
 * $Id: ps_util.h,v 1.1 2001/04/25 08:43:37 vizerie Exp $
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

#ifndef NL_PS_UTIL_H
#define NL_PS_UTIL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/vector.h"




namespace NLMISC
{
	class CMatrix ;
	class CVector ;
} ;

namespace NL3D 
{


	class CFontGenerator ;
	class CFontManager ;


	using NLMISC::CMatrix ;
	using NLMISC::CVector ;
/**
 * This struct contains utility functions used by the particle system.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
struct CPSUtil
{
	/// this draw a bounding box		

	static void displayBBox(const NLMISC::CAABBox &box) ;
	 

	/// enlarge a bounding box by the specified radius	 
	inline static void addRadiusToAABBox(NLMISC::CAABBox &box, float radius) ;
	 
	/// display a basis using the given matrix

	static void displayBasis(const NLMISC::CMatrix &m, float size, CFontGenerator &fg, CFontManager &fm) ;


	/// display a string at the given position
	static void print(const std::string &text, CFontGenerator &fg, CFontManager &fm, const NLMISC::CVector &pos, float size) ;


	
	/**
	* Compute the union of 2 aabboxes, that is the  aabbox that contains the 2.
	* Should end up in NLMISC
	*/


	static NLMISC::CAABBox computeAABBoxUnion(const NLMISC::CAABBox &b1, const NLMISC::CAABBox &b2) ;


	/** Apply a matrix on an aabbox
	 *  \return an aabbox, bigger or equal to parameter, after the matrix multiplication
	 */

	static NLMISC::CAABBox transformAABBox(const NLMISC::CMatrix &mat, const NLMISC::CAABBox &box) ;

	/** build a basis from a vector using Schmidt orthogonalization method
	 *  \param v : K axis if the resulting basis
	 */
	static CMatrix buildSchmidtBasis(const CVector &v) ;	 
};

void CPSUtil::addRadiusToAABBox(NLMISC::CAABBox &box, float radius)
{
	box.setHalfSize(box.getHalfSize() + NLMISC::CVector(radius, radius, radius) ) ;
}



} // NL3D


#endif // NL_PS_UTIL_H

/* End of ps_util.h */
