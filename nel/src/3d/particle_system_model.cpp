/** \file particle_system_model.cpp
 * <File description>
 *
 * $Id: particle_system_model.cpp,v 1.20 2001/09/26 17:03:05 vizerie Exp $
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
#include "nel/misc/common.h"


#include "cluster.h" // ask trap




namespace NL3D {

/// ctor
CParticleSystemModel::CParticleSystemModel() : _ParticleSystem(NULL), _EllapsedTime(0.01f), _ToolDisplayEnabled(false)
						, _AutoGetEllapsedTime(true), _TransparencyStateTouched(true), _Scene(NULL), _EditionMode(false)
						, _Invalidated(false), _InsertedInVisibleList(false), _InCluster(false)
{
	setOpacity(false);
	setTransparency(true);
	IAnimatable::resize(AnimValueLast);
	_TriggerAnimatedValue.Value = true;
}



void CParticleSystemModel::setEditionMode(bool enable /*= true*/)
{ 
	if (enable)
	{
		/// we need to have the system resources instanciated if we want to work with it
		if (!_ParticleSystem)
		{
			nlassert(_Scene);
			nlassert(Shape);
			_ParticleSystem = (NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape))->instanciatePS(*_Scene);				
		}
	}
	_EditionMode = enable; 
}


void CParticleSystemModel::registerPSModelObserver(IPSModelObserver *obs)
{
	nlassert(!isPSModelObserver(obs)); // this observer has already been registered
	_Observers.push_back(obs);
}

void CParticleSystemModel::removePSModelObserver(IPSModelObserver *obs)
{	
	nlassert(isPSModelObserver(obs)); // the observer must have been registered
	std::vector<IPSModelObserver *>::iterator it = std::find(_Observers.begin(), _Observers.end(), obs);
	_Observers.erase(it);
}

bool CParticleSystemModel::isPSModelObserver(IPSModelObserver *obs)
{
	return std::find(_Observers.begin(), _Observers.end(), obs) != _Observers.end();
}



void CParticleSystemModel::registerBasic()
{
	// register the model and his observers
	CMOT::registerModel(ParticleSystemModelId, TransformShapeId, CParticleSystemModel::creator);	
	CMOT::registerObs(AnimDetailTravId, ParticleSystemModelId, CParticleSystemDetailObs::creator);
	CMOT::registerObs(ClipTravId, ParticleSystemModelId, CParticleSystemClipObs::creator);
	CMOT::registerObs(RenderTravId, ParticleSystemModelId, CParticleSystemRenderObs::creator);
}



void CParticleSystemModel::updateOpacityInfos(void)
{
	nlassert(_ParticleSystem);
	if (!_TransparencyStateTouched) return;
	nlassert(_ParticleSystem);
	setOpacity(_ParticleSystem->hasOpaqueObjects() || _ToolDisplayEnabled);
	setTransparency(_ParticleSystem->hasTransparentObjects());
	_TransparencyStateTouched = false;
}

void CParticleSystemModel::getAABBox(NLMISC::CAABBox &bbox) const
{
	if (_ParticleSystem)
	{
		_ParticleSystem->computeBBox(bbox);
	}
	else
	{
		NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape)->getAABBox(bbox);
	}
}


CParticleSystemModel::~CParticleSystemModel()
{	
	delete _ParticleSystem;
}




void CParticleSystemModel::invalidate(void)
{
	delete _ParticleSystem;
	_ParticleSystem = NULL;
	_Invalidated = true;
	std::vector<IPSModelObserver *> copyVect(_Observers.begin(), _Observers.end());
	for (std::vector<IPSModelObserver *>::iterator it = _Observers.begin(); it != _Observers.end(); ++it)
	{
		(*it)->invalidPS(this); // if this crash, then you forgot to call removePSModelObserver !
	}
}

IAnimatedValue* CParticleSystemModel::getValue (uint valueId)
{
	nlassert(valueId < AnimValueLast);
	if (valueId < OwnerBit) return CTransformShape::getValue(valueId);	
	if (valueId < PSTrigger)
	{
	
		return &_UserParam[valueId - (uint)  PSParam0];
	}
	return &_TriggerAnimatedValue;
}

const char *CParticleSystemModel::getPSParamName (uint valueId)
{	
	nlassert(valueId < AnimValueLast);
	const char *name[] = { "PSParam0", "PSParam1", "PSParam2", "PSParam3" };	
	return name[valueId - (uint) PSParam0];
}

const char *CParticleSystemModel::getValueName (uint valueId) const 
{ 
	nlassert(valueId < AnimValueLast);
	if (valueId < OwnerBit) return CTransformShape::getValueName(valueId);
	if (valueId < PSTrigger) return getPSParamName(valueId); 
	return "PSTrigger";
}

