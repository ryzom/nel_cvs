/** \file ps_plane_basis_maker.h
 * <File description>
 *
 * $Id: ps_plane_basis_maker.cpp,v 1.11 2002/08/07 08:37:40 vizerie Exp $
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

#include "std3d.h"


#include "3d/ps_plane_basis_maker.h"
#include "3d/ps_register_plane_basis_attribs.h"

namespace NL3D 
{


CPlaneBasis CPSPlaneBasisGradient::DefaultPlaneBasisTab[] = { CPlaneBasis(NLMISC::CVector::I), CPlaneBasis(NLMISC::CVector::J) };

/////////////////////////////////////////////
// CPSPlaneBasisFollowSpeed implementation //
/////////////////////////////////////////////

///============================================================================
CPlaneBasis CPSPlaneBasisFollowSpeed::get(CPSLocated *loc, uint32 index)
{
   return (CPlaneBasis(loc->getSpeed()[index]));
}

///============================================================================
void *CPSPlaneBasisFollowSpeed::make(CPSLocated *loc,
									 uint32 startIndex,
									 void *tab, uint32 stride,
									 uint32 numAttrib,
									 bool enableNoCopy /* = false*/,
									 uint32 srcStep /*= (1 << 16)*/
									) const
{
	nlassert(numAttrib);
	if (srcStep == (1 << 16))
	{		
		TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
										, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib;
		uint8 *ptDat  = (uint8 *) tab; 
		do
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt);
			++ speedIt;
			ptDat += stride;
		}
		while (speedIt != endSpeedIt);
		return tab;
	}
	else
	{
		uint32 fpIndex = startIndex * srcStep;
		const TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin();										
		uint8 *ptDat  = (uint8 *) tab; 
		while (numAttrib --)
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*(speedIt + (fpIndex >> 16)));			
			ptDat += stride;
			fpIndex += srcStep;
		}		
		return tab;	
	}
}

///============================================================================
void CPSPlaneBasisFollowSpeed::make4(CPSLocated *loc,
									 uint32 startIndex,
									 void *tab,
									 uint32 stride,
									 uint32 numAttrib,
									 uint32 srcStep /*= (1 << 16)*/
									) const
{
	nlassert(numAttrib);
	if (srcStep == (1 << 16))
	{		
		TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
										, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib;	
		uint8 *ptDat  = (uint8 *) tab;
		do
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt);
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride << 1;
			++ speedIt;		
		}
		while (speedIt != endSpeedIt);
	}
	else
	{		
		uint32 fpIndex = startIndex * srcStep;
		const TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin();	
		uint8 *ptDat  = (uint8 *) tab;
		while (numAttrib --)
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*(speedIt + (fpIndex >> 16)));
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride;
			*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
			ptDat += stride << 1;
			fpIndex += srcStep;
		}		
	}
}		

///============================================================================
void CPSPlaneBasisFollowSpeed::makeN(CPSLocated *loc,
									 uint32 startIndex,
									 void *tab,
									 uint32 stride,
									 uint32 numAttrib,
									 uint32 nbReplicate,
									 uint32 srcStep /*= (1 << 16) */
									) const
{
	nlassert(numAttrib);
	if (srcStep == (1 << 16))
	{
		nlassert(nbReplicate > 1); 
		TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin() + startIndex
										, endSpeedIt = loc->getSpeed().begin() + startIndex + numAttrib;	
		uint8 *ptDat  = (uint8 *) tab;
		uint k; 
		do
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*speedIt);
			
			k = nbReplicate - 1;		

			do
			{
				*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
				ptDat += stride;
			}
			while (--k);
			ptDat += stride;
		
			++ speedIt;		
		}
		while (speedIt != endSpeedIt);
	}
	else
	{
		uint32 fpIndex = startIndex * srcStep;
		nlassert(nbReplicate > 1); 
		const TPSAttribVector::const_iterator speedIt = loc->getSpeed().begin();
		uint8 *ptDat  = (uint8 *) tab;
		uint k; 
		while (numAttrib --)
		{
			*(CPlaneBasis *) ptDat = CPlaneBasis(*(speedIt + (fpIndex >> 16)));
			
			k = nbReplicate - 1;		

			do
			{
				*(CPlaneBasis *) (ptDat + stride) = *(CPlaneBasis *) ptDat;
				ptDat += stride;
			}
			while (--k);
			ptDat += stride;
	
			fpIndex += srcStep;
		}		
	}
}


/////////////////////////////////////////////
// CSpinnerFunctor implementation		   //
/////////////////////////////////////////////


///============================================================================
CSpinnerFunctor::CSpinnerFunctor() : _NbSamples(16), _Axis(NLMISC::CVector::K)
{
	updateSamples();
}

///============================================================================
void CSpinnerFunctor::setAxis(const NLMISC::CVector &axis)
{
	_Axis = axis;
	updateSamples();
}

///============================================================================
const void CSpinnerFunctor::setNumSamples(uint32 nbSamples)
{
	nlassert(nbSamples > 0);
	_NbSamples = nbSamples;
	updateSamples();
}

///============================================================================
const uint32 CSpinnerFunctor::getNumSamples(void) const
{
	return _NbSamples;
}

///============================================================================
void CSpinnerFunctor::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	f.serial(_Axis, _NbSamples);
	if (f.isReading()) updateSamples();
}

///============================================================================
void CSpinnerFunctor::updateSamples(void)
{
	// compute step between each angle
	const float angInc = (float) (2.f * NLMISC::Pi / _NbSamples);
	_PBTab.resize(_NbSamples + 1);
	NLMISC::CMatrix mat;
	// compute each sample
	for (uint32 k = 0; k < _NbSamples; ++k)
	{		
		mat.setRot(NLMISC::CQuat(_Axis, k * angInc));
		_PBTab[k] = CPlaneBasis(mat.getI(), mat.getJ());
	}
}

///============================================================================
void PSRegisterPlaneBasisAttribs()
{
	NLMISC_REGISTER_CLASS(CPSPlaneBasisBlender);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisGradient);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisMemory);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisBinOp);
	NLMISC_REGISTER_CLASS(CPSPlaneBasisFollowSpeed);
	NLMISC_REGISTER_CLASS(CPSBasisSpinner);
}

} // NL3D


