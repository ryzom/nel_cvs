/** \file particle_system.cpp
 * <File description>
 *
 * $Id: particle_system.cpp,v 1.10 2001/05/10 09:18:27 vizerie Exp $
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

#include "nel/3d/particle_system.h"
#include "nel/3d/driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/primitive_block.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/nelu.h"
#include "nel/3d/ps_util.h"




namespace NL3D {




uint32 CParticleSystem::_NbParticlesDrawn = 0 ;


/////////////////////////////////////////////
// CParticleSystemProcess implementation   //
/////////////////////////////////////////////


CFontGenerator *CParticleSystemProcess::getFontGenerator(void)
{
			nlassert(_Owner) ;
			return _Owner->getFontGenerator() ;
}

const CFontGenerator *CParticleSystemProcess::getFontGenerator(void) const 
{
			nlassert(_Owner) ;
			return _Owner->getFontGenerator() ;
}

CFontManager *CParticleSystemProcess::getFontManager(void)
{
			nlassert(_Owner) ;
			return _Owner->getFontManager() ;
}

const CFontManager *CParticleSystemProcess::getFontManager(void) const 
{
			nlassert(_Owner) ;
			return _Owner->getFontManager() ;
}



void CParticleSystemProcess::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'PSPR') ;
	f.serialVersion(1) ;
	f.serialPtr(_Owner) ;
	f.serial(_SystemBasisEnabled) ;
}




///////////////////////////////////
// CPaticleSystem implementation //
///////////////////////////////////


/*
 * Constructor
 */
CParticleSystem::CParticleSystem() : _FontGenerator(NULL), _FontManager(NULL)
{
}



// clonage
CParticleSystem *CParticleSystem::clone(CPSCopyHelper *ch)
{

/*	if (!ch)
	{
		ch = new CPSCopyHelper ;		
		CParticleSystem *result =  ch->clone(*this) ;
		delete ch ;
		return result ;
	}
	else
	{
		CParticleSystem *copy = NULL ;
		
	
		copy = new CParticleSystem ;
	
		for (TProcessVect::const_iterator it = ps._ProcessVect.begin() ; it != ps._ProcessVect.end() ; ++it)
		{
			copy->_ProcessVect.push_back(*ch.clone(*it)) ;
		}
	// copy the attributes

		copy->_FontGenerator = ps._FontGenerator ;
		copy->_FontManager = ps._FontManager ;
		copy->_ViewMat = ps._ViewMat ;
		copy->_SysMat = ps._SysMat ;
		copy->_InvSysMat = ps._InvSysMat ;

		return copy ;
	} */
	return NULL ;
}



/// dtor
CParticleSystem::~CParticleSystem()
{
	for (TProcessVect::iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		delete *it ;
	}
}


void CParticleSystem::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	if (pass == PSSolidRender ||pass == PSBlendRender)
	{
		 // store the view matrix for the rendring pass
		 // it is needed for FaceLookat or the like
		_ViewMat = CNELU::Driver->getViewMatrix() ;
		_ViewMat.transpose() ;
	}

	for (TProcessVect::iterator it = _ProcessVect.begin() ; it != _ProcessVect.end() ; ++it)
	{
		(*it)->step(pass, ellapsedTime) ;
	}
}

void CParticleSystem::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	f.serialCheck((uint32) 'PSYS') ;
	f.serialVersion(1) ;	
	f.serial(_ViewMat) ;
	f.serial(_SysMat) ;
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
	delete *it ;
	_ProcessVect.erase(it) ;
	ptr->setOwner(NULL) ;
}





bool CParticleSystem::computeBBox(NLMISC::CAABBox &aabbox) const
{
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
	return foundOne ;
}



void CParticleSystem::setSysMat(const CMatrix &m)
{
	_SysMat = m ;
	_InvSysMat = _SysMat.inverted() ;
}







/*
CTransformShape *CParticleSystemShape::createInstance(CScene &scene)
{
	return new CParticleSystemTransformShape(_ParticleSystem) ; // make a copy of the system
}


void	CParticleSystemShape::render(IDriver *drv, CTransformShape *trans)
{
	nlassert(drv);
	// get the particle system instance
	nlassert(dynamic_cast<CMeshInstance*>(trans));
}*/

} // NL3D
