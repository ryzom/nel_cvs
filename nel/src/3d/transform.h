/** \file transform.h
 * <File description>
 *
 * $Id: transform.h,v 1.18 2002/03/29 13:13:45 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_TRANSFORM_H
#define NL_TRANSFORM_H

#include "3d/mot.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/render_trav.h"
#include "3d/track.h"
#include "3d/transformable.h"
#include "3d/animated_value.h"
#include "3d/anim_detail_trav.h"
#include "3d/channel_mixer.h"
#include "nel/misc/matrix.h"
#include "3d/light_trav.h"
#include "3d/light_contribution.h"
#include "3d/lighting_manager.h"


namespace	NLMISC
{
	class	CAABBox;
};


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	CTransformHrcObs;
class	CTransformClipObs;
class	CTransformLightObs;
class	CSkeletonModel;
class	CSkeletonModelAnimDetailObs;
class	CInstanceGroup;
class	ILogicInfo;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		TransformId=NLMISC::CClassId(0x174750cb, 0xf952024);


// ***************************************************************************
/**
 * A basic node which provide an animatable matrix (ITransformable).
 * May be derived for each node who want to support such a scheme (CCamera, CLight, CInstance ... )
 *
 * CTransform ALWAYS herit scale from fathers! (joints skeleton may not...) (nbyoyo: else, this breaks the touch system with observers).
 *
 * CTransform Default tracks are identity (derived class may change this).
 *
 * No observer is provided for RenderTrav (not renderable => use default).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTransform : public IModel, public ITransformable
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:

	/// Accessors for opacity/transparency
	void setTransparency(bool v) { _Transparent = v; }
	void setOpacity(bool v) { _Opaque = v; }
	bool isOpaque() { return _Opaque; }
	bool isTransparent() { return _Transparent; }


	/** Set the current layer for this transform.
	  * Typically, this is used to sort transparent objects. Isn't used with solid objects.
	  * For now :
	  * Layer 0 is for underwater
	  * Layer 1 is for water surfaces
	  * Layer 2 is for object above water
	  */
	void  setOrderingLayer(uint layer) { _OrderingLayer = layer; }

	/// Get the ordering layer
	uint getOrderingLayer() const { return _OrderingLayer; }

	/// Hide the object and his sons.
	void		hide();
	/// Show the objet and his sons.
	void		show();
	/// herit the visibility from his father. (default behavior).
	void		heritVisibility();
	/// Get the local visibility state.
	CHrcTrav::TVisibility	getVisibility() {return Visibility;}
	/// Get the skeleton model. Returnr NULL in normal mode.
	CSkeletonModel*			getSkeletonModel () const {return _FatherSkeletonModel;}


	/// \name Derived from ITransformable.
	// @{
	/// Default Track Values are identity (pos,pivot= 0, scale= 1, rots=0).
	virtual ITrack* getDefaultTrack (uint valueId);
	/// register transform channels (in global anim mode).
	virtual void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);
	// @}


	/** freeze the preceding position of the model. Do not use, special code for cluster.
	 *	This inform the scene that preceding position setuped by user is "frozen". ie at next render(), this
	 *	object won't be added to the "object moving list" (usefull for cluster mgt).
	 *	The "frozen" state is disabled (at render() time) if:
	 *	- change in position (directly or indireclty, such as animation) is performed after the freeze().
	 *	- the "frozen" state of a father is not enabled (or disabled by a change in position of him :) ).
	 */
	void		freeze();
	void		setDontUnfreezeChildren(bool val);


	/** freeze the HRC so the WorldMatrix computed at next render() will be kept for long, and the model won't 
	 *	either be tested in HRC (which is still expensive, even if the worldmatrix doesn't need to be recomputed).
	 *	The model won't either be validated. It is suposed to not change at all. Also, if it is not a son of a CCluster,
	 *	it may be accelerated during Cliping (with CQuadGridClipManager).
	 *
	 *	NB: the model won't be tested in HRC only if this model is a "root", ie 
	 *	 HrcTrav->getFirstParent()==HrcTrav->getRoot().
	 *	calling freezeHRC() on a model in a hierarchy without calling it to the root of the hierarchy 
	 *	will result in that the model won't be validated, but still HRC traversed.
	 *	To be simplier, you should freezeHRC() all the models of a hierarchy, from base root to leaves.
	 *
	 *	NB: if the hierarchy of this object must change, or if the object must moves, you must call unfreezeHRC() first,
	 *	and you should do this for all the parents of this model.
	 */
	void		freezeHRC();


	/**	see freezeHRC().
	 */
	void		unfreezeHRC();


	/** special feature for CQuadGridClipManager.
	 */
	bool		isQuadGridClipManagerEnabled() const {return _QuadGridClipManagerEnabled;}

	/**
	 * Get the worldMatrix that is stored in the hrc observer
	 */
	const CMatrix& getWorldMatrix();

	/**
	 * Get the Visible state that is stored in the clip observer. True if visible.
	 */
	bool	getLastClippedState() const;



	void				setClusterSystem (CInstanceGroup *pIG) { _ClusterSystem = pIG; }
	CInstanceGroup*		getClusterSystem () { return _ClusterSystem; }


	/// name Lighting Behavior.
	// @{
	/** reset lights which influence this models. NB: the model is removed from all lights's list (except
	 *	FrozenStaticLightSetup). Called by light rendering.
	 *
	 *	NB: the model is NOT removed from LightingManager (with eraseStaticLightedModel()). 
	 */
	void				resetLighting();

	/** override this method if the model can be lighted (such as CMeshBaseInstance)
	 *	Default behavior is false.
	 *	Derived MUST take into account _UserLightable, and return false if _UserLightable==false.
	 */
	virtual bool		isLightable() const {return false;}

	/** Set the UserLightable flag. if false, isLightable() will always return false.
	 *	Doing this, user can disable lighting on a model which may be interesting for speed.
	 *	Default behavior is UserLightable==true.
	 */
	void				setUserLightable(bool enable) {_UserLightable= enable;}

	/** Get the UserLightable flag.
	 */
	bool				getUserLightable() const {return _UserLightable;}

	/** Freeze and set the Static Light Setup. Called by CInstanceGroup::addToScene()
	 *	NB: it calls resetLighting() first.
	 *	NB: nlassert(numPointLights<=NL3D_MAX_LIGHT_CONTRIBUTION)
	 */
	void				freezeStaticLightSetup(CPointLight *pointLight[NL3D_MAX_LIGHT_CONTRIBUTION], 
		uint numPointLights, uint8 sunContribution, CPointLight *frozenAmbientlight);

	/** unFreeze the Static Light Setup. Must be called if static pointLights are deleted.
	 *	NB: it calls resetLighting() first.
	 *	NB: do not need to call it if pointLights and this transform are deleted at same time.
	 */
	void				unfreezeStaticLightSetup();

	/** override this method if the lighting Manager must take into account the bbox of the transform.
	 *	Default behavior is false.
	 */
	virtual bool		isBigLightable() const {return false;}

	// @}


	/** Set the LogicInfo for this transfrom, eg to retrieve statc light information, see ILogicInfo.
	 *	Ptr is kept in CTransfrom, so should call setLogicInfo(NULL) before to clean up.
	 */
	void				setLogicInfo(ILogicInfo *logicInfo) {_LogicInfo= logicInfo;}


	/** Get the untransformed AABBox of the transform. NULL (gtCenter()= 0, gtSize()==0) by default.
	 */
	virtual void		getAABBox(NLMISC::CAABBox &bbox) const;


	/// name Load Balancing Behavior.
	// @{

	/** Change the load Balancing group of a model. Every models are in a special LoadBalancingGroup.
	 *	NB: the group is  created if did not exist.
	 *
	 *	By default, models lies in the "Default" group, but MRM with skinning and ParticlesSystems which
	 *	are in "Global" group (for backward compatibility).
	 *	The "Default" group is special because it is not balanced (ie models are only degraded from 
	 *	their distance to camera)
	 */
	void				setLoadBalancingGroup(const std::string &group);

	/** Get the load Balancing group of a model. see setLoadBalancingGroup().
	 */
	const std::string	&getLoadBalancingGroup() const;

	// @}

