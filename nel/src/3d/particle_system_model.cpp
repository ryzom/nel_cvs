/** \file particle_system_model.cpp
 * <File description>
 *
 * $Id: particle_system_model.cpp,v 1.40 2002/08/21 09:39:52 lecroart Exp $
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

#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"
#include "3d/particle_system_model.h"
#include "3d/particle_system_shape.h"
#include "3d/particle_system.h"
#include "3d/scene.h"
#include "3d/anim_detail_trav.h"
#include "3d/clip_trav.h"
#include "3d/render_trav.h"
#include "3d/skeleton_model.h"



#include "cluster.h" // ask trap




namespace NL3D {

///=====================================================================================
/// ctor
CParticleSystemModel::CParticleSystemModel() : _AutoGetEllapsedTime(true),
											   _ParticleSystem(NULL),
											   _Scene(NULL),
											   _EllapsedTime(0.01f),
											   _EllapsedTimeRatio(1.f),
											   _ToolDisplayEnabled(false),
											   _TransparencyStateTouched(true),
											   _EditionMode(false),
											   _Invalidated(false),
											   _InsertedInVisibleList(false),
											   _InClusterAndVisible(false)
{
	setOpacity(false);
	setTransparency(true);
	IAnimatable::resize(AnimValueLast);
	_TriggerAnimatedValue.Value = true;

	// AnimDetail behavior: Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);
}


///=====================================================================================
void CParticleSystemModel::setEditionMode(bool enable /*= true*/)
{ 
	if (enable)
	{
		/// we need to have the system resources instanciated if we want to work with it
		if (!_ParticleSystem)
		{
			nlassert(_Scene);
			nlassert(Shape);
			reallocRsc();
		}
	}
	_EditionMode = enable; 
}

///=====================================================================================
void CParticleSystemModel::registerPSModelObserver(IPSModelObserver *obs)
{
	nlassert(!isPSModelObserver(obs)); // this observer has already been registered
	_Observers.push_back(obs);
}

///=====================================================================================
void CParticleSystemModel::removePSModelObserver(IPSModelObserver *obs)
{	
	nlassert(isPSModelObserver(obs)); // the observer must have been registered
	std::vector<IPSModelObserver *>::iterator it = std::find(_Observers.begin(), _Observers.end(), obs);
	_Observers.erase(it);
}


///=====================================================================================
bool CParticleSystemModel::isPSModelObserver(IPSModelObserver *obs)
{
	return std::find(_Observers.begin(), _Observers.end(), obs) != _Observers.end();
}


///=====================================================================================
void CParticleSystemModel::registerBasic()
{
	// register the model and his observers
	CMOT::registerModel(ParticleSystemModelId, TransformShapeId, CParticleSystemModel::creator);	
	CMOT::registerObs(AnimDetailTravId, ParticleSystemModelId, CParticleSystemDetailObs::creator);
	CMOT::registerObs(ClipTravId, ParticleSystemModelId, CParticleSystemClipObs::creator);
	CMOT::registerObs(RenderTravId, ParticleSystemModelId, CParticleSystemRenderObs::creator);
}

///=====================================================================================
void CParticleSystemModel::updateOpacityInfos(void)
{
	nlassert(_ParticleSystem);
	if (!_TransparencyStateTouched) return;
	nlassert(_ParticleSystem);
	setOpacity(_ParticleSystem->hasOpaqueObjects() || _ToolDisplayEnabled);
	setTransparency(_ParticleSystem->hasTransparentObjects());
	_TransparencyStateTouched = false;
}


///=====================================================================================
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

///=====================================================================================
CParticleSystemModel::~CParticleSystemModel()
{	
	nlassert(_Scene);
	if (_ParticleSystem)
	{
		_Scene->getParticleSystemManager().removeSystemModel(_ModelHandle);
		/* _ParticleSystem = NULL; */
	}
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// Observers hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		nlassert(_FatherSkeletonModel==NULL);
	}
}


