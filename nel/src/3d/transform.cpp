/** \file transform.cpp
 * <File description>
 *
 * $Id: transform.cpp,v 1.55 2003/03/13 15:25:57 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "3d/transform.h"
#include "3d/skeleton_model.h"
#include "3d/skip_model.h"
#include "3d/scene.h"
#include "3d/root_model.h"
#include "3d/fast_floor.h"


using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
void	CTransform::registerBasic()
{
	CMOT::registerModel( TransformId, 0, CTransform::creator);
	CMOT::registerObs( HrcTravId,			TransformId, CTransformHrcObs::creator			);
	CMOT::registerObs( ClipTravId,			TransformId, CTransformClipObs::creator			);
	CMOT::registerObs( AnimDetailTravId,	TransformId, CTransformAnimDetailObs::creator	);
	CMOT::registerObs( LightTravId,			TransformId, CTransformLightObs::creator		);
	CMOT::registerObs( RenderTravId,		TransformId, CTransformRenderObs::creator		);
}


// ***************************************************************************
CTransform::CTransform()
{
	_HrcObs= NULL;
	_ClipObs= NULL;
	_LightObs=NULL;

	TouchObs.resize(Last);

	Visibility= CHrcTrav::Herit;

	_LastTransformableMatrixDate= 0;

	_FatherSkeletonModel= NULL;

	_ClusterSystem = NULL;

	_FreezeHRCState= FreezeHRCStateDisabled;

	_OrderingLayer = 2;

	
	// No logicInfo by default
	_LogicInfo= NULL;

	_ForceCLodSticked= false;

	// default MeanColor value
	_MeanColor.set(255,255,255,255);


	// Setup some state.

	/*
		Default are:
			IsAnimDetailable= 0
			IsLoadBalancable= 0
			IsLightable= 0
			IsRenderable= 0
			IsTransparent= 0
			IsOpaque= 1
			QuadGridClipEnabled= 0.

			IsUserLightable= 1			// default, the model may be lighted.
			IsFinalLightable= 0
			IsNeedUpdateLighting= 0
			ISNeedUpdateFrozenStaticLightSetup= 0

			IsSkeleton= 0

			IsDeleteChannelMixer = 0;
	*/
	_StateFlags= IsOpaque | IsUserLightable;

	// By default, always allow rendering of Transform Models.
	_RenderFilterType= ~0;
}


// ***************************************************************************
void	CTransform::initModel()
{
	IModel::initModel();

	_HrcObs= (CTransformHrcObs*)getObs(HrcTravId);
	_ClipObs= (CTransformClipObs*)getObs(ClipTravId);
	_LightObs= (CTransformLightObs*)getObs(LightTravId);
}


// ***************************************************************************
CTransform::~CTransform()
{
	// If still binded to a father skeleton
	if( _FatherSkeletonModel )
	{
		/* If skinned, cannot detach me from skeleton here because detachSkeletonSon()
			use some virtual calls of transform: setApplySkin().
			Hence, It is the deriver job to detach himself from the skeleton.

			NB: test isSkinned(), not isSkinnable(), since isSkinned() is not virtual ....
			This means that if a Mesh isSkinnable(), but never skinned, it is not asserted here.
		*/
		if( isSkinned() )
		{
			nlstop;
		}
		else
			// Can detach Me. Important for UTransform sticked
			_FatherSkeletonModel->detachSkeletonSon(this);
	}

	// resetLighting, removing me from PointLight Transform list.
	// NB: not done for FrozenStaticLightSetup, because those lights don't owns me.
	resetLighting();

	// Must also remove me from the lightingManager.
	// must test _LightObs because of CCluster usage out of mot (thanks to mat!! :) )
	if(_LightObs)
	{
		CLightTrav	*lightTrav= (CLightTrav*)_LightObs->Trav;
		_LightedModelIt= lightTrav->LightingManager.eraseStaticLightedModel(_LightedModelIt);
	}

	if (getChannelMixerOwnerShip()) delete (CChannelMixer *) _ChannelMixer;
}

