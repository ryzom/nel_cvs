/** \file particle_system_manager.cpp
 * <File description>
 *
 * $Id: particle_system_manager.cpp,v 1.6 2003/03/03 12:56:08 boucher Exp $
 */

/* Copyright, 2000 - 2002 Nevrax Ltd.
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

#include "3d/particle_system_manager.h"
#include "3d/particle_system_model.h"
#include "3d/scene.h"


namespace NL3D {

///=========================================================	
CParticleSystemManager::CParticleSystemManager() : _NumModels(0)
{
	_CurrListIterator = _ModelList.end();
}

///=========================================================	
void	CParticleSystemManager::refreshModels(const std::vector<NLMISC::CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos)
{	
	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif

	if (_NumModels == 0) return;
	const uint toProcess = std::min(_NumModels, (uint) NumProcessToRefresh);	

	for (uint k = 0; k < toProcess; ++k)
	{
		if (_CurrListIterator == _ModelList.end())
		{
			_CurrListIterator = _ModelList.begin();
		}
		
		if ((*_CurrListIterator)->refreshRscDeletion(worldFrustumPyramid, viewerPos)) // check if the resources of the model must be released (which also release them)
		{						
			_CurrListIterator = _ModelList.erase(_CurrListIterator);			
			-- _NumModels;
		}
		else
		{
			++ _CurrListIterator;
		}		
	}

	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif
}

///=========================================================	
CParticleSystemManager::TModelHandle CParticleSystemManager::addSystemModel(CParticleSystemModel *model)
{
	#ifdef NL_DEBUG
		nlassert(std::find(_ModelList.begin(), _ModelList.end(), model) == _ModelList.end()); 
	#endif
	_ModelList.push_front(model);
	++_NumModels;

	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif

	TModelHandle handle;
	handle.Valid = true;
	handle.Iter = _ModelList.begin();
	return handle;
}


///=========================================================	
void		CParticleSystemManager::removeSystemModel(TModelHandle &handle)
{	
	nlassert(handle.Valid);
	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif
	nlassert(_NumModels != 0);	
	if (handle.Iter == _CurrListIterator)
	{
		++_CurrListIterator;		
	}

	_ModelList.erase(handle.Iter);
	--_NumModels;
	#ifdef NL_DEBUG
		nlassert(_NumModels == _ModelList.size());
	#endif
}


///=========================================================	
CParticleSystemManager::TModelHandle	CParticleSystemManager::addPermanentlyAnimatedSystem(CParticleSystemModel *ps)
{
	#ifdef NL_DEBUG
		nlassert(std::find(_PermanentlyAnimatedModelList.begin(), _PermanentlyAnimatedModelList.end(), ps) == _PermanentlyAnimatedModelList.end());
	#endif
	_PermanentlyAnimatedModelList.push_front(ps);


	TModelHandle handle;
	handle.Valid = true;
	handle.Iter = _PermanentlyAnimatedModelList.begin();
	return handle;	
}

///=========================================================	
void			CParticleSystemManager::removePermanentlyAnimatedSystem(CParticleSystemManager::TModelHandle &handle)
{
	nlassert(handle.Valid);
	_PermanentlyAnimatedModelList.erase(handle.Iter);	
}

///=========================================================	
void	CParticleSystemManager::processAnimate(TAnimationTime deltaT)
{
	for (TModelList::iterator it = _PermanentlyAnimatedModelList.begin(); it != _PermanentlyAnimatedModelList.end(); ++it)
	{
		CParticleSystemModel &psm = *(*it);
		CParticleSystem		 *ps  = psm.getPS();
		if (ps)
		{
			if (psm.isAutoGetEllapsedTimeEnabled())
			{
				psm.setEllapsedTime(ps->getScene()->getEllapsedTime() * psm.getEllapsedTimeRatio());
			}
			TAnimationTime delay = psm.getEllapsedTime();
			// animate particles
			ps->step(CParticleSystem::Anim, delay);	

			if (!psm.getEditionMode())
			{			
				// test deletion condition (no more particle, no more particle and emitters)
				if (ps->getDestroyCondition() != CParticleSystem::none)
				{
					if (ps->getSystemDate() > ps->getDelayBeforeDeathConditionTest())
					{
						switch (ps->getDestroyCondition())
						{
							case CParticleSystem::noMoreParticles:
								if (!ps->hasParticles())
								{							
									psm.releaseRscAndInvalidate();
									return;
								}
							break;
							case CParticleSystem::noMoreParticlesAndEmitters:
								if (!ps->hasParticles() && !ps->hasEmitters())
								{
									psm.releaseRscAndInvalidate();
									return;
								}
							break;
							default: break;
						}
					}
				}
			}
		}
	}
}

} // NL3D
