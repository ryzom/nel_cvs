/** \file particle_system_model.cpp
 * <File description>
 *
 * $Id: particle_system_model.cpp,v 1.6 2001/07/18 09:07:36 vizerie Exp $
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

#include "3d/particle_system_model.h"
#include "3d/particle_system_shape.h"
#include "3d/particle_system.h"
#include "3d/scene.h"
#include "nel/misc/debug.h"

namespace NL3D {

/// ctor
CParticleSystemModel::CParticleSystemModel() : _ParticleSystem(NULL), _EllapsedTime(0.01f), _ToolDisplayEnabled(false)
						, _AutoGetEllapsedTime(true), _TransparencyStateTouched(true), _Scene(NULL), _EditionMode(false)
{
	setOpacity(false) ;
	setTransparency(true) ;
	IAnimatable::resize(AnimValueLast);
}


void CParticleSystemModel::registerDtorObserver(IPSModelObserver *obs)
{
	nlassert(!isDtorObserver(obs)) ; // this observer has already been registered
	_Observers.push_back(obs) ;
}

void CParticleSystemModel::removeDtorObserver(IPSModelObserver *obs)
{	
	nlassert(isDtorObserver(obs)) ; // the observer must have been registered
	std::vector<IPSModelObserver *>::iterator it = std::find(_Observers.begin(), _Observers.end(), obs) ;
	_Observers.erase(it) ;
}

bool CParticleSystemModel::isDtorObserver(IPSModelObserver *obs)
{
	return std::find(_Observers.begin(), _Observers.end(), obs) != _Observers.end() ;
}



void CParticleSystemModel::registerBasic()
{
	// register the model and his observers
	CMOT::registerModel(ParticleSystemModelId, TransformShapeId, CParticleSystemModel::creator);	
	CMOT::registerObs(AnimDetailTravId, ParticleSystemModelId, CParticleSystemDetailObs::creator) ;
	CMOT::registerObs(ClipTravId, ParticleSystemModelId, CParticleSystemClipObs::creator) ;
}



void CParticleSystemModel::updateOpacityInfos(void)
{
	nlassert(_ParticleSystem) ;
	if (!_TransparencyStateTouched) return ;
	nlassert(_ParticleSystem) ;
	setOpacity(_ParticleSystem->hasOpaqueObjects() || _ToolDisplayEnabled) ;
	setTransparency(_ParticleSystem->hasTransparentObjects()) ;
	_TransparencyStateTouched = false ;
}

CParticleSystemModel::~CParticleSystemModel()
{
	std::vector<IPSModelObserver *> copyVect(_Observers.begin(), _Observers.end()) ;
	for (std::vector<IPSModelObserver *>::iterator it = _Observers.begin(); it != _Observers.end() ; ++it)
	{
		(*it)->psDestroyed(this) ;
	}
	delete _ParticleSystem ;
}

IAnimatedValue* CParticleSystemModel::getValue (uint valueId)
{
	nlassert(valueId < AnimValueLast) ;
	if (valueId < OwnerBit) return CTransformShape::getValue(valueId) ;
	nlassert(_ParticleSystem) ;
	return _ParticleSystem->getUserParamAnimatedValue(valueId - (uint)  PSParam0) ;	
}

const char *CParticleSystemModel::getPSParamName (uint valueId)
{	
	nlassert(valueId < AnimValueLast) ;
	const char *name[] = { "PSParam0", "PSParam1", "PSParam2", "PSParam3" } ;	
	return name[valueId - (uint) PSParam0] ;
}

const char *CParticleSystemModel::getValueName (uint valueId) const 
{ 
	nlassert(valueId < AnimValueLast) ;
	if (valueId < OwnerBit) return CTransformShape::getValueName(valueId) ;
	return getPSParamName(valueId) ; 
}

ITrack* CParticleSystemModel::getDefaultTrack (uint valueId)
{
	nlassert(valueId < AnimValueLast) ;
	nlassert(Shape) ;
	if (valueId < OwnerBit) return CTransformShape::getDefaultTrack(valueId) ;
	return (NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape)->getUserParamDefaultTrack(valueId - (uint) PSParam0)) ;
}

	
void CParticleSystemModel::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix /* =std::string() */)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix) ;
	addValue(chanMixer, PSParam0, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam1, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam2, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam3, OwnerBit, prefix, true);	
}

//////////////////////////////////////////////
// CParticleSystemDetailObs implementation  //
//////////////////////////////////////////////


