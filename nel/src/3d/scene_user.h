/** \file scene_user.h
 * <File description>
 *
 * $Id: scene_user.h,v 1.38 2003/03/13 14:15:51 berenguier Exp $
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

#ifndef NL_SCENE_USER_H
#define NL_SCENE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_scene.h"
#include "3d/driver_user.h"
#include "3d/camera_user.h"
#include "3d/instance_user.h"
#include "3d/particle_system_instance_user.h"
#include "3d/landscape_user.h"
#include "3d/cloud_scape_user.h"
#include "3d/instance_group_user.h"
#include "3d/skeleton_user.h"
#include "3d/visual_collision_manager_user.h"
#include "3d/animation_set_user.h"
#include "3d/play_list_manager_user.h"

#define NL3D_MEM_SCENE						NL_ALLOC_CONTEXT( 3dScene )
#define NL3D_MEM_SCENE_INIT					NL_ALLOC_CONTEXT( 3dScIni )
#define NL3D_MEM_SCENE_INIT0				NL_ALLOC_CONTEXT( 3dScIn0 )
#define NL3D_MEM_SCENE_INIT1				NL_ALLOC_CONTEXT( 3dScIn1 )
#define NL3D_MEM_SCENE_INIT2				NL_ALLOC_CONTEXT( 3dScIn2 )
#define NL3D_MEM_SCENE_INIT3				NL_ALLOC_CONTEXT( 3dScIn3 )
#define NL3D_MEM_SCENE_INIT4				NL_ALLOC_CONTEXT( 3dScIn4 )
#define NL3D_MEM_SCENE_INIT5				NL_ALLOC_CONTEXT( 3dScIn5 )
#define NL3D_MEM_SCENE_INIT6				NL_ALLOC_CONTEXT( 3dScIn6 )

namespace NL3D {


/**
 * <Class description>
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSceneUser : public UScene
{
private:
	/// update async loading during a call to render	  
	virtual void			updateWaitingInstances();
protected:
	/// The driver which owns this scene.
	CDriverUser			*_DriverUser;
	/// The Scene.
	CScene				_Scene;
	/// The Current camera.
	CCameraUser			*_CurrentCamera;


	// Components List.
	typedef	CPtrSet<CTransformUser>		TTransformSet;
	typedef	CPtrSet<CLandscapeUser>		TLandscapeSet;
	typedef	CPtrSet<CCloudScapeUser>	TCloudScapeSet;
	typedef	CPtrSet<CInstanceGroupUser>	TInstanceGroupSet;
	typedef	CPtrSet<CVisualCollisionManagerUser>	TVisualCollisionManagerSet;
	typedef	CPtrSet<CAnimationSetUser>		TAnimationSetSet;
	typedef	CPtrSet<CPlayListManagerUser>	TPlayListManagerSet;
	TTransformSet				_Transforms;
	TLandscapeSet				_Landscapes;
	TCloudScapeSet				_CloudScapes;
	TInstanceGroupSet			_InstanceGroups;
	TVisualCollisionManagerSet	_VisualCollisionManagers;
	TAnimationSetSet			_AnimationSets;
	TPlayListManagerSet			_PlayListManagers;

	std::map<UInstance**,CTransformShape*> _WaitingInstances;

	// IG that wait to be created and added to that scene
	struct CWaitingIG
	{
		UInstanceGroup	*IGToLoad;
		UInstanceGroup	**CallerPtr;
		NLMISC::CVector Offset;
		CWaitingIG(UInstanceGroup **callerPtr, const NLMISC::CVector &offset)	: IGToLoad(NULL), CallerPtr(callerPtr), Offset(offset)
		{}
	};	
	//
	typedef std::list<CWaitingIG> TWaitingIGList;
	TWaitingIGList		  _WaitingIGs;

public:

	/// \name Object
	// @{
	CSceneUser(CDriverUser *drv);
	virtual	~CSceneUser();
	// @}

public:

	/// \name Render
	//@{
	virtual	void			render();
	// update async loading whithout a call to render
	virtual void			updateWaitingInstances(double ellapsedTime);
	virtual	void			animate(TGlobalAnimationTime time);	
	//@}


	/// \name Camera/Viewport.
	//@{
	virtual	void			setCam(UCamera *cam);
	virtual	UCamera			*getCam();
	virtual	void			setViewport(const class CViewport& viewport);
	virtual	CViewport		getViewport();
	//@}


	/// \name Component Mgt.
	//@{

	virtual	UCamera			*createCamera();
	virtual	void			deleteCamera(UCamera *cam);

	virtual	UInstance		*createInstance(const std::string &shapeName);
	virtual	void			createInstanceAsync(const std::string &shapeName, UInstance**ppInstance);
	virtual	void			deleteInstance(UInstance *inst);

	virtual	void createInstanceGroupAndAddToSceneAsync (const std::string &instanceGroup, UInstanceGroup **pIG, const NLMISC::CVector &offset);

	virtual	void stopCreatingAndAddingIG(UInstanceGroup **pIG);

	/// should be called at each render
	void	updateWaitingIG();


	virtual UTransform		*createTransform();
	virtual	void			deleteTransform(UTransform *tr);

	virtual	USkeleton		*createSkeleton(const std::string &shapeName);
	virtual	void			deleteSkeleton(USkeleton *skel);

	virtual	ULandscape		*createLandscape();
	virtual	void			deleteLandscape(ULandscape *land);

	virtual	UCloudScape		*createCloudScape();
	virtual	void			deleteCloudScape(UCloudScape *cs);
/*

	virtual	UInstanceGroup	*createInstanceGroup (const std::string &instanceGroup);
	virtual	void			deleteInstanceGroup (UInstanceGroup	*group);
*/

	virtual void setToGlobalInstanceGroup(UInstanceGroup *pIG);

	virtual	UPointLight		*createPointLight();
	virtual	void			deletePointLight(UPointLight *light);

	//@}

	/// \name Animation.
	//@{
	//@}
	/// \name Animation gestion.
	// @{
	/// Create an empty AnimationSet.
	virtual	UAnimationSet	*createAnimationSet();
	/// Create a new AnimationSet, load it from a file. Use CPath to search the animation set. exception EPathNotFound if not found.
	virtual	UAnimationSet	*createAnimationSet(const std::string &animationSetFile);
	/// Delete a AnimationSet.
	virtual	void			deleteAnimationSet(UAnimationSet *animationSet);
	/** Set the automatic animation set used by the scene.
	  */
	virtual void				setAutomaticAnimationSet(UAnimationSet *as);	
	/// Create a new PlayListManager.
	virtual	UPlayListManager	*createPlayListManager() ;
	/// Delete a PlayListManager.
	virtual	void				deletePlayListManager(UPlayListManager *playListManager) ;
	// @}

	/// \name Visual Collision manager.
	//@{
	virtual	UVisualCollisionManager		*createVisualCollisionManager();
	virtual	void						deleteVisualCollisionManager(UVisualCollisionManager *mgr);
	//@}


	/// \name LoadBalancing mgt.
	//@{
	virtual	void					setPolygonBalancingMode(TPolygonBalancingMode polBalMode);
	virtual	TPolygonBalancingMode	getPolygonBalancingMode() const;

	virtual float				getNbFaceAsked () const;

	virtual	void				setGroupLoadMaxPolygon(const std::string &group, uint nFaces);
	virtual	uint				getGroupLoadMaxPolygon(const std::string &group);
	virtual float				getGroupNbFaceAsked (const std::string &group) const;

	virtual void				setMaxSkeletonsInNotCLodForm(uint m);
	virtual uint				getMaxSkeletonsInNotCLodForm() const;

	//@}

	/// \name Coarse meshes mgt.
	//@{
	virtual void				setCoarseMeshManagerTexture (const char *sPath);
	virtual void				setCoarseMeshLightingUpdate(uint8 period);
	virtual uint8				getCoarseMeshLightingUpdate() const;
	//@}

	/// \name transparent Layers mgt
	//@{
	virtual void  setLayersRenderingOrder(bool directOrder = true) { _Scene.setLayersRenderingOrder(directOrder); }
	virtual bool  getLayersRenderingOrder() const { return _Scene.getLayersRenderingOrder(); }
	//@}

	/// \name Global light setup.
	//@{

	virtual	void				enableLightingSystem(bool enable);

	virtual	void				setAmbientGlobal(NLMISC::CRGBA ambient);
	virtual	void				setSunAmbient(NLMISC::CRGBA ambient);
	virtual	void				setSunDiffuse(NLMISC::CRGBA diffuse);
	virtual	void				setSunSpecular(NLMISC::CRGBA specular);
	virtual	void				setSunDirection(const NLMISC::CVector &direction);

	virtual	NLMISC::CRGBA		getAmbientGlobal() const;
	virtual	NLMISC::CRGBA		getSunAmbient() const;
	virtual	NLMISC::CRGBA		getSunDiffuse() const;
	virtual	NLMISC::CRGBA		getSunSpecular() const;
	virtual	NLMISC::CVector		getSunDirection() const;

	virtual	void				setMaxLightContribution(uint nlights);
	virtual	uint				getMaxLightContribution() const;

	virtual	void				setLightTransitionThreshold(float lightTransitionThreshold);
	virtual	float				getLightTransitionThreshold() const;

	//@}


	/// \name Weather mgt
	//@{

	virtual void				setGlobalWindPower(float gwp);
	virtual float				getGlobalWindPower() const;
	virtual void				setGlobalWindDirection(const NLMISC::CVector &gwd);
	virtual const NLMISC::CVector		&getGlobalWindDirection() const;

	//@}


	/// \name CLod / Character Lod mgt
	//@{
	virtual void				resetCLodManager();
	virtual uint32				loadCLodShapeBank(const std::string &fileName);
	virtual void				deleteCLodShapeBank(uint32 bankId);
	virtual sint32				getCLodShapeIdByName(const std::string &name) const;
	virtual sint32				getCLodAnimIdByName(uint32 shapeId, const std::string &name) const;
	//@}

	/// \name Profiling and Render Filtering
	// @{
	virtual	void				enableElementRender(TRenderFilter elt, bool state);
	virtual void				profileNextRender();
	virtual void				getProfileResults(CBenchResults &results);
	// @}

public:
	/// \name Accessor for CSceneUser.
	// @{
	CScene		&getScene()
	{
		NL3D_MEM_SCENE
		return _Scene;
	}
	// @}

};


} // NL3D


#endif // NL_SCENE_USER_H

/* End of scene_user.h */
