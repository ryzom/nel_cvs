/** \file particle_system_model.cpp
 * <File description>
 *
 * $Id: particle_system_model.cpp,v 1.53 2003/05/07 09:48:25 vizerie Exp $
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
											   _InClusterAndVisible(false),
											   _BypassGlobalUserParam(0),
											   _AnimType(CParticleSystem::AnimVisible)
{
	setOpacity(false);
	setTransparency(true);
	IAnimatable::resize(AnimValueLast);
	_TriggerAnimatedValue.Value = true;

	// AnimDetail behavior: Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);

	// RenderFilter: We are a Landscape
	_RenderFilterType= UScene::FilterPS;
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
	// register the model
	CScene::registerModel(ParticleSystemModelId, TransformShapeId, CParticleSystemModel::creator);	
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
	releaseRsc();
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// clip and hrc hierarchy is modified.
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
	_AnimType = _ParticleSystem->getAnimType();
	if (_ParticleSystem->getAnimType() == CParticleSystem::AnimAlways)
	{
		_AnimatedModelHandle = psmgt.addPermanentlyAnimatedSystem(this);
	}
	// touch user params animated value. If the system rsc have been released before, this force to restore them
	for (uint k = 0; k < MaxPSUserParam; ++k)
	{		
		touch((uint)CParticleSystemModel::PSParam0 + k, OwnerBit);
	}
}

///=====================================================================================
void CParticleSystemModel::releasePSPointer()
{
	if (_ParticleSystem.getNbRef() == 1)
	{	
		// Backup user params (in animated value) so that they will be restored when the system is recreated
		for (uint k = 0; k < MaxPSUserParam; ++k)
		{
			_UserParam[k].Value = _ParticleSystem->getUserParam(k);					
		}
	}
	//
	_ParticleSystem = NULL; // one less ref with the smart ptr 
}

///=====================================================================================
bool CParticleSystemModel::refreshRscDeletion(const std::vector<CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos)
{
	if (_EditionMode) return false;
	/** Here we test wether the system has not gone out of scope.
	  * Why do we test this here addtionnaly to the clip traversal ?
	  * Simply because the clip traversal is not called if the cluster it is inserted in is not parsed.
	  * This is not good, because we want to keep few CParticleSystem instance.
	  * This method solve that problem. This is called by the particle system manager when each scene has rendered
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
		releasePSPointer();
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
			releasePSPointer();
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
	nlassert(_Scene);
	_Scene->getParticleSystemManager().removeSystemModel(_ModelHandle);
	releasePSPointer();
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
	releasePSPointer();
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
	nlassert(_ParticleSystem);
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
// CParticleSystem AnimDetail implementation  //
//////////////////////////////////////////////

///=====================================================================================
void	CParticleSystemModel::traverseAnimDetail()
{    
	CTransformShape::traverseAnimDetail();	
	CParticleSystem *ps = getPS();
	if (!_WorldVis) return;
	if (_Invalidated) return;
	if (getVisibility() == CHrcTrav::Hide) return;
	
	
	if (!_EditionMode && !_InClusterAndVisible)
	{
		CParticleSystemShape		*pss = NLMISC::safe_cast<CParticleSystemShape *>((IShape *)Shape);
		if (pss->_DestroyWhenOutOfFrustum)
		{
			if (pss->_DestroyModelWhenOutOfRange)
			{
				releaseRscAndInvalidate();			
			}
			else // remove rsc but do not invalidate the system
			{
				releaseRsc();
			}
			return;
		}
		if (!ps) return;
	}	

	// check for trigger. If the trigger is false, and there is a system instanciated, we delete it.
	if (!_EditionMode)
	{
		if (!_TriggerAnimatedValue.Value)
		{									
			// system is off, or hasn't been instanciated now...
			if (ps)
			{
				releaseRsc();				
			}
			return;
		}
	}

	// the system or its center is in the view frustum, but it may not have been instanciated from its shape now
	if (!ps)
	{
		nlassert(_Scene);
		nlassert(Shape);
		reallocRsc();
		ps = _ParticleSystem;
	}

	CClipTrav			&clipTrav= getOwnerScene()->getClipTrav();
	
	if (_InClusterAndVisible ||  ps->getAnimType() == CParticleSystem::AnimInCluster)
	{		
		bool animate = true;
		if (ps->isSharingEnabled()) /// with shared system, we only animate one version!
		{
			if (ps->_LastUpdateDate == clipTrav.CurrentDate)
			{
				animate = false;				
			}
			else
			{
				ps->_LastUpdateDate = clipTrav.CurrentDate;
			}
		}
		else
		{
			ps->_LastUpdateDate = clipTrav.CurrentDate;
		}	
		ps->_LastUpdateDate = clipTrav.CurrentDate;
		if (animate)
		{
			const CMatrix		&mat= getWorldMatrix();	 
			ps->setSysMat(mat);
			ps->setViewMat(clipTrav.ViewMatrix);
			updateOpacityInfos();

			//ps->setSysMat(getWorldMatrix());
			nlassert(ps->getScene());	


			// setup the number of faces we allow
			ps->setNumTris((uint) getNumTrianglesAfterLoadBalancing());


			// set the global user param that are bypassed
			nlctassert(MaxPSUserParam < 8); // there should be less than 8 parameters because of mask stored in a byte
			ps->_BypassGlobalUserParam = _BypassGlobalUserParam;

			// setup system user parameters for parameters that have been touched
			for (uint k = 0; k < MaxPSUserParam; ++k)
			{
				if (isTouched((uint)CParticleSystemModel::PSParam0 + k))
				{
					ps->setUserParam(k, _UserParam[k].Value);
					clearFlag((uint)CParticleSystemModel::PSParam0 + k);
				}
			}

			if (ps->getAnimType() != CParticleSystem::AnimAlways) // if the animation is always perfomed, then its done by the particle system manager
			{
				if (isAutoGetEllapsedTimeEnabled())
				{
					setEllapsedTime(ps->getScene()->getEllapsedTime() * getEllapsedTimeRatio());
				}
				TAnimationTime delay = getEllapsedTime();
				// animate particles				
				ps->step(CParticleSystem::Anim, delay);					
			}
		}
	}		
	

	// add a render model if in cluster & not hidden
	if (_InClusterAndVisible)
	{
		getOwnerScene()->getRenderTrav().addRenderModel(this);
	}
}



//////////////////////////////////////////////
// CParticleSystem Render implementation  //
//////////////////////////////////////////////
void	CParticleSystemModel::traverseRender()
{
/*
	if (!_OutOfFrustum)
	{*/
		CTransformShape::traverseRender();
	//}
}


