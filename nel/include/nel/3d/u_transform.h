/** \file u_transform.h
 * <File description>
 *
 * $Id: u_transform.h,v 1.20 2003/12/01 09:39:48 vizerie Exp $
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

#ifndef NL_U_TRANSFORM_H
#define NL_U_TRANSFORM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/3d/u_instance_group.h"
#include "nel/3d/u_transformable.h"


namespace NL3D 
{

using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CQuat;

class	ILogicInfo;
class	CCluster;

// ***************************************************************************
/**
 * Base interface for manipulating Movable Objects in the scene: camera, lights, instances etc...
 * see UTransformable. A UTransform can be set in a hierachy, and can be hidden.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UTransform : virtual public UTransformable
{
protected:

	/// \name Object
	// @{
	/// Constructor. By default, RotQuat mode.
	UTransform() {}
	virtual	~UTransform() {}
	// @}

public:
	// Enum should be the same than in CHrcTrav.

	/// The visibility flag. In the root case, Herit means Show.
	enum	TVisibility
	{
		Show=0,		// The model is shown in the hierarchy
		Hide,		// The model is hidden in the hierarchy
		Herit,		// The model herit the visibilty from his father

		VisibilityCount
	};


public:


	/// \name Hierarchy manipulation
	// @{
	/** Hierarchy edit. unlink this from oldparent, and make this be a son of newFather.
	 * if this was already a son of newFather, no-op.
	 * \param newFather the new Father. If NULL, the transform will be linked to the root of the hierarchy (Default!).
	 */
	virtual	void			parent(UTransform *newFather)=0;


	virtual void			setClusterSystem (UInstanceGroup *pIG)=0; 
	virtual UInstanceGroup *getClusterSystem ()=0;
