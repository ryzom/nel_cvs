/** \file skeleton_user.cpp
 * <File description>
 *
 * $Id: skeleton_user.cpp,v 1.11 2002/07/11 09:26:57 berenguier Exp $
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

#include "3d/skeleton_user.h"
#include "nel/misc/hierarchical_timer.h"


namespace NL3D
{

H_AUTO_DECL( NL3D_UI_Skeleton )

#define	NL3D_HAUTO_UI_SKELETON						H_AUTO_USE( NL3D_UI_Skeleton )


// ***************************************************************************
uint		CSkeletonUser::getNumBoneComputed() const
{
	NL3D_HAUTO_UI_SKELETON;

	return _Skeleton->getNumBoneComputed();
}

// ***************************************************************************
void		CSkeletonUser::setInterpolationDistance(float dist)
{
	NL3D_HAUTO_UI_SKELETON;

	_Skeleton->setInterpolationDistance(dist);
}

// ***************************************************************************
float		CSkeletonUser::getInterpolationDistance() const
{
	NL3D_HAUTO_UI_SKELETON;

	return _Skeleton->getInterpolationDistance();
}


// ***************************************************************************
void		CSkeletonUser::setShapeDistMax(float distMax)
{
	NL3D_HAUTO_UI_SKELETON;

	if(_Skeleton && _Skeleton->Shape)
	{
		_Skeleton->Shape->setDistMax(distMax);
	}
}

// ***************************************************************************
float		CSkeletonUser::getShapeDistMax() const
{
	NL3D_HAUTO_UI_SKELETON;

	if(_Skeleton && _Skeleton->Shape)
	{
		return _Skeleton->Shape->getDistMax();
	}
	else
		return -1;
}


// ***************************************************************************
bool		CSkeletonUser::bindSkin(UInstance *mi)
{
	NL3D_HAUTO_UI_SKELETON;

	if(mi==NULL)
	{
		nlerror("USkeleton::bindSkin(): mi is NULL");
		return false;
	}
	CTransform			*trans= dynamic_cast<CTransformUser*>(mi)->getTransform();
	CMeshBaseInstance	*meshi= dynamic_cast<CMeshBaseInstance*>(trans);
	if(meshi==NULL)
	{
		nlerror("USkeleton::bindSkin(): mi is not a MeshInstance or MeshMRMInstance");
		return false;
	}
	return _Skeleton->bindSkin(meshi);
}

// ***************************************************************************
void		CSkeletonUser::stickObject(UTransform *mi, uint boneId)
{
	NL3D_HAUTO_UI_SKELETON;

	if(mi==NULL)
		nlerror("USkeleton::stickObject(): mi is NULL");
	CTransform		*trans= (dynamic_cast<CTransformUser*>(mi))->getTransform();
	_Skeleton->stickObject(trans, boneId);
}

// ***************************************************************************
void		CSkeletonUser::stickObjectEx(UTransform *mi, uint boneId, bool forceCLod)
{
	NL3D_HAUTO_UI_SKELETON;

	if(mi==NULL)
		nlerror("USkeleton::stickObject(): mi is NULL");
	CTransform		*trans= (dynamic_cast<CTransformUser*>(mi))->getTransform();
	_Skeleton->stickObjectEx(trans, boneId, forceCLod);
}

// ***************************************************************************
void		CSkeletonUser::detachSkeletonSon(UTransform *mi)
{
	NL3D_HAUTO_UI_SKELETON;

	if(mi==NULL)
		nlerror("USkeleton::detachSkeletonSon(): mi is NULL");
	CTransform		*trans= (dynamic_cast<CTransformUser*>(mi))->getTransform();
	_Skeleton->detachSkeletonSon(trans);
}


// ***************************************************************************
uint		CSkeletonUser::getNumBones() const
{
	NL3D_HAUTO_UI_SKELETON;

	return _Bones.size();
}
UBone		&CSkeletonUser::getBone(uint boneId)
{
	NL3D_HAUTO_UI_SKELETON;

	if(boneId>=_Bones.size())
		nlerror("getBone(): bad boneId");
	return dynamic_cast<UBone&>(_Bones[boneId]);
}
sint		CSkeletonUser::getBoneIdByName(const std::string &boneName) const
{
	NL3D_HAUTO_UI_SKELETON;

	return _Skeleton->getBoneIdByName(boneName);
}
bool		CSkeletonUser::isBoneComputed(uint boneId) const
{
	NL3D_HAUTO_UI_SKELETON;

	return _Skeleton->isBoneComputed(boneId);
}

// ***************************************************************************
void		CSkeletonUser::setLodCharacterShape(sint shapeId)
{
	NL3D_HAUTO_UI_SKELETON;
	_Skeleton->setLodCharacterShape(shapeId);
}
sint		CSkeletonUser::getLodCharacterShape() const
{
	NL3D_HAUTO_UI_SKELETON;
	return _Skeleton->getLodCharacterShape();
}
void		CSkeletonUser::setLodCharacterAnimId(uint animId)
{
	NL3D_HAUTO_UI_SKELETON;
	_Skeleton->setLodCharacterAnimId(animId);
}
uint		CSkeletonUser::getLodCharacterAnimId() const
{
	NL3D_HAUTO_UI_SKELETON;
	return _Skeleton->getLodCharacterAnimId();
}
void		CSkeletonUser::setLodCharacterAnimTime(TGlobalAnimationTime time)
{
	NL3D_HAUTO_UI_SKELETON;
	_Skeleton->setLodCharacterAnimTime(time);
}
TGlobalAnimationTime	CSkeletonUser::getLodCharacterAnimTime() const
{
	NL3D_HAUTO_UI_SKELETON;
	return _Skeleton->getLodCharacterAnimTime();
}
bool		CSkeletonUser::isDisplayedAsLodCharacter() const
{
	NL3D_HAUTO_UI_SKELETON;
	return _Skeleton->isDisplayedAsLodCharacter();
}
void		CSkeletonUser::setLodCharacterDistance(float dist)
{
	NL3D_HAUTO_UI_SKELETON;
	_Skeleton->setLodCharacterDistance(dist);
}
float		CSkeletonUser::getLodCharacterDistance() const
{
	NL3D_HAUTO_UI_SKELETON;
	return _Skeleton->getLodCharacterDistance();
}
void		CSkeletonUser::setLodCharacterWrapMode(bool wrapMode)
{
	NL3D_HAUTO_UI_SKELETON;
	_Skeleton->setLodCharacterWrapMode(wrapMode);
}
bool		CSkeletonUser::getLodCharacterWrapMode() const
{
	NL3D_HAUTO_UI_SKELETON;
	return _Skeleton->getLodCharacterWrapMode();
}


// ***************************************************************************
void		CSkeletonUser::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	NL3D_HAUTO_UI_SKELETON;
	_Skeleton->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
}

} // NL3D
