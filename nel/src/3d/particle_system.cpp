/** \file particle_system.cpp
 * <File description>
 *
 * $Id: particle_system.cpp,v 1.23 2001/07/17 15:57:02 vizerie Exp $
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

#include "3d/particle_system.h"
#include "3d/ps_located.h"
#include "3d/driver.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include "3d/primitive_block.h"
#include "3d/nelu.h"
#include "3d/ps_util.h"
#include "3d/ps_particle.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"
#include "nel/misc/stream.h"




namespace NL3D {




uint32 CParticleSystem::_NbParticlesDrawn = 0 ;






///////////////////////////////////
// CPaticleSystem implementation //
///////////////////////////////////


/*
 * Constructor
 */
CParticleSystem::CParticleSystem() : _FontGenerator(NULL), _FontManager(NULL)
									, _Date(0), _Scene(NULL), _CurrEditedElementLocated(NULL)
									, _CurrEditedElementIndex(0), _Driver(NULL)
									, _TimeThreshold(0.1f)
									, _MaxNbIntegrations(4)
									, _CanSlowDown(true)
									, _AccurateIntegration(false)
									, _InvMaxViewDist(1.f / 50.f)									
									, _LODRatio(0.5f)
									, _ComputeBBox(true)
									, _DieCondition(none)
									, _DelayBeforeDieTest(0.2f) 
									, _DestroyModelWhenOutOfRange(false)
									, _DestroyWhenOutOfFrustrum(false)
									, _SystemDate(0.f)
									, _CurrentLODRatio(0)
{
	for (uint k = 0 ; k < MaxPSUserParam ; ++k) _UserParam[k].Value = 0 ;
}


/// dtor
CParticleSystem::~CParticleSystem()
{
	for (TProcessVect::iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		delete *it ;
	}
}


void CParticleSystem::setViewMat(const NLMISC::CMatrix &m)
{
	_ViewMat = m ;
	_InvertedViewMat = m.inverted() ;
}				




bool CParticleSystem::hasEmitters(void) const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		if ((*it)->hasEmitters()) return true ;
	}
	return false ;
}
	
bool CParticleSystem::hasParticles(void) const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		if ((*it)->hasParticles()) return true ;
	}
	return false ;
}


void CParticleSystem::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{	
	CAnimationTime et = ellapsedTime ;
	uint32 nbPass = 1 ;

	if (pass == PSSolidRender ||pass == PSBlendRender)
	{
		++_Date ; // update time		 
	}
	else if (_AccurateIntegration && pass != PSToolRender)
	{
		if (et > _TimeThreshold)
		{
			nbPass = (uint32) ceilf(et / _TimeThreshold) ;
			if (nbPass > _MaxNbIntegrations)
			{ 
				nbPass = _MaxNbIntegrations ;
				et = _CanSlowDown ? _TimeThreshold : (ellapsedTime / nbPass) ;
			}
			else
			{
				et = ellapsedTime / nbPass ;
			}
		}
	}

	if (pass == PSMotion)
	{
		// store the view matrix for the rendring pass
		 // it is needed for FaceLookat or the like		
		// update current lod ratio

		const CVector d = _SysMat.getPos() - _ViewMat.getPos() ;		
		_CurrentLODRatio = 1.f - (d.norm() * _InvMaxViewDist) ; 
		if (_CurrentLODRatio < 0) _CurrentLODRatio = 0.f ;
		
	
		// update system date
		_SystemDate += ellapsedTime ;

	
	}
	
	do
	{
		for (TProcessVect::iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
		{
			(*it)->step(pass, et) ;
		}
	}
	while (--nbPass) ;
}

void CParticleSystem::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	sint version =  f.serialVersion(6) ;	
	//f.serial(_ViewMat) ;
	f.serial(_SysMat) ;
	f.serial(_Date) ;
	if (f.isReading())
	{
		// delete previously attached process
		for (TProcessVect::iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
		{
			delete (*it) ;
		}

		_ProcessVect.clear() ;

		f.serialContPolyPtr(_ProcessVect) ;		
	
		_InvSysMat = _SysMat.inverted() ;
		_FontGenerator = NULL ;
		_FontManager = NULL ;
	}
	else
	{
		f.serialContPolyPtr(_ProcessVect) ;	
	}
	
	if (version > 1) // name of the system
	{
		f.serial(_Name) ;
	}

	if (version > 2) // infos about integration, and LOD
	{
		f.serial(_AccurateIntegration) ;
		if (_AccurateIntegration) f.serial(_CanSlowDown, _TimeThreshold, _MaxNbIntegrations) ;
		f.serial(_InvMaxViewDist, _LODRatio) ;	
	}

	if (version > 3) // tell wether the system must compute his bbox, hold a precomputed bbox
	{
		f.serial(_ComputeBBox) ;
		if (!_ComputeBBox)
		{
			f.serial(_PreComputedBBox) ;
		}
	}

	if (version > 4) // lifetime informations
	{
		f.serial(_DestroyModelWhenOutOfRange) ;
		f.serialEnum(_DieCondition) ;
		if (_DieCondition != none)
		{
			f.serial(_DelayBeforeDieTest) ;
		}
	}	

	if (version > 5)
	{
		f.serial(_DestroyWhenOutOfFrustrum) ;
	}


}


void CParticleSystem::attach(CParticleSystemProcess *ptr)
{
	nlassert(std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr) == _ProcessVect.end() ) ;
	nlassert(ptr->getOwner() == NULL) ; // deja attache a un autre systeme
	_ProcessVect.push_back(ptr) ;
	ptr->setOwner(this) ;
}



