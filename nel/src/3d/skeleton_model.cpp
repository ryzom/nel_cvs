/** \file skeleton_model.cpp
 * <File description>
 *
 * $Id: skeleton_model.cpp,v 1.2 2001/04/13 16:38:33 berenguier Exp $
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

#include "nel/3d/skeleton_model.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/skeleton_shape.h"


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
		detachSkeletonSon(_StickedObjects.begin()->Transform);
	}

}



// ***************************************************************************
void		CSkeletonModel::bindSkin(CMeshInstance *mi)
{
	nlassert(mi);

	// try to detach this object from me first.
	detachSkeletonSon(mi);

	// Then Add me.
	_Skins.insert(mi);

	// advert meshinstance it is skinned.
	mi->_FatherSkeletonModel= this;
	mi->_ApplySkinOk= true;

	// link correctly Hrc and Clip.
	cacheTravs();
	HrcTrav->link(this, mi);
	ClipTrav->link(this, mi);

}
// ***************************************************************************
void		CSkeletonModel::stickObject(CTransform *mi, uint boneId)
{
	nlassert(mi);

	// try to detach this object from me first.
	detachSkeletonSon(mi);

	// Then Add me.
	// insert node into list.
	CStickObject	node;
	node.Transform= mi;
	node.BoneId= boneId;
	_StickedObjects.insert(node);

	// advert transform of its sticked state.
	mi->_FatherSkeletonModel= this;

	// link correctly Hrc and Clip.
	cacheTravs();
	HrcTrav->link(this, mi);
	ClipTrav->link(this, mi);

}
// ***************************************************************************
void		CSkeletonModel::detachSkeletonSon(CTransform *tr)
{
	nlassert(tr);

	// try to erase from StickObject.
	CStickObject	node;
	node.Transform= tr;
	_StickedObjects.erase(node);
	// try to erase from Skins.
	_Skins.erase((CMeshInstance*)tr);

	// advert transform it is no more sticked/skinned.
	tr->_FatherSkeletonModel= NULL;

	// If it is a skin, advert him the skinning is no more OK.
	CMeshInstance	*mi= dynamic_cast<CMeshInstance*>(tr);
	if(mi)
		mi->_ApplySkinOk= false;

	// link correctly Hrc and Clip: link to Roots!
	cacheTravs();
	HrcTrav->link(NULL, tr);
	ClipTrav->link(NULL, tr);

}


// ***************************************************************************
void		CSkeletonModel::cacheTravs()
{
	IObs			*HrcObs= getObs(NL3D::HrcTravId);
	IObs			*ClipObs= getObs(NL3D::ClipTravId);

	HrcTrav= (CHrcTrav*)HrcObs->Trav;
	ClipTrav= (CClipTrav*)ClipObs->Trav;
}


// ***************************************************************************
sint32		CSkeletonModel::getBoneIdByName(const std::string &name) const
{
	nlassert(dynamic_cast<CSkeletonShape*>((IShape*)Shape));
	CSkeletonShape		*shp= (CSkeletonShape*)(IShape*)Shape;
	return shp->getBoneIdByName(name);
}


} // NL3D
