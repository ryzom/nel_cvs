/** \file skeleton_model.h
 * <File description>
 *
 * $Id: skeleton_model.h,v 1.15 2002/05/15 16:55:56 berenguier Exp $
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
class CSkeletonModelClipObs;
class CLodCharacterManager;


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

	/** same method as stickObject(), but if you set forceCLod as true, then this object will be visible
	 *	even if the skeleton father is in CLod state (ie displayed with a CLodCharacterShape)
	 *	NB: if "mi" is a skeleton model, forceCLod is considerer true, whatever the value passed in.
	 */
	void		stickObjectEx(CTransform *mi, uint boneId, bool forceCLod);

	/** unparent a CTransform from a bone of the skeleton, or unbind a skin. No-op if not here.
	 * NB: mi is made son of Root in Traversals Hrc, and change are made at render() for ClipTrav.
	 */
	void		detachSkeletonSon(CTransform *mi);
	// @}


	/// \name Skin/BoneUsage Accessor. Called by CMeshInstance only.
	// @{

	typedef enum {UsageNormal, UsageForced, UsageCLodForced} TBoneUsageType;

	/** increment the refCount of the ith bone. 
	 *	set boneUsageType to UsageNormal if enable Skeleton Bone degradation (skins)
	 *	Forced usage are for Sticked objects
	 */
	void		incBoneUsage(uint i, TBoneUsageType boneUsageType);
	/// decrement the refCount of the ith bone. set forced to the same param passed when incBoneUsage()
	void		decBoneUsage(uint i, TBoneUsageType boneUsageType);

	/** This method update boneUsage (must be of size of Bones).
	 *	It's flag boneUsage[boneId] to true, and all parents of boneId.
	 */
	void		flagBoneAndParents(uint32 boneId, std::vector<bool>	&boneUsage) const;

	// @}


	/// \name Misc.
	// @{
	/// return, from skeleton shape, the BoneIdByName. -1 if not here.
	sint32		getBoneIdByName(const std::string &name) const;

	/// return the number of bones currently animated/computed (because of bindSkin()/stickObject() / Lod system).
	uint		getNumBoneComputed() const {return _NumBoneComputed;}

	/** change the Lod Bone interpolation distance (in meters). If 0, interpolation is disabled.
	 *	The smaller this value is, the more Lod skeleton system will "pop". Default is 0.5 meters.
	 */
	void		setInterpolationDistance(float dist);

	/// see setInterpolationDistance()
	float		getInterpolationDistance() const;

	/** if Bones[boneId] is "Computed" (usage/lod), return Bones[boneId].getBoneSkinMatrix()
	 *	else return parent ones (recurs, but precomputed)
	 */
	const NLMISC::CMatrix	&getActiveBoneSkinMatrix(uint boneId) const;


	/** Tool function, especially for animation bake. It updates all bones (independent of bone usage, 
	 *	and lod interpolation), and take a user skeleton worldMatrix as input.
	 *	NB: no detail animation is performed here, just the compute of bone hierarchy.
	 */
	void		computeAllBones(const CMatrix &modelWorldMatrix);


	// @}


	/** Lighting behavior. return true because skins/stickedObjects may surely need its LightContribution
	 *	Actually return getUserLightable();
	 */
	virtual bool		isLightable() const {return getUserLightable();}


	/// \name CLod / Character Lod
	// @{

	/// Change the Character Lod shape Id. set -1 if want to disable the feature (default)
	void			setLodCharacterShape(sint shapeId);
	/// see setLodCharacterShape
	sint			getLodCharacterShape() const {return _CLodShapeId;}

	/// Change/get the Character Lod anim setup.
	void			setLodCharacterAnimId(uint animId);
	uint			getLodCharacterAnimId() const {return _CLodAnimId;}
	void			setLodCharacterAnimTime(TGlobalAnimationTime time);
	TGlobalAnimationTime	getLodCharacterAnimTime() const {return _CLodAnimTime;}

	/// tells if the animation must loop or clamp.
	void			setLodCharacterWrapMode(bool wrapMode);
	bool			getLodCharacterWrapMode() const {return _CLodWrapMode;}


	/** True if the skeleton model and his skins are to be displayed with a CLodCharacterShape, instead of the std way
	 *	This state is modified early during the HRC Traversal. Because Clip traversal need this result.
	 */
	bool			isDisplayedAsLodCharacter() const {return _DisplayedAsLodCharacter;}

	/** This is the distance at which the skeleton use a CLodCharacterShape to display himself
	 *	if 0, never display the skeleton as a CLodCharacterShape
	 */
	void			setLodCharacterDistance(float dist);

	/// see setLodCharacterDistance. 0 if disabled
	float			getLodCharacterDistance() const {return _LodCharacterDistance;}

	/** Called by CTransformClipObs. update the flag _DisplayedAsLodCharacter.
	 *	No-op if already done (compare clipTrav current date).
	 */
	void			updateDisplayLodCharacterFlag(const CClipTrav *clipTrav);

	/** Called by CTransform::setMeanColor()
	 */
	void			dirtLodVertexColor() {_CLodVertexColorDirty= true;}

	// @}


