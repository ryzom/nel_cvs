/** \file plane_basis_maker.h
 * <File description>
 *
 * $Id: ps_plane_basis_maker.cpp,v 1.5 2001/09/07 12:01:08 vizerie Exp $
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


CPlaneBasis CPSPlaneBasisGradient::DefaultPlaneBasisTab[] = { CPlaneBasis(NLMISC::CVector::I), CPlaneBasis(NLMISC::CVector::J) } ;

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


/////////////////////////////////////////////
// CSpinnerFunctor implementation		   //
/////////////////////////////////////////////



CSpinnerFunctor::CSpinnerFunctor() : _NbSamples(16), _Axis(NLMISC::CVector::K)
{
	updateSamples();
}

void CSpinnerFunctor::setAxis(const NLMISC::CVector &axis)
{
	_Axis = axis;
	updateSamples();
}
const void CSpinnerFunctor::setNumSamples(uint32 nbSamples)
{
	nlassert(nbSamples > 0);
	_NbSamples = nbSamples;
	updateSamples();
}

const uint32 CSpinnerFunctor::getNumSamples(void) const
{
	return _NbSamples;
}

void CSpinnerFunctor::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	f.serial(_Axis, _NbSamples);
	if (f.isReading()) updateSamples();
}

void CSpinnerFunctor::updateSamples(void)
{
	// compute step between each angle
	const float angInc = (float) (NLMISC::Pi / _NbSamples);
	_PBTab.resize(_NbSamples);
	NLMISC::CMatrix mat;
	// compute each sample
	for (uint32 k = 0; k < _NbSamples ; ++k)
	{		
		mat.setRot(NLMISC::CQuat(_Axis, k * angInc));
		_PBTab[k] = CPlaneBasis(mat.getI(), mat.getJ());
	}
}


} // NL3D