// ********
private:
	// Add our own dirty states.
	enum	TDirty
	{
		TransformDirty= IModel::Last,	// The matrix or the visibility state is modified.
		Last
	};

private:
	CHrcTrav::TVisibility	Visibility;

	static	CTrackDefaultVector		DefaultPos;
	static	CTrackDefaultVector		DefaultPivot;
	static	CTrackDefaultVector		DefaultRotEuler;
	static	CTrackDefaultQuat		DefaultRotQuat;
	static	CTrackDefaultVector		DefaultScale;

	void	foulTransform()
	{
		IModel::foul(TransformDirty);
	}

protected:
	/// Constructor
	CTransform();
	/// Destructor
	virtual ~CTransform();

	/// Implement the update method.
	virtual void	update();

	/// Implement the initModel method.
	virtual void	initModel();

	/// \name Skinning Behavior.
	// @{

	/// Deriver must change this method if the model can be skinned.
	virtual	bool			isSkinnable() const {return false;}
	/// Deriver must change this method to konw when the model is skinned.
	virtual	bool			isSkinned() const {return false;}
	/** Deriver must change this method if isSkinnable(). called by CSkeletonModel::bindSkin()
	 *	NB: _FatherSkeletonModel is valid when setApplySkin() is called
	 */
	virtual	void			setApplySkin(bool state) {}


	// The SkeletonModel, root of us (skinning or sticked object). NULL , if normal mode.
	CSkeletonModel	*_FatherSkeletonModel;
	// If sticked object, id of the bone in the _FatherSkeletonModel.
	uint			_FatherBoneId;

	// @}

	/// name Lighting Behavior.
	// @{

	/// The contribution of all lights. This enlarge the struct only of approx 15%
	CLightContribution		_LightContribution;

	/// true if the object needs to updatelighting.
	bool					_NeedUpdateLighting;
	/// true if the object has a FrozenStaticLightSetup not correclty updated.
	bool					_NeedUpdateFrozenStaticLightSetup;


	/// true (default) if the user agree that the object is lightable.
	bool					_UserLightable;


	/// each transform may be in a quadGird of lighted models (see CLightingManager)
	CLightingManager::CQGItLightedModel		_LightedModelIt;

	// @}


	/** get the channelMixer owned by the transform. return result of a refPtr => may be NULL.
	 */
	CChannelMixer			*getChannelMixer() const {return _ChannelMixer;}

