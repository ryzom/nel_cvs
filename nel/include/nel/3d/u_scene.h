/** \file u_scene.h
 * <File description>
 *
 * $Id: u_scene.h,v 1.22 2002/06/10 09:30:51 berenguier Exp $
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
class CVector;
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
class UPointLight;


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
	 * NB: the UDriver Light setup (see UDriver::setLight() / UDriver::setAmbientColor()) is modified.
	 *	At the exit of render(), all UDriver lights are disabled.
	 */
	virtual	void			render()=0;

	/**
	 *	Set the animation time for lightmap and Particle System animation.
	 */
	virtual	void			animate (TGlobalAnimationTime time) =0;

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
	/** Delete an instance via his pointer.
	 */
	virtual	void			deleteInstance(UInstance *inst)=0;

	/** create an instance group asynchronously, and add it to a scene asynchronously once it has been 
	  * loaded. The instance groups that must be added to the scene are checked at each render.
	  * The pIG pointer is filled once the instance group has been loaded and add to the scene
	  */
	virtual	void createInstanceGroupAndAddToSceneAsync (const std::string &instanceGroup,														
													    UInstanceGroup **pIG,
														const NLMISC::CVector &offset
													   ) = 0;
	
	/**	Stop loading and adding an ig to the scene
	  */
	virtual	void stopCreatingAndAddingIG(UInstanceGroup **pIG) = 0;
	

	/** Create a dummy object
	 */
	virtual	UTransform		*createTransform()=0;
	/** Delete a dummy object
	 */
	virtual	void			deleteTransform(UTransform *tr)=0;

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


	/// Create a dynamic PointLight. Usefull for this scene only.
	virtual	UPointLight		*createPointLight()=0;
	/// Delete a dynamic PointLight.
	virtual	void			deletePointLight(UPointLight *light)=0;


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
	/** Set the automatic animation set used by the scene.	  
	  */
	virtual void				setAutomaticAnimationSet(UAnimationSet *as) = 0;	
	// @}

	/// \name Visual Collision manager.
	//@{
	virtual	UVisualCollisionManager		*createVisualCollisionManager() =0;
	virtual	void						deleteVisualCollisionManager(UVisualCollisionManager *mgr) =0;
	//@}


	/// \name LoadBalancing mgt.
	//@{

	/** The mode of polygon balancing. NB: this apply to All LoadBalancingGroups, but the "Default" group
	 *	which is always considered as PolygonBalancingOff
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


	/** Setup the number of faces max you want. For backward compatibility only, this is same as
	 *	setGroupLoadMaxPolygon("Global", nFaces);
	 */
	virtual	void				setLoadMaxPolygon(uint nFaces) =0;
	/** Get the number of faces max you asked. For backward compatibility only, this is same as
	 *	getGroupLoadMaxPolygon("Global", nFaces);
	 */
	virtual	uint				getLoadMaxPolygon() =0;
	/** Get the last face count asked from the instances before reduction.
	 *	It gets the sum of All groups.
	 */
	virtual float				getNbFaceAsked () const =0;


	/** Set the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	virtual	void				setGroupLoadMaxPolygon(const std::string &group, uint nFaces) =0;
	/** Get the number of faces wanted for a LoadBlancingGroup.
	 *	The Group is created if did not exist.
	 */
	virtual	uint				getGroupLoadMaxPolygon(const std::string &group) =0;
	/** Get the last face count asked from the instances before reduction. only for the given group
	 *	return 0 if the Group does not exist.
	 */
	virtual float				getGroupNbFaceAsked (const std::string &group) const =0;

	//@}

	/// \name Coarse meshes mgt.
	//@{

	/** Set the static coarse meshes's common texture.
	*/
	virtual void				setStaticCoarseMeshManagerTexture (const char *sPath) =0;

	/** Set the dynamic coarse meshes's common texture.
	*/
	virtual void				setDynamicCoarseMeshManagerTexture (const char *sPath) =0;

	/**
	 *	Each coarseMesh lighting will be updated every "period" frame. clamped to 1,255
	 */
	virtual void				setCoarseMeshLightingUpdate(uint8 period) =0;

	/// see setCoarseMeshLightingUpdate()
	virtual uint8				getCoarseMeshLightingUpdate() const  =0;

	//@}


	/// \name Global light setup.
	//@{

	/** Enable Scene Lighting system. For backward compatibility, false by default.
	 *	If false, all objects will take last driver 's light setup
	 */
	virtual	void				enableLightingSystem(bool enable) =0;


	/// set the global Ambient used for the scene. Default to (50, 50, 50).
	virtual	void				setAmbientGlobal(NLMISC::CRGBA ambient) =0;
	/// set the Ambient of the Sun used for the scene.
	virtual	void				setSunAmbient(NLMISC::CRGBA ambient) =0;
	/// set the Diffuse of the Sun used for the scene.
	virtual	void				setSunDiffuse(NLMISC::CRGBA diffuse) =0;
	/// set the Specular of the Sun used for the scene.
	virtual	void				setSunSpecular(NLMISC::CRGBA specular) =0;
	/// set the Direction of the Sun used for the scene.
	virtual	void				setSunDirection(const NLMISC::CVector &direction) =0;


	/// get the global Ambient used for the scene. Default to (50, 50, 50).
	virtual	NLMISC::CRGBA		getAmbientGlobal() const =0;
	/// get the Ambient of the Sun used for the scene.
	virtual	NLMISC::CRGBA		getSunAmbient() const =0;
	/// get the Diffuse of the Sun used for the scene.
	virtual	NLMISC::CRGBA		getSunDiffuse() const =0;
	/// get the Specular of the Sun used for the scene.
	virtual	NLMISC::CRGBA		getSunSpecular() const =0;
	/// get the Direction of the Sun used for the scene.
	virtual	NLMISC::CVector		getSunDirection() const =0;

	/** setup the max number of point light that can influence a model. NB: clamped by NL3D_MAX_LIGHT_CONTRIBUTION
	 *	Default is 3.
	 *	NB: the sun contribution is not taken into account
	 */
	virtual	void				setMaxLightContribution(uint nlights) =0;
	/// \see setMaxLightContribution()
	virtual	uint				getMaxLightContribution() const =0;

	/** Advanced. When a model is influenced by more light than allowed, or when it reach the limits 
	 *	of the light (attenuationEnd), the light can be darkened according to some threshold.
	 *	The resultLightColor begin to fade when distModelToLight== attEnd- threshold*(attEnd-attBegin).
	 *	when distModelToLight== 0, resultLightColor==Black.
	 *	By default, this value is 0.1f. Setting higher values will smooth transition but will 
	 *	generally darken the global effects of lights.
	 *	NB: clamp(value, 0, 1);
	 */
	virtual	void				setLightTransitionThreshold(float lightTransitionThreshold) =0;
	/// \see getLightTransitionThreshold()
	virtual	float				getLightTransitionThreshold() const =0;


	//@}


	/// \name transparent Layers mgt
	//@{
		/** Set the order or rendering of layers containing transparent objects.
		  * In real case, with direct order, we have:
		  * - Underwater is rendered.
		  * - Water is rendered.
		  * - Objects above water are rendered.
		  */
		virtual void  setLayersRenderingOrder(bool directOrder = true) = 0;
		virtual bool  getLayersRenderingOrder() const = 0;
	//@}


	/// \name Weather mgt
	//@{

	/// Set the current windPower for all the scene. 0-1.
	virtual void				setGlobalWindPower(float gwp) =0;
	/// get the current windPower
	virtual float				getGlobalWindPower() const =0;

	/// Set the current windDirection for all the scene. dir.z set to 0 and vector normalized.
	virtual void				setGlobalWindDirection(const NLMISC::CVector &gwd) =0;
	/// get the current windDirection
	virtual const NLMISC::CVector		&getGlobalWindDirection() const =0;

	//@}


	/// \name CLod / Character Lod mgt
	//@{

	/// reset the manager.
	virtual void				resetCLodManager() =0;

	/** Load a Shape Bank. The ShapeMap is rebuilded. Hence slow call.
	 *	NB: a vector of ShapeBank is maintained internally, hence, not so many shapeBank should be 
	 *	created at same Time.
	 *	trhow exception if failed to load the file
	 *	\param fileName is a .clodbank file, to be loaded. CPath::lookup is used.
	 *	\return	id of the shape Bank.
	 */
	virtual uint32				loadCLodShapeBank(const std::string &fileName) =0;

	/// delete a Shape Bank. No-op if bad id.
	virtual void				deleteCLodShapeBank(uint32 bankId) =0;

	/** Get a shapeId by its name. -1 if not found.
	 */
	virtual sint32				getCLodShapeIdByName(const std::string &name) const =0;

	/** Get a AnimId of a shape by its name. -1 if not found.
	 */
	virtual sint32				getCLodAnimIdByName(uint32 shapeId, const std::string &name) const =0;

	//@}

};


} // NL3D


#endif // NL_U_SCENE_H

/* End of u_scene.h */