/*
 * CParticleSystem Clip implementation              
 * IMPORTANT : the _Visible attribute is interpreted as 'in traversed clusters'. We need this because we want
 * to know when a p.s is in clusters, but not visible. As a matter of fact we may need to have system that are animated
 * as long as in cluster, but not visible. 
 */
	
void	CParticleSystemModel::traverseClip()
{
	// disable H_AUTO, because slowdown when lot of models (eg 1000-2000 tested in forest)
	//H_AUTO ( NL3D_Particles_Clip );

//	    CTransformShape::traverseClip();
		// Traverse the Clip sons.
		uint	numClipChildren= clipGetNumChildren();
		for(uint i=0;i<numClipChildren;i++)
			clipGetChild(i)->traverseClip();

		if (!_WorldVis) return;
		if (_Invalidated) return;		
		CClipTrav			&clipTrav= getOwnerScene()->getClipTrav();
		

		if (_ClipDate != clipTrav.CurrentDate) 
		{
			_InsertedInVisibleList = false;
			_InClusterAndVisible = false;
			_ClipDate = clipTrav.CurrentDate;
		}
		if (_InClusterAndVisible) return; // already visible
		

		CParticleSystem *ps = _ParticleSystem;

		
		if (ps) // system instanciated
		{
			// if there are no more particles, no need to even clip..
			if (checkDestroyCondition(ps)) return;
			// check for anim mode change
			if (_AnimType != ps->getAnimType())
			{
				CParticleSystemManager &psmgt = _Scene->getParticleSystemManager();
				if (_AnimType == CParticleSystem::AnimAlways) // was previously always animated ?
				{
					if (_AnimatedModelHandle.Valid)
					{					
						psmgt.removePermanentlyAnimatedSystem(_AnimatedModelHandle);
					}
				}
				_AnimType = ps->getAnimType();
				if (_AnimType == CParticleSystem::AnimAlways)
				{
					_AnimatedModelHandle = 	psmgt.addPermanentlyAnimatedSystem(this);
				}
			}
		}
		
		// check wether display filtered or not
		if( !(_Scene->getFilterRenderFlags() & _RenderFilterType) )
		{
			_Visible = false;					
			return;
		}

		// special case : system sticked to a skeleton
		if( _AncestorSkeletonModel!=NULL )
		{
			bool visible = _AncestorSkeletonModel->isClipVisible();
			// Special test: if we are sticked to a skeletonModel, and if we are still visible, maybe we don't have to
			if(_Visible && _FatherSkeletonModel)
			{
				// if our skeletonModel father is displayed with a Lod, maybe we are not to be displayed
				if(_FatherSkeletonModel->isDisplayedAsLodCharacter())
				{
					// We are visible only if we where sticked to the skeleton with forceCLod==true.
					// This is also true if we are actually a skeletonModel
					if(!getShowWhenLODSticked())
						// otherWise we are not visible. eg: this is the case of skins and some sticked object
						visible = false;
				}
			}
			//
			if (visible)
			{				
				_Visible = true;
				insertInVisibleList();				
				_InClusterAndVisible = true;
				return;
			}
			else // not visible, may need animation however..
			{				
				if (!ps) // no resc allocated
				{
					CParticleSystemShape		*pss= NLMISC::safe_cast<CParticleSystemShape *>((IShape *)Shape);
					nlassert(pss);
					// invalidate the system if too far
					const CVector pos = getWorldMatrix().getPos();
					const CVector d = pos - clipTrav.CamPos;
					if (d * d > pss->_MaxViewDist * pss->_MaxViewDist) 
					{
						_Visible = false;					
						if (pss->_DestroyModelWhenOutOfRange)
						{
							_Invalidated = true;					
						}													
					}
				}
				else
				{				
					// NB : The test to see wether the system is not too far is performed by the particle system manager					
					if (!_EditionMode)
					{
						_Visible = true;     // system near, but maybe not in cluster..
						insertInVisibleList();
					}					
				}
			}
			return;				
		}


		//
		const std::vector<CPlane>	&pyramid= clipTrav.WorldPyramid;	
		/** traverse the sons
		  * we must do this before us, because this object may delete himself from the scene
		  */		

		// now the pyramid is directly expressed in the world
		const CMatrix		&mat= getWorldMatrix();	 

				
		// Transform the pyramid in Object space.

		
		if(!ps) ///====================== system resource not allocated, test if it entered the scope
		{
			CParticleSystemShape		*pss= NLMISC::safe_cast<CParticleSystemShape *>((IShape *)Shape);
			nlassert(pss);

			// the system wasn't present the last time, we use its center to see if it's back in the view frustum,
			// or if it is near enough.
			// if this is the case, we say it isn't clipped, so it will be reinstanciated from the shape
			// during the DetailAnimTraversal

			const CVector pos = getWorldMatrix().getPos();
		
			const CVector d = pos - clipTrav.CamPos;


			// check wether system not too far		
			if (d * d > pss->_MaxViewDist * pss->_MaxViewDist) 
			{
				_Visible = false;					
				if (pss->_DestroyModelWhenOutOfRange)
				{
					_Invalidated = true;					
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
						_Visible = true;
						insertInVisibleList();
						return;
					}
				}	
				
				_Visible = true;
				insertInVisibleList();				
				_InClusterAndVisible = true;
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
						
						_Visible = true;
						insertInVisibleList();
						return;					
					}
				}			

				_Visible = true;
				insertInVisibleList();
				_InClusterAndVisible = true;
				return;
				
			}
		}
		
		//=========================================================================================================
		// the system is already instanciated
		
		nlassert(ps);						
		/// Pyramid test. IMPORTANT : The test to see wether the system is not too far is performed by the particle system manager
		// In edition mode, it isn't done by the manager (system never removed), so we do it here in this case
		if (_EditionMode)
		{
			CParticleSystemShape		*pss= NLMISC::safe_cast<CParticleSystemShape *>((IShape *)Shape);
			nlassert(pss);
			const CVector pos = getWorldMatrix().getPos();
			const CVector d = pos - clipTrav.CamPos;
			// check wether system not too far		
			if (d * d > ps->getMaxViewDist() * ps->getMaxViewDist()) 
			{
				return; // not visible
			}
		}
		if (checkAgainstPyramid(pyramid) == false)
		{
			if (!_EditionMode)
			{
				// system near, but maybe not in cluster..	
				_Visible = true;
				insertInVisibleList();
			}				
			return;
		}
		

		_Visible = true;
		insertInVisibleList();
		_InClusterAndVisible = true;
}