///=====================================================================================
/// Called when the resource (attached system) for this system must be reallocated
void CParticleSystemModel::reallocRsc()
{
	nlassert(_ParticleSystem == NULL);
	_ParticleSystem = NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape)->instanciatePS(*_Scene);
	nlassert(_ParticleSystem);
	nlassert(_Scene);
	CParticleSystemManager &psmgt = _Scene->getParticleSystemManager();
	_ModelHandle = psmgt.addSystemModel(this);
	if (_ParticleSystem->getAnimType() == CParticleSystem::AnimAlways)
	{
		_AnimatedModelHandle = psmgt.addPermanentlyAnimatedSystem(this);
	}
}

///=====================================================================================
bool CParticleSystemModel::refreshRscDeletion(const std::vector<CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos)
{
	if (_EditionMode) return false;
	/** Here we test wether the system has not gone out of scope.
	  * Why do we test this here addtionnaly to the clip traversal ?
	  * Simply because the clip observer is not called if the cluster it is inserted in is not parsed.
	  * This is not good, because we want to keep few CParticleSystem instance.
	  * This method solve that problem. This is called by the particle system manager when each scene as rendered
	  */


	nlassert(_ParticleSystem);	
	CParticleSystemShape		*shape = NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape);
	
	/* NLMISC::CVector sysPos = getTransformMode() == DirectMatrix ?
							 getMatrix().getPos()			    :
							 getPos(); */

	NLMISC::CVector sysPos = getWorldMatrix().getPos();
	
	NLMISC::CVector v = sysPos - viewerPos;
	/// test if not too far
	const float dist2 = v * v;
	
	if (dist2 > shape->_MaxViewDist * shape->_MaxViewDist) // too far ?
	{
		if (_AnimatedModelHandle.Valid)
		{
			_Scene->getParticleSystemManager().removePermanentlyAnimatedSystem(_AnimatedModelHandle);
			_AnimatedModelHandle.Valid = false;
		}		
		_ParticleSystem = NULL; // one less ref with the smart ptr
		if (shape->_DestroyModelWhenOutOfRange)
		{			
			_Invalidated = true;
		}		
		return true;		
	}

	/// frustum test
	if (shape->_DestroyWhenOutOfFrustum)
	{
		if (checkAgainstPyramid(worldFrustumPyramid) == false)
		{
			if (_AnimatedModelHandle.Valid)
			{
				_Scene->getParticleSystemManager().removePermanentlyAnimatedSystem(_AnimatedModelHandle);
				_AnimatedModelHandle.Valid = false;
			}

			if (shape->_DestroyModelWhenOutOfRange)
			{							
				_Invalidated = true;
			}
			_ParticleSystem = NULL; // one less ref with the smart ptr
			return true;
		}
	}

	return false;
}		

///=====================================================================================
void CParticleSystemModel::releaseRsc()
{
	if (!_ParticleSystem) return;

	if (_AnimatedModelHandle.Valid)
	{
		_Scene->getParticleSystemManager().removePermanentlyAnimatedSystem(_AnimatedModelHandle);
		_AnimatedModelHandle.Valid = false;
	}	
	_ParticleSystem = NULL; // one less ref with the smart ptr
	nlassert(_Scene);
	_Scene->getParticleSystemManager().removeSystemModel(_ModelHandle);
}

///=====================================================================================
void CParticleSystemModel::releaseRscAndInvalidate()
{

	if (!_ParticleSystem) return;

	if (_AnimatedModelHandle.Valid)
	{
		_Scene->getParticleSystemManager().removePermanentlyAnimatedSystem(_AnimatedModelHandle);
		_AnimatedModelHandle.Valid = false;
	}	
	_ParticleSystem = NULL; // one less ref with the smart ptr
	_Invalidated = true;

	nlassert(_Scene);
	_Scene->getParticleSystemManager().removeSystemModel(_ModelHandle);

	static std::vector<IPSModelObserver *> copyVect;
	copyVect.resize(_Observers.size());
	std::copy(_Observers.begin(), _Observers.end(), copyVect.begin());
	
	for (std::vector<IPSModelObserver *>::iterator it = copyVect.begin(); it != copyVect.end(); ++it)
	{
		(*it)->invalidPS(this); // if this crash, then you forgot to call removePSModelObserver !
	}
}

///=====================================================================================
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

