/** \file skeleton_model.cpp
 * <File description>
 *
 * $Id: skeleton_model.cpp,v 1.24 2002/07/08 10:00:09 berenguier Exp $
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

#include "3d/skeleton_model.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/anim_detail_trav.h"
#include "3d/render_trav.h"
#include "3d/skeleton_shape.h"
#include "3d/scene.h"
#include "3d/lod_character_manager.h"
#include "3d/lod_character_shape.h"
#include "3d/skip_model.h"
#include "nel/misc/rgba.h"



using namespace std;
using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
void		CSkeletonModel::registerBasic()
{
	CMOT::registerModel(SkeletonModelId, TransformShapeId, CSkeletonModel::creator);
	CMOT::registerObs(ClipTravId, SkeletonModelId, CSkeletonModelClipObs::creator);
	CMOT::registerObs(AnimDetailTravId, SkeletonModelId, CSkeletonModelAnimDetailObs::creator);
	CMOT::registerObs(RenderTravId, SkeletonModelId, CSkeletonModelRenderObs::creator);
}


// ***************************************************************************
void		CSkeletonModel::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix);

	// Add any bones.
	for(uint i=0;i<Bones.size();i++)
	{
		// append  bonename.
		Bones[i].registerToChannelMixer(chanMixer, prefix + Bones[i].getBoneName() + ".");
	}

}

// ***************************************************************************
CSkeletonModel::CSkeletonModel()
{
	IAnimatable::resize(AnimValueLast);
	HrcTrav= NULL;
	ClipTrav= NULL;
	_DisplayedAsLodCharacter= false;
	_LodCharacterDistance= 0;
	_DisplayLodCharacterDate= -1;

	_SkinToRenderDirty= false;

	_CLodShapeId= -1;
	_CLodAnimId= 0;
	_CLodAnimTime= 0;
	_CLodWrapMode= true;
	_CLodVertexColorDirty= true;

	// Inform the transform that I am a skeleton
	CTransform::setIsSkeleton(true);

	// By default, no skins, hence, impossible to have transparent pass. But opaque pass is always possible
	// because of CLod rendering
	setOpacity(true);
	setTransparency(false);

	// AnimDetail behavior: Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);

	// LoadBalancing behavior. true because directly act on skins to draw all their MRM level
	CTransform::setIsLoadbalancable(true);

	// Lighting behavior. Lightable because skins/stickedObjects may surely need its LightContribution
	CTransform::setIsLightable(true);

	// Render behavior. Always renderable, because either render the skeleton as a CLod, or render skins
	CTransform::setIsRenderable(true);

	// build a bug-free level detail
	buildDefaultLevelDetail();
}

	
// ***************************************************************************
CSkeletonModel::~CSkeletonModel()
{

	// detach skeleton sons from skins.
	while(_Skins.begin()!=_Skins.end())
	{
		detachSkeletonSon(*_Skins.begin());
	}

	// detach skeleton sons from sticked objects.
	while(_StickedObjects.begin()!=_StickedObjects.end())
	{
		detachSkeletonSon(*_StickedObjects.begin());
	}

}


// ***************************************************************************
void	CSkeletonModel::initModel()
{
	IObs			*HrcObs= getObs(NL3D::HrcTravId);
	HrcTrav= (CHrcTrav*)HrcObs->Trav;
	IObs			*ClipObs= getObs(NL3D::ClipTravId);
	ClipTrav= (CClipTrav*)ClipObs->Trav;

	// Call base class
	CTransformShape::initModel();
}


// ***************************************************************************
void		CSkeletonModel::initBoneUsages()
{
	// reset all to 0.
	_BoneUsage.resize(Bones.size());
	for(uint i=0; i<_BoneUsage.size(); i++)
	{
		_BoneUsage[i].Usage= 0;
		_BoneUsage[i].ForcedUsage= 0;
		_BoneUsage[i].CLodForcedUsage= 0;
		_BoneUsage[i].MustCompute= 0;
		_BoneUsage[i].ValidBoneSkinMatrix= 0;
	}

	_BoneToComputeDirty= false;
	_NumBoneComputed= 0;
	_CurLod= 0;
	_CurLodInterp= 1.f;
	// Default is 0.5 meters.
	_LodInterpMultiplier= 1.f / 0.5f;
}


// ***************************************************************************
void		CSkeletonModel::incBoneUsage(uint i, TBoneUsageType boneUsageType)
{
	nlassert(i<_BoneUsage.size());

	// Get ptr on according refCount
	uint8	*usagePtr;
	if(boneUsageType == UsageNormal)
		usagePtr= &_BoneUsage[i].Usage;
	else if(boneUsageType == UsageForced)
		usagePtr= &_BoneUsage[i].ForcedUsage;
	else
		usagePtr= &_BoneUsage[i].CLodForcedUsage;

	// If the bone was not used before, must update MustCompute.
	if(*usagePtr==0)
		_BoneToComputeDirty= true;

	// Inc the refCount of the bone.
	nlassert(*usagePtr<255);
	(*usagePtr)++;
}


// ***************************************************************************
void		CSkeletonModel::decBoneUsage(uint i, TBoneUsageType boneUsageType)
{
	nlassert(i<_BoneUsage.size());

	// Get ptr on according refCount
	uint8	*usagePtr;
	if(boneUsageType == UsageNormal)
		usagePtr= &_BoneUsage[i].Usage;
	else if(boneUsageType == UsageForced)
		usagePtr= &_BoneUsage[i].ForcedUsage;
	else
		usagePtr= &_BoneUsage[i].CLodForcedUsage;

	// If the bone was used before (and now won't be more), must update MustCompute.
	if(*usagePtr==1)
		_BoneToComputeDirty= true;

	// Inc the refCount of the bone.
	nlassert(*usagePtr>0);
	(*usagePtr)--;
}


// ***************************************************************************
void		CSkeletonModel::flagBoneAndParents(uint32 boneId, std::vector<bool>	&boneUsage) const
{
	nlassert( boneUsage.size()==Bones.size() );
	nlassert( boneId<Bones.size() );

	// Flag this bone.
	boneUsage[boneId]= true;

	// if has father, flag it (recurs).
	sint	fatherId= Bones[boneId].getFatherId();
	if(fatherId>=0)
		flagBoneAndParents(fatherId, boneUsage);
}


// ***************************************************************************
void		CSkeletonModel::incForcedBoneUsageAndParents(uint i, bool forceCLod)
{
	// inc forced.
	incBoneUsage(i, forceCLod?UsageCLodForced:UsageForced );

	// recurs to father
	sint	fatherId= Bones[i].getFatherId();
	// if not a root bone...
	if(fatherId>=0)
		incForcedBoneUsageAndParents(fatherId, forceCLod);
}

// ***************************************************************************
void		CSkeletonModel::decForcedBoneUsageAndParents(uint i, bool forceCLod)
{
	// dec forced
	decBoneUsage(i, forceCLod?UsageCLodForced:UsageForced);

	// recurs to father
	sint	fatherId= Bones[i].getFatherId();
	// if not a root bone...
	if(fatherId>=0)
		decForcedBoneUsageAndParents(fatherId, forceCLod);
}


// ***************************************************************************
void		CSkeletonModel::updateBoneToCompute()
{
	// If already computed, skip
	if(!_BoneToComputeDirty)
		return;

	// get the channelMixer owned by CTransform.
	CChannelMixer	*chanMixer= getChannelMixer();

	// Get Lod infos from skeletonShape
	const CSkeletonShape::CLod	&lod= ((CSkeletonShape*)(IShape*)Shape)->getLod(_CurLod);

	// recompute _NumBoneComputed
	_NumBoneComputed= 0;

	// For all bones
	for(uint i=0; i<_BoneUsage.size(); i++)
	{
		// If we are in CLod mode
		if(isDisplayedAsLodCharacter())
			// don't compute the bone
			_BoneUsage[i].MustCompute= 0;
		else
		{
			// set MustCompute to non 0 if (Usage && Lod) || ForcedUsage;
			_BoneUsage[i].MustCompute= (_BoneUsage[i].Usage & lod.ActiveBones[i]) | _BoneUsage[i].ForcedUsage;
		}
		// if CLodForcedUsage for the bone, it must be computed, whatever _DisplayedAsLodCharacter state
		_BoneUsage[i].MustCompute|= _BoneUsage[i].CLodForcedUsage;

		// If the bone must be computed (if !0)
		if(_BoneUsage[i].MustCompute)
		{
			// Inc _NumBoneComputed.
			_NumBoneComputed++;
			// lodEnable the channels of this bone
			if(chanMixer)
				Bones[i].lodEnableChannels(chanMixer, true);

			// This bone is computed => take his valid boneSkinMatrix.
			_BoneUsage[i].ValidBoneSkinMatrix= i;
		}
		else
		{
			// lodDisable the channels of this bone
			if(chanMixer)
				Bones[i].lodEnableChannels(chanMixer, false);

			// This bone is not computed => take the valid boneSkinMatrix of his father
			sint	fatherId= Bones[i].getFatherId();
			if(fatherId<0)
				// just take me, even if not computed.
				_BoneUsage[i].ValidBoneSkinMatrix= i;
			else
				// NB: father ValidBoneSkinMatrix already computed because of the hierarchy order of Bones array.
				_BoneUsage[i].ValidBoneSkinMatrix= _BoneUsage[fatherId].ValidBoneSkinMatrix;
		}
	}

	// computed
	_BoneToComputeDirty= false;
}


// ***************************************************************************
const NLMISC::CMatrix	&CSkeletonModel::getActiveBoneSkinMatrix(uint boneId) const
{
	// Get me or first father with MustCompute==true.
	uint validBoneId= _BoneUsage[boneId].ValidBoneSkinMatrix;
	// return his WorldMatrix.
	return Bones[validBoneId].getBoneSkinMatrix();
}


// ***************************************************************************
bool		CSkeletonModel::bindSkin(CTransform *mi)
{
	nlassert(mi);
	if( !mi->isSkinnable() )
		return false;

	// try to detach this object from any skeleton first (possibly me).
	if(mi->_FatherSkeletonModel)
		mi->_FatherSkeletonModel->detachSkeletonSon(mi);

	// Then Add me.
	_Skins.insert(mi);

	// advert skin transform it is skinned.
	mi->_FatherSkeletonModel= this;
	// setApplySkin() use _FatherSkeletonModel to computeBonesId() and to update current skeleton bone usage.
	mi->setApplySkin(true);


	// Unlink the Skin from Hrc and clip, because SkeletonModel now does the job for him.
	nlassert(HrcTrav && ClipTrav);
	// First ensure that the transform is not frozen (unlink from some quadGrids etc...)
	mi->unfreezeHRC();
	// then never re-parse in validateList/Hrc/Clip
	mi->unlinkFromValidateList();
	HrcTrav->link(HrcTrav->Scene->getSkipModelRoot(), mi);
	// ClipTrav is a graph, so must unlink from ALL olds models.
	IModel	*father= ClipTrav->getFirstParent(mi);
	while(father)
	{
		ClipTrav->unlink(father, mi);
		father= ClipTrav->getFirstParent(mi);
	}
	// Ensure flag is correct
	mi->_HrcObs->ClipLinkedInSonsOfAncestorSkeletonModelGroup= false;
	// link to the SkipModelRoot One.
	ClipTrav->link(ClipTrav->Scene->getSkipModelRoot(), mi);


	// must recompute lod vertex color when LodCharacter used
	dirtLodVertexColor();
	// must recompute list of skins.
	dirtSkinRenderLists();

	// Ok, skinned
	return true;
}
// ***************************************************************************
void		CSkeletonModel::stickObject(CTransform *mi, uint boneId)
{
	// by default don't force display of "mi" if the skeleton become in CLod state
	stickObjectEx(mi, boneId, false);
}
// ***************************************************************************
void		CSkeletonModel::stickObjectEx(CTransform *mi, uint boneId, bool forceCLod)
{
	nlassert(mi);

	// if "mi" is a skeleton, forceCLod must be true, for correct animation purpose
	if(dynamic_cast<CSkeletonModel*>(mi))
		forceCLod= true;

	// try to detach this object from any skeleton first (possibly me).
	if(mi->_FatherSkeletonModel)
		mi->_FatherSkeletonModel->detachSkeletonSon(mi);

	// Then Add me.
	_StickedObjects.insert(mi);
	// increment the refCount usage of the bone
	incForcedBoneUsageAndParents(boneId, forceCLod);

	// advert transform of its sticked state.
	mi->_FatherSkeletonModel= this;
	mi->_FatherBoneId= boneId;
	// advert him if it is "ForceCLod" sticked
	mi->_ForceCLodSticked= forceCLod;

	// link correctly Hrc only. ClipTrav grah updated in Hrc traversal.
	nlassert(HrcTrav && ClipTrav);
	HrcTrav->link(this, mi);

	// must recompute lod vertex color when LodCharacter used
	dirtLodVertexColor();
}
// ***************************************************************************
void		CSkeletonModel::detachSkeletonSon(CTransform *tr)
{
	nlassert(tr);

	// If the instance is not binded/sticked to the skeleton, exit.
	if(tr->_FatherSkeletonModel!=this)
		return;

	// try to erase from StickObject.
	_StickedObjects.erase(tr);
	// try to erase from Skins.
	_Skins.erase(tr);

	// If the instance is not skinned, then it is sticked
	bool	wasSkinned= tr->isSkinned()!=0;
	if( !wasSkinned )
	{
		// Then decrement Bone Usage RefCount. Decrement from CLodForcedUsage if was sticked with forceCLod==true
		decForcedBoneUsageAndParents(tr->_FatherBoneId, tr->_ForceCLodSticked);
	}
	else
	{
		// it is skinned, advert the skinning is no more OK.
		// setApplySkin() use _FatherSkeletonModel to update current skeleton bone usage.
		tr->setApplySkin(false);
	}

	// advert transform it is no more sticked/skinned.
	tr->_FatherSkeletonModel= NULL;
	tr->_ForceCLodSticked= false;

	// link correctly Hrc / Clip / ValidateList...
	nlassert(HrcTrav && ClipTrav);
	HrcTrav->link(NULL, tr);
	if( !wasSkinned )
	{
		//  No-op. ClipTrav graph/ValidateList updated in Hrc traversal.
	}
	else
	{
		// Skin case: must do the Job here.
		// Update ClipTrav here.
		ClipTrav->unlink(ClipTrav->Scene->getSkipModelRoot(), tr);
		ClipTrav->link(ClipTrav->getRoot(), tr);
		// Must re-add to the validate list.
		tr->linkToValidateList();
	}


	// must recompute lod vertex color when LodCharacter used
	dirtLodVertexColor();
	// must recompute list of skins if was skinned
	if( wasSkinned )
		dirtSkinRenderLists();
}


// ***************************************************************************
sint32		CSkeletonModel::getBoneIdByName(const std::string &name) const
{
	CSkeletonShape		*shp= safe_cast<CSkeletonShape*>((IShape*)Shape);
	return shp->getBoneIdByName(name);
}


// ***************************************************************************
void		CSkeletonModel::setInterpolationDistance(float dist)
{
	dist= std::max(0.f, dist);
	// disable interpolation?
	if(dist==0)
		_LodInterpMultiplier= 0.f;
	else
		_LodInterpMultiplier= 1.f / dist;
}

// ***************************************************************************
float		CSkeletonModel::getInterpolationDistance() const
{
	if(_LodInterpMultiplier==0)
		return 0.f;
	else
		return 1.f / _LodInterpMultiplier;
}


// ***************************************************************************
void	CSkeletonModelAnimDetailObs::traverse(IObs *caller)
{
	CSkeletonModel	*sm= (CSkeletonModel*)Model;
	CSkeletonShape	*skeShape= ((CSkeletonShape*)(IShape*)sm->Shape);

	// Update Lod, and animate.
	//===============

	/*
		CTransformAnimDetailObs::traverse() is torn in 2 here because 
		channels may be enabled/disabled by updateBoneToCompute()
	*/

	// First update Skeleton WorldMatrix (case where the skeleton is sticked).
	CTransformAnimDetailObs::updateWorldMatrixFromFather();
	// get dist from camera.
	float	dist= (HrcObs->WorldMatrix.getPos() - ((CClipTrav*)ClipObs->Trav)->CamPos).norm();
	// Use dist to get current lod to use for this skeleton
	uint	newLod= skeShape->getLodForDistance( dist );
	if(newLod != sm->_CurLod)
	{
		// set new lod to use.
		sm->_CurLod= newLod;
		// dirt the skeleton.
		sm->_BoneToComputeDirty= true;
	}

	// If needed, let's know which bone has to be computed, and enable / disable (lod) channels in channelMixer.
	bool forceUpdate= sm->_BoneToComputeDirty;
	sm->updateBoneToCompute();

	// Animate skeleton.
	CTransformAnimDetailObs::traverseWithoutUpdateWorldMatrix(caller);


	// Prepare Lod Bone interpolation.
	//===============

	float	lodBoneInterp;
	const CSkeletonShape::CLod	*lodNext= NULL;
	// if a lod exist after current lod, and if lod interpolation enabled
	if( sm->_CurLod < skeShape->getNumLods()-1 && sm->_LodInterpMultiplier>0 )
	{
		// get next lod.
		lodNext= &skeShape->getLod(sm->_CurLod+1);
		// get interp value to next.
		lodBoneInterp= (lodNext->Distance - dist) * sm->_LodInterpMultiplier;
		NLMISC::clamp(lodBoneInterp, 0.f, 1.f);
		// if still 1, keep cur matrix => disable interpolation
		if(lodBoneInterp==1.f)
			lodNext=NULL;
	}
	// else, no interpolation
	else
	{
		lodBoneInterp=1.f;
	}
	// If the interpolation value is different from last one, must update.
	if(lodBoneInterp != sm->_CurLodInterp)
	{
		// set new one.
		sm->_CurLodInterp= lodBoneInterp;
		// must update bone compute.
		forceUpdate= true;
	}



	// Compute bones
	//===============

	// test if bones must be updated. either if animation change or if BoneUsage change.
	if(sm->IAnimatable::isTouched(CSkeletonModel::OwnerBit) || forceUpdate)
	{
		// Retrieve the WorldMatrix of the current CTransformShape.
		CMatrix		&modelWorldMatrix= HrcObs->WorldMatrix;

		// must test / update the hierarchy of Bones.
		// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
		for(uint i=0;i<sm->Bones.size();i++)
		{
			// Do nothing if the bone doesn't need to be computed (not used, lods ...)
			if(sm->_BoneUsage[i].MustCompute)
			{
				sint	fatherId= sm->Bones[i].getFatherId();
				// if a root bone...
				if(fatherId==-1)
					// Compute root bone worldMatrix.
					sm->Bones[i].compute( NULL, modelWorldMatrix);
				else
				{
					// Compute bone worldMatrix.
					sm->Bones[i].compute( &sm->Bones[fatherId], modelWorldMatrix);

					// Lod interpolation on this bone ?? only if at next lod, the bone is disabled.
					// And only if it is not enabed because of a "Forced reason"
					if(lodNext && lodNext->ActiveBones[i]==0 && 
						sm->_BoneUsage[i].ForcedUsage==0 && sm->_BoneUsage[i].CLodForcedUsage==0)
					{
						// interpolate with my father matrix.
						const CMatrix		&fatherMatrix= sm->Bones[fatherId].getBoneSkinMatrix();
						sm->Bones[i].interpolateBoneSkinMatrix(fatherMatrix, lodBoneInterp);
					}
				}
			}
		}

		sm->IAnimatable::clearFlag(CSkeletonModel::OwnerBit);
	}

	// Sticked Objects: 
	// they will update their WorldMatrix after, because of the AnimDetail traverse scheme:
	// traverse visible ClipObs, and if skeleton, traverse Hrc sons.


	// Update Animated Skins.
	//===============
	for(uint i=0;i<sm->_AnimDetailSkins.size();i++)
	{
		// get the detail Obs, via the clipObs
		CTransformAnimDetailObs		*adObs;
		adObs= safe_cast<CTransformAnimDetailObs*>(sm->_AnimDetailSkins[i]->_ClipObs->AnimDetailObs);

		// traverse it. NB: updateWorldMatrixFromFather() is called but no-op because isSkinned()
		adObs->traverse(NULL);
	}

}