ITrack* CParticleSystemModel::getDefaultTrack (uint valueId)
{
	nlassert(valueId < AnimValueLast);
	nlassert(Shape);
	if (valueId < OwnerBit) return CTransformShape::getDefaultTrack(valueId);
	if (valueId < PSTrigger) 
	{
		return (NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape)->getUserParamDefaultTrack(valueId - (uint) PSParam0));
	}
	return NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape)->getDefaultTriggerTrack();
}

	
void CParticleSystemModel::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix /* =std::string() */)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix);
	addValue(chanMixer, PSParam0, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam1, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam2, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam3, OwnerBit, prefix, true);	
	addValue(chanMixer, PSTrigger, OwnerBit, prefix, true);	
}


float CParticleSystemModel::getNumTriangles (float distance)
{
	if (!_ParticleSystem) return 0;
	return (float) _ParticleSystem->getWantedNumTris(distance);
}

//////////////////////////////////////////////
// CParticleSystemDetailObs implementation  //
//////////////////////////////////////////////


void	CParticleSystemDetailObs::traverse(IObs *caller)
{    
	CTransformAnimDetailObs::traverse(caller);	
	nlassert(dynamic_cast<CParticleSystemModel *>(Model));
	CParticleSystemModel *psm= (CParticleSystemModel *)Model;
	CParticleSystem *ps = psm->getPS();
	if (psm->_Invalidated) return;

	if (!psm->_InCluster)
	{
		CParticleSystemShape		*pss= NLMISC::safe_cast<CParticleSystemShape *>((IShape *)psm->Shape);
		if (pss->_DestroyWhenOutOfFrustum)
		{
			if (pss->_DestroyModelWhenOutOfRange)
			{
				psm->invalidate();			
			}
			else // remove rsc but do not invalidate the system
			{
				delete psm->_ParticleSystem;
				psm->_ParticleSystem = NULL;
			}
			return;
		}
		if (!ps) return;
	}

	

	

	// check for trigger. If the trigger is false, and there is a system instanciated, we delete it.
	if (!psm->getEditionMode())
	{
		if (!psm->_TriggerAnimatedValue.Value)
		{									
			// system is off, or hasn't been instanciated now...
			if (ps)
			{
				delete ps;
				psm->_ParticleSystem = NULL;			
			}
			return;
		}
	}

	// the system or its center is in the view frustum, but it may not have been instanciated from its shape now
	if (!ps)
	{
		nlassert(psm->_Scene);
		nlassert(psm->Shape);
		ps = psm->_ParticleSystem = (NLMISC::safe_cast<CParticleSystemShape *>((IShape *) psm->Shape))->instanciatePS(*psm->_Scene);			
	}

	if (psm->_InCluster ||  ps->doesPerformMotionWhenOutOfFrustum())
	{
		CClipTrav			*trav= (CClipTrav*) ClipObs->Trav;
		const CMatrix		&mat= HrcObs->WorldMatrix;	 
		ps->setSysMat(mat);
		ps->setViewMat(trav->ViewMatrix);
		if (psm->isAutoGetEllapsedTimeEnabled())
		{
			psm->setEllapsedTime(ps->getScene()->getEllapsedTime());
		}
		CAnimationTime delay = psm->getEllapsedTime();

		psm->updateOpacityInfos();

		//ps->setSysMat(psm->getWorldMatrix());
		nlassert(ps->getScene());	


		// setup the number of faces we allow
		ps->setNumTris((uint) psm->getNumTrianglesAfterLoadBalancing());

		// setup system user parameters for parameters that have been touched
		for (uint k = 0; k < MaxPSUserParam; ++k)
		{
			if (psm->isTouched((uint)CParticleSystemModel::PSParam0 + k))
			{
				ps->setUserParam(k, psm->_UserParam[k].Value);
				psm->clearFlag((uint)CParticleSystemModel::PSParam0 + k);
			}
		}
		// animate particles
		ps->step(CParticleSystem::Anim, delay);

		// add a render obs if in cluster
		if (psm->_InCluster)
		{
			trav->RenderTrav->addRenderObs(ClipObs->RenderObs);
		}
	}
}



//////////////////////////////////////////////
// CParticleSystemRenderObs implementation  //
//////////////////////////////////////////////
void	CParticleSystemRenderObs::traverse(IObs *caller)
{

	

/*
	if (!psm->_OutOfFrustum)
	{*/
		CTransformShapeRenderObs::traverse(caller);
	//}
}


/*
 * CParticleSystemClipObs implementation              
 * For p.s., the Visible attribute is interpreted as 'not to far'. The '_InCluster'
 * flags tells us that the object is visible and not too far. We need this because we want
 * to know when this object is clipped out. This can only be done in the next traversal
 */
	