private:
	static IModel	*creator() {return new CTransform;}
	friend class	CTransformHrcObs;
	friend class	CTransformClipObs;
	friend class	CTransformLightObs;
	friend class	CTransformAnimDetailObs;
	friend class	CSkeletonModel;
	friend class	CSkeletonModelAnimDetailObs;

	/** For Skeleton Object Stick.
	 *	update the wolrd matrix. no-op if skinned.
	 *	use standard father WorldMatrix if !_FatherSkeletonModel else get the correct boneId 
	 *	WorldMatrix from _FatherSkeletonModel
	 */
	void			updateWorldMatrixFromFather();


	// For anim detail.
	NLMISC::CRefPtr<CChannelMixer>		_ChannelMixer;

	// Last date of ITransformable matrix.
	uint64			_LastTransformableMatrixDate;

	// Information of transparency
	bool			_Opaque;
	bool			_Transparent;

	CInstanceGroup* _ClusterSystem;


	enum	TFreezeHRCState	{ FreezeHRCStateDisabled=0, FreezeHRCStateRequest, FreezeHRCStateReady, FreezeHRCStateEnabled};
	TFreezeHRCState			_FreezeHRCState;

	// For fast clip.
	bool			_QuadGridClipManagerEnabled;

	uint8				_OrderingLayer;


	/// See ILogicInfo. Used for lighting.	default is NULL.
	ILogicInfo			*_LogicInfo;


protected:
	// shortcut to the HrcObs.
	CTransformHrcObs	*_HrcObs;
	// shortcut to the ClipObs.
	CTransformClipObs	*_ClipObs;
	// shortcut to the LightObs.
	CTransformLightObs	*_LightObs;

};