// ***********************
protected:
	/// Constructor
	CSkeletonModel();
	/// Destructor
	virtual ~CSkeletonModel();


private:
	static IModel	*creator() {return new CSkeletonModel;}
	friend	class CSkeletonShape;
	friend	class CSkeletonModelClipObs;
	friend	class CSkeletonModelAnimDetailObs;
	friend	class CSkeletonModelRenderObs;
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

	struct CBoneUsage
	{
		/// The bone Usage (refCount).
		uint8			Usage;
		/// Same as Usage, but must be animated/computed, even if Skeleton Lods say not (stickedObjects).
		uint8			ForcedUsage;
		/** Same as ForcedUsage, but must be animated/computed, even if the skeleton is in CLod state 
		 *	ie displayed with a CLodCharacterShape. This is important for skeletons which have skeletons 
		 *	sons sticked on them
		 */
		uint8			CLodForcedUsage;
		/** The current state: which bones need to be computed. ie: 
		 *	(CLodForcedUsage) | ( ((Usage & currentLodUsage) | ForcedUsage) & skeleton not in CLod state )
		 */
		uint8			MustCompute;
		/// Myself if MustCompute==true, or the first parent with MustCompute==true.
		uint			ValidBoneSkinMatrix;
	};

	/// The list of BoneUsage. Updated by Meshes, stickObject(), and lod changes.
	std::vector<CBoneUsage>		_BoneUsage;
	/// Flag set if the MRSkeleton lod change, or if a new bone Usage change (only if was 0 or become 0).
	bool						_BoneToComputeDirty;
	/// Number of bones computed, ie number of entries in _BoneToCompute which are not 0.
	uint						_NumBoneComputed;
	/// The current lod activated for this skeleton
	uint						_CurLod;
	/// The current lod Interpolation Value for this skeleton
	float						_CurLodInterp;
	/// For lod interpolation. Inverse of distance. If 0, disable interpolation.
	float						_LodInterpMultiplier;


	/// called by CSkeletonShape::createInstance(). init the vector.
	void		initBoneUsages();

	/// increment the refCount of the ith bone and its parents. for stickObjects.
	void		incForcedBoneUsageAndParents(uint i, bool forceCLod);
	/// increment the refCount of the ith bone and its parents. for stickObjects.
	void		decForcedBoneUsageAndParents(uint i, bool forceCLod);

	/// According to Usage, ForedUsage and current skeleton Lod, update MustCompute and ValidBoneSkinMatrix
	void		updateBoneToCompute();

	// @}


	/// \name CLod / Character Lod
	// @{

	// return the contribution of lights (for Character Lod render).
	const CLightContribution	&getSkeletonLightContribution() {return _LightContribution;}

	/** True if the skeleton model and his skins have to be displayed with a CLodCharacterShape, instead of the std way
	 *	This state is modified early during the HRC Traversal. Because Clip traversal need this result.
	 */
	bool			_DisplayedAsLodCharacter;

	/// see setLodCharacterDistance
	float			_LodCharacterDistance;

	/// The last date _DisplayedAsLodCharacter has been computed
	sint64			_DisplayLodCharacterDate;

	/// The LodCharacter Shape/Anim setup
	sint			_CLodShapeId;	// -1 if disabled
	uint			_CLodAnimId;
	TGlobalAnimationTime	_CLodAnimTime;
	bool			_CLodWrapMode;

	/// The precomputed color array
	std::vector<CRGBA>	_CLodVertexColors;
	/** dirt when a bindSkin/stickObject/detachSkeletonSon is called
	 *	dirt when a transform mean color is changed.
	 */
	bool				_CLodVertexColorDirty;

	/// recompute _CLodVertexColors, ignoring _CLodVertexColorDirty
	void				computeCLodVertexColors(CLodCharacterManager *mngr);

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
 * \sa CHrcTrav CTransformHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CSkeletonModelClipObs : public CTransformShapeClipObs
{
public:

	/** this do :
	 *  - call CTransformShapeClipObs::traverse()
	 *  - If needed flag _DisplayedAsLodCharacter as true, and add renderObs in this case
	 */
	virtual	void	traverse(IObs *caller);

	/// Leave clip() method as CTransformShape one, but the skeleton model is not renderable.
	virtual	bool	isRenderable() const {return false;}


public:
	static IObs	*creator() {return new CSkeletonModelClipObs;}
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


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - extend the traverse method.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CSkeletonModelRenderObs : public CTransformShapeRenderObs
{
public:
 
	/** this should be called only if in CLodCaharacter state. In this case, it replaces CTransformShapeRenderObs:
	 *  - update instance Lighting
	 *  - render the lod.
	 */
	virtual	void	traverse(IObs *caller);


public:
	static IObs	*creator() {return new CSkeletonModelRenderObs;}
};


} // NL3D


#endif // NL_SKELETON_MODEL_H

/* End of skeleton_model.h */