// ***************************************************************************
void		CSkeletonModel::computeAllBones(const CMatrix &modelWorldMatrix)
{
	// must test / update the hierarchy of Bones.
	// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
	for(uint i=0;i<Bones.size();i++)
	{
		sint	fatherId= Bones[i].getFatherId();
		// if a root bone...
		if(fatherId==-1)
			// Compute root bone worldMatrix.
			Bones[i].compute( NULL, modelWorldMatrix);
		else
			// Compute bone worldMatrix.
			Bones[i].compute( &Bones[fatherId], modelWorldMatrix);
	}

}


// ***************************************************************************
void		CSkeletonModel::setLodCharacterDistance(float dist)
{
	_LodCharacterDistance= max(dist, 0.f);
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterShape(sint shapeId)
{
	_CLodShapeId= shapeId;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterAnimId(uint animId)
{
	_CLodAnimId= animId;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterAnimTime(TGlobalAnimationTime time)
{
	_CLodAnimTime= time;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterWrapMode(bool wrapMode)
{
	_CLodWrapMode= wrapMode;
}


// ***************************************************************************
void		CSkeletonModel::updateDisplayLodCharacterFlag(const CClipTrav *clipTrav)
{
	// If this compute has not been already done during this clip pass, do it.
	if(_DisplayLodCharacterDate < clipTrav->CurrentDate)
	{
		bool	precFlag= _DisplayedAsLodCharacter;

		// reset
		_DisplayedAsLodCharacter= false;

		// if enabled
		if(_LodCharacterDistance!=0 && _CLodShapeId>=0)
		{
			CVector		globalPos;

			// Get object position. 
			// If has a skeleton ancestor, take his world position instead, because ours is invalid.
			if( _HrcObs->_AncestorSkeletonModel != NULL)
				// take ancestor world position
				globalPos= _HrcObs->_AncestorSkeletonModel->getWorldMatrix().getPos();
			else
				// take our world position
				globalPos= _HrcObs->WorldMatrix.getPos();

			// compute distance from camera.
			float	dist= (clipTrav->CamPos - globalPos).norm();

			// compare with param.
			if(dist>_LodCharacterDistance)
				_DisplayedAsLodCharacter= true;
		}

		// If the flag has changed since last frame, must recompute bone Usage.
		if(precFlag != _DisplayedAsLodCharacter)
			_BoneToComputeDirty= true;

		// Ok, just do it one time per clip pass.
		_DisplayLodCharacterDate= clipTrav->CurrentDate;
	}
}


// ***************************************************************************
void		CSkeletonModelClipObs::traverse(IObs *caller)
{
	// call base clip method
	CTransformShapeClipObs::traverse(caller);

	// some extra stuff if visible
	if (Visible)
	{
		// update the _DisplayedAsLodCharacter flag
		CClipTrav		*clipTrav= (CClipTrav*)Trav;
		CSkeletonModel	*sm= (CSkeletonModel*)Model;
		// do it if not already done
		sm->updateDisplayLodCharacterFlag(clipTrav);

		/* Update Here the Skin render Lists.
			Done here, because AnimDetail and Render need correct lists. NB: important to do it 
			before Render Traversal, because updateSkinRenderLists() may change the transparency flag!!
		*/
		sm->updateSkinRenderLists();
	}
}


// ***************************************************************************
void		CSkeletonModelRenderObs::traverse(IObs *caller)
{
	CSkeletonModel		*sm= (CSkeletonModel*)Model;

	// render as CLod, or render Skins.
	if(sm->isDisplayedAsLodCharacter())
		renderCLod();
	else
		renderSkins();
}


// ***************************************************************************
void			CSkeletonModel::computeCLodVertexColors(CLodCharacterManager *mngr)
{
	// if shape id set.
	if(_CLodShapeId<0)
		return;
	// get the lod shape,a nd check exist in the manager
	const CLodCharacterShape	*lodShape= mngr->getShape(_CLodShapeId);
	if(lodShape)
	{
		static vector<CRGBAF>	tmpColors;
		tmpColors.clear();

		// start process.
		//-----------------
		lodShape->startBoneColor(tmpColors);

		// build an Id map, from Skeleton Ids to the lodShapes ids. (because may be differents)
		static vector<sint>	boneMap;
		// reset to -1 (ie not found)
		boneMap.clear();
		boneMap.resize(Bones.size(), -1);
		uint i;
		// for all skeletons bones.
		for(i=0; i<boneMap.size(); i++)
		{
			boneMap[i]= lodShape->getBoneIdByName(Bones[i].getBoneName());;
		}

		// Parse all skins
		//-----------------
		ItTransformSet	it;
		for(it= _Skins.begin(); it!=_Skins.end(); it++)
		{
			CTransform	*skin= *it;

			// get color of this skin.
			CRGBA	color= skin->getMeanColor();

			// get array of bone used for this skin.
			const vector<sint32>	*skinUsage= skin->getSkinBoneUsage();
			// check correct skin
			if(skinUsage)
			{
				// For all bones used
				for(uint i=0; i<skinUsage->size(); i++)
				{
					// the id in the vector point to a bone in the skeleton. Hence use the boneMap to translate it
					// in the lodShape ids.
					sint	idInLod= boneMap[(*skinUsage)[i]];
					// only if id found in the lod shape
					if(idInLod>=0)
						// add color to this bone.
						lodShape->addBoneColor(idInLod, color, tmpColors);
				}

			}
		}

		// Parse all sticked objects
		//-----------------
		for(it= _StickedObjects.begin(); it!=_StickedObjects.end(); it++)
		{
			CTransform	*object= *it;

			// get color of this object.
			CRGBA	color= object->getMeanColor();

			// get on which bone this object is linked.
			// use the boneMap to translate id to lodShape id.
			sint	idInLod= boneMap[object->_FatherBoneId];

			// only if id found in the lod shape
			if(idInLod>=0)
				// add color to this bone.
				lodShape->addBoneColor(idInLod, color, tmpColors);
		}


		// compile colors
		//-----------------
		lodShape->endBoneColor(tmpColors, _CLodVertexColors);
	}

}


// ***************************************************************************
void			CSkeletonModel::updateSkinRenderLists()
{
	// If need to update array of skins to compute
	if(_SkinToRenderDirty)
	{
		_SkinToRenderDirty= false;

		// Reset the LevelDetail.
		_LevelDetail.MinFaceUsed= 0;
		_LevelDetail.MaxFaceUsed= 0;

		// Parse to count new size of the arrays, and to build MRM info
		uint	opaqueSize= 0;
		uint	transparentSize= 0;
		uint	animDetailSize= 0;
		ItTransformSet		it;
		for(it= _Skins.begin();it!=_Skins.end();it++)
		{
			CTransform	*skin= *it;
			// if transparent, then must fill in transparent list.
			if(skin->isTransparent())
				transparentSize++;
			// else may fill in opaquelist. NB: for optimisation, don't add in opaqueList 
			// if added to the transperent list (all materials are rendered)
			else if(skin->isOpaque())
				opaqueSize++;

			// if animDetailable, then must fill list
			if(skin->isAnimDetailable())
				animDetailSize++;

			// if the skin support MRM, then must update levelDetal number of faces
			CTransformShape		*trShape= dynamic_cast<CTransformShape*>(skin);
			if(trShape)
			{
				const	CMRMLevelDetail		*skinLevelDetail= trShape->getMRMLevelDetail();
				if(skinLevelDetail)
				{
					// Add Faces to the Skeleton level detail
					_LevelDetail.MinFaceUsed+= skinLevelDetail->MinFaceUsed;
					_LevelDetail.MaxFaceUsed+= skinLevelDetail->MaxFaceUsed;
				}
			}
		}


		// alloc array.
		_OpaqueSkins.clear();
		_TransparentSkins.clear();
		_AnimDetailSkins.clear();
		_OpaqueSkins.resize(opaqueSize);
		_TransparentSkins.resize(transparentSize);
		_AnimDetailSkins.resize(animDetailSize);

		// ReParse, to fill array.
		uint	opaqueId= 0;
		uint	transparentId= 0;
		uint	animDetailId= 0;
		for(it= _Skins.begin();it!=_Skins.end();it++)
		{
			CTransform	*skin= *it;
			// if transparent, then must fill in transparent list.
			if(skin->isTransparent())
			{
				nlassert(transparentId<transparentSize);
				_TransparentSkins[transparentId++]= skin;
			}
			// else may fill in opaquelist. NB: for optimisation, don't add in opaqueList 
			// if added to the transperent list (all materials are rendered)
			else if(skin->isOpaque())
			{
				nlassert(opaqueId<opaqueSize);
				_OpaqueSkins[opaqueId++]= skin;
			}

			// if animDetailable, then must fill list
			if(skin->isAnimDetailable())
			{
				nlassert(animDetailId<animDetailSize);
				_AnimDetailSkins[animDetailId++]= skin;
			}
		}

		// set the Transparency to the skeleton only if has at least one transparent skin
		setTransparency( transparentSize>0 );
	}
}


// ***************************************************************************
void			CSkeletonModel::buildDefaultLevelDetail()
{
	// Avoid divide by zero.
	_LevelDetail.MinFaceUsed= 0;
	_LevelDetail.MaxFaceUsed= 0;
	_LevelDetail.DistanceFinest= 3;
	_LevelDetail.DistanceMiddle= 10;
	_LevelDetail.DistanceCoarsest= 50;
	_LevelDetail.compileDistanceSetup();
}


// ***************************************************************************
void			CSkeletonModelRenderObs::renderCLod()
{
	CRenderTrav			*trav= (CRenderTrav*)Trav;
	CSkeletonModel		*sm= (CSkeletonModel*)Model;
	IDriver				*drv= trav->getDriver();
	CScene				*scene= trav->Scene;

	// Get global lighting on the instance. Suppose SunAmbient only.
	//=================
	const CLightContribution	*lightContrib;
	// Get HrcObs.
	CTransformHrcObs	*hrcObs= (CTransformHrcObs*)HrcObs;

	// the std case is to take my model lightContribution
	if(hrcObs->_AncestorSkeletonModel==NULL)
		lightContrib= &sm->getSkeletonLightContribution();
	// but if skinned/sticked (directly or not) to a skeleton, take its.
	else
		lightContrib= &hrcObs->_AncestorSkeletonModel->getSkeletonLightContribution();

	// compute his sun contribution result
	CRGBA	sunContrib= scene->getSunDiffuse();
	// simulate/average diffuse lighting over the mesh by dividing diffuse by 2.
	sunContrib.modulateFromuiRGBOnly(sunContrib, lightContrib->SunContribution/2 );
	// Add Ambient
	sunContrib.addRGBOnly(sunContrib, scene->getSunAmbient());
	sunContrib.A= 255;


	// compute colors of the lods.
	//=================
	// the lod manager
	CLodCharacterManager	*mngr= trav->Scene->getLodCharacterManager();

	// If must recompute color because of change of skin color or if skin added/deleted
	if(sm->_CLodVertexColorDirty)
	{
		// recompute vertex colors
		sm->computeCLodVertexColors(mngr);
		// set sm->_CLodVertexColorDirty to false.
		sm->_CLodVertexColorDirty= false;
	}

	// render the Lod in the LodManager.
	//=================
	// render must have been intialized
	nlassert(mngr->isRendering());

	// add the instance to the manager. 
	if(!mngr->addRenderCharacterKey(sm->_CLodShapeId, sm->_CLodAnimId, sm->_CLodAnimTime, sm->_CLodWrapMode, 
		hrcObs->WorldMatrix, sm->_CLodVertexColors, sunContrib))
	{
		// If failed to add it because no more vertex space in the manager, retry.

		// close vertexBlock, compile render
		mngr->endRender();
		// and restart.
		mngr->beginRender(drv, trav->CamPos);

		// retry. but no-op if refail.
		mngr->addRenderCharacterKey(sm->_CLodShapeId, sm->_CLodAnimId, sm->_CLodAnimTime, sm->_CLodWrapMode, 
			hrcObs->WorldMatrix, sm->_CLodVertexColors, sunContrib);
	}
}


// ***************************************************************************
void			CSkeletonModelRenderObs::renderSkins()
{
	// Render skins according to the pass.
	CRenderTrav			*rdrTrav= (CRenderTrav*)Trav;
	CSkeletonModel		*sm= (CSkeletonModel*)Model;
	CTransformHrcObs	*hrcObs= (CTransformHrcObs*)HrcObs;
	// get a ptr on the driver
	IDriver				*drv= rdrTrav->getDriver();
	nlassert(drv);


	// Compute the levelOfDetail
	float	alphaMRM= sm->_LevelDetail.getLevelDetailFromPolyCount(sm->getNumTrianglesAfterLoadBalancing());

	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);			


	// rdr good pass
	if(rdrTrav->isCurrentPassOpaque())
	{
		// Compute in Pass Opaque only the light contribution. 
		// Easier for skeleton: suppose lightable, no local attenuation

		// the std case is to take my model lightContribution
		if(hrcObs->_AncestorSkeletonModel==NULL)
			sm->setupCurrentLightContribution(&sm->_LightContribution, false);
		// but if sticked (directly or not) to a skeleton, take its.
		else
			sm->setupCurrentLightContribution(&hrcObs->_AncestorSkeletonModel->_LightContribution, false);


		// Activate Driver setup: light and modelMatrix
		sm->changeLightSetup( rdrTrav );
		rdrTrav->getDriver()->setupModelMatrix(hrcObs->WorldMatrix);


		// Render all totaly opaque skins.
		for(uint i=0;i<sm->_OpaqueSkins.size();i++)
		{
			sm->_OpaqueSkins[i]->renderSkin(alphaMRM);
		}
	}
	else
	{
		// NB: must have some transparent skins, since thee skeletonModel is traversed in the transparent pass.

		// Activate Driver setup: light and modelMatrix
		sm->changeLightSetup( rdrTrav );
		rdrTrav->getDriver()->setupModelMatrix(hrcObs->WorldMatrix);


		// render all opaque/transparent skins
		for(uint i=0;i<sm->_TransparentSkins.size();i++)
		{
			sm->_TransparentSkins[i]->renderSkin(alphaMRM);
		}
	}


	// bkup force normalisation.
	drv->forceNormalize(bkupNorm);
}


// ***************************************************************************
float			CSkeletonModel::getNumTriangles (float distance)
{
	// NB: this is an approximation, but this is continious.
	return _LevelDetail.getNumTriangles(distance);
}

// ***************************************************************************
void			CSkeletonModel::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	// check input.
	if(distanceFinest<0)	return;
	if(distanceMiddle<=distanceFinest)	return;
	if(distanceCoarsest<=distanceMiddle)	return;

	// Change.
	_LevelDetail.DistanceFinest= distanceFinest;
	_LevelDetail.DistanceMiddle= distanceMiddle;
	_LevelDetail.DistanceCoarsest= distanceCoarsest;

	// compile 
	_LevelDetail.compileDistanceSetup();
}


} // NL3D
