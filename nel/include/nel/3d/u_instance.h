/** \file u_instance.h
 * <File description>
 *
 * $Id: u_instance.h,v 1.16 2004/04/27 11:53:08 vizerie Exp $
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

#ifndef NL_U_INSTANCE_H
#define NL_U_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_transform.h"
#include "nel/misc/aabbox.h"


namespace NL3D
{


class	UInstanceMaterial;


// ***************************************************************************
/**
 * Game interface for manipulating Objects, animations etc...
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UInstance : virtual public UTransform
{
protected:

	/// \name Object
	// @{
	UInstance() {}
	virtual	~UInstance() {}
	// @}

public:


	/** Get the untransformed AABBox of the mesh. NULL (gtSize()==0) if no mesh.
	 */
	virtual void				getShapeAABBox(NLMISC::CAABBox &bbox) const =0;

	/**
	 * Set the blend shape factor for this instance
	 * blenShapeName is the name of the blendshape we want to set
	 * factor the blendshape percentage from -100.0 to 100.0
	 * dynamic tells the optimizer if the blendshape have to change in real time
	 */
	virtual void				setBlendShapeFactor (const std::string &blendShapeName, float factor, bool dynamic) { }

	/// \name Material access.
	// @{
	/// return number of materials this mesh instance use.
	virtual	uint				getNumMaterials() const =0;
	/// return a local access on a material, to change its values. (NB: overwrited, if animated).
	virtual	UInstanceMaterial	&getMaterial(uint materialId)=0;
	/** Select textures of material among several sets (if available)
	 *	NB: if success and if getAsyncTextureMode()==true, then setAsyncTextureDirty(true) is called
	 */
	virtual void selectTextureSet(uint id)=0;
	// @}

	/** Change MRM Distance setup. Only for mesh which support MRM. NB MeshMultiLod apply it only on Lod0 
	 *	(if Lod0 is a MRM).
	 *	NB: This apply to the shape direclty!! ie All instances using same shape will be affected
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest) =0;


	/** Change Max Display distance. After this distance the shape won't be displayed.
	 *	setting <0 means -1 and so means DistMax = infinite (default in meshs but multilod meshes).
	 *	NB: This apply to the shape direclty!! ie All instances using same shape will be affected
	 *
	 *	Note: If the instance is skinned/sticked to a skeleton, this setup is not taken into account. But you can
	 *	call USkeleton::setShapeDistMax() to have same effect.
	 *
	 *	Note (complex): All instances of the same shape which are freezeHRC()-ed and are linked to the 
	 *	QuadGridClipManager (ie not linked to a cluster) may not be updated corretcly. 
	 *	In other words, you should setup this value only at beginning of program, just after creating your 
	 *	instance (more exactly before UScene::render()), and all instances of the same shape should be setuped 
	 *	with same value (or don't call setShapeDistMax() for subsequent instances).
	 *	If you don't do this, QuadGridClipManager may clip such instances nearer than they should
	 */
	virtual void		setShapeDistMax(float distMax) =0;

	/// see setShapeDistMax()
	virtual float		getShapeDistMax() const =0;

	/// Test if there is a start/stop caps in the objects (some fxs such as remanence)
	virtual bool		canStartStop() = 0;
	// For instance that have a start/stop caps
	virtual void		start() = 0;
	// For instance that have a start/stop caps
	virtual void		stop()  = 0;
	// For instance that have a start/stop caps
	virtual bool		isStarted() const = 0;
	
	// Get the model distmax.
	virtual float               getDistMax() const = 0;	
	// Set the model distmax.
	virtual void                setDistMax(float distMax) = 0;
	// If the model has a coarse mesh, it set its dist. Set to -1 to keep default
	virtual void                setCoarseMeshDist(float dist) = 0;
	// If the model has a coarse mesh, it returns its distance if it has been set, or -1 if default is used (or if no coarse mesh present)
	virtual float               getCoarseMeshDist() const = 0;


	/// \name Async Texture Loading
	/// All those methods assert if the instance is not a CMeshBaseInstance.
	// @{
	/** if true, the instance is said in "AsyncTextureMode". Ie user must fill AsyncTextures field with name of the
	 *	textures to load. At each startAsyncTextureLoading(), the system start to load async them.
	 *	Then, isAsyncTextureReady() should be test each frame, to know if loading has completed.
	 *	By default, AsyncTextureMode=false. 
	 *	When it swap from false to true, each texture file in Materials are replaced with 
	 *	"blank.tga", and true fileNames are copied into AsyncTextures.
	 *	When it swap from true to false, the inverse is applied.
	 *	NB: calling enableAsyncTextureMode(true) calls setAsyncTextureDirty(true)
	 */
	virtual	void		enableAsyncTextureMode(bool enable) =0;
	virtual	bool		getAsyncTextureMode() const =0;
	/** Start to load all textures in AsyncTextures array (if needed)
	 *	NB: old setup is kept in Material => instance is still rendered with "coherent" textures, until new textures
	 *	are ready
	 *	no op if not in async texture mode.
	 */
	virtual	void		startAsyncTextureLoading() =0;
	/**	return true if all the async textures of the instances are uploaded.
	 *	if was not ready before, this swap the upload textures into the rendered ones so they are rendered
	 *	return always true if not in async texture mode, or if startAsyncTextureLoading() has not been called
	 *	since last enableAsyncTextureMode(true)
	 */
	virtual	bool		isAsyncTextureReady() =0;

	/** For Lod of texture, and load balancing, set the approximate distance of the instance to the camera.
	 */
	virtual	void		setAsyncTextureDistance(float dist) =0;
	/** \see setAsyncTextureDistance()
	 */
	virtual	float		getAsyncTextureDistance() const =0;

	/** User is free to flag this state, to know if startAsyncTextureLoading() should be called. 
	 *	Internal system don't use this flag. 
	 *	Default is false
	 */
	virtual	void		setAsyncTextureDirty(bool flag) =0;
	/// see dirtAsyncTextureState()
	virtual	bool		isAsyncTextureDirty() const =0;

	// @}

	/** Trails specific. Set the slice time (period used to sample the trail pos)
	  * If the object is not a trail, this has no effect
	  */
	// @{
	virtual void		setSliceTime(float duration) = 0;
	virtual float		getSliceTime() const = 0;
	// @}

	/** Test if driver support rendering of all material of that shape.
	  * \param  forceBaseCaps When true, the driver is considered to have the most basic required caps (2 stages hardwares, no pixelShader), so that any fancy material will fail the test.
	  */
	virtual	bool		supportMaterialRendering(UDriver &drv, bool forceBaseCaps) = 0;

};


} // NL3D


#endif // NL_U_INSTANCE_H

/* End of u_instance.h */
