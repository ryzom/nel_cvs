/** \file nelu.cpp
 * <File description>
 *
 * $Id: nelu.cpp,v 1.29 2002/07/25 16:45:48 corvazier Exp $
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

#include "std3d.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "3d/nelu.h"
#include "3d/dru.h"
#include "3d/camera.h"
#include "3d/register_3d.h"
#include "3d/init_3d.h"
#include "nel/misc/debug.h"
using namespace std;
using namespace NLMISC;


namespace NL3D 
{

const float		CNELU::DefLx=0.26f;
const float		CNELU::DefLy=0.2f;
const float		CNELU::DefLzNear=0.15f;
const float		CNELU::DefLzFar=1000.0f;

IDriver				*CNELU::Driver=NULL;
CScene				CNELU::Scene;
CShapeBank			*CNELU::ShapeBank;
CRefPtr<CCamera>	CNELU::Camera;
CEventServer		CNELU::EventServer;
CEventListenerAsync	CNELU::AsyncListener;


bool			CNELU::initDriver (uint w, uint h, uint bpp, bool windowed, void *systemWindow, bool offscreen) throw(EDru)
{
	// Init debug system
//	NLMISC::InitDebug();

	ShapeBank = new CShapeBank;
	// Init driver.
	CNELU::Driver= CDRU::createGlDriver();
	if (!CNELU::Driver->init())
	{
		nlwarning ("CNELU::initDriver: init() failed");
		return false;
	}
	if (!CNELU::Driver->setDisplay(systemWindow, GfxMode(w, h, bpp, windowed, offscreen)))
	{
		nlwarning ("CNELU::initDriver: setDisplay() failed");
		return false;
	}
	if (!CNELU::Driver->activate())
	{
		nlwarning ("CNELU::initDriver: activate() failed");
		return false;
	}
	return true;
}


void			CNELU::initScene(CViewport viewport)
{
	// Register basic csene.
	CScene::registerBasics();

	// Init Scene.
	CNELU::Scene.initDefaultTravs();

	// Don't add any user trav.
	// init default Roots.
	CNELU::Scene.initDefaultRoots();
	
	// Set driver.
	CNELU::Scene.setDriver(CNELU::Driver);

	// Set viewport
	CNELU::Scene.setViewport (viewport);

	// Init the world instance group
	CNELU::Scene.initGlobalnstanceGroup();

	// Create coarse mesh manager.
	CNELU::Scene.initCoarseMeshManager ();

	// init QuadGridClipManager
	CNELU::Scene.initQuadGridClipManager ();

	// Create/link a camera.
	CNELU::Camera= (CCamera*)Scene.createModel(NL3D::CameraId);
	CNELU::Scene.setCam(CNELU::Camera);
	CNELU::Camera->setFrustum(DefLx, DefLy, DefLzNear, DefLzFar);

	// Link to the shape bank
	CNELU::Scene.setShapeBank(CNELU::ShapeBank);
}


void			CNELU::initEventServer()
{
	CNELU::AsyncListener.reset ();
	CNELU::EventServer.addEmitter(CNELU::Driver->getEventEmitter());
	CNELU::AsyncListener.addToServer(CNELU::EventServer);
}


void			CNELU::releaseEventServer()
{
	CNELU::AsyncListener.removeFromServer(CNELU::EventServer);
	if (CNELU::Driver != NULL)
	{
		CNELU::EventServer.removeEmitter(CNELU::Driver->getEventEmitter());
	}
}

	
void			CNELU::releaseScene()
{
	// Release the camera.
	CNELU::Camera= NULL;

	// "Release" the Scene.
	CNELU::Scene.setDriver(NULL);
	CNELU::Scene.release();
}


void			CNELU::releaseDriver()
{
	// "Release" the driver.
	if (CNELU::Driver != NULL)
	{
		CNELU::Driver->release();
		delete CNELU::Driver;
		CNELU::Driver = NULL;
	}
	if( CNELU::ShapeBank != NULL )
	{
		delete CNELU::ShapeBank;
		CNELU::ShapeBank = NULL;
	}
}

bool			CNELU::init (uint w, uint h, CViewport viewport, uint bpp, bool windowed, void *systemWindow, bool offscreen) throw(EDru)
{
	NL3D::registerSerial3d();
	if (initDriver(w,h,bpp,windowed,systemWindow,offscreen))
	{
		initScene(viewport);
		initEventServer();
		return true;
	}
	else
		return false;
}

void			CNELU::release()
{
	releaseEventServer();
	releaseScene();
	releaseDriver();
}

void			CNELU::screenshot()
{
	if (AsyncListener.isKeyPushed(KeyF12))
	{
		CBitmap btm;
		CNELU::Driver->getBuffer(btm);
		string filename = CFile::findNewFile ("screenshot.tga");
		COFile fs(filename);
		btm.writeTGA (fs,24,true);
		nlinfo("Screenshot '%s' saved", filename.c_str());
	}
}


void			CNELU::clearBuffers(CRGBA col)
{
	CNELU::Driver->clear2D(col);
	CNELU::Driver->clearZBuffer();
}

void			CNELU::swapBuffers()
{
	CNELU::Driver->swapBuffers();
}



} // NL3D