///=====================================================================================
const char *CParticleSystemModel::getPSParamName (uint valueId)
{	
	nlassert(valueId < AnimValueLast);
	const char *name[] = { "PSParam0", "PSParam1", "PSParam2", "PSParam3" };	
	return name[valueId - (uint) PSParam0];
}

///=====================================================================================
const char *CParticleSystemModel::getValueName (uint valueId) const 
{ 
	nlassert(valueId < AnimValueLast);
	if (valueId < OwnerBit) return CTransformShape::getValueName(valueId);
	if (valueId < PSTrigger) return getPSParamName(valueId); 
	return "PSTrigger";
}

///=====================================================================================
ITrack* CParticleSystemModel::getDefaultTrack (uint valueId)
{
	nlassert(valueId < AnimValueLast);
	nlassert(Shape);

	CParticleSystemShape *pss = NLMISC::safe_cast<CParticleSystemShape *>((IShape *) Shape);

	switch (valueId)
	{
		case PosValue:			return pss->getDefaultPos();		
		case RotQuatValue:		return pss->getDefaultRotQuat();
		case ScaleValue:		return pss->getDefaultScale();		
	}
	if (valueId < OwnerBit) return CTransformShape::getDefaultTrack(valueId); // delegate to parent

	// this value belong to us
	if (valueId < PSTrigger) 
	{
		return pss->getUserParamDefaultTrack(valueId - (uint) PSParam0);
	}
	return pss->getDefaultTriggerTrack();
}

///=====================================================================================
	void CParticleSystemModel::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix /* =std::string() */)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix);
	addValue(chanMixer, PSParam0, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam1, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam2, OwnerBit, prefix, true);
	addValue(chanMixer, PSParam3, OwnerBit, prefix, true);	
	addValue(chanMixer, PSTrigger, OwnerBit, prefix, true);	
}


///=====================================================================================
float CParticleSystemModel::getNumTriangles (float distance)
{
	if (!_ParticleSystem) return 0;
	return (float) _ParticleSystem->getWantedNumTris(distance);
}

///=========================================================================================
bool CParticleSystemModel::checkAgainstPyramid(const std::vector<CPlane>	&pyramid) const
{		
	nlassert(_ParticleSystem)
	NLMISC::CAABBox bbox;
	_ParticleSystem->computeBBox(bbox);		
	const CMatrix		&mat = getMatrix();
	
	// Transform the pyramid in Object space.	
	for(sint i=0; i < (sint) pyramid.size(); i++)
	{	
		// test wether the bbox is entirely in the neg side of the plane
		if (!bbox.clipBack(pyramid[i]  * mat  )) 
		{
			return false;			
		}
	}
	return true;

}

//////////////////////////////////////////////
// CParticleSystemDetailObs implementation  //
//////////////////////////////////////////////

