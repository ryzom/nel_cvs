/** \file u_scene.h
 * <File description>
 *
 * $Id: u_scene.h,v 1.2 2001/04/09 14:25:39 corvazier Exp $
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


namespace NL3D
{


class UCamera;
class UInstance;
class ULandscape;
class UInstanceGroup;

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
	 */
	virtual	UInstance		*createInstance(const std::string &shapeName)=0;
	/** Delete an instance via his pointer.
	 */
	virtual	void			deleteInstance(UInstance *inst)=0;

	/// Create a landscape. There could be many Landscape per Scene (Driver? :). But never tested :).
	virtual	ULandscape		*createLandscape()=0;
	/// delete a landscape.
	virtual	void			deleteLandscape(ULandscape *land)=0;

	/** Create a new instance group for this scene.
	  * If fails (file not found or serial error), return NULL.
	  */
	virtual	UInstanceGroup	*createInstanceGroup(const std::string &instanceGroup) =0;
	/// Delete an instance group from this scene.
	virtual	void			deleteInstanceGroup(UInstanceGroup	*group) =0;

	//@}

};


} // NL3D


#endif // NL_U_SCENE_H

/* End of u_scene.h */
