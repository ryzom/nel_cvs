/** \file client.cpp
 * Snowballs 2 main file
 *
 * $Id: client.cpp,v 1.16 2001/07/12 13:53:18 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

//
// Includes
// 

#include <nel/misc/types_nl.h>

#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
#include <windows.h>
#endif

#include <nel/misc/command.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/i18n.h>
#include <nel/misc/config_file.h>
#include <nel/misc/vectord.h>
#include <nel/misc/time_nl.h>

#include <string>
#include <deque>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>


#include "commands.h"
#include "landscape.h"
#include "entities.h"
#include "camera.h"
#include "pacs.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

//
// Globals
//

CConfigFile ConfigFile;

UDriver				*Driver = NULL;
UScene				*Scene = NULL;
U3dMouseListener	*MouseListener = NULL;
UInstance			*Cube = NULL;

UTextContext		*TextContext = NULL;

TTime				LastTime, 
					NewTime;


// true if you want to exit the main loop
bool				 NeedExit = false;

bool				 ShowRadar;
bool				 ShowCommands;

// true if the client is in online mode
bool				 Online = false;

//
// Main
//

#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
	nlinfo ("Starting Snowballs 2");

	// Load config file

	ConfigFile.load ("client.cfg");

	ShowCommands = ConfigFile.getVar("ShowCommands").asInt () == 1;
	ShowRadar = ConfigFile.getVar("ShowRadar").asInt () == 1;

	// Manage paths

	string dataPath = ConfigFile.getVar("DataPath").asString ();
	if (dataPath[dataPath.size()-1] != '/') dataPath += '/';
	CPath::addSearchPath (dataPath);
	CPath::addSearchPath (dataPath + "zones/");
	CPath::addSearchPath (dataPath + "tiles/");
	CPath::addSearchPath (dataPath + "meshes/");
	CPath::addSearchPath (dataPath + "materials/");
	CPath::addSearchPath (dataPath + "pacs/");

	// Create a driver
	Driver = UDriver::createDriver();

	// Text context
	Driver->setDisplay (UDriver::CMode(640, 480, 0));
	Driver->setFontManagerMaxMemory (2000000);

	TextContext = Driver->createTextContext (CPath::lookup(ConfigFile.getVar("FontName").asString ()));

	// Create a scene
	Scene = Driver->createScene();

	// load a default cube shape
	Cube = Scene->createInstance("BARMAN.shape");
//	Cube->setTransformMode (UTransformable::DirectMatrix);
	Cube->setScale(1.0f, 1.0f, 1.0f);
	Cube->setPivot(0.0f, 0.0f, 0.0f);
	Cube->show();

	// Camera
	initCamera();

	// Create a 3D mouse listener
	MouseListener = Driver->create3dMouseListener ();
	MouseListener->setHotSpot (CVectorD (0,0,0));
	MouseListener->setFrustrum (Camera->getFrustum());
	MouseListener->setMatrix (Camera->getMatrix());
	MouseListener->setMouseMode (U3dMouseListener::firstPerson);

	// Init the command control
	initCommands ();

	// Init the radar
	initRadar ();

	// Init the landscape using the previously created UScene
	initLandscape();

	// Init the pacs
	initPACS();

	// Display the firsts line
	nlinfo ("Welcome to Snowballs 2");

	LastTime = CTime::getLocalTime();

	while ((!NeedExit) && Driver->isActive())
	{
		// Clear
		Driver->clearBuffers (CRGBA (64,64,64,0));

		// Update the landscape
		updateLandscape ();

		// Update the commands panel
		updateCommands ();

		// update the box
		CMatrix		cmat = MouseListener->getViewMatrix();
		//Cube->setMatrix(cmat);
		Cube->setPos(cmat.getPos());
		CVector	j = cmat.getJ();
		j.z = 0.0f;
		Cube->setRotQuat(j);

		// setup the camera
		updateCamera();
		
		// Render
		Scene->render ();


		
		// Update the commands panel
		if (ShowCommands) updateCommands ();

		// Update the radar
		if (ShowRadar) updateRadar ();

		TextContext->setHotSpot (UTextContext::TopRight);
		TextContext->setColor (CRGBA(255, 0, 0));
		TextContext->setFontSize (20);
		TextContext->printfAt (1.0f, 1.0f, Online?"Online":"Offline");


		// Swap
		Driver->swapBuffers ();

		// Pump messages
		Driver->EventServer.pump();

		// Manage the keyboard
		if (Driver->AsyncListener.isKeyPushed (KeyESCAPE))
		{
			NeedExit = true;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF5))
		{
			ShowCommands = !ShowCommands;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF6))
		{
			ShowRadar = !ShowRadar;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF12))
		{
			clearCommands ();
		}

		// Check if the config file was modified by another program
		CConfigFile::checkConfigFiles ();
	}

	releasePACS();
	releaseLandscape();

	delete Driver;

	return EXIT_SUCCESS;
}

NLMISC_COMMAND(quit,"quit the client","")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 0) return false;

	log.displayNL("Exit requested");

	NeedExit = true;

	return true;
}
