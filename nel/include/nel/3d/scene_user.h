/** \file scene_user.h
 * <File description>
 *
 * $Id: scene_user.h,v 1.4 2001/04/13 16:39:03 berenguier Exp $
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
#include "nel/3d/tmp/u_scene.h"
#include "nel/3d/driver_user.h"
#include "nel/3d/camera_user.h"
#include "nel/3d/instance_user.h"
#include "nel/3d/landscape_user.h"
#include "nel/3d/instance_group_user.h"
#include "nel/3d/skeleton_user.h"


namespace NL3D {


/**
 * <Class description>
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSceneUser : public UScene
{
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
	typedef	CPtrSet<CInstanceGroupUser>	TInstanceGroupSet;
	TTransformSet		_Transforms;
	TLandscapeSet		_Landscapes;
	TInstanceGroupSet	_InstanceGroups;

public:

	/// \name Object
	// @{
	CSceneUser(CDriverUser *drv)
	{
		nlassert(drv);
		_DriverUser= drv;
		_CurrentCamera= NULL;

		// Init Scene.
		_Scene.initDefaultTravs();
		// Don't add any user trav.
		// init default Roots.
		_Scene.initDefaultRoots();
		// Set driver.
		_Scene.setDriver(_DriverUser->getDriver());
		// Set viewport
		_Scene.setViewport (CViewport());

		// Create default camera, and active!!
		setCam(createCamera());
	}
	virtual	~CSceneUser()
	{
		_Transforms.clear();
		_Landscapes.clear();
		_Scene.setDriver(NULL);
		_Scene.setCam(NULL);
		_Scene.release();
		_CurrentCamera= NULL;
		_DriverUser= NULL;
	}
	// @}

public:

	/// \name Render
	//@{
	virtual	void			render()
	{
		if(_CurrentCamera==NULL)
			nlerror("render(): try to render with no camera linked (may have been deleted)");
		_Scene.render();
		// Must restore the matrix context, so 2D/3D interface not disturbed.
		_DriverUser->restoreMatrixContext();
	}
	//@}


	/// \name Camera/Viewport.
	//@{
	virtual	void			setCam(UCamera *cam)
	{
		if(!cam)
			nlerror("setCam(): cannot set a NULL camera");
		CCameraUser		*newCam= dynamic_cast<CCameraUser*>(cam);
		if( newCam->getScene() != &_Scene)
			nlerror("setCam(): try to set a current camera not created from this scene");

		_CurrentCamera= newCam;
		_Scene.setCam(newCam->getCamera());
	}
	virtual	UCamera			*getCam()
	{
		return dynamic_cast<UCamera*>(_CurrentCamera);
	}
	virtual	void			setViewport(const class CViewport& viewport)
	{
		_Scene.setViewport(viewport);
	}
	virtual	CViewport		getViewport()
	{
		return _Scene.getViewport();
	}
	//@}


	/// \name Component Mgt.
	//@{

	virtual	UCamera			*createCamera()
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		return dynamic_cast<UCamera*>( _Transforms.insert(new CCameraUser(&_Scene)) );
	}
	virtual	void			deleteCamera(UCamera *cam)
	{
		CCameraUser		*oldCam= dynamic_cast<CCameraUser*>(cam);
		// Is this the current camera??
		if(oldCam==_CurrentCamera)
			_CurrentCamera=NULL;

		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Transforms.erase(oldCam);
	}

	virtual	UInstance		*createInstance(const std::string &shapeName)
	{
		IModel	*model= _Scene.createInstance(shapeName);
		// If not found, return NULL.
		if(model==NULL)
			return NULL;

		if( dynamic_cast<CMeshInstance*>(model)==NULL )
			nlerror("UScene::createInstance(): shape is not a mesh");

		// The component is auto added/deleted to _Scene in ctor/dtor.
		return dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, model)) );
	}
	virtual	void			deleteInstance(UInstance *inst)
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Transforms.erase(dynamic_cast<CTransformUser*>(inst));
	}


	virtual	USkeleton		*createSkeleton(const std::string &shapeName)
	{
		IModel	*model= _Scene.createInstance(shapeName);
		// If not found, return NULL.
		if(model==NULL)
			return NULL;

		if( dynamic_cast<CSkeletonModel*>(model)==NULL )
			nlerror("UScene::createSkeleton(): shape is not a skeletonShape");

		// The component is auto added/deleted to _Scene in ctor/dtor.
		return dynamic_cast<USkeleton*>( _Transforms.insert(new CSkeletonUser(&_Scene, model)) );
	}
	virtual	void			deleteSkeleton(USkeleton *skel)
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Transforms.erase(dynamic_cast<CTransformUser*>(skel));
	}


	virtual	ULandscape		*createLandscape()
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		return _Landscapes.insert(new CLandscapeUser(&_Scene));
	}
	virtual	void			deleteLandscape(ULandscape *land)
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Landscapes.erase((CLandscapeUser*) land);
	}


	virtual	UInstanceGroup	*createInstanceGroup (const std::string &instanceGroup)
	{
		// Create the instance group
		CInstanceGroupUser *user=new CInstanceGroupUser;

		// Init the class
		if (!user->init (instanceGroup, _Scene))
		{
			// Prb, erase it
			delete user;

			// Return error code
			return NULL;
		}

		// Insert the pointer in the pointer list
		_InstanceGroups.insert (user);

		// return the good value
		return user;
	}

	virtual	void			deleteInstanceGroup (UInstanceGroup	*group)
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_InstanceGroups.erase (dynamic_cast<CInstanceGroupUser*>(group));
	}

	//@}

};


} // NL3D


#endif // NL_SCENE_USER_H

/* End of scene_user.h */