void	CParticleSystemClipObs::traverse(IObs *caller)
{
		nlassert(!caller || dynamic_cast<IBaseClipObs*>(caller));
		CParticleSystemModel		*m= (CParticleSystemModel*)Model;	
		if (m->_Invalidated) return;		
		CClipTrav			*trav= (CClipTrav*)Trav;
		

		if (Date != trav->CurrentDate) 
		{
			m->_InsertedInVisibleList = false;
			m->_InCluster = false;
		}
		if (m->_InCluster) return; // already visible
		Date = trav->CurrentDate;


		const std::vector<CPlane>	&pyramid= trav->WorldPyramid;	
		/** traverse the sons
		  * we must do this before us, because this object may delete himself from the scene
		  */
		traverseSons();

		// now the pyramid is directly expressed in the world
		const CMatrix		&mat= HrcObs->WorldMatrix;	 

		

		CParticleSystem *ps = m->_ParticleSystem;
		// Transform the pyramid in Object space.

		
		if(!ps)
		{
			CParticleSystemShape		*pss= NLMISC::safe_cast<CParticleSystemShape *>((IShape *)m->Shape);
			nlassert(pss);

			// the system wasn't present the last time, we use its center to see if it's back in the view frustum,
			// or if it is near enough.
			// if this is the case, we say it isn't clipped, so it will be reinstanciated from the shape
			// during the DetailAnimTraversal

			const CVector pos = m->getMatrix().getPos();
		
			const CVector d = pos - trav->CamPos;


			// test the shape to see wether we have a precomputed bbox

			if (!pss->_UsePrecomputedBBox)
			{
				// check wether system not too far		
				if (d * d > pss->_MaxViewDist * pss->_MaxViewDist) 
				{
					Visible = false;					
					if (pss->_DestroyModelWhenOutOfRange)
					{
						m->invalidate();
					}							
					return;
				}		
				
				/// frustum test		
				for(sint i=0; i < (sint)pyramid.size(); i++)
				{					
					if ( (pyramid[i]   *  mat  ) * pos > 0.f ) 

					{						
						Visible = true;
						if (!m->_InsertedInVisibleList)
						{
							trav->addVisibleObs(this);
							m->_InsertedInVisibleList = true;
						}						
						return;
					}
				}			
				m->_InCluster = true;						
			}
			else
			{

				CPlane farPlane;
				farPlane.make(trav->CamLook, trav->CamPos + pss->_MaxViewDist * trav->CamLook);
				if (!pss->_PrecomputedBBox.clipBack(farPlane  * mat  ))				
				{
					Visible = false;				
					if (pss->_DestroyModelWhenOutOfRange)
					{
						m->invalidate();
					}			
					return;
				}					
				/// frustum test		
				for(sint i=0; i < (sint)pyramid.size(); i++)
				{					
					if ( !pss->_PrecomputedBBox.clipBack(pyramid[i]  * mat  ) ) 
					{
						Visible = true;
						if (!m->_InsertedInVisibleList)
						{
							trav->addVisibleObs(this);
							m->_InsertedInVisibleList = true;
						}						
						return;					
					}
				}			

				m->_InCluster = true;
				return;
				
			}
		}
		
		


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
								m->invalidate();
								return;
							}
						break;
						case CParticleSystem::noMoreParticlesAndEmitters:
							if (!ps->hasParticles() && !ps->hasEmitters())
							{
								m->invalidate();
								return;
							}
						break;
					}
				}
			}
		}

		NLMISC::CAABBox bbox;
		ps->computeBBox(bbox);

		/// test if object is not too far	  
		CPlane farPlane;
		farPlane.make(trav->CamLook, trav->CamPos + ps->getMaxViewDist() * trav->CamLook);
		if (!bbox.clipBack(farPlane  * mat  ))
		{			
       		
			if (!m->_EditionMode)
			{						
				if (ps->getDestroyModelWhenOutOfRange())
				{ 
					m->invalidate();
					return;
				}
				else
				{
					delete ps;
					m->_ParticleSystem = NULL;	
				}
			}
			Visible = false;
			return;
		}

		// Transform the pyramid in Object space.	
		for(sint i=0;i<(sint)pyramid.size();i++)
		{	
			// test wether the bbox is entirely in the neg side of the plane
			if (!bbox.clipBack(pyramid[i]  * mat  )) 
			{
				
				if (!m->_EditionMode)
				{
					Visible = true; // not too far, but not in cluster
					if (!m->_InsertedInVisibleList)
					{
						trav->addVisibleObs(this);
						m->_InsertedInVisibleList = true;
					}														
				}				
				return;			  			
			}
		}

		m->_InCluster = true;
}


} // NL3D