// ***************************************************************************
/**
 * This observer:
 * - implement the notification system (just the update() method).
 * - implement the traverse() method.
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformHrcObs : public IBaseHrcObs
{
public:
	CTransformHrcObs()
	{
		Frozen = false;
		DontUnfreezeChildren = false;
		_AncestorSkeletonModel= NULL;
		ClipLinkedInSonsOfAncestorSkeletonModelGroup= false;
	}

	virtual	void	update();

	/// \name Utility methods.
	//@{
	/// Update the world state according to the parent world state and the local states.
	void	updateWorld(IBaseHrcObs *caller);
	//@}


	/// \name The base doit method.
	//@{
	/// The base behavior is to update() the observer, updateWorld() states, and traverseSons().
	virtual	void	traverse(IObs *caller);
	//@}
	static IObs	*creator() {return new CTransformHrcObs;}


public:
	bool	Frozen;
	bool	DontUnfreezeChildren; // Usefull when cluster system move to not test instance again

	bool	ClipLinkedInSonsOfAncestorSkeletonModelGroup;
	// !NULL if any skeleton is an ancestor in hierarchy. Updated at each Hrc traversal!!
	CSkeletonModel	*_AncestorSkeletonModel;

private:
	// according to _AncestorSkeletonModel, link clipTrav.
	void	updateClipTravForAncestorSkeleton();
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the clip() method to return true (not renderable)
 * - implement traverse() method.
 *
 * \sa CHrcTrav IBaseClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformClipObs : public IBaseClipObs
{

public:

	sint64 Date;

public:

	CTransformClipObs() : Date(0) {}

	/// don't render.
	virtual	bool	isRenderable() const {return false;}

	/// Don't clip.
	virtual	bool	clip(IBaseClipObs *caller) 
	{
		return true;
	}

	/** The base doit method.
	 * The behavior is to:
	 *	- test if HrcObs->WorldVis is visible.
	 *	- test if the observer is clipped with clip() OR IF SKELETON MODEL, USE SKELETON MODEL clip!!
	 *	- if visible and not clipped, set \c Visible=true (else false). and
	 *		- add the <CTransformClipObs*> to the ClipTrav list
	 *	- if Visible==true, and renderable, add it to the RenderTraversal: \c RenderTrav->addRenderObs(RenderObs);
	 *	- always traverseSons(), to clip the sons.
	 */
	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CTransformClipObs;}
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse method.
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformAnimDetailObs : public IBaseAnimDetailObs
{
public:

	/// update the WorldMatrix if needed.
	void			updateWorldMatrixFromFather();
	/** traverse without updatin WorldMatrixFromFather:
	 *	- animdetail if the model channelmixer is not NULL, and if model not clipped
	 */
	void			traverseWithoutUpdateWorldMatrix(IObs *caller);

	/// call updateWorldMatrixFromFather(), then traverseWithoutUpdateWorldMatrix()
	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CTransformAnimDetailObs;}
};

// ***************************************************************************
/**
 * \sa CTransformRenderObs
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CTransformRenderObs : public IBaseRenderObs
{
public:
	/** 
	 * The base render method.
	 * The observers should not traverseSons(), for speed improvement.
	 */
	virtual	void	traverse(IObs *caller)
	{
	}

	/**
	 * To avoid dynamic casting in mot fault of yoyo
	 */
	virtual CTransform* getTransformModel()
	{
		return static_cast<CTransform*>(Model);
	}

	static IObs	*creator() {return new CTransformRenderObs;}

};


// ***************************************************************************
/**
 * \sa CTransformLightObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTransformLightObs : public IBaseLightObs
{
public:

	/** 
	 * The base light method. This do all the good thing and should not be derived.
	 * traverse() is called only on visible objects with no _AncestorSkeletonModel, 
	 * It test if transform->_NeedUpdateLighting==true.
	 *
	 * The observers should not traverseSons(), for speed improvement.
	 */
	virtual	void	traverse(IObs *caller);


	static IObs	*creator() {return new CTransformLightObs;}

};


} // namespace NL3D


#endif // NL_TRANSFORM_H

/* End of transform.h */