///=====================================================================================
void	CParticleSystemDetailObs::traverse(IObs *caller)
{    
	CTransformAnimDetailObs::traverse(caller);	
	CParticleSystemModel *psm= NLMISC::safe_cast<CParticleSystemModel *>(Model);
	CParticleSystem *ps = psm->getPS();
	if (psm->_Invalidated) return;


	
	if (!psm->_EditionMode && !psm->_InClusterAndVisible)
	{
		CParticleSystemShape		*pss = NLMISC::safe_cast<CParticleSystemShape *>((IShape *)psm->Shape);
		if (pss->_DestroyWhenOutOfFrustum)
		{
			if (pss->_DestroyModelWhenOutOfRange)
			{
				psm->releaseRscAndInvalidate();			
			}
			else // remove rsc but do not invalidate the system
			{
				psm->releaseRsc();
			}
			return;
		}
		if (!ps) return;
	}	

	// check for trigger. If the trigger is false, and there is a system instanciated, we delete it.
	if (!psm->_EditionMode)
	{
		if (!psm->_TriggerAnimatedValue.Value)
		{									
			// system is off, or hasn't been instanciated now...
			if (ps)
			{
				psm->releaseRsc();				
			}
			return;
		}
	}

	// the system or its center is in the view frustum, but it may not have been instanciated from its shape now
	if (!ps)
	{
		nlassert(psm->_Scene);
		nlassert(psm->Shape);
		psm->reallocRsc();
		ps = psm->_ParticleSystem;
	}

	CClipTrav			*trav= (CClipTrav*) ClipObs->Trav;
	
	if (psm->_InClusterAndVisible ||  ps->getAnimType() == CParticleSystem::AnimInCluster)
	{		
		bool animate = true;
		if (ps->isSharingEnabled()) /// with shared system, we only animate one version!
		{
			if (ps->_LastUpdateDate != trav->CurrentDate)
			{
				ps->_LastUpdateDate = trav->CurrentDate;
			}
			else
			{
				animate = false;
			}
		}


		if (animate)
		{
			const CMatrix		&mat= HrcObs->WorldMatrix;	 
			ps->setSysMat(mat);
			ps->setViewMat(trav->ViewMatrix);				

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

			if (ps->getAnimType() != CParticleSystem::AnimAlways) // if the animation is always perfomed, then its done by the particle system manager
			{
				if (psm->isAutoGetEllapsedTimeEnabled())
				{
					psm->setEllapsedTime(ps->getScene()->getEllapsedTime() * psm->getEllapsedTimeRatio());
				}
				TAnimationTime delay = psm->getEllapsedTime();
				// animate particles				
				ps->step(CParticleSystem::Anim, delay);					
			}
		}
	}	

	// add a render obs if in cluster
	if (psm->_InClusterAndVisible)
	{
		trav->RenderTrav->addRenderObs(ClipObs->RenderObs);
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
 * IMPORTANT : the Visible attribute is interpreted as 'in traversed clusters'. We need this because we want
 * to know when a p.s is in clusters, but not visible. As a matter of fact we may need to have system that are animated
 * as long as in cluster, but not visible. 
 */
	
void	CParticleSystemClipObs::traverse(IObs *caller)
{
	H_AUTO ( NL3D_Particles_Clip );

//	    CTransformClipObs::traverse(caller);
		traverseSons();
		nlassert(!caller || dynamic_cast<IBaseClipObs*>(caller));
		CParticleSystemModel		*m= (CParticleSystemModel*)Model;	
		if (m->_Invalidated) return;		
		CClipTrav			*trav= (CClipTrav*)Trav;
		

		if (Date != trav->CurrentDate) 
		{
			m->_InsertedInVisibleList = false;
			m->_InClusterAndVisible = false;
			Date = trav->CurrentDate;
		}
		if (m->_InClusterAndVisible) return; // already visible
		

		CParticleSystem *ps = m->_ParticleSystem;

		
		if (ps) // system instanciated
		{
			// if there are no more particles, no need to even clip..
			if (checkDestroyCondition(ps, m)) return;
		}
		
		// special case : system sticked to a skeleton
		if( ((CTransformHrcObs*)HrcObs)->_AncestorSkeletonModel!=NULL )
		{
			bool visible = ((CTransformHrcObs*)HrcObs)->_AncestorSkeletonModel->isClipVisible();
			// Special test: if we are sticked to a skeletonModel, and if we are still visible, maybe we don't have to
			if(Visible && m->_FatherSkeletonModel)
			{
				CClipTrav		*clipTrav= NLMISC::safe_cast<CClipTrav*>(Trav);
				// must ensure the skeleton has computed the state
				m->_FatherSkeletonModel->updateDisplayLodCharacterFlag(clipTrav);

				// if our skeletonModel father is displayed with a Lod, maybe we are not to be displayed
				if(m->_FatherSkeletonModel->isDisplayedAsLodCharacter())
				{
					// We are visible only if we where sticked to the skeleton with forceCLod==true.
					// This is also true if we are actually a skeletonModel
					if(!m->getShowWhenLODSticked())
						// otherWise we are not visible. eg: this is the case of skins and some sticked object
						visible = false;
				}
			}
			//
			if (visible)
			{				
				Visible = true;
				insertInVisibleList();				
				m->_InClusterAndVisible = true;
				return;
			}
			else // not visible, may need animation however..
			{				
				if (!ps) // no resc allocated
				{
					CParticleSystemShape		*pss= NLMISC::safe_cast<CParticleSystemShape *>((IShape *)m->Shape);
					nlassert(pss);
					// invalidate the system if too far
					const CVector pos = m->getMatrix().getPos();		
					const CVector d = pos - trav->CamPos;
					if (d * d > pss->_MaxViewDist * pss->_MaxViewDist) 
					{
						Visible = false;					
						if (pss->_DestroyModelWhenOutOfRange)
						{
							m->_Invalidated = true;					
						}													
					}
				}
				else
				{				
					// NB : The test to see wether the system is not too far is performed by the particle system manager					
					if (!m->_EditionMode)
					{
						Visible = true;     // system near, but maybe not in cluster..
						insertInVisibleList();
					}					
				}
			}
			return;				
		}


		//
		const std::vector<CPlane>	&pyramid= trav->WorldPyramid;	
		/** traverse the sons
		  * we must do this before us, because this object may delete himself from the scene
		  */		

		// now the pyramid is directly expressed in the world
		const CMatrix		&mat= HrcObs->WorldMatrix;	 

				
		// Transform the pyramid in Object space.

		
		if(!ps) ///====================== system resource not allocated, test if it entered the scope
		{
			CParticleSystemShape		*pss= NLMISC::safe_cast<CParticleSystemShape *>((IShape *)m->Shape);
			nlassert(pss);

			// the system wasn't present the last time, we use its center to see if it's back in the view frustum,
			// or if it is near enough.
			// if this is the case, we say it isn't clipped, so it will be reinstanciated from the shape
			// during the DetailAnimTraversal

			const CVector pos = m->getMatrix().getPos();
		
			const CVector d = pos - trav->CamPos;


			// check wether system not too far		
			if (d * d > pss->_MaxViewDist * pss->_MaxViewDist) 
			{
				Visible = false;					
				if (pss->_DestroyModelWhenOutOfRange)
				{
					m->_Invalidated = true;					
				}							
				return;
			}		

			// test the shape to see wether we have a precomputed bbox
			if (!pss->_UsePrecomputedBBox)
			{								
				///============================= the system has no precomputed bbox			
				/// frustum test		
				for(sint i=0; i < (sint)pyramid.size(); i++)
				{					
					if ( (pyramid[i]   *  mat  ).d > 0.0f )  // in its basis, the system is at the center

					{						
						Visible = true;
						insertInVisibleList();
						return;
					}
				}	
				
				Visible = true;
				insertInVisibleList();				
				m->_InClusterAndVisible = true;
				return;						
			}
			else
			{
				///============================= the system has a precomputed bbox			
				/// frustum test		
				for(sint i=0; i < (sint)pyramid.size(); i++)
				{					
					if ( !pss->_PrecomputedBBox.clipBack(pyramid[i]  * mat  ) ) 
					{
						
						Visible = true;
						insertInVisibleList();
						return;					
					}
				}			

				Visible = true;
				insertInVisibleList();
				m->_InClusterAndVisible = true;
				return;
				
			}
		}
		
		//=========================================================================================================
		// the system is already instanciated
		
		nlassert(ps);						
		/// Pyramid test. IMPORTANT : The test to see wether the system is not too far is performed by the particle system manager
		if (m->checkAgainstPyramid(pyramid) == false)
		{
			if (!m->_EditionMode)
			{
				// system near, but maybe not in cluster..	
				Visible = true;
				insertInVisibleList();
			}				
			return;
		}
		

		Visible = true;
		insertInVisibleList();
		m->_InClusterAndVisible = true;
}


//===================================================================
bool CParticleSystemClipObs::checkDestroyCondition(CParticleSystem *ps, CParticleSystemModel *m)
{
	nlassert(ps && m);
	if (!m->_EditionMode)
	{
		/** NB : we don't do this test here for always animated system, as it is done 
		 * by the CParticleSystemManager, because this code is not sure to be executed if the system has been clipped by a cluster
		 */
		if (ps->getAnimType() != CParticleSystem::AnimAlways)
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
								m->releaseRscAndInvalidate();
								return true;
							}
							break;
						case CParticleSystem::noMoreParticlesAndEmitters:
							if (!ps->hasParticles() && !ps->hasEmitters())
							{
								m->releaseRscAndInvalidate();
								return true;
							}
							break;
						default: break;
					}
				}
			}
		}
	}
	return false;
}


} // NL3D
