/** \file skeleton_user.h
 * <File description>
 *
 * $Id: skeleton_user.h,v 1.22 2003/11/06 14:53:11 vizerie Exp $
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

#ifndef NL_SKELETON_USER_H
#define NL_SKELETON_USER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/3d/u_skeleton.h"
#include "nel/3d/u_instance.h"
#include "3d/transform_user.h"
#include "3d/skeleton_model.h"
#include "3d/mesh_base_instance.h"
#include "3d/bone_user.h"

#define NL3D_MEM_SKELETON						NL_ALLOC_CONTEXT( 3dSkel )

namespace NL3D {

class UPlayList;

// ***************************************************************************
/**
 * USkeleton implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSkeletonUser : virtual public USkeleton, public CTransformUser
{
public:
	/// This is the SAME pointer than _Transform, but correctly casted.
	CSkeletonModel			*_Skeleton;

	/// This is a mirror of _Skeleton->Bones
	std::vector<CBoneUser>	_Bones;


public:

	/// \name Object
	// @{
	/// This model should have been created with Scene::createInstance().
	CSkeletonUser(CScene *scene, CTransform *model) : 
	  CTransformUser(scene, model, true)
	{
		NL3D_MEM_SKELETON
		_Skeleton= NLMISC::safe_cast<CSkeletonModel*>(_Transform);

		// create user bones.
		uint	numBones= _Skeleton->Bones.size();
		_Bones.reserve(numBones);
		for(uint i=0;i<numBones;i++)
		{
			_Bones.push_back(&_Skeleton->Bones[i]);
		}
	}
	virtual	~CSkeletonUser()
	{
		NL3D_MEM_SKELETON
		// deleted in CTransformUser.
		_Skeleton= NULL;
		// user bones are auto deleted.
	}
	// @}



	/// \name Skin operation.
	// @{
	virtual	bool		bindSkin(UInstance *mi);
	virtual	void		stickObject(UTransform *mi, uint boneId);
	virtual	void		stickObjectEx(UTransform *mi, uint boneId, bool forceCLod);
	virtual	void		detachSkeletonSon(UTransform *mi);
	// @}


	/// \name Bone access.
	// @{
	virtual	uint		getNumBones() const;
	virtual	UBone		&getBone(uint boneId);
	virtual	sint		getBoneIdByName(const std::string &boneName) const;
	virtual	bool		isBoneComputed(uint boneId) const;
	virtual bool		forceComputeBone(uint boneId);
	// @}

	/// \name Lod interaction
	// @{
	virtual	uint		getNumBoneComputed() const;
	virtual	void		setInterpolationDistance(float dist);
	virtual	float		getInterpolationDistance() const;
	virtual void		setShapeDistMax(float distMax);
	virtual float		getShapeDistMax() const;
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);
	// @}


	/// \name CLod / Character Lod
	// @{
	virtual void		setLodCharacterShape(sint shapeId);
	virtual sint		getLodCharacterShape() const;
	virtual void		setLodCharacterAnimId(uint animId);
	virtual uint		getLodCharacterAnimId() const;
	virtual void		setLodCharacterAnimTime(TGlobalAnimationTime	time);
	virtual TGlobalAnimationTime	getLodCharacterAnimTime() const;
	virtual bool		isDisplayedAsLodCharacter() const;
	virtual void		setLodCharacterDistance(float dist);
	virtual float		getLodCharacterDistance() const;
	virtual void		setLodCharacterWrapMode(bool wrapMode);
	virtual bool		getLodCharacterWrapMode() const;
	virtual void		computeLodTexture();
	// @}


	/// \name Misc
	// @{
	virtual	bool		computeRenderedBBox(NLMISC::CAABBox &bbox, bool computeInWorld= false);
	virtual	bool		computeCurrentBBox(NLMISC::CAABBox &bbox, UPlayList *playList, double playTime=0, bool forceCompute = false, bool computeInWorld= false);
	// @}

	/// \name AnimCtrl (IK...)
	// @{
	virtual	void		setBoneAnimCtrl(uint boneId, IAnimCtrl *ctrl);
	virtual	IAnimCtrl	*getBoneAnimCtrl(uint boneId) const;
	// @}

public:
	/// \name Accessor for CSeneUser.
	// @{
	CSkeletonModel		*getSkeleton()
	{
		NL3D_MEM_SKELETON
		return _Skeleton;
	}
	// @}

};


} // NL3D


#endif // NL_SKELETON_USER_H

/* End of skeleton_user.h */
