/** \file skeleton_model.h
 * <File description>
 *
 * $Id: skeleton_model.h,v 1.34 2003/08/07 08:49:13 berenguier Exp $
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
#include "nel/misc/object_vector.h"
#include "3d/transform_shape.h"
#include "3d/bone.h"
#include "3d/mrm_level_detail.h"
#include "3d/lod_character_instance.h"

namespace NLMISC
{
	class	CAABBox;
}

namespace NL3D
{

class CSkeletonShape;
class CLodCharacterManager;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		SkeletonModelId=NLMISC::CClassId(0x7d4703b4, 0x43ad6ab1);


// ***************************************************************************
/** Struct used when SkinGroup is used.
 *	Used to sort specular render pass of all Skins (sorted by specular map).
 */
class	CSkinSpecularRdrPass
{
public:
	// The skin index in the grouped rendering.
	uint16		SkinIndex;
	// The index of the skin rdrPass
	uint16		RdrPassIndex;
	// The texture id of the specular texture. This is the sort Key.
	uint32		SpecId;

	bool	operator<(const CSkinSpecularRdrPass &o) const
	{
		return SpecId<o.SpecId;
	}
};


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
	/// Call at the begining of the program, to register the model
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
	 * NB: an object can't be skinned and sticked at same time :)
	 * NB: replaced if already here.
	 * NB: when a skin is binded, the command hide(), show(), ... have no effect on it, until it is detachSkeletonSon()-ed
	 * NB: For Skins, all Hrc/Clip/UpdateList link is done here
	 * \return false if mi is not skinnable, true otherwise
	 */
	bool		bindSkin(CTransform *mi);
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

	/** unparent a CTransform from a bone of the skeleton, or unbind a skin. No-op if not a son of this skeleton
	 * NB: mi is made son of Root in Traversals Hrc, and change are made at render() for ClipTrav.
	 * NB: For Skins, all Hrc/Clip/UpdateList link is done here
	 */
	void		detachSkeletonSon(CTransform *mi);

	/** Force the skeletonModel to recompute at next render which skins to render, at wich pass.
	 *	If you call setOpacity()/setTransparency() on one of the skins binded to the skeleton, you should call this
	 *	method, else strange result may occurs.
	 *	NB: this is automatically called by bindSkin()/detachSkeletonSon()
	 */
	void		dirtSkinRenderLists() {_SkinToRenderDirty= true;}
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

	/// Tell if a bone has been computed in the last frame or not. false if boneId is invalid
	bool		isBoneComputed(uint boneId) const;

	/// return the number of bones currently animated/computed (because of bindSkin()/stickObject() / Lod system).
	uint		getNumBoneComputed() const {return _BoneToCompute.size();}

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
	 *	NB: also, no special AnimCtrl (IK etc....) is performed here 
	 */
	void		computeAllBones(const CMatrix &modelWorldMatrix);

	/** Retrieve the current approx BBox around the skeleton, computed in the last CSene::render().
	 *	for all computed bones, extend the bbox with their pos
	 *	\param bbox return the bbox of the skinned skeleton, local to the skeleton. If the skeleton was clipped, the bbox
	 *	is not modified.
	 *	\param computeInWorld true if want to get the bbox in world.
	 *	\return true if the bbox is computed, false otherwise.
	 */
	bool		computeRenderedBBox(NLMISC::CAABBox &bbox, bool computeInWorld= false);

	/** same as computeRenderedBBox() but force animation and compute of all bones => don't need render(), but slower.
	 *	for all used bones, extend the bbox with their pos
	 *	NB: AnimCtrl are not evaluated by this method (since computed with 0 pos).
	 *	\param bbox return the bbox of the skinned skeleton, local to the skeleton. If the skeleton is not skinned/sticked
	 *	at all, bbox is not modified.
	 *  \param forceCompute force evalution even if not skinned
	 *	\param computeInWorld true if want to get the bbox in world.
	 *	\return true if the bbox is computed, false otherwise.
	 */
	bool		computeCurrentBBox(NLMISC::CAABBox &bbox, bool forceCompute = false, bool computeInWorld= false);

	// @}


	/// \name CLod / Character Lod
	// @{

	/// Change the Character Lod shape Id. set -1 if want to disable the feature (default)
	void			setLodCharacterShape(sint shapeId);
	/// see setLodCharacterShape
	sint			getLodCharacterShape() const {return _CLodInstance.ShapeId;}

	/// Change/get the Character Lod anim setup.
	void			setLodCharacterAnimId(uint animId);
	uint			getLodCharacterAnimId() const {return _CLodInstance.AnimId;}
	void			setLodCharacterAnimTime(TGlobalAnimationTime time);
	TGlobalAnimationTime	getLodCharacterAnimTime() const {return _CLodInstance.AnimTime;}

	/// tells if the animation must loop or clamp.
	void			setLodCharacterWrapMode(bool wrapMode);
	bool			getLodCharacterWrapMode() const {return _CLodInstance.WrapMode;}


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

	/** Called in ClipTrav pass. Used to update the flag _DisplayedAsLodCharacter.
	 *	return 0 if CLod not enabled. Else return a priority>0 computed from curDistance/LodCharacterDistance
	 *	return 0 too if the skeleton is not visible or if his ancestorSkeletonModel is not visible.
	 *	If priority is >1 then the skeleton must be displayed in CLod form
	 */
	float			computeDisplayLodCharacterPriority() const;

	/** Called in ClipTrav pass. setup the flag.
	 */
	void			setDisplayLodCharacterFlag(bool displayCLod);

	/** Called by CTransform::setMeanColor()
	 */
	void			dirtLodVertexColor() {_CLodVertexColorDirty= true;}

	/** Call it when you want the system to recompute the Lod texture
	 *	NB: Lod texturing is possible only in conjunction with AsyncTextureManager. Hence, instances skinned
	 *	to the skeleton should be in AsyncTextureMode.
	 *	For best result, you should wait that each of these instances are isAsyncTextureReady() (texture loaded)
	 */
	void			computeLodTexture();

	// @}

	/// \name Load balancing methods
	// @{

	/// Special version for skins
	virtual float	getNumTriangles (float distance);

	/** Special version for skins. NB: skins never follow their original MRM distance setup, but follow
	 *	this skeleton MRM setup. Default is to follow the MAX of all skins binded (ie the finer).
	 *	NB: Unlike CMeshBaseInstance::changeMRMDistanceSetup(), this setup applies to the SkeletonModel, not the shape.
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void			changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);

	/// Reset the Default MRM setup: follow MAX skin setup (ie the finer)
	void			resetDefaultMRMDistanceSetup();

	// @}


	/// \name CTransform traverse specialisation
	// @{
	/** this do :
	 *  - call CTransformShape::traverseAnimDetail()
	 *  - update animated bones.
	 */
	virtual void	traverseAnimDetail();
	/**	If displayed as a CLod, render it, else render the skins binded to this skeleton
	 */
	virtual	void	traverseRender();
	// @}

	// Lighting: get the Root world position!
	virtual	void		getLightHotSpotInWorld(CVector &modelPos, float &modelRadius) const;

	/// \name AnimCtrl (IK...)
	// @{
	/** Set a special ctrl on a bone. see IAnimCtrl. NB: once an animCtrl is set to a bone in a skeleton, 
	 *	his bones are always computed each frame. 
	 *	set to NULL if you want to reset this bone AnimCtrl.
	 */
	void			setBoneAnimCtrl(uint boneId, IAnimCtrl *ctrl);
	IAnimCtrl		*getBoneAnimCtrl(uint boneId) const;
	// @}


	/// \name ShadowMap CTransform Implementation
	// @{
	virtual	void		generateShadowMap(const CVector &lightDir);
	virtual	CShadowMap	*getShadowMap();
	virtual bool		computeWorldBBoxForShadow(NLMISC::CAABBox &worldBB);
	virtual void		renderIntoSkeletonShadowMap(CSkeletonModel *rootSkeleton, CMaterial	&castMat);
	// @}

