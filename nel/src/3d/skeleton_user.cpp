/** \file skeleton_user.cpp
 * <File description>
 *
 * $Id: skeleton_user.cpp,v 1.7 2002/05/13 16:45:56 berenguier Exp $
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


namespace NL3D
{


// ***************************************************************************
uint		CSkeletonUser::getNumBoneComputed() const
{
	return _Skeleton->getNumBoneComputed();
}

// ***************************************************************************
void		CSkeletonUser::setInterpolationDistance(float dist)
{
	_Skeleton->setInterpolationDistance(dist);
}

// ***************************************************************************
float		CSkeletonUser::getInterpolationDistance() const
{
	return _Skeleton->getInterpolationDistance();
}


// ***************************************************************************
void		CSkeletonUser::setShapeDistMax(float distMax)
{
	if(_Skeleton && _Skeleton->Shape)
	{
		_Skeleton->Shape->setDistMax(distMax);
	}
}

// ***************************************************************************
float		CSkeletonUser::getShapeDistMax() const
{
	if(_Skeleton && _Skeleton->Shape)
	{
		return _Skeleton->Shape->getDistMax();
	}
	else
		return -1;
}


// ***************************************************************************
void		CSkeletonUser::stickObject(UTransform *mi, uint boneId)
{
	if(mi==NULL)
		nlerror("USkeleton::stickObject(): mi is NULL");
	CTransform		*trans= (dynamic_cast<CTransformUser*>(mi))->getTransform();
	_Skeleton->stickObject(trans, boneId);
}

// ***************************************************************************
void		CSkeletonUser::stickObjectEx(UTransform *mi, uint boneId, bool forceCLod)
{
	if(mi==NULL)
		nlerror("USkeleton::stickObject(): mi is NULL");
	CTransform		*trans= (dynamic_cast<CTransformUser*>(mi))->getTransform();
	_Skeleton->stickObjectEx(trans, boneId, forceCLod);
}


// ***************************************************************************
void		CSkeletonUser::setLodCharacterShape(sint shapeId)
{
	_Skeleton->setLodCharacterShape(shapeId);
}
sint		CSkeletonUser::getLodCharacterShape() const
{
	return _Skeleton->getLodCharacterShape();
}
void		CSkeletonUser::setLodCharacterAnimId(uint animId)
{
	_Skeleton->setLodCharacterAnimId(animId);
}
uint		CSkeletonUser::getLodCharacterAnimId() const
{
	return _Skeleton->getLodCharacterAnimId();
}
void		CSkeletonUser::setLodCharacterAnimTime(TGlobalAnimationTime time)
{
	_Skeleton->setLodCharacterAnimTime(time);
}
TGlobalAnimationTime	CSkeletonUser::getLodCharacterAnimTime() const
{
	return _Skeleton->getLodCharacterAnimTime();
}
bool		CSkeletonUser::isDisplayedAsLodCharacter() const
{
	return _Skeleton->isDisplayedAsLodCharacter();
}
void		CSkeletonUser::setLodCharacterDistance(float dist)
{
	_Skeleton->setLodCharacterDistance(dist);
}
float		CSkeletonUser::getLodCharacterDistance() const
{
	return _Skeleton->getLodCharacterDistance();
}
void		CSkeletonUser::setLodCharacterWrapMode(bool wrapMode)
{
	_Skeleton->setLodCharacterWrapMode(wrapMode);
}
bool		CSkeletonUser::getLodCharacterWrapMode() const
{
	return _Skeleton->getLodCharacterWrapMode();
}


} // NL3D
