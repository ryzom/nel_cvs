/** \file scene_ut.cpp
 * <File description>
 *
 * $Id: nelu.cpp,v 1.4 2000/12/01 10:10:51 corvazier Exp $
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

#include "nel/3d/scene_ut.h"
#include "nel/3d/dru.h"
#include "nel/3d/camera.h"
#include "nel/misc/debug.h"


namespace NL3D 
{

const float		CSceneUt::DefLx=0.26f;
const float		CSceneUt::DefLy=0.2f;
const float		CSceneUt::DefLzNear=0.15f;
const float		CSceneUt::DefLzFar=1000.0f;
NLMISC::CSmartPtr<CCamera>	CSceneUt::Camera;
IDriver			*CSceneUt::Driver;


void			CSceneUt::init3d(CScene &scene, uint w, uint h, const CViewport& viewport, uint bpp, bool windowed )
{
	// Init debug system
	NLMISC::InitDebug();

	// Init driver.
	CSceneUt::Driver= CDRU::createGlDriver();
	nlverify(CSceneUt::Driver->init());
	nlverify(CSceneUt::Driver->setDisplay(NULL, GfxMode(w, h, bpp, windowed)));
	nlverify(CSceneUt::Driver->activate());

	// Register basic csene.
	CScene::registerBasics();

	// Init scene.
	scene.initDefaultTravs();

	// Don't add any user trav.
	// init default Roots.
	scene.initDefaultRoots();
	
	// Set driver.
	scene.setDriver(CSceneUt::Driver);

	// Set viewport
	scene.setViewport (viewport);

	// Create/link a camera.
	CSceneUt::Camera= (CCamera*)scene.createModel(CameraId);
	scene.CurrentCamera= CSceneUt::Camera;
	CSceneUt::Camera->setFrustum(DefLx, DefLy, DefLzNear, DefLzFar);

}


void			CSceneUt::release3d(CScene &scene)
{
	// Release the camera.
	CSceneUt::Camera= NULL;

	// "Release" the scene.
	scene.setDriver(NULL);

	// "Release" the driver.
	CSceneUt::Driver->release();
	delete CSceneUt::Driver;
	CSceneUt::Driver= NULL;
}


} // NL3D
