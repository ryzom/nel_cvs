/** \file u_skeleton.h
 * <File description>
 *
 * $Id: u_skeleton.h,v 1.5 2002/05/13 16:47:57 berenguier Exp $
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

#ifndef NL_U_SKELETON_H
#define NL_U_SKELETON_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_transform.h"


namespace NL3D
{

class	UInstance;
class	UBone;

// ***************************************************************************
/**
 * Game interface for manipulating Skeleton.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class USkeleton : virtual public UTransform
{
protected:

	/// \name Object
	// @{
	USkeleton() {}
	virtual	~USkeleton() {}
	// @}

public:


	/// \name Skin operation.
	// @{
	/** bind a MeshInstance skin to the skeleton. NB: ~UTransform() and ~USkeleton() call detachSkeletonSon().
	 * NB: nlerror() if mi is not a UInstance. (ie a mesh instance).
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 */
	virtual	void		bindSkin(UInstance *mi) =0;
	/** parent a CTransform to a bone of the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * This object will be visible only when the Skeleton is not clipped.
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 */
	virtual	void		stickObject(UTransform *mi, uint boneId) =0;
	/** same method as stickObject(), but if you set forceCLod as true, then this object will be visible
	 *	even if the skeleton father is in CLod state (ie displayed with a CLodCharacterShape)
	 *	NB: if "mi" is a skeleton model, forceCLod is considerer true, whatever the value passed in.
	 */
	virtual	void		stickObjectEx(UTransform *mi, uint boneId, bool forceCLod) =0;
	/** unparent a CTransform from a bone of the skeleton, or unbind a skin. No-op if not here.
	 * NB: mi is placed at root of hierarchy.
	 */
	virtual	void		detachSkeletonSon(UTransform *mi) =0;
	// @}


	/// \name Bone access.
	// @{
	/// retrieve the number of bones.
	virtual	uint		getNumBones() const =0;
	/// retrieve the bone. nlerror if not here. (>=getNumBones())
	virtual	UBone		&getBone(uint boneId) =0;
	/// retrieve the bone Id, by his name. -1 if not found.
	virtual	sint		getBoneIdByName(const std::string &boneName) const=0;
	// @}


	/// \name Bone Lod interaction
	// @{

	/// return the number of bones currently animated/computed (because of bindSkin()/stickObject() / Lod system).
	virtual	uint		getNumBoneComputed() const =0;

	/** change the Lod Bone interpolation distance (in meters). If 0, interpolation is disabled.
	 *	The smaller this value is, the more Lod skeleton system will "pop". Default is 0.5 meters.
	 */
	virtual	void		setInterpolationDistance(float dist) =0;
	/// see setInterpolationDistance()
	virtual	float		getInterpolationDistance() const =0;


	/** Change Max Display Skeleton distance. After this distance the shape won't be displayed.
	 *	setting <0 means -1 and so means DistMax = infinite (default in meshs but multilod meshes).
	 *	NB: This apply to the shape direclty!! ie All instances using same shape will be affected
	 *
	 *	Note: If the skeleton himself is sticked to an other skeleton, this setup is not taken into account.
	 *	ie the skeleton clip follow the ancestor skeleton clip result (ie the first skeleton in hierarchy 
	 *	which is not sticked).
	 *
	 *	Note (complex): same remark for QuadGridClipManager interaction with this function as in 
	 *	UInstance::setShapeDistMax()
	 */
	virtual void		setShapeDistMax(float distMax) =0;

	/// see setShapeDistMax()
	virtual float		getShapeDistMax() const =0;

	// @}


	/// \name CLod / Character Lod
	/**	At a certain distance, the whole skeleton and all its skins may be replaced with a small Lod, animated
	 *	with a CLodCharacterManager.
	 */
	// @{

	/** Change the Character Lod shape Id. set -1 if want to disable the feature (default)
	 */
	virtual void		setLodCharacterShape(sint shapeId) =0;
	/// see setLodCharacterShape
	virtual sint		getLodCharacterShape() const =0;

	/// Change/get the Character Lod anim setup.
	virtual void		setLodCharacterAnimId(uint animId) =0;
	virtual uint		getLodCharacterAnimId() const =0;
	virtual void		setLodCharacterAnimTime(TGlobalAnimationTime time) =0;
	virtual TGlobalAnimationTime	getLodCharacterAnimTime() const =0;

	/// tells if the animation must loop or clamp.
	virtual void		setLodCharacterWrapMode(bool wrapMode) =0;
	virtual bool		getLodCharacterWrapMode() const =0;

	/** True if the skeleton model and his skins have been displayed with a CLodCharacterShape at last scene render
	 */
	virtual bool		isDisplayedAsLodCharacter() const =0;

	/** This is the distance at which the skeleton use a CLodCharacterShape to display himself
	 *	if 0, never display the skeleton as a CLodCharacterShape
	 */
	virtual void		setLodCharacterDistance(float dist) =0;

	/// see setLodCharacterDistance. 0 if disabled
	virtual float		getLodCharacterDistance() const =0;

	// @}


};


} // NL3D


#endif // NL_U_SKELETON_H

/* End of u_skeleton.h */
