/** \file skeleton_model.cpp
 * <File description>
 *
 * $Id: skeleton_model.cpp,v 1.11 2002/03/20 11:17:25 berenguier Exp $
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
#include "3d/skeleton_shape.h"


namespace NL3D
{


// ***************************************************************************
void		CSkeletonModel::registerBasic()
{
	CMOT::registerModel(SkeletonModelId, TransformShapeId, CSkeletonModel::creator);
	CMOT::registerObs(AnimDetailTravId, SkeletonModelId, CSkeletonModelAnimDetailObs::creator);
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
void		CSkeletonModel::initBoneUsages()
{
	// reset all to 0.
	_BoneUsage.resize(Bones.size(), 0);
	_ForcedBoneUsage.resize(Bones.size(), 0);
	_BoneToCompute.resize(Bones.size(), 0);
	_BoneToComputeDirty= false;
	_NumBoneComputed= 0;
}


// ***************************************************************************
void		CSkeletonModel::incBoneUsage(uint i, bool forced)
{
	nlassert(i<_BoneUsage.size());
	nlassert(i<_ForcedBoneUsage.size());

	if(forced)
	{
		// If the bone was not used before, must update _BoneToCompute.
		if(_ForcedBoneUsage[i]==0)
			_BoneToComputeDirty= true;

		// Inc the refCount of the bone.
		nlassert(_ForcedBoneUsage[i]<255);
		_ForcedBoneUsage[i]++;
	}
	else
	{
		// If the bone was not used before, must update _BoneToCompute.
		if(_BoneUsage[i]==0)
			_BoneToComputeDirty= true;

		// Inc the refCount of the bone.
		nlassert(_BoneUsage[i]<255);
		_BoneUsage[i]++;
	}
}


// ***************************************************************************
void		CSkeletonModel::decBoneUsage(uint i, bool forced)
{
	nlassert(i<_BoneUsage.size());
	nlassert(i<_ForcedBoneUsage.size());

	if(forced)
	{
		// If the bone was used before (and now won't be more), must update _BoneToCompute.
		if(_ForcedBoneUsage[i]==1)
			_BoneToComputeDirty= true;

		// Inc the refCount of the bone.
		nlassert(_ForcedBoneUsage[i]>0);
		_ForcedBoneUsage[i]--;
	}
	else
	{
		// If the bone was used before (and now won't be more), must update _BoneToCompute.
		if(_BoneUsage[i]==1)
			_BoneToComputeDirty= true;

		// Inc the refCount of the bone.
		nlassert(_BoneUsage[i]>0);
		_BoneUsage[i]--;
	}
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
void		CSkeletonModel::incForcedBoneUsageAndParents(uint i)
{
	// inc forced.
	incBoneUsage(i, true);

	// recurs to father
	sint	fatherId= Bones[i].getFatherId();
	// if not a root bone...
	if(fatherId>=0)
		incForcedBoneUsageAndParents(fatherId);
}

// ***************************************************************************
void		CSkeletonModel::decForcedBoneUsageAndParents(uint i)
{
	// dec forced
	decBoneUsage(i, true);

	// recurs to father
	sint	fatherId= Bones[i].getFatherId();
	// if not a root bone...
	if(fatherId>=0)
		decForcedBoneUsageAndParents(fatherId);
}


// ***************************************************************************
void		CSkeletonModel::updateBoneToCompute()
{
	// If already computed, skip
	if(!_BoneToComputeDirty)
		return;

	// get the channelMixer owned by CTransform.
	CChannelMixer	*chanMixer= getChannelMixer();

	// TODODO: skel shape Lods infos.

	// recompute _NumBoneComputed
	_NumBoneComputed= 0;

	// For all bones
	for(uint i=0; i<_BoneToCompute.size(); i++)
	{
		// set _BoneToCompute[i] to non 0 if BoneUsage[i] || ForcedBoneUsage[i];
		_BoneToCompute[i]= _BoneUsage[i] | _ForcedBoneUsage[i];
		// If the bone must be computed (if !0)
		if(_BoneToCompute[i])
		{
			// Inc _NumBoneComputed.
			_NumBoneComputed++;
			// lodEnable the channels of this bone
			if(chanMixer)
				Bones[i].lodEnableChannels(chanMixer, true);
		}
		else
		{
			// lodDisable the channels of this bone
			if(chanMixer)
				Bones[i].lodEnableChannels(chanMixer, false);
		}
	}

	// computed
	_BoneToComputeDirty= false;
}


// ***************************************************************************
void		CSkeletonModel::bindSkin(CTransform *mi)
{
	nlassert(mi);
	nlassert(mi->isSkinnable());

	// try to detach this object from me first.
	detachSkeletonSon(mi);

	// Then Add me.
	_Skins.insert(mi);

	// advert skin transform it is skinned.
	mi->_FatherSkeletonModel= this;
	// setApplySkin() use _FatherSkeletonModel to computeBonesId() and to update current skeleton bone usage.
	mi->setApplySkin(true);

	// link correctly Hrc only. ClipTrav grah updated in Hrc traversal.
	cacheTravs();
	HrcTrav->link(this, mi);
}
// ***************************************************************************
void		CSkeletonModel::stickObject(CTransform *mi, uint boneId)
{
	nlassert(mi);

	// try to detach this object from me first.
	detachSkeletonSon(mi);

	// Then Add me.
	_StickedObjects.insert(mi);
	// increment the refCount usage of the bone
	incForcedBoneUsageAndParents(boneId);

	// advert transform of its sticked state.
	mi->_FatherSkeletonModel= this;
	mi->_FatherBoneId= boneId;

	// link correctly Hrc only. ClipTrav grah updated in Hrc traversal.
	cacheTravs();
	HrcTrav->link(this, mi);
}
// ***************************************************************************
void		CSkeletonModel::detachSkeletonSon(CTransform *tr)
{
	nlassert(tr);

	// If the instance is not binded/sticked to the skeleton, exit.
	if(!tr->_FatherSkeletonModel)
		return;

	// try to erase from StickObject.
	_StickedObjects.erase(tr);
	// try to erase from Skins.
	_Skins.erase(tr);

	// If the instance is not skinned, then it is sticked
	if( !tr->isSkinned() )
	{
		// Then decrement Bone Usage RefCount.
		decForcedBoneUsageAndParents(tr->_FatherBoneId);
	}
	else
	{
		// it is skinned, advert the skinning is no more OK.
		// setApplySkin() use _FatherSkeletonModel to update current skeleton bone usage.
		tr->setApplySkin(false);
	}

	// advert transform it is no more sticked/skinned.
	tr->_FatherSkeletonModel= NULL;

	// link correctly Hrc only. ClipTrav grah updated in Hrc traversal.
	cacheTravs();
	HrcTrav->link(NULL, tr);
}


// ***************************************************************************
void		CSkeletonModel::cacheTravs()
{
	IObs			*HrcObs= getObs(NL3D::HrcTravId);

	HrcTrav= (CHrcTrav*)HrcObs->Trav;
}


// ***************************************************************************
sint32		CSkeletonModel::getBoneIdByName(const std::string &name) const
{
	nlassert(dynamic_cast<CSkeletonShape*>((IShape*)Shape));
	CSkeletonShape		*shp= (CSkeletonShape*)(IShape*)Shape;
	return shp->getBoneIdByName(name);
}


// ***************************************************************************
void	CSkeletonModelAnimDetailObs::traverse(IObs *caller)
{
	CSkeletonModel	*sm= (CSkeletonModel*)Model;

	/* If needed, let's know which bone has to be computed.
		Additionaly, enable / disable (lod) channels in channelMixer.
	*/ 
	sm->updateBoneToCompute();

	// Animate skeleton
	CTransformAnimDetailObs::traverse(caller);

	// if skeleton is clipped, no need to transform.
	// NB: no need to test ClipObs->Visible because of VisibilityList use.

	// test if bones must be updated.
	if(sm->IAnimatable::isTouched(CSkeletonModel::OwnerBit))
	{
		// Retrieve the WorldMatrix of the current CTransformShape.
		CMatrix		&modelWorldMatrix= HrcObs->WorldMatrix;

		// must test / update the hierarchy of Bones.
		// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
		for(uint i=0;i<sm->Bones.size();i++)
		{
			// Do nothing if the bone doesn't need to be computed (not used, lods ...)
			if(sm->_BoneToCompute[i])
			{
				sint	fatherId= sm->Bones[i].getFatherId();
				// if a root bone...
				if(fatherId==-1)
					// Compute root bone worldMatrix.
					sm->Bones[i].compute( NULL, modelWorldMatrix);
				else
					// Compute bone worldMatrix.
					sm->Bones[i].compute( &sm->Bones[fatherId], modelWorldMatrix);
			}
		}

		sm->IAnimatable::clearFlag(CSkeletonModel::OwnerBit);
	}

	// Sticked Objects: 
	// they will update their WorldMatrix after, because of the AnimDetail traverse scheme:
	// traverse visible ClipObs, and if skeleton, traverse Hrc sons.
}


} // NL3D
