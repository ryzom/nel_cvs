/** \file skeleton_model.h
 * <File description>
 *
 * $Id: skeleton_model.h,v 1.8 2002/03/20 11:17:25 berenguier Exp $
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

#ifndef NL_SKELETON_MODEL_H
#define NL_SKELETON_MODEL_H

#include "nel/misc/types_nl.h"
#include "3d/transform_shape.h"
#include "3d/bone.h"


namespace NL3D
{


class CSkeletonShape;
class CTransformClipObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		SkeletonModelId=NLMISC::CClassId(0x7d4703b4, 0x43ad6ab1);


// ***************************************************************************
/**
 * A Skeleton model, instance of CSkeletonShape.
 *
 * Skeletons sons are added with bindSkin(), stickObject(). They are removed auto at dtor.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSkeletonModel : public CTransformShape
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:

	/** The list of CBone, created from the CSkeletonShape.
	 * They are odered in depth-first order.
	 */
	std::vector<CBone>			Bones;

public:
	/// \name IAnimatable Interface (registering only IAnimatable sons (bones)).
	// @{
	enum	TAnimValues
	{
		OwnerBit= CTransformShape::AnimValueLast, 

		AnimValueLast,
	};

	// Register bones into chanMixer.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);
	// @}


	/// \name Skin operation.
	// @{
	/** bind a skin to the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * NB: nlassert() if there is too many skins/sticked objects on this skeleton (more than 255).
	 * NB: nlassert(mi->isSkinnable());
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: mi is made son of skeleton model in Traversals Hrc, and change are made at render() for ClipTrav.
	 */
	void		bindSkin(CTransform *mi);
	/** parent a CTransform to a bone of the skeleton. NB: ~CTransform() calls detachSkeletonSon().
	 * NB: nlassert() if there is too many skins/sticked objects on this skeleton (more than 255).
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: mi is made son of skeleton model in Traversals Hrc, and change are made at render() for ClipTrav.
	 */
	void		stickObject(CTransform *mi, uint boneId);
	/** unparent a CTransform from a bone of the skeleton, or unbind a skin. No-op if not here.
	 * NB: mi is made son of Root in Traversals Hrc, and change are made at render() for ClipTrav.
	 */
	void		detachSkeletonSon(CTransform *mi);
	// @}


	/// \name Skin/BoneUsage Accessor. Called by CMeshInstance only.
	// @{
	/// increment the refCount of the ith bone. set forced to false for MRM that follow MultiResolutionSkeleton reduction.
	void		incBoneUsage(uint i, bool forced= true);
	/// decrement the refCount of the ith bone. set forced to false for MRM that follow MultiResolutionSkeleton reduction.
	void		decBoneUsage(uint i, bool forced= true);

	/** This method update boneUsage (must be of size of Bones).
	 *	It's flag boneUsage[boneId] to true, and all parents of boneId.
	 */
	void		flagBoneAndParents(uint32 boneId, std::vector<bool>	&boneUsage) const;

	// @}


	/// \name Misc.
	// @{
	/// return, from skeleton shape, the BoneIdByName. -1 if not here.
	sint32		getBoneIdByName(const std::string &name) const;

	/// return the number of bones currently animated/computed (because of bindSkin()/stickObject()).
	uint		getNumBoneComputed() const {return _NumBoneComputed;}
	// @}


	/** Lighting behavior. return true because skins/stickedObjects may surely need its LightContribution
	 *	Actually return getUserLightable();
	 */
	virtual bool		isLightable() const {return getUserLightable();}


// ***********************
protected:
	/// Constructor
	CSkeletonModel()
	{
		IAnimatable::resize(AnimValueLast);
		HrcTrav= NULL;
	}
	/// Destructor
	virtual ~CSkeletonModel();


private:
	static IModel	*creator() {return new CSkeletonModel;}
	friend	class CSkeletonShape;
	friend	class CSkeletonModelAnimDetailObs;
	friend	class CTransformClipObs;


	/// tells if the skeleton has been clipped in the clip traversal.
	bool	isClipVisible() const
	{
		return _ClipObs->Visible;
	}

	/// skins/sticked objects
	typedef	std::set<CTransform*>		TTransformSet;
	typedef	TTransformSet::iterator		ItTransformSet;
	/// The skins.
	TTransformSet				_Skins;
	/// The StickedObjects.
	TTransformSet				_StickedObjects;


	/// \name Bone Usage.
	// @{

	/// The list of BoneUsage (refCount). Updated by Meshes and stickObject().
	std::vector<uint8>			_BoneUsage;
	/// Same as BoneUsage, but must be animated/computed, even if Skeleton Lods say not (stickedObjects, std Meshes, old MRM meshes).
	std::vector<uint8>			_ForcedBoneUsage;
	/// The current state: which bones need to be computed. ie (BoneUsage & currentLodUsage) | ForcedBoneUsage.
	std::vector<uint8>			_BoneToCompute;
	/// Flag set if the MRSkeleton lod change, or if a new bone Usage change (only if was 0 or become 0).
	bool						_BoneToComputeDirty;
	/// Number of bones computed, ie number of entries in _BoneToCompute which are not 0.
	uint						_NumBoneComputed;

	/// called by CSkeletonShape::createInstance(). init the 3 vectors.
	void		initBoneUsages();

	/// increment the refCount of the ith bone and its parents. for stickObjects.
	void		incForcedBoneUsageAndParents(uint i);
	/// increment the refCount of the ith bone and its parents. for stickObjects.
	void		decForcedBoneUsageAndParents(uint i);

	/// According to _BoneUsage, _ForedBoneUsage and current skeleton Lod, update _BoneToCompute.
	void		updateBoneToCompute();

	// @}


	// The Hrc traversal of the Scene which owns this Skeleton.
	CHrcTrav		*HrcTrav;
	// test if HrcTrav!=NULL, else get from observers (done only one time).
	void			cacheTravs();
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - extend the traverse method.
 *
 * \sa CAnimDetailTrav IBaseAnimDetailObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CSkeletonModelAnimDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse()
	 *  - update animated bones.
	 */
	virtual	void	traverse(IObs *caller);


public:
	static IObs	*creator() {return new CSkeletonModelAnimDetailObs;}
};


} // NL3D


#endif // NL_SKELETON_MODEL_H

/* End of skeleton_model.h */
