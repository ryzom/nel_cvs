/** \file nelu.cpp
 * <File description>
 *
 * $Id: nelu.cpp,v 1.6 2000/12/04 13:22:00 berenguier Exp $
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

#include "nel/3d/nelu.h"
#include "nel/3d/dru.h"
#include "nel/3d/camera.h"
#include "nel/misc/debug.h"


namespace NL3D 
{

const float		CNELU::DefLx=0.26f;
const float		CNELU::DefLy=0.2f;
const float		CNELU::DefLzNear=0.15f;
const float		CNELU::DefLzFar=1000.0f;
NLMISC::CSmartPtr<CCamera>	CNELU::Camera;
IDriver			*CNELU::Driver;


void			CNELU::init3d(CScene &scene, uint w, uint h, const CViewport& viewport, uint bpp, bool windowed ) throw(EDru)
{
	// Init debug system
	NLMISC::InitDebug();

	// Init driver.
	CNELU::Driver= CDRU::createGlDriver();
	nlverify(CNELU::Driver->init());
	nlverify(CNELU::Driver->setDisplay(NULL, GfxMode(w, h, bpp, windowed)));
	nlverify(CNELU::Driver->activate());

	// Register basic csene.
	CScene::registerBasics();

	// Init scene.
	scene.initDefaultTravs();

	// Don't add any user trav.
	// init default Roots.
	scene.initDefaultRoots();
	
	// Set driver.
	scene.setDriver(CNELU::Driver);

	// Set viewport
	scene.setViewport (viewport);

	// Create/link a camera.
	CNELU::Camera= (CCamera*)scene.createModel(CameraId);
	scene.CurrentCamera= CNELU::Camera;
	CNELU::Camera->setFrustum(DefLx, DefLy, DefLzNear, DefLzFar);

}


void			CNELU::release3d(CScene &scene)
{
	// Release the camera.
	CNELU::Camera= NULL;

	// "Release" the scene.
	scene.setDriver(NULL);

	// "Release" the driver.
	CNELU::Driver->release();
	delete CNELU::Driver;
	CNELU::Driver= NULL;
}


} // NL3D