void	CParticleSystemDetailObs ::traverse(IObs *caller)
{    
	CTransformAnimDetailObs::traverse(caller);

	if (ClipObs->Visible)
	{
		// test if bones must be updated.
		nlassert(dynamic_cast<CParticleSystemModel *>(Model)) ;
		CParticleSystemModel *psm= (CParticleSystemModel *)Model;

		
		
		
			
		CParticleSystem *ps = psm->getPS() ;

		// the system or its center is in the view frustrum, but it may not have been instanciated from its shape now
		if (!ps)
		{
			nlassert(psm->_Scene) ;
			nlassert(psm->Shape) ;
			ps = psm->_ParticleSystem = (NLMISC::safe_cast<CParticleSystemShape *>((IShape *) psm->Shape))->instanciatePS(*psm->_Scene) ;		
		}

		if (psm->isAutoGetEllapsedTimeEnabled())
		{
			psm->setEllapsedTime(ps->getScene()->getEllapsedTime()) ;
		}
		CAnimationTime delay = psm->getEllapsedTime() ;

		psm->updateOpacityInfos() ;

		ps->setSysMat(psm->getWorldMatrix()) ;
		nlassert(ps->getScene()) ;	

		// animate particles
		ps->step(PSCollision, delay) ;
		ps->step(PSMotion, delay) ;	 		
	}
}


////////////////////////////////////////////
// CParticleSystemClipObs implementation  //
////////////////////////////////////////////
	
void	CParticleSystemClipObs::traverse(IObs *caller)
{
	nlassert(!caller || dynamic_cast<IBaseClipObs*>(caller)) ;
	CClipTrav			*trav= (CClipTrav*)Trav;
	CParticleSystemModel		*m= (CParticleSystemModel*)Model;

	const std::vector<CPlane>	&pyramid= trav->WorldPyramid;
	CMatrix		&mat= HrcObs->WorldMatrix;

	



	/** traverse the sons
	  * we must do this before us, because this object may delete himself from the scene
	  */

	traverseSons() ;

	CParticleSystem *ps = m->_ParticleSystem ;
	// Transform the pyramid in Object space.

	

	
	

	if(!ps)
	{
		// the system wasn't present the last time, we use its center to see if its back in the view frustrum.
		// if this is the case, we say it isn't clipped, so it will be reinstancied from the shape
		// during the DetailAnimTraversal

	
		const CVector pos = m->getMatrix().getPos() ;

		for(sint i=0;i<(sint)pyramid.size();i++)
		{					
			if ( (pyramid[i] * mat) * pos < 0.f ) 
			{
				Visible = false ;
				return ;		
			}
		}


		const CVector d = pos - trav->CamPos ;
		// check wether system not too far		
		if (d * d > ps->getMaxViewDist() * ps->getMaxViewDist()) 
		{
			Visible = false ;
			return ;
		}

		nlassert(dynamic_cast<CClipTrav*>(Trav));
		static_cast<CClipTrav*>(Trav)->RenderTrav->addRenderObs(RenderObs);

		Visible = true ;
		return ;
	}
	

	/// set the view matrix of the system
	ps->setViewMat(trav->ViewMatrix) ;

	if (!m->_EditionMode)
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
							nlassert(m->_Scene) ;
							m->_Scene->deleteInstance(m);

							Visible = false ;
							return ;
						}
					break ;
					case CParticleSystem::noMoreParticlesAndEmitters:
						if (!ps->hasParticles() && !ps->hasEmitters())
						{
							nlassert(m->_Scene) ;
							m->_Scene->deleteInstance(m);

							Visible = false ;
							return ;
						}
					break ;
				}
			}
		}
	}

	NLMISC::CAABBox bbox ;
	ps->computeBBox(bbox) ;

	// Transform the pyramid in Object space.	
	for(sint i=0;i<(sint)pyramid.size();i++)
	{	
		// test wether the bbox is entirely in the neg side of the plane
		if (!bbox.clipBack(pyramid[i] * mat)) 
		{
			if (!m->_EditionMode)
			{
				if (ps->doesDestroyWhenOutOfFrustrum())
				{
					/// the system has gone out of the scope
					/// for now, we just delete the system
				

					if (m->_ParticleSystem->getDestroyModelWhenOutOfRange())
					{
						delete ps ;
						m->_ParticleSystem = NULL ;
						nlassert(m->_Scene) ;
						m->_Scene->deleteInstance(m);
					}
					else
					{
						delete ps ;
						m->_ParticleSystem = NULL ;
					}

				}				
			}

			Visible = false ;
			return ;			  			
		}
	}

	/// test if object is not too far	  
    CPlane farPlane ;
	farPlane.make(trav->CamLook, trav->CamPos + ps->getMaxViewDist() * trav->CamLook) ;
	if (!bbox.clipBack(farPlane * mat))
	{			
       	
		if (!m->_EditionMode)
		{
			if (m->_ParticleSystem->getDestroyModelWhenOutOfRange())
			{
				nlassert(m->_Scene) ;
				m->_Scene->deleteInstance(m);
			}
			
			delete ps ;
			m->_ParticleSystem = NULL ;			
		}

		Visible = false ;
		return ;
	}

	// set this model to visible, so that the detail anim traversal will be performed
	

	nlassert(dynamic_cast<CClipTrav*>(Trav));
	static_cast<CClipTrav*>(Trav)->RenderTrav->addRenderObs(RenderObs);
	Visible = true ;

}


} // NL3D
