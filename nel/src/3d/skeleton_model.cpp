/** \file skeleton_model.cpp
 * <File description>
 *
 * $Id: skeleton_model.cpp,v 1.42 2003/05/06 15:34:42 berenguier Exp $
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

#include "nel/misc/hierarchical_timer.h"
#include "3d/skeleton_model.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/anim_detail_trav.h"
#include "3d/render_trav.h"
#include "3d/skeleton_shape.h"
#include "3d/scene.h"
#include "3d/lod_character_manager.h"
#include "3d/lod_character_shape.h"
#include "nel/misc/rgba.h"
#include "nel/misc/aabbox.h"
#include "3d/mesh_skin_manager.h"
#include "3d/mesh_base_instance.h"
#include "3d/async_texture_manager.h"



using namespace std;
using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
void		CSkeletonModel::registerBasic()
{
	CScene::registerModel(SkeletonModelId, TransformShapeId, CSkeletonModel::creator);
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
	_DisplayedAsLodCharacter= false;
	_LodCharacterDistance= 0;
	_OOLodCharacterDistance= 0;

	_DefaultMRMSetup= true;

	_SkinToRenderDirty= false;

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

	// RenderFilter: We are a skeleton
	_RenderFilterType= UScene::FilterSkeleton;
}

	
// ***************************************************************************
CSkeletonModel::~CSkeletonModel()
{
	// if initModel() called
	if(getOwnerScene())
	{
		// remove from scene
		getOwnerScene()->eraseSkeletonModelToList(_ItSkeletonInScene);
	}


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

	// Free Lod instance
	setLodCharacterShape(-1);

}


// ***************************************************************************
void	CSkeletonModel::initModel()
{
	// Link this skeleton to the CScene.
	_ItSkeletonInScene= getOwnerScene()->appendSkeletonModelToList(this);

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
	// reserve space for bone to compute
	_BoneToCompute.reserve(Bones.size());

	_BoneToComputeDirty= false;
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
	CSkeletonShape		*skeShape= (CSkeletonShape*)(IShape*)Shape;
	const CSkeletonShape::CLod	&lod= skeShape->getLod(_CurLod);

	// reset _BoneToCompute
	_BoneToCompute.clear();

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
			// lodEnable the channels of this bone
			if(chanMixer)
				Bones[i].lodEnableChannels(chanMixer, true);

			// This bone is computed => take his valid boneSkinMatrix.
			_BoneUsage[i].ValidBoneSkinMatrix= i;

			// Append to the list to compute.
			//-------
			CBoneCompute	bc;
			bc.Bone= &Bones[i];
			sint	fatherId= Bones[i].getFatherId();
			// if a root bone...
			if(fatherId==-1)
				bc.Father= NULL;
			else
				bc.Father= &Bones[fatherId];
			// MustInterpolate??
			bc.MustInterpolate= false;
			const CSkeletonShape::CLod	*lodNext= NULL;
			// if a lod exist after current lod, and if lod interpolation enabled
			if( _CurLod < skeShape->getNumLods()-1 && _LodInterpMultiplier>0 )
			{
				// get next lod.
				lodNext= &skeShape->getLod(_CurLod+1);
				// Lod interpolation on this bone ?? only if at next lod, the bone is disabled.
				// And only if it is not enabed because of a "Forced reason"
				// Must also have a father, esle can't interpolate.
				if(lodNext->ActiveBones[i]==0 && _BoneUsage[i].ForcedUsage==0 && _BoneUsage[i].CLodForcedUsage==0 
					&& bc.Father)
					bc.MustInterpolate= true;
			}
			// append
			_BoneToCompute.push_back(bc);
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

	// For 

	// computed
	_BoneToComputeDirty= false;
}


// ***************************************************************************
bool		CSkeletonModel::isBoneComputed(uint boneId) const
{
	if(boneId>=_BoneUsage.size())
		return false;
	else
		return _BoneUsage[boneId].MustCompute!=0;
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
	// First ensure that the transform is not frozen (unlink from some quadGrids etc...)
	mi->unfreezeHRC();
	// then never re-parse in validateList/Hrc/Clip
	mi->unlinkFromUpdateList();
	mi->hrcUnlink();
	// ClipTrav is a graph, so must unlink from ALL olds models.
	mi->clipUnlinkFromAll();
	// Ensure flag is correct
	mi->_ClipLinkedInSonsOfAncestorSkeletonModelGroup= false;


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
	hrcLinkSon( mi );

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

	// link correctly Hrc / Clip / UpdateList...
	getOwnerScene()->getRoot()->hrcLinkSon( tr );
	if( !wasSkinned )
	{
		//  No-op. ClipTrav graph/UpdateList updated in Hrc traversal.
	}
	else
	{
		// Skin case: must do the Job here.
		// Update ClipTrav here.
		getOwnerScene()->getRoot()->clipAddChild(tr);
		// Must re-add to the update list.
		tr->linkToUpdateList();
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
void	CSkeletonModel::traverseAnimDetail()
{
	CSkeletonShape	*skeShape= ((CSkeletonShape*)(IShape*)Shape);

	// Update Lod, and animate.
	//===============

	/*
		CTransformShape::traverseAnimDetail() is torn in 2 here because 
		channels may be enabled/disabled by updateBoneToCompute()
	*/

	// First update Skeleton WorldMatrix (case where the skeleton is sticked).
	CTransform::updateWorldMatrixFromFather();
	// get dist from camera.
	float	dist= (getWorldMatrix().getPos() - getOwnerScene()->getClipTrav().CamPos).norm();
	// Use dist to get current lod to use for this skeleton
	uint	newLod= skeShape->getLodForDistance( dist );
	if(newLod != _CurLod)
	{
		// set new lod to use.
		_CurLod= newLod;
		// dirt the skeleton.
		_BoneToComputeDirty= true;
	}

	// If needed, let's know which bone has to be computed, and enable / disable (lod) channels in channelMixer.
	bool forceUpdate= _BoneToComputeDirty;
	updateBoneToCompute();

	// Animate skeleton.
	CTransformShape::traverseAnimDetailWithoutUpdateWorldMatrix();


	// Prepare Lod Bone interpolation.
	//===============

	float	lodBoneInterp;
	const CSkeletonShape::CLod	*lodNext= NULL;
	// if a lod exist after current lod, and if lod interpolation enabled
	if( _CurLod < skeShape->getNumLods()-1 && _LodInterpMultiplier>0 )
	{
		// get next lod.
		lodNext= &skeShape->getLod(_CurLod+1);
		// get interp value to next.
		lodBoneInterp= (lodNext->Distance - dist) * _LodInterpMultiplier;
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
	if(lodBoneInterp != _CurLodInterp)
	{
		// set new one.
		_CurLodInterp= lodBoneInterp;
		// must update bone compute.
		forceUpdate= true;
	}



	// Compute bones
	//===============

	// test if bones must be updated. either if animation change or if BoneUsage change.
	if(IAnimatable::isTouched(CSkeletonModel::OwnerBit) || forceUpdate)
	{
		// Retrieve the WorldMatrix of the current CTransformShape.
		const CMatrix		&modelWorldMatrix= getWorldMatrix();

		// must test / update the hierarchy of Bones.
		// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
		uint							numBoneToCompute= _BoneToCompute.size();
		CSkeletonModel::CBoneCompute	*pBoneCompute= numBoneToCompute? &_BoneToCompute[0] : NULL;
		// traverse only bones which need to be computed
		for(;numBoneToCompute>0;numBoneToCompute--, pBoneCompute++)
		{
			// compute the bone with his father, if any
			pBoneCompute->Bone->compute( pBoneCompute->Father, modelWorldMatrix);

			// Lod interpolation on this bone .. only if interp is enabled now, and if bone wants it
			if(lodNext && pBoneCompute->MustInterpolate)
			{
				// interpolate with my father matrix.
				const CMatrix		&fatherMatrix= pBoneCompute->Father->getBoneSkinMatrix();
				pBoneCompute->Bone->interpolateBoneSkinMatrix(fatherMatrix, lodBoneInterp);
			}
		}

		IAnimatable::clearFlag(CSkeletonModel::OwnerBit);
	}

	// Sticked Objects: 
	// they will update their WorldMatrix after, because of the AnimDetail traverse scheme:
	// traverse visible Clip models, and if skeleton, traverse Hrc sons.


	// Update Animated Skins.
	//===============
	for(uint i=0;i<_AnimDetailSkins.size();i++)
	{
		// traverse it. NB: updateWorldMatrixFromFather() is called but no-op because isSkinned()
		_AnimDetailSkins[i]->traverseAnimDetail();
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
	if(_LodCharacterDistance>0)
		_OOLodCharacterDistance= 1.0f/_LodCharacterDistance;
	else
		_OOLodCharacterDistance= 0;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterShape(sint shapeId)
{
	// get a ptr on the scene which owns us, and so on the lodManager.
	CScene					*scene= getOwnerScene();
	CLodCharacterManager	*mngr= scene->getLodCharacterManager();

	// if mngr not setuped, noop (lod not possible).
	if(!mngr)
		return;

	// If a shape was setup, free the instance
	if(_CLodInstance.ShapeId>=0)
	{
		mngr->releaseInstance(_CLodInstance);
		_CLodInstance.ShapeId= -1;
	}

	// assign
	_CLodInstance.ShapeId= shapeId;

	// if a real shape is setuped, alloc an instance
	if(_CLodInstance.ShapeId>=0)
	{
		mngr->initInstance(_CLodInstance);
	}
}


// ***************************************************************************
void		CSkeletonModel::computeLodTexture()
{
	// is lod setuped
	if(_CLodInstance.ShapeId<0)
		return;

	// get a ptr on the scene which owns us, and so on the lodManager.
	CScene					*scene= getOwnerScene();
	CLodCharacterManager	*mngr= scene->getLodCharacterManager();
	// mngr must be setuped since shape Id is >-1
	nlassert(mngr);
	/* Get the asyncTextureManager. This is a Hack. We use the AsyncTextureManager to store very low version of Textures
		(kept in DXTC1 format for minimum memory overhead).
		HENCE Lod Texture can work only with Async Textured instances!!
	*/
	CAsyncTextureManager	*asyncMngr= scene->getAsyncTextureManager();
	// if not setuped, cancel
	if(!asyncMngr)
		return;


	// **** start process. If cannot (TextureId==no more texture space), just quit.
	if(!mngr->startTextureCompute(_CLodInstance))
		return;
	uint maxNumBmpToReset= 0;

	// **** For all skins which have a LodTexture setuped
	ItTransformSet	it= _Skins.begin();
	for(;it!=_Skins.end();it++)
	{
		// the skin should be a meshBaseInstance setuped to asyncTexturing
		CMeshBaseInstance	*mbi= dynamic_cast<CMeshBaseInstance*>(*it);
		if(mbi && mbi->getAsyncTextureMode() && mbi->Shape)
		{
			CMeshBase	*mb= (CMeshBase*)(IShape*)(mbi->Shape);
			// get the LodTexture info of this shape.
			const CLodCharacterTexture	*lodText= mb->getLodCharacterTexture();
			// if setuped
			if(lodText)
			{
				// Ok, compute influence of this instance on the Lod.

				// ---- Build all bmps of the instance with help of the asyncTextureManager
				uint	numMats= mbi->Materials.size();
				// 256 materials possibles for the lod Manager
				numMats= min(numMats, 256U);
				// for endTexturecompute
				maxNumBmpToReset= max(maxNumBmpToReset, numMats);
				// process each materials
				for(uint i=0;i<numMats;i++)
				{
					// get the manager bitmap to write to
					CLodCharacterTmpBitmap	&dstBmp= mngr->getTmpBitmap(i);

					// if the material stage 0 is not textured, or has not a valid async id, build the bitmap with a color.
					sint			asyncTextId= mbi->getAsyncTextureId(i,0);
					const CBitmap	*coarseBitmap= NULL;
					if(asyncTextId!=-1)
					{
						// get it from async manager
						coarseBitmap= asyncMngr->getCoarseBitmap(asyncTextId);
					}

					// So if we have no bmp here, build with material color, else build a texture
					if(!coarseBitmap)
					{
						dstBmp.build(mbi->Materials[i].getDiffuse());
					}
					else
					{
						dstBmp.build(*coarseBitmap);
					}
				}

				// ---- add the lodTextureInfo to the current texture computed
				mngr->addTextureCompute(_CLodInstance, *lodText);
			}
		}
	}

	// **** compile the process
	mngr->endTextureCompute(_CLodInstance, maxNumBmpToReset);

}


// ***************************************************************************
void		CSkeletonModel::setLodCharacterAnimId(uint animId)
{
	_CLodInstance.AnimId= animId;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterAnimTime(TGlobalAnimationTime time)
{
	_CLodInstance.AnimTime= time;
}

// ***************************************************************************
void		CSkeletonModel::setLodCharacterWrapMode(bool wrapMode)
{
	_CLodInstance.WrapMode= wrapMode;
}


// ***************************************************************************
float		CSkeletonModel::computeDisplayLodCharacterPriority() const
{
	// if enabled
	if(_LodCharacterDistance>0 && _CLodInstance.ShapeId>=0)
	{
		CVector		globalPos;
		/* \todo yoyo: bad test of visibility. If the skeleton is hidden but has a _AncestorSkeletonModel 
			wich is visible, then it is supposed to be visible (in this test), but only for The CLod LoadBalancing 
			(priority not 0). Not so important...
		*/

		// Get object position, test visibility;
		// If has a skeleton ancestor, take his world position instead, because ours is invalid.
		if( _AncestorSkeletonModel != NULL)
		{
			// if the ancestore is clipped, quit
			if( !_AncestorSkeletonModel->isClipVisible() )
				return 0;
			// take ancestor world position
			globalPos= _AncestorSkeletonModel->getWorldMatrix().getPos();
		}
		else
		{
			// if the skeleton is clipped, quit
			if( !isClipVisible() )
				return 0;
			// take our world position
			globalPos= getWorldMatrix().getPos();
		}

		// compute distance from camera.
		float	dist= (getOwnerScene()->getClipTrav().CamPos - globalPos).norm();

		// compute priority
		return dist*_OOLodCharacterDistance;
	}
	else
		return 0;
}


// ***************************************************************************
void		CSkeletonModel::setDisplayLodCharacterFlag(bool displayCLod)
{
	// if enabled
	if(_LodCharacterDistance>0 && _CLodInstance.ShapeId>=0)
	{
		// If the flag has changed since last frame, must recompute bone Usage.
		if(_DisplayedAsLodCharacter != displayCLod)
			_BoneToComputeDirty= true;

		// set new state
		_DisplayedAsLodCharacter= displayCLod;
	}
}


// ***************************************************************************
void		CSkeletonModel::traverseRender()
{
	H_AUTO( NL3D_Skeleton_Render );

	// render as CLod, or render Skins.
	if(isDisplayedAsLodCharacter())
		renderCLod();
	else
		renderSkins();
}


// ***************************************************************************
void			CSkeletonModel::computeCLodVertexColors(CLodCharacterManager *mngr)
{
	// if shape id set.
	if(_CLodInstance.ShapeId<0)
		return;
	// get the lod shape,a nd check exist in the manager
	const CLodCharacterShape	*lodShape= mngr->getShape(_CLodInstance.ShapeId);
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
		lodShape->endBoneColor(tmpColors, _CLodInstance.VertexColors);
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
		// If must follow default MRM setup from skins.
		if(_DefaultMRMSetup)
		{
			_LevelDetail.DistanceCoarsest= 0;
			_LevelDetail.DistanceMiddle= 0;
			_LevelDetail.DistanceFinest= 0;
		}

		// Parse to count new size of the arrays, and to build MRM info
		uint	opaqueSize= 0;
		uint	transparentSize= 0;
		uint	animDetailSize= 0;
		ItTransformSet		it;
		for(it= _Skins.begin();it!=_Skins.end();it++)
		{
			CTransform	*skin= *it;

			// If the skin is hidden, don't add it to any list!
			if(skin->getVisibility()==CHrcTrav::Hide)
				continue;

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
					// MRM Max skin setup.
					if(_DefaultMRMSetup)
					{
						// Get the maximum distance setup (ie the one which degrades the less)
						_LevelDetail.DistanceCoarsest= max(_LevelDetail.DistanceCoarsest, skinLevelDetail->DistanceCoarsest);
						_LevelDetail.DistanceMiddle= max(_LevelDetail.DistanceMiddle, skinLevelDetail->DistanceMiddle);
						_LevelDetail.DistanceFinest= max(_LevelDetail.DistanceFinest, skinLevelDetail->DistanceFinest);
					}
				}
			}
		}

		// MRM Max skin setup.
		if(_DefaultMRMSetup)
		{
			// compile LevelDetail.
			if(_LevelDetail.MaxFaceUsed==0)
				// build a bug-free level detail
				buildDefaultLevelDetail();
			else
				_LevelDetail.compileDistanceSetup();
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

			// If the skin is hidden, don't add it to any list!
			if(skin->getVisibility()==CHrcTrav::Hide)
				continue;

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
	_LevelDetail.DistanceFinest= 1;
	_LevelDetail.DistanceMiddle= 2;
	_LevelDetail.DistanceCoarsest= 3;
	_LevelDetail.compileDistanceSetup();
}


// ***************************************************************************
void			CSkeletonModel::renderCLod()
{
	CRenderTrav			&renderTrav= getOwnerScene()->getRenderTrav();
	IDriver				*drv= renderTrav.getDriver();
	CScene				*scene= getOwnerScene();
	// the lod manager. no op if not here
	CLodCharacterManager	*mngr= scene->getLodCharacterManager();
	if(!mngr)
		return;

	// Get global lighting on the instance. Suppose SunAmbient only.
	//=================
	const CLightContribution	*lightContrib;

	// the std case is to take my model lightContribution
	if(_AncestorSkeletonModel==NULL)
		lightContrib= &getSkeletonLightContribution();
	// but if skinned/sticked (directly or not) to a skeleton, take its.
	else
		lightContrib= &_AncestorSkeletonModel->getSkeletonLightContribution();

	// compute his main light contribution result. Try first with sun
	CRGBA	mainAmbient= scene->getSunAmbient();
	CRGBA	mainDiffuse= scene->getSunDiffuse();
	// modulate sun contribution
	mainDiffuse.modulateFromuiRGBOnly(mainDiffuse, lightContrib->SunContribution );
	CVector	mainLightDir= scene->getSunDirection();


	/* During night, and in the buildings, it may be better to use one of the other Points lights
		Test only with the first pointLight, for faster compute, even if It may fail in some cases.
	*/
	CPointLight	*mainPL= lightContrib->PointLight[0];
	if(mainPL)
	{
		CRGBA	plDiffuse;
		// get the diffuse of the pointLight, attenuated from distance and importance.
		plDiffuse.modulateFromuiRGBOnly(mainPL->getDiffuse(), lightContrib->AttFactor[0]);
		// compare the 2 diffuse
		uint	d0= mainDiffuse.R + mainDiffuse.G + mainDiffuse.B;
		uint	d1= plDiffuse.R + plDiffuse.G + plDiffuse.B;
		// if the pointLight is lighter, take it.
		if(d1>d0)
		{
			// leave ambient, but take diffuse and pointLight fake Direction
			mainDiffuse= plDiffuse;
			mainLightDir= getWorldMatrix().getPos() - mainPL->getPosition();
			mainLightDir.normalize();
		}
	}


	// compute colors of the lods.
	//=================
	// NB: even if texturing is sufficient, still important for AlphaTest.

	// If must recompute color because of change of skin color or if skin added/deleted
	if(_CLodVertexColorDirty)
	{
		// recompute vertex colors
		computeCLodVertexColors(mngr);
		// set _CLodVertexColorDirty to false.
		_CLodVertexColorDirty= false;
	}

	// render the Lod in the LodManager.
	//=================
	// render must have been intialized
	nlassert(mngr->isRendering());


	// add the instance to the manager. 
	if(!mngr->addRenderCharacterKey(_CLodInstance, getWorldMatrix(), 
		mainAmbient, mainDiffuse, mainLightDir) )
	{
		// If failed to add it because no more vertex space in the manager, retry.

		// close vertexBlock, compile render
		mngr->endRender();
		// and restart.
		mngr->beginRender(drv, renderTrav.CamPos);

		// retry. but no-op if refail.
		mngr->addRenderCharacterKey(_CLodInstance, getWorldMatrix(), 
			mainAmbient, mainDiffuse, mainLightDir);
	}
}


// ***************************************************************************
void			CSkeletonModel::renderSkins()
{
	// Render skins according to the pass.
	CRenderTrav			&rdrTrav= getOwnerScene()->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= rdrTrav.getDriver();
	nlassert(drv);


	// Compute the levelOfDetail
	float	alphaMRM= _LevelDetail.getLevelDetailFromPolyCount(getNumTrianglesAfterLoadBalancing());

	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);			


	// rdr good pass
	if(rdrTrav.isCurrentPassOpaque())
	{
		// Compute in Pass Opaque only the light contribution. 
		// Easier for skeleton: suppose lightable, no local attenuation

		// the std case is to take my model lightContribution
		if(_AncestorSkeletonModel==NULL)
			setupCurrentLightContribution(&_LightContribution, false);
		// but if sticked (directly or not) to a skeleton, take its.
		else
			setupCurrentLightContribution(&_AncestorSkeletonModel->_LightContribution, false);


		// Activate Driver setup: light and modelMatrix
		changeLightSetup( &rdrTrav );
		rdrTrav.getDriver()->setupModelMatrix(getWorldMatrix());


		// Render all totaly opaque skins.
		renderSkinList(_OpaqueSkins, alphaMRM);
	}
	else
	{
		// NB: must have some transparent skins, since thee skeletonModel is traversed in the transparent pass.

		// Activate Driver setup: light and modelMatrix
		changeLightSetup( &rdrTrav );
		rdrTrav.getDriver()->setupModelMatrix(getWorldMatrix());


		// render all opaque/transparent skins
		renderSkinList(_TransparentSkins, alphaMRM);
	}


	// bkup force normalisation.
	drv->forceNormalize(bkupNorm);
}


// ***************************************************************************
void			CSkeletonModel::renderSkinList(NLMISC::CObjectVector<CTransform*, false> &skinList, float alphaMRM)
{
	CRenderTrav			&rdrTrav= getOwnerScene()->getRenderTrav();

	// if the SkinManager is not possible at all, just rendered the std way
	if( !rdrTrav.getMeshSkinManager() || !rdrTrav.getMeshSkinManager()->enabled() )
	{
		for(uint i=0;i<skinList.size();i++)
		{
			skinList[i]->renderSkin(alphaMRM);
		}
	}
	else
	{
		// get the meshSkinManager
		CMeshSkinManager	&meshSkinManager= *rdrTrav.getMeshSkinManager();

		// array (rarely allocated) of skins with grouping support
		static	std::vector<CTransform*>	skinsToGroup;
		static	std::vector<uint>			baseVertices;
		skinsToGroup.clear();
		baseVertices.clear();

		// get the maxVertices the manager support
		uint	maxVertices= meshSkinManager.getMaxVertices();
		uint	vertexSize= meshSkinManager.getVertexSize();

		// render any skins which do not support SkinGrouping, and fill array of skins to group
		for(uint i=0;i<skinList.size();i++)
		{
			// If don't support, or if too big to fit in the manager, just renderSkin()
			if(!skinList[i]->supportSkinGrouping())
			{
				H_AUTO( NL3D_Skin_NotGrouped );
				skinList[i]->renderSkin(alphaMRM);
			}
			else
			{
				skinsToGroup.push_back(skinList[i]);
			}
		}

		H_AUTO( NL3D_Skin_Grouped );

		// For each skin, have an index which gives the decal of the vertices in the buffer
		baseVertices.resize(skinsToGroup.size());

		// while there is skin to render in group
		uint	skinId= 0;
		while(skinId<skinsToGroup.size())
		{
			// space left in the manager
			uint	remainingVertices= maxVertices;
			uint	currentBaseVertex= 0;

			// First pass, fill The VB.
			//------------
			// lock buffer
			uint8	*vbDest= meshSkinManager.lock();

			// For all skins until the buffer is full
			uint	startSkinId= skinId;
			while(skinId<skinsToGroup.size())
			{
				// if success to fill the AGP
				sint	numVerticesAdded= skinsToGroup[skinId]->renderSkinGroupGeom(alphaMRM, remainingVertices, 
					vbDest + vertexSize*currentBaseVertex );
				// -1 means that this skin can't render because no space left for her. Then stop for this block
				if(numVerticesAdded==-1)
					break;
				// Else ok, get the currentBaseVertex for this skin
				baseVertices[skinId]= currentBaseVertex;
				// and jump to the next place
				currentBaseVertex+= numVerticesAdded;
				remainingVertices-= numVerticesAdded;

				// go to the next skin
				skinId++;
			}

			// release buffer. ATI: release only vertices used.
			meshSkinManager.unlock(currentBaseVertex);

			// Second pass, render the primitives.
			//------------
			meshSkinManager.activate();

			/* Render any primitives that are not specular. Group specular ones into specularRdrPasses.
				NB: this speed a lot (specular setup is heavy)!
			*/
			static std::vector<CSkinSpecularRdrPass>	specularRdrPasses;
			specularRdrPasses.clear();
			for(uint i=startSkinId;i<skinId;i++)
			{
				// render the skin in the current buffer
				skinsToGroup[i]->renderSkinGroupPrimitives(baseVertices[i], specularRdrPasses, i);
			}

			// If any skin Specular rdrPass to render
			if(!specularRdrPasses.empty())
			{
				// Sort by Specular Map
				sort(specularRdrPasses.begin(), specularRdrPasses.end());

				// Batch Specular!
				rdrTrav.getDriver()->startSpecularBatch();

				// Render all of them
				for(uint i=0;i<specularRdrPasses.size();i++)
				{
					CSkinSpecularRdrPass	&specRdrPass= specularRdrPasses[i];
					// render the associated skin in the current buffer
					skinsToGroup[specRdrPass.SkinIndex]->renderSkinGroupSpecularRdrPass(specRdrPass.RdrPassIndex);
				}

				// End Batch Specular!
				rdrTrav.getDriver()->endSpecularBatch();
			}


			// End of this block, swap to the next buffer
			meshSkinManager.swapVBHard();
		}
	}
}


// ***************************************************************************
float			CSkeletonModel::getNumTriangles (float distance)
{
	// If the skeleton is displayed as a CLod suppose 0 triangles.
	if( isDisplayedAsLodCharacter() )
		return 0;
	else
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

	// Never more use MAX skin setup.
	_DefaultMRMSetup= false;
}


// ***************************************************************************
void			CSkeletonModel::resetDefaultMRMDistanceSetup()
{
	_DefaultMRMSetup= true;

	// Must use Skins linked to know the MRM setup.
	dirtSkinRenderLists();
}


// ***************************************************************************
bool			CSkeletonModel::computeRenderedBBox(NLMISC::CAABBox &bbox)
{
	// reset bbox
	CAABBox		tmpBBox;
	tmpBBox.setCenter(CVector::Null);
	tmpBBox.setHalfSize(CVector::Null);
	bool	empty= true;

	// Not visible => empty bbox
	if(!isClipVisible())
		return false;

	// For all bones
	uint	i;
	for(i=0;i<Bones.size();i++)
	{
		if(isBoneComputed(i))
		{
			const CVector	&pos= Bones[i].getLocalSkeletonMatrix().getPos();
			if(empty)
			{
				empty= false;
				tmpBBox.setCenter(pos);
			}
			else
				tmpBBox.extend(pos);
		}
	}

	// End!
	if(!empty)
	{
		bbox= tmpBBox;
		return true;
	}
	else
		return false;
}


// ***************************************************************************
bool			CSkeletonModel::computeCurrentBBox(NLMISC::CAABBox &bbox, bool forceCompute /* = false*/)
{
	// animate all bones channels (detail only channels). don't bother cur lod state.
	CChannelMixer	*chanmix= getChannelMixer();
	if (chanmix)
	{	
		// Force detail evaluation.
		chanmix->resetEvalDetailDate();
		chanmix->eval(true, 0);
		chanmix->resetEvalDetailDate();
	}
	// compute all skeleton bones
	computeAllBones(CMatrix::Identity);

	// reset bbox
	CAABBox		tmpBBox;
	tmpBBox.setCenter(CVector::Null);
	tmpBBox.setHalfSize(CVector::Null);
	bool	empty= true;

	// For all bones
	uint	i;
	for(i=0;i<Bones.size();i++)
	{
		// Is the bone used ?? (whatever bone lod, or CLod state)
		uint8	mustCompute = forceCompute ? 1 : _BoneUsage[i].Usage | _BoneUsage[i].ForcedUsage | _BoneUsage[i].CLodForcedUsage;

		// If the bone is used.
		if(mustCompute)
		{
			const CVector	&pos= Bones[i].getLocalSkeletonMatrix().getPos();
			if(empty)
			{
				empty= false;
				tmpBBox.setCenter(pos);
			}
			else
				tmpBBox.extend(pos);
		}
	}

	// End!
	if(!empty)
	{
		bbox= tmpBBox;
		return true;
	}
	else
		return false;
}


} // NL3D
