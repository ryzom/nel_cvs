/** \file u_scene.h
 * <File description>
 *
 * $Id: u_scene.h,v 1.11 2001/10/10 15:36:45 besson Exp $
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

#ifndef NL_U_SCENE_H
#define NL_U_SCENE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/animation_time.h"


namespace NLMISC
{
class CRGBA;
};

namespace NL3D
{


class UCamera;
class UInstance;
class ULandscape;
class UInstanceGroup;
class USkeleton;
class UTransform;
class UVisualCollisionManager;
class UAnimationSet;
class UPlayListManager;


//****************************************************************************
/**
 * Game Interface for Scene.
 * A UScene create instances, camera lights etc...  Those objects are not sharable through different scenes. You
 * cannot use a camera create with Scene1, for viewing Scene2.
 *
 * The scene is created with a default camera that you can retrieve with getCam().
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UScene
{
protected:

	/// \name Object
	// @{
	UScene() {}
	virtual	~UScene() {}
	// @}

public:

	/// \name Render
	//@{
	/** Render the scene from the CurrentCamera view (setCam()), and the current Viewport. nlError if no current camera.
	 * NB: no Driver clear buffers (color or ZBuffer) are done.... \n
	 * NB: The UDriver Matrix/Viewport context for 2D/3D interface is restored after this render.
	 * NB: nlerror if the current camera has been deleted.
	 */
	virtual	void			render()=0;

	/**
	 *	Set the animation time for lightmap and Particle System animation.
	 */
	virtual	void			animate(double time)=0;

	//@}


	/// \name Camera/Viewport.
	//@{
	/// Set the current camera for this scene. nlError if NULL, or if the camera was not created by this UScene.
	virtual	void			setCam(UCamera *cam)=0;
	/// Get the current camera.
	virtual	UCamera			*getCam()=0;
	/// Set the current Viewport.
	virtual	void			setViewport(const class CViewport& viewport)=0;
	/// Get the current Viewport.
	virtual	CViewport		getViewport()=0;
	//@}


	/// \name Component Mgt.
	/** A Uscene is a factory of Instance/Landscape/Lights etc...
	 *
	 */
	//@{

	/// Create a camera. Usefull for this scene only.
	virtual	UCamera			*createCamera()=0;
	/// Delete a camera.
	virtual	void			deleteCamera(UCamera *cam)=0;

	/** Create an instance of the shape "shapename". If not present, try to load "shapename" via CPath.
	 * If fails (file not found), return NULL.
	 * nlerror if the file is not a mesh file.
	 */
	virtual	UInstance		*createInstance(const std::string &shapeName)=0;
	/** Same as createInstance but the instance is loaded asynchronously.
	 * You must poll to know if the instance if created by calling render()
	 */
	virtual	void			createInstanceAsync(const std::string &shapeName,UInstance**ppInstance)=0;
	/** Create a dummy object
	 */
	virtual	UTransform		*createTransform()=0;
	/** Delete an instance via his pointer.
	 */
	virtual	void			deleteInstance(UInstance *inst)=0;

	/** Create a skeleton instance of the skeleton shape "shapename". If not present, try to load "shapename" via CPath.
	 * If fails (file not found), return NULL.
	 * nlerror if the file is not a skeleton shape file.
	 */
	virtual	USkeleton		*createSkeleton(const std::string &shapeName)=0;
	/** Delete a skeleton instance via his pointer.
	 */
	virtual	void			deleteSkeleton(USkeleton *skel)=0;

	/// Create a landscape. There could be many Landscape per Scene (Driver? :). But never tested :).
	virtual	ULandscape		*createLandscape()=0;
	/// delete a landscape.
	virtual	void			deleteLandscape(ULandscape *land)=0;

	/** Assign the Instance Group to the root cluster
	  * 
	  */
	virtual	void setToGlobalInstanceGroup(UInstanceGroup *pIGU) = 0;
	//virtual	UInstanceGroup	*createInstanceGroup(const std::string &instanceGroup) =0;
	/// Delete an instance group from this scene.
	//virtual	void			deleteInstanceGroup(UInstanceGroup	*group) =0;
	//@}

	/// \name Animation gestion.
	// @{
	/// Create an empty AnimationSet.
	virtual	UAnimationSet		*createAnimationSet() =0;
	/// Create a new AnimationSet, load it from a file. Use CPath to search the animation set.  exception EPathNotFound if not found.
	virtual	UAnimationSet		*createAnimationSet(const std::string &animationSetFile) =0;
	/// Delete a AnimationSet. NB: actually, this animation set is internally deleted only when no more UPlayList use it.
	virtual	void				deleteAnimationSet(UAnimationSet *animationSet) =0;
	/// Create a new PlayListManager.
	virtual	UPlayListManager	*createPlayListManager() =0;
	/// Delete a PlayListManager.
	virtual	void				deletePlayListManager(UPlayListManager *playListManager) =0;
	// @}

	/// \name Visual Collision manager.
	//@{
	virtual	UVisualCollisionManager		*createVisualCollisionManager() =0;
	virtual	void						deleteVisualCollisionManager(UVisualCollisionManager *mgr) =0;
	//@}


	/// \name LoadBalancing mgt.
	//@{

	/** The mode of polygon balancing.
	 * PolygonBalancingOff => Models will be rendered with the number of faces they want to render.
	 * PolygonBalancingOn  => Models will be rendered with the number of faces the LoadBalancing want.
	 * PolygonBalancingClamp => Same as PolygonBalancingOn, but factor <= 1, ie models won't be rendered
	 *	with more face they want to render.
	 */
	enum			TPolygonBalancingMode {PolygonBalancingOff=0, PolygonBalancingOn, PolygonBalancingClamp, CountPolygonBalancing };

	/// Set the PolygonBalancingMode
	virtual	void				setPolygonBalancingMode(TPolygonBalancingMode polBalMode) =0;
	/// Get the PolygonBalancingMode
	virtual	TPolygonBalancingMode	getPolygonBalancingMode() const =0;


	/// Setup the number of faces max you want, (for Shapes only, not landscape)
	virtual	void				setLoadMaxPolygon(uint nFaces) =0;
	virtual	uint				getLoadMaxPolygon() const =0;

	/// Get the last face count asked from the instances before reduction.
	virtual float				getNbFaceAsked () const =0;

	//@}

	/// \name Coarse meshes mgt.
	//@{

	/** Set the static coarse meshes's common texture.
	*/
	virtual void setStaticCoarseMeshManagerTexture (const char *sPath) =0;

	/** Set the dynamic coarse meshes's common texture.
	*/
	virtual void setDynamicCoarseMeshManagerTexture (const char *sPath) =0;

	/** Set the static coarse meshes global color.
	*/
	virtual void setStaticCoarseMeshManagerColor (const NLMISC::CRGBA& color) =0;

	/** Set the dynamic coarse meshes's common texture.
	*/
	virtual void setDynamicCoarseMeshManagerColor (const NLMISC::CRGBA& color) =0;

	//@}
};


} // NL3D


#endif // NL_U_SCENE_H

/* End of u_scene.h */