// ***********************
protected:
	/// Constructor
	CSkeletonModel();
	/// Destructor
	virtual ~CSkeletonModel();

	/// Build link to traversals.
	virtual	void	initModel();

private:
	static CTransform	*creator() {return new CSkeletonModel;}
	friend	class CSkeletonShape;


public:
	// update if needed the renderList
	void						updateSkinRenderLists();

private:

	// The iterator of the skeleton inserted in the scene
	std::list<CSkeletonModel*>::iterator	_ItSkeletonInScene;

	/// skins/sticked objects
	typedef	std::set<CTransform*>		TTransformSet;
	typedef	TTransformSet::iterator		ItTransformSet;
	/// The skins.
	TTransformSet				_Skins;
	/// The StickedObjects.
	TTransformSet				_StickedObjects;

	// see dirtSkinRenderLists
	bool						_SkinToRenderDirty;
	// see _LevelDetail
	bool						_DefaultMRMSetup;
	// Raw lists of Skins. Both for transparent and opaque pass
	typedef NLMISC::CObjectVector<CTransform*, false>	TTransformArray;
	TTransformArray				_OpaqueSkins;
	TTransformArray				_TransparentSkins;
	// Skins which need to be animated (very rare)
	TTransformArray				_AnimDetailSkins;

	// The level detail used to drive MRM skins
	CMRMLevelDetail				_LevelDetail;
	// build a bug-free level detail
	void						buildDefaultLevelDetail();

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

	// A bone to compute information
	struct CBoneCompute
	{
		// The bone
		CBone			*Bone;
		// Father of the bone. May be NULL
		CBone			*Father;
		// true if must interpolate this bone with next lod
		bool			MustInterpolate;
	};

	/// The list of BoneUsage. Updated by Meshes, stickObject(), and lod changes.
	std::vector<CBoneUsage>		_BoneUsage;
	/// List of bones to compute.
	std::vector<CBoneCompute>	_BoneToCompute;
	/// Flag set if the MRSkeleton lod change, or if a new bone Usage change (only if was 0 or become 0).
	bool						_BoneToComputeDirty;
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
	bool					_DisplayedAsLodCharacter;

	/// see setLodCharacterDistance
	float					_LodCharacterDistance;
	float					_OOLodCharacterDistance;

	/// The Lod instance. -1 by default
	CLodCharacterInstance	_CLodInstance;

	/** dirt when a bindSkin/stickObject/detachSkeletonSon is called
	 *	dirt when a transform mean color is changed.
	 */
	bool					_CLodVertexColorDirty;

	/// recompute _CLodVertexColors, ignoring _CLodVertexColorDirty
	void				computeCLodVertexColors(CLodCharacterManager *mngr);

	// @}


	/// \name Rendering
	// @{
	/** render the skeleton as a CLod.
	 *  - update instance Lighting
	 *  - render the lod.
	 */
	void			renderCLod();

	/** render the skins of the skeleton
	 *  - update instance Lighting, and setup Driver lighting
	 *	- activate skeleton Matrix
	 *  - render all the skins (according if passOpaque or not)
	 */
	void			renderSkins();

	/** render a list of skin, no lighting setup etc..., but use where possible CMeshSkinManager
	 */
	void			renderSkinList(NLMISC::CObjectVector<CTransform*, false>	&skinList, float alphaMRM);
	// @}

	/// \name AnimCtrl (IK...)
	// @{
	// If >0, then user may change any if this bone each frame...
	sint			_AnimCtrlUsage;
	// @}


	// SkeletonModel can generate Shadow Map
	CShadowMap			*_ShadowMap;
	void			updateShadowMap(IDriver *driver);
	void			renderShadowSkins(CMaterial	&castMat);

};



} // NL3D


#endif // NL_SKELETON_MODEL_H

/* End of skeleton_model.h */
