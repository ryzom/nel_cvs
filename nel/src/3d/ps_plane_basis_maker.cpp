/** \file plane_basis_maker.h
 * <File description>
 *
 * $Id: ps_plane_basis_maker.cpp,v 1.3 2001/07/04 12:29:08 vizerie Exp $
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


#include "3d/ps_plane_basis_maker.h"

namespace NL3D {


CPlaneBasis CPSPlaneBasisGradient::DefaultPlaneBasisTab[] = { CPlaneBasis(CVector::I), CPlaneBasis(CVector::J) } ;

/////////////////////////////////////////////
// CPSPlaneBasisFollowSpeed implementation //
/////////////////////////////////////////////


CPlaneBasis CPSPlaneBasisFollowSpeed::get(CPSLocated *loc, uint32 index)
{
   return (CPlaneBasis(loc->getSpeed()[index])) ;
}


void *CPSPlaneBasisFollowSpeed::make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, bool enableNoCopy /* = false*/) const
{
	nlassert(numAttrib) ;
	TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
									, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib ;
	uint8 *ptDat  = (uint8 *) tab ; 
	do
	{
		*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt) ;
		++ speedIt ;
		ptDat += stride ;
	}
	while (speedIt != endSpeedIt) ;
	return tab ;
}


void CPSPlaneBasisFollowSpeed::make4(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const
{
	nlassert(numAttrib) ;
	TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
									, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib ;	
	uint8 *ptDat  = (uint8 *) tab  ;
	do
	{
		*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt) ;
		*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat  ;
		ptDat += stride ;
		*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat  ;
		ptDat += stride ;
		*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat  ;
		ptDat += stride << 1;
		++ speedIt ;		
	}
	while (speedIt != endSpeedIt) ;
}		


void CPSPlaneBasisFollowSpeed::makeN(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const
{
	nlassert(numAttrib) ;
	nlassert(nbReplicate > 1) ; 
	TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
									, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib ;	
	uint8 *ptDat  = (uint8 *) tab  ;
	uint k ; 
	do
	{
		*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt) ;
		
		k = nbReplicate - 1 ;
		ptDat += stride ;

		do
		{
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat  ;
			ptDat += stride ;
		}
		while (--k) ;
	
		++ speedIt ;		
	}
	while (speedIt != endSpeedIt) ;

}

} // NL3D