//===================================================================
bool	CParticleSystemModel::clip()
{
	// no-op clip() because all done in special traverse()
	return true;
}


//===================================================================
bool	CParticleSystemModel::checkDestroyCondition(CParticleSystem *ps)
{
	nlassert(ps);
	if (!_EditionMode)
	{
		/** NB : we don't do this test here for always animated system, as it is done 
		 * by the CParticleSystemManager, because this code is not sure to be executed if the system has been clipped by a cluster
		 */
		if (ps->getAnimType() != CParticleSystem::AnimAlways)
		{
			if (ps->isDestroyConditionVerified())
			{
				releaseRscAndInvalidate();
				return true;
			}			
		}
	}
	return false;
}

//===================================================================
void CParticleSystemModel::bypassGlobalUserParamValue(uint userParamIndex,bool byPass /*=true*/)
{
	nlctassert(MaxPSUserParam < 8); // there should be less than 8 parameters because of mask stored in a byte
	nlassert(userParamIndex < MaxPSUserParam);
	if (byPass) _BypassGlobalUserParam |= (1 << userParamIndex);
	else _BypassGlobalUserParam &= ~(1 << userParamIndex);
}

//===================================================================
bool CParticleSystemModel::isGlobalUserParamValueBypassed(uint userParamIndex) const
{
	nlctassert(MaxPSUserParam < 8); // there should be less than 8 parameters because of mask stored in a byte
	nlassert(userParamIndex < MaxPSUserParam);
	return (_BypassGlobalUserParam & (1 << userParamIndex)) != 0;
}

//===================================================================
void CParticleSystemModel::enableDisplayTools(bool enable /*=true*/)
{	 
	_ToolDisplayEnabled = enable; touchTransparencyState(); 	
}

//===================================================================
void CParticleSystemModel::invalidateAutoAnimatedHandle()
{
	_AnimatedModelHandle.Valid  = false;
}


} // NL3D
