/** \file skeleton_model.cpp
 * <File description>
 *
 * $Id: skeleton_model.cpp,v 1.10 2002/02/28 12:59:51 besson Exp $
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

	// try to erase from StickObject.
	_StickedObjects.erase(tr);
	// try to erase from Skins.
	_Skins.erase(tr);

	// advert transform it is no more sticked/skinned.
	tr->_FatherSkeletonModel= NULL;

	// If it is a skin, advert him the skinning is no more OK.
	if(tr->isSkinnable())
		tr->setApplySkin(false);

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
	CTransformAnimDetailObs::traverse(caller);

	// if skeleton is clipped, no need to transform.
	// NB: no need to test ClipObs->Visible because of VisibilityList use.

	// test if bones must be updated.
	CSkeletonModel	*sm= (CSkeletonModel*)Model;
	if(sm->IAnimatable::isTouched(CSkeletonModel::OwnerBit))
	{
		// Retrieve the WorldMatrix of the current CTransformShape.
		CMatrix		&modelWorldMatrix= HrcObs->WorldMatrix;

		// must test / update the hierarchy of Bones.
		// Since they are orderd in depth-first order, we are sure that parent are computed before sons.
		for(uint i=0;i<sm->Bones.size();i++)
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

		sm->IAnimatable::clearFlag(CSkeletonModel::OwnerBit);
	}

	// Sticked Objects: 
	// they will update their WorldMatrix after, because of the AnimDetail traverse scheme:
	// traverse visible ClipObs, and if skeleton, traverse Hrc sons.
}


} // NL3D