void CParticleSystem::remove(CParticleSystemProcess *ptr)
{
	TProcessVect::iterator it = std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr) ;
	nlassert(it != _ProcessVect.end() ) ;	
	_ProcessVect.erase(it) ;
	
	

	delete ptr ;
}





void CParticleSystem::computeBBox(NLMISC::CAABBox &aabbox)
{
	if (!_ComputeBBox)
	{
		aabbox = _PreComputedBBox ;
		return ;
	}

	bool foundOne = false ;
	NLMISC::CAABBox tmpBox ;
	for (TProcessVect::const_iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		if ((*it)->computeBBox(tmpBox))
		{
			if ((*it)->isInSystemBasis())
			{
				// rotate the aabbox so that it is in the correct basis
				tmpBox = CPSUtil::transformAABBox(_SysMat, tmpBox) ;
			}
			if (foundOne)
			{
				aabbox = CPSUtil::computeAABBoxUnion(aabbox, tmpBox) ;
			}
			else
			{
				aabbox = tmpBox ;
				foundOne = true ;
			}
		}
	}

	if (!foundOne)
	{
		aabbox.setCenter(NLMISC::CVector::Null) ;
		aabbox.setHalfSize(NLMISC::CVector::Null) ;
	}
	
	_PreComputedBBox = aabbox ;	
}



void CParticleSystem::setSysMat(const CMatrix &m)
{
	_SysMat = m ;
	_InvSysMat = _SysMat.inverted() ;
}




bool CParticleSystem::hasOpaqueObjects(void) const
{
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		if (dynamic_cast<CPSLocated *>(*it))
		{
			for (uint k = 0 ; k < ((CPSLocated *) *it)->getNbBoundObjects() ; ++k)
			{
				CPSLocatedBindable *lb = ((CPSLocated *) *it)->getBoundObject(k) ;
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasOpaqueFaces()) return true ;
				}
			}
		}
	}
	return false ;
}


bool CParticleSystem::hasTransparentObjects(void) const
{
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		if (dynamic_cast<CPSLocated *>(*it))
		{
			for (uint k = 0 ; k < ((CPSLocated *) *it)->getNbBoundObjects() ; ++k)
			{
				CPSLocatedBindable *lb = ((CPSLocated *) *it)->getBoundObject(k) ;
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasTransparentFaces()) return true ;
				}
			}
		}
	}
	return false ;
}




void CParticleSystem::getLODVect(NLMISC::CVector &v, float &offset,  bool systemBasis)
{
	if (!systemBasis)
	{
		v = _InvMaxViewDist * _InvertedViewMat.getJ() ;
		offset = - _InvertedViewMat.getPos() * v ;
	}
	else
	{
		const CVector tv = _InvSysMat * _InvertedViewMat.getJ() ;
		const CVector org = _InvSysMat * _InvertedViewMat.getPos() ;
		v = _InvMaxViewDist * tv ;
		offset = - org * v ;
	}
}


TPSLod CParticleSystem::getLOD(void) const
{
	const float dist = fabsf(_InvMaxViewDist * (_SysMat.getPos() - _InvertedViewMat.getPos()) * _InvertedViewMat.getJ()) ;
	return dist > _LODRatio ? PSLod2 : PSLod1 ;
}


} // NL3D