// @}


	/// \name visibility
	// @{
	/// Hide the object and his sons.
	virtual	void			hide()=0;
	/// Show the objet and his sons.
	virtual	void			show()=0;
	/*
	 *	Enable / disable user clipping. If enable, the transform is not clipped into the engine. 
	 *  The user has to use show / hide to clip or not the transform.
	 */
	virtual void			setUserClipping(bool enable)=0;
	/// Return the user clipping state
	virtual bool			getUserClipping() const=0;
	/// herit the visibility from his father. (default behavior).
	virtual	void			heritVisibility()=0;
	/// Get the local visibility state.
	virtual	TVisibility		getVisibility()=0;
	// @}


	/// \name freezeHRC
	// @{
	/** Optimisation: freeze the HRC so the WorldMatrix computed at next render() will be kept for long, and 
	 *	the model won't either be tested in HRC.
	 *
	 *	NB: the model won't be tested in HRC only if this model is a "root"
	 *	For maximum optimisation, you should freezeHRC() all the models of a hierarchy, from base root to leaves.
	 *
	 *	NB: if the hierarchy of this object must change, or if the object must moves, you must call unfreezeHRC() first,
	 *	and you should do this for all the parents of this model.
	 */
	virtual	void			freezeHRC()=0;


	/**	see freezeHRC().
	 */
	virtual	void			unfreezeHRC()=0;
	// @}


	/// name Lighting Behavior.
	// @{
	/** Set the UserLightable flag. if false, isLightable() will always return false.
	 *	Doing this, user can disable lighting on a model which may be interesting for speed.
	 *	NB: most of models don't need it. For example models with LightMaps are by default Lighing-disabled.
	 *	Default behavior is UserLightable==true.
	 */
	virtual	void			setUserLightable(bool enable) =0;

	/** Get the UserLightable flag.
	 */
	virtual	bool			getUserLightable() const  =0;
	// @}


	/** Set the current ordering layer for this transform.
	  * Typically, this is used to sort transparent objects. Isn't used with solid objects.
	  * For now :
	  * Layer 0 is for underwater
	  * Layer 1 is for water surfaces
	  * Layer 2 is for object above water
	  */
	virtual void			setOrderingLayer(uint layer) = 0;

	/// Get the ordering layer
	virtual uint			getOrderingLayer() const = 0;


	/** Set the LogicInfo for this transfrom, eg to retrieve statc light information, see ILogicInfo.
	 *	Ptr is kept in UTransfrom, so should call setLogicInfo(NULL) before to clean up.
	 */
	virtual void			setLogicInfo(ILogicInfo *logicInfo) =0;


	/// Return true if the object was determined as Visible in Hrc during the last Scene->rendere(). NB: interesting only if Herit. else can use getVisibility()
	virtual bool			getLastWorldVisState() const = 0;

	/// Return true if the object was rendered during the last Scene->rendere(). return false else (ie clipped)
	virtual bool			getLastClippedState() const = 0;

	/// Fill a list of cluster that contain this tranform. This is valid after the clip traversal
	virtual void			getLastParentClusters(std::vector<CCluster*> &clusters) const = 0;

	/** get the last world matrix computed in last render().
	 *	NB: this WM is computed in last render() only if the object was not clipped. So use it wisely.
	 *	use getLastClippedState() to konw if the object was visible in last render().
	 */
	virtual	const CMatrix	&getLastWorldMatrixComputed() const =0;


	/// name Load Balancing Behavior.
	// @{

	/** Change the load Balancing group of a model. Every models are in a special LoadBalancingGroup.
	 *	NB: the group is  created if did not exist.
	 *	NB: if models are skinned, it is their Skeleton which drive the group
	 *
	 *	By default, models lies in the "Default" group, but Skeletons for skinning and ParticlesSystems which
	 *	are in "Skin" and "Fx" group respectively.
	 *	The "Default" group is special because it is not balanced (ie models are only degraded from 
	 *	their distance to camera)
	 */
	virtual void			setLoadBalancingGroup(const std::string &group) =0;

	/** Get the load Balancing group of a model. see setLoadBalancingGroup().
	 */
	virtual const std::string	&getLoadBalancingGroup() const =0;

	// @}


	/// name Misc
	// @{

	/** set the Mean color of the transform. The mean color can be used for many purpose, such as drawing
	 *	objects if the textures are not loaded. It is used also for Lod Character.
	 *	Default color is (255,255,255)
	 */
	virtual	void			setMeanColor(NLMISC::CRGBA color) =0;

	/// see setMeanColor()
	virtual	NLMISC::CRGBA	getMeanColor() const  =0;

	// @}

	/// name Accessors for opacity/transparency
	// @{	
	virtual void			setTransparency(bool v) = 0;
	virtual void			setOpacity(bool v) = 0;
	// return a non-zero value if true
	virtual uint32			isOpaque() = 0;
	virtual uint32			isTransparent() = 0;
	// @}

	/// \name ShadowMapping
	// @{
	/** By default, map shadow casting is disabled. This enabled shadow for this model. 
	 *	Fails if the model don't support dynamic Map Shadow Casting (eg landscape)
	 */
	virtual void			enableCastShadowMap(bool state) =0;
	/// true if the instance cast shadow. By default false
	virtual bool			canCastShadowMap() const =0;

	/** By default, map shadow receiving is disabled. This enabled shadow for this model. 
	 *	Fails if the model don't support dynamic Map Shadow Receiving (eg Particle system)
	 */
	virtual void			enableReceiveShadowMap(bool state) =0;
	/// true if the instance receive shadow. By default false
	virtual bool			canReceiveShadowMap() const =0;
	// @}
	
	/** Force the transform to always be attached to the root
	  * As a consequence, it can't be inserted into a cluster system (even the root cluster)
	  * and is thus always visible when in the frustum (not clusterized)
	  * NB : any call to setClusterSystem will be ignored (must remain unclesterized)
	  * NB : any call to parent will be ignored (must remain linked to the root)
      */
	virtual void		setForceClipRoot(bool forceClipRoot) = 0;
	virtual bool		getForceClipRoot() const = 0;
	

};


} // NL3D


#endif // NL_U_TRANSFORM_H

/* End of u_transform.h */
