/** \file scene_ut.h
 * <File description>
 *
 * $Id: nelu.h,v 1.3 2000/11/15 17:23:24 berenguier Exp $
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

#ifndef NL_SCENE_UT_H
#define NL_SCENE_UT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/scene.h"


namespace NL3D 
{


/**
 * 3d Engine Utilities. Simple Open / Close framework.
 * Designed to work only with a mono-threaded / mono-scene / single-windowed app.
 *
 * If your app want to register other Models with basics CScene traversals, it could use CSceneUt, and register his 
 * models/observers after, or even before CSceneUt::init3d().
 *
 * If your app want to add funky traversals, it MUST NOT use CSceneUt (see CScene for more information...).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CSceneUt
{
public:
	// Default Perspective of camera.
	static const float		DefLx;		//=0.26f;
	static const float		DefLy;		//=0.2f;
	static const float		DefLzNear;	//=0.15f;
	static const float		DefLzFar;	//=1000.0f;

public:
	static NLMISC::CSmartPtr<CCamera>	Camera;

public:

	/** Init all that we need for a single GL window:
	 * - create / init / openWindow / activate a IDriver.
	 * - register scene basics models, 
	 * - init the scene, with basic Traversals, 
	 * - create a default camera, linked to the scene, and with default frustum as specified above.
	 *
	 * After creation, use the CSceneUt::Camera to manipulates the camera of scene (but you may change all you want 
	 * to this camera or create/use an other camera if you want...)
	 *
	 * You can retrieve the created driver with scene.getDriver(). Usefull for window message processing as example.
	 * \param scene a non-inited CScene.
	 */
	static void		init3d(CScene &scene, uint w, uint h, uint bpp=32, bool windowed=true);

	/** Delete all 3d:
	 * - delete the camera.
	 * - release scene.
	 * - release / delete Driver.
	 *
	 */
	static void		release3d(CScene &scn);

public:
	static IDriver	*Driver;
};


} // NL3D


#endif // NL_SCENE_UT_H

/* End of scene_ut.h */