// ***************************************************************************
void		CTransform::hide()
{
	// Optim: do nothing if already set (=> not foulTransform() -ed).
	if(Visibility!= CHrcTrav::Hide)
	{
		foulTransform();
		Visibility= CHrcTrav::Hide;
		// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}
// ***************************************************************************
void		CTransform::show()
{
	// Optim: do nothing if already set (=> not foulTransform() -ed).
	if(Visibility!= CHrcTrav::Show)
	{
		foulTransform();
		Visibility= CHrcTrav::Show;
		// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}
// ***************************************************************************
void		CTransform::heritVisibility()
{
	// Optim: do nothing if already set (=> not foulTransform() -ed).
	if(Visibility!= CHrcTrav::Herit)
	{
		foulTransform();
		Visibility= CHrcTrav::Herit;
		// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
		if(isSkinned())
		{
			nlassert(_FatherSkeletonModel);
			_FatherSkeletonModel->dirtSkinRenderLists();
		}
	}
}


// ***************************************************************************
CTrackDefaultVector		CTransform::DefaultPos( CVector::Null );
CTrackDefaultVector		CTransform::DefaultRotEuler( CVector::Null );
CTrackDefaultQuat		CTransform::DefaultRotQuat( NLMISC::CQuat::Identity );
CTrackDefaultVector		CTransform::DefaultScale( CVector(1,1,1) );
CTrackDefaultVector		CTransform::DefaultPivot( CVector::Null );

ITrack* CTransform::getDefaultTrack (uint valueId)
{
	// Cyril: prefer do it here in CTransform, because of CCamera, CLight etc... (which may not need a default value too!!)

	// what value ?
	switch (valueId)
	{
	case PosValue:			return &DefaultPos;
	case RotEulerValue:		return &DefaultRotEuler;
	case RotQuatValue:		return &DefaultRotQuat;
	case ScaleValue:		return &DefaultScale;
	case PivotValue:		return &DefaultPivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getDefaultTrack(valueId);

	return NULL;

}

// ***************************************************************************
void	CTransform::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	if (getChannelMixerOwnerShip() && chanMixer != _ChannelMixer)
	{
		delete _ChannelMixer;
		setChannelMixerOwnerShip(false);
	}

	// Hey!! we are animated!!
	_ChannelMixer= chanMixer;

	// Update flag, if we must be inserted in AnimDetail
	setStateFlag(IsAnimDetailable, _ChannelMixer || getStateFlag(IsForceAnimDetail) );

	// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
	if(isSkinned())
	{
		nlassert(_FatherSkeletonModel);
		_FatherSkeletonModel->dirtSkinRenderLists();
	}

	// For CTransfom, channels are not detailled.
	addValue(chanMixer, PosValue, OwnerBit, prefix, false);
	addValue(chanMixer, RotEulerValue, OwnerBit, prefix, false);
	addValue(chanMixer, RotQuatValue, OwnerBit, prefix, false);
	addValue(chanMixer, ScaleValue, OwnerBit, prefix, false);
	addValue(chanMixer, PivotValue, OwnerBit, prefix, false);

	// Deriver note: if necessary, call	BaseClass::registerToChannelMixer(chanMixer, prefix);
}



// ***************************************************************************
void			CTransform::updateWorldMatrixFromFather()
{
	// If I am not skinned
	if(!isSkinned())
	{
		// Compute the HRC WorldMatrix.
		// if I am not sticked.
		if(!_FatherSkeletonModel)
		{
			// get the normal father worldMatrix in Hrc.
			CTransform	*fatherTransform= dynamic_cast<CTransform*>(_HrcObs->Trav->getFirstParent(this));
			// if exist
			if(fatherTransform)
			{
				const CMatrix &parentWM= fatherTransform->_HrcObs->WorldMatrix;
				// combine worldMatrix
				_HrcObs->WorldMatrix= parentWM * _HrcObs->LocalMatrix;
			}
			else
				_HrcObs->WorldMatrix= _HrcObs->LocalMatrix;
		}
		else
		{
			// get the worldMatrix of the bone if I am sticked.
			const CMatrix &parentWM= _FatherSkeletonModel->Bones[_FatherBoneId].getWorldMatrix();
			// combine worldMatrix
			_HrcObs->WorldMatrix= parentWM * _HrcObs->LocalMatrix;
		}
	}
}


// ***************************************************************************
void			CTransform::freeze()
{
	// First, validate the model, and all his observers.
	// Frozen state is disabled here (in CTransformHrcObs::update()).
	validate();

	// Then flag the frozen state.
	_HrcObs->Frozen= true;
}

// ***************************************************************************
void			CTransform::setDontUnfreezeChildren(bool val)
{
	_HrcObs->DontUnfreezeChildren = val;
}


// ***************************************************************************
const CMatrix& CTransform::getWorldMatrix()
{
	return _HrcObs->WorldMatrix;
}

// ***************************************************************************
bool	CTransform::getLastClippedState() const
{
	return _ClipObs->Visible;
}



// ***************************************************************************
void		CTransform::freezeHRC()
{
	// if disabled, say we are ready to validate our worldMatrix for long.
	if(_FreezeHRCState==FreezeHRCStateDisabled)
	{
		_FreezeHRCState= FreezeHRCStateRequest;
		setStateFlag(QuadGridClipEnabled, true);
	}
}


// ***************************************************************************
void		CTransform::unfreezeHRC()
{
	// if this model is no HRC frozen disabled
	if(_FreezeHRCState!=FreezeHRCStateDisabled)
	{
		// if model correctly frozen.
		if(_FreezeHRCState == CTransform::FreezeHRCStateEnabled )
		{
			// Trick: get the traversal via the HrcObs.
			CHrcTrav	*hrcTrav= static_cast<CHrcTrav*>(_HrcObs->Trav);
			// if linked to SkipModelRoot, link this model to root of HRC.
			if( hrcTrav->getFirstParent(this) == hrcTrav->Scene->getSkipModelRoot() )
				hrcTrav->link(NULL, this);

			// Link this object to the validateList.
			linkToValidateList();

			// if lightable()
			if( isLightable() )
			{
				CLightTrav	*lightTrav= static_cast<CLightTrav*>(_LightObs->Trav);
				// Lighting: must remove the object from the quadGrid.
				// NB: works if _LightedModelIt==NULL. result is that _LightedModelIt= NULL.
				_LightedModelIt= lightTrav->LightingManager.eraseStaticLightedModel(_LightedModelIt);
			}

		}

		_FreezeHRCState= FreezeHRCStateDisabled;
		setStateFlag(QuadGridClipEnabled, false);
	}
}


// ***************************************************************************
void		CTransform::update()
{
	IModel::update();

	// test if the matrix has been changed in ITransformable.
	if(ITransformable::compareMatrixDate(_LastTransformableMatrixDate))
	{
		_LastTransformableMatrixDate= ITransformable::getMatrixDate();
		foul(TransformDirty);
	}

	// update the freezeHRC state.
	if(_FreezeHRCState != CTransform::FreezeHRCStateDisabled)
	{
		// if the model request to be frozen in HRC
		if(_FreezeHRCState == CTransform::FreezeHRCStateRequest )
		{
			// Wait for next Hrc traversal to compute good WorldMatrix for this model and his sons.
			// Also, next Hrc traversal will insert the model in the LightingManager quadGrid (if lightable)
			_FreezeHRCState = CTransform::FreezeHRCStateReady;
		}
		// if the model is ready to be frozen in HRC, then do it!!
		else if( _FreezeHRCState == CTransform::FreezeHRCStateReady )
		{
			// Trick: get the traversal via the HrcObs.
			CHrcTrav	*hrcTrav= static_cast<CHrcTrav*>(_HrcObs->Trav);
			// if linked to root of HRC, link this model to SkipModelRoot.
			if( hrcTrav->getFirstParent(this) == hrcTrav->getRoot() )
				hrcTrav->link(hrcTrav->Scene->getSkipModelRoot(), this);

			// unLink this object from the validateList. NB: the list will still be correclty parsed.
			unlinkFromValidateList();

			// if lightable, the model is inserted in a quadgrid to update his lighting only when
			// dynamicLights touch him (since himself is static).
			if( isLightable() )
			{
				CLightTrav	*lightTrav= static_cast<CLightTrav*>(_LightObs->Trav);
				// Lighting: must reinsert the object from the quadGrid.
				// NB: works if _LightedModelIt==NULL. result is that _LightedModelIt= NULL.
				_LightedModelIt= lightTrav->LightingManager.eraseStaticLightedModel(_LightedModelIt);
				// insert in the quadgrid.
				_LightedModelIt= lightTrav->LightingManager.insertStaticLightedModel(this);
			}

			// Now this model won't be tested for validation nor for worldMatrix update. End!!
			_FreezeHRCState = CTransform::FreezeHRCStateEnabled;
		}
	}
}


// ***************************************************************************
void	CTransform::getAABBox(NLMISC::CAABBox &bbox) const
{
	bbox.setCenter(CVector::Null);
	bbox.setHalfSize(CVector::Null);
}


// ***************************************************************************
void	CTransform::setLoadBalancingGroup(const std::string &group)
{
	// get the LoadBalancing observer
	IBaseLoadBalancingObs	*obs= (IBaseLoadBalancingObs*)getObs(LoadBalancingTravId);
	nlassert(obs);

	// Get the traversal.
	CLoadBalancingTrav	*trav= (CLoadBalancingTrav*)obs->Trav;
	nlassert(trav);
	// get the group from trav (create if needed), and set it.
	obs->LoadBalancingGroup= trav->getOrCreateGroup(group);
}


// ***************************************************************************
const std::string &CTransform::getLoadBalancingGroup() const
{
	// get the LoadBalancing observer
	IBaseLoadBalancingObs	*obs= (IBaseLoadBalancingObs*)getObs(LoadBalancingTravId);
	nlassert(obs);

	// get the group name
	return obs->LoadBalancingGroup->Name;
}


// ***************************************************************************
void		CTransform::setMeanColor(CRGBA color)
{
	// if the color is different from prec
	if(color!=_MeanColor)
	{
		// change it.
		_MeanColor= color;
		// if skinned or sticked to a skeleton model.
		if(_FatherSkeletonModel)
		{
			// must dirt the vertex color of the lod skeleton because some object color has changed
			_FatherSkeletonModel->dirtLodVertexColor();
		}
	}
}


// ***************************************************************************
void		CTransform::setIsLightable(bool val)
{
	setStateFlag(IsLightable, val);
	// update IsFinalLightable
	setStateFlag(IsFinalLightable, (getStateFlag(IsLightable) && getStateFlag(IsUserLightable)) );
}
// ***************************************************************************
void		CTransform::setUserLightable(bool enable)
{
	setStateFlag(IsUserLightable, enable);
	// update IsFinalLightable
	setStateFlag(IsFinalLightable, (getStateFlag(IsLightable) && getStateFlag(IsUserLightable)) );
}


// ***************************************************************************
void		CTransform::setIsRenderable(bool val)
{
	setStateFlag(IsRenderable, val);
}

// ***************************************************************************
void		CTransform::setIsBigLightable(bool val)
{
	setStateFlag(IsBigLightable, val);
}
// ***************************************************************************
void		CTransform::setIsSkeleton(bool val)
{
	setStateFlag(IsSkeleton, val);
}

// ***************************************************************************
void		CTransform::setApplySkin(bool state)
{
	setStateFlag(IsSkinned, state);
}

// ***************************************************************************
void		CTransform::setIsForceAnimDetail(bool val)
{
	setStateFlag(IsForceAnimDetail, val );

	// Update flag, if we must be inserted in AnimDetail
	setStateFlag(IsAnimDetailable, _ChannelMixer || getStateFlag(IsForceAnimDetail) );

	// If skinned, then must inform skeleton parent that it must recompute skin render/animDetail lists
	if(isSkinned())
	{
		nlassert(_FatherSkeletonModel);
		_FatherSkeletonModel->dirtSkinRenderLists();
	}
}
// ***************************************************************************
void		CTransform::setIsLoadbalancable(bool val)
{
	setStateFlag(IsLoadBalancable, val );
}


// ***************************************************************************
// ***************************************************************************
// Observers.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CTransformHrcObs::update()
{
	IBaseHrcObs::update();

	if(Model->TouchObs[CTransform::TransformDirty])
	{
		// update the local matrix.
		LocalMatrix= static_cast<CTransform*>(Model)->getMatrix();
		IBaseHrcObs::LocalVis= static_cast<CTransform*>(Model)->Visibility;
		// update the date of the local matrix.
		LocalDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;

		// The transform has been modified. Hence, it is no more frozen.
		Frozen= false;
	}

}


// ***************************************************************************
void	CTransformHrcObs::updateWorld(IBaseHrcObs *caller)
{
	const	CMatrix		*pFatherWM;
	bool				visFather;
	CTransform			*transModel= (CTransform*)Model;

	// If not root case, link to caller.
	if(caller)
	{
		pFatherWM= &(caller->WorldMatrix);
		visFather= caller->WorldVis;

		// If father is not Frozen, so do I.
		CTransformHrcObs	*hrcTransCaller= dynamic_cast<CTransformHrcObs*>(caller);

		// if caller is a CTransformHrcObs
		//  and if it is not frozen (for any reason), disable us!

		if (hrcTransCaller && !hrcTransCaller->Frozen && !hrcTransCaller->DontUnfreezeChildren)
			Frozen= false;

		// if caller is a CTransformHrcObs, may herit his _AncestorSkeletonModel
		// if caller is ! a CTransformHrcObs, final result undefined.
		if (hrcTransCaller && hrcTransCaller->_AncestorSkeletonModel)
			// If my father has an _AncestorSkeletonModel, get it.
			_AncestorSkeletonModel= hrcTransCaller->_AncestorSkeletonModel;
		else
			// else I have an ancestor skel model if I am sticked/binded directly to a skeleton model.
			_AncestorSkeletonModel= transModel->_FatherSkeletonModel;

	}
	// else, default!!
	else
	{
		pFatherWM= &(CMatrix::Identity);
		visFather= true;

		// at the root of the hierarchy, we have no parent, hence no FatherSkeletonModel nor _AncestorSkeletonModel.
		_AncestorSkeletonModel= NULL;

		// NB: Root is Frozen by essence :), so don't modify the frozen state here.
	}

	// Combine matrix
	if(LocalDate>WorldDate || (caller && caller->WorldDate>WorldDate) )
	{
		// Must recompute the world matrix.  ONLY IF I AM NOT SKINNED/STICKED TO A SKELETON in the hierarchy!
		if( _AncestorSkeletonModel==NULL )
		{
			WorldMatrix=  *pFatherWM * LocalMatrix;
			WorldDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;

			// Add the model to the moving object list
			if (!Frozen)
				static_cast<CHrcTrav*>(Trav)->_MovingObjects.push_back (Model);
		}
	}

	// Update dynamic lighting.
	/*
		If the model is not frozen in StaticLight, then must update lighting each frame.
		Even if the object doesn't move, a new dynamic light may enter in its aera. Hence we must test
		it in the light quadrid. StaticLight-ed Objects don't need it because they are inserted in a special quadgrid, 
		where dynamics lights touch all StaticLight-ed object to force their computing

		NB: not done if _AncestorSkeletonModel!=NULL. no need because  in this case, 
		result is driven by the _LightContribution of the _AncestorSkeletonModel.
	*/
	if( !transModel->_LightContribution.FrozenStaticLightSetup && _AncestorSkeletonModel==NULL )
	{
		// if the model is lightable reset lighting
		if( transModel->isLightable() )
			transModel->resetLighting();
	}

	// Combine visibility.
	switch(LocalVis)
	{
		case CHrcTrav::Herit: WorldVis= visFather; break;
		case CHrcTrav::Hide: WorldVis= false; break;
		case CHrcTrav::Show: WorldVis= true; break;
		default: break;
	}


	// If I have an ancestor Skeleton Model, I must be binded in ClipTrav to the SonsOfAncestorSkeletonModelGroup
	updateClipTravForAncestorSkeleton();

}


// ***************************************************************************
void	CTransformHrcObs::updateClipTravForAncestorSkeleton()
{
	CTransform			*transModel= (CTransform*)Model;
	CClipTrav			*clipTrav= (CClipTrav*)transModel->_ClipObs->Trav;

	// If I have an ancestor Skeleton Model, I must be binded in ClipTrav to the SonsOfAncestorSkeletonModelGroup
	if(_AncestorSkeletonModel && !ClipLinkedInSonsOfAncestorSkeletonModelGroup)
	{
		// ClipTrav is a graph, so must unlink from ALL olds models.
		IModel	*father= clipTrav->getFirstParent(transModel);
		while(father)
		{
			clipTrav->unlink(father, transModel);
			father= clipTrav->getFirstParent(transModel);
		}

		// And link to SonsOfAncestorSkeletonModelGroup.
		clipTrav->link(clipTrav->SonsOfAncestorSkeletonModelGroup, transModel);

		// update the flag.
		ClipLinkedInSonsOfAncestorSkeletonModelGroup= true;
	}


	// else I must be binded to the standard Root.
	if(!_AncestorSkeletonModel && ClipLinkedInSonsOfAncestorSkeletonModelGroup)
	{
		// verify first I am really still linked to the SonsOfAncestorSkeletonModelGroup.
		// This test is important, because link may have changed for any reason (portals, clipManager....).
		if( clipTrav->getNumParents(transModel) == 1 &&
			clipTrav->getFirstParent(transModel)==clipTrav->SonsOfAncestorSkeletonModelGroup )
		{
			// yes, unlink from it.
			clipTrav->unlink(clipTrav->SonsOfAncestorSkeletonModelGroup, transModel);
			// and now, link to std root.
			clipTrav->link(clipTrav->getRoot(), transModel);
		}

		// update the flag
		ClipLinkedInSonsOfAncestorSkeletonModelGroup= false;
	}
}


// ***************************************************************************
void	CTransformHrcObs::traverse(IObs *caller)
{
	// Recompute the matrix, according to caller matrix mode, and local matrix.
	nlassert(!caller || dynamic_cast<IBaseHrcObs*>(caller));
	updateWorld(static_cast<IBaseHrcObs*>(caller));
	// DoIt the sons.
	traverseSons();
}


// ***************************************************************************
void	CTransformClipObs::traverse(IObs *caller)
{
	nlassert(!caller || dynamic_cast<IBaseClipObs*>(caller));

	CClipTrav		*clipTrav= safe_cast<CClipTrav*>(Trav);
	IBaseClipObs	*callerClipObs= static_cast<IBaseClipObs*>(caller);
	CTransform		*transform= (CTransform*)Model;
	CScene			*scene= safe_cast<CScene*>(transform->_OwnerMot);

	if ((Date == clipTrav->CurrentDate) && Visible)
		return;
	Date = clipTrav->CurrentDate;

	// clip: update Visible flag.
	Visible= false;
	// if at least visible.
	if(HrcObs->WorldVis)
	{
		// If linked to a SkeletonModel anywhere in the hierarchy, don't clip, and use skeleton model clip result.
		// This works because we are sons of a special node which is not in the clip traversal, and
		// which is traversed at end of the traversal.
		if( ((CTransformHrcObs*)HrcObs)->_AncestorSkeletonModel!=NULL )
		{
			Visible= ((CTransformHrcObs*)HrcObs)->_AncestorSkeletonModel->isClipVisible();
			// Special test: if we are sticked to a skeletonModel, and if we are still visible, maybe we don't have to
			if(Visible && transform->_FatherSkeletonModel)
			{
				// if our skeletonModel father is displayed with a Lod, maybe we are not to be displayed
				if(transform->_FatherSkeletonModel->isDisplayedAsLodCharacter())
				{
					// We are visible only if we where sticked to the skeleton with forceCLod==true.
					// This is also true if we are actually a skeletonModel
					if(!transform->_ForceCLodSticked)
						// otherWise we are not visible. eg: this is the case of skins and some sticked object
						Visible= false;
				}
			}
		}
		// else, clip.
		else
		{
			// If the instance is not filtered
			if(scene->getFilterRenderFlags() & transform->_RenderFilterType)
				Visible= clip(callerClipObs);
		}
	}

	// if visible, add to list.
	if(Visible)
	{
		// add this observer to the visibility list.
		clipTrav->addVisibleObs(this);

		// Has not an ancestor skeleton model?
		if( ((CTransformHrcObs*)HrcObs)->_AncestorSkeletonModel==NULL )
		{
			// If needed, insert the model in the lighted list.
			// don't insert if has an ancestorSkeletonModel, because in this case, result is driven by 
			// the _LightContribution of the _AncestorSkeletonModel.
			if( transform->isLightable() )
				clipTrav->LightTrav->addLightedObs(transform->_LightObs);

			// If needed, insert the model in the animDetail list.
			// don't insert if has an ancestoreSkeletonModel, because in this case, this ancestore will 
			// animDetail through the hierarchy...
			if( transform->isAnimDetailable() )
				clipTrav->AnimDetailTrav->addVisibleObs(safe_cast<CTransformAnimDetailObs*>(AnimDetailObs));
		}

		// If needed, Add it to the loadBalancing trav
		if( transform->isLoadBalancable() )
			clipTrav->LoadBalancingTrav->addVisibleObs(LoadBalancingObs);

		// If needed, insert the model in the render list.
		if( transform->isRenderable() )
			clipTrav->RenderTrav->addRenderObs(RenderObs);
	}

	// DoIt the sons.
	traverseSons();
}



// ***************************************************************************
void	CTransformAnimDetailObs::updateWorldMatrixFromFather()
{
	CTransform		*transModel= static_cast<CTransform*>(Model);

	// If I have an ancestore Skeleton model
	if(transModel->_HrcObs->_AncestorSkeletonModel)
	{
		// then must first update my worldMatrix.
		transModel->updateWorldMatrixFromFather();
	}
}

// ***************************************************************************
void	CTransformAnimDetailObs::traverseWithoutUpdateWorldMatrix(IObs *caller)
{
	CTransform		*transModel= static_cast<CTransform*>(Model);

	// AnimDetail behavior: animate only if not clipped.
	// NB: no need to test because of VisibilityList use.

	// test if the refptr is NULL or not (RefPtr).
	CChannelMixer	*chanmix= transModel->_ChannelMixer;
	if(chanmix)
	{
		// eval detail!!
		chanmix->eval(true, static_cast<CAnimDetailTrav*>(Trav)->CurrentDate);
	}
}

// ***************************************************************************
void	CTransformAnimDetailObs::traverse(IObs *caller)
{
	// First, test if I must update my HrcObs worldMatrix because of the ancestorSkeleton scheme
	updateWorldMatrixFromFather();

	// eval channelMixer.
	traverseWithoutUpdateWorldMatrix(caller);

	// NB: if want to add something, do it in traverseWithoutUpdateWorldMatrix(), because
	// CSkeletonModel doesn't call CTransformAnimDetailObs::traverse()

	// no need to traverseSons. No graph here.
}


// ***************************************************************************
// ***************************************************************************
// Lighting.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CTransform::resetLighting()
{
	// if the model is already isNeedUpdateLighting, his light setup is reseted.
	// so no need to reset again
	if(isNeedUpdateLighting())
		return;


	// For all light not in FrozenStaticLightSetup, remove me from their list
	uint	startLight= 0;
	if(_LightContribution.FrozenStaticLightSetup)
	{
		startLight= _LightContribution.NumFrozenStaticLight;
	}

	// for all light in the list, remove me from their list.
	for(uint i=startLight; i<NL3D_MAX_LIGHT_CONTRIBUTION; i++)
	{
		CPointLight		*pl= _LightContribution.PointLight[i];
		// if end of list, break.
		if(!pl)
			break;
		else
		{
			// remove me from this light.
			pl->removeLightedModel(_LightContribution.TransformIterator[i]);
		}
	}
	// empty the list.
	if(startLight<NL3D_MAX_LIGHT_CONTRIBUTION)
		_LightContribution.PointLight[startLight]= NULL;


	// the model needs to update his lighting.
	setStateFlag(IsNeedUpdateLighting, true);
	
}


// ***************************************************************************
void			CTransform::freezeStaticLightSetup(CPointLight *pointLight[NL3D_MAX_LIGHT_CONTRIBUTION], 
		uint numPointLights, uint8 sunContribution, CPointLight *frozenAmbientlight)
{
	nlassert(numPointLights <= NL3D_MAX_LIGHT_CONTRIBUTION);

	// resetLighting() first.
	resetLighting();

	// Enable StaticLightSetup.
	_LightContribution.FrozenStaticLightSetup= true;
	_LightContribution.NumFrozenStaticLight= numPointLights;
	_LightContribution.SunContribution= sunContribution;
	// setup the FrozenAmbientLight
	_LightContribution.FrozenAmbientLight= frozenAmbientlight;
	// Setup other pointLights
	uint i;
	for(i=0;i<numPointLights;i++)
	{
		// set the light
		_LightContribution.PointLight[i]= pointLight[i];
		// Enable at max.
		_LightContribution.Factor[i]= 255;
		// Compute static AttFactor Later because don't have WorlPosition of the model here!!
		setStateFlag(IsNeedUpdateFrozenStaticLightSetup, true);

		// Do NOT set the iterator, because it is a staticLight.
	}
	// End the list
	if(i<NL3D_MAX_LIGHT_CONTRIBUTION)
		_LightContribution.PointLight[i]= NULL;
}

// ***************************************************************************
void			CTransform::unfreezeStaticLightSetup()
{
	// resetLighting() first.
	resetLighting();

	// Disable StaticLightSetup.
	_LightContribution.FrozenStaticLightSetup= false;
	_LightContribution.NumFrozenStaticLight= 0;
	// End the list
	_LightContribution.PointLight[0]= NULL;
	// No more FrozenAmbientLight
	_LightContribution.FrozenAmbientLight= NULL;

	// Don't need to update StaticLightSetup since no more exist.
	setStateFlag(IsNeedUpdateFrozenStaticLightSetup, false);
}


// ***************************************************************************
void	CTransformLightObs::traverse(IObs *caller)
{
	CTransform		*transform= (CTransform*)Model;

	// if the model do not need to update his lighting, just skip.
	if(!transform->isNeedUpdateLighting())
		return;


	// If a freezeStaticLightSetup() has been called on this model recently.
	if(transform->isNeedUpdateFrozenStaticLightSetup())
	{
		// Now, the correct matrix is computed.
		// get the untransformed bbox from the model.
		CAABBox		bbox;
		transform->getAABBox(bbox);
		// get transformed center pos of bbox
		CVector	worldModelPos= transform->getWorldMatrix() * bbox.getCenter();

		// So we can compute AttFactor for each static light influencing this static object
		uint	numPointLights= transform->_LightContribution.NumFrozenStaticLight;
		for(uint i=0;i<numPointLights;i++)
		{
			const CPointLight	*pl= transform->_LightContribution.PointLight[i];
			// don't worry about the precision of floor, because of *255.
			float	distToModel= (pl->getPosition() - worldModelPos).norm();
			sint	attFactor= OptFastFloor( 255 * pl->computeLinearAttenuation(worldModelPos, distToModel) );
			transform->_LightContribution.AttFactor[i]= (uint8)attFactor;
		}

		// clean.
		transform->setStateFlag(CTransform::IsNeedUpdateFrozenStaticLightSetup, false);
	}


	// see CTransformClipObs::clip(), here I am Lightable(), and I have no _AncestorSkeletonModel
	// So I am sure that I really need to recompute my ModelLightContributions.
	((CLightTrav*)Trav)->LightingManager.computeModelLightContributions(transform,
		transform->_LightContribution, transform->_LogicInfo);

	// done!
	transform->setStateFlag(CTransform::IsNeedUpdateLighting, false);
}



}
